#include <stdint.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <stcp/stcp_buffer_pool.h>
LOG_MODULE_REGISTER(stcp_buffer_pool, CONFIG_STCP_LOG_LEVEL);
#define FRAME_SIZE 1152U
#define LARGE_SIZE 8192U
#define FRAME_COUNT CONFIG_STCP_BUFFER_POOL_FRAMES
#define LARGE_COUNT CONFIG_STCP_BUFFER_POOL_LARGE
struct frame_slot { _Alignas(16) unsigned char bytes[FRAME_SIZE]; };
struct large_slot { _Alignas(16) unsigned char bytes[LARGE_SIZE]; };
static struct frame_slot frames[FRAME_COUNT];
static struct large_slot large[LARGE_COUNT];
enum slot_mode { SLOT_FREE = 0, SLOT_RX = 1, SLOT_TX = 2 };
/* Each region has its own mutex. mode is the exclusive buffer lease; the
 * mutex protects claiming/releasing that lease, not the entire IO lifetime.
 * This lets a different thread return a completed frame safely. */
struct buffer_slot {
    int mode;
    struct k_mutex lock;
    size_t size;
    unsigned char *ptr;
    unsigned epoch;
};
static struct buffer_slot frame_slots[FRAME_COUNT], large_slots[LARGE_COUNT];
static int stcp_buffer_pool_init(void)
{
    for (unsigned i = 0; i < FRAME_COUNT; ++i) {
        k_mutex_init(&frame_slots[i].lock);
        frame_slots[i].size = FRAME_SIZE;
        frame_slots[i].ptr = frames[i].bytes;
    }
    for (unsigned i = 0; i < LARGE_COUNT; ++i) {
        k_mutex_init(&large_slots[i].lock);
        large_slots[i].size = LARGE_SIZE;
        large_slots[i].ptr = large[i].bytes;
    }
    return 0;
}
SYS_INIT(stcp_buffer_pool_init, PRE_KERNEL_1, 0);
static struct k_spinlock pool_lock;
static unsigned frame_used, large_used, frame_peak, large_peak;
static unsigned failures;
/* POOL_FLOW_FIX: reserve transient TX allocations atomically. */
#define RX_RESERVE 6U
#define RX_WORK_SLOTS 4U
static K_MUTEX_DEFINE(tx_gate);
static k_tid_t tx_thread;
static unsigned tx_credits, tx_epoch, tx_blocked, rx_deferred;


