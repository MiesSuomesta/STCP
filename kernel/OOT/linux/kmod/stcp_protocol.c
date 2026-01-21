// kmod/stcp_protocol.c

#include <net/tcp.h>
#include <net/sock.h>
#include <linux/errno.h>

#include <stcp/debug.h>
#include <stcp/settings.h>
#include <stcp/stcp_misc.h>
#include <stcp/stcp_socket_struct.h>
#include <stcp/stcp_misc.h>
#include <stcp/proto_layer.h>   // Rust proto_ops API
#include <stcp/rust_exported_functions.h>
#include <stcp/handshake_worker.h>
#include <stcp/stcp_protocol.h>

void                stcp_state_change(struct sock *sk);
int                 stcp_init_sock(struct sock *sk);
struct stcp_sock *  stcp_attach(struct sock *sk);
void                stcp_state_change(struct sock *sk);

#if USE_OWN_PROT_OPTS
int                 stcp_connect(struct sock *sk, struct sockaddr_unsized *uaddr, int addr_len);
struct sock *       stcp_accept(struct sock *sk, struct proto_accept_arg *arg);

int                 stcp_sendmsg(struct sock *sk, struct msghdr *msg, size_t len);
int                 stcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int flags, int *recv_len);
void                stcp_destroy(struct sock *sk);

extern struct proto_ops stcp_stream_ops;
extern struct proto stcp_prot;
#endif
// forward
void stcp_state_change(struct sock *sk);

static inline void stcp_handshake_kick(struct stcp_sock *st, int server_side, int reason)
{
    if (!st) return;

    /* jos jo valmis tai failannut -> ei enää mitään */
    if (test_bit(STCP_FLAG_HS_COMPLETE_BIT, &st->flags)) return;
    if (test_bit(STCP_FLAG_HS_FAILED_BIT,   &st->flags)) return;

    /* ONLY ONCE per socket */
    if (test_and_set_bit(STCP_FLAG_HS_STARTED_BIT, &st->flags))
        return;

    /* rooli lukkoon kerran */
    if (server_side) {
        set_bit(STCP_FLAG_HS_SERVER_BIT, &st->flags);
        clear_bit(STCP_FLAG_HS_CLIENT_BIT, &st->flags);
        st->is_server = 1;
    } else {
        set_bit(STCP_FLAG_HS_CLIENT_BIT, &st->flags);
        clear_bit(STCP_FLAG_HS_SERVER_BIT, &st->flags);
        st->is_server = 0;
    }

    /* queue work heti */
    st->hs_result = reason;
    stcp_rust_queue_work_for_stcp_hanshake(st, 0, reason);
}


int stcp_init_sock(struct sock *sk)
{
    /* TÄRKEÄ: wirellä pitää olla TCP */
#if ENABLE_PROTO_FORCING
    SDBG("INIT: PROTO FORCING: sk=%px protocol before=%u", sk, sk->sk_protocol);
    sk->sk_protocol = IPPROTO_TCP;
#endif 
    SDBG("INIT: Socket %px protocol=%u, state=%d",
        sk, READ_ONCE(sk->sk_protocol), READ_ONCE(sk->sk_state));
    SDBG("INIT: Doing TCP init for sock %px...", sk);
    return tcp_prot.init(sk);
}

struct stcp_sock *stcp_attach(struct sock *sk)
{
    struct stcp_sock *st = sk->sk_user_data;
    if (st) return st;

    DEBUG_INCOMING_STCP_STATUS(st);
    
    if (sk->sk_state == TCP_LISTEN) {
        SDBG("Not attaching LISTEN sock %px", sk);
        return NULL;
    }

    st = stcp_struct_alloc_st();
    if (!st) return NULL;

    // tätä sun attachin pitää toimia ilman että vaatii sk->sk_socket
    stcp_struct_attach_st_to_sk(st, sk);

    st->orginal_state_change = sk->sk_state_change;
    sk->sk_state_change = stcp_state_change;

    return st;
}

void stcp_state_change(struct sock *sk)
{
    struct stcp_sock *st = READ_ONCE(sk->sk_user_data);

    if (st &&
        test_bit(STCP_FLAG_HS_PENDING_BIT, &st->flags) &&
        sk->sk_state == TCP_ESTABLISHED)
    {
        clear_bit(STCP_FLAG_HS_PENDING_BIT, &st->flags);
        stcp_handshake_kick(st, st->is_server, REASON_TCP_ESTABLISHED);
    }

    if (st && st->orginal_state_change)
        st->orginal_state_change(sk);
}

