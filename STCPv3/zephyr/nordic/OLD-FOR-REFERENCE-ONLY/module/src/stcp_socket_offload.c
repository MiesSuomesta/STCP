#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/posix/fcntl.h>
#include <zephyr/sys/fdtable.h>
#include <stcp/stcp_internal.h>
#ifdef CONFIG_STCP_RUST_CORE
#include <stcp/stcp_rust_ffi.h>
#endif

LOG_MODULE_REGISTER(stcp_offload, CONFIG_STCP_LOG_LEVEL);

#ifdef CONFIG_STCP_RUST_CORE
static int rust_errno(int rc)
{
    if (rc >= 0) return rc;
    errno = -rc;
    return -1;
}

static int wait_rust_ready(struct stcp_ctx *ctx, int timeout_ms)
{
    int64_t deadline = k_uptime_get() + timeout_ms;
    while (k_uptime_get() < deadline) {
        int ready = stcp_rust_is_connected(ctx->rust_ctx);
        if (ready > 0) return 0;
        if (ready < 0 && ready != -EAGAIN) return ready;
        (void)k_sem_take(&ctx->rust_event, K_MSEC(25));
        (void)stcp_rust_tick(ctx->rust_ctx);
    }
    return -ETIMEDOUT;
}
#endif

static int stcp_close(void *obj)
{
    struct stcp_ctx *ctx = obj;
    if (!ctx) return 0;
#ifdef CONFIG_STCP_RUST_CORE
    stcp_rust_rx_stop(ctx);
    if (ctx->rust_ctx) {
        stcp_rust_set_carrier(ctx->rust_ctx, NULL);
        stcp_rust_release(ctx->rust_ctx);
        ctx->rust_ctx = NULL;
    }
#endif
    if (ctx->carrier_fd >= 0) zsock_close(ctx->carrier_fd);
    stcp_ctx_free(ctx);
    return 0;
}

static ssize_t stcp_read(void *obj, void *buf, size_t len)
{
    struct stcp_ctx *ctx = obj;
#ifdef CONFIG_STCP_RUST_CORE
    while (true) {
        ssize_t rc = stcp_rust_recv(ctx->rust_ctx, buf, len, 0);
        if (rc >= 0) return rc;
        if (rc != -EAGAIN) { errno = (int)-rc; return -1; }
        (void)k_sem_take(&ctx->rust_event, K_MSEC(100));
    }
#else
    return zsock_recv(ctx->carrier_fd, buf, len, 0);
#endif
}

static ssize_t stcp_write(void *obj, const void *buf, size_t len)
{
    struct stcp_ctx *ctx = obj;
#ifdef CONFIG_STCP_RUST_CORE
    while (true) {
        ssize_t rc = stcp_rust_send(ctx->rust_ctx, buf, len, 0);
        if (rc >= 0) return rc;
        if (rc != -EAGAIN) { errno = (int)-rc; return -1; }
        (void)k_sem_take(&ctx->rust_event, K_MSEC(10));
        (void)stcp_rust_tick(ctx->rust_ctx);
    }
#else
    return zsock_send(ctx->carrier_fd, buf, len, 0);
#endif
}

static int stcp_ioctl(void *obj, unsigned int request, va_list args)
{
    ARG_UNUSED(obj); ARG_UNUSED(args);
    /* Rust-core client mode currently uses blocking socket semantics. The
     * benchmark application skips O_NONBLOCK for AF_STCP. */
    ARG_UNUSED(request);
    errno = ENOTSUP;
    return -1;
}

static int stcp_bind(void *obj, const struct sockaddr *addr, socklen_t addrlen)
{
    struct stcp_ctx *ctx = obj;
    const struct sockaddr_in *sin = (const struct sockaddr_in *)addr;
    if (!ctx || !addr || addrlen < sizeof(*sin)) { errno = EINVAL; return -1; }
    if (zsock_bind(ctx->carrier_fd, addr, addrlen) < 0) return -1;
#ifdef CONFIG_STCP_RUST_CORE
    if (rust_errno(stcp_rust_bind(ctx->rust_ctx, sin->sin_addr.s_addr, sin->sin_port)) < 0) return -1;
#endif
    memcpy(&ctx->local, addr, sizeof(ctx->local));
    ctx->state = STCP_BOUND;
    return 0;
}

