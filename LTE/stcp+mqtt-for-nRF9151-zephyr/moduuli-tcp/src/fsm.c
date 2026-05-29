#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <stdbool.h>
#include "stcp/fsm.h"
#include "stcp/stcp_lte_fsm.h"

void stcp_fsm_start(struct stcp_fsm *fsm);
void stcp_fsm_notify_lte_ready(struct stcp_fsm *fsm);
void stcp_fsm_notify_pdn_ready(struct stcp_fsm *fsm);

#include <stcp/stcp_api.h>
#include <stcp/fsm.h>
#include <stcp/settings.h>
#include <stcp/debug.h>
#include <stcp/stcp_alloc.h>
#include <stcp/stcp_struct.h>
#include <stcp/utils.h>
#include <stcp/stcp_net.h>
#include <stcp/workers.h>
#include <stcp/utils.h>
#include <stcp/stcp_operations_zephyr.h>
#include <stcp/stcp_rust_exported_functions.h>
#include <stcp/stcp_transport_api.h>
#include <stcp/stcp_transport.h>
#include <stcp/dns.h>

#include <testing/include/stcp_testing.h>

#define STCP_FSM_REAL_IMPL 1

// Ei olemassa per yhteys vaan globaaleina, modeemin tilaa tämä.
struct k_sem g_sem_lte_ready;
struct k_sem g_sem_pdn_ready;
struct k_sem g_sem_ip_ready;
struct k_sem g_sem_radio_ready;

#define STCP_MAX_HANDSHAKE_TRY_COUNT               6000
#define STCP_MAX_HANDSHAKE_TRY_SLEEP_MS            10
#define STCP_MAX_HANDSHAKE_TRY_SLEEP_JITTER_MS     10

#define STCP_LOG_REFERENCES(ctx) \
    do {                                                    \
        LDBG("Context %p has %d references open..",         \
            ctx,                                            \
            stcp_context_lifespan_get_span(theContext)      \
        );                                                  \
    } while(0)

#define STCP_FSM_LOCK_CONTEXT(ctx, FAILCODE) \
    {                                                                       \
        int __to = -1;                                                      \
        if (ctx) {                                                          \
            __to = k_mutex_lock(&(VOID_TO_CTX(ctx)->lock), K_MSEC(5000));   \
        }                                                                   \
        if (__to != 0) {                                                    \
            LWRNBIG("FSM %p: LOCK Timeout? %d.. while in state: %d",        \
                    fsm, __to, fsm->state);                                 \
            ctx_locked = 0;                                                 \
            FAILCODE;                                                       \
        }                                                                   \
        ctx_locked = 1;                                                     \
    }


int stcp_fsm_check_if_access_is_granted(struct stcp_api* pAPI) {

    if (!pAPI) {
        return 0;
    }

    struct stcp_ctx* ctx = pAPI->ctx;

    STCP_REF_COUNT_GET(ctx, "@ access granted", LDBG("Got no ref"); return -EACCES; );

    LINFBIG("@ALLOWED CTX STATE: ctx=%p closing=%d destroyed=%d ref=%d",
            ctx,
            ctx ? GET_ATOMIC_VALUE_FROM(ctx->closing) : -1,   
            ctx ? GET_ATOMIC_VALUE_FROM(ctx->destroyed) : -1, 
            ctx ? GET_ATOMIC_VALUE_FROM(ctx->refcnt) : -1    
        );

    int api_is_usable = stcp_api_is_usable(pAPI);
    CONTEXT_SET_API_ACCESS_FLAG(VOID_TO_API(pAPI)->ctx, api_is_usable); 
    LDBG("[API %p] Access allowed: %s", pAPI, GET_YES_NO_STR(api_is_usable));
    STCP_REF_COUNT_PUT(ctx, "@ access granted");
    return api_is_usable;
}

void stcp_fsm_reached_ip_network_up(struct stcp_fsm *fsm) {
    LDBG("Reached Network UP....");
    atomic_set(&g_ip_active, 1);
    k_sem_give(&g_sem_ip_ready);
}

void stcp_fsm_reached_lte_ready(struct stcp_fsm *fsm) {
    LDBG("Reached LTE ready....");
    k_sem_give(&g_sem_lte_ready);

}


