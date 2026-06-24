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
#include <stcp/stcp_transport.h>
#include <stcp/recovery.h>

void stcp_rust_session_rust_session_thread(void *a, void *b, void *c)
{
    // A => session
    // B => semaphore
    if (a) {
        LDBG("Heavy Cleanup: Session %p cleanup starts....", a);
        rust_session_destroy(a);
        LDBG("Heavy Cleanup: Session %p freed", a);
    }

    if (b) {
        struct k_sem *pSem = b;
        LDBG("Heavy Cleanup: Now it is ok to complete rest of cleanup");
        k_sem_give(pSem);
        LDBG("Heavy Cleanup: cleanup resumed.. exitting ...");
    }

    LDBG("Heavy Cleanup: Bye!");
}



K_THREAD_STACK_DEFINE(rust_heavy_cleanup_stack, 8192);
struct k_thread rust_heavy_cleanup_thread_data;

void stcp_rust_session_rust_session_thread_init(void *a, void *b, void *c) {
    k_thread_create(&rust_heavy_cleanup_thread_data,
        rust_heavy_cleanup_stack,
        K_THREAD_STACK_SIZEOF(rust_heavy_cleanup_stack),
        stcp_rust_session_rust_session_thread,
        a, b, c,
        5, 0, K_NO_WAIT);
}



