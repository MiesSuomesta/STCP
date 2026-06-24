// stcp_proto_ops.c
#include <linux/kernel.h>
#include <linux/net.h>
#include <linux/string.h>
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

#if USE_OWN_SOCK_OPTS

#define FORCE_TCP(sck) \
    do { if ((sck) &&                                           \
        ((sck)->sk) &&                                          \
        ((sck)->sk)->sk_protocol == IPPROTO_STCP) {             \
            SDBG("Forcing TCP on sock: %px, from %u => %u",     \
                (sck), ((sck)->sk)->sk_protocol, IPPROTO_TCP);  \
            ((sck)->sk)->sk_protocol = IPPROTO_TCP;             \
        }                                                       \
    } while (0)


const struct proto_ops *orig_inet_ops = &inet_stream_ops;
struct proto_ops stcp_stream_ops;

static int stcp_proto_socket_ops_sendmsg(struct socket *sock,
                                         struct msghdr *msg,
                                         size_t len);

static int stcp_proto_socket_ops_recvmsg(struct socket *sock,
                                         struct msghdr *msg,
                                         size_t len,
                                         int flags);


static inline bool stcp_is_restart_err(int ret)
{
    return ret == -ERESTARTSYS ||
           ret == -ERESTARTNOINTR ||
           ret == -ERESTARTNOHAND ||
           ret == -ERESTART_RESTARTBLOCK;
}

/* ------------------------------------------------------------------ */
/* BIND / LISTEN / CONNECT / ACCEPT (delegating)                       */
/* ------------------------------------------------------------------ */

int stcp_proto_socket_ops_bind(struct socket *sock,
                               struct sockaddr_unsized *uaddr,
                               int addr_len)
{
    int ret;

    if (!sock)
        return -EINVAL;
    
    /* TÄRKEÄ: ennen kuin SYN lähtee */
    FORCE_TCP(sock);

    ret = orig_inet_ops->bind(sock, uaddr, addr_len);
    if (ret < 0)
        return ret;

    if (sock->sk) {
        struct stcp_sock *st = stcp_struct_get_or_alloc_st_from_sk(sock->sk);
        if (st) {
            if (addr_len > 0 && addr_len <= sizeof(st->bind_addr)) {
                memcpy(&st->bind_addr, uaddr, addr_len);
                st->bind_addr_len = addr_len;
            }
            set_bit(STCP_FLAG_SOCKET_BOUND_BIT, &st->flags);
        }
    }

    return 0;
}

int stcp_proto_socket_ops_listen(struct socket *sock, int backlog)
{
    if (!sock)
        return -EINVAL;
    
    /* TÄRKEÄ: ennen kuin SYN lähtee */
    FORCE_TCP(sock);

    return orig_inet_ops->listen(sock, backlog);
}

int stcp_proto_socket_ops_connect(struct socket *sock,
                                  struct sockaddr_unsized *uaddr,
                                  int addr_len,
                                  int flags)
{
    if (!sock)
        return -EINVAL;

    /* TÄRKEÄ: ennen kuin SYN lähtee */
    FORCE_TCP(sock);

    return orig_inet_ops->connect(sock, uaddr, addr_len, flags);
}

int stcp_proto_socket_ops_accept(struct socket *sock,
                                 struct socket *newsock,
                                 struct proto_accept_arg *arg)
{
    if (!sock)
        return -EINVAL;

    return orig_inet_ops->accept(sock, newsock, arg);
}

/* ------------------------------------------------------------------ */
/* SEND / RECV (proto_ops layer, socket *)                             */
/* ------------------------------------------------------------------ */

static int stcp_tcp_sendmsg_bypass(struct stcp_sock *st,
                                   struct socket *sock,
                                   struct msghdr *msg,
                                   size_t len)
{
    if (!sock)
        return -EINVAL;

    return orig_inet_ops->sendmsg(sock, msg, len);
}

static int stcp_tcp_recvmsg_bypass(struct stcp_sock *st,
                                   struct socket *sock,
                                   struct msghdr *msg,
                                   size_t len,
                                   int flags)
{
    if (!sock)
        return -EINVAL;

    return orig_inet_ops->recvmsg(sock, msg, len, flags);
}

