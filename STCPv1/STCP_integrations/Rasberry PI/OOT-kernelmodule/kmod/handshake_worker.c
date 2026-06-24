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

#include <stcp/debug.h>
#include <stcp/settings.h>
#include <stcp/proto_layer.h>   // Rust proto_ops API

#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/proto_operations.h>
#include <stcp/handshake_worker.h>
#include <stcp/state.h>

#define STCP_WORKER_AGAIN_DELAY_IN_MS   1000

#define STCP_SERVER_HANSHAKE_ENABLED 1
#define STCP_CLIENT_HANSHAKE_ENABLED 1
#define STCP_ALIVE_CHECKS            0

/*
 * ============================================================
 */
static int stcp_handshake_pump(void *sess, void *transport /* struct sock* */, int reason)
{
    int ret = rust_exported_session_handshake_pump(sess, transport, reason);
    SDBG("Called rust, got: %d", ret);
    return ret;
}

/*
 * Queue handshake work for given stcp_sock.
 * - Ei queuea jos EXIT_MODE on päällä
 * - Varmistaa että workqueue on olemassa
 * - Idempotent HS_QUEUED bitillä (st->flags)
 */
int stcp_rust_queue_work_for_stcp_hanshake(struct stcp_sock *st,
                                           unsigned int delayMS,
                                           int reason)
{
    if (!st)
        return -EBADF;

    if (!is_stcp_magic_ok(st)) {
        SDBG("Magic check fails..");
        return -EBADF;
    }

    if (test_bit(STCP_FLAG_HS_COMPLETE_BIT, &st->flags)) {
        SDBG("Tried to enqueue after HS DONE (st: %px)", st);
        return 0;
    }

    if (test_bit(STCP_FLAG_SOCKET_DESTROY_QUEUED_BIT, &st->flags)) {
        SDBG("Tried to enqueue after destroy queued (st: %px)", st);
        return -ESHUTDOWN;
    }

    if (test_bit(STCP_FLAG_SOCKET_DETACHED_BIT, &st->flags)) {
        SDBG("Tried to enqueue after detach as been done (st: %px)", st);
        return -ESHUTDOWN;
    }
    
#if 0
# if STCP_ALIVE_CHECKS
    {
        int alive = stcp_exported_rust_ctx_alive_count();
        if (alive < 1) {
            SDBG("WARNING: Called when not alive!, stcp_sock: %px", st);
            return -500;
        }
    }
# endif
#endif

    /* Älä queuea jos ei enää elossa */
    if (unlikely(READ_ONCE(st->magic) != STCP_MAGIC_ALIVE))
        return -ESHUTDOWN;

    /* Exit-mode: älä queuea */
    if (unlikely(test_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags))) {
        clear_bit(STCP_FLAG_HS_QUEUED_BIT, &st->flags);
        return -ECANCELED;
    }

    /* Workqueue pitää olla olemassa */
    if (unlikely(!READ_ONCE(st->the_wq))) {
        clear_bit(STCP_FLAG_HS_QUEUED_BIT, &st->flags);
        return -ESHUTDOWN;
    }

    st->hs_result = reason;

    /* Idempotent: jos jo queued -> done */
    if (test_and_set_bit(STCP_FLAG_HS_QUEUED_BIT, &st->flags))
        return -EALREADY;

    /* Vielä viimeinen varmistus raceen: wq ei saa kadota juuri tässä */
    if (unlikely(!READ_ONCE(st->the_wq) ||
                 READ_ONCE(st->magic) != STCP_MAGIC_ALIVE)) {
        clear_bit(STCP_FLAG_HS_QUEUED_BIT, &st->flags);
        return -ESHUTDOWN;
    }

    SDBG("Worker[%px] queue handshake work (in %u ms) reason=%d", st, delayMS, reason);

    if (!refcount_inc_not_zero(&st->refcnt))
        return -ESHUTDOWN;

    if (!queue_delayed_work(READ_ONCE(st->the_wq),
                            &st->handshake_work,
                            msecs_to_jiffies(delayMS))) {
        /* Ei queued (oli jo jonossa tms) => vapauta bittilippu */
        clear_bit(STCP_FLAG_HS_QUEUED_BIT, &st->flags);
        stcp_struct_put_st(st);   /* queue epäonnistui */
        return -EALREADY;
    }

    return 0;
}

