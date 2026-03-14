/*
 * STCP socket dispatcher for Zephyr / NCS 2.9 (Zephyr 3.7)
 *
 * Transparent protocol hook:
 *   zsock_socket(..., IPPROTO_STCP) -> stcp_socket()
 *
 * STCP rides on TCP underneath (for now)
 */

#include <zephyr/logging/log.h>

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

    LDBG("Starting handshake for LTE %d / %p", ctx->ks.fd, ctx);

    LDBG("======================================================================================");
    LDBG("=== HS Start for LTE =================================================================");
    LDBG("======================================================================================");
	LDBG("Context at %s: %p // HS Done: %d // FD: %d",
        __func__, ctx, ctx->handshake_done, ctx->ks.fd
    );

	int rc = rust_session_handshake_lte(ctx->session, &(ctx->ks));
	
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
        LWRN("close skipped, fd already invalid");
        return 0;
    }

    int old = *fd;
    *fd = -1; 

	int linger = 0;
	LDBG("Setting linger OFF for fd: %d", old);
	zsock_setsockopt(old, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
	
	int rc = zsock_close(old);
    LINF("close(fd=%d) rc=%d but returning 0", old, rc);

    return 0; // Oli rc
}

int stcp_ctx_ref_count_get(struct stcp_ctx *ctx)
{
	if (!ctx) {
		return;
	}

	return (int)atomic_inc(&ctx->refcnt);
}

int stcp_ctx_ref_count_is_what(struct stcp_ctx *ctx)
{
	if (!ctx) {
		return;
	}
	return (int)atomic_get(&ctx->refcnt);
}

int stcp_ctx_ref_count_put(struct stcp_ctx *ctx)
{
	if (!ctx) {
		return;
	}

    if ( atomic_dec(&ctx->refcnt) == 1 ) {
		LDBG("Last reference of ctx: %p", ctx);
		return 1;
	}
	return 0;
}

void stcp_create_init_new_context(struct stcp_ctx *ctx) {
	if (!ctx) {
		LERR("No context");
		return;
 	}

	ctx->magic = STCP_CTX_MAGIC_ALIVE;
	ctx->ks.kctx = ctx;
	ctx->handshake_done = 0;
	ctx->poll_timeouts = 0;
	atomic_set(&ctx->closing, 0);
	atomic_set(&ctx->refcnt, 1);
	atomic_set(&ctx->connection_closed, 0);
	atomic_set(&ctx->destroyed, 0);
	k_mutex_init(&ctx->lock);
	worker_context_init(ctx);
	stcp_context_recv_stream_init(ctx);
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
	stcp_create_init_new_context(ctx);

	LDBG("STCP: Settings via fd: %d / %p (KS.FD:%d)", under, ctx, ctx->ks.fd);
	LDBG("Creating STCP session for %p", ctx);
	int ret = rust_exported_session_create(&ctx->session, &ctx->ks);
	LDBGBIG("CREATION: Created STCP session for %p: %p", ctx, ctx->session);

	if (ret < 0) {
		LDBG("Error while creating STCP session: %d", ret);
		errno = ENOMEM;
		return NULL;
	}

	// Last: init RX buffer.....
	stcp_context_recv_stream_init(ctx);
	return ctx;
}