int stcp_proto_socket_ops_sendmsg(struct socket *sock,
                                  struct msghdr *msg,
                                  size_t len)
{
    struct sock *sk = sock ? sock->sk : NULL;
    struct stcp_sock *st = sk ? sk->sk_user_data : NULL;

#if STCP_SOCKET_BYPASS_ALL_IO
    return stcp_tcp_sendmsg_bypass(st, sock, msg, len);
#endif

    if (!sk)
        return -EINVAL;

    if (!st)
        return orig_inet_ops->sendmsg(sock, msg, len);

    DEBUG_INCOMING_STCP_STATUS(st);

    if (test_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags))
        return stcp_tcp_sendmsg_bypass(st, sock, msg, len);

    if (!stcp_state_hanshake_is_complete(st))
        return -EAGAIN;

    // Rusti lähettää plain => AES viestin piuhalle
    return stcp_rust_glue_proto_op_sendmsg(sk, msg, len);
}

int stcp_proto_socket_ops_recvmsg(struct socket *sock,
                                  struct msghdr *msg,
                                  size_t len,
                                  int flags)
{
    struct sock *sk = sock ? sock->sk : NULL;
    struct stcp_sock *st = sk ? sk->sk_user_data : NULL;

#if STCP_SOCKET_BYPASS_ALL_IO
    return stcp_tcp_recvmsg_bypass(st, sock, msg, len, flags);
#endif

    if (!sk)
        return -EINVAL;

    if (!st)
        return orig_inet_ops->recvmsg(sock, msg, len, flags);

    DEBUG_INCOMING_STCP_STATUS(st);

    if (test_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags))
        return stcp_tcp_recvmsg_bypass(st, sock, msg, len, flags);

    {
        int recv_len = 0;
        int ret = stcp_rust_glue_proto_op_recvmsg(sk, msg, len, flags, &recv_len);

        /*
         * Palautetaan proto_ops-muodossa:
         *  - <0: errno
         *  - >=0: bytes
         *
         * Sun glue palauttaa yleensä samat bytes kuin recv_len,
         * mutta pidetään tämä varmuuden vuoksi:
         */
        if (ret < 0)
            return ret;

        if (ret == 0)
            return recv_len;

        return ret;
    }
}

/* ------------------------------------------------------------------ */
/* RELEASE                                                            */
/* ------------------------------------------------------------------ */

int stcp_proto_socket_ops_release(struct socket *sock)
{
    struct sock *sk = sock ? sock->sk : NULL;
    struct stcp_sock *st = NULL;

    if (sk) {
        st = stcp_detach_once(sk);

        if (st && test_and_set_bit(STCP_FLAG_SOCKET_RELEASE_ENTERED_BIT,
                                   &st->flags))
            goto out;

        if (st) {
            stcp_detach_from_st(st);
            set_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags);
            stcp_struct_put_st(st);
        }
    }

out:
    return orig_inet_ops->release(sock);
}

/* ------------------------------------------------------------------ */
/* OPS SETUP                                                          */
/* ------------------------------------------------------------------ */

void stcp_socket_ops_setup(struct proto_ops *theOps)
{
    if (!theOps)
        return;

    memcpy(theOps, &inet_stream_ops, sizeof(*theOps));
    theOps->owner = THIS_MODULE;

#if USE_OWN_BIND
    theOps->bind = stcp_proto_socket_ops_bind;
#endif

#if USE_OWN_LISTEN
    theOps->listen = stcp_proto_socket_ops_listen;
#endif

#if USE_OWN_CONNECT
    theOps->connect = stcp_proto_socket_ops_connect;
#endif

#if USE_OWN_ACCEPT
    theOps->accept = stcp_proto_socket_ops_accept;
#endif

#if USE_OWN_RELEASE
    theOps->release = stcp_proto_socket_ops_release;
#endif

#if USE_OWN_SEND_MSG
    theOps->sendmsg = stcp_proto_socket_ops_sendmsg;
#endif

#if USE_OWN_RECV_MSG
    theOps->recvmsg = stcp_proto_socket_ops_recvmsg;
#endif
}

#endif /* USE_OWN_SOCK_OPTS */
