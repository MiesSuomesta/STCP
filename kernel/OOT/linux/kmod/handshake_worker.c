#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/net.h>
#include <linux/errno.h>
#include <linux/jiffies.h>

#include <net/sock.h>
#include <net/inet_common.h>
#include <net/inet_connection_sock.h>
#include <net/protocol.h>
#include <net/tcp.h>
#include <linux/inet.h>

#define DISABLE_DEBUG 1
#include <stcp/debug.h>
#include <stcp/proto_layer.h>   // Rust proto_ops API

#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/proto_operations.h>
#include <stcp/handshake_worker.h>

#define STCP_WORKER_AGAIN_DELAY_IN_MS   1000

#define STCP_SERVER_HANSHAKE_ENABLED 1
#define STCP_CLIENT_HANSHAKE_ENABLED 1
#define STCP_ALIVE_CHECKS            0


static int stcp_handshake_pump(void *sess, void *transport /* struct sock* */, int reason) {

    // RUST Call
    int ret = rust_exported_session_handshake_pump(sess, transport, reason);
    SDBG("Called rust, got: %d", ret);

    return ret;
}

/*
 * Queue handshake work for given stcp_sock.
 * - Tarkistaa että Rust-konteksti on elossa
 * - Ei queuea jos EXIT_MODE on päällä
 * - Varmistaa että the_wq on olemassa
 */
int stcp_queue_work_for_stcp_hanshake(struct stcp_sock *st, unsigned int delayMS, int reason)
{

#if STCP_ALIVE_CHECKS
    int alive;

    alive = stcp_exported_rust_ctx_alive_count();
    if (alive < 1) {
        SDBG("WARNING: Called when not alive!, stcp_sock: %px", st);
        return -500;
    }
#endif

    SDBG("Worker Queue: %px, delay: %d ms", st, delayMS);


    if (!st) {
        SDBG("Worker: No st!");
        return -EBADF;
    }

    /* Jos soketti on merkitty exit-tilaan, ei enää queuea */
    if ((st->status & STCP_STATUS_HANDSHAKE_EXIT_MODE) > 0) {
        SDBG("STCP socket %px marked to be in exit mode, not queuing work.", st);

        if (st->the_wq) {
            SDBG("STCP socket %px work queue emptied & destroyed..", st);
            cancel_delayed_work_sync(&st->handshake_work);
            destroy_workqueue(st->the_wq);
            st->the_wq = NULL;
        }

        SDBG("STCP socket %px, work request cleaned", st);
        return -ECANCELED;
    }

    if (!st->the_wq) {
        SDBG("Worker: No queue!");
        return -EBADF;
    }

    st->hs_result = reason;

    if (test_and_set_bit(STCP_STATUS_HS_QUEUED_BIT, &st->status)) {
        SDBG("Already queued work for %px", st);
        return -EALREADY;
    }

    SDBG("Worker[%px] Adding work to queue (resched in %u ms)...", st, delayMS);
    queue_delayed_work(st->the_wq,
                       &st->handshake_work,
                       msecs_to_jiffies(delayMS));
    return 0;
}

/*
 * Destroy workqueue for given stcp_sock.
 */
int destroy_the_work_queue(struct stcp_sock *st)
{
    if (!st) {
        SDBG("Worker: No st!");
        return -EBADF;
    }

    if (!st->the_wq) {
        SDBG("Worker: No WQ!");
        return -EBADF;
    }

    SDBG("Cancel work for %px", st);
    cancel_delayed_work_sync(&st->handshake_work);
    destroy_workqueue(st->the_wq);
    st->the_wq = NULL;

    SDBG("Worker: Queue destroyed....");
    return 0;
}

/*
 * Handshake worker:
 *  - Ajetaan stcp_session_wq-workqueuessa
 *  - Hoitaa sekä client- että server-handshaken
 *  - Ei pidä isoja puskureita stackilla (vain muutama pieni lokaali)
 */
