#include <errno.h>
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

static int set_errno_from_ret(int ret)
{
    errno = ret < 0 ? -ret : ret;
    return -1;
}

static bool stcp_is_supported(int family, int type, int protocol)
{
    return family == AF_STCP && type == SOCK_STREAM &&
           (protocol == 0 || protocol == STCP_PROTO);
}

static int stcp_socket_create(int family, int type, int protocol)
{
    struct stcp_socket_ctx *ctx;
    int fd;
    if (!stcp_is_supported(family, type, protocol)) {
        errno = EAFNOSUPPORT;
        return -1;
    }
    fd = zvfs_reserve_fd();
    if (fd < 0) return -1;
    ctx = stcp_ctx_alloc();
    if (ctx == NULL) {
        zvfs_free_fd(fd);
        errno = ENOMEM;
        return -1;
    }
    ctx->fd = fd;
    ctx->type = type;
    ctx->protocol = protocol == 0 ? STCP_PROTO : protocol;
    zvfs_finalize_typed_fd(fd, ctx, (const struct fd_op_vtable *)&stcp_vtable,
                           ZVFS_MODE_IFSOCK);
    LOG_INF("created STCP fd=%d", fd);
    return fd;
}

NET_SOCKET_OFFLOAD_REGISTER(stcp, CONFIG_STCP_SOCKET_PRIORITY, AF_STCP,
                            stcp_is_supported, stcp_socket_create);

static int stcp_close(void *obj)
{
    struct stcp_socket_ctx *ctx = obj;
    if (ctx == NULL) return set_errno_from_ret(-EBADF);
    (void)stcp_core_shutdown(ctx, ZSOCK_SHUT_RDWR);
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
    if (obj == NULL) {
        return set_errno_from_ret(-EBADF);
    }

    ARG_UNUSED(request);
    ARG_UNUSED(args);

    /*
     * Zephyr 4.4 no longer exposes ZFD_IOCTL_FCNTL to socket
     * providers. Poll/nonblocking ioctl integration will be added
     * when the real STCP transport and readiness handling are wired in.
     */
    return set_errno_from_ret(-ENOTSUP);
}

static int validate_addr(const struct sockaddr *addr, socklen_t len,
                         const struct sockaddr_stcp **out)
{
    if (addr == NULL || out == NULL || len < sizeof(struct sockaddr_stcp)) return -EINVAL;
    if (addr->sa_family != AF_STCP) return -EAFNOSUPPORT;
    *out = (const struct sockaddr_stcp *)addr;
    return 0;
}

static int stcp_bind(void *obj, const struct sockaddr *addr, socklen_t addrlen)
{
    const struct sockaddr_stcp *sa;
    int ret = validate_addr(addr, addrlen, &sa);
    if (ret < 0) return set_errno_from_ret(ret);
    ret = stcp_core_bind(obj, sa);
    return ret < 0 ? set_errno_from_ret(ret) : 0;
}

static int stcp_connect(void *obj, const struct sockaddr *addr, socklen_t addrlen)
{
    const struct sockaddr_stcp *sa;
    int ret = validate_addr(addr, addrlen, &sa);
    if (ret < 0) return set_errno_from_ret(ret);
    ret = stcp_core_connect(obj, sa);
    return ret < 0 ? set_errno_from_ret(ret) : 0;
}

static int stcp_listen(void *obj, int backlog)
{
    int ret = stcp_core_listen(obj, backlog);
    return ret < 0 ? set_errno_from_ret(ret) : 0;
}

static int stcp_accept(void *obj, struct sockaddr *addr, socklen_t *addrlen)
{
    ARG_UNUSED(obj); ARG_UNUSED(addr); ARG_UNUSED(addrlen);
    return set_errno_from_ret(-EAGAIN);
}

static ssize_t stcp_sendto(void *obj, const void *buf, size_t len, int flags,
                           const struct sockaddr *dest_addr, socklen_t addrlen)
{
    ARG_UNUSED(dest_addr); ARG_UNUSED(addrlen);
    ssize_t ret = stcp_core_send(obj, buf, len, flags);
    return ret < 0 ? set_errno_from_ret((int)ret) : ret;
}

static ssize_t stcp_recvfrom(void *obj, void *buf, size_t max_len, int flags,
                             struct sockaddr *src_addr, socklen_t *addrlen)
{
    struct stcp_socket_ctx *ctx = obj;
    ARG_UNUSED(src_addr); ARG_UNUSED(addrlen);
    ssize_t ret = stcp_core_recv(ctx, buf, max_len, flags);
    return ret < 0 ? set_errno_from_ret((int)ret) : ret;
}

static int stcp_getsockopt(void *obj, int level, int optname, void *optval, socklen_t *optlen)
{
    struct stcp_socket_ctx *ctx = obj;
    if (ctx == NULL || optval == NULL || optlen == NULL) return set_errno_from_ret(-EINVAL);
    if (level == SOL_SOCKET && optname == SO_ERROR && *optlen >= sizeof(int)) {
        *(int *)optval = ctx->last_error;
        *optlen = sizeof(int);
        ctx->last_error = 0;
        return 0;
    }
    return set_errno_from_ret(-ENOPROTOOPT);
}

static int stcp_setsockopt(void *obj, int level, int optname, const void *optval, socklen_t optlen)
{
    ARG_UNUSED(obj); ARG_UNUSED(level); ARG_UNUSED(optname); ARG_UNUSED(optval); ARG_UNUSED(optlen);
    return set_errno_from_ret(-ENOPROTOOPT);
}

static int copy_name(const struct sockaddr_stcp *src, struct sockaddr *addr, socklen_t *addrlen)
{
    if (addr == NULL || addrlen == NULL || *addrlen < sizeof(*src)) return -EINVAL;
    memcpy(addr, src, sizeof(*src));
    *addrlen = sizeof(*src);
    return 0;
}

static int stcp_getpeername(void *obj, struct sockaddr *addr, socklen_t *addrlen)
{
    struct stcp_socket_ctx *ctx = obj;
    if (ctx->state != STCP_STATE_CONNECTED) return set_errno_from_ret(-ENOTCONN);
    int ret = copy_name(&ctx->peer, addr, addrlen);
    return ret < 0 ? set_errno_from_ret(ret) : 0;
}

static int stcp_getsockname(void *obj, struct sockaddr *addr, socklen_t *addrlen)
{
    struct stcp_socket_ctx *ctx = obj;
    int ret = copy_name(&ctx->local, addr, addrlen);
    return ret < 0 ? set_errno_from_ret(ret) : 0;
}

static int stcp_shutdown(void *obj, int how)
{
    int ret = stcp_core_shutdown(obj, how);
    return ret < 0 ? set_errno_from_ret(ret) : 0;
}
