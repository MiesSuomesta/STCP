
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <stdbool.h>
#include <errno.h>


#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/debug.h>

#define LOGTAG     "[STCP/Torture/Main] "
#include <stcp_testing_bplate.h>

#include <stcp_testing_common.h>


K_THREAD_STACK_ARRAY_DEFINE(worker_stacks, STCP_TORTURE_WORKERS, STCP_TORTURE_STACK);
static struct k_thread worker_threads[STCP_TORTURE_WORKERS];

struct worker_ctx {
    int worker_id;
    int mode;
};

static struct worker_ctx workers[STCP_TORTURE_WORKERS];


static k_thread_entry_t get_worker_entry(void)
{
    switch (CONFIG_STCP_TESTING_MODE) {

    case 1:
        LINF("Starting TCP mode..");
        return tcp_torture_worker;

    case 2:
        LINF("Starting STCP mode..");
        return stcp_torture_worker;

    case 3:
        LINF("Starting MQTT mode..");
        return mqtt_torture_worker;

    default:
        LINF("Starting default mode..");
        return stcp_torture_worker;
    }
}

void stcp_torture_start(void)
{
    LINFBIG("Starting torture testing..");
    //stcp_api_wait_until_reached_lte_ready(NULL, -1);

    // initial all => does DNS if any ..
    // and not in thread context..
    stcp_testing_resolve_test_host_address();

    if (CONFIG_STCP_TESTING_MODE == 0) {
        LINF("Starting IDLE mode..");
        return;
    }

        
    k_thread_entry_t entry = get_worker_entry();
    LINF("Worker entry = %p", entry);
    LINF("Expected stcp_torture_worker = %p", stcp_torture_worker);

    for (int i = 0; i < STCP_TORTURE_WORKERS; i++) {

        LDBG("Initialising worker %d/%d", 
            i, STCP_TORTURE_WORKERS);

        workers[i].worker_id = i;

        k_thread_create(
            &worker_threads[i],
            worker_stacks[i],
            K_THREAD_STACK_SIZEOF(worker_stacks[i]),
            entry,
            &workers[i],
            NULL,
            NULL,
            STCP_TORTURE_PRIO,
            0,
            K_NO_WAIT
        );
        k_msleep(500);
    }
}
