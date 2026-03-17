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


K_MUTEX_DEFINE(g_mutex_new_context_create);
static void g_mutex_new_context_create_init() {
    static int init = 0;
    if (init) return;
    init = 1;

    k_mutex_init(&g_mutex_new_context_create);

}

int stcp_new_context(struct stcp_ctx **ctxSaveTo)
{
    g_mutex_new_context_create_init();

    if (ctxSaveTo == NULL) {
        return -EINVAL;
    }

    k_mutex_lock(&g_mutex_new_context_create, K_FOREVER);
    
    int rc = stcp_new_empty_context(ctxSaveTo);
    if (rc < 0) {
        k_mutex_unlock(&g_mutex_new_context_create);
        return -ENOMEM;
    }

    int fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        k_mutex_unlock(&g_mutex_new_context_create);
        return fd;
    }

    if (! *ctxSaveTo ) {
        zsock_close(fd);
        k_mutex_unlock(&g_mutex_new_context_create);
        return -ENOMEM;
    }

    // Override FD (is -1)
    struct stcp_ctx *ctx = *ctxSaveTo;
    ctx->ks.fd = fd;
    STCP_DBG_CTX_FD(ctx);

    LDBG("STCP Context created: %p with fd: %d...", ctx, fd);
    k_mutex_unlock(&g_mutex_new_context_create);
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

    STCP_DBG_CTX_FD(ctx);
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
    
    STCP_DBG_CTX_FD(ctx);
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
    STCP_DBG_CTX_FD(parent);
    int new_fd = zsock_accept(parent->ks.fd, peer_addr, peer_len);
    if (new_fd < 0) {
        LDBG("Parent lock PUT");
        CTX_SOCK_UNLOCK(parent);
        return -errno;
    }

    struct stcp_ctx *child = NULL;

    // TÄRKEÄ
    int ret = stcp_new_context_with_fd(&child, new_fd);
    STCP_DBG_CTX_FD(child);
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
    LINF("GOT SOCKET fd=%d errno=%d", fd, errno);
    if(fd < 0) {
        LERR("Error when creating sock, rc: %d, errno: %d", fd, errno);
    }
    return fd;
}

static int stcp_wait_for_connect(int fd, int timeout_ms)
{
    struct zsock_pollfd pfd = {
        .fd = fd,
        .events = ZSOCK_POLLOUT,
    };

    LDBG("At poll fd=%d timeout=%d", fd, timeout_ms);
    int rc = zsock_poll(&pfd, 1, timeout_ms);
    LDBG("after poll rc=%d errno=%d fd=%d", rc, errno, fd);

    if (rc == 0) {
        LWRN("Connect timeout");
        return -ETIMEDOUT;
    }

    if (rc < 0) {
        LERR("Poll error: %d", errno);
        return -errno;
    }

    int err = 0;
    socklen_t len = sizeof(err);

    if (zsock_getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
        LERR("getsockopt failed: %d", errno);
        return -errno;
    }

    if (err != 0) {
        LERR("Connect failed, SO_ERROR=%d", err);
        return -err;
    }

    LDBG("Connect completed OK");

    return 0;
}

int stcp_connect(struct stcp_ctx *ctx,
                 const struct sockaddr *addr,
                 socklen_t addrlen)
{
    if (!ctx) {
        LERR("CTX was null!");
        return -EINVAL;
    }

    if (ctx->closing) {
        LWRN("CTX already scheduled for cleanup");
        return -EBADFD;
    }

    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        return -ENOTCONN;
    }
    
    int fd = ctx->ks.fd;
    if (fd < 0) {
        LERR("Invalid FD: %d", fd);
        return -EBADF;
    }

    LDBG("Doing connect with fd: %d from context %p (api %p)", 
        fd, ctx, ctx->api);
    STCP_DBG_CTX_FD(ctx);

    LDBG("Doing connect with fd: %d from context %p (api %p)", 
        fd, ctx, ctx->api);

    struct sockaddr_in *sin = (struct sockaddr_in *)addr;

    LDBG("CONNECT ip=%d.%d.%d.%d port=%d addrlen=%d",
        sin->sin_addr.s4_addr[0],
        sin->sin_addr.s4_addr[1],
        sin->sin_addr.s4_addr[2],
        sin->sin_addr.s4_addr[3],
        ntohs(sin->sin_port),
        addrlen);

     int rc = zsock_connect(fd, addr, addrlen);

    if (rc == 0) {
        LDBG("Connect completed immediately");
        ctx->state = STCP_STATE_CONNECTING;
    } else {
        if (rc < 0) {
            int err;
            socklen_t len = sizeof(err);

            zsock_getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
            LDBG("SO_ERROR[Ctx: %p]: err=%d", ctx, err);

            if (errno != EINPROGRESS) {
                LERR("Connect failed immediately errno=%d", errno);
                return -errno;
            }

            LDBG("Connect in progress...");
            rc = stcp_wait_for_connect(fd, 10000);

            if (rc < 0) {
                LERR("Connect wait failed rc=%d", rc);
                ctx->state = STCP_STATE_CLOSING;
                return rc;
            }
        }
    }

    LINF("TCP connect OK");

    LDBG("[Ctx: %p] Doing handshake...", ctx);
    ctx->state = STCP_STATE_CONNECTING;
    STCP_DBG_CTX_FD(ctx);
    rc = stcp_handshake_for_context(ctx);
    
    if (rc == 1) {
        ctx->state = STCP_STATE_ESTABLISHED;
    } else {
        ctx->state = STCP_STATE_CLOSING;
    }

    return rc;
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

    if (ctx->state != STCP_STATE_ESTABLISHED) {
        LDBG("Context %p at sttate %d tried to send..", ctx, ctx->state);
        return -ENOTCONN;
    }

    ARG_UNUSED(flags);
    STCP_DBG_CTX_FD(ctx);

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

    if (ctx->state != STCP_STATE_ESTABLISHED) {
        LDBG("Context %p at sttate %d tried to send..", ctx, ctx->state);
        return -ENOTCONN;
    }

    STCP_DBG_CTX_FD(ctx);
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

    if (ctx->state != STCP_STATE_ESTABLISHED) {
        LDBG("Context %p at sttate %d tried to recv..", ctx, ctx->state);
        return -ENOTCONN;
    }

    STCP_DBG_CTX_FD(ctx);
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

    if (ctx->state == STCP_STATE_CLOSING) {
        return 0;
    }
    ctx->state = STCP_STATE_CLOSING;

    LDBG("Context at %s: %p // HS Done: %d // FD: %d",
        __func__, ctx, ctx->handshake_done, ctx->ks.fd
    );

    STCP_DBG_CTX_FD(ctx);
    stcp_transport_close(ctx); // TÄMÄ HOITAA KAIKEN...
    return 0;
}