bool stcp_buffer_tx_begin(size_t frame_count)
{
    if (frame_count > (FRAME_COUNT - RX_RESERVE) / 2U) return false;
    if (k_mutex_lock(&tx_gate, K_NO_WAIT) != 0) return false;
    k_spinlock_key_t key = k_spin_lock(&pool_lock);
    unsigned needed = (unsigned)frame_count * 2U;
    bool ready = FRAME_COUNT - frame_used >= needed + RX_RESERVE;
    if (ready) {
        ++tx_epoch;
        if (!tx_epoch) ++tx_epoch;
        tx_thread = k_current_get();
        tx_credits = needed;
    }
    k_spin_unlock(&pool_lock, key);
    if (!ready) {
        key = k_spin_lock(&pool_lock);
        ++tx_blocked;
        k_spin_unlock(&pool_lock, key);
        k_mutex_unlock(&tx_gate);
    }
    return ready;
}
void stcp_buffer_tx_end(void)
{
    k_spinlock_key_t key = k_spin_lock(&pool_lock);
    tx_thread = NULL;
    tx_credits = 0;
    k_spin_unlock(&pool_lock, key);
    k_mutex_unlock(&tx_gate);
}
bool stcp_buffer_tx_ready(size_t frame_count)
{
    if (frame_count > (FRAME_COUNT - RX_RESERVE) / 2U) return false;
    k_spinlock_key_t key = k_spin_lock(&pool_lock);
    bool ready = !tx_thread && FRAME_COUNT - frame_used >= frame_count * 2U + RX_RESERVE;
    k_spin_unlock(&pool_lock, key);
    return ready;
}
bool stcp_buffer_rx_ready(void)
{
    k_spinlock_key_t key = k_spin_lock(&pool_lock);
    bool ready = FRAME_COUNT - frame_used >= tx_credits + RX_WORK_SLOTS;
    if (!ready) ++rx_deferred;
    k_spin_unlock(&pool_lock, key);
    return ready;
}
void *stcp_buffer_alloc(size_t size)
{
    struct buffer_slot *slots;
    unsigned count;
    bool small = size <= FRAME_SIZE;
    if (small) { slots = frame_slots; count = FRAME_COUNT; }
    else if (size <= LARGE_SIZE) { slots = large_slots; count = LARGE_COUNT; }
    else { slots = NULL; count = 0; }

    for (unsigned i = 0; i < count; ++i) {
        struct buffer_slot *slot = &slots[i];
        if (k_mutex_lock(&slot->lock, K_NO_WAIT) != 0) continue;
        if (slot->mode != SLOT_FREE) {
            k_mutex_unlock(&slot->lock);
            continue;
        }
        /* Lock order is always slot -> accounting. Never wait for a slot
         * with the accounting spinlock held. Recheck admission at claim. */
        k_spinlock_key_t key = k_spin_lock(&pool_lock);
        bool tx = tx_thread && tx_thread == k_current_get();
        bool admitted = !small ||
            (tx ? tx_credits != 0 : FRAME_COUNT - frame_used > tx_credits);
        if (admitted) {
            slot->mode = tx ? SLOT_TX : SLOT_RX;
            slot->epoch = small && tx ? tx_epoch : 0;
            if (small) {
                if (tx) --tx_credits;
                if (++frame_used > frame_peak) frame_peak = frame_used;
            } else if (++large_used > large_peak) large_peak = large_used;
        }
        k_spin_unlock(&pool_lock, key);
        if (admitted) {
            /* Clear the full region on every acquisition, before publishing
             * its pointer. Hold this slot's mutex; keep interrupts enabled. */
            memset(slot->ptr, 0, slot->size);
        }
        k_mutex_unlock(&slot->lock);
        if (admitted) return slot->ptr;
        break;
    }
    k_spinlock_key_t key = k_spin_lock(&pool_lock);
    ++failures;
    k_spin_unlock(&pool_lock, key);
    return NULL;
}
bool stcp_buffer_free(void *ptr)
{
    if (!ptr) return true;
    for (unsigned group = 0; group < 2; ++group) {
        struct buffer_slot *slots = group == 0 ? frame_slots : large_slots;
        unsigned count = group == 0 ? FRAME_COUNT : LARGE_COUNT;
        for (unsigned i = 0; i < count; ++i) {
            struct buffer_slot *slot = &slots[i];
            if (slot->ptr != ptr) continue;
            k_mutex_lock(&slot->lock, K_FOREVER);
            if (slot->mode != SLOT_FREE) {
                /* Clear on the next acquisition, once, before reuse. */
                k_spinlock_key_t key = k_spin_lock(&pool_lock);
                if (group == 0) {
                    --frame_used;
                    if (tx_thread && slot->epoch == tx_epoch) ++tx_credits;
                } else --large_used;
                slot->epoch = 0;
                slot->mode = SLOT_FREE;
                k_spin_unlock(&pool_lock, key);
            }
            k_mutex_unlock(&slot->lock);
            return true;
        }
    }
    return false;
}
void stcp_buffer_pool_report(void)
{
    unsigned fu, lu, fp, lp, fail, tb, rd;
    k_spinlock_key_t key = k_spin_lock(&pool_lock);
    fu=frame_used; lu=large_used; fp=frame_peak; lp=large_peak; fail=failures; tb=tx_blocked; rd=rx_deferred;
    k_spin_unlock(&pool_lock, key);
    LOG_ERR("STCP_POOL frame=%u/%u large=%u/%u peak=%u/%u failures=%u static_bytes=%u tx_wait=%u rx_defer=%u",
            fu, (unsigned)FRAME_COUNT, lu, (unsigned)LARGE_COUNT,
            fp, lp, fail, (unsigned)(sizeof(frames)+sizeof(large)+sizeof(frame_slots)+sizeof(large_slots)), tb, rd);
}
