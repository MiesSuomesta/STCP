#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <zephyr/init.h>
#include <zephyr/sys/heap_listener.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/sys/atomic.h>

#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>


#include <stcp_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_net.h>
#include <stcp/workers.h>
#include <stcp/utils.h>
#include <stcp/stcp_mqtt.h>
#include <stcp/utils.h>
#include <stcp/stcp_operations_zephyr.h>

#include <stcp/stcp_rust_exported_functions.h>

#define STCP_USE_LTE			1
#include <stcp/debug.h>

// LTE / BT kytkin ...
#define TEST_CONNECTON_TO_HOST 	"lja.fi"
#define TEST_CONNECTON_TO_PORT 	"7777"
#define STCP_WAIT_IN_SECONDS    180

#define STCP_HEAP_DEBUG         0

#include "stcp/stcp_transport.h"
#include "stcp/stcp_net.h"
#include "stcp/stcp_operations_zephyr.h"
#include "stcp/stcp_platform.h"
#include "stcp/stcp_transport.h"
#include "stcp/utils.h"


#include <zephyr/net/net_ip.h>

void dump_socket_error(int fd)
{
    int err = 0;
    socklen_t len = sizeof(err);

    int rc = zsock_getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);

    if (rc < 0) {
        LERR("getsockopt(SO_ERROR) failed errno=%d", errno);
        return;
    }

    if (err == 0) {
        LINF("SO_ERROR: no pending error");
    } else {
        LERR("SO_ERROR: %d (%s)", err, strerror(err));
    }
}

void stcp_util_log_sockaddr(char *tag, const struct zsock_addrinfo *ai)
{

    if (!ai) {
        LINF("addrinfo: NULL");
        return;
    }

    LINF("=== zsock_addrinfo // %s ===", tag);
    LINF("family   : %d", ai->ai_family);
    LINF("socktype : %d", ai->ai_socktype);
    LINF("protocol : %d", ai->ai_protocol);
    LINF("addrlen  : %d", ai->ai_addrlen);

    if (!ai->ai_addr) {
        LINF("ai_addr is NULL");
        return;
    }

    if (ai->ai_family == AF_INET) {

        struct sockaddr_in *addr4 = (struct sockaddr_in *)ai->ai_addr;

        char ipbuf[NET_IPV4_ADDR_LEN];
        zsock_inet_ntop(AF_INET, &addr4->sin_addr, ipbuf, sizeof(ipbuf));

        uint16_t port = ntohs(addr4->sin_port);

        LINF("IPv4     : %s", ipbuf);
        LINF("Port     : %u", port);

    } else if (ai->ai_family == AF_INET6) {

        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)ai->ai_addr;

        char ipbuf[NET_IPV6_ADDR_LEN];
        zsock_inet_ntop(AF_INET6, &addr6->sin6_addr, ipbuf, sizeof(ipbuf));

        uint16_t port = ntohs(addr6->sin6_port);

        LINF("IPv6     : %s", ipbuf);
        LINF("Port     : %u", port);

    } else {
        LINF("Unknown address family");
    }

    LINF("======================");
}

int stcp_util_hostname_resolver(const char *host, const char *port, struct zsock_addrinfo **result) {
    struct addrinfo hints = { 0 };
    struct zsock_addrinfo *res;
    int err;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    LDBG("DNS lookup: %s:%s", host, port);

    *result = NULL;

    err = zsock_getaddrinfo(host, port, &hints, &res);
    if (err < 0) {
        LERR("getaddrinfo failed: %d", err);
        return -ENOBUFS;
    }

    LDBG("Setting results...");
    *result = res;

    return 0;
}

struct addrinfo *resolve_to_connect(const char *host, const char *port) {
    struct addrinfo hints = { 0 };
    struct addrinfo *res;
    int err;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    LDBG("DNS lookup: %s:%s", host, port);
    err = zsock_getaddrinfo(host, port, &hints, &res);
    if (err < 0) {
        LERR("getaddrinfo failed: %d", err);
        return NULL;
    }
    return res;
}