static int stcp_connect(void *obj, const struct sockaddr *addr, socklen_t addrlen)
{
    struct stcp_ctx *ctx = obj;
    const struct sockaddr_in *peer = (const struct sockaddr_in *)addr;
    struct sockaddr_in carrier_peer;
    int rc;
    if (!ctx || !addr || addrlen < sizeof(*peer) || peer->sin_family != AF_INET) {
        errno = EINVAL; return -1;
    }
    memset(&carrier_peer, 0, sizeof(carrier_peer));
    carrier_peer.sin_family = AF_INET;
    carrier_peer.sin_port = peer->sin_port;
    carrier_peer.sin_addr = peer->sin_addr;
    ctx->state = STCP_CONNECTING;
    {
        uint32_t peer_host = ntohl(carrier_peer.sin_addr.s_addr);

        LOG_INF("carrier connect fd=%d peer=%u.%u.%u.%u:%u",
                ctx->carrier_fd,
                (unsigned int)((peer_host >> 24) & 0xffU),
                (unsigned int)((peer_host >> 16) & 0xffU),
                (unsigned int)((peer_host >> 8) & 0xffU),
                (unsigned int)(peer_host & 0xffU),
                ntohs(carrier_peer.sin_port));
    }
    rc = zsock_connect(ctx->carrier_fd, (const struct sockaddr *)&carrier_peer,
                       sizeof(carrier_peer));
    if (rc < 0) { ctx->state = STCP_CREATED; return -1; }
#ifdef CONFIG_STCP_RUST_CORE
    rc = stcp_rust_connect(ctx->rust_ctx, peer->sin_addr.s_addr,
                           peer->sin_port, 0);
    if (rc < 0) { errno = -rc; return -1; }
    rc = stcp_rust_rx_start(ctx);
    if (rc < 0) { errno = -rc; return -1; }
    LOG_INF("Rust STCP handshake start fd=%d rust_ctx=%p", ctx->fd, ctx->rust_ctx);
    rc = stcp_rust_start_handshake(ctx->rust_ctx);
    if (rc < 0) { errno = -rc; return -1; }
    rc = wait_rust_ready(ctx, CONFIG_STCP_CONNECT_TIMEOUT_MS);
    if (rc < 0) {
        LOG_ERR("Rust STCP handshake failed rc=%d", rc);
        errno = -rc;
        return -1;
    }
    LOG_INF("Rust STCP handshake complete fd=%d", ctx->fd);
#endif
    memcpy(&ctx->peer, &carrier_peer, sizeof(ctx->peer));
    ctx->state = STCP_CONNECTED;
    return 0;
}

static int stcp_listen(void *obj, int backlog)
{
    struct stcp_ctx *ctx = obj;
    if (zsock_listen(ctx->carrier_fd, backlog) < 0) return -1;
#ifdef CONFIG_STCP_RUST_CORE
    if (rust_errno(stcp_rust_listen(ctx->rust_ctx, backlog)) < 0) return -1;
#endif
    ctx->state = STCP_LISTENING;
    return 0;
}

static int stcp_accept(void *obj, struct sockaddr *addr, socklen_t *addrlen)
{
    /* Phase 1 keeps server-side accept on the existing C carrier path.
     * Client connect/send/recv already use the shared Raspberry Rust core. */
    struct stcp_ctx *listener = obj;
    int cfd = zsock_accept(listener->carrier_fd, addr, addrlen);
    if (cfd < 0) return -1;
    struct stcp_ctx *child = stcp_ctx_alloc();
    if (!child) { zsock_close(cfd); errno = ENOMEM; return -1; }
    int fd = zvfs_reserve_fd();
    if (fd < 0) { zsock_close(cfd); stcp_ctx_free(child); return -1; }
    child->fd = fd; child->carrier_fd = cfd;
    child->socket_type = listener->socket_type;
    child->protocol = listener->protocol;
    child->state = STCP_CONNECTED;
    extern const struct socket_op_vtable stcp_vtable;
    zvfs_finalize_typed_fd(fd, child, (const struct fd_op_vtable *)&stcp_vtable, ZVFS_MODE_IFSOCK);
    return fd;
}

static ssize_t stcp_sendto(void *obj, const void *buf, size_t len, int flags,
                           const struct sockaddr *dst, socklen_t dstlen)
{
    ARG_UNUSED(dst); ARG_UNUSED(dstlen);
#ifdef CONFIG_STCP_RUST_CORE
    return stcp_write(obj, buf, len);
#else
    struct stcp_ctx *ctx=obj;
    return dst ? zsock_sendto(ctx->carrier_fd,buf,len,flags,dst,dstlen) : zsock_send(ctx->carrier_fd,buf,len,flags);
#endif
}

