#include <zephyr/net/socket.h>
#include <zephyr/kernel.h>
// Offloadi
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_offload.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/device.h>

#include <zephyr/net/socket.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_context.h>
#include <zephyr/net/net_pkt.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#include <errno.h>

#define STCP_SOCKET_INTERNAL 1
#include <stcp_api.h>
#include <stcp/debug.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_socket.h>
#include <stcp/stcp_transport.h>
#include <stcp/stcp_lte.h>
#include <stcp/stcp_rx_transmission.h>
#include <stcp/stcp_rust_exported_functions.h>

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

#define STCP_RESET_WAIT_SERVICES_FOR_SECONDS    (5*60)

#define MUTEX_DO_CHECK_ASSERTS(ctx) \
    do {                                                                      \
        __ASSERT(ctx != NULL, "CTX NULL");                                    \
        __ASSERT((ctx)->lock.owner != (void *)0xffffffff, "Mutex corrupted"); \
    } while (0)

#define MUTEX_DO_CLOSING_CHECK(ctx)    \
    do {                                                  \
        LDBG("CTX %p state: closing=%d ref=%d",           \
              ctx,                                        \
              atomic_get(&ctx->closing),                  \
              atomic_get(&ctx->refcnt)                    \
        );                                                \
        if (atomic_get(&(ctx)->closing)) {                \
            LWRNBIG("Context %p is marked as closing!");  \
            return -ESHUTDOWN;                            \
        }                                                 \
    } while (0)

#define NULL_CHECK_GUARD_CODE(val, CODE)                   \
    do {                                                   \
        if ((val) == NULL) {                               \
            LERRBIG("NULL CHECK FAILED!");                 \
        } else {                                           \
            LDBG("%s Owner %p, Thread = %p",               \
                #val,                                      \
                (val)->lock.owner,                         \
                k_current_get()                            \
            );                                             \
            CODE;                                          \
        }                                                  \
    } while (0)

#define __CTX_SOCK_LOCK(ctx)                   \
    do {                                       \
        MUTEX_DO_CLOSING_CHECK(ctx);           \
        MUTEX_DO_CHECK_ASSERTS(ctx);           \
        LDBG("Locking %p context...", ctx);    \
        k_mutex_lock(&(ctx)->lock, K_FOREVER); \
        LDBG("Locked %p context...", ctx);     \
    } while (0)

#define __CTX_SOCK_UNLOCK(ctx) \
    do {                                       \
        MUTEX_DO_CLOSING_CHECK(ctx);           \
        MUTEX_DO_CHECK_ASSERTS(ctx);           \
        LDBG("Unlocking %p context...", ctx);  \
        k_mutex_unlock(&(ctx)->lock);          \
        LDBG("Unlocked %p context...", ctx);   \
    } while (0)


#define CTX_SOCK_LOCK(ctx) \
    NULL_CHECK_GUARD_CODE(ctx, __CTX_SOCK_LOCK(ctx))

#define CTX_SOCK_UNLOCK(ctx) \
    NULL_CHECK_GUARD_CODE(ctx, __CTX_SOCK_UNLOCK(ctx))


int stcp_new_context_with_fd(struct stcp_ctx **ctxSaveTo, int fd)
{
    
    if (ctxSaveTo == NULL) {
        LERR("No place for context to be saved.");
        return -EINVAL;
    }

    if (fd < 0) {
        LERR("No FD for context to be saved.");
        return -EINVAL;
    }

    struct stcp_ctx *ctx = stcp_create_new_context( fd );
    LDBG("STCP Context created: %p", ctx);

    if (ctx) {
        *ctxSaveTo = ctx;
        return 0;
    }

    return -ENOMEM;

}

int stcp_new_empty_context(struct stcp_ctx **ctxSaveTo)
{
    
    if (ctxSaveTo == NULL) {
        LERR("No place for context to be saved.");
        return -EINVAL;
    }

    struct stcp_ctx *ctx = stcp_create_new_context( -1 );
    LDBG("STCP Context created: %p", ctx);

    if (ctx) {
        *ctxSaveTo = ctx;
        return 0;
    }

    return -ENOMEM;

}

int stcp_new_context(struct stcp_ctx **ctxSaveTo)
{

    if (ctxSaveTo == NULL) {
        return -EINVAL;
    }

    int rc = stcp_new_empty_context(ctxSaveTo);
    if (rc < 0) {
        return -ENOMEM;
    }

    int fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        return fd;
    }

    if (! *ctxSaveTo ) {
        zsock_close(fd);
        return -ENOMEM;
    }

    // Override FD (is -1)
    struct stcp_ctx *ctx = *ctxSaveTo;
    ctx->ks.fd = fd;

    LDBG("STCP Context created: %p with fd: %d...", ctx, fd);
    return fd;
}

