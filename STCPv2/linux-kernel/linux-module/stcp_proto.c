/*
 * Replace/adapt your stcp_create() with this protocol-number selection.
 */

#include <linux/err.h>
#include <linux/errno.h>
#include <linux/net.h>
#include <linux/slab.h>

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

/*
 * Keep your existing proto registration/unregistration and child allocation.
 *
 * In child allocation initialize:
 *
 *     child->carrier = NULL;
 */