/*
 * Destroy workqueue for given stcp_sock.
 *
 * HUOM:
 * - Tämä on vaarallinen kutsua worker-kontekstissa jos wq on per-socket.
 * - Parempi: älä destroyä workerissa; tee cancel + exitmode, ja destroy detach/destroy-polussa.
 */
int destroy_the_work_queue(struct stcp_sock *st)
{
    struct workqueue_struct *wq;

    if (!st)
        return -EBADF;

    if (!is_stcp_magic_ok(st)) {
        SDBG("Magic check fails..");
        return -EBADF;
    }

    set_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags);

    wq = xchg(&st->the_wq, NULL);
    if (!wq)
        return 0;

    cancel_delayed_work_sync(&st->handshake_work);

    /*
     * Jos haluat pitää vanhan käytöksen, voit palauttaa nämä,
     * mutta huomioi deadlock-riski jos tätä kutsutaan workerista:
     *
     * flush_workqueue(wq);
     * destroy_workqueue(wq);
     */
    flush_workqueue(wq);
    destroy_workqueue(wq);

    return 0;
}

/*
 * Handshake worker:
 *  - Ajetaan stcp_session_wq-workqueuessa
 *  - Hoitaa sekä client- että server-handshaken
 */
void stcp_handshake_worker(struct work_struct *work)
{
    struct delayed_work *dwork;
    struct stcp_sock *st;
    const char *tmp;
    int ret = 0;

    SDBG("Worker: Woken up.");

    if (!work) {
        SDBG("Worker: No work!");
        return;
    }

    if (!is_rust_init_done()) {
        SDBG("Worker: Rust init not done, work=%px", work);
        return;
    }

    dwork = to_delayed_work(work);
    st = container_of(dwork, struct stcp_sock, handshake_work);

    if (!st) {
        SDBG("Worker[%px//%px]: FAIL: No st!", work, st);
        return;
    }

    DEBUG_INCOMING_STCP_STATUS(st);

    if (stcp_state_is_handshake_complete(st) > 0) {
        SDBG("Tried to do handshake work after HS DONE (st: %px)", st);
        stcp_struct_put_st(st);   /* queue epäonnistui */
        return;
    }

    if (!is_stcp_magic_ok(st)) {
        SDBG("Worker[%px//%px]: FAIL: Magic failure!", work, st);
        stcp_struct_put_st(st);   /* queue epäonnistui */
        return;
    }

    SDBG("HANDSHAKE WORKER START child_st=%px transport=%px",
         st, st->transport);

    /* Nyt kun worker on käynnissä, queued-bit alas */
    clear_bit(STCP_FLAG_HS_QUEUED_BIT, &st->flags);

    if (test_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags)) {
        stcp_struct_put_st(st);   /* queue epäonnistui */
        return;
    }
    if (unlikely(READ_ONCE(st->magic) != STCP_MAGIC_ALIVE)) {
        stcp_struct_put_st(st);   /* queue epäonnistui */
        return;
    }

    /* Tässä ei ole enää connectin lock_sock päällä */
    tmp = test_bit(STCP_FLAG_HS_SERVER_BIT, &st->flags) ? "Server" : "Client";

    SDBG("Worker/%s[%px//%px]: sk=%px session=%px wq=%px flags=%lx status=%d flags=%ld",
         tmp, work, st, st->sk, st->session, st->the_wq, st->flags, st->handshake_status, st->flags);

    st->is_server = test_bit(STCP_FLAG_HS_SERVER_BIT, &st->flags);
    SDBG("Is server? %d", st->is_server);

#if STCP_SERVER_HANSHAKE_ENABLED
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
            // Marking internal ...
            set_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);
            ret = rust_exported_data_server_ready_worker(
                      (void *)st->session,
                      (void *)st->sk);
            clear_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);
        }
    }
#endif



#if STCP_CLIENT_HANSHAKE_ENABLED
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
            // Marking internal ...
            set_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);
            ret = rust_exported_data_client_ready_worker(
                      (void *)st->session,
                      (void *)st->sk);
            clear_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);
        }
    }
