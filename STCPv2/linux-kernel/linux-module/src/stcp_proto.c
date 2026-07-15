#include <linux/err.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>

#include <net/sock.h>

#include "stcp.h"
#include "stcp_carrier.h"
#include "stcp_proto.h"
#include "stcp_rust_ffi.h"
#include "stcp_socket.h"

static int stcp_protocol_to_carrier(
	int protocol,
	enum stcp_carrier_kind *kind
)
{
	if (!kind)
		return -EINVAL;

	switch (protocol) {
	case STCP_PROTO_DEFAULT:
	case STCP_PROTO_TCP:
		*kind = STCP_CARRIER_TCP;
		return 0;

	case STCP_PROTO_UDP:
		*kind = STCP_CARRIER_UDP;
		return 0;

	default:
		return -EPROTONOSUPPORT;
	}
}

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
	enum stcp_carrier_kind carrier_kind;
	struct stcp_sock *ssk;
	struct sock *sk;
	void *rust_ctx = NULL;
	int ret;

	if (!sock)
		return -EINVAL;

	if (sock->type != SOCK_STREAM)
		return -ESOCKTNOSUPPORT;

	ret = stcp_protocol_to_carrier(
		protocol,
		&carrier_kind
	);
	if (ret)
		return ret;

	sk = sk_alloc(
		net,
		PF_STCP,
		GFP_KERNEL,
		&stcp_proto,
		kern
	);
	if (!sk)
		return -ENOMEM;

	sock_init_data(sock, sk);
	sock->ops = &stcp_proto_ops;
	sock->state = SS_UNCONNECTED;

	ssk = stcp_sk(sk);
	ssk->rust_ctx = NULL;
	ssk->carrier = NULL;
	init_waitqueue_head(&ssk->accept_wq);
	init_waitqueue_head(&ssk->recv_wq);

	ret = stcp_rust_create(
		(u8)protocol,
		&rust_ctx
	);
	if (ret)
		goto error_release_sock;

	ssk->rust_ctx = rust_ctx;

	ssk->carrier = stcp_carrier_create(
		carrier_kind,
		ssk->rust_ctx,
		ssk
	);

	if (IS_ERR(ssk->carrier)) {
		ret = PTR_ERR(ssk->carrier);
		ssk->carrier = NULL;
		goto error_release_rust;
	}

	stcp_rust_set_owner(
		ssk->rust_ctx,
		ssk
	);

	stcp_rust_set_carrier(
		ssk->rust_ctx,
		ssk->carrier
	);

	return 0;

error_release_rust:
	stcp_rust_release(ssk->rust_ctx);
	ssk->rust_ctx = NULL;

error_release_sock:
	sock_orphan(sk);
	sock->sk = NULL;
	sk_free(sk);

	return ret;
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


/*
 * Keep your existing proto registration/unregistration and child allocation.
 *
 * In child allocation initialize:
 *
 *     child->carrier = NULL;
 */