static ssize_t stcp_recvfrom(void *obj, void *buf, size_t len, int flags,
                             struct sockaddr *src, socklen_t *srclen)
{
    ARG_UNUSED(flags); ARG_UNUSED(src); ARG_UNUSED(srclen);
#ifdef CONFIG_STCP_RUST_CORE
    return stcp_read(obj, buf, len);
#else
    struct stcp_ctx *ctx=obj;
    return src ? zsock_recvfrom(ctx->carrier_fd,buf,len,flags,src,srclen) : zsock_recv(ctx->carrier_fd,buf,len,flags);
#endif
}

static int stcp_shutdown(void *obj, int how)
{
    struct stcp_ctx *ctx = obj;
#ifdef CONFIG_STCP_RUST_CORE
    stcp_rust_shutdown(ctx->rust_ctx, how);
#endif
    return zsock_shutdown(ctx->carrier_fd, how);
}
static int stcp_getsockopt(void *obj,int level,int optname,void *optval,socklen_t *optlen)
{ return zsock_getsockopt(((struct stcp_ctx *)obj)->carrier_fd,level,optname,optval,optlen); }
static int stcp_setsockopt(void *obj,int level,int optname,const void *optval,socklen_t optlen)
{ return zsock_setsockopt(((struct stcp_ctx *)obj)->carrier_fd,level,optname,optval,optlen); }
static int stcp_getpeername(void *obj, struct sockaddr *addr, socklen_t *len)
{ return zsock_getpeername(((struct stcp_ctx *)obj)->carrier_fd,addr,len); }
static int stcp_getsockname(void *obj, struct sockaddr *addr, socklen_t *len)
{ return zsock_getsockname(((struct stcp_ctx *)obj)->carrier_fd,addr,len); }

const struct socket_op_vtable stcp_vtable = {
    .fd_vtable = { .read = stcp_read, .write = stcp_write, .close = stcp_close, .ioctl = stcp_ioctl },
    .bind = stcp_bind, .connect = stcp_connect, .listen = stcp_listen, .accept = stcp_accept,
    .sendto = stcp_sendto, .recvfrom = stcp_recvfrom, .shutdown = stcp_shutdown,
    .getsockopt = stcp_getsockopt, .setsockopt = stcp_setsockopt,
    .getpeername = stcp_getpeername, .getsockname = stcp_getsockname,
};

static bool stcp_supported(int family, int type, int protocol)
{
    return family == AF_STCP && (type == SOCK_STREAM || type == SOCK_DGRAM) &&
           (protocol == 0 || protocol == IPPROTO_STCP);
}

static int stcp_socket(int family, int type, int protocol)
{
    struct stcp_ctx *ctx;
    int fd;
    int rc;
    ARG_UNUSED(family);
    if (protocol == 0) protocol = IPPROTO_STCP;
    ctx = stcp_ctx_alloc();
    if (!ctx) { errno = ENOMEM; return -1; }
    ctx->carrier_fd = stcp_carrier_open(type);
    if (ctx->carrier_fd < 0) { stcp_ctx_free(ctx); return -1; }
#ifdef CONFIG_STCP_RUST_CORE
    rc = stcp_rust_create(type == SOCK_DGRAM ? 254 : 253, &ctx->rust_ctx);
    if (rc < 0) {
        zsock_close(ctx->carrier_fd); stcp_ctx_free(ctx); errno = -rc; return -1;
    }
    stcp_rust_set_owner(ctx->rust_ctx, ctx);
    stcp_rust_set_carrier(ctx->rust_ctx, ctx);
#endif
    fd = zvfs_reserve_fd();
    if (fd < 0) { stcp_close(ctx); return -1; }
    ctx->fd = fd; ctx->socket_type = type; ctx->protocol = protocol;
    zvfs_finalize_typed_fd(fd, ctx, (const struct fd_op_vtable *)&stcp_vtable, ZVFS_MODE_IFSOCK);
    LOG_INF("STCP fd=%d type=%d protocol=%d carrier_fd=%d rust=%p",
            fd, type, protocol, ctx->carrier_fd,
#ifdef CONFIG_STCP_RUST_CORE
            ctx->rust_ctx
#else
            NULL
#endif
    );
    return fd;
}
NET_SOCKET_OFFLOAD_REGISTER(stcp, CONFIG_STCP_SOCKET_PRIORITY, AF_STCP, stcp_supported, stcp_socket);
