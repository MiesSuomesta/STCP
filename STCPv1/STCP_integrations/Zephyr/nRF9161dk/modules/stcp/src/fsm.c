#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <stdbool.h>
#include "stcp/fsm.h"
#include "stcp/stcp_lte_fsm.h"

void stcp_fsm_start(struct stcp_fsm *fsm);
void stcp_fsm_notify_lte_ready(struct stcp_fsm *fsm);
void stcp_fsm_notify_pdn_ready(struct stcp_fsm *fsm);


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

#include <testing/include/stcp_testing.h>

#define STCP_FSM_REAL_IMPL 1

// Ei olemassa per yhteys vaan globaaleina, modeemin tilaa tämä.
struct k_sem g_sem_lte_ready;
struct k_sem g_sem_pdn_ready;
struct k_sem g_sem_ip_ready;


void stcp_fsm_reached_ip_network_up(struct stcp_fsm *fsm) {
    LDBG("Reached Network UP....");
    atomic_set(&g_ip_active, 1);
    k_sem_give(&g_sem_ip_ready);
}

void stcp_fsm_reached_lte_ready(struct stcp_fsm *fsm) {
    LDBG("Reached LTE ready....");
    atomic_set(&g_lte_active, 1);
    k_sem_give(&g_sem_lte_ready);

}

void stcp_fsm_reached_pdn_ready(struct stcp_fsm *fsm) {
    LDBG("Reached PDN ready....");
    atomic_set(&g_pdn_active, 1);
    k_sem_give(&g_sem_pdn_ready);
}