int stcp_tcp_resolve_and_make_socket(const char *host, const char *port) {
    struct addrinfo *res = resolve_to_connect(host, port);
    LDBG("STCP: Creating socket...");
	int fd = zsock_socket(res->ai_family, res->ai_socktype, IPPROTO_TCP);
    zsock_freeaddrinfo(res);
    if (fd < 0) {
        LERR("TCP socket rc: %d", fd);
        LERR("TCP socket failed: %d", errno);
        return -errno;
    }
    return fd;
}

struct stcp_ctx *stcp_tcp_resolve_and_make_context(const char *host, const char *port) {

    int fd = stcp_tcp_resolve_and_make_socket(host, port);
    if (fd >= 0) {
        struct stcp_ctx *ctx =  stcp_create_new_context(fd);

        // TODO: TEhdä nämä taulukoiksi => EI MALLOC
        memset(ctx->hostname_str, 0, sizeof(ctx->hostname_str));
        memset(ctx->port_str, 0, sizeof(ctx->port_str));
        
        snprintf(
            ctx->hostname_str, 
            sizeof(ctx->hostname_str) - 1,
            "%s",
            host
        );

        snprintf(
            ctx->port_str, 
            sizeof(ctx->port_str) - 1,
            "%s",
            port
        );

        ctx->ks.resolved_host = resolve_to_connect(host, port);

        return ctx;
    }
    return NULL;
}

int stcp_tcp_connect_shake_hands_with_addr(
        struct stcp_ctx *ctx,
        const struct sockaddr *addr,
        socklen_t addrlen,
        int timeout_ms)
{

    if (!stcp_pdn_is_active()) {
        LDBG("Tried to do hanshake when PDN not active!");
        return -EAGAIN;
    }

    int rc = 0;

    /* Aseta timeout connectille (Zephyr käyttää SO_SNDTIMEO connectissä) */
    struct timeval tv = {
        .tv_sec = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000,
    };
    zsock_setsockopt(ctx->ks.fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    zsock_setsockopt(ctx->ks.fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    LDBG("Doing connect call with FD: %d...", ctx->ks.fd);
    rc = zsock_connect(ctx->ks.fd, addr, addrlen);
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

int stcp_mqtt_connect_via_stcp(int fd) {

    int rc = mqtt_connect_via_stcp(fd);
    if (rc < 0) {
        LERR("MQTT Connection failed, rc: %d errno=%d", rc, errno);
        return rc;
    }

    LDBG(".----------------------------------------------------------------------->");
    LDBG("|");
    LDBG("| MQTT state: CONNECTED via STCP, fd: %d", fd);
    LDBG("|");
    LDBG("'--------------------------------------------->");

    return rc;
}

int stcp_config_debug_enabled() {
    return IS_ENABLED(CONFIG_STCP_DEBUG);
}

int stcp_config_aes_bypass_enabled() {
    return IS_ENABLED(CONFIG_STCP_AES_BYPASS);
}

int stcp_is_file_desc_alive(int fd) {
    uint8_t tmp;
    int ret = zsock_recv(fd, &tmp, 1, ZSOCK_MSG_PEEK | ZSOCK_MSG_DONTWAIT);

    if (ret == 0) {
        return 0;
    } else if (ret < 0 && errno == EAGAIN) {
        return 1;
    }
    return -errno;
}

int stcp_get_pending_fd_error(int fd)
{
    
    if (fd < 0 ) {
        LDBG("STCP Context is destroyed!\n");
        return -EBADFD;
    }

    int err = 0;
    socklen_t len = sizeof(err);

    zsock_getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);


    return err;
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
    zsock_setsockopt(ctx->ks.fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    zsock_setsockopt(ctx->ks.fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct addrinfo *res = 
        resolve_to_connect(ctx->hostname_str, ctx->port_str);

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

    // Set handshake done
    ctx->handshake_done = 1;

    LDBG(".----------------------------------------------------------------------->");
    LDBG("|");
    LDBG("| STCP state: STCP CONNECTED, ctx: %p, session: %p", ctx, ctx->session);
    LDBG("|");
    LDBG("'--------------------------------------------->");

    return 1;
}

