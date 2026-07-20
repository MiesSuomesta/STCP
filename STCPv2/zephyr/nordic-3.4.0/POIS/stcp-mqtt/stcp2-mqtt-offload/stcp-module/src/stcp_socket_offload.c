#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <stcp/stcp_internal.h>
LOG_MODULE_REGISTER(stcp_offload, CONFIG_STCP_LOG_LEVEL);

static int stcp_close(void *obj)
{
    struct stcp_ctx *ctx = obj;
    if (!ctx) return 0;
    if (ctx->carrier_fd >= 0) zsock_close(ctx->carrier_fd);
    stcp_ctx_free(ctx);
    return 0;
}
static ssize_t stcp_read(void *obj, void *buf, size_t len)
{ return zsock_recv(((struct stcp_ctx *)obj)->carrier_fd, buf, len, 0); }
static ssize_t stcp_write(void *obj, const void *buf, size_t len)
{ return zsock_send(((struct stcp_ctx *)obj)->carrier_fd, buf, len, 0); }
static int stcp_ioctl(void *obj, unsigned int request, va_list args)
{ ARG_UNUSED(obj); ARG_UNUSED(request); ARG_UNUSED(args); errno = ENOTSUP; return -1; }
static int stcp_bind(void *obj, const struct sockaddr *addr, socklen_t addrlen)
{
    struct stcp_ctx *ctx = obj;
    if (!ctx || !addr || addrlen < sizeof(struct sockaddr_in)) { errno = EINVAL; return -1; }
    if (zsock_bind(ctx->carrier_fd, addr, addrlen) < 0) return -1;
    memcpy(&ctx->local, addr, sizeof(ctx->local)); ctx->state = STCP_BOUND; return 0;
}
static int stcp_connect(void *obj, const struct sockaddr *addr, socklen_t addrlen)
{
    struct stcp_ctx *ctx = obj;
    if (!ctx || !addr || addrlen < sizeof(struct sockaddr_in)) { errno = EINVAL; return -1; }
    ctx->state = STCP_CONNECTING;
    int rc = zsock_connect(ctx->carrier_fd, addr, addrlen);
    if (rc < 0 && (errno == EINPROGRESS || errno == EALREADY || errno == EAGAIN)) {
        rc = stcp_carrier_wait_connected(ctx->carrier_fd, CONFIG_STCP_CONNECT_TIMEOUT_MS);
        if (rc < 0) { errno = -rc; ctx->last_error = errno; return -1; }
    } else if (rc < 0) { ctx->last_error = errno; return -1; }
    memcpy(&ctx->peer, addr, sizeof(ctx->peer)); ctx->state = STCP_CONNECTED; return 0;
}
static int stcp_listen(void *obj, int backlog)
{
    struct stcp_ctx *ctx = obj;
    if (zsock_listen(ctx->carrier_fd, backlog) < 0) return -1;
    ctx->state = STCP_LISTENING; return 0;
}
static int stcp_accept(void *obj, struct sockaddr *addr, socklen_t *addrlen)
{
    struct stcp_ctx *listener = obj;
    int cfd = zsock_accept(listener->carrier_fd, addr, addrlen);
    if (cfd < 0) return -1;
    struct stcp_ctx *child = stcp_ctx_alloc();
    if (!child) { zsock_close(cfd); errno = ENOMEM; return -1; }
    int fd = zvfs_reserve_fd();
    if (fd < 0) { zsock_close(cfd); stcp_ctx_free(child); return -1; }
    child->fd = fd; child->carrier_fd = cfd; child->protocol = listener->protocol; child->state = STCP_CONNECTED;
    extern const struct socket_op_vtable stcp_vtable;
    zvfs_finalize_typed_fd(fd, child, (const struct fd_op_vtable *)&stcp_vtable, ZVFS_MODE_IFSOCK);
    return fd;
}
static ssize_t stcp_sendto(void *obj, const void *buf, size_t len, int flags, const struct sockaddr *dst, socklen_t dstlen)
{ struct stcp_ctx *ctx=obj; return dst ? zsock_sendto(ctx->carrier_fd,buf,len,flags,dst,dstlen) : zsock_send(ctx->carrier_fd,buf,len,flags); }
static ssize_t stcp_recvfrom(void *obj, void *buf, size_t len, int flags, struct sockaddr *src, socklen_t *srclen)
{ struct stcp_ctx *ctx=obj; return src ? zsock_recvfrom(ctx->carrier_fd,buf,len,flags,src,srclen) : zsock_recv(ctx->carrier_fd,buf,len,flags); }
static int stcp_shutdown(void *obj, int how)
{ return zsock_shutdown(((struct stcp_ctx *)obj)->carrier_fd, how); }
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
{ return family == AF_STCP && type == SOCK_STREAM && (protocol == 0 || protocol == STCP_PROTO_TCP || protocol == STCP_PROTO_UDP); }
static int stcp_socket(int family, int type, int protocol)
{
    ARG_UNUSED(family); ARG_UNUSED(type);
    if (protocol == 0) protocol = STCP_PROTO_TCP;
    struct stcp_ctx *ctx = stcp_ctx_alloc();
    if (!ctx) { errno = ENOMEM; return -1; }
    ctx->carrier_fd = stcp_carrier_open(protocol);
    if (ctx->carrier_fd < 0) { stcp_ctx_free(ctx); return -1; }
    int fd = zvfs_reserve_fd();
    if (fd < 0) { zsock_close(ctx->carrier_fd); stcp_ctx_free(ctx); return -1; }
    ctx->fd = fd; ctx->protocol = protocol;
    zvfs_finalize_typed_fd(fd, ctx, (const struct fd_op_vtable *)&stcp_vtable, ZVFS_MODE_IFSOCK);
    LOG_INF("STCP fd=%d protocol=%d carrier_fd=%d", fd, protocol, ctx->carrier_fd);
    return fd;
}
NET_SOCKET_OFFLOAD_REGISTER(stcp, CONFIG_STCP_SOCKET_PRIORITY, AF_STCP, stcp_supported, stcp_socket);