#define STCP_SEMA_TAKE(sema, atom) \
    do {                                                        \
        if (atomic_get(&(atom)) == 0) {                         \
            LDBG("Starting to wait %s", #sema);                 \
            k_sem_take(&(sema), K_FOREVER);                     \
        } else {                                                \
            LDBG("Skipped wait for %s, cause atomic %s set",    \
                #sema, #atom);                                  \
        }                                                       \
    } while(0)

#define FSM_STATE(sta)  \
    LDBG("[FSM(%p)] " #sta, ctx)


static void stcp_fsm_thread(void *p1, void *p2, void *p3)
{
    struct stcp_fsm *fsm = p1;
    struct stcp_ctx *ctx = fsm->ctx;

    if (p1 == NULL) {
        LDBG("p1 == NULL!");
        return;
    }

    LINF("fsm=%p ctx_ptr=%p", fsm, fsm->ctx);

    while (!fsm->stop) {
        LDBG("FSM %p locking", fsm);
        k_mutex_lock(&fsm->lock, K_FOREVER);
        LDBG("FSM %p locked", fsm);
        switch (fsm->state) {

        case STCP_FSM_INIT:
            FSM_STATE(INIT);
            fsm->state = STCP_FSM_WAIT_LTE;
            break;

        case STCP_FSM_WAIT_LTE:
            FSM_STATE(WAIT_LTE);
            STCP_SEMA_TAKE(g_sem_lte_ready, g_lte_active);
            fsm->state = STCP_FSM_WAIT_PDN;
            break;

        case STCP_FSM_WAIT_PDN:
            FSM_STATE(WAIT_PDN);
            STCP_SEMA_TAKE(g_sem_pdn_ready, g_pdn_active);
            fsm->state = STCP_FSM_WAIT_IP;
            break;

        case STCP_FSM_WAIT_IP:
            FSM_STATE(WAIT_IP);
            STCP_SEMA_TAKE(g_sem_ip_ready, g_ip_active);
            fsm->state = STCP_FSM_WAIT_STABLE;
            break;

        case STCP_FSM_WAIT_STABLE:
            FSM_STATE(WAIT_STABLE);
            LDBG("[FSM] LTE stable settle time...");
            k_sleep(K_SECONDS(3));
            LDBG("[FSM] LTE stable settle time, after");
            fsm->state = STCP_FSM_TCP_CONNECT;
            LDBG("[FSM] LTE stable settle time, exit");
            break;

        case STCP_FSM_TCP_CONNECT:
            FSM_STATE(TCP_CONNECT);
            int rc = stcp_tcp_resolve_and_make_socket(
                    ctx->hostname_str,
                    ctx->port_str);

            if (rc == 0) {

                LDBG("[FSM] connect started");
                fsm->state = STCP_FSM_TCP_WAIT_CONNECT;
                break;
            }

            if (rc < 0 && errno == EINPROGRESS) {

                LDBG("[FSM] connect in progress");
                fsm->state = STCP_FSM_TCP_WAIT_CONNECT;
                break;
            }

            LWRN("[FSM] TCP connect failed rc=%d errno=%d", rc, errno);

            k_sleep(K_SECONDS(3));
            fsm->state = STCP_FSM_TCP_RECONNECT;
            break;

        case STCP_FSM_TCP_WAIT_CONNECT:
        {
            FSM_STATE(TCP_WAIT_CONNECT);
            int err = 0;
            socklen_t len = sizeof(err);

            int rc = zsock_getsockopt(
                    ctx->ks.fd,
                    SOL_SOCKET,
                    SO_ERROR,
                    &err,
                    &len);

            if (rc < 0) {
                LWRN("[FSM] getsockopt failed errno=%d", errno);
                fsm->state = STCP_FSM_TCP_RECONNECT;
                break;
            }

            if (err == 0) {

                LINF("[FSM] TCP connected");
                fsm->state = STCP_FSM_STCP_HANDSHAKE;
                break;
            }

            if (err == EINPROGRESS || err == EALREADY) {

                LDBG("[FSM] still connecting...");
                k_sleep(K_MSEC(200));
                break;
            }

            LWRN("[FSM] TCP connect error %d", err);

            fsm->state = STCP_FSM_TCP_RECONNECT;
            break;
        }
    

        case STCP_FSM_STCP_HANDSHAKE: {
            FSM_STATE(STCP_HANDSHAKE);
            int rc = stcp_tcp_context_connect_and_shake_hands(fsm->ctx, 3*60*1000);
            if (rc == 1) {
                ctx->handshake_done = 1;
                fsm->state = STCP_FSM_RUN;
                break;
            } else {
                LWRN("[FSM] HS failed rc=%d", rc);
                stcp_transport_close(ctx);
                fsm->state = STCP_FSM_TCP_RECONNECT;
            }
            break;
        }

        case STCP_FSM_RUN:
            FSM_STATE(RUN);
            k_sem_give(&fsm->connection_ready);
            fsm->stop = 1;            
            break;

        case STCP_FSM_TCP_RECONNECT:
            FSM_STATE(TCP_RECONNECT);
#if STCP_FSM_REAL_IMPL
            stcp_transport_close(fsm->ctx);
#endif 
            k_sleep(K_SECONDS(3));
            fsm->state = STCP_FSM_TCP_CONNECT;
            break;

        case STCP_FSM_FATAL:
        default:
            FSM_STATE(FSM_FATAL);
            k_sleep(K_SECONDS(10));
            break;
        }
        LDBG("FSM %p unlocking", fsm);
        k_mutex_unlock(&fsm->lock);
        LDBG("FSM %p unlock", fsm);
    }

    if (fsm->state == STCP_FSM_RUN) {
        LDBG("STCP Connected OK...");
    }
    LDBG("Bye from thread!");
}


#define STCP_FSM_STACK_SIZE 4096

K_THREAD_STACK_DEFINE(stcp_fsm_stack, 8192);
static struct k_thread stcp_fsm_thread_data;
static k_tid_t stcp_fsm_tid;

void stcp_fsm_init_globals(void) {
    static int done = 0;
    if (! done) {
        done = 1;
        LINF("Initialising global modem state semaphores..");
        k_sem_init(&g_sem_lte_ready, 0, 1);
        k_sem_init(&g_sem_pdn_ready, 0, 1);
        k_sem_init(&g_sem_ip_ready, 0, 1);
    }
}

void stcp_fsm_init(struct stcp_fsm *fsm, struct stcp_ctx *ctx)
{

    memset(fsm, 0, sizeof(*fsm));

    stcp_fsm_init_globals();

    k_sem_init(&fsm->connection_ready, 0, 1);
    k_mutex_init(&fsm->lock);

    fsm->ctx = ctx;
    fsm->state = STCP_FSM_INIT;

    stcp_fsm_tid = k_thread_create(
        &stcp_fsm_thread_data,
        stcp_fsm_stack,
        K_THREAD_STACK_SIZEOF(stcp_fsm_stack),
        stcp_fsm_thread,
        fsm, NULL, NULL,
        5, 0, K_NO_WAIT
    );
}

int stcp_fsm_wait_until_reached_ip_network_up(struct stcp_fsm *fsm, int timeout) {
    LINF("Waiting for network UP, max %d seconds....", timeout);

    if (atomic_get(&g_ip_active)) {
        LWRN("IP Already marked active.. no need to wait.");
        return 0;
    }

    if (timeout<0) {
        LINF("Waiting forever for network UP...");
        return k_sem_take(&g_sem_ip_ready, K_FOREVER);
    }

    LINF("Waiting %d seconds for network UP...", timeout);
    return k_sem_take(&g_sem_ip_ready, K_SECONDS(timeout));
}

int stcp_fsm_wait_until_reached_lte_ready(struct stcp_fsm *fsm, int timeout) {
    LINF("Waiting for LTE ready, max %d seconds....", timeout);
    if (atomic_get(&g_lte_active)) {
        LWRN("LTE Already marked active.. no need to wait.");
        return 0;
    }

    if (timeout<0) {
        LINF("Waiting forever for LTE ready...");
        return k_sem_take(&g_sem_lte_ready, K_FOREVER);
    }
    return k_sem_take(&g_sem_lte_ready, K_SECONDS(timeout));
}

int stcp_fsm_wait_until_reached_connect_ready(struct stcp_fsm *fsm, int timeout) {
    if (fsm) {
        LINF("Waiting for connection ready, max %d seconds....", timeout);
        return k_sem_take(&fsm->connection_ready, K_SECONDS(timeout));
    }
    return 0;
}

int stcp_fsm_wait_until_reached_pdn_ready(struct stcp_fsm *fsm, int timeout) {
    LINF("Waiting for PDN ready, max %d seconds....", timeout);
    if (atomic_get(&g_pdn_active)) {
        LWRN("PDN Already marked active.. no need to wait.");
        return 0;
    }

    if (timeout<0) {
        LINF("Waiting forever for PDN ready...");
        return k_sem_take(&g_sem_pdn_ready, K_FOREVER);
    }

    return k_sem_take(&g_sem_pdn_ready, K_SECONDS(timeout));
}

void stcp_fsm_start(struct stcp_fsm *fsm)
{
    k_thread_start(stcp_fsm_tid);
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

