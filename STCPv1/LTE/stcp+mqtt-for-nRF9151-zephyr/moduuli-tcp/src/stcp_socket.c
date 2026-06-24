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
#include <stcp/stcp_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_socket.h>
#include <stcp/stcp_transport.h>
#include <stcp/stcp_lte.h>
#include <stcp/stcp_rx_transmission.h>
#include <stcp/stcp_rust_exported_functions.h>
#include <stcp/debug.h>

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

// Helppiä kun kaatuilee oudosti jne.
#define STCP_DEBUG_IO                       1
#define STCP_DEBUG_MAX_CONNECTIONS_DEBUG    0
#define STCP_DEBUG_MAX_CONNECTIONS_ALLOWED  4

#define STCP_RESET_WAIT_SERVICES_FOR_SECONDS    (5*60)

#define MUTEX_DO_CHECK_ASSERTS(ctx) \
    do {                                                                      \
        __ASSERT(ctx != NULL, "CTX NULL");                                    \
        __ASSERT((ctx)->lock.owner != (void *)0xffffffff, "Mutex corrupted"); \
    } while (0)

#define MUTEX_DO_CLOSING_CHECK(ctx)    \
    do {                                                        \
        LDBG("CTX %p state: closing=%lu ref=%lu",               \
              ctx,                                              \
              atomic_get(&(ctx)->closing),                      \
              atomic_get(&(ctx)->refcnt)                        \
        );                                                      \
        if (atomic_get(&(ctx)->closing)) {                      \
            LWRNBIG("Context %p is marked as closing!", ctx);   \
            return -ESHUTDOWN;                                  \
        }                                                       \
    } while (0)

#define NULL_CHECK_GUARD_CODE(val, CODE)                   \
    do {                                                   \
        if ((val) == NULL) {                               \
            LERRBIG("NULL CHECK FAILED!");                 \
        } else {                                           \
            CODE;                                          \
        }                                                  \
    } while (0)

