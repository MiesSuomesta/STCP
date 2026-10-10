#include <stdint.h>
#include <string.h>
#include <zephyr/kernel.h>
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
static bool frame_busy[FRAME_COUNT], large_busy[LARGE_COUNT];
static struct k_spinlock pool_lock;
static unsigned frame_used, large_used, frame_peak, large_peak;
static unsigned failures;
/* POOL_FLOW_FIX: reserve transient TX allocations atomically. */
#define RX_RESERVE 6U
#define RX_WORK_SLOTS 4U
static K_MUTEX_DEFINE(tx_gate);
static k_tid_t tx_thread;
static unsigned tx_credits, tx_epoch, tx_blocked, rx_deferred;
static unsigned frame_epoch[FRAME_COUNT];

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
    void *result = NULL;
    k_spinlock_key_t key = k_spin_lock(&pool_lock);
    if (size <= FRAME_SIZE) {
        bool tx = tx_thread && tx_thread == k_current_get();
        /* Non-TX allocations cannot consume another thread's reservation. */
        if ((tx && tx_credits == 0) || (!tx && FRAME_COUNT - frame_used <= tx_credits)) {
            k_spin_unlock(&pool_lock, key);
            return NULL;
        }
        for (unsigned i = 0; i < FRAME_COUNT; ++i) {
            if (!frame_busy[i]) {
                frame_busy[i] = true; result = frames[i].bytes;
                frame_epoch[i] = tx ? tx_epoch : 0;
                if (tx) --tx_credits;
                if (++frame_used > frame_peak) frame_peak = frame_used;
                break;
            }
        }
    } else if (size <= LARGE_SIZE) {
        for (unsigned i = 0; i < LARGE_COUNT; ++i) {
            if (!large_busy[i]) {
                large_busy[i] = true; result = large[i].bytes;
                if (++large_used > large_peak) large_peak = large_used;
                break;
            }
        }
    }
    if (!result) ++failures;
    k_spin_unlock(&pool_lock, key);
    return result;
}
bool stcp_buffer_free(void *ptr)
{
    if (!ptr) return true;
    uintptr_t address = (uintptr_t)ptr;
    bool owned = false;
    k_spinlock_key_t key = k_spin_lock(&pool_lock);
    for (unsigned i = 0; i < FRAME_COUNT; ++i) {
        if (address == (uintptr_t)frames[i].bytes) {
            owned = true;
            if (frame_busy[i]) {
                memset(frames[i].bytes, 0, FRAME_SIZE);
                frame_busy[i] = false; --frame_used;
                if (tx_thread && frame_epoch[i] == tx_epoch) ++tx_credits;
                frame_epoch[i] = 0;
            }
            break;
        }
    }
    if (!owned) for (unsigned i = 0; i < LARGE_COUNT; ++i) {
        if (address == (uintptr_t)large[i].bytes) {
            owned = true;
            if (large_busy[i]) {
                memset(large[i].bytes, 0, LARGE_SIZE);
                large_busy[i] = false; --large_used;
            }
            break;
        }
    }
    k_spin_unlock(&pool_lock, key);
    return owned;
}
void stcp_buffer_pool_report(void)
{
    unsigned fu, lu, fp, lp, fail, tb, rd;
    k_spinlock_key_t key = k_spin_lock(&pool_lock);
    fu=frame_used; lu=large_used; fp=frame_peak; lp=large_peak; fail=failures; tb=tx_blocked; rd=rx_deferred;
    k_spin_unlock(&pool_lock, key);
    LOG_ERR("STCP_POOL frame=%u/%u large=%u/%u peak=%u/%u failures=%u static_bytes=%u tx_wait=%u rx_defer=%u",
            fu, (unsigned)FRAME_COUNT, lu, (unsigned)LARGE_COUNT,
            fp, lp, fail, (unsigned)(sizeof(frames)+sizeof(large)), tb, rd);
}
