#include <zephyr/kernel.h>
#include <stdlib.h>

#define STCP_SOCKET_INTERNAL 1
#include <stcp/stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/stcp_lte.h>
#include <stcp/lte_workers.h>
#include <stcp/stcp_transport.h>
#include <stcp/low_level_pointer.h>
#include <stcp/low_level_api_context_tracking.h>
#include <stcp/low_level_refcount_tracker.h>
#include <stcp/sanity.h>

#define STCP_RESET_WAIT_SERVICES_FOR_SECONDS (5*60)

#define STCP_TRY_ONLY_ONLY_CONNECT_ROUND 0

#define API_CONTEXT_REF_GET(api, name, FAILCODE) \
    if (api) { STCP_REF_COUNT_GET(VOID_TO_API(api)->ctx, "@ " name, FAILCODE; ); }

#define API_CONTEXT_REF_PUT(api, name) \
    if (api) { STCP_REF_COUNT_PUT(VOID_TO_API(api)->ctx, "@ End of " name ); }

//    LDBG("API: API %p Entering locked state (%s)", api, name);
#define ENTER_LOCKED(api, name) \
    API_CONTEXT_REF_GET(api, "@ Get: " name, LDBG("Got no ref for %s", name); return -EACCES; ); \
        API_LOCK(api, "@ Lock: " name );

#define EXIT_LOCKED(api, name) \
        API_UNLOCK(api, "@ Unlock: " name );    \
    API_CONTEXT_REF_PUT(api, "@ Put: " name); 
    //LDBG("API: API %p Exit locked state (%s)", api, name);

extern atomic_t stcp_context_alive_count;

int stcp_api_replace_fd(struct stcp_api *api, int fd) {
    if (!api) {
        return -EINVAL;
    }

    // LDBG("[API %p] Switching to FD: %d", api, fd);
    struct stcp_ctx* ctx = VOID_TO_CTX(api->ctx);
    int rv = 0;
    if (ctx) {
        STCP_REF_COUNT_GET(ctx, "replace FD of API", LDBG("Could not get ref.."); return -EACCES; );
            if (ctx->ks.fd >= 0) {
                // LDBG("[API %p] Closing fd: %d", api, ctx->ks.fd);
                STCP_SOCKET_CLOSE(ctx->ks.fd);
            }
            ctx->ks.fd = fd;
            rv = 1;
        STCP_REF_COUNT_PUT(ctx, "@ end of replace FD");
    }
    return rv;
}

int stcp_api_connection_set_as_connected(struct stcp_api *api) {
    if (!api) {
        return -EINVAL;
    }

    // LDBG("[API] Setting API %p as connected", api);
    struct stcp_ctx *ctx = api->ctx;
    stcp_debug_dump_stcp_ctx(ctx);
    
    if (ctx) {
        atomic_set(&ctx->connected, 1);
        ctx->state = STCP_STATE_ESTABLISHED;
    }
    atomic_set(&api->connect_in_progress, 0);
    api->connected = 1;
    k_sem_give(&api->connected_sem);
}

int stcp_api_connection_wait_for_hanshake(struct stcp_api *api, int timeout_ms) {
    if (!api) {
        return -EINVAL;
    }

    // LDBG("[API] API %p starting to wait for hanshake complete....", api);
    struct stcp_ctx *ctx = api->ctx;
    if (ctx) {
        return stcp_wait_for_hanshake_done(ctx, timeout_ms);
    }
    return -EINVAL;
}

int stcp_api_connection_state_init(struct stcp_api *api) {
    if (!api) {
        return -EINVAL;
    }

    // LDBG("[API] Resetting API %p connection status", api);
    struct stcp_ctx *ctx = api->ctx;
    if (ctx) {
        atomic_set(&ctx->connected, 0);
    }
    api->connected = 0;
    k_sem_init(&api->connected_sem, 0, 1);
}


int stcp_api_get_connect_in_progress(struct stcp_api *api) {
    if (!api || !api->ctx) {
        return -EINVAL;
    }
    int cip = atomic_get(&api->connect_in_progress);
    LDBG("STCP: Got connection in progress flag set as: %s",
        GET_YES_NO_STR(cip)
    );

    int state_ok = api->ctx->state == STCP_STATE_ESTABLISHED;

    LDBG("STCP: Got connection status as establlished: %s",
            GET_YES_NO_STR(state_ok)
        );

    int rv = cip && state_ok;

    LDBG("STCP: Got connection in progress: %s",
        GET_YES_NO_STR(rv)
    );
    return rv;
}

