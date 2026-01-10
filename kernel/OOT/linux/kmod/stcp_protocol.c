// kmod/stcp_protocol.c

#include <net/tcp.h>
#include <net/sock.h>
#include <linux/errno.h>

#include <stcp/debug.h>
#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/handshake_worker.h>
#include <stcp/stcp_protocol.h>



void                stcp_state_change(struct sock *sk);
int                 stcp_init_sock(struct sock *sk);
struct stcp_sock *  stcp_attach(struct sock *sk);
void                stcp_state_change(struct sock *sk);
int                 stcp_connect(struct sock *sk, struct sockaddr_unsized *uaddr, int addr_len);
struct sock *       stcp_accept(struct sock *sk, struct proto_accept_arg *arg);

int                 stcp_sendmsg(struct sock *sk, struct msghdr *msg, size_t len);
int                 stcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int flags, int *recv_len);
void                stcp_destroy(struct sock *sk);

// forward
void stcp_state_change(struct sock *sk);

int stcp_init_sock(struct sock *sk)
{
    /* TÄRKEÄ: wirellä pitää olla TCP */
    SDBG("INIT: sk=%px protocol before=%u", sk, sk->sk_protocol);
    sk->sk_protocol = IPPROTO_TCP;
    SDBG("INIT: protocol after=%u", sk->sk_protocol);
    /* Aja TCP:n init normaalisti */
    return tcp_prot.init(sk);
}

struct stcp_sock *stcp_attach(struct sock *sk)
{
    struct stcp_sock *st = sk->sk_user_data;
    if (st) return st;

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
    struct stcp_sock *st = sk ? sk->sk_user_data : NULL;

    SDBG("Sock[%px // %px] state change... ", sk, st);

    if (st && st->orginal_state_change)
        st->orginal_state_change(sk);

    if (!st) return;

    int started = (st->status & STCP_STATUS_HANDSHAKE_STARTED) > 0;
    int server = st->is_server;

#ifndef STCP_RELEASE
    int pending = (st->status & STCP_STATUS_HANDSHAKE_PENDING) > 0;
    int tcp_ok = sk->sk_state == TCP_ESTABLISHED;

    SDBG("Sock[%px // %px] Status change to 0x%x // 0x%x .. Pending: %d Started: %d, TCP OK: %d, Server: %d", 
            sk, st, (unsigned int)sk->sk_state, (unsigned int)st->status, pending, started, tcp_ok, server);
#endif

    if (!started) {
        SDBG("Sock[%px // %px] server: %d", sk, st, server);
        stcp_handshake_start(st, server);
    }
}

int stcp_connect(struct sock *sk, struct sockaddr_unsized *uaddr, int addr_len)
{
    int ret;
    struct stcp_sock *st;

    ret = orginal_tcp_connect(sk, uaddr, addr_len);

    // connect voi olla async:
    // ret==0 => ready now
    // ret==-EINPROGRESS => state_change hoitaa startin kun established
    // muut negatiiviset => virhe ulos
    if (ret < 0 && ret != -EINPROGRESS)
        return ret;

    st = stcp_attach(sk);
    if (!st) return -ENOMEM;

    if (!st->session) {
        int initDone = is_rust_init_done();
        SDBG("Connect/Session create: Is rust initted: %d", initDone);
        if (initDone) {        
            int r = rust_exported_session_create((void **)&st->session, sk);
            if (r < 0) {
                st->status |= STCP_STATUS_SOCKET_FATAL_ERROR;
                return r;
            }
        }
    }
    st->is_server = 0;
    stcp_handshake_start(st, START_HANDSHAKE_SIDE_CLIENT);
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
                st->status |= STCP_STATUS_SOCKET_FATAL_ERROR;
                stcp_destroy(newsk);
                return NULL;
            }
        }
    }

    st->is_server = 1;
    stcp_handshake_start(st, START_HANDSHAKE_SIDE_SERVER);
    return newsk;
}

// send/recv voi olla sun salauspolku. Jos handshake ei valmis,
// vaihtoehto A:ssa worker hoitaa handshaken “oikeilla” TCP send/recv -kutsuilla.
int stcp_sendmsg(struct sock *sk, struct msghdr *msg, size_t len) {
    struct stcp_sock *st = sk ? sk->sk_user_data : NULL;
    int completed = (st && st->status & STCP_STATUS_HANDSHAKE_COMPLETE) > 0;

    if (!st || !completed) {
        return orginal_tcp_sendmsg(sk, msg, len);
    } else {
        return stcp_rust_glue_proto_op_sendmsg(sk, msg, len);
    }
   
}

int stcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int flags, int *recv_len) {
    struct stcp_sock *st = sk ? sk->sk_user_data : NULL;
    int completed = (st && st->status & STCP_STATUS_HANDSHAKE_COMPLETE) > 0;

    if (!st || !completed) {
        return orginal_tcp_recvmsg(sk, msg, len, flags, recv_len);
    } else {
        return stcp_rust_glue_proto_op_recvmsg(sk, msg, len, flags, recv_len);
    }
}

void stcp_destroy(struct sock *sk) {
    stcp_rust_glue_proto_op_destroy(sk);
}

struct proto stcp_prot;

int stcp_proto_setup(void)
{
    stcp_prot = tcp_prot;

    orginal_tcp_sendmsg  = tcp_prot.sendmsg;
    orginal_tcp_recvmsg  = tcp_prot.recvmsg;
    orginal_tcp_connect  = tcp_prot.connect;
    orginal_tcp_accept   = tcp_prot.accept;

    strscpy(stcp_prot.name, "stcp", sizeof(stcp_prot.name));
    stcp_prot.owner   = THIS_MODULE;

    stcp_prot.init    = stcp_init_sock;
    stcp_prot.connect = stcp_connect;
    stcp_prot.accept  = stcp_accept;
    stcp_prot.destroy = stcp_destroy;
    stcp_prot.sendmsg = stcp_sendmsg;
    stcp_prot.recvmsg = stcp_recvmsg;

    return 0;
}
