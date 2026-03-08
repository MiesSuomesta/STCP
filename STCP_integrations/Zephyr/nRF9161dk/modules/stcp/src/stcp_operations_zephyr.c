/*
 * STCP socket dispatcher for Zephyr / NCS 2.9 (Zephyr 3.7)
 *
 * Transparent protocol hook:
 *   zsock_socket(..., IPPROTO_STCP) -> stcp_socket()
 *
 * STCP rides on TCP underneath (for now)
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(stcp_operations, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>

#include <errno.h>

#include <stcp/debug.h>
#include <stcp/stcp_struct.h>
#include <stcp/fsm.h>
#include <stcp/utils.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/stcp_rust_exported_functions.h>
#include <stcp/stcp_alloc.h>

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

/* -------------------------------------------------------------------------- */
/* Missing from socket API: Connect call                                      */
/* -------------------------------------------------------------------------- */
int stcp_handshake_for_context(struct stcp_ctx *ctx)
{
    if (stcp_is_context_valid(ctx) < 0) {
        return -ENOTCONN;
    }

    LDBG("Starting handshake for LTE %d / %p", ctx->ks.fd, ctx);

    LDBG("======================================================================================");
    LDBG("=== HS Start for LTE =================================================================");
    LDBG("======================================================================================");
	LDBG("Context at %s: %p // HS Done: %d // FD: %d",
        __func__, ctx, ctx->handshake_done, ctx->ks.fd
    );

	int rc = rust_session_client_handshake_lte(ctx->session, &(ctx->ks));
	
	if (rc < 0) {
		rust_session_reset_everything_now(ctx->session);
	} 

	ctx->handshake_done = rc == 1;
	
	LDBG("Context at %s: %p // HS Done: %d // FD: %d",
        __func__, ctx, ctx->handshake_done, ctx->ks.fd
    );

	LDBG("======================================================================================");
	LDBG("=== HS END for LTE %d / %p DONE, RC: %d / %d", ctx->ks.fd, ctx, rc, errno);
	LDBG("======================================================================================");
	return rc;
}

int stcp_net_close_fd(int *fd)
{
    if (*fd < 0) {
        LOG_WRN("close skipped, fd already invalid");
        return 0;
    }

    int old = *fd;
    *fd = -1; 

	int rc = zsock_close(old);
    LOG_INF("close(fd=%d) rc=%d but returning 0", old, rc);

    return 0; // Oli rc
}

struct stcp_ctx *stcp_create_new_context(int under) {

	struct stcp_ctx *ctx = stcp_alloc(sizeof(struct stcp_ctx));

	if (!ctx) {
		LERRBIG("FATAL: OOM!");
		while (1) {
			k_msleep(5000);
		}
 	}

	LDBG("Created CTX: %p / %d", ctx, under);

	ctx->ks.fd = under;
	ctx->ks.kctx = ctx;
	k_mutex_init(&ctx->lock);
//	worker_context_init(ctx);

	ctx->handshake_done = 0;

	LDBG("STCP: Settings via fd: %d / %p (KS.FD:%d)", under, ctx, ctx->ks.fd);
	LDBG("Creating STCP session for %p", ctx);
	int ret = rust_exported_session_create(&ctx->session, &ctx->ks);

	LDBG("Created STCP session for %p: %p", 
		ctx, ctx->session);

	if (ret < 0) {
		LDBG("Error while creating STCP session: %d", ret);
		errno = ENOMEM;
		return NULL;
	}

	// Last: init RX buffer.....
	stcp_context_recv_stream_init(ctx);
	return ctx;
}
