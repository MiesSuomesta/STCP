#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/fdtable.h>
#include <stcp/stcp_internal.h>

LOG_MODULE_REGISTER(stcp_offload, CONFIG_STCP_LOG_LEVEL);

static int stcp_close(void *obj);
static ssize_t stcp_read(void *obj, void *buffer, size_t count);
static ssize_t stcp_write(void *obj, const void *buffer, size_t count);
static int stcp_ioctl(void *obj, unsigned int request, va_list args);
static int stcp_bind(void *obj, const struct sockaddr *addr, socklen_t addrlen);
static int stcp_connect(void *obj, const struct sockaddr *addr, socklen_t addrlen);
static int stcp_listen(void *obj, int backlog);
static int stcp_accept(void *obj, struct sockaddr *addr, socklen_t *addrlen);
static ssize_t stcp_sendto(void *obj, const void *buf, size_t len, int flags,
                           const struct sockaddr *dest_addr, socklen_t addrlen);
static ssize_t stcp_recvfrom(void *obj, void *buf, size_t max_len, int flags,
                             struct sockaddr *src_addr, socklen_t *addrlen);
static int stcp_getsockopt(void *obj, int level, int optname, void *optval, socklen_t *optlen);
static int stcp_setsockopt(void *obj, int level, int optname, const void *optval, socklen_t optlen);
static int stcp_getpeername(void *obj, struct sockaddr *addr, socklen_t *addrlen);
static int stcp_getsockname(void *obj, struct sockaddr *addr, socklen_t *addrlen);
static int stcp_shutdown(void *obj, int how);

static const struct socket_op_vtable stcp_vtable = {
    .fd_vtable = {
        .read = stcp_read,
        .write = stcp_write,
        .close = stcp_close,
        .ioctl = stcp_ioctl,
    },
    .bind = stcp_bind,
    .connect = stcp_connect,
    .listen = stcp_listen,
    .accept = stcp_accept,
    .sendto = stcp_sendto,
    .recvfrom = stcp_recvfrom,
    .getsockopt = stcp_getsockopt,
    .setsockopt = stcp_setsockopt,
    .getpeername = stcp_getpeername,
    .getsockname = stcp_getsockname,
    .shutdown = stcp_shutdown,
};

static int fail(int ret)
{
    errno = ret < 0 ? -ret : ret;
    return -1;
}

static bool stcp_is_supported(int family, int type, int protocol)
{
    if (family != AF_STCP || type != SOCK_STREAM) return false;
    return protocol == STCP_PROTO_DEFAULT || protocol == STCP_PROTO_TCP ||
           protocol == STCP_PROTO_UDP;
}

static int stcp_socket_create(int family, int type, int protocol)
{
    struct stcp_socket_ctx *ctx;
    int fd;
    int ret;

    if (!stcp_is_supported(family, type, protocol)) return fail(-EPROTONOSUPPORT);

    fd = zvfs_reserve_fd();
    if (fd < 0) return -1;

    ctx = stcp_ctx_alloc();
    if (ctx == NULL) {
        zvfs_free_fd(fd);
        return fail(-ENOMEM);
    }

    ctx->fd = fd;
    ctx->type = type;
    ctx->protocol = protocol == STCP_PROTO_DEFAULT ? STCP_PROTO_TCP : protocol;
    ret = stcp_carrier_init(&ctx->carrier, ctx->protocol);
    if (ret < 0) {
        stcp_ctx_release(ctx);
        zvfs_free_fd(fd);
        return fail(ret);
    }

    zvfs_finalize_typed_fd(fd, ctx, (const struct fd_op_vtable *)&stcp_vtable,
                           ZVFS_MODE_IFSOCK);
    LOG_INF("STCP fd=%d protocol=%d carrier_fd=%d", fd, ctx->protocol, ctx->carrier.fd);
    return fd;
}

NET_SOCKET_OFFLOAD_REGISTER(stcp, CONFIG_STCP_SOCKET_PRIORITY, AF_STCP,
                            stcp_is_supported, stcp_socket_create);

static int stcp_close(void *obj)
{
    struct stcp_socket_ctx *ctx = obj;
    if (ctx == NULL) return fail(-EBADF);
    stcp_ctx_release(ctx);
    return 0;
}

static ssize_t stcp_read(void *obj, void *buffer, size_t count)
{
    return stcp_recvfrom(obj, buffer, count, 0, NULL, NULL);
}

static ssize_t stcp_write(void *obj, const void *buffer, size_t count)
{
    return stcp_sendto(obj, buffer, count, 0, NULL, 0);
}

static int stcp_ioctl(void *obj, unsigned int request, va_list args)
{
    ARG_UNUSED(request);
    ARG_UNUSED(args);
    if (obj == NULL) return fail(-EBADF);
    return fail(-ENOTSUP);
}

static int validate_inet(const struct sockaddr *addr, socklen_t len,
                         const struct sockaddr_in **out)
{
    if (addr == NULL || out == NULL || len < sizeof(struct sockaddr_in)) return -EINVAL;
    if (addr->sa_family != AF_INET) return -EAFNOSUPPORT;
    *out = (const struct sockaddr_in *)addr;
    return 0;
}

