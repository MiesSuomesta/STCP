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

#include "debug.h"
#include "utils.h"
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

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

int stcp_close(void *obj)
{
    if (obj) {
        struct stcp_ctx *ctx = obj;
// pois         int rc = zsock_close(ctx->ks.fd);
        return 0;
    }
    return 0;
}

/* -------------------------------------------------------------------------- */
/* Missing from socket API: Connect call                                      */
/* -------------------------------------------------------------------------- */
int stcp_handshake_for_context(struct stcp_ctx *ctx)
{
    if (stcp_is_context_valid(ctx) < 0) {
        return -ENOTCONN;
    }

    LDBG("Starting handshake for LTE %d / %p", ctx->ks.fd, ctx);
    
    // Ensure FD IS right one
    ctx->ks.fd = ctx->ks.fd;

    LDBG("======================================================================================");
    LDBG("=== HS Start for LTE =================================================================");
    LDBG("======================================================================================");
	int rc = rust_session_client_handshake_lte(ctx->session, &(ctx->ks));
	
	if (rc < 0) {
		rust_session_reset_everything_now(ctx->session);
	} 

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

// pois     int rc = zsock_close(old);
    LOG_INF("close(fd=%d) rc=%d", old, 0);

    return 0; // Oli rc
}

#if 0
    LDBG("Starting STCP handshake...");
    rc = 0;
    int state = 0;
    int failed = 0;
    ctx->handshake_done = 0;
    while ((!failed) && (!ctx->handshake_done)) {
    	LDBG("Handshake state %d start .....", state);
        rc = rust_session_client_handshake_lte(ctx->session, &ctx->ks);
        switch (rc) {
            case 1:
                LERR("CLIENT: Handshake completed at state: %d ==========================", state);
                LERR("CLIENT: Handshake completed at state: %d ==========================", state);
                LERR("CLIENT: Handshake completed at state: %d ==========================", state);
                LERR("CLIENT: Handshake completed at state: %d ==========================", state);
                ctx->handshake_done = 1;
                break;

            default:
                LERR("Handshake failed at state: %d, RC: %d", state, rc);
                failed = 1;
                break;
        }

        if (! ctx->handshake_done ) {
            LDBG("Sleeping %d secs", secondsWait);
            k_msleep(secondsWait*1000);
        }

        LDBG("Handshake state %d end: %d / %d .....", state, rc, ctx->handshake_done);
    }
    
    LDBG("STCP Handshake done? %d", ctx->handshake_done);
    if (ctx->handshake_done) {
        if (stcp_is_context_valid(ctx) == 1) {
            LDBG("==========================================================");
            LDBG("================ STCP handshake COMPLETED ================");
            LDBG("==========================================================");
            return 0;
        } else {
            LERR("BUG: Context not valid after handshake!");
        }
    } else {
        LERR("Handshake incomplete!");
    }
    return -EAGAIN;
}
#endif

/* -------------------------------------------------------------------------- */
/* Socket API                                                                 */
/* -------------------------------------------------------------------------- */

bool stcp_is_supported(int family, int type, int proto)
{
	if (proto != IPPROTO_STCP) {
		return false;
	}

	if (type != SOCK_STREAM) {
		return false;
	}

	return (family == AF_INET || family == AF_INET6 || family == AF_UNSPEC);
}

struct stcp_ctx *stcp_create_new_context(int under) {

	struct stcp_ctx *ctx = stcp_alloc(sizeof(*ctx));

	LDBG("Created CTX: %p / %d", ctx, under);

	ctx->ks.fd = under;
	ctx->ks.kctx = ctx;

	k_mutex_init(&ctx->lock);
	worker_context_init(ctx);

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
	return ctx;
}

int stcp_socket(int family, int type, int proto)
{
	ARG_UNUSED(proto);
	LDBG("Creating STCP socket....");
	int under = zsock_socket(family, type, IPPROTO_TCP);
	LOG_INF("STCP socket created fd=%d", under);
	return under;
}

/* -------------------------------------------------------------------------- */
/* Registration                                                               */
/* -------------------------------------------------------------------------- */

NET_SOCKET_REGISTER(stcp, 40, AF_UNSPEC, stcp_is_supported, stcp_socket);

