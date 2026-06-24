#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>
#include <zephyr/debug/symtab.h>

#include <stcp/debug.h>
#include <stcp/low_level_operations.h>
#include <stcp/stcp_alloc.h>
#include <stcp/stcp_struct.h>
#include <stcp/fsm.h>
#include <stcp/sanity.h>

#define DEBUG_MUTEX_TIMEOUT_MS          (5*1000)

#define D_LOCK(lock, CODE) \
    do {                                                     \
        stcp_debug_info_lock();                              \
        LDBG("Locking debug inner: %s", #lock);              \
        int timeout = k_mutex_lock(&(lock),                  \
                        K_MSEC(DEBUG_MUTEX_TIMEOUT_MS));     \
        if (timeout != 0) {                                  \
            LDBG("Debug mutex timeout!");                    \
            CODE;                                            \
        };                                                   \
        LDBG("Locked debug...");                             \
    } while(0)

#define D_UNLOCK(lock) \
    do {                                  \
        LDBG("Unlocking debug...");       \
        k_mutex_unlock(&(lock));          \
        LDBG("Unlocked debug outer...");  \
        stcp_debug_info_unlock();         \
    } while(0)

static struct k_mutex g_debug_mutex_create_new;
static atomic_t g_debug_mutex_create_new_init = ATOMIC_INIT(0);
static atomic_t debug_in_progress = ATOMIC_INIT(0);

char * stcp_debug_find_symbol_name(uint32_t *pFrom) {
#ifdef CONFIG_SYMTAB
    if (pFrom) {
        uint32_t offs;
        return symtab_find_symbol_name((uint32_t)pFrom, &offs);
    }
#endif
    return "N/A";
}


void stcp_debug_info_lock() {

    if (!g_debug_mutex_create_new_init) {
        g_debug_mutex_create_new_init = 1;
        k_mutex_init(&g_debug_mutex_create_new);
    }
    LDBG("Locking for NEW ...");
    k_mutex_lock(&g_debug_mutex_create_new, K_FOREVER);
    LDBG("Locked for NEW ...");
}

void stcp_debug_info_unlock() {
    if (!g_debug_mutex_create_new_init) {
        g_debug_mutex_create_new_init = 1;
        k_mutex_init(&g_debug_mutex_create_new);
    }
    LDBG("Unlocking for NEW ...");
    k_mutex_unlock(&g_debug_mutex_create_new);
    LDBG("Unlocked for NEW ...");
}

void stcp_debug_info_free(struct stcp_debug_info* DIP) {
    if (!DIP) {
        return;
    }
    
    D_LOCK(DIP->lock, return);

        struct stcp_debug_stack_list_item *last_bt_item = DIP->backtrace;
        LDBG("DIP last item: %p", last_bt_item);
        if (last_bt_item) {
            do {
                LDBG("Iterating: %p", last_bt_item);
                void *next = last_bt_item->next;
                LDBG("Freeing: %p, Next: %p", last_bt_item, next);

                k_stcp_free(last_bt_item);
                last_bt_item = NULL;

                last_bt_item = next;
            } while (last_bt_item != NULL);
        }

    D_UNLOCK(DIP->lock);

    k_stcp_free(DIP);
    DIP = NULL;
    
}

int stcp_debug_info_count_list_items_no_lock(struct stcp_debug_stack_list_item *list) {
    if (!list) {
        return 0;
    }
    int i = 0;
    struct stcp_debug_stack_list_item *last_bt_item = list;
    while (last_bt_item != NULL) {
        void *next = last_bt_item->next;
        i++;
        last_bt_item = next;
    }

    return i;
}

int stcp_debug_info_count_list_items(struct stcp_debug_info* DIP, struct stcp_debug_stack_list_item *list) {
    if (!DIP) {
        return 0;
    }
    
    D_LOCK(DIP->lock, return -ENODATA);
        int itemcount = stcp_debug_info_count_list_items_no_lock(list);
    D_UNLOCK(DIP->lock);
    return itemcount;
}

void stcp_debug_info_dump(struct stcp_debug_info* DIP) {
    if (!DIP) {
        return NULL;
    }
    int depth = 0;
    D_LOCK(DIP->lock, return );
    LDBGBIG("DEBUG: Start to dump info .................");
    struct stcp_debug_stack_list_item *last_bt_item = DIP->backtrace;
        int counter = stcp_debug_info_count_list_items_no_lock(DIP->backtrace);

        LDBG("Backtrace from lr %p (%s, in rust: %d) @ %d ms...", 
            last_bt_item->item.lr, 
            stcp_debug_find_symbol_name(last_bt_item->item.lr),
            last_bt_item->item.timestamp);
        do {
            LDBG("   Depth: %02d %s (LR:%p) TS: %d ms", 
                depth, 
                stcp_debug_find_symbol_name(last_bt_item->item.lr),
                last_bt_item->item.lr, 
                last_bt_item->item.timestamp);

            last_bt_item = last_bt_item->next;
            depth++;
        } while (last_bt_item != NULL && depth < counter);
    LDBG("EOD.");
    D_UNLOCK(DIP->lock);
}

void* stcp_debug_info_new() {
    stcp_debug_info_lock();
        struct stcp_debug_info *dip = k_stcp_alloc(sizeof(struct stcp_debug_info));
        memset(dip, 0, sizeof(struct stcp_debug_info));
        k_mutex_init(&dip->lock);
    stcp_debug_info_unlock();
    return dip;
}

// EI OTA LUKKOA
void* stcp_debug_info_add_backtrace_element(struct stcp_debug_info* DIP, uintptr_t lr, uintptr_t sp) {

    if (!DIP) {
        return NULL;
    }

    struct stcp_debug_stack_list_item *last_bt_item = DIP->backtrace;
    if (last_bt_item != NULL) {
        while (last_bt_item->next != NULL) {
            last_bt_item = last_bt_item->next;
            LDBG("Past of %p", last_bt_item);
        }
    }
    // Viimeisessä nyt..
    struct stcp_debug_stack_list_item *new = 
        k_stcp_alloc(sizeof(struct stcp_debug_stack_list_item));

    memset(new, 0, sizeof(struct stcp_debug_stack_list_item));

    if (!new) {
        LWRN("DEBUG: Alloc failed, element lr: %lu", lr);
        return DIP;
    }

    new->item.lr = lr;
    new->item.timestamp = STCP_GET_TIMESTAMP;

    if (last_bt_item) {
        last_bt_item->next = new;
    } else {
        DIP->backtrace = new;
    }

    return DIP;
}

int is_address_legit(uintptr_t val) {
    return is_stack_ptr_valid(val);
}

struct stcp_debug_info *stcp_debug_info_snapshot(void)
{

    stcp_debug_info_lock();

        void *lr;
        void *sp;
        __asm volatile ("mov %0, lr" : "=r"(lr));
        __asm volatile ("mov %0, sp" : "=r"(sp));

        uintptr_t stack = (uintptr_t)sp;
        LDBG("Creating bt element from SP %lu", stack);

        if (!is_address_legit(stack)) {
            LDBG("Stack pointer invalid!");
            return;
        }

        struct stcp_debug_info *dip = stcp_debug_info_new();

        if (!dip){
            LERRBIG("OOM, mana low!");
            return NULL;
        }

        dip->stack_ptr = (uintptr_t)sp;
        

        for (int i = 0; i < 16; i++) {
            uintptr_t addr = ((uintptr_t*)sp)[i];
            if (is_prt_at_text_addr(addr)) {
                LDBG("Adding from stack: %lu (%s)", addr, stcp_debug_find_symbol_name(addr));
                stcp_debug_info_add_backtrace_element(dip, addr, sp);
            }
        }

    stcp_debug_info_lock();

    return dip;
}

uintptr_t stcp_get_address_of_valid_symbol_at_stack_frame(unsigned int frame)
{
    void *sp;

    __asm volatile ("mov %0, sp" : "=r"(sp));

    uintptr_t *stack = (uintptr_t *)sp;
    
    if (frame < 16) {
        uintptr_t addr = stack[frame];
        if (addr >= 0x00000000 && addr < 0x00100000) {
            if (stcp_debug_find_symbol_name((uint32_t *)addr) != NULL) {
                return addr;
            }
        }
    }

    return 0;
}

void stcp_dump_bt(void)
{
    void *lr;
    void *sp;

    __asm volatile ("mov %0, lr" : "=r"(lr));
    __asm volatile ("mov %0, sp" : "=r"(sp));

    printk("---- STCP BACKTRACE ----\n");
    printk("LR=%p SP=%p\n", lr, sp);

    uintptr_t *stack = (uintptr_t *)sp;

    for (int i = 0; i < 16; i++) {
        uintptr_t addr = stack[i];

        if (addr >= 0x00000000 && addr < 0x00100000) {
            printk("#%d %p (%s)\n", i, (void *)addr, stcp_debug_find_symbol_name((uint32_t *)addr));
        }
    }

    printk("------------------------\n");
}


void stcp_debug_dump_stcp_ctx(void *pFrom) {
    void *pLR =  __builtin_return_address(0);

#define __GAT(val)    (int)atomic_get(&(val))
#define GET_ATOM(val) __GAT(val)

    if (k_is_in_isr()) {
        // EI Printtejä!
        return;
    }

    struct stcp_ctx* ctx = pFrom;
    if (!stcp_is_context_valid_no_ref(ctx)) {
        LERR("Not valid context!");
        return;
    }

    stcp_context_lifespan_extend(ctx);
    CONTEXT_LOCK(ctx);

        LDBG("Context %p dump from LR: %p", ctx, pLR);
        LDBG("    Magic (header)   : 0x%08x", ctx->magic);
        LDBG("    Magic (footer)   : 0x%08x", ctx->magic_footer);
        LDBG("    refcnt           : %d", GET_ATOM(ctx->refcnt));
        LDBG("    closing          : %d", GET_ATOM(ctx->closing));
        LDBG("    connection_closed: %d", GET_ATOM(ctx->connection_closed));
        LDBG("    destroyed        : %d", GET_ATOM(ctx->destroyed));
        LDBG("    connected        : %d", GET_ATOM(ctx->connected));
        LDBG("    handshake_done   : %d", ctx->handshake_done);
        LDBG("    poll_timeouts    : %d", ctx->poll_timeouts);

        // TODO: simplify this.
        LDBG("    rx_frame_len     : %zu", ctx->rx_frame_len);
        LDBG("    rx_payload_pos   : %zu", ctx->rx_payload_pos);
        LDBG("    rx_payload_len   : %zu", ctx->rx_payload_len);
        LDBG("    RX stream pos    : %zu", ctx->rx_stream.pos);
        LDBG("    RX stream len    : %zu", ctx->rx_stream.len);

        LDBG("  Pointers:");
        LDBG("    STCP CTX (C)     : %p", ctx);
        LDBG("    API              : %p", ctx->api);
        LDBG("    Session (RUST)   : %p", ctx->session);
        LDBG("    RX Buffer        : %p", ctx->rx_stream.buffer);
        LDBG("    DNS Resolved     : %p", ctx->dns_resolved);

        LDBG("  States (C):");
        LDBG("    API FSM          : %d", GET_API_STATE_FROM_CTX(ctx));
        LDBG("    Transport FD     : %d", ctx->ks.fd);
        
    CONTEXT_UNLOCK(ctx);
    stcp_context_lifespan_shorten(ctx);
    
}

int stcp_check_if_access_is_ok(void *pFrom) {
    return stcp_fsm_check_if_access_is_granted((struct stcp_api *)pFrom);
}
