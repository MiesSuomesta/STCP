#include <errno.h>
#include <modem/lte_lc.h>
#include <modem/pdn.h>

#include <zephyr/net/socket.h>
#include <zephyr/posix/poll.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/sys/atomic.h>
#include <modem/nrf_modem_lib.h>


#include <stcp_api.h>
#include <stcp/debug.h>
#include <stcp/stcp_alloc.h>
#include <stcp/stcp_struct.h>
#include <stcp/utils.h>
#include <stcp/stcp_net.h>
#include <stcp/workers.h>
#include <stcp/stcp_operations_zephyr.h>
#include <stcp/stcp_rust_exported_functions.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/recovery.h>


// 2kb stack
K_THREAD_STACK_DEFINE(stcp_lte_worker_stack, (2048));
static struct k_work_q stcp_lte_work_reconnect_q;
struct k_work stcp_lte_reconnect_work;
atomic_t g_is_reconnect_scheduled_already;

#define STCP_LTE_DO_HARD_RESET_ON_RECONNECT     0

static int stcp_lte_workers_init(const struct device *dev)
{
    ARG_UNUSED(dev);
    k_work_queue_start(&stcp_lte_work_reconnect_q,
                       stcp_lte_worker_stack,
                       K_THREAD_STACK_SIZEOF(stcp_lte_worker_stack),
                       5,   // priority
                       NULL);

    LINF("LTE worker queue started");
    return 0;
}

SYS_INIT(stcp_lte_workers_init, APPLICATION, 90);

void worker_reconnect_work_handler(struct k_work *work);
void worker_reconnect_context_init(struct stcp_ctx *ctx) {
    LDBG("Init of worker..");
    k_work_init(&stcp_lte_reconnect_work, worker_reconnect_work_handler);
    LDBG("Init of worker, DONE");
}

void worker_reconnect_work_handler(struct k_work *work)
{
    int ok = 1;
    LINF("Worker thread started prio=%d", k_thread_priority_get(k_current_get()));
    LDBG("STCP reconnect staring running for %p\n", work);

    LINF("LTE Worker: modem reset (hard=%d)",
        STCP_LTE_DO_HARD_RESET_ON_RECONNECT);

#if 0
    /* reset modem */
    int err = stcp_api_reset_modem(STCP_LTE_DO_HARD_RESET_ON_RECONNECT);
    k_yield();
    if (err != 0) {
        LWRN("LTE Worker: Modem reset reported error, rc: %d", err);
        ok = 0;
    }
    /* pieni tauko että modem ehtii nousta */
    stcp_utils_sleep_ms_jitter(2000, 400);
#endif
    /* reconnect LTE */
    
    LINF("LTE Worker: Recovering connection....");
    int err = stcp_lte_recover();
    if (err != 0) {
        LWRN("LTE Worker: LTE Recovery reported error, rc: %d", err);
        ok = 0;
    }

    atomic_set(&g_is_reconnect_scheduled_already, 0);
    LINF("LTE: All done, status: %d", ok);
}

static void __reconnecting_do_init() {
    static int done = 0;
    if (!done) {
        LDBG("Work init...");
        atomic_set(&g_is_reconnect_scheduled_already, 0);
        k_work_init(&stcp_lte_reconnect_work, worker_cleanup_work_handler); 
        done = 1;
    }
}

void worker_schedule_lte_reconnect() {
    __reconnecting_do_init();

    if (atomic_get(&g_is_reconnect_scheduled_already)) {
        LWRN("LTE: Already requested reconnecting...");
        return;
    }
    LINF("LTE: Setting reconnecting requested...");
	atomic_set(&g_is_reconnect_scheduled_already, 1);
    LINF("LTE: Scheduling reconnect...");
    /* Aikatauluta reconnect */
    k_work_submit_to_queue(&stcp_lte_work_reconnect_q, &stcp_lte_reconnect_work);
    LINF("LTE: All ok, reconnect scheduled.");
}
