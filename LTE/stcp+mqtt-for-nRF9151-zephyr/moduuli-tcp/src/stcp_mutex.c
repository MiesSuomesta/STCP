#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>
#include <zephyr/debug/symtab.h>

#include <stcp/debug.h>
#include <stcp/settings.h>
#include <stcp/low_level_operations.h>
#include <stcp/stcp_alloc.h>
#include <stcp/stcp_struct.h>
#include <stcp/fsm.h>

#include <stcp/stcp_mutex.h>

int stcp_mutex_lock(struct stcp_mutex *lock, int timeout_ms) {
    void *pLR =  __builtin_return_address(0);
    const char *name = stcp_debug_find_symbol_name(pLR);
    k_tid_t self = k_current_get();

    int timeout = -1;

    int deadlock = lock->owner == self;

    if (deadlock) {

        LERRBIG("STCP MUTEX: Already locked by (%s / LR: %p) => DEADLOCK called from %s (LR: %p)",
            stcp_debug_find_symbol_name(lock->last_lr_lock),
            lock->last_lr_lock,
            name,
            pLR);

#if STCP_STCP_FSM_TRACKING
        LDBGBIG("Dumping FSM status....");
        stcp_trace_fsm_dump_status();
#endif

        stcp_dump_bt();
        k_panic();
    }


    if (timeout_ms < 0) {
        LDBG("STCP MUTEX: Locking @ %s .. waiting forever", name);
        k_mutex_lock(&(lock->mutex), K_FOREVER);
        timeout = 0;
    } else {
        LDBG("STCP MUTEX: Locking @ %s .. timeout: %d ms", name, timeout_ms);
        timeout = k_mutex_lock(&(lock->mutex), K_MSEC(timeout_ms));
    }

    if (!timeout) {
        LDBG("STCP MUTEX: Locked @ %s with timeout: %d ms", 
            name, timeout_ms
        );
        lock->last_lr_lock = pLR;
        lock->last_lock_ts = k_uptime_get_32();
        lock->owner = self;
    } else {

        LERR("STCP MUTEX: Timeout @ %s with timeout: %d ms, locked by %s / %p", 
            name, timeout_ms,
            stcp_debug_find_symbol_name(lock->last_lr_lock),
            lock->last_lr_lock
        );
    }

    return timeout;
}


int stcp_mutex_init(struct stcp_mutex *newMutex) {

    memset(newMutex, 0, sizeof(*newMutex));
    k_mutex_init(&newMutex->mutex);
    return 0;
}



int stcp_mutex_unlock(struct stcp_mutex *lock) {
    void *pLR =  __builtin_return_address(0);
    const char *name = stcp_debug_find_symbol_name(pLR);
    k_tid_t self = k_current_get();


    LDBG("STCP MUTEX: Unlocking @ %s ..", name);
    if (lock->owner != self) {

        LERRBIG(
            "STCP MUTEX: Unlock by NON OWNER "
            "owner=%p current=%p "
            "locked_from=%s/%p "
            "unlock_from=%s/%p",

            lock->owner,
            self,

            stcp_debug_find_symbol_name(lock->last_lr_lock),
            lock->last_lr_lock,

            name,
            pLR
        );

#if STCP_STCP_FSM_TRACKING
        LDBGBIG("Dumping FSM status....");
        stcp_trace_fsm_dump_status();
#endif

        stcp_dump_bt();
        k_panic();
    }


    LDBG("STCP MUTEX: Unlocking @ %s ....", name);
    int rc = k_mutex_unlock(&(lock->mutex));
    int locked_duration = k_uptime_get_32() - lock->last_lock_ts;
    LDBG("STCP MUTEX: Unlocked @ %s (Lock held %d ms, rc:%d)", name, locked_duration, rc);
    lock->last_lr_unlock = pLR;
    lock->owner = NULL;

    return rc;
}