#endif

    SDBG("Worker/%s[%px//%px]: Work complete: %d", tmp, work, st, ret);

    /* pump */
    if (st->pump_counter++ < STCP_HANDSHAKE_STATUS_MAX_PUMPS) {
        set_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);
        ret = stcp_handshake_pump((void *)st->session, (void *)st->sk, st->hs_result);
        clear_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);
    } else {
        stcp_state_hanshake_mark_failed(st, -EPROTO);
    }

    /*
     * Edistystä → resched uusiksi
     */
    if (ret > 0) {
        SDBG("Got progress, rescheduling (%d ms until next wakeup) ....",
             STCP_WORKER_AGAIN_DELAY_IN_MS);
        stcp_rust_queue_work_for_stcp_hanshake(
            st,
            STCP_WORKER_AGAIN_DELAY_IN_MS,
            HS_PUMP_REASON_NEXT_STEP);
        stcp_struct_put_st(st);   /* queue epäonnistui */
        return;
    }

    // Handshake returnaa nollan JOS Complete,
    // ret > 0 : HS on käynnissä, vaihtoi tilaa
    // ret < 0 : HW failasi
    st->hs_result = ret;
    if (ret == 0) {
        SDBG("Worker[%px//%px]: Handshake DONE!", work, st);
        stcp_state_hanshake_mark_complete(st);
        stcp_struct_put_st(st);   /* queue epäonnistui */
        return;
    }

    /* ret < 0 : fail */
    SDBG("Worker[%px//%px]: Handshake FAILED ret=%d", work, st, ret);

    stcp_state_hanshake_mark_failed(st, -EPROTO);
    stcp_log_st_fields("Handshake failed", st, st->sk);

    SDBG("Worker[%px//%px]: Handshake failed, marking error to socket %px",
         work, st, st->sk);

    /*
     * HUOM: älä välttämättä destroyä wq:ta workerista (deadlock riski).
     * Jos haluat pitää per-socket wq:n, parempi on:
     *  - aseta EXIT_MODE
     *  - cancel_delayed_work_sync tehdään destroy-polussa
     *
     * Tässä jätetään vanha kutsu pois. (Jos haluat, voit palauttaa.)
     */
    /* destroy_the_work_queue(st); */

    /* Tapetaan yhteys.. */
    if (st->sk) {
        SDBG("Killing stcp sk socket %px", st->sk);
        st->sk->sk_err = EPROTO;
        sk_error_report(st->sk);
    }
    stcp_struct_put_st(st);   /* queue epäonnistui */
}

void stcp_handshake_start(struct stcp_sock *st, int server_side)
{
    SDBG("Called with %px, %d", st, server_side);

    if (!st)
        return;

    if (!is_stcp_magic_ok(st)) {
        SDBG("HS Start: Magic failure!");
        return;
    }

    /* estä tuplakäynnistys ATOMISESTI */
    if (test_and_set_bit(STCP_FLAG_HS_STARTED_BIT, &st->flags))
        return;

    if (test_bit(STCP_FLAG_HS_COMPLETE_BIT, &st->flags))
        return;
    if (test_bit(STCP_FLAG_HS_FAILED_BIT, &st->flags))
        return;
    if (test_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags))
        return;

    if (server_side) {
        set_bit(STCP_FLAG_HS_SERVER_BIT, &st->flags);
        clear_bit(STCP_FLAG_HS_CLIENT_BIT, &st->flags);
    } else {
        set_bit(STCP_FLAG_HS_CLIENT_BIT, &st->flags);
        clear_bit(STCP_FLAG_HS_SERVER_BIT, &st->flags);
    }

    SDBG("HS: start queued st=%px sk=%px role=%s",
         st, st->sk, server_side ? "server" : "client");

    int ret = stcp_rust_queue_work_for_stcp_hanshake(st, 5, HS_PUMP_REASON_MANUAL);
    SDBG("HS: start queued st=%px sk=%px role=%s, queue ret: %d",
         st, st->sk, server_side ? "server" : "client", ret);

}