int stcp_api_init(struct stcp_api **api)
{
    // LDBG("[API] At init: %p, API size %d bytes", api, sizeof(struct stcp_api));
    // LDBG("[API] Init called from:");
    stcp_dump_bt();

    if (!api) {
        return -EINVAL;
    }
    if (*api) {
        LERR("NOT NULL point");
        stcp_dump_bt();
        return -EINVAL; 
    } else {
        // LDBG("Points to null, OK!");
    }

    struct stcp_api *inst = STCP_MEMORY_ALLOC(sizeof(struct stcp_api));
    // LDBG("[API] New: %p", inst);
    if (!inst) {
        LERR("[API] OOM, no mana left!");
        return -ENOMEM;
    }

    inst->magic         = STCP_CTX_MAGIC_ALIVE;
    inst->magic_footer  = STCP_CTX_MAGIC_ALIVE_FOOTER;

    if ( stcp_mutex_init(&inst->lock) < 0 ) {
        STCP_MEMORY_DEALLOC(inst);
        return -ENODEV;
    }

    struct stcp_ctx *newContext = NULL;
    int ret = stcp_new_context(&newContext);
    // LDBG("[API] New context done: %p, ret: %d",newContext, ret);
    if (ret < 0) {
        STCP_MEMORY_DEALLOC(inst);
        return -ENOBUFS;
    }

    if (newContext == NULL) {
        STCP_MEMORY_DEALLOC(inst);
        return -ENOBUFS;
    }

    // LDBG("[API] Cross linked API(%p) <=>  CTX(%p)", inst, newContext);
    newContext->api = inst;
    inst->ctx = newContext;

    LDBG("[API] Cross linked API->ctx=%p <=> CTX->api=%p now",
        inst->ctx, newContext->api);

    STCP_REF_COUNT_GET(newContext, "at stcp api init", LDBG("Not able to get ref at init.."); return -EACCES;);

    atomic_inc(&stcp_context_alive_count);
    atomic_set(&inst->alive, 1);
    atomic_set(&inst->connected, 0);
    atomic_set(&inst->connect_in_progress, 0);
    
    inst->nonblocking = 0;
    inst->connack_seen = 0;
    inst->connack_reset_done = 0;

    // LDBG("[API] Setting connack sema & int");
    inst->connack_seen = 0;
    k_sem_init(&inst->connack_sem, 0, 1);

    stcp_api_connection_state_init(inst);

    // LDBG("[API] Setting stuff....");
    STCP_API_CONTEXT_TRACK(inst);

    // LDBG("[API] @ Save %p", inst);
    *api = inst;
    // LDBG("[API] %p saved", *api);
/*
    LDBG("[API] Before sanity: api=%p ctx=%p state=%d ref=%d",
        inst,
        inst->ctx,
        inst->ctx ? inst->ctx->state : -1,
        inst->ctx ? inst->ctx->refcnt : -1);
*/
    int rc = stcp_api_sanity_check(inst, STCP_SANITY_API_LINKED);
    // LDBG("Sanity checks of fresh API: %d", rc);
    if (rc < 0) {
        LWRN("Sanity checks failed, rc: %d", rc);
    }


    // LDBG("[API] Allocating STCP FSM ....");
    struct stcp_fsm *newFSM = k_malloc(sizeof(*newFSM));
    memset(newFSM, 0, sizeof( *newFSM));

    if (newFSM) {
        //LINF("[API] Doing FSM %p init..", newFSM);
        stcp_fsm_init(newFSM);
        inst->fsm = newFSM;
    } else {
        LERR("[API] Not able to allocate FSM!");
        k_panic();
    }

    LINF("[API] FSM %p start for API %p", newFSM, inst);
    stcp_fsm_instance_start(newFSM, inst);

    // LDBG("[API] All for Api done.");
    stcp_debug_dump_stcp_ctx(inst->ctx);
/*
    // LDBG("Sanity checks of fresh API %p:", inst);
    // LDBG("[API] Sanity check of fresh API Check A: %d", stcp_api_is_alive(inst));
    // LDBG("[API] Sanity check of fresh API Check B: %d", stcp_api_is_valid(inst));
    // LDBG("[API] Sanity check of fresh API Check C: %d", stcp_api_is_usable(inst));
*/

    return 0;
}