#define __CTX_SOCK_LOCK(ctx)                   \
    do {                                       \
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


int stcp_new_context_with_fd(struct stcp_ctx **ctxSaveTo, 
                             int fd)
{
    
    if (ctxSaveTo == NULL) {
        LERR("No place for context to be saved.");
        return -EINVAL;
    }

    if (fd < 0) {
        LERR("No FD for context to be saved.");
        return -EINVAL;
    }

    LDBG("Going to stcp_create_new_context with fd %d", fd);
    struct stcp_ctx *ctx = stcp_create_new_context( fd );
    if (ctx) {
        *ctxSaveTo = ctx;
        LDBG("STCP Context created: %p", ctx);
        LDBG("Vielä ei tarvi olla apia..");
        stcp_debug_dump_stcp_ctx(ctx);
        return 0;
    }

    LDBG("STCP Context created failed");
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
        LDBG("[CTX] No place to put contex to..");
        return -EINVAL;
    }

    LDBG("[CTX] Waiting for mutex.....");
    k_mutex_lock(&g_mutex_new_context_create, K_FOREVER);
    LDBG("[CTX] Got lock!");
  
    int fd = STCP_SOCKET_OPEN(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    LDBG("[CTX] Got FD %d", fd);
    if (fd < 0) {
        k_mutex_unlock(&g_mutex_new_context_create);
        LDBG("No sock");
        return fd;
    }

    LDBG("[CTX] Getting new api tp %p with FD: %d", ctxSaveTo, fd);
    int rc = stcp_new_context_with_fd(ctxSaveTo, fd);
    LDBG("[CTX] New context: empty done: %d => %p with fd %d", rc, *ctxSaveTo, fd);
    if (rc < 0) {
        k_mutex_unlock(&g_mutex_new_context_create);
        LDBG("[CTX] New context: Not done");
        STCP_CLOSE_FD(fd);
        if (*ctxSaveTo) {
            LDBG("[CTX] New context: Closing %p", *ctxSaveTo);
            stcp_close(*ctxSaveTo);
        }
        LDBG("[CTX] No mana");
        return -ENOMEM;
    }
  
    if (! *ctxSaveTo ) {
        STCP_CLOSE_FD(fd);
        LDBG("New context: No context..OOM?");
        k_mutex_unlock(&g_mutex_new_context_create);
        LDBG("No mana");
        return -ENOMEM;
    }

    // Override FD (is -1)
    LDBG("OK path...");
    struct stcp_ctx *ctx = *ctxSaveTo;
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

    STCP_REF_COUNT_GET(ctx, "@ bind", return -EACCES; );

        STCP_DBG_CTX_FD(ctx);
        int ret = zsock_bind(ctx->ks.fd, addr, addrlen);

        if (ret < 0) {
            ret = -errno;
        }

    STCP_REF_COUNT_PUT(ctx, " @ end of bind");
    return ret;
}

int stcp_listen(struct stcp_ctx *ctx,
                int backlog)
{
    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        return -ENOTCONN;
    }
    
    STCP_REF_COUNT_GET(ctx, "@ listen", return -EACCES; );

        STCP_DBG_CTX_FD(ctx);
        int ret = zsock_listen(ctx->ks.fd, backlog);
        if (ret < 0) {
            ret = -errno;
        }

    STCP_REF_COUNT_PUT(ctx, "@ End of listen");
    return ret;
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

    STCP_REF_COUNT_GET(parent, "Accept: parent", return -EACCES; );
    
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
        STCP_REF_COUNT_GET(parent, "Accept: parent deref", return -EACCES; );
        return -errno;
    }

    struct stcp_ctx *child = NULL;

    // TÄRKEÄ
    int ret = stcp_new_context_with_fd(&child, new_fd);
    STCP_REF_COUNT_GET(child, "Accept: child", return -EBADFD; );
    STCP_DBG_CTX_FD(child);
    CTX_SOCK_LOCK(child);
    LDBG("Child lock GET");

    if (ret < 0) {
        STCP_CLOSE_FD(new_fd);
        if (child != NULL) {
            LDBG("Child lock PUT");
            CTX_SOCK_UNLOCK(child);
            STCP_REF_COUNT_PUT(child, "@ error, child EOL");

            LDBG("Parent lock PUT");
            CTX_SOCK_UNLOCK(parent);
            STCP_REF_COUNT_PUT(parent, "@ error, parent EOL?");
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
    STCP_REF_COUNT_PUT(child, "@ end of accept, child");

    LDBG("Parent lock PUT");
    CTX_SOCK_UNLOCK(parent);
    STCP_REF_COUNT_PUT(parent, "@ end of accept, parent");
    return 0;
}

int stcp_socket(int not_used_1,int not_used_2, int not_used_3) {

    int fd = STCP_SOCKET_OPEN(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

static int g_ctx_connect_done = 0;
int stcp_connect(struct stcp_ctx *ctx,
                 const struct sockaddr *addr,
                 socklen_t addrlen)
{
    if (!ctx) {
        LERR("CTX was null!");
        return -EINVAL;
    }
    STCP_REF_COUNT_GET(ctx, "@ connect", LERR("Access error"); return -EACCES; );

    if ( atomic_get(&ctx->closing) ) {
        LWRN("CTX already scheduled for cleanup");
        STCP_REF_COUNT_PUT(ctx, "@ connect: closing already");
        return -EBADFD;
    }

/*    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        return -ENOTCONN;
    }
  */  
    int fd = ctx->ks.fd;
    if (fd < 0) {
        LERR("Invalid FD: %d", fd);
        STCP_REF_COUNT_PUT(ctx, "@ connect: FD invalid");
        return -EBADF;
    }

#if STCP_DEBUG_MAX_CONNECTIONS_DEBUG
    LDBG("Checking max connections ...");
    if (g_ctx_connect_done > STCP_DEBUG_MAX_CONNECTIONS_ALLOWED) {
        LDBG("CTX Connected panic");
        k_panic();
    }
    g_ctx_connect_done++;
#endif

    CONTEXT_LOCK(ctx);

        LDBG("Doing connect with fd: %d from context %p (api %p)", 
            fd, ctx, ctx->api);
        STCP_DBG_CTX_FD(ctx);

        LDBG("Doing connect with fd: %d from context %p (api %p)", 
            fd, ctx, ctx->api);

        struct sockaddr_in *sin = (struct sockaddr_in *)addr;

        LDBG("CONNECT to ip=%d.%d.%d.%d port=%d addrlen=%d",
            sin->sin_addr.s4_addr[0],
            sin->sin_addr.s4_addr[1],
            sin->sin_addr.s4_addr[2],
            sin->sin_addr.s4_addr[3],
            ntohs(sin->sin_port),
            addrlen);

        LDBGBIG("Calling ZSOCK_CONNECT with fd: %d", fd);
        int rc = zsock_connect(fd, addr, addrlen);
        LDBGBIG("Called ZSOCK_CONNECT with fd: %d => %d", fd, rc);
        STCP_DBG_CTX_FD(ctx);

        if (rc < 0) {
            int err;
            socklen_t len = sizeof(err);

            zsock_getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
            LDBG("SO_ERROR[Ctx: %p FD: %d]: err=%d", ctx, fd, err);
            
            if (err == ESTALE) {
                LWRN("STCP: FD %d stale?", fd);
            }

            if (err != EINPROGRESS && err != ESTALE && err != 0) {
                LERR("Connect failed immediately err=%d", err);
                CONTEXT_UNLOCK(ctx);
                STCP_REF_COUNT_PUT(ctx, "@ connect: failed");
                return -err;
            }

            LDBG("Connect in progress...");
            rc = stcp_wait_for_connect(fd, 10000);

            if (rc < 0) {
                LERR("Connect wait failed rc=%d", rc);
                ctx->state = STCP_STATE_CLOSING;
                CONTEXT_UNLOCK(ctx);
                STCP_REF_COUNT_PUT(ctx, "@ connect: wait failed");
                return rc;
            }
        }

        LINF("TCP connect OK");

        LDBG("[Ctx: %p] Doing handshake FD: %d ...", ctx, ctx->ks.fd);
        ctx->state = STCP_STATE_CONNECTING;
        STCP_DBG_CTX_FD(ctx);

    CONTEXT_UNLOCK(ctx);

    rc = stcp_handshake_for_context(ctx);
    
    if (rc == 1) {
        ctx->state = STCP_STATE_ESTABLISHED;
        LDBG("[Ctx: %p / FD: %d] State set: ESTABLISHED", ctx, ctx->ks.fd);
    } else {
        if (rc == -EALREADY) {
            ctx->state = STCP_STATE_ESTABLISHED;
            LDBG("[Ctx: %p / FD: %d] State already done handshake => ESTABLISHED", ctx, ctx->ks.fd);
        } else {
            ctx->state = STCP_STATE_CONNECTING;
            LDBG("[Ctx: %p / FD: %d] State set: CONNECTING", ctx, ctx->ks.fd);
        }
    }
    LDBG("Got rc: %d", rc);
 
    STCP_REF_COUNT_PUT(ctx, "@ End of connect");
    return rc;
}



ssize_t stcp_send(struct stcp_ctx *ctx, const void *buf, size_t len, int flags)
{
    STCP_REF_COUNT_GET(ctx, "@ send", return -EACCES; );

    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        STCP_REF_COUNT_PUT(ctx, "@ send: Failed, no valid context");
        return -ENOTCONN;
    }

    LDBG("Context at %s: %p // HS Done: %d // FD: %d",
        __func__, ctx, ctx->handshake_done, ctx->ks.fd
    );

    if (ctx->state != STCP_STATE_ESTABLISHED) {
        LDBG("Context %p at sttate %d tried to send..", ctx, ctx->state);
        STCP_REF_COUNT_PUT(ctx, "@ send: Failed, invalid state");
        return -ENOTCONN;
    }

    ARG_UNUSED(flags);
    STCP_DBG_CTX_FD(ctx);

    LDBG("STCP: Send locking ....");
    //CONTEXT_LOCK(ctx);
    LDBG("STCP: Send locked ....");
        ssize_t ret = stcp_transport_send(ctx, buf, len);
    //CTX_SOCK_UNLOCK(ctx);
    STCP_REF_COUNT_PUT(ctx, "@ end of send");
    return ret;
}

ssize_t stcp_send_msg(struct stcp_ctx *ctx, const struct msghdr *message)
{
    STCP_REF_COUNT_GET(ctx, "@ sendmsg", return -EACCES; );

    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        STCP_REF_COUNT_PUT(ctx, "@ sendmsg: Not valid context");
        return -ENOTCONN;
    }

    LDBG("Context at %s: %p // HS Done: %d // FD: %d",
        __func__, ctx, ctx->handshake_done, ctx->ks.fd
    );

    if (ctx->state != STCP_STATE_ESTABLISHED) {
        LDBG("Context %p at state %d tried to send..", ctx, ctx->state);
        STCP_REF_COUNT_PUT(ctx, "@ sendmsg: Invalid state");
        return -ENOTCONN;
    }

    STCP_DBG_CTX_FD(ctx);
    //CONTEXT_LOCK(ctx);
        ssize_t ret = stcp_transport_send_iovec(ctx, message);
    //CTX_SOCK_UNLOCK(ctx);

    STCP_REF_COUNT_PUT(ctx, "@ end of sendmsg");
    return ret;
}


ssize_t stcp_recv(struct stcp_ctx *ctx, void *buf, size_t len, int flags)
{
    ARG_UNUSED(flags);

    STCP_REF_COUNT_GET(ctx, "@ recv", return -EACCES; );


    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        STCP_REF_COUNT_PUT(ctx, "@ recv: Not valid context");
        return -ENOTCONN;
    }


    LDBG("Context at %s: %p // HS Done: %d // FD: %d",
        __func__, ctx, ctx->handshake_done, ctx->ks.fd
    );

    if (ctx->state != STCP_STATE_ESTABLISHED) {
        LDBG("Context %p at sttate %d tried to recv..", ctx, ctx->state);
        STCP_REF_COUNT_PUT(ctx, "@ recv: Not valid state");
        return -ENOTCONN;
    }

    STCP_DBG_CTX_FD(ctx);
    //CONTEXT_LOCK(ctx);
        ssize_t ret = stcp_transport_recv(ctx, buf, len);
    //CTX_SOCK_UNLOCK(ctx);

    STCP_REF_COUNT_PUT(ctx, "@ end of recv");
    return ret;
}