void stcp_fsm_reached_pdn_ready(struct stcp_fsm *fsm) {
    LDBG("Reached PDN ready....");
    atomic_set(&g_pdn_active, 1);
    k_sem_give(&g_sem_pdn_ready);
}

 
/*
#define STCP_SEMA_TAKE(sema, atom) \
    do {                                                        \
        int __sema_rc = -1;                                     \
        LERR(                                                   \
            "SEMA WAIT START %s flag=%d fsm_state=%d",          \
            #sema,                                              \
            atomic_get(&(atom),                                 \
            fsm->state                                          \
        );                                                      \
        if (atomic_get(&(atom)) == 0) {                         \
            LDBG("Starting to wait %s", #sema);                 \
            __sema_rc = k_sem_take(&(sema), K_FOREVER);         \
            if (__sema_rc == 0) {                               \
                LDBG("Setting flag true for %s", #sema);        \
                atomic_get(&(atom), 1);                         \
            }                                                   \
        } else {                                                \
            LDBG("Skipped wait for %s, cause atomic %s set",    \
                #sema, #atom);                                  \
        }                                                       \
        LERR(                                                   \
            "SEMA WAIT DONE %s rc=%d fsm_state=%d",             \
            #sema,                                              \
            __sema_rc,                                          \
            fsm->state                                          \
        );                                                      \
    } while(0)
*/
int stcp_fms_state_change_validation(struct stcp_fsm *fsm, stcp_fsm_state_t newstate) {
    // Checkataan handshake state
    if (fsm->state == STCP_FSM_STCP_HANDSHAKE) {
        int ignore = 0;
            ignore = newstate == STCP_FSM_TCP_RECONNECT;

        if (ignore) {
            LDBG("[FSM] Ignoring state change...");
            return -EACCES;
        }
    }

    return 1;
}


#define STCP_SEMA_TAKE_TIMEOUT_SECONDS  60
#define STCP_SEMA_TAKE_TIMEOUT_VALUE    K_SECONDS(STCP_SEMA_TAKE_TIMEOUT_SECONDS)

