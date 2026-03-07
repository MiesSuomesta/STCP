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

#include <stcp_api.h>
#include <stcp/fsm.h>
#include <stcp/debug.h>
#include <stcp/stcp_rust_exported_functions.h>

// LTE / BT kytkin ...
#define TEST_CONNECTON_TO_HOST 	STCP_CONNECT_TO_ADDRESS_HOSTNAME
#define TEST_CONNECTON_TO_PORT 	STCP_CONNECT_TO_ADDRESS_PORT
#define STCP_WAIT_IN_SECONDS    (3*60)

#define STCP_HEAP_DEBUG         0

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

void stcp_fsm_init_globals(void);
void dump_sim_status(void);

int stcp_library_init()
{

    int wait_timeout_sec = 3*60;
    int64_t startTime = k_uptime_get();
    stcp_platform_init_banner();

    dump_stack("boot");

    LDBG("Boot");

    int stcpAlive = stcp_rust_alive();

    LDBG("stcpAlive=0x%x", stcpAlive);

    // Init?
    stcp_module_rust_enter();

    LDBG("Initialising globals....");
    stcp_fsm_init_globals();

    LDBG("Initialising platform....");
    stcp_platform_init(NULL);

    LDBG("Initialising transport....");
    stcp_transport_init();

    LDBG("Waiting until PDN state reached....");
    int err = stcp_pdn_wait_until_active_or_secs_passed(wait_timeout_sec);
    if (err<0) {
        LDBG("Got nothing!");
        return err;
    }

#if 1
    LDBG("Waiting for network up ... ");
    err = stcp_transport_wait_for_network_up(wait_timeout_sec);
    int64_t endTime = k_uptime_get();
    int64_t spent = endTime - startTime;

    if (err < 0) {
        LWRNBIG("Network NOT ready within %d seconds!", wait_timeout_sec);
    } else {
        LDBGBIG("Network UP => STCP READY, Init done in %llu ms", spent);
    }
#endif

    dump_sim_status();
    return err;
    
}