int stcp_set_non_bloking_to(struct stcp_ctx *ctx, int val)
{

    STCP_REF_COUNT_GET(ctx, "@ non blocking set", return -EACCES; );

    if (stcp_is_context_valid(ctx) < 0) {
        LERR("Not valid context, ctx: %p errno: %d", ctx, errno);
        STCP_REF_COUNT_PUT(ctx, "@ non blocking set");
        return -ENOTCONN;
    }

    int fd = ctx->ks.fd;

    LINF("[Ctx: %p] Setting nonblocking %d for fd: %d...", ctx, val, fd);
    int ret = zsock_ioctl(fd, ZFD_IOCTL_FIONBIO, &val);

    STCP_REF_COUNT_PUT(ctx, "@ end of non blocking set");

    return ret;

}

int stcp_close(struct stcp_ctx *ctx) { 
    LDBG("STCP: stcp_close called from:");
    stcp_dump_bt();
    if (stcp_is_context_valid(ctx) < 0) { 
        LERR("Not valid context or closing, ctx: %p errno: %d",
            ctx, errno); return -ENOTCONN;
    }

    // Pidetään ref tuhoamisen liipasun yli
    STCP_REF_COUNT_GET(ctx, "@ CTX close", return -EACCES; );

        if (ctx->state == STCP_STATE_CLOSING) {
            STCP_REF_COUNT_PUT(ctx, "@ CTX close"); 
            return 0;
        }
        
        LDBG("STCP: Setting contexxt %p as closing..", ctx);
        ctx->state = STCP_STATE_CLOSING; 

        LDBG("Closing context called from %p: %p // HS Done: %d // FD: %d",
            __builtin_return_address(0), ctx, ctx->handshake_done, ctx->ks.fd ); 
        STCP_DBG_CTX_FD(ctx);
        stcp_transport_close(ctx); // TÄMÄ HOITAA KAIKEN... 
    // Poistetaan safe guardi, nyt contextin refcnt pitää olla 1
    
    STCP_REF_COUNT_PUT(ctx, "@ CTX close"); 
    LDBG("[Ctx %p] Final refcount at close: %d", ctx, (int)atomic_get(&ctx->refcnt));
    return 0;
}