#if STCP_FSM_VERBOSE
#define STCP_SEMA_TAKE(sema, atom, fsm, gotoLabel, failstate) \
    do {                                                                                              \
        LINF(                                                                                         \
            "SEMA WAIT START %s flag=%d fsm_state=%d",                                                \
            #sema,                                                                                    \
            atomic_get(&(atom)),                                                                      \
            fsm->state                                                                                \
        );                                                                                            \
        if (fsm->stop) {                                                                              \
            LDBG("STCP/FSM: Stop requested!");                                                        \
            goto gotoLabel;                                                                           \
        }                                                                                             \
        int __rc = 0;                                                                                 \
        LDBG("Checking %s flag...", #sema);                                                           \
        if (atomic_get(&(atom)) == 0) {                                                               \
            LDBG("Starting to wait %s for %d seconds", #sema, STCP_SEMA_TAKE_TIMEOUT_SECONDS);        \
            __rc = k_sem_take(&(sema), STCP_SEMA_TAKE_TIMEOUT_VALUE);                                 \
        }                                                                                             \
        if (fsm->stop) {                                                                              \
            LDBG("STCP/FSM: Stop requested!");                                                        \
            goto gotoLabel;                                                                           \
        }                                                                                             \
        LINF(                                                                                         \
            "SEMA WAIT DONE %s rc=%d fsm_state=%d",                                                   \
            #sema,                                                                                    \
            __rc,                                                                                     \
            fsm->state                                                                                \
        );                                                                                            \
        if (__rc < 0) {                                                                               \
            LDBG("Wait %s timedout for.. %d seconds passed.", #sema, STCP_SEMA_TAKE_TIMEOUT_SECONDS); \
            STCP_STCP_FSM_STATE_SET(fsm, failstate);                                                                   \
            goto gotoLabel;                                                                           \
        }                                                                                             \
    } while(0)


#define FSM_STATE_START(ctx, sta)  \
    LDBG("[FSM(%p)] Start of case " #sta, ctx);                         \
    if (api) {                                                          \
        if (api && !stcp_api_is_alive(api)) {                           \
            LDBG("[FSM(%p)] Got not alive API for case " #sta, ctx);    \
            ctx = NULL;                                                 \
        } else {                                                        \
            ctx = stcp_fsm_acquire_ctx(api);                            \
        }                                                               \
    } else {                                                            \
        LDBG("[FSM(%p)] Got no API for case " #sta, ctx);               \
        ctx = NULL;                                                     \
    }                                                                   \
    if (ctx) {                                                          \
        STCP_LOG_REFERENCES(ctx);                                       \
    } else {                                                            \
        LDBG("[FSM(%p)] Got no context for case " #sta, ctx);           \
        goto out_of_case;                                               \
    }

#define FSM_STATE_END(ctx, sta)  \
    LDBG("[FSM(%p)] End of case " #sta, ctx);

#else // NOT VERBOSE

#define STCP_SEMA_TAKE(sema, atom, fsm, gotoLabel, failstate) \
    do {                                                                                              \
        if (fsm->stop) {                                                                              \
            goto gotoLabel;                                                                           \
        }                                                                                             \
        int __rc = 0;                                                                                 \
        if (atomic_get(&(atom)) == 0) {                                                               \
            __rc = k_sem_take(&(sema), STCP_SEMA_TAKE_TIMEOUT_VALUE);                                 \
        }                                                                                             \
        if (fsm->stop) {                                                                              \
            goto gotoLabel;                                                                           \
        }                                                                                             \
        if (__rc < 0) {                                                                               \
            LDBG("Wait %s timedout for.. %d seconds passed.", #sema, STCP_SEMA_TAKE_TIMEOUT_SECONDS); \
            STCP_STCP_FSM_STATE_SET(fsm, failstate);                                                                   \
            goto gotoLabel;                                                                           \
        }                                                                                             \
    } while(0)


#define FSM_STATE_START(ctx, sta)  \
    if (api) {                                                          \
        if (api && !stcp_api_is_alive(api)) {                           \
            ctx = NULL;                                                 \
        } else {                                                        \
            ctx = stcp_fsm_acquire_ctx(api);                            \
        }                                                               \
    } else {                                                            \
        ctx = NULL;                                                     \
    }                                                                   \
    if (!ctx) {                                                          \
        LDBG("[FSM(%p)] Got no context for case " #sta, ctx);           \
        goto out_of_case;                                               \
    }

#define FSM_STATE_END(ctx, sta)
#endif // VERBOSE

void stcp_fsm_ctx_reset(struct stcp_ctx *theContext)
{
    LWRN("RESETTING CTX %p", theContext);
    STCP_REF_COUNT_GET(theContext, "@ context reset", return; );
/*
    if (theContext->session) {
        LDBG("Reset: Freeing session %p", theContext->session);
        rust_exported_session_destroy(theContext->session);
        LDBG("Reset: Freed session now..");
        theContext->session = NULL;
    }
*/

    if (theContext->dns_resolved) {
        zsock_freeaddrinfo(theContext->dns_resolved);
        theContext->dns_resolved = NULL;
    }

    if (theContext->ks.fd >= 0) {
        zsock_close(theContext->ks.fd);
        theContext->ks.fd = -1;
    }
    // Proper init after frees
    stcp_create_soft_reset_context(theContext);
    STCP_REF_COUNT_PUT(theContext, "@ context reset");
}

void stcp_fsm_last_run_done_ok_update(struct stcp_fsm *fsm) {
    if (fsm) {
        fsm->last_run_done_ok = k_uptime_get_32();
        LDBG("Updated last ok for fsm %p...", fsm);
    }
}

static void stcp_fsm_ctx_soft_reset(struct stcp_ctx *theContext)
{
    LWRN("RESETTING CTX %p", theContext);
    STCP_REF_COUNT_GET(theContext, "@ context reset", return; );

    if (theContext->session) {
        LDBG("Reset: Freeing session %p", theContext->session);
        rust_exported_session_destroy(theContext->session);
        LDBG("Reset: Freed session now..");
        theContext->session = NULL;
    }

    if (theContext->dns_resolved) {
        zsock_freeaddrinfo(theContext->dns_resolved);
        theContext->dns_resolved = NULL;
    }

    if (theContext->ks.fd >= 0) {
        zsock_close(theContext->ks.fd);
        theContext->ks.fd = -1;
    }

    // Proper init after frees
    stcp_create_soft_reset_context(theContext);
    STCP_REF_COUNT_PUT(theContext, "@ context reset");
}

static inline struct stcp_ctx *stcp_ctx_acquire_safe(struct stcp_api *api)
{
    if (!api) return NULL;

    struct stcp_ctx *ctx;

    if (!stcp_api_is_alive(api)) {
        LWRN("[FSM/Aquire] API is not alive");
        return NULL;
    }

    /* 🔥 lukitaan API */
    API_CONTEXT_LOCK(api);


        ctx = api->ctx;
        LDBG("[FSM/Aquire] Aquired context %p from api %p", ctx, api);

    API_CONTEXT_UNLOCK(api);

    LDBG("[FSM/Aquire] Aquired %p context from api %p", ctx, api);

    if (!ctx) {
        return NULL;
    }

    STCP_REF_COUNT_GET(ctx, "@ safe aquire", return NULL; );

    return ctx;
}

static inline struct stcp_ctx *stcp_fsm_acquire_ctx(struct stcp_api *api)
{
    if (!api) return NULL;

    return stcp_ctx_acquire_safe(api);
}

static void stcp_fsm_thread(void *p1, void *p2, void *p3)
{
    struct stcp_fsm *fsm = p1;
    struct stcp_api *api = p2;
    int fsm_locked = 0;
    int ctx_locked = 0;
    int refs = 0;
    if (p1 == NULL) {
        LDBG("p1 == NULL!");
        return;
    }

    //LINF("[FSM] fsm=%p, api=%p", fsm, api);

    if (fsm && api) {
        LINF("[FSM] api=%p ctx:%p",api, api->ctx);
        refs = stcp_context_lifespan_get_span(api->ctx);
        //LDBG("[FSM] @ Before while: API context %p has now %d references", api->ctx, refs);
    }
    // Init arvo...
    struct stcp_ctx *theContext = api->ctx;

    while (!fsm->stop) {

        if (!api || !stcp_api_is_alive(api)) {
            LDBG("[FSM] exitting, API null");
            sleep_ms_jitter(1000,300);
            if (fsm_locked) {
                k_mutex_unlock(&fsm->lock);
                fsm_locked = 0;
            }
            // Breakataan => exit polku 
            break;
        }

        stcp_fsm_last_run_done_ok_update(fsm);

        // 🔑 TÄRKEÄ, pitää varmasti elossa koko luupin
        // STCP_REF_COUNT_GET(api->ctx, "@ FSM loop", LDBG("FSM LOOP ref get FAIL"); sleep_ms_jitter(100,20); continue; );
                    
        // Tuolla sisällä otetaan refcountti ...
        // Nykyisin tehdään per state.. 
        //LDBG("[FSM] Aquire from API %p context %p: %p", api, api->ctx);
        //LDBG("[FSM] Context aquired: %p", theContext);

        if (!theContext) {
            LWRN("[FSM] context gone → exiting");
            if (fsm_locked) {
                k_mutex_unlock(&fsm->lock);
                fsm_locked = 0;
            }
            // Exit from thread...
            break;
        }

        refs = stcp_context_lifespan_get_span(theContext);
        //LDBG("[FSM] @ Start of while: API context %p has now %d references", theContext, refs);

        if (atomic_get(&theContext->closing)) {
            LWRN("[FSM] ctx closing → abort state handling..");
            //STCP_REF_COUNT_PUT(theContext, "safe aquired");
            sleep_ms_jitter(500,30);
            if (fsm_locked) {
                k_mutex_unlock(&fsm->lock);
                fsm_locked = 0;
            }
            continue;
        }

        //LDBG("FSM %p locking", fsm);
        int to = k_mutex_lock(&fsm->lock, K_MSEC(5000));
        if (to != 0) {
            LWRNBIG("FSM %p: LOCK Timeout while in state: %d", fsm, fsm->state);
            STCP_REF_COUNT_PUT(theContext, "safe aquired @ timeout");
            if (fsm_locked) {
                k_mutex_unlock(&fsm->lock);
                fsm_locked = 0;
            }
            continue;
        }
        //LDBG("FSM %p locked", fsm);
        fsm_locked = 1;
        //LDBG("[FSM] ctx=%p fd=%d session=%p state=%d",
        //        theContext,
        //        theContext->ks.fd,
        //        theContext->session,
        //        theContext->state);

        switch (fsm->state) {

        case STCP_FSM_INIT:
            FSM_STATE_START(theContext, INIT);
            STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_WAIT_LTE);
            fsm->last_run_done_ok = k_uptime_get_32();
            {
                //struct stcp_api *api = GET_API_FROM_CTX(theContext);
                if (api != NULL) {
                    LDBG("[FSM] Resetting connected for API %p", api);
                    atomic_set(&api->connected, 0);
                }
            }
            FSM_STATE_END(theContext, INIT);
            goto out_of_case;

        case STCP_FSM_WAIT_LTE:
            FSM_STATE_START(theContext, WAIT_LTE);
            LDBG("Starting to wait LTE sema..");
            if (fsm->stop) {
                LDBG("STCP/FSM: Stop requested!");
                goto out_of_wait_lte;
            }

            int rc = stcp_fsm_wait_until_reached_lte_ready(fsm, -1);

            if (fsm->stop) {
                LDBG("STCP/FSM: Stop requested!");
                goto out_of_wait_lte;
            }

            if (rc < 0) {
                STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_RECONNECT);
            } else {
                STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_WAIT_PDN);
            }
out_of_wait_lte:
            FSM_STATE_END(theContext, WAIT_LTE);
            goto out_of_case;

        case STCP_FSM_WAIT_PDN:
            FSM_STATE_START(theContext, WAIT_PDN);
            STCP_SEMA_TAKE(g_sem_pdn_ready, g_pdn_active, fsm, out_of_wait_pdn, STCP_FSM_TCP_RECONNECT);
            STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_WAIT_IP);
out_of_wait_pdn:
            FSM_STATE_END(theContext, WAIT_PDN);
            goto out_of_case;

        case STCP_FSM_WAIT_IP:
            FSM_STATE_START(theContext, WAIT_IP);
            STCP_SEMA_TAKE(g_sem_ip_ready, g_ip_active, fsm, out_of_wait_ip, STCP_FSM_TCP_RECONNECT);
            STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_WAIT_STABLE);
            LDBG("State set to %d", fsm->state);
out_of_wait_ip:
            FSM_STATE_END(theContext, WAIT_IP);
            goto out_of_case;

        case STCP_FSM_WAIT_STABLE:
            FSM_STATE_START(theContext, WAIT_STABLE);
            if (fsm->stop) {
                LDBG("STCP/FSM: Stop requested!");
                goto out_of_wait_stable;
            }

            LDBG("[FSM] LTE stable settle time...");
            sleep_ms_jitter(2000,400);

            LDBG("[FSM] LTE stable settle time, after");
            if (fsm->stop) {
                LDBG("STCP/FSM: Stop requested!");
                goto out_of_wait_stable;
            }
            fsm->eagain_counter = STCP_MAX_HANDSHAKE_TRY_COUNT; 
            STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_CONNECT);
            LDBG("State set to %d", fsm->state);
            LDBG("[FSM] LTE stable settle time, exit");
out_of_wait_stable:
            FSM_STATE_END(theContext, WAIT_STABLE);
            goto out_of_case;

        case STCP_FSM_TCP_CONNECT: {
            FSM_STATE_START(theContext, TCP_CONNECT);

            /* 🔥 TÄRKEIN FIX */
            STCP_FSM_LOCK_CONTEXT(theContext, STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_RECONNECT);  goto out_of_tcp_connect; );
                
                // Tehdään välittömästi!
                //if (theContext->session) {
                //    rust_exported_session_destroy(theContext->session);
                //    theContext->session = NULL;
                //}

                struct stcp_ctx* pCurrentContext = theContext;
                void* pCurrentSession = NULL;

                if (pCurrentContext && pCurrentContext->session) {
                    pCurrentSession = pCurrentContext->session;
                }
                int now_connected = atomic_get(&pCurrentContext->connected);
                if (now_connected && pCurrentSession) {
                    LWRN("[FSM] Already connected (CTX: %p, Session: %p)", 
                        fsm->state, pCurrentContext, pCurrentSession);
                    CONTEXT_UNLOCK(theContext);
                    ctx_locked = 0;
                    sleep_ms_jitter(200, 50);
                    goto out_of_tcp_connect;
                }
                
                if (!atomic_get(&g_ip_active)) {
                    LWRN("[FSM] IP not ready → skip DNS");
                    CONTEXT_UNLOCK(theContext);
                    ctx_locked = 0;
                    goto out_of_tcp_connect;
                }
                
                int fd = -1;
                int rc = -1;
                struct zsock_addrinfo *res = NULL;
                
                if (theContext) {
                    LDBGBIG("Doing dump at TCP_CONNECT state, fsm");
                    stcp_debug_dump_stcp_ctx(theContext);
                }

            CONTEXT_UNLOCK(theContext);
            ctx_locked = 0;

            LDBGBIG("[FSM] DNS fetching ....");
            
            rc = stcp_tcp_resolve_and_make_socket(
                    theContext->ctx_hostname,
                    theContext->ctx_port,
                    &res,
                    &fd
                );

            LDBG("[FSM] host %s:%d => resolved: ptr %p & fd: %d, errno: %d", 
                theContext->ctx_hostname,
                theContext->ctx_port,
                res, fd, errno);
    

            STCP_FSM_LOCK_CONTEXT(theContext, STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_RECONNECT);  goto out_of_tcp_connect; );
                if ((rc != 0) || (fd < 0)) {
                    LWRN("[FSM] DNS/socket failed rc=%d fd=%d", rc, fd);
                    STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_RECONNECT);
                    LDBG("State set to %d", fsm->state);
                    goto out_of_tcp_connect;
                }

                LDBGBIG("[FSM] DNS/socket OK: rc=%d fd=%d", rc, fd);
                // OK Keissi...

                if (theContext->dns_resolved) {
                    LDBGBIG("[FSM] Old resolved DNS info free...");
                    zsock_freeaddrinfo(theContext->dns_resolved);
                    theContext->dns_resolved = NULL;
                }

                theContext->dns_resolved = res;
                theContext->ks.fd = fd;

                if (atomic_get(&theContext->closing) ||
                    atomic_get(&theContext->destroyed))
                {
                    LWRNBIG("[FSM] Connection is stale...reconnecting!");
                    STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_RECONNECT);
                    goto out_of_tcp_connect;
                }

                rc = stcp_transport_context_connect(theContext);

            CONTEXT_UNLOCK(theContext);
            ctx_locked = 0;

            if (rc < 0) {
                STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_RECONNECT);
            } else {
                STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_WAIT_CONNECT);
            }

            LDBG("[FSM] State is now %d", fsm->state);
out_of_tcp_connect:
            FSM_STATE_END(theContext, TCP_CONNECT);
            goto out_of_case;
        }

        case STCP_FSM_TCP_WAIT_CONNECT:
        {
            FSM_STATE_START(theContext, TCP_WAIT_CONNECT);

            if (fsm->stop) {
                LDBG("STCP/FSM: Stop requested!");
                goto out_of_tcp_wait_connect;
            }

            // Check if already done ...
            if (theContext->handshake_done) {
                LWRN("[FSM] Wait context connect: already done hanshake");
            }

            int err = 0;
            socklen_t len = sizeof(err);
            LWRN("waiting with dns_resolved: %p, fd: %d", theContext->dns_resolved, theContext->ks.fd);
            int rc = zsock_getsockopt(
                    theContext->ks.fd,
                    SOL_SOCKET,
                    SO_ERROR,
                    &err,
                    &len);

            if (fsm->stop) {
                LDBG("STCP/FSM: Stop requested!");
                goto out_of_tcp_connect;
            }

            if (rc < 0) {
                LWRN("[FSM] getsockopt failed errno=%d", errno);
                STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_RECONNECT);
                LDBG("State set to %d", fsm->state);
                goto out_of_tcp_wait_connect;
            }

            if (err == 0) {
                LDBG("[FSM] Re-Creating rust session .....");
                STCP_FSM_LOCK_CONTEXT(theContext, STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_RECONNECT);  goto out_of_tcp_wait_connect; );

                    LINF("[FSM] TCP connected..fd %d", theContext->ks.fd);

                    if ( theContext->session ) {
                        LDBG("[FSM] Recreating session: Destroying old...");
                        rust_exported_session_destroy(theContext->session);
                        theContext->session = NULL;
                    }

                    int bypass = stcp_crypto_is_aes_bypass_enabled();
                    LDBG("[FSM] AES bypass enabled? %s", GET_YES_NO_STR(bypass));
                    rc = rust_exported_session_create(&(theContext->session), &(theContext->ks), bypass);
                    LERR("[FSM] Session create rc=%d session=%p fd=%d",
                            rc,
                            theContext->session,
                            theContext->ks.fd
                        );

                    if (rc == 0) {
                        LINF("[FSM] TCP connected, setting STCP State to HANSHAKE");
                        STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_STCP_HANDSHAKE);
                    } else {
                        STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_RECONNECT);
                    }

                CONTEXT_UNLOCK(theContext);
                ctx_locked = 0;
                
                LDBG("[FSM] RUST Session %p done for API/CTX %p/%p ", 
                    theContext ? theContext->session : NULL,
                    theContext ? theContext->api : NULL,
                    theContext);

                goto out_of_tcp_wait_connect;
            }

            if (err == EINPROGRESS || err == EALREADY) {

                LDBG("[FSM] still connecting...");
                k_sleep(K_MSEC(200));
                LINF("[FSM] Contex %p connected => State to CONNECTING", theContext);
                theContext->state = STCP_STATE_CONNECTING;
                goto out_of_tcp_connect;
            }

            LWRN("[FSM] TCP connect error %d", err);

            STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_RECONNECT);
            LDBG("State set to %d", fsm->state);
            LINF("[FSM] Contex %p connected => State to CONNECTING", theContext);
            theContext->state = STCP_STATE_CONNECTING;

