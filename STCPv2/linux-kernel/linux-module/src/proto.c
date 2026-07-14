#include <linux/module.h>
#include <linux/net.h>
#include <net/sock.h>
#include "stcp.h"

extern const struct proto_ops stcp_proto_ops;

static struct proto stcp_proto = {
    .name     = "STCP",
    .owner    = THIS_MODULE,
    .obj_size = sizeof(struct stcp_sock),
};

static int stcp_create(struct net *net, struct socket *sock, int protocol, int kern)
{
    struct sock *sk;
    struct stcp_sock *ssk;

    if (sock->type != SOCK_STREAM)
        return -ESOCKTNOSUPPORT;
    if (protocol != 0 && protocol != STCP_PROTO_ID)
        return -EPROTONOSUPPORT;

    sk = sk_alloc(net, PF_STCP, GFP_KERNEL, &stcp_proto, kern);
    if (!sk)
        return -ENOMEM;

    sock_init_data(sock, sk);
    sock->ops = &stcp_proto_ops;
    sk->sk_protocol = STCP_PROTO_ID;
    sk->sk_state = TCP_CLOSE;

    ssk = stcp_sk(sk);
    init_waitqueue_head(&ssk->accept_wq);
    ssk->rust_ctx = stcp_rust_create(STCP_PROTO_ID);
    if (!ssk->rust_ctx) {
        sk_free(sk);
        return -ENOMEM;
    }
    return 0;
}

static const struct net_proto_family stcp_family = {
    .family = PF_STCP,
    .create = stcp_create,
    .owner  = THIS_MODULE,
};

int stcp_proto_register(void)
{
    int ret = proto_register(&stcp_proto, 1);
    if (ret)
        return ret;
    ret = sock_register(&stcp_family);
    if (ret)
        proto_unregister(&stcp_proto);
    return ret;
}

void stcp_proto_unregister(void)
{
    sock_unregister(PF_STCP);
    proto_unregister(&stcp_proto);
}