int stcp_api_init_with_fd(struct stcp_api **api, int fd)
{
    if (!api) {
        LERR("No place to put");
        return -EINVAL;
    }

    if (fd < 0) {
        LERR("Invalid fd in api creation");
        return -EBADF;
    }

    struct stcp_api *newApi = NULL;
    // Tekee reffin
    int rc = stcp_api_init(&newApi);
    if (rc < 0) {
        if (newApi) {
            stcp_api_close(newApi);
        }
        return rc;
    }
    
    rc = stcp_api_replace_fd(newApi, fd);

    if (rc < 0) {
        if (newApi) {
            stcp_api_close(newApi);
        }
        return rc;
    }

    if (api) {
        // LDBG("Storing API handle..");
        *api = newApi;
    }

    // LDBG("API handle with FD, done!");
    return 0;
}

int stcp_api_resolve(const char *host, const char *port, struct zsock_addrinfo **result) {
    return stcp_util_hostname_resolver(host, port, result);
}

int stcp_api_connect(struct stcp_api *api,
                     const struct zsock_addrinfo *addr,
                     socklen_t addrlen)
{
    ENTER_LOCKED(api, "Connecting");
        int rv = stcp_connect(api->ctx, (struct sockaddr *)addr, addrlen);
        if (rv == 0) {
#if STCP_TRY_ONLY_ONLY_CONNECT_ROUND
            if (g_api_connect_done > 0) {
                // LDBG("API Connected panic");
                k_panic();
            }

            g_api_connect_done++;
#endif // STCP_TRY_ONLY_ONLY_CONNECT_ROUND
        }
    EXIT_LOCKED(api, "Connecting");

    return rv;
}

int stcp_api_set_io_timeout(struct stcp_api *api, int timeout_ms)
{
    if (!api) {
        return -EINVAL;
    }
    ENTER_LOCKED(api, "Timeout set");
            int fd = stcp_api_get_fd(api);
            LINF("Setting timeout for IO to %d msec", timeout_ms);
            int rv = stcp_tcp_timeout_set_to_fd(fd, timeout_ms);
            // LDBG("Setting ret: %d", rv);
    EXIT_LOCKED(api, "Timeout set");
    return rv;
}

int stcp_api_connection_reset(struct stcp_api *api)
{
    if (!api) {
        return -EINVAL;
    }
    int ret = -ENOBUFS;
    ENTER_LOCKED(api, "Connection reset");

            if (api->ctx) {
                ret = stcp_lte_do_full_reset(api->ctx, 0);

                // LDBG("Forcing reattach....");
                (void)stcp_lte_issue_at_command("AT+CFUN=0");
                (void)stcp_lte_issue_at_command("AT+CFUN=1");

                LINFBIG("Full reset completed!");
            }

    EXIT_LOCKED(api, "Connection reset");
    return ret;
}

ssize_t stcp_api_send(struct stcp_api *api,
                       const void *buf,
                       size_t len,
                       int flags)
{

    ssize_t ret = 0;

    ENTER_LOCKED(api, "send");
 
        int access_ok = stcp_api_is_open_for_fd_io(api);

        if (access_ok < 0) {
           // LWRN("API CALL: SEND: Fail: Access denied.");
            return access_ok;
        } else {
            // LDBG("API CALL: SEND");
            ret = stcp_send(api->ctx, buf, len, flags);
            // LDBG("API CALL: SEND, ret %d, errno=%d", (int)ret, errno);
        }

    EXIT_LOCKED(api, "send");

    if (ret < 0) {
        if (ret == -ECONNRESET || ret == -EPIPE) {
                LERR("Connection lost => returning -ENOTCONN");
                ret = -ENOTCONN;
        }
    }

    // LDBG("STCP: stcp_api_send ret: %d", ret);
    return ret;
}

ssize_t stcp_api_recv(struct stcp_api *api,
                      void *buf,
                      size_t len,
                      int flags
                    )
{

    ssize_t ret = 0;
    ENTER_LOCKED(api, "recv");

        int access_ok = stcp_api_is_open_for_fd_io(api);

        if (access_ok < 0) {
            //LWRN("API CALL: RECV: Fail: Access denied.");
            return access_ok;
        } else {
            flags |= api->nonblocking ? ZSOCK_MSG_DONTWAIT : 0;
            // LDBG("API CALL: RECV");
            ret = stcp_recv(api->ctx, buf, len, flags);
            // LDBG("API CALL: RECV, ret: %d", (int) ret );
        }

    EXIT_LOCKED(api, "recv");
    
    if (ret < 0) {
        if (ret == -ECONNRESET || ret == -EPIPE) {
                LERR("Connection lost => returning -ENOTCONN");
                ret = -ENOTCONN;
        }
    }

    // LDBG("STCP: stcp_api_recv ret: %d", ret);
    return ret;
}