out_of_tcp_wait_connect:
            FSM_STATE_END(theContext, TCP_WAIT_CONNECT);
            goto out_of_case;
        }

        case STCP_FSM_STCP_HANDSHAKE: {
            FSM_STATE_START(theContext, STCP_HANDSHAKE);
            LERR(
                "[FSM HS ENTRY] fd=%d session=%p hs_done=%d",
                theContext->ks.fd,
                theContext->session,
                theContext->handshake_done
            );
#if CONFIG_STCP_DEBUG
            struct stcp_api *ctx_api = theContext->api;
            LERRBIG(
                "[FSM] HANDSHAKE LOOP hs_done=%d connect=%d fd=%d",
                theContext->handshake_done,
                atomic_get(&ctx_api->connect_in_progress),
                theContext->ks.fd
            );
#endif
            LWRN("dns_resolved: %p, fd: %d", theContext->dns_resolved, theContext->ks.fd);
            int rc = stcp_tcp_context_shake_hands(theContext, 60*1000);

            if (rc == 0) {
                LDBG("[FSM] Handshake in progress?");
                goto out_of_handshake;
            }

            if (rc == 1) {
                LDBG("[FSM] Handshake OK -> RUN state.");
                theContext->handshake_done = 1;
                STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_RUN);
                LDBG("State set to %d", fsm->state);
                LINF("[FSM] Contex %p connected => State to ESTABLISHED", theContext);
                stcp_api_connection_set_as_connected(theContext->api);
                goto out_of_handshake;
            } else {

                // Check if already done ...
                if (rc == -EALREADY) {
                    LWRN("[FSM] Context already done hanshake, settin state to RUN");
                    STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_RUN);
                    LDBG("State set to %d", fsm->state);
                    LINF("[FSM] Contex %p connected => State to ESTABLISHED", theContext);
                    theContext->state = STCP_STATE_ESTABLISHED;
                    goto out_of_handshake;
                }


                // Check if again
                if (rc == -EAGAIN) {
                    LWRN("[FSM] Hanshake returned EAGAIN, keeping state as is (Tries left: %d, %d ms (%d ms jitter) sleep between)",
                        fsm->eagain_counter,
                        STCP_MAX_HANDSHAKE_TRY_SLEEP_MS,
                        STCP_MAX_HANDSHAKE_TRY_SLEEP_JITTER_MS
                    );

                    if (fsm->eagain_counter > 0) {
                        fsm->eagain_counter -= 1;
                        sleep_ms_jitter(STCP_MAX_HANDSHAKE_TRY_SLEEP_MS, STCP_MAX_HANDSHAKE_TRY_SLEEP_JITTER_MS);
                        goto out_of_handshake;
                    }
                }

                LWRN("[FSM] Handshake failed rc=%d", rc);
                LERRBIG(
                    "STCP FSM state=%d connected=%d handshake=%d reconnect=%d fd=%d",
                    fsm->state,
                    atomic_get(&api->connected),
                    api->ctx->handshake_done,
                    atomic_get(&api->connect_in_progress),
                    api->ctx->ks.fd
                );
                STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_RECONNECT);
                LDBG("State set to %d", fsm->state);
                LINF("[FSM] Contex %p connected => State to CONNECTING", theContext);
                theContext->state = STCP_STATE_CONNECTING;
            }
