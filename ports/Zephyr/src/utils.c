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
#include "utils.h"

LOG_MODULE_REGISTER(stcp_utils, LOG_LEVEL_INF);

struct addrinfo *resolve_to_connect(const char *host, const char *port) {
    struct addrinfo hints = { 0 };
    struct addrinfo *res;
    int fd, err;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    LDBG("DNS lookup: %s:%s", host, port);
    err = zsock_getaddrinfo(host, port, &hints, &res);
    if (err < 0) {
        LOG_ERR("getaddrinfo failed: %d", err);
        return NULL;
    }
    return res;
}

int stcp_tcp_resolve_and_make_socket(const char *host, const char *port) {
    struct addrinfo *res = resolve_to_connect(host, port);
    LDBG("STCP: Creating socket...");
	int fd = stcp_socket(res->ai_family, res->ai_socktype, IPPROTO_TCP);
    zsock_freeaddrinfo(res);
    if (fd < 0) {
        LOG_ERR("TCP socket rc: %d", fd);
        LOG_ERR("TCP socket failed: %d", errno);
        return -errno;
    }
}

struct stcp_ctx *stcp_tcp_resolve_and_make_context(const char *host, const char *port) {

    int fd = stcp_tcp_resolve_and_make_socket(host, port);
    if (fd >= 0) {
        struct stcp_ctx *ctx =  stcp_create_new_context(fd);
        ctx->ks.resolved_host = resolve_to_connect("lja.fi", "7777");
        return ctx;
    }
    return NULL;
}

int stcp_tcp_context_connect_and_shake_hands(struct stcp_ctx *ctx, int timeout_ms)
{

    if (!stcp_pdn_is_active()) {
        LDBG("Tried to connect when PDN not active!");
        return -EAGAIN;
    }

    int rc = 0;

    /* Aseta timeout connectille (Zephyr käyttää SO_SNDTIMEO connectissä) */
    struct timeval tv = {
        .tv_sec = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000,
    };
    zsock_setsockopt(ctx->tcp_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    zsock_setsockopt(ctx->tcp_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct addrinfo *res = ctx->ks.resolved_host;

    LDBG("Doing connect call with FD: %d...", ctx->ks.fd);
    rc = zsock_connect(ctx->ks.fd, res->ai_addr, res->ai_addrlen);
    LDBG("Did connect call, rc: %d", rc);

    if (rc < 0) {
        LERR("connect failed: errno=%d", errno);
        return -errno;
    }

    LDBG(".----------------------------------------------------------------------->");
    LDBG("|");
    LDBG("| STCP state: TCP CONNECTED, ctx: %p, session: %p", ctx, ctx->session);
    LDBG("|");
    LDBG("'--------------------------------------------->");

    // Keep alive settingssi päälle
    int yes = 1;
    zsock_setsockopt(ctx->ks.fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));

    LDBG(".----------------------------------------------------------------------->");
    LDBG("|");
    LDBG("| STCP state: STCP HANDSHAKE, ctx: %p, session: %p", ctx, ctx->session);
    LDBG("|");
    LDBG("'--------------------------------------------->");

    rc = rust_session_client_handshake_lte(ctx->session, &ctx->ks);
    if (rc < 0) {
        LERR("Handshake failed: errno=%d", errno);
        return -errno;
    }


    LDBG(".----------------------------------------------------------------------->");
    LDBG("|");
    LDBG("| STCP state: STCP CONNECTED, ctx: %p, session: %p", ctx, ctx->session);
    LDBG("|");
    LDBG("'--------------------------------------------->");
    return 1;
}

#if 0
static int test_tcp(const char *host, const char *port, struct stcp_ctx **theContext)
{
    int fd, err;

    fd = stcp_tcp_resolve_and_make_socket(host, port);

    LDBG("STCP: Creating socket...");

    struct stcp_ctx *ctx = stcp_create_new_context(fd);
    *theContext = ctx;
    
    if (!ctx) {
        LDBG("No context?");
        return -ENOMEM;
    }

    LDBG("STCP: Connecting via fd: %d / %p(TCP:%d, KS:%d)", fd, ctx, ctx->tcp_fd, ctx->ks.fd);
    errno = 0;

    LDBG("closing flag before TCP connect: %d", atomic_get(&ctx->closing));
    err = stcp_context_connect(ctx, (struct sockaddr*)&ss, res->ai_addrlen, 180000);
    LDBG("closing flag after TCP connect: %d", atomic_get(&ctx->closing));
    zsock_freeaddrinfo(res);

    if (err >= 0) {
        LDBG(".-------------------------------------------------------->");
        LDBG("| STCP state: CONNECTED");
        LDBG("'--------------------------->");
        ctx->handshake_done = 1;
    } else {
        LDBG("Connection failed?, rc & errno: %d, %d", err, errno);
        return -errno;
    }

    LDBG("TCP: CONNECT OK");
    return 0;
}
#endif