int stcp_api_close(struct stcp_api *api)
{
    // LDBG("STCP: stcp_api_close called from:");
    stcp_dump_bt();
    
    if (!api) {
        LERR("STCP: Close got no api");
        return -EBADFD;
    }

    if (!api->ctx) {
        LERR("STCP: Close got no context");
        return -EBADFD;
    }

    if (!api->ctx->handshake_done) {
        LDBGBIG("STCP: Close called before HS complete, from:");
        stcp_dump_bt();
        return -EAGAIN;
    }

    // LDBG("STCP: Closing API: %p with context %p", api, api->ctx);

    ENTER_LOCKED(api, "recv");
        LWRNBIG("Set API %s as dead, and disabling access.", api);
        atomic_set(&api->ctx->allow_api_access, 0);
        atomic_set(&api->alive, 0);
        int ret = stcp_close(api->ctx);
        // LDBG("API: Closing, rc: %d", ret);
    EXIT_LOCKED(api, "recv");
    return ret;
}

int stcp_api_reset(struct stcp_api *api)
{
    if (!api) {
        return -EBADFD;
    }

    if (!api->ctx) {
        return -EBADFD;
    }
    
    ENTER_LOCKED(api, "Full reset");
            stcp_do_full_reset(api->ctx);
    EXIT_LOCKED(api, "Full reset");
    return 0;
}

int stcp_api_accept(struct stcp_api *api,
                    struct stcp_api **new_api,
                    struct zsock_addrinfo *peer,
                    socklen_t *peer_len)
{
    //LDBGBIG("STCP: Accept called from:");
    //stcp_dump_bt();

    struct stcp_ctx *child = NULL;

    ENTER_LOCKED(api, "Accept");
            int ret = stcp_accept(api->ctx, &child, (struct sockaddr *)peer, peer_len);
            if (ret < 0) {
                API_UNLOCK(api, __func__);
                API_CONTEXT_REF_PUT(api, "Accept error");
                return ret;
            }

            struct stcp_api *inst = STCP_MEMORY_ALLOC(sizeof(struct stcp_api));
            if (!inst) {
                API_UNLOCK(api, __func__);
                API_CONTEXT_REF_PUT(api, "No mem");
                return -ENOMEM;
            }

            inst->ctx = child;
            *new_api = inst;

    EXIT_LOCKED(api, "Accept");
    return 0;
}

ssize_t stcp_api_sendmsg(struct stcp_api *api,
                         const struct msghdr *msg)
{
    if (!api || !msg) {
        return -EINVAL;
    }

    ENTER_LOCKED(api, "SendMSG");
            int rv = stcp_send_msg(api->ctx, msg); 
    EXIT_LOCKED(api, "SendMSG");

    return rv;
}


int stcp_api_poll(struct stcp_api *api,
                  int events,
                  int timeout_ms,
                  int *revents)
{
    if (!api || !revents) {
        return -EINVAL;
    }

    ENTER_LOCKED(api, "Poll: FD");
            int fd = api->ctx->ks.fd;
    EXIT_LOCKED(api, "Poll: FD");
            
        struct zsock_pollfd pfd = {
            .fd = fd,
            .events = events,
            .revents = 0,
        };
        
        *revents = 0;
        int ret = zsock_poll(&pfd, 1, timeout_ms);
        if (ret < 0) {
            return ret;   // error
        }

        // Timeout ?

        *revents = pfd.revents;

    return ret;      // number of ready fds (POSIX style)
}

int stcp_api_get_dns_info(struct stcp_api *api, struct zsock_addrinfo **res)
{
    if (api && api->ctx) {
        ENTER_LOCKED(api, "Resolve DNS");
                struct zsock_addrinfo * dns = api->ctx->dns_resolved;
                if (res) {
                    *res = dns;
                    return 0;
                }
        EXIT_LOCKED(api, "Resolve DNS");
    }
    return -EBADFD;
}

int stcp_api_get_fd(struct stcp_api *api)
{
    if (api && api->ctx) {
        ENTER_LOCKED(api, "Get FD");
                int fd = api->ctx->ks.fd;
        EXIT_LOCKED(api, "Get FD");
        return fd;
    }
    return -EBADFD;
}