out_of_handshake:
            LERR("[FSM HS RC] rc=%d, fsm_state=%d, ctx_state=%d", rc, fsm->state, theContext->state);
            FSM_STATE_END(theContext, STCP_HANDSHAKE);
            goto out_of_case;
        }

        case STCP_FSM_RUN:
            FSM_STATE_START(theContext, RUN);

            
            int modem_lte           = 0;
            int modem_pdn           = 0;
            int modem_ip            = 0;
            int modem_radio         = 0;
            int modem_connection_ok = 0;

            stcp_transport_get_modem_states(&modem_lte, &modem_pdn, &modem_ip, &modem_radio, &modem_connection_ok);

            {
                uint32_t last_run_done_ok = fsm->last_run_done_ok;
                uint32_t now = k_uptime_get_32();

                if (now - last_run_done_ok > 60000) {
                    LWRN("[FSM] RUN watchdog timeout → reconnect");
                    LERRBIG(
                        "STCP FSM state=%d connected=%d handshake=%d reconnect=%d fd=%d",
                        fsm->state,
                        atomic_get(&api->connected),
                        api->ctx->handshake_done,
                        atomic_get(&api->connect_in_progress),
                        api->ctx->ks.fd
                    );

                    STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_TCP_RECONNECT);
                    stcp_fsm_last_run_done_ok_update(fsm);
                    goto out_of_run;
                }

            }

            {
                struct stcp_api *api = GET_API_FROM_CTX(theContext);
                if (!api) {
                    LWRN("[FSM] API null in RUN → INIT");
                    LERRBIG(
                        "STCP FSM state=%d connected=%d handshake=%d reconnect=%d fd=%d",
                        fsm->state,
                        atomic_get(&api->connected),
                        api->ctx->handshake_done,
                        atomic_get(&api->connect_in_progress),
                        api->ctx->ks.fd
                    );
                    
                    sleep_ms_jitter(1000, 5000);
                    STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_INIT);
                    goto out_of_run;
                }

                atomic_set(&api->alive, 1);

                int old = atomic_get(&theContext->allow_api_access);

                if (modem_connection_ok != old) {
                    LINFBIG("[FSM] API %p Access %s in future...", theContext, GET_OK_NOK_STR(modem_connection_ok));
                    atomic_set(&theContext->allow_api_access, modem_connection_ok);
                }

                // signal connection ready,
                if (! atomic_get(&api->connected) ) {
                    atomic_set(&api->connected, 1);
                    k_sem_give(&api->connected_sem);
                    api->connection_timeouts = 0;
                }
            }

            LDBG("[FSM] At running nicely ...");
