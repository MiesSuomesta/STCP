/* stcp_lte_fsm.c */

#include <zephyr/logging/log.h>

#include "debug.h"
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>

#include <errno.h>

#include "debug.h"
#include "stcp_alloc.h"
#include "stcp_struct.h"
#include "stcp_bridge.h"
#include "stcp_net.h"
#include "workers.h"
#include "stcp_operations_zephyr.h"

#include "stcp_rust_exported_functions.h"
LOG_MODULE_REGISTER(stcp_fsm_func, LOG_LEVEL_INF);

int stcp_run_loop(struct stcp_ctx *theCtx) {
    char buf[512] = { 0 };
    uint32_t ts = k_uptime_get_32();

    int len = snprintf(buf, sizeof(buf), "STCP-DBG ts=%u ms PLAINTEXT HERE....JEAH!", ts);
    LDBG("[LOOP] Sending: %s", buf);
    int err = stcp_transport_send(theCtx, buf, len);
    if (err < 0) {
        LDBG("SEND error: %d", err);
        if (err == -128) {
            LDBG("Connection died!");
            stcp_on_disconnect(theCtx);
            return -EAGAIN;
        }
        return err;
    }

    char bufin[512] = { 0 };
    err = stcp_transport_recv(theCtx, bufin, sizeof(bufin) - 1);
    if (err < 0) {
        LDBG("RECV error: %d", err);
        if (err == -128) {
            LERR("Connection died!");
            stcp_on_disconnect(theCtx);
            return -EAGAIN;
        }
        return err;
    } else {
        LDBG("No error, breaking out..");
    }

    return 0;
}


