#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>

#include "lte_worker.h"
#include "stcp_lte_transport.h"

LOG_MODULE_REGISTER(stcp_lte_worker, LOG_LEVEL_INF);

K_THREAD_STACK_DEFINE(lte_worker_stack, CONFIG_STCP_LTE_WORKER_STACK_SIZE);

static struct k_work_q lte_work_q;
static struct k_work_delayable reconnect_work;
static atomic_t worker_initialized = ATOMIC_INIT(0);
static atomic_t reconnect_pending = ATOMIC_INIT(0);
static uint32_t retry_delay_seconds = CONFIG_STCP_LTE_RECONNECT_INITIAL_DELAY_SECONDS;

static void reconnect_handler(struct k_work *work)
{
    int ret;

    ARG_UNUSED(work);

    LOG_INF("LTE worker: recovering data connection");
    ret = stcp_lte_transport_recover();
    if (ret == 0) {
        retry_delay_seconds = CONFIG_STCP_LTE_RECONNECT_INITIAL_DELAY_SECONDS;
        atomic_clear(&reconnect_pending);
        LOG_INF("LTE worker: connection recovered");
        return;
    }

    LOG_WRN("LTE worker: recovery failed (%d), retry in %u s",
            ret, retry_delay_seconds);

    (void)k_work_reschedule_for_queue(&lte_work_q, &reconnect_work,
                                      K_SECONDS(retry_delay_seconds));

    retry_delay_seconds = MIN(retry_delay_seconds * 2U,
                              (uint32_t)CONFIG_STCP_LTE_RECONNECT_MAX_DELAY_SECONDS);
}

int stcp_lte_worker_init(void)
{
    if (!atomic_cas(&worker_initialized, 0, 1)) {
        return 0;
    }

    k_work_queue_init(&lte_work_q);
    k_work_queue_start(&lte_work_q, lte_worker_stack,
                       K_THREAD_STACK_SIZEOF(lte_worker_stack),
                       CONFIG_STCP_LTE_WORKER_PRIORITY, NULL);
    k_thread_name_set(&lte_work_q.thread, "stcp_lte_reconnect");

    k_work_init_delayable(&reconnect_work, reconnect_handler);
    LOG_INF("LTE reconnect worker started");
    return 0;
}

int stcp_lte_worker_schedule(k_timeout_t delay)
{
    int ret;

    if (!atomic_get(&worker_initialized)) {
        ret = stcp_lte_worker_init();
        if (ret < 0) {
            return ret;
        }
    }

    if (!atomic_cas(&reconnect_pending, 0, 1)) {
        return 0;
    }

    retry_delay_seconds = CONFIG_STCP_LTE_RECONNECT_INITIAL_DELAY_SECONDS;
    ret = k_work_reschedule_for_queue(&lte_work_q, &reconnect_work, delay);
    if (ret < 0) {
        atomic_clear(&reconnect_pending);
        return ret;
    }

    LOG_WRN("LTE reconnect scheduled");
    return 0;
}

void stcp_lte_worker_cancel(void)
{
    if (!atomic_get(&worker_initialized)) {
        return;
    }

    (void)k_work_cancel_delayable(&reconnect_work);
    atomic_clear(&reconnect_pending);
    retry_delay_seconds = CONFIG_STCP_LTE_RECONNECT_INITIAL_DELAY_SECONDS;
}

bool stcp_lte_worker_pending(void)
{
    return atomic_get(&reconnect_pending) != 0;
}