out_of_run:
            FSM_STATE_END(theContext, RUN);
            goto out_of_case;

        case STCP_FSM_TCP_RECONNECT:
            FSM_STATE_START(theContext, TCP_RECONNECT);
            LERRBIG(
                "STCP FSM @ RECONNECT state=%d connected=%d handshake=%d reconnect=%d fd=%d",
                fsm->state,
                atomic_get(&api->connected),
                api->ctx->handshake_done,
                atomic_get(&api->connect_in_progress),
                api->ctx->ks.fd
            );

            sleep_ms_jitter(1500, 500);
            
            LDBG("[FSM] Setting CTX %p not connected..", theContext);
            atomic_set(&theContext->connected, 0);

            LDBG("[FSM] Setting API %p not connected..", api);
            atomic_set(&api->connected, 0);
            api->connack_seen = 0;
            k_sem_reset(&api->connack_sem);

            LDBG("[FSM] Setting API %p not alive..", api);
            atomic_set(&api->alive, 0);

            LDBG("[FSM] Closing old context %p ..", theContext);
            stcp_api_close(theContext);

            // MUST BE NULL fro init to work!
            theContext = NULL;
            rc = stcp_api_init(&theContext);
            LDBG("[FSM] Got new context %p rc: %d..", theContext, rc);
            if (rc < 0) {
                if (theContext) {
                    stcp_api_close(theContext);
                    theContext = NULL;
                }
                // try again with same state!
                break;
            }

            STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_INIT); 
