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
		return -EINVAL;
	}
    int old = atomic_inc(&ctx->refcnt);
    LDBG("REF++ %p => %d\n", ctx, old + 1);
    return old + 1;
}

int stcp_ctx_ref_count_is_what(struct stcp_ctx *ctx)
{
	if (!ctx) {
		return -EINVAL;
	}
	return (int)atomic_get(&ctx->refcnt);
}

int stcp_ctx_ref_count_put(struct stcp_ctx *ctx)
{
	if (!ctx) {
		return -EINVAL;
	}

    int old = atomic_dec(&ctx->refcnt);
    LDBG("REF-- %p => %d\n", ctx, old - 1);

    if (old == 1) {
        LWRNBIG("💣 FREE TRIGGER %p\n", ctx);
		stcp_dump_bt();
    }

    return old - 1;
}

int stcp_wait_for_handshake_signal(struct stcp_ctx *ctx) {

	if (!ctx) {
		return -EBADFD;
	}

	struct k_poll_event events[1];

    k_poll_event_init(&events[0],
        K_POLL_TYPE_SIGNAL,
        K_POLL_MODE_NOTIFY_ONLY,
        &ctx->handshake_signal);

    LINF("[CTX %p] Waiting for hanshake signal..", ctx);
    int ret = k_poll(events, 1, K_FOREVER);
    LINF("[CTX %p] Got hanshake signal, ret: %d", ctx, ret);
	k_poll_signal_reset(&ctx->handshake_signal);
	return ret;
}

void stcp_create_init_new_context(struct stcp_ctx *ctx) {
	if (!ctx) {
		LERR("No context");
		return;
 	}

	memset(ctx, 0, sizeof(*ctx));


	ctx->magic = STCP_CTX_MAGIC_ALIVE;
	ctx->ks.kctx = ctx;
	ctx->handshake_done = 0;
	ctx->poll_timeouts = 0;
	ctx->state = STCP_STATE_INIT;

	atomic_set(&ctx->closing, 0);
	atomic_set(&ctx->refcnt, 1);
	atomic_set(&ctx->connection_closed, 0);
	atomic_set(&ctx->destroyed, 0);

	k_poll_signal_init(&(ctx->handshake_signal));

	stcp_fsm_init(&(ctx->fsm), ctx);

#if CONFIG_STCP_TESTING
	char *pHostName = CONFIG_STCP_TESTING_PEER_HOSTNAME_TO_CONNECT;
	char *pHostPort = CONFIG_STCP_TESTING_PEER_PORT_TO_CONNECT;
#else
	char *pHostName = CONFIG_STCP_CONNECT_TO_HOST;
	char *pHostPort = CONFIG_STCP_CONNECT_TO_PORT;
#endif

	LINF("Set context %p target %s:%s", ctx, pHostName, pHostPort);
	stcp_context_set_target(ctx, pHostName, pHostPort);

	k_mutex_init(&ctx->lock);
	LDBG("At init:");	
	LDBG("CTX=%p\n", ctx);
	LDBG("MAGIC=%x\n", ctx->magic);
	stcp_context_recv_stream_init(ctx);
}


struct stcp_ctx *stcp_create_new_context(int under) {

	struct stcp_ctx *ctx = stcp_alloc(sizeof(struct stcp_ctx));
	LDBG("CTX ALLOC %p size=%d\n", ctx, sizeof(struct stcp_ctx));

	if (!ctx) {
		LERRBIG("FATAL: OOM!");
		while (1) {
			k_msleep(5000);
		}
 	}

	LDBG("Created CTX: %p / %d", ctx, under);
	stcp_create_init_new_context(ctx);
	ctx->ks.fd = under;

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