int stcp_api_is_valid(struct stcp_api *api)
{
    int rv = 0;

    int pv = stcp_api_pointer_valid(api);

    if (pv && api && api->ctx) {
        //ENTER_LOCKED(api, "Is valid");
                rv = stcp_is_context_valid(api->ctx);
        //EXIT_LOCKED(api, "Is valid");
        return rv;
    }
    return -EBADFD;
}

int stcp_api_is_usable(struct stcp_api *api)
{
    if (!api) {
        return 0;
    }

    struct stcp_ctx *ctx = api->ctx;

    if (!ctx) {
        return 0;
    }

    if (!stcp_api_is_alive(api)) {
        return 0;
    }

    if (!ctx->handshake_done) {
        // LDBG("CTX %p handshake not done", ctx);
        return 0;
    }

    if (ctx->state != STCP_STATE_ESTABLISHED) {
       /* LDBG(
            "CTX %p not established yet state=%d",
            ctx,
            ctx->state
        );*/
        return 0;
    }

    int closing =
        GET_ATOMIC_VALUE_FROM(ctx->closing);

    int destroyed =
        GET_ATOMIC_VALUE_FROM(ctx->destroyed);

    if (closing || destroyed) {
        return 0;
    }

    return 1;
}

int stcp_api_wait_until_connected_to_peer_no_lock(struct stcp_api *api, int timeout_ms)
{
    LINFBIG("STCP: Waiting until API %p connected (NO API LOCK), Max %d ms", api, timeout_ms);
    int ret = -ENODEV;
    if (api) {
        if (! atomic_get(&api->connected) ) {
            // LDBG("STCP: Taking connected_sem...");
            ret = k_sem_take(&api->connected_sem, K_MSEC(timeout_ms));
            // LDBG("STCP: Taking connected_sem...");
        } else {
            // LDBG("STCP: Already connected...no need to wait.");
            ret = 0;
        }
    } else {
        LERRBIG("No API for waiting connection!");
    }
    // LDBG("STCP: Wait ret: %d", ret);
    return ret;
}


int stcp_api_wait_until_connected_to_peer(struct stcp_api *api, int timeout_ms)
{
    LDBGBIG("STCP: Waiting until API %p connected, Max %d ms", api, timeout_ms);
    int ret = -ENODEV;
    if (api) {
        if (! atomic_get(&api->connected) ) {
            // LDBG("STCP: Taking connected_sem...");
            ret = k_sem_take(&api->connected_sem, K_MSEC(timeout_ms));
            // LDBG("STCP: Taking connected_sem...");
        } else {
            // LDBG("STCP: Already connected...no need to wait.");
        }
    } else {
        LERRBIG("No API for waiting connection!");
    }
    return ret;
}

int stcp_api_wait_until_modem_lte_ready(struct stcp_api *api, int timeout_ms) {
    int rv = 0;
    if (api) {
        rv = stcp_fsm_wait_until_reached_lte_ready(api->fsm, timeout_ms);
        return rv;
    }
    return -EBADFD;
}

int stcp_api_wait_until_modem_pdn_ready(struct stcp_api *api, int timeout_ms) {
    int rv = 0;
    if (api && api->ctx) {
        ENTER_LOCKED(api, "Wait: PDN ready");
                rv = stcp_fsm_wait_until_reached_pdn_ready(api->fsm, timeout_ms);
        EXIT_LOCKED(api, "Wait: PDN ready");
        return rv;
    }
    return -EBADFD;
}

int stcp_api_wait_until_modem_ip_network_up(struct stcp_api *api, int timeout_ms) {
    int rv = 0;
    if (api) {
        ENTER_LOCKED(api, "Wait: IP ready");
                rv = stcp_fsm_wait_until_reached_ip_network_up(api->fsm, timeout_ms);
        EXIT_LOCKED(api, "Wait: IP ready");
        return rv;
    }
    return -EBADFD;
}

int stcp_api_set_nonblocking(struct stcp_api *api,
                             bool enable)
{
    if (!api) {
        return -EINVAL;
    }
    ENTER_LOCKED(api, "Set nonbloking");
            api->nonblocking = (enable) ? 1 : 0;
            int rv = stcp_set_non_bloking_to(api->ctx, api->nonblocking);
    EXIT_LOCKED(api, "Set nonbloking");
    return rv;
}

