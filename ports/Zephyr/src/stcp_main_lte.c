#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/init.h>
#include <modem/lte_lc.h>

#include <zephyr/sys/heap_listener.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/kernel.h>

#include <zephyr/net/socket.h>
#include <zephyr/net/net_ip.h>
#include <errno.h>
#include <zephyr/sys/atomic.h>
#include <modem/nrf_modem_lib.h>

#include "stcp_struct.h"
#include "stcp_net.h"
#include "workers.h"
#include "utils.h"
#include "fsm.h"
#include "stcp_bridge.h"
#include "stcp_operations_zephyr.h"

#include "stcp_rust_exported_functions.h"

#include <stcp.h>

#define STCP_USE_LTE			1
#include "debug.h"

// LTE / BT kytkin ...
#define TEST_CONNECTON_TO_HOST 	"lja.fi"
#define TEST_CONNECTON_TO_PORT 	"7777"
#define STCP_WAIT_IN_SECONDS    180

#define STCP_HEAP_DEBUG         0

#include "stcp_transport.h"
#include "stcp_net.h"
#include "stcp_operations_zephyr.h"
#include "stcp_platform.h"
#include "stcp_transport.h"
#include "network_testing.h"

LOG_MODULE_REGISTER(stcp_lte_module, LOG_LEVEL_INF);

int  stcp_rust_alive(void);

static atomic_t KEEP_RUNNING = ATOMIC_INIT(1);
extern int g_sock;
extern struct k_heap _system_heap;


int stcp_lte_reset_everythign(struct stcp_ctx *ctx) {
    LDBG("Resetting platform ....");
    int rc = stcp_platform_soft_reset();
    if (rc<0) { return rc; }

    // TODO: RIKKOO!!
    // LDBG("Resetting transport (NOP)....");
    // rc = stcp_transport_soft_reset(ctx);
    // if (rc<0) { return rc; }
    
    if (stcp_is_context_valid(ctx) > 0) {
        LDBG("Resetting session ....");
        rust_session_reset_everything_now(ctx);
    } else {
        LDBG("Context not valid!");
    }

    return 0;
}

int lte_connect(void)
{
	return stcp_transport_connect();
}

static int callback_platform_ready() {
	LDBG("Platform ready!");
    stcp_module_rust_enter();
    LDBG("RUST init done..");
}

void stcp_shutdown(void)
{
	/* The HTTP transaction is done, take the network connection down */
	LDBG("Shutdown...");
}


static void dump_stack(const char *tag)
{
    size_t unused = 0;

    k_thread_stack_space_get(k_current_get(), &unused);
    LDBG("%s: main unused stack = %u bytes", tag, (unsigned)unused);

#if STCP_HEAP_DEBUG    
    struct sys_heap_runtime_stats stats;
    sys_heap_runtime_stats_get(&_system_heap.heap, &stats);

    printk("HEAP STATS:\n");
    printk("  total:     %u\n", stats.total_bytes);
    printk("  free:      %u\n", stats.free_bytes);
    printk("  used:      %u\n", stats.allocated);
#endif

}


struct stcp_fsm theFSM;
struct stcp_ctx *theStcpContext = NULL;

int main(void)
{

    stcp_platform_init_banner();

    dump_stack("boot");

    LDBG("Boot");

    int stcpAlive = stcp_rust_alive();

    LDBG("stcpAlive=0x%x", stcpAlive);

    stcp_platform_init(NULL);
    stcp_transport_init();

    stcp_fsm_init(&theFSM);
    LDBG("STCP state machine init...");

    stcp_fsm_start(&theFSM);
    LDBG("STCP state machine started...");

    int err = stcp_pdn_wait_until_active_or_secs_passed(3*60);

    if (err<0) {
        LDBG("Got nothing!");
        return 1;
    }

    int tries = 3*60;
    while(tries-- > 0) {
        err = stcp_network_test_network_avalability("lja.fi", "7777");
        if (err == 0) {
            stcp_fsm_reached_ip_network_up(&theFSM);
            break;
        }
        k_msleep(2500);
    }





    while (1) {
        LDBG("STCP Alive....");
        k_sleep(K_SECONDS(5));
    }
	LDBG("Connection lost, shutting down..");
	stcp_shutdown();
	LDBG("Shutdown..., BYE!");
}