int stcp_is_context_open_for_fd_io(struct stcp_ctx *ctx) {
    if (!ctx) return -EINVAL;

    if (
        atomic_get(&ctx->closing) ||
        !atomic_get(&ctx->allow_api_access) ||
        ctx->doing_replace
    ) {
        LDBG("STCP: IO Operations NOT ALLOWED..");
#if STCP_DEBUG_IO
        stcp_dump_bt();
#endif
        return -ECANCELED;
    }
    return 0;
}

int stcp_replace_context(struct stcp_ctx **ctxFrom) { 
    if (ctxFrom == NULL) {
        LERR("STCP: Replace context, no save point.");
        return -ENODEV;
    }

    if (*ctxFrom == NULL) {
        LERR("STCP: Replace context, no old.");
        return -ENODEV;
    }

    struct stcp_ctx *ctxOld = *ctxFrom;
    LDBG("STCP: stcp_replace_context called from:");
    stcp_dump_bt();

    int oldFD = ctxOld->ks.fd;
    if (oldFD < 0) {
        LERR("STCP: Replace context, no valid FD.");
        return -EBADFD;
    }

    LDBG("STCP: Replace: Marking contest to be replaced!");
    atomic_set(&ctxOld->closing, 1);
    atomic_set(&ctxOld->allow_api_access, 0);

    // Flag to set FD NOT TO BE CLOSED at cleanup ...
    ctxOld->doing_replace = 1;
    stcp_close(ctxOld);

    stcp_api_init_with_fd(ctxFrom, oldFD);
    LDBG("STCP: Replace: Context replaced from %p (FD: %d) => %p (FD: %d)", 
        ctxOld, ctxOld->ks.fd,
        (*ctxFrom), (*ctxFrom)->ks.fd
     );
    
    return 0;
}
