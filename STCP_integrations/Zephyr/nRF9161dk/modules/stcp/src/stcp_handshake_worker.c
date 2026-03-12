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


K_THREAD_STACK_DEFINE(stcp_handshake_worker_stack, 2048);
static struct k_thread stcp_handshake_worker_thread;

static void stcp_handshake_worker(void *a, void *b, void *c)
{
    struct stcp_ctx *ctx = a;
    LDBG("HST: Starting for %p", ctx);
    while (1) {

        if (!ctx)
            continue;


        if (!ctx->handshake_done) {

            int ret = rust_session_server_handshake_lte(
                ctx->session,
                &ctx->ks);

            if (ret == 1) {

                LINF("STCP handshake done");

                ctx->handshake_done = true;

                k_poll_signal_raise(&ctx->handshake_signal, 1);
            }
        }

        k_sleep(K_MSEC(5));
    }

    LDBG("HST: Done for %p", ctx);
}

int stcp_hanshake_worker_start_for_context(struct stcp_ctx *ctx) {

    if (!ctx) {
        LERR("No context for HS thread!");
        return -EBADFD;
    }

    k_thread_create(&stcp_handshake_worker_thread,
                    stcp_handshake_worker_stack,
                    K_THREAD_STACK_SIZEOF(stcp_handshake_worker_stack),
                    stcp_handshake_worker,
                    ctx, NULL, NULL,
                    5, 0, K_NO_WAIT);

    LDBGBIG("Started hanshake for context %p", ctx);

    return 0;
}