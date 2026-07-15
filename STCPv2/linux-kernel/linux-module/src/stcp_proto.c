#include <linux/errno.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>

#include <net/sock.h>

#include "stcp.h"
#include "stcp_proto.h"
#include "stcp_rust_ffi.h"
#include "stcp_socket.h"

static int stcp_proto_hash(struct sock *sk)
{
	return 0;
}

static void stcp_proto_unhash(struct sock *sk)
{
	/* STCP does not use a kernel socket hash table yet. */
}

static void stcp_proto_destroy(struct sock *sk)
{
	/* Rust context is released by stcp_release(). */
}


#define STCP_RETRANSMIT_INTERVAL_MS 100

static void stcp_retransmit_workfn(struct work_struct *work)
{
	struct stcp_sock *ssk;

	ssk = container_of(
		to_delayed_work(work),
		struct stcp_sock,
		retransmit_work
	);

	if (!READ_ONCE(ssk->rust_ctx))
		return;

	if (stcp_rust_tick(ssk->rust_ctx) > 0) {
		schedule_delayed_work(
			&ssk->retransmit_work,
			msecs_to_jiffies(STCP_RETRANSMIT_INTERVAL_MS)
		);
	} else {
		WRITE_ONCE(ssk->retransmit_work_started, false);
	}
}

void stcp_start_retransmit_work(struct stcp_sock *ssk)
{
	if (!ssk || !ssk->rust_ctx || ssk->retransmit_work_started)
		return;

	ssk->retransmit_work_started = true;

	schedule_delayed_work(
		&ssk->retransmit_work,
		msecs_to_jiffies(STCP_RETRANSMIT_INTERVAL_MS)
	);
}

void stcp_stop_retransmit_work(struct stcp_sock *ssk)
{
	if (!ssk || !ssk->retransmit_work_started)
		return;

	cancel_delayed_work_sync(&ssk->retransmit_work);
	ssk->retransmit_work_started = false;
}

struct proto stcp_proto = {
	.name     = "STCP",
	.owner    = THIS_MODULE,
	.obj_size = sizeof(struct stcp_sock),
	.hash     = stcp_proto_hash,
	.unhash   = stcp_proto_unhash,
	.destroy  = stcp_proto_destroy,
};

static int stcp_create(
	struct net *net,
	struct socket *sock,
	int protocol,
	int kern
)
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
	init_waitqueue_head(&ssk->accept_wq);
	init_waitqueue_head(&ssk->recv_wq);
	INIT_DELAYED_WORK(&ssk->retransmit_work, stcp_retransmit_workfn);
	ssk->retransmit_work_started = false;

	ssk->rust_ctx = stcp_rust_create((u8)protocol);
	if (!ssk->rust_ctx) {
		sock_orphan(sk);
		sock->sk = NULL;
		sk_free(sk);
		return -ENOMEM;
	}

	stcp_rust_set_owner(ssk->rust_ctx, ssk);
	stcp_start_retransmit_work(ssk);
	return 0;
}

static struct net_proto_family stcp_family_ops = {
	.family = PF_STCP,
	.create = stcp_create,
	.owner  = THIS_MODULE,
};

struct sock *stcp_alloc_child_sock(
	struct net *net,
	struct socket *newsock
)
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
	INIT_DELAYED_WORK(&ssk->retransmit_work, stcp_retransmit_workfn);
	ssk->retransmit_work_started = false;

	return newsk;
}

int stcp_proto_register(void)
{
	int ret;

	ret = proto_register(&stcp_proto, 1);
	if (ret)
		return ret;

	ret = sock_register(&stcp_family_ops);
	if (ret) {
		proto_unregister(&stcp_proto);
		return ret;
	}

	return 0;
}

void stcp_proto_unregister(void)
{
	sock_unregister(PF_STCP);
	proto_unregister(&stcp_proto);
}
