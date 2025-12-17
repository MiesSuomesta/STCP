// stcp_proto_ops.c
#include <linux/kernel.h>
#include <linux/net.h>
#include <net/sock.h>
#include <net/inet_common.h>
#include <net/tcp.h>

#include <stcp/debug.h>
#include <stcp/settings.h>
#include <stcp/tcp_callbacks.h>
#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/handshake_worker.h>
#include <stcp/stcp_proto_ops.h>
#if 0
static const struct proto_ops *orig_inet_ops = &inet_stream_ops;

static inline bool stcp_is_restart_err(int ret)
{
    return ret == -ERESTARTSYS ||
           ret == -ERESTARTNOINTR ||
           ret == -ERESTARTNOHAND ||
           ret == -ERESTART_RESTARTBLOCK;
}

/*
 * BIND/LISTEN: delegoi 100% inet_*:lle,
 * mutta pidä STCP state olemassa (attach) jos haluat.
 */
int stcp_proto_socket_ops_bind(struct socket *sock,
                               struct sockaddr *uaddr,
                               int addr_len)
{
    int ret;

    // delegoi OIKEIN: orig_inet_ops->bind (ei inet_bind!)
    ret = orig_inet_ops->bind(sock, uaddr, addr_len);
    SDBG("BIND: orig_inet_ops->bind ret=%d", ret);
    if (ret < 0) return ret;

    // attach STCP metadata nyt kun sk varmasti elossa
    if (sock && sock->sk) {
        struct stcp_sock *st = stcp_struct_get_or_alloc_st_from_sk(sock->sk);
        if (st) {
            // jos haluat talteen bind addr:
            if (addr_len <= sizeof(st->bind_addr)) {
                memcpy(&st->bind_addr, uaddr, addr_len);
                st->bind_addr_len = addr_len;
            }
            st->status |= STCP_STATUS_SOCKET_BOUND;
        }
    }

    SDBG("BIND: final ret=%d", ret);
    return 0;
}

int stcp_proto_socket_ops_listen(struct socket *sock, int backlog)
{
    int ret;

    // delegoi OIKEIN: orig_inet_ops->listen (ei inet_listen!)
    ret = orig_inet_ops->listen(sock, backlog);
    SDBG("LISTEN: orig_inet_ops->listen ret=%d state=%d", ret,
         sock && sock->sk ? sock->sk->sk_state : -1);
    return ret;
}

/*
 * CONNECT: anna TCP:n tehdä connect, ja käynnistä handshake-workkeri
 * VASTA kun yhteys on oikeasti ESTABLISHED.
 *
 * Tärkeä idea:
 * - connect voi palauttaa -EINPROGRESS / restart-koodit.
 * - ÄLÄ tee handshakea tai session_createa "SYN_SENT"-tilassa.
 * - Käynnistä worker joka pollaa sk_state ja aloittaa vasta kun ESTABLISHED.
 */
int stcp_proto_socket_ops_connect(struct socket *sock,
                                  struct sockaddr *uaddr,
                                  int addr_len,
                                  int flags)
{
    int ret;

    // delegoi OIKEIN: orig_inet_ops->connect (ei inet_stream_connect!)
    ret = orig_inet_ops->connect(sock, uaddr, addr_len, flags);
    SDBG("CONNECT: orig_inet_ops->connect ret=%d state=%d err=%d",
         ret,
         sock && sock->sk ? sock->sk->sk_state : -1,
         sock && sock->sk ? sock->sk->sk_err : 0);

    if (ret < 0 && ret != -EINPROGRESS)
        return ret;

    // handshake käyntiin connectissa (sun vaatimus)
    if (sock && sock->sk) {
        struct stcp_sock *st = stcp_struct_get_or_alloc_st_from_sk(sock->sk);
        if (!st) return -ENOMEM;

        if (!st->session) {
            int r = rust_exported_session_create((void **)&st->session, sock->sk);
            if (r < 0) {
                st->status |= STCP_STATUS_SOCKET_FATAL_ERROR;
                return r;
            }
        }

        stcp_handshake_start(st, START_HANDSHAKE_SIDE_CLIENT);
    }

    SDBG("Connect: Final ret: %d", ret);
    return ret; /* 0 tai -EINPROGRESS */
}

/*
 * ACCEPT: anna inet_acceptin hoitaa blokkaus + newsock->sk graft.
 * Kun ret==0 -> newsock->sk on valid ja voit attachata STCP:n ja käynnistää workkerin.
 */
int stcp_proto_socket_ops_accept(struct socket *sock,
                                 struct socket *newsock,
                                 struct proto_accept_arg *arg)
{
    int ret;

    // delegoi OIKEIN: orig_inet_ops->accept (ei inet_accept!)
    ret = orig_inet_ops->accept(sock, newsock, arg);

    if (ret == -ERESTARTSYS || ret == -EAGAIN) {
        SDBG("ACCEPT: not ready? orig_inet_ops->accept ret=%d", ret);
        return ret;
    }

    if (ret < 0) {
        SDBG("ACCEPT: fail orig_inet_ops->accept ret=%d", ret);
        return ret;
    }

    if (!newsock || !newsock->sk) {
        SDBG("ACCEPT: newsock->sk NULL after accept");
        return -EINVAL;
    }

    // attach STCP + start server handshake täällä
    struct stcp_sock *st = stcp_struct_get_or_alloc_st_from_sk(newsock->sk);
    if (!st) return -ENOMEM;

    if (!st->session) {
        int r = rust_exported_session_create((void **)&st->session, newsock->sk);
        if (r < 0) {
            st->status |= STCP_STATUS_SOCKET_FATAL_ERROR;
            return r;
        }
    }
    
    st->transport = newsock;
    stcp_handshake_start(st, START_HANDSHAKE_SIDE_SERVER);
    SDBG("ACCEPT: OK newsock_sk=%px transport=%px transport_sk=%px st=%px",
        newsock->sk, st->transport, st->transport->sk, st);

//        SDBG("ACCEPT: OK newsock_sk=%px transport=%px transport_sk=%px st=%px",
//        newsock->sk, st->transport, st->transport->sk, st);

    return 0;
}

int stcp_proto_socket_ops_release(struct socket *sock)
{
    struct stcp_sock *st = (sock && sock->sk) ? sock->sk->sk_user_data : NULL;

    SDBG("RELEASE sock=%px sk=%px st=%px", sock, sock ? sock->sk : NULL, st);

    if (st)
        stcp_struct_free_st(st);

    return orig_inet_ops->release(sock);

}
#endif
