#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <stdbool.h>
#include "fsm.h"
#include "stcp_lte_fsm.h"

void stcp_fsm_init(struct stcp_fsm *fsm);
void stcp_fsm_start(struct stcp_fsm *fsm);
void stcp_fsm_notify_lte_ready(struct stcp_fsm *fsm);
void stcp_fsm_notify_pdn_ready(struct stcp_fsm *fsm);

LOG_MODULE_REGISTER(stcp_fsm, LOG_LEVEL_INF);

#include "fsm.h"
#include "debug.h"
#include "stcp_alloc.h"
#include "stcp_struct.h"
#include "stcp_bridge.h"
#include "stcp_net.h"
#include "workers.h"
#include "utils.h"
#include "stcp_operations_zephyr.h"

#include "stcp_rust_exported_functions.h"

#define STCP_FSM_REAL_IMPL 1
extern struct stcp_ctx* theStcpContext;

static void stcp_fsm_thread(void *p1, void *p2, void *p3)
{
    struct stcp_fsm *fsm = p1;

    if (p1 == NULL) {
        LDBG("p1 == NULL!");
        return;
    }

    LOG_INF("fsm=%p ctx_ptr=%p", fsm, fsm->ctx);

    while (!fsm->stop) {
        switch (fsm->state) {

        case STCP_FSM_INIT:
            LDBG("[FSM] INIT");
            fsm->state = STCP_FSM_WAIT_LTE;
            break;

        case STCP_FSM_WAIT_LTE:
            LDBG("[FSM] WAIT LTE");
            k_sem_take(&(fsm->lte_ready), K_FOREVER);
            fsm->state = STCP_FSM_WAIT_PDN;
            break;

        case STCP_FSM_WAIT_PDN:
            LDBG("[FSM] WAIT PDN");
            k_sem_take(&(fsm->pdn_ready), K_FOREVER);
            fsm->state = STCP_FSM_WAIT_IP;
            break;

        case STCP_FSM_WAIT_IP:
            LDBG("[FSM] WAIT IP");
            k_sem_take(&(fsm->ip_ready), K_FOREVER);
            fsm->state = STCP_FSM_WAIT_STABLE;
            break;

        case STCP_FSM_WAIT_STABLE:
            LDBG("[FSM] LTE stable settle time...");
            k_sleep(K_SECONDS(3));
            fsm->state = STCP_FSM_TCP_CONNECT;
            break;

        case STCP_FSM_TCP_CONNECT:
            LDBG("[FSM] TCP CONNECT");
#if STCP_FSM_REAL_IMPL
            struct stcp_ctx *ctx = stcp_tcp_resolve_and_make_context("lja.fi", "7777");
            if (!ctx) {
                LWRN("[FSM] ctx alloc failed");
                k_sleep(K_SECONDS(3));
                break;
            }

            if (ctx != NULL) {
                fsm->ctx = ctx;
                theStcpContext = ctx;
                fsm->state = STCP_FSM_STCP_HANDSHAKE;
            } else {
                //stcp_transport_close(ctx);
                //fsm->ctx = NULL;
                k_sleep(K_SECONDS(3));
                fsm->state = STCP_FSM_TCP_RECONNECT;
            }
#endif
            break;

        case STCP_FSM_STCP_HANDSHAKE: {
            LDBG("[FSM] STCP HANDSHAKE");
#if STCP_FSM_REAL_IMPL
            int rc = stcp_tcp_context_connect_and_shake_hands(fsm->ctx, 3*60*1000);
            if (rc == 1) {
                fsm->state = STCP_FSM_RUN;
            } else {
                LWRN("[FSM] HS failed rc=%d", rc);
                stcp_transport_close(theStcpContext);
                fsm->state = STCP_FSM_TCP_RECONNECT;
            }
#endif
            break;
        }

        case STCP_FSM_RUN:
            LDBG("[FSM] RUN");
#if STCP_FSM_REAL_IMPL
            LDBG("[FSM] Starting AES loop....");
            int rc = 0;
            while (!fsm->stop) {
                rc = stcp_run_loop(fsm->ctx);
                if (rc < 0) {
                    LWRN("[FSM] RUN loop error %d", rc);
                    fsm->state = STCP_FSM_TCP_RECONNECT;
                    break;
                }
                k_sleep(K_MSEC(5));
            }
#endif
            LDBG("[FSM] AES loop ended, rc: %d", rc);
            break;

        case STCP_FSM_TCP_RECONNECT:
            LDBG("[FSM] TCP RECONNECT");
#if STCP_FSM_REAL_IMPL
            stcp_transport_close(fsm->ctx);
#endif 
            k_sleep(K_SECONDS(3));
            fsm->state = STCP_FSM_TCP_CONNECT;
            break;

        case STCP_FSM_FATAL:
        default:
            LERR("[FSM] FATAL STATE");
            k_sleep(K_SECONDS(10));
            break;
        }
    }
    LDBG("Found out...");
}


#define STCP_FSM_STACK_SIZE 4096

K_THREAD_STACK_DEFINE(stcp_fsm_stack, 8192);
static struct k_thread stcp_fsm_thread_data;
static k_tid_t stcp_fsm_tid;

void stcp_fsm_init(struct stcp_fsm *fsm)
{
    memset(fsm, 0, sizeof(*fsm));

    k_sem_init(&fsm->lte_ready, 0, 1);
    k_sem_init(&fsm->pdn_ready, 0, 1);
    k_sem_init(&fsm->ip_ready, 0, 1);

    fsm->ctx = NULL;
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

void stcp_fsm_reached_ip_network_up(struct stcp_fsm *fsm) 
{
    if (fsm) {
        k_sem_give(&fsm->ip_ready);
    }
}

void stcp_fsm_reached_lte_ready(struct stcp_fsm *fsm) 
{
    if (fsm) {
        k_sem_give(&fsm->lte_ready);
    }
}

void stcp_fsm_reached_pnd_ready(struct stcp_fsm *fsm) 
{
    if (fsm) {
        k_sem_give(&fsm->pdn_ready);
    }
}

void stcp_fsm_start(struct stcp_fsm *fsm)
{
    k_thread_start(stcp_fsm_tid);
}

void stcp_fsm_notify_lte_ready(struct stcp_fsm *fsm)
{
    k_sem_give(&fsm->lte_ready);
}

void stcp_fsm_notify_pdn_ready(struct stcp_fsm *fsm)
{
    k_sem_give(&fsm->pdn_ready);
}