int stcp_bind(struct stcp_ctx *ctx,
              const struct sockaddr *addr,
              socklen_t addrlen)
{
    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        return -ENOTCONN;
    }

    int ret = zsock_bind(ctx->ks.fd, addr, addrlen);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int stcp_listen(struct stcp_ctx *ctx,
                int backlog)
{
    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        return -ENOTCONN;
    }
    
    int ret = zsock_listen(ctx->ks.fd, backlog);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int stcp_accept(struct stcp_ctx *parent,
                struct stcp_ctx **child_out,
                struct sockaddr *peer_addr,
                socklen_t *peer_len)
{

    if (stcp_is_context_valid(parent) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", parent, errno);
        return -ENOTCONN;
    }
    

    LDBG("Context at %s: parent %p // HS Done: %d // FD: %d",
        __func__, parent, parent->handshake_done, parent->ks.fd
    );

    if (!parent || !child_out) {
        return -EINVAL;
    }

    CTX_SOCK_LOCK(parent);
    LDBG("Parent lock GET");
    int new_fd = zsock_accept(parent->ks.fd, peer_addr, peer_len);
    if (new_fd < 0) {
        LDBG("Parent lock PUT");
        CTX_SOCK_UNLOCK(parent);
        return -errno;
    }

    struct stcp_ctx *child = NULL;

    // TÄRKEÄ
    int ret = stcp_new_context_with_fd(&child, new_fd);
    CTX_SOCK_LOCK(child);
    LDBG("Child lock GET");

    if (ret < 0) {
        zsock_close(new_fd);
        if (child != NULL) {
            LDBG("Child lock PUT");
            CTX_SOCK_UNLOCK(child);
            LDBG("Parent lock PUT");
            CTX_SOCK_UNLOCK(parent);
            stcp_close(child);
        }
        return ret;
    }

    LDBG("Context at %s: child %p // HS Done: %d // FD: %d",
        __func__, child, child->handshake_done, child->ks.fd
    );

    *child_out = child;
    LDBG("Child lock PUTT");
    CTX_SOCK_UNLOCK(child);

    LDBG("Parent lock PUT");
    CTX_SOCK_UNLOCK(parent);
    return 0;
}

int stcp_socket(int not_used_1,int not_used_2, int not_used_3) {

    int fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(fd < 0) {
        LERR("Error when creating sock, rc: %d", fd);
    }
    return fd;
}

int stcp_connect(struct stcp_ctx *ctx, const struct sockaddr *addr, socklen_t addrlen)
{
    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        return -ENOTCONN;
    }

    LDBG("Context at %s: %p // HS Done: %d // FD: %d",
        __func__, ctx, ctx->handshake_done, ctx->ks.fd
    );

    if (!addr) {
        LERR("No address!");
        return -EINVAL;
    }

    LDBG("Conneting...");

    int err;
    socklen_t len = sizeof(err);

    if (zsock_getsockopt(ctx->ks.fd,
                        SOL_SOCKET,
                        SO_ERROR,
                        &err,
                        &len) == 0) {
       LDBG("Pre-connect SO_ERROR: %d", err);
    }

    int rc = zsock_connect(ctx->ks.fd, addr, addrlen);


    if (zsock_getsockopt(ctx->ks.fd,
                        SOL_SOCKET,
                        SO_ERROR,
                        &err,
                        &len) == 0) {
       LDBG("Post-connect SO_ERROR: %d", err);
    }

    if (errno != EISCONN) {
        if (rc < 0) {
            LERR("Connect error, rc: %d, errno: %d", rc, errno);
            return rc;
        }
    }

    LDBG("[Ctx: %p] Doing handshake...", ctx);
    int rv = stcp_handshake_for_context(ctx);
    return rv;
}


ssize_t stcp_send(struct stcp_ctx *ctx, const void *buf, size_t len, int flags)
{

    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        return -ENOTCONN;
    }

    LDBG("Context at %s: %p // HS Done: %d // FD: %d",
        __func__, ctx, ctx->handshake_done, ctx->ks.fd
    );

    ARG_UNUSED(flags);

    CTX_SOCK_LOCK(ctx);
        ssize_t ret = stcp_transport_send(ctx, buf, len);
    CTX_SOCK_UNLOCK(ctx);

    return ret;
}

ssize_t stcp_send_msg(struct stcp_ctx *ctx, const struct msghdr *message)
{
    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        return -ENOTCONN;
    }

    LDBG("Context at %s: %p // HS Done: %d // FD: %d",
        __func__, ctx, ctx->handshake_done, ctx->ks.fd
    );

    CTX_SOCK_LOCK(ctx);
        ssize_t ret = stcp_transport_send_iovec(ctx, message);
    CTX_SOCK_UNLOCK(ctx);

    return ret;
}


ssize_t stcp_recv(struct stcp_ctx *ctx, void *buf, size_t len, int flags)
{
    ARG_UNUSED(flags);

    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        return -ENOTCONN;
    }

    LDBG("Context at %s: %p // HS Done: %d // FD: %d",
        __func__, ctx, ctx->handshake_done, ctx->ks.fd
    );


    CTX_SOCK_LOCK(ctx);
        ssize_t ret = stcp_transport_recv(ctx, buf, len);
    CTX_SOCK_UNLOCK(ctx);

    return ret;
}

int stcp_set_non_bloking_to(struct stcp_ctx *ctx, int val)
{

    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        return -ENOTCONN;
    }

    int fd = ctx->ks.fd;

    LINF("[Ctx: %p] Setting nonblocking %d for fd: %d...", ctx, val, fd);
    return zsock_ioctl(fd, ZFD_IOCTL_FIONBIO, &val);
}

int stcp_close(struct stcp_ctx *ctx)
{
    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context or closing, ctx: %p errno: %d", ctx, errno);
        return -ENOTCONN;
    }

    LDBG("Context at %s: %p // HS Done: %d // FD: %d",
        __func__, ctx, ctx->handshake_done, ctx->ks.fd
    );

    stcp_transport_close(ctx); // TÄMÄ HOITAA KAIKEN...
    return 0;
}