out_of_tcp_reconnect:
            FSM_STATE_END(theContext, TCP_RECONNECT);
            goto out_of_case;

        case STCP_FSM_FATAL:
        default:
            FSM_STATE_START(theContext, FSM_FATAL);
            sleep_ms_jitter(10*1000, 250);
            FSM_STATE_END(theContext, FSM_FATAL);
            goto out_of_case;
        }

out_of_case:
        LDBG("STCP: FSM locked: %d", fsm_locked);
        if (fsm_locked) {
            LDBG("FSM Exit path %p unlocking", fsm);
            k_mutex_unlock(&fsm->lock);
            LDBG("FSM Exit path %p unlock", fsm);
        }
        fsm_locked = 0;

        if (ctx_locked) {
            LDBG("FSM Exit path %p unlocking ctx %p", fsm, theContext);
            CONTEXT_UNLOCK(theContext);
            LDBG("FSM Exit path %p unlocked ctx %p", fsm, theContext);
        }

        // Log end of switch...
        LDBG("[FSM(%p)] @ end of switch", theContext);
        STCP_LOG_REFERENCES(theContext);

        ctx_locked = 0;

        if (theContext) {
            STCP_REF_COUNT_PUT(theContext, "End of FSM aquire");
        }
    }

    if (fsm->state == STCP_FSM_RUN) {
        LDBG("STCP Connected OK...");
    }

    atomic_set(
        &api->thread_running,
        0
    );
    LDBG("Bye from thread!");
}