static int stcp_bind(void *obj, const struct sockaddr *addr, socklen_t addrlen)
{
    const struct sockaddr_in *sin;
    int ret = validate_inet(addr, addrlen, &sin);
    if (ret < 0) return fail(ret);
    ret = stcp_core_bind(obj, sin);
    return ret < 0 ? fail(ret) : 0;
}

static int stcp_connect(void *obj, const struct sockaddr *addr, socklen_t addrlen)
{
    const struct sockaddr_in *sin;
    int ret = validate_inet(addr, addrlen, &sin);
    if (ret < 0) return fail(ret);
    ret = stcp_core_connect(obj, sin);
    return ret < 0 ? fail(ret) : 0;
}

static int stcp_listen(void *obj, int backlog)
{
    int ret = stcp_core_listen(obj, backlog);
    return ret < 0 ? fail(ret) : 0;
}

static int stcp_accept(void *obj, struct sockaddr *addr, socklen_t *addrlen)
{
    struct stcp_socket_ctx *listener = obj;
    struct stcp_socket_ctx *child;
    struct sockaddr_in peer;
    socklen_t peer_len = sizeof(peer);
    int fd;
    int ret;

    if (listener == NULL) return fail(-EBADF);

    fd = zvfs_reserve_fd();
    if (fd < 0) return -1;

    child = stcp_ctx_alloc();
    if (child == NULL) {
        zvfs_free_fd(fd);
        return fail(-ENOMEM);
    }

    ret = stcp_core_accept(listener, child, &peer, &peer_len);
    if (ret < 0) {
        stcp_ctx_release(child);
        zvfs_free_fd(fd);
        return fail(ret);
    }

    child->fd = fd;
    zvfs_finalize_typed_fd(fd, child, (const struct fd_op_vtable *)&stcp_vtable,
                           ZVFS_MODE_IFSOCK);

    if (addr != NULL && addrlen != NULL) {
        if (*addrlen < sizeof(peer)) {
            stcp_ctx_release(child);
            return fail(-EINVAL);
        }
        memcpy(addr, &peer, sizeof(peer));
        *addrlen = sizeof(peer);
    }
    return fd;
}

static ssize_t stcp_sendto(void *obj, const void *buf, size_t len, int flags,
                           const struct sockaddr *dest_addr, socklen_t addrlen)
{
    ssize_t ret;
    ARG_UNUSED(dest_addr);
    ARG_UNUSED(addrlen);
    ret = stcp_core_send(obj, buf, len, flags);
    return ret < 0 ? fail((int)ret) : ret;
}

static ssize_t stcp_recvfrom(void *obj, void *buf, size_t max_len, int flags,
                             struct sockaddr *src_addr, socklen_t *addrlen)
{
    ssize_t ret;
    ARG_UNUSED(src_addr);
    ARG_UNUSED(addrlen);
    ret = stcp_core_recv(obj, buf, max_len, flags);
    return ret < 0 ? fail((int)ret) : ret;
}

static int stcp_getsockopt(void *obj, int level, int optname, void *optval, socklen_t *optlen)
{
    struct stcp_socket_ctx *ctx = obj;
    if (ctx == NULL || optval == NULL || optlen == NULL) return fail(-EINVAL);
    if (level == SOL_SOCKET && optname == SO_ERROR && *optlen >= sizeof(int)) {
        *(int *)optval = ctx->last_error;
        *optlen = sizeof(int);
        ctx->last_error = 0;
        return 0;
    }
    return fail(-ENOPROTOOPT);
}

static int stcp_setsockopt(void *obj, int level, int optname, const void *optval, socklen_t optlen)
{
    ARG_UNUSED(obj); ARG_UNUSED(level); ARG_UNUSED(optname); ARG_UNUSED(optval); ARG_UNUSED(optlen);
    return fail(-ENOPROTOOPT);
}

static int copy_addr(const struct sockaddr_in *src, struct sockaddr *addr, socklen_t *addrlen)
{
    if (src == NULL || addr == NULL || addrlen == NULL || *addrlen < sizeof(*src)) return -EINVAL;
    memcpy(addr, src, sizeof(*src));
    *addrlen = sizeof(*src);
    return 0;
}

static int stcp_getpeername(void *obj, struct sockaddr *addr, socklen_t *addrlen)
{
    struct stcp_socket_ctx *ctx = obj;
    struct sockaddr_in peer;
    socklen_t len = sizeof(peer);
    int ret;
    if (ctx == NULL || ctx->state != STCP_STATE_CONNECTED) return fail(-ENOTCONN);
    ret = stcp_carrier_getpeername(&ctx->carrier, &peer, &len);
    if (ret < 0) peer = ctx->peer;
    ret = copy_addr(&peer, addr, addrlen);
    return ret < 0 ? fail(ret) : 0;
}

static int stcp_getsockname(void *obj, struct sockaddr *addr, socklen_t *addrlen)
{
    struct stcp_socket_ctx *ctx = obj;
    struct sockaddr_in local;
    socklen_t len = sizeof(local);
    int ret;
    if (ctx == NULL) return fail(-EBADF);
    ret = stcp_carrier_getsockname(&ctx->carrier, &local, &len);
    if (ret < 0) local = ctx->local;
    ret = copy_addr(&local, addr, addrlen);
    return ret < 0 ? fail(ret) : 0;
}

static int stcp_shutdown(void *obj, int how)
{
    int ret = stcp_core_shutdown(obj, how);
    return ret < 0 ? fail(ret) : 0;
}
