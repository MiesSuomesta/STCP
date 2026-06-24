#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
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
#include <stcp/low_level_pointer.h>

#include <stcp/stcp_rust_exported_functions.h>

#define STCP_USE_LTE			1
#include <stcp/debug.h>

// LTE / BT kytkin ...
#define STCP_WAIT_IN_SECONDS    180

#define STCP_HEAP_DEBUG         0

#include "stcp/stcp_transport.h"
#include "stcp/stcp_net.h"
#include "stcp/stcp_operations_zephyr.h"
#include "stcp/stcp_platform.h"
#include "stcp/stcp_transport.h"
#include "stcp/utils.h"
#include <stcp/dns.h>
#include <stcp/low_level_pointer.h>
#include <stcp/stcp_struct.h>
#include <zephyr/net/net_ip.h>

void sleep_ms_jitter(uint32_t base_ms, uint32_t jitter_ms)
{
    uint32_t rnd = sys_rand32_get();
    uint32_t jitter = rnd % jitter_ms;

    k_sleep(K_MSEC(base_ms + jitter));
}

int stcp_crypto_is_aes_bypass_enabled() {
#if CONFIG_STCP_AES_BYPASS
    return 1;
#else
    return 0;
#endif 
}


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
    struct zsock_addrinfo hints = { 0 };
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

struct zsock_addrinfo *resolve_to_connect(const char *host, const char *port)
{
    struct zsock_addrinfo *rv = NULL;
    int rc = stcp_util_hostname_resolver(host, port, &rv);
    return rv;
}


static int is_ip_address(const char *host)
{
    struct in_addr addr;
    return zsock_inet_pton(AF_INET, host, &addr) == 1;
}

int stcp_tcp_resolve_and_make_socket_ip(
    const char *host,
    const int port,
    int *pfd);

int stcp_tcp_resolve_and_make_socket_dns(
    const char *host,
    const int port,
    struct zsock_addrinfo **res,
    int* pfd);

int stcp_tcp_resolve_and_make_socket_ip(
    const char *host,
    const int port,
    int *pfd)
{
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);

    if (zsock_inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        LERR("[TCP] Invalid IP address");
        return -EINVAL;
    }

    int fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        LERR("[TCP] socket() failed errno=%d", errno);
        return -errno;
    }

    *pfd = fd;

    LDBG("[TCP] IP ready %s:%d fd=%d", host, port, fd);

    return 0;
}    

int stcp_tcp_resolve_and_make_socket_dns(
    const char *host,
    const int port,
    struct zsock_addrinfo **res,
    int *pfd)
{
    if (!pfd) {
        return -EINVAL;
    }

    *pfd = -1;

    struct zsock_addrinfo *ai = NULL;

    int rc = stcp_dns_resolve(host, port, &ai);
    if (rc < 0 || !ai) {
        LERR("[TCP] DNS resolve failed rc=%d", rc);
        return -EIO;
    }

    int fd = zsock_socket(
        ai->ai_family,
        ai->ai_socktype,
        ai->ai_protocol
    );

    if (fd < 0) {
        LERR("[TCP] socket() failed errno=%d", errno);
        return -errno;
    }

    if (res) {
        *res = ai;   // ⚠️ tämä on cache pointer, EI saa freeata
    }

    *pfd = fd;

    LDBG("[TCP] DNS ready %s:%d fd=%d ai=%p", host, port, fd, ai);

    return 0;
}

int stcp_tcp_resolve_and_make_socket(
    const char *host,
    const int port,
    struct zsock_addrinfo **res,
    int *pfd)
{
    if (!host || !pfd) {
        return -EINVAL;
    }

    *pfd = -1;
    if (res) {
        *res = NULL;
    }

    if (is_ip_address(host)) {
        LDBG("[TCP] Using IP path");
        return stcp_tcp_resolve_and_make_socket_ip(host, port, pfd);
    } else {
        LDBG("[TCP] Using DNS path");
        return stcp_tcp_resolve_and_make_socket_dns(host, port, res, pfd);
    }
}

int stcp_context_set_target(struct stcp_ctx *ctx, const char *pHost, const int pPort) {

    if (!ctx) {
        LWRN("No contest to set target to!");
        return -ENOBUFS;
    }

    memset(ctx->ctx_hostname, 0, sizeof(ctx->ctx_hostname));
    snprintk(ctx->ctx_hostname, sizeof(ctx->ctx_hostname), "%s", pHost);
    ctx->ctx_port = pPort;

    return 0;
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

    if (ctx->handshake_done) {
        LWRN("Context already done hanshake, returning EALREADY");
        return -EALREADY;
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

    rc = rust_session_handshake_lte(ctx->session, &ctx->ks);
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

int stcp_tcp_context_shake_hands(struct stcp_ctx *ctx, int timeout_ms)
{

    if (!stcp_pdn_is_active()) {
        LDBG("Tried to connect when PDN not active!");
        return -EAGAIN;
    }

    if (ctx->handshake_done) {
        LWRN("Context already done hanshake, returning EALREADY");
        return -EALREADY;
    }

    // Keep alive settingssi päälle
    int yes = 1;
    zsock_setsockopt(ctx->ks.fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));

    LDBG(".--<[ MODEM Handshake start ]>------------------------------------------------------>");
    LDBG("|");
    LDBG("| Starting handshake, ctx: %p, session: %p", ctx, ctx->session);
    LDBG("|");
    LDBG("'--------------------------------------------->");

    int rc = rust_session_handshake_lte(ctx->session, &ctx->ks);
    if (rc < 0) {
        LERR("MODEM: Handshake failed: rc=%d errno=%d", rc, errno);
        return rc;
    }
    
    // Set handshake done
    ctx->handshake_done = (rc == 1);
    LDBGBIG("STCP: Allowing IO operations from now on? %s",
        GET_YES_NO_STR(ctx->handshake_done)
    );
    atomic_set(&ctx->allow_api_access, ctx->handshake_done);

    if (ctx->handshake_done) {
        LDBGBIG("[Handshake] Completed, setting socket as non bloking");
        stcp_set_non_bloking_to(ctx, 1);
    }

    LDBG(".--<[ MODEM Handshake status ]>------------------------------------------------------>");
    LDBG("|");
    LDBG("| Handshake done? %d ctx: %p, session: %p", ctx->handshake_done, ctx, ctx->session);
    LDBG("|");
    LDBG("'--------------------------------------------->");
 
    return rc;
}

