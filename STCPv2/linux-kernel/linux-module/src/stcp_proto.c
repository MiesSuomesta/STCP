#include <linux/errno.h>
#include <linux/module.h>
#include <linux/net.h>
#include <net/sock.h>

#include "stcp.h"
#include "stcp_proto.h"
#include "stcp_rust_ffi.h"
#include "stcp_socket.h"

struct proto stcp_proto = {
	.name = "STCP",
	.owner = THIS_MODULE,
	.obj_size = sizeof(struct stcp_sock),
};

static int stcp_create(struct net *net, struct socket *sock, int protocol, int kern)
{
	struct sock *sk;
	struct stcp_sock *ssk;

	if (!sock)
		return -EINVAL;
	if (sock->type != SOCK_STREAM)
		return -ESOCKTNOSUPPORT;
	if (protocol != 0 && protocol != STCP_PROTO_ID)
		return -EPROTONOSUPPORT;

	sk = sk_alloc(net, PF_STCP, GFP_KERNEL, &stcp_proto, kern);
	if (!sk)
		return -ENOMEM;

	sock_init_data(sock, sk);
	sock->ops = &stcp_proto_ops;
	sock->state = SS_UNCONNECTED;

	ssk = stcp_sk(sk);
	ssk->rust_ctx = stcp_rust_create((u8)protocol);
	if (!ssk->rust_ctx) {
		sock_orphan(sk);
		sock->sk = NULL;
		sk_free(sk);
		return -ENOMEM;
	}

	init_waitqueue_head(&ssk->accept_wq);
	init_waitqueue_head(&ssk->recv_wq);
	return 0;
}

static struct net_proto_family stcp_family_ops = {
	.family = PF_STCP,
	.create = stcp_create,
	.owner = THIS_MODULE,
};

struct sock *stcp_alloc_child_sock(struct net *net, struct socket *newsock)
{
	struct sock *newsk;
	struct stcp_sock *ssk;

	if (!newsock)
		return ERR_PTR(-EINVAL);

	newsk = sk_alloc(net, PF_STCP, GFP_KERNEL, &stcp_proto, 0);
	if (!newsk)
		return ERR_PTR(-ENOMEM);

	sock_init_data(newsock, newsk);
	newsock->ops = &stcp_proto_ops;
	newsock->state = SS_UNCONNECTED;

	ssk = stcp_sk(newsk);
	ssk->rust_ctx = NULL;
	init_waitqueue_head(&ssk->accept_wq);
	init_waitqueue_head(&ssk->recv_wq);
	return newsk;
}

int stcp_proto_register(void)
{
	int ret = proto_register(&stcp_proto, 1);
	if (ret)
		return ret;

	ret = sock_register(&stcp_family_ops);
	if (ret)
		proto_unregister(&stcp_proto);
	return ret;
}

void stcp_proto_unregister(void)
{
	sock_unregister(PF_STCP);
	proto_unregister(&stcp_proto);
}