#define STCP_FSM_STACK_SIZE         4096

//K_THREAD_STACK_DEFINE(stcp_fsm_stack, STCP_THREAD_STACK_SIZE);

void stcp_fsm_init_globals(void) {
    static int done = 0;
    if (! done) {
        done = 1;
        LINF("Initialising global modem state semaphores..");
        k_sem_init(&g_sem_lte_ready, 0, 1);
        k_sem_init(&g_sem_pdn_ready, 0, 1);
        k_sem_init(&g_sem_ip_ready, 0, 1);
        k_sem_init(&g_sem_radio_ready, 0, 1);
        
    }
}

void stcp_fsm_init(struct stcp_fsm *fsm)
{
    LDBG("[FSM] Doing FSM %p init", fsm);

    stcp_fsm_init_globals();

    LDBG("[FSM] Globals done ..");
    k_mutex_init(&fsm->lock);
    LDBG("[FSM] Lock done ..");

    LDBG("Setting up FSM...");
    STCP_STCP_FSM_STATE_SET(fsm, STCP_FSM_INIT);
    fsm->last_run_done_ok = 0;
}


void stcp_fsm_instance_start(struct stcp_fsm *fsm, struct stcp_api *api)
{
    LDBG("[FSM] Doing FSM %p init of API: %p", fsm, api);
    //memset(&api->fsm, 0, sizeof(api->fsm));

    if (!atomic_cas(
            &api->thread_running,
            0,
            1))
    {
        LWRN("[FSM] FSM already running for api %p", api);
        return;
    }

    stcp_debug_dump_stcp_ctx(api->ctx);

    LDBG("[FSM] API Check Alive : %d", stcp_api_is_alive(api));
    LDBG("[FSM] API Check Usable: %d", stcp_api_is_usable(api));    

    LDBG("[FSM] Starting thread ..");

    api->thread_tid = k_thread_create(
        &api->thread_data,
        api->thread_stack,
        K_KERNEL_STACK_SIZEOF(api->thread_stack),
        stcp_fsm_thread,
        fsm, api, NULL,
        5, 0, K_NO_WAIT
    );

    __ASSERT(api->thread_tid != NULL,
        "[FSM] FSM thread create failed");

    LDBG("[FSM] Thread done for API %p", api);
}

int stcp_fsm_wait_until_reached_ip_network_up(struct stcp_fsm *fsm, int timeout) {
    LINF("Waiting for network UP, max %d seconds....", timeout);

    if (atomic_get(&g_ip_active)) {
        LWRN("IP Already marked active.. no need to wait.");
        return 0;
    }

    if (timeout<0) {
        LINF("Waiting minute for network UP...");
        return k_sem_take(&g_sem_ip_ready, K_SECONDS(60));
    }

    LINF("Waiting %d seconds for network UP...", timeout);
    return k_sem_take(&g_sem_ip_ready, K_SECONDS(timeout));
}

#define STCP_FSM_WAIT_FOREVER_SECONDS 15
int stcp_fsm_wait_until_reached_lte_ready(struct stcp_fsm *fsm, int timeout) {
    LINF("Waiting for LTE ready, max %d seconds....", timeout);
    int to = 0;

    if (atomic_get(&g_lte_active)) {
        LWRN("LTE Already marked active.. no need to wait.");
        return 0;
    }

    if (timeout<0) {
        to = STCP_FSM_WAIT_FOREVER_SECONDS;
    } else {
        to = timeout;
    }

    LINF("Waiting %d seconds for LTE ready...", to);
    int ret = k_sem_take(&g_sem_lte_ready, K_SECONDS(to));
    return ret;
}

int stcp_fsm_wait_until_reached_pdn_ready(struct stcp_fsm *fsm, int timeout) {
    LINF("Waiting for PDN ready, max %d seconds....", timeout);
    if (atomic_get(&g_pdn_active)) {
        LWRN("PDN Already marked active.. no need to wait.");
        return 0;
    }

    if (timeout<0) {
        LINF("Waiting minute for PDN ready...");
        return k_sem_take(&g_sem_pdn_ready, K_SECONDS(60));
    }

    return k_sem_take(&g_sem_pdn_ready, K_SECONDS(timeout));
}

void stcp_fsm_start(struct stcp_fsm *fsm)
{
    // NOP
}

void stcp_fsm_notify_lte_ready(struct stcp_fsm *fsm)
{
    LDBG("Notifying %s....", __func__);
    k_sem_give(&g_sem_lte_ready);
}

void stcp_fsm_notify_pdn_ready(struct stcp_fsm *fsm)
{
    LDBG("Notifying %s....", __func__);
    k_sem_give(&g_sem_pdn_ready);
}

    void stcp_fsm_notify_ip_ready(struct stcp_fsm *fsm)
{
    LDBG("Notifying %s....", __func__);
    k_sem_give(&g_sem_ip_ready);
}