#if USE_OWN_PROT_OPTS
int stcp_connect(struct sock *sk, struct sockaddr_unsized *uaddr, int addr_len)
{
    int ret;
    struct stcp_sock *st;

    if (!sk) return -EINVAL;

    st = stcp_attach(sk);
    if (!st) return -ENOMEM;

    if (!is_rust_init_done()) {
        pr_err("STCP: rust init not done\n");
        return -ENODEV;
    }

    if (!st->session) {
        int r = rust_exported_session_create((void **)&st->session, sk);
        if (r < 0) {
            st->flags |= STCP_FLAG_SOCKET_FATAL_ERROR_BIT;
            return r;
        }
    }

    ret = orginal_tcp_connect(sk, uaddr, addr_len);

    st->is_server = 0;

    if (ret == 0) {
        /* connect valmis heti */
        if (READ_ONCE(sk->sk_state) == TCP_ESTABLISHED) {
            stcp_handshake_kick(st, st->is_server, REASON_PUMP_CONNECT_DONE);
        } else {
            /* varmuus: jos state ei vielä established, käynnistä pending */
            set_bit(STCP_FLAG_HS_PENDING_BIT, &st->flags);
        }
    } else if (ret == -EINPROGRESS) { /* -EINPROGRESS */
        /* jos state on jo established, älä jää odottamaan state_change-eventtiä */
        if (READ_ONCE(sk->sk_state) == TCP_ESTABLISHED) {
            clear_bit(STCP_FLAG_HS_PENDING_BIT, &st->flags);
            stcp_handshake_start(st, START_HANDSHAKE_SIDE_CLIENT);
        } else {
            set_bit(STCP_FLAG_HS_PENDING_BIT, &st->flags);
        }
    } else {
        /* ERROR */
        SDBG("Connect: Error: %d", ret);
    }
    

    return ret;
}

struct sock *stcp_accept(struct sock *sk, struct proto_accept_arg *arg)
{
    struct sock *newsk = orginal_tcp_accept(sk, arg);
    if (!newsk)
        return NULL;

    struct stcp_sock *st = stcp_attach(newsk);
    if (!st) {
        stcp_destroy(newsk);
        return NULL;
    }

    if (!st->session) {
        int initDone = is_rust_init_done();
        SDBG("Accept/Session create: Is rust initted: %d", initDone);
        if (initDone) {        
            int r = rust_exported_session_create((void **)&st->session, newsk);
            if (r < 0) {
                set_bit(STCP_FLAG_SOCKET_FATAL_ERROR_BIT, &st->flags);
                st->handshake_status = STCP_HS_STATUS_HS_FAILED;
                stcp_destroy(newsk);
                return NULL;
            }
        }
    }

    st->is_server = 1;
    stcp_handshake_kick(st, st->is_server, REASON_PUMP_ACCEPT_DONE);
    return newsk;
}

static int stcp_tcp_sendmsg_bypass(struct stcp_sock *st, struct sock *sk,
                            struct msghdr *msg, size_t len)
{
    int ret;

    if (!st) {
        return -EINVAL;
    }

    if (!sk) {
        return -EINVAL;
    }

    SDBG("SEND BYPASS: sk_prot=%px tcp_sendmsg=%px stcp_sendmsg=%px",
        sk->sk_prot, orginal_tcp_sendmsg, stcp_sendmsg);

    SDBG("BYPASS: sk_prot=%px orig_sk_prot=%px stcp_prot=%px tcp_prot=%px sendmsg=%px",
     sk->sk_prot,
     st->orig_sk_prot,
     &stcp_prot,
     &tcp_prot,
     st->orig_sk_prot ? st->orig_sk_prot->sendmsg : NULL);

// Kutsutaan internal kontekstissä, ei tarvita näitä    
//    set_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);

    SDBG("[SEND %px] Sending %d bytes..", sk, (int)len);
//    ret = st->orig_sk_prot->sendmsg(sk, msg, len);
    ret = orginal_tcp_sendmsg(sk, msg, len);

//    clear_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);
    return ret;
}

// send/recv voi olla sun salauspolku. Jos handshake ei valmis,
// vaihtoehto A:ssa worker hoitaa handshaken “oikeilla” TCP send/recv -kutsuilla.
int stcp_sendmsg(struct sock *sk, struct msghdr *msg, size_t len)
{
    struct stcp_sock *st = sk ? sk->sk_user_data : NULL;

#if STCP_SOCKET_BYPASS_ALL_IO
    return stcp_tcp_sendmsg_bypass(st, sk, msg, len);
#endif 

    if (st) {
        DEBUG_INCOMING_STCP_STATUS(st);

        SDBG("Sendmsg flags=%lx internal=%d state=%d",
            st->flags,
            test_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags),
            sk->sk_state);
    } else {
        SDBG("Sendmsg got no st! state=%d",
                sk ? sk->sk_state : -1);
    }   

    SDBG("LJA: Sendmsg 1");

    if (!st)
        return orginal_tcp_sendmsg(sk, msg, len);

    SDBG("LJA: Sendmsg 2");

    // 1) INTERNAL IO bypass (HUOM: flags, ei status)
    if (test_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags)) {
        SDBG("LJA: Sendmsg: INTERNAL_IO -> original");
        return stcp_tcp_sendmsg_bypass(st, sk, msg, len);
    }

    // 2) ESTABLISHED gate
    if (sk->sk_state != TCP_ESTABLISHED) {
        SDBG("LJA: Sendmsg: not established (state=%d) -> EAGAIN", sk->sk_state);
        return -EAGAIN;
    }

    SDBG("LJA: Sendmsg 3");
    return stcp_rust_glue_proto_op_sendmsg(sk, msg, len);
}

