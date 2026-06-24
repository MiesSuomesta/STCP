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

