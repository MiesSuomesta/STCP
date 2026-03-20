#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/init.h>
#include <modem/lte_lc.h>
#include <modem/at_cmd_custom.h>
#include <modem/nrf_modem_lib.h>

#include <zephyr/sys/heap_listener.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/kernel.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/sys/atomic.h>

#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/fsm.h>
#include <stcp/debug.h>
#include <stcp/stcp_rust_exported_functions.h>
#include <stcp/stcp_rx_transmission.h>
#include <stcp/stcp_soft_reset.h>
#include <stcp/stcp_rx_transmission.h>
#include <stcp/stcp_transport_api.h>
#include <stcp/stcp_transport.h>
#include <stcp/stcp_platform.h>

#include <testing/include/status_monitor.h>
#include <testing/include/stcp_testing.h>


#define STCP_HEAP_DEBUG         0

int  stcp_rust_alive(void);

#if STCP_HEAP_DEBUG
extern struct k_heap _system_heap;
#endif

extern struct k_sem g_sem_lte_ready;
extern struct k_sem g_sem_pdn_ready;
extern struct k_sem g_sem_ip_ready;

int stcp_lte_reset_everythign(struct stcp_ctx *ctx) {
    
    LDBG("Resetting platform called..");

    LDBG("Resetting semaphores LTE/PDN/IP...");

    if (stcp_is_context_valid(ctx) > 0) {
        LDBG("Resetting platform ....");
        stcp_full_soft_reset(ctx);
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


void stcp_shutdown(void)
{
	/* The HTTP transaction is done, take the network connection down */
	LDBG("Shutdown...");
}

void stcp_fsm_init_globals(void);
void dump_sim_status(void);
void stcp_platform_init_banner(void);

int stcp_lte_do_full_reset(struct stcp_ctx *ctx, int wait)
{
    int64_t startTime = k_uptime_get();
    

    if (ctx) {

        LDBG("Guards up...");
        atomic_set(&ctx->connection_closed, 1);
        ctx->handshake_done = 0;

        LDBG("Connection RESET => closing transport...");
        stcp_transport_close(ctx);

        LDBG("Re-Init context..");
        stcp_create_init_new_context(ctx);

        LDBG("Doing soft reset of transport.");
        stcp_transport_soft_reset(ctx);
    }

    LDBG("Initialising globals....");
    stcp_fsm_init_globals();

    LDBG("Resetting semaphores LTE/PDN/IP...");

    LDBG("Initialising platform....");
    stcp_platform_init(NULL);

    LDBG("Initialising transport....");
    stcp_transport_init();

    int64_t endTime = k_uptime_get();
    int64_t spent = endTime - startTime;

    LDBGBIG("Full reset to wait for READY, done in %llu ms", spent);

    if (wait > 0)
    {
        LDBG("Waiting until PDN state reached....");
        int err = stcp_pdn_wait_until_active_or_secs_passed(wait);
        if (err<0) {
            LWRN("PDN not activated within %d seconds", wait);
            return err;
        } else {
            LINF("PDN Activated!");
        }

        LDBG("Waiting for network up ... ");
        err = stcp_transport_wait_for_network_up(wait);
        if (err<0) {
            LWRN("Network not up within %d seconds", wait);
            return err;
        } else {
            LINF("IP Network is UP!");
        }

    }

    endTime = k_uptime_get();
    spent = endTime - startTime;

    LDBGBIG("Full reset to STCP READY, done in %llu ms", spent);

    return 0;
}


int stcp_library_init()
{

    int wait_timeout_sec = 3*60;
    int64_t startTime = k_uptime_get();
    stcp_platform_init_banner();

    LDBG("Boot");

    int stcpAlive = stcp_rust_alive();

    LDBG("stcpAlive=0x%x", stcpAlive);

    // Init?
    stcp_module_rust_enter();

    stcp_status_monitor_start();

    LDBG("Doing reset....");
    stcp_lte_do_full_reset(NULL, wait_timeout_sec);

    
    int64_t endTime = k_uptime_get();
    int64_t spent = endTime - startTime;

    LDBGBIG("INIT => STCP READY, done in %llu ms", spent);

    dump_sim_status();

#if CONFIG_STCP_TESTING
    LDBGBIG("Torture server %s:%d. STARTING Torture, mode %d!", 
        CONFIG_STCP_TESTING_PEER_HOSTNAME_TO_CONNECT, 
        CONFIG_STCP_TESTING_PEER_PORT_TO_CONNECT,
        CONFIG_STCP_TESTING_MODE
    );

    stcp_torture_start();
    LDBG("Torture started...");
#endif

    return 0;
    
}


