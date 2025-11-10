#include <linux/net.h>

// SPDX-License-Identifier: GPL
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/printk.h>
#include <net/sock.h>
#include <net/tcp.h>

#define USE_SAFEGUARD 1 
#include <stcp/kmod.h>
#include <stcp/sock_glue.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lauri Jakku / Paxsudos IT <lauri.jakku@paxsudos.fi>");
MODULE_DESCRIPTION("STCP: SecureTCP inner TCP helpers");

/*
 * STCP inner socket management
 * - one TCP socket per STCP socket
 * - NO global state
 */

int stcp_inner_create(struct stcp_sock *st)
{
    int err = 0;
    struct stcp_inner *in;

    stcp_sock_phase_set(st, STCPF_STATE_INIT);

    in = kzalloc(sizeof(*in), GFP_KERNEL);
    if (!in)
        return -ENOMEM;

    /* Luo sisäinen TCP (tms) socket: */
    err = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &in->sock);
    if (err) {
        kfree(in);
        return err;
    }

    /* Julkaise inner atomisesti: */
    WRITE_ONCE(st->inner, in);

    // Mark soket as READY and AES lauer to PUBLIC KEY mode..

    stcp_sock_phase_set(st, STCPF_STATE_READY); // Tämä on karkea tcp soketin tila, jota STCP vvain käyttää..
//    stcp_security_layer_phase_set(st, STCPF_PHASE_INIT); // Aloitetaan salattu juttelu aivan alusta ...

    return 0;
}

int stcp_inner_release(struct stcp_sock *st)
{
    CHECK_ST(st);

    if (!st->inner)
        return 0;

    if (st->inner->sock) {
        pr_info("stcp: releasing inner socket %px\n", st->inner->sock);
        sock_release(st->inner->sock);
    }

    return 0;
}

int stcp_inner_free(struct stcp_sock *st)
{
    CHECK_ST(st);

    if (!st->inner)
        return 0;

    kfree(st->inner);
    st->inner = NULL;
    return 0;
}

void stcp_inner_destroy(struct stcp_sock *st)
{
    CHECK_ST_VOID(st);
    stcp_inner_release(st);
    stcp_inner_free(st);
}

/* Bind inner */
int stcp_inner_bind(struct stcp_sock *st, struct sockaddr *uaddr, int addr_len)
{
    STCP_CHECK_INNER(st);
    return kernel_bind(st->inner->sock, uaddr, addr_len);
}

int stcp_inner_listen(struct stcp_sock *st, int backlog)
{
    STCP_CHECK_INNER(st);
    return kernel_listen(st->inner->sock, backlog);
}

int stcp_inner_connect(struct stcp_sock *st, struct sockaddr *addr,
                       int addr_len, int flags)
{
    STCP_CHECK_INNER(st);
    return kernel_connect(st->inner->sock, addr, addr_len, flags);
}

void stcp_inner_close(struct sock *sk, long timeout) {
    /* Tyhjä */
}

int stcp_inner_accept(struct stcp_sock *parent,
                      struct stcp_sock **out_child,
                      int flags)
{
    int err;
    struct socket *new_sock = NULL;

    STCP_CHECK_INNER(parent);

    pr_info("stcp_inner_accept: waiting...\n");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0)
    err = kernel_accept(parent->inner->sock, &new_sock, flags);
#else
    err = kernel_accept(parent->inner->sock, &new_sock, flags);
#endif

    if (err)
        return err;

    pr_info("stcp_inner_accept: got child sock=%px\n", new_sock);

    parent->accept_pending = new_sock;
    *out_child = NULL;

    return 0;
}

/* Assign a kernel socket to stcp_sock */
int stcp_inner_set(struct stcp_sock *st, struct socket *sock)
{
    CHECK_ST(st);
    if (!sock)
        return -EINVAL;

    if (!st->inner) {
        st->inner = kzalloc(sizeof(*st->inner), GFP_KERNEL);
        if (!st->inner)
            return -ENOMEM;
    }

    st->inner->sock = sock;
    return 0;
}

int stcp_inner_shutdown(struct stcp_sock *st, int how)
{
    STCP_CHECK_INNER(st);
    return kernel_sock_shutdown(st->inner->sock, how);
}

/* ------------  TEMPORARY STUBS (we'll fill later) -------------- */

int stcp_inner_sendmsg(struct stcp_sock *st, struct msghdr *msg, size_t len)
{
    pr_info("stcp: inner sendmsg STUB\n");
    return -EOPNOTSUPP;
}

int stcp_inner_recvmsg(struct stcp_sock *st, struct msghdr *msg,
                       size_t len, int flags)
{
    pr_info("stcp: inner recvmsg STUB\n");
    return -EOPNOTSUPP;
}

int stcp_inner_getname(struct stcp_sock *st, struct sockaddr *uaddr,
                       int *uaddr_len, int peer)
{
    pr_info("stcp: inner getname STUB\n");
    return -EOPNOTSUPP;
}

/* sockopt passthrough placeholder */
#ifdef SETSOCK
int stcp_setsockopt(struct socket *sock, int level, int optname,
                    sockptr_t optval, unsigned int optlen)
{
    pr_info("stcp: setsockopt STUB level=%d opt=%d\n", level, optname);
    return -EOPNOTSUPP;
}

int stcp_getsockopt(struct socket *sock, int level, int optname,
                    char __user *optval, int __user *optlen)
{
    pr_info("stcp: getsockopt STUB level=%d opt=%d\n", level, optname);
    return -EOPNOTSUPP;
}
#endif