#include <linux/net.h>
#include <linux/uio.h>
#include <net/sock.h>
#include "stcp.h"
#include "stcp_rust.h"

static int stcp_release(struct socket *sock)
{
    struct sock *sk = sock ? sock->sk : NULL;
    struct stcp_sock *ssk;
    if (!sk)
        return 0;
    ssk = stcp_sk(sk);
    wake_up_interruptible_all(&ssk->accept_wq);
    if (ssk->rust_ctx) {
        stcp_rust_release(ssk->rust_ctx);
        ssk->rust_ctx = NULL;
    }
    sock_orphan(sk);
    sock->sk = NULL;
    sk_free(sk);
    return 0;
}

static int stcp_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
    struct sockaddr_in *sin = (struct sockaddr_in *)uaddr;
    struct stcp_sock *ssk = stcp_sk(sock->sk);
    if (addr_len < sizeof(*sin) || sin->sin_family != AF_INET)
        return -EINVAL;
    return stcp_rust_bind(ssk->rust_ctx, sin->sin_addr.s_addr, sin->sin_port);
}

static int stcp_listen(struct socket *sock, int backlog)
{
    struct stcp_sock *ssk = stcp_sk(sock->sk);
    if (backlog < 0) return -EINVAL;
    if (backlog > STCP_MAX_BACKLOG) backlog = STCP_MAX_BACKLOG;
    return stcp_rust_listen(ssk->rust_ctx, backlog);
}

static int stcp_connect(struct socket *sock, struct sockaddr *uaddr,
                        int addr_len, int flags)
{
    struct sockaddr_in *sin = (struct sockaddr_in *)uaddr;
    struct stcp_sock *ssk = stcp_sk(sock->sk);
    if (addr_len < sizeof(*sin) || sin->sin_family != AF_INET)
        return -EINVAL;
    return stcp_rust_connect(ssk->rust_ctx, sin->sin_addr.s_addr,
                             sin->sin_port, flags);
}

static int stcp_accept(struct socket *sock, struct socket *newsock,
                       struct proto_accept_arg *arg)
{
    struct stcp_sock *listener = stcp_sk(sock->sk);
    struct sock *newsk;
    struct stcp_sock *new_ssk;
    void *accepted = NULL;
    int flags = arg ? arg->flags : 0;
    int ret;

    for (;;) {
        ret = stcp_rust_accept(listener->rust_ctx, &accepted, flags);
        if (ret != -EAGAIN && ret != -EWOULDBLOCK)
            break;
        if (flags & O_NONBLOCK)
            return -EAGAIN;
        ret = wait_event_interruptible(listener->accept_wq,
            stcp_rust_accept(listener->rust_ctx, &accepted, O_NONBLOCK) == 0);
        if (ret)
            return ret;
        if (accepted)
            break;
    }
    if (ret)
        return ret;

    newsk = sk_alloc(sock_net(sock->sk), PF_STCP, GFP_KERNEL,
                     sock->sk->sk_prot, 0);
    if (!newsk) {
        stcp_rust_release(accepted);
        return -ENOMEM;
    }
    sock_init_data(newsock, newsk);
    newsock->ops = sock->ops;
    new_ssk = stcp_sk(newsk);
    init_waitqueue_head(&new_ssk->accept_wq);
    new_ssk->rust_ctx = accepted;
    return 0;
}

static int stcp_sendmsg(struct socket *sock, struct msghdr *msg, size_t len)
{
    struct stcp_sock *ssk = stcp_sk(sock->sk);
    u8 *buf;
    ssize_t ret;
    if (!len) return 0;
    buf = kmalloc(len, GFP_KERNEL);
    if (!buf) return -ENOMEM;
    if (!copy_from_iter_full(buf, len, &msg->msg_iter)) {
        kfree(buf);
        return -EFAULT;
    }
    ret = stcp_rust_send(ssk->rust_ctx, buf, len, msg->msg_flags);
    kfree(buf);
    return ret;
}

static int stcp_recvmsg(struct socket *sock, struct msghdr *msg, size_t len,
                        int flags)
{
    struct stcp_sock *ssk = stcp_sk(sock->sk);
    u8 *buf;
    ssize_t ret;
    if (!len) return 0;
    buf = kmalloc(len, GFP_KERNEL);
    if (!buf) return -ENOMEM;
    ret = stcp_rust_recv(ssk->rust_ctx, buf, len, flags);
    if (ret > 0 && copy_to_iter(buf, ret, &msg->msg_iter) != ret)
        ret = -EFAULT;
    kfree(buf);
    return ret;
}

static int stcp_shutdown(struct socket *sock, int how)
{
    stcp_rust_shutdown(stcp_sk(sock->sk)->rust_ctx, how);
    return 0;
}

const struct proto_ops stcp_proto_ops = {
    .family        = PF_STCP,
    .owner         = THIS_MODULE,
    .release       = stcp_release,
    .bind          = stcp_bind,
    .connect       = stcp_connect,
    .socketpair    = sock_no_socketpair,
    .accept        = stcp_accept,
    .getname       = sock_no_getname,
    .poll          = sock_no_poll,
    .ioctl         = sock_no_ioctl,
    .listen        = stcp_listen,
    .shutdown      = stcp_shutdown,
    .setsockopt    = sock_no_setsockopt,
    .getsockopt    = sock_no_getsockopt,
    .sendmsg       = stcp_sendmsg,
    .recvmsg       = stcp_recvmsg,
    .mmap          = sock_no_mmap,
};