void stcp_handshake_worker(struct work_struct *work)
{
    struct delayed_work *dwork;
    struct stcp_sock *st;
    const char *tmp;
    int ret = 0;

    SDBG("Worker: Woken up.");
    int alive = stcp_exported_rust_ctx_alive_count();
    if (alive < 1) {
        SDBG("Worker: Rust ctx not alive, work=%px", work);
        return;
    }

    if (!work) {
        SDBG("Worker: No work!");
        return;
    }

    SDBG("Worker: CP 1");
    dwork = to_delayed_work(work);
    st = container_of(dwork, struct stcp_sock, handshake_work);

    SDBG("HANDSHAKE WORKER START child_st=%px transport=%px",
            st, st && st->transport);

    if (!st) {
        SDBG("Worker[%px//%px]: FAIL: No st!", work, st);
        return;
    }

    clear_bit(STCP_STATUS_HS_QUEUED_BIT, &st->status);

    if (st->status & STCP_STATUS_HANDSHAKE_EXIT_MODE)
        return;

    if (!is_rust_init_done()) {
        SDBG("Rust no initialised..");        
        return;
    } 

    /* Tässä ei ole enää connectin lock_sock päällä */
    tmp = ((st->status & STCP_STATUS_HANDSHAKE_SERVER) > 0) ? "Server" : "Client";

    SDBG("Worker/%s[%px//%px]: State: %u  sk: %px session: %px wq: %px",
         tmp, work, st, st->status, st->sk, st->session, st->the_wq);

    st->is_server = (st->status & STCP_STATUS_HANDSHAKE_SERVER) > 0;
    SDBG("Is server? %d", st->is_server);

#if STCP_SERVER_HANSHAKE_ENABLED
    SDBG("Define STCP_SERVER_HANSHAKE_ENABLED active.. %px, %u", st, st ? st->status : 0);
    if (st->is_server) {
        SDBG("Starting server work, with st=%px sk=%px session=%px",
             st, st->sk, st->session);

        if (!st->session) {
            SDBG("Starting server work: FAILED, no ProtoSession (st->session == NULL)!");
            ret = -EBADMSG;
        } else if (!st->sk) {
            SDBG("Starting server work: FAILED, no sock!");
            ret = -EBADMSG;
        } else {
            SDBG("HANDSHAKE START: Starting server worker...");
            ret = rust_exported_data_server_ready_worker(
                      (void *)st->session,
                      (void *)st->sk);
        }
    }
#else
    SDBG("Server handshake disabled");
#endif

#if STCP_CLIENT_HANSHAKE_ENABLED
    SDBG("Define STCP_CLIENT_HANSHAKE_ENABLED active.. %px, %u", st, st ? st->status : 0);
    if (!st->is_server) {
        SDBG("Starting client work, with st=%px sk=%px session=%px",
             st, st->sk, st->session);

        if (!st->session) {
            SDBG("Starting client work: FAILED, no ProtoSession (st->session == NULL)!");
            ret = -EBADMSG;
        } else if (!st->sk) {
            SDBG("Starting client work: FAILED, no sock!");
            ret = -EBADMSG;
        } else {
            SDBG("HANDSHAKE START: Starting client work...");
            ret = rust_exported_data_client_ready_worker(
                      (void *)st->session,
                      (void *)st->sk);
        }
    }
#else
    SDBG("Client handshake disabled");
#endif

    SDBG("Worker/%s[%px//%px]: Work complete: %d", tmp, work, st, ret);

    ret = stcp_handshake_pump((void *)st->session, (void *)st->sk, st->hs_result);

    /*
     * Edistystä → resched uusiksi
     */

    if (ret > 0) {
        SDBG("Got progress, rescheduling (%d ms until next wakeup) ....", 
            STCP_WORKER_AGAIN_DELAY_IN_MS);

        stcp_queue_work_for_stcp_hanshake(st, 
            STCP_WORKER_AGAIN_DELAY_IN_MS, HS_PUMP_REASON_NEXT_STEP );

    } else if (ret == 0) {
        SDBG("Worker[%px//%px]: Handshake had no progress, waiting data ready...", work, st);
    } else if (ret < 0) {
        // Lippuja 
        st->status  = STCP_STATUS_HANDSHAKE_FAILED;
        st->status |= STCP_STATUS_HANDSHAKE_EXIT_MODE;
        st->status |= STCP_STATUS_SOCKET_FATAL_ERROR;

        st->hs_result = ret;
        complete_all(&st->hs_done);
        SDBG("Comleted hs_done");

        SDBG("Worker[%px//%px]: Handshake failed, marking error to socket %px",
             work, st, st->sk);

        destroy_the_work_queue(st);

        /* Tapetaan yhteys.. */
        if (st->sk) {
            SDBG("Killing stcp sk socket %px", st->sk)
            st->sk->sk_err = EPROTO;
            sk_error_report(st->sk);
        }
    }
}

void stcp_handshake_start(struct stcp_sock *st, int server_side)
{
    SDBG("Called with %px, %d", st, server_side);

    if (!st)
        return;

    /* estä tuplakäynnistys */
    if (st->status & STCP_STATUS_HANDSHAKE_PENDING)
        return;

    if (server_side)
        st->status |= STCP_STATUS_HANDSHAKE_SERVER;
    else
        st->status |= STCP_STATUS_HANDSHAKE_CLIENT;

    st->status |= STCP_STATUS_HANDSHAKE_PENDING;

    SDBG("HS: start queued st=%px sk=%px role=%s",
         st, st->sk, server_side ? "server" : "client");

    stcp_queue_work_for_stcp_hanshake(st, 0, HS_PUMP_REASON_MANUAL);
}