static int stcp_tcp_recvmsg_bypass(struct stcp_sock* st, struct sock *sk, struct msghdr *msg, size_t len, int flags, int *recv_len)
{
    int ret;
     
    if (!st) {
        return -EINVAL;
    }

    if (!sk) {
        return -EINVAL;
    }
    SDBG("RECV BYPASS: sk_prot=%px tcp_recvmsg=%px stcp_recvmsg=%px",
        sk->sk_prot, orginal_tcp_recvmsg, stcp_recvmsg);

    SDBG("BYPASS: sk_prot=%px orig_sk_prot=%px stcp_prot=%px tcp_prot=%px sendmsg=%px",
        sk->sk_prot,
        st->orig_sk_prot,
        &stcp_prot,
        &tcp_prot,
        st->orig_sk_prot ? st->orig_sk_prot->sendmsg : NULL);
        
// Sama kuin sendmsg:ssä .. internal lippu on päällä jo
//    set_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);
    SDBG("[RECV %px] Receiving..", sk);
// EI tätä    ret = st->orig_sk_prot->recvmsg(sk, msg, len, flags, recv_len);
    ret = orginal_tcp_recvmsg(sk, msg, len, flags, recv_len);
    SDBG("[RECV %px] Received %d bytes.", sk, (int)( recv_len ? *recv_len : -1 ));
//    clear_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);
    return ret;
}

int stcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int flags, int *recv_len)
{
    struct stcp_sock *st = sk ? sk->sk_user_data : NULL;
    SDBG("LJA: Recvmsg 1");
    DEBUG_INCOMING_STCP_STATUS(st);

#if STCP_SOCKET_BYPASS_ALL_IO
    return stcp_tcp_recvmsg_bypass(st, sk, msg, len, flags, recv_len);
#endif 

    if (!st)
        return orginal_tcp_recvmsg(sk, msg, len, flags, recv_len);

    SDBG("LJA: Recvmsg 2");

    if (test_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags)) {
        SDBG("LJA: Recvmsg: INTERNAL_IO -> original");
        return stcp_tcp_recvmsg_bypass(st, sk, msg, len, flags, recv_len);
    }

    SDBG("LJA: Recvmsg 3");
    return stcp_rust_glue_proto_op_recvmsg(sk, msg, len, flags, recv_len);
}

void stcp_close(struct sock *sk, long timeout)
{
    struct stcp_sock *st;

    lock_sock(sk);
    st = stcp_rust_try_detach_for_cleanup(sk);
    release_sock(sk);

    if (st) {
        SDBG("@ Cleaning of stcp_sock: %px", st);
        DEBUG_INCOMING_STCP_STATUS(st);
        stcp_rust_cleanup_detached(st);
    }

    tcp_prot.close(sk, timeout);
}

static inline bool stcp_atomic_ctx(void)
{
    return in_interrupt() || in_softirq() || irqs_disabled();
}

void stcp_destroy(struct sock *sk)
{
    struct stcp_sock *st;

    SDBG("Got sk: %px", sk);
    if (!sk)
        return;

    st = xchg(&sk->sk_user_data, NULL);
    if (!st) return;

    DEBUG_INCOMING_STCP_STATUS(st);
    SDBG("Got st: %px", st);
    if (!st)
        return;

    SDBG("Detaching....");
    stcp_detach_from_st(st);

    if (unlikely(stcp_atomic_ctx())) {
        pr_warn_ratelimited("STCP: Atomic: int:%d softirq:%d irqdis:%d\n",
            in_interrupt() > 0, in_softirq() > 0, irqs_disabled() > 0);
        pr_warn_ratelimited("STCP: atomic destroy %px => %px\n", sk, st);
    }

    SDBG("Requesting destroy for %px", st);

    set_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags);
    set_bit(STCP_FLAG_SOCKET_DESTROY_QUEUED_BIT, &st->flags);

    stcp_struct_session_destroy_request(st,
        stcp_atomic_ctx() ? REASON_DESTROY_FROM_INTERRUPT : REASON_DESTROY_FROM_OK_CONTEXT);
}

#if USE_OWN_PROT_OPTS
struct proto stcp_prot;
static const char stcp_proto_name[] = "stcp";
#endif

#if 1
  #define STCP_SET_PROTO_NAME(p) strscpy((p)->name, stcp_proto_name, sizeof((p)->name))
#else
  #define STCP_SET_PROTO_NAME(p) do { (p)->name = stcp_proto_name; } while (0)
#endif
 

#endif // USE_OWN_PROT_OPTS
