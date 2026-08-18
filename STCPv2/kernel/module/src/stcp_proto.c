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
#include "stcp_users.h"

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


#define STCP_RETRANSMIT_INTERVAL_MS 20

static atomic64_t stcp_lifetime_seq = ATOMIC64_INIT(0);

static void stcp_retransmit_workfn(struct work_struct *work)
{
	struct stcp_sock *ssk;
	void *rust_ctx;
	int active;
	bool requeued = false;

	ssk = container_of(
		to_delayed_work(work),
		struct stcp_sock,
		retransmit_work
	);

	active = atomic_inc_return(&ssk->retransmit_callbacks);
	pr_err("stcp-lifetime: RETX-ENTER id=%llu ssk=%px sk=%px ctx=%px carrier=%px "
	       "started=%d teardown=%d active=%d pid=%d comm=%s\n",
	       READ_ONCE(ssk->lifetime_id), ssk, &ssk->sk,
	       READ_ONCE(ssk->rust_ctx), READ_ONCE(ssk->carrier),
	       READ_ONCE(ssk->retransmit_work_started),
	       READ_ONCE(ssk->teardown_started), active,
	       current->pid, current->comm);

	if (unlikely(READ_ONCE(ssk->teardown_started)))
		pr_err("STCP-LIFETIME-BUG: retransmit callback entered after teardown "
		       "id=%llu ssk=%px ctx=%px carrier=%px active=%d\n",
		       READ_ONCE(ssk->lifetime_id), ssk,
		       READ_ONCE(ssk->rust_ctx), READ_ONCE(ssk->carrier), active);

	/* release() clears the run flag before cancel_delayed_work_sync(). */
	if (!READ_ONCE(ssk->retransmit_work_started))
		goto out;

	rust_ctx = READ_ONCE(ssk->rust_ctx);
	if (!rust_ctx) {
		WRITE_ONCE(ssk->retransmit_work_started, false);
		goto out;
	}

	if (stcp_rust_tick(rust_ctx) > 0 &&
	    READ_ONCE(ssk->retransmit_work_started) &&
	    !READ_ONCE(ssk->teardown_started) &&
	    READ_ONCE(ssk->rust_ctx) == rust_ctx) {
		mod_delayed_work(
			system_dfl_wq,
			&ssk->retransmit_work,
			msecs_to_jiffies(STCP_RETRANSMIT_INTERVAL_MS)
		);
		requeued = true;
	} else {
		WRITE_ONCE(ssk->retransmit_work_started, false);
	}

out:
	active = atomic_dec_return(&ssk->retransmit_callbacks);
	pr_err("stcp-lifetime: RETX-EXIT id=%llu ssk=%px ctx=%px carrier=%px "
	       "started=%d teardown=%d active=%d requeued=%d pid=%d comm=%s\n",
	       READ_ONCE(ssk->lifetime_id), ssk,
	       READ_ONCE(ssk->rust_ctx), READ_ONCE(ssk->carrier),
	       READ_ONCE(ssk->retransmit_work_started),
	       READ_ONCE(ssk->teardown_started), active, requeued,
	       current->pid, current->comm);
}

void stcp_start_retransmit_work(struct stcp_sock *ssk)
{
	if (!ssk || !READ_ONCE(ssk->rust_ctx))
		return;

	/* Do not queue the same delayed work twice. */
	if (xchg(&ssk->retransmit_work_started, true))
		return;

	mod_delayed_work(
		system_dfl_wq,
		&ssk->retransmit_work,
		msecs_to_jiffies(STCP_RETRANSMIT_INTERVAL_MS)
	);
}

void stcp_stop_retransmit_work(struct stcp_sock *ssk)
{
	if (!ssk)
		return;

	/*
	 * Clear the run flag first.  A callback already executing on another CPU
	 * will then return without requeueing itself.  Always cancel synchronously:
	 * the flag may already be false while delayed_work is still pending.
	 */
	pr_err("stcp-lifetime: RETX-STOP-ENTER id=%llu ssk=%px active=%d pending=%d teardown=%d\n",
	       READ_ONCE(ssk->lifetime_id), ssk,
	       atomic_read(&ssk->retransmit_callbacks),
	       delayed_work_pending(&ssk->retransmit_work),
	       READ_ONCE(ssk->teardown_started));
	WRITE_ONCE(ssk->retransmit_work_started, false);
	cancel_delayed_work_sync(&ssk->retransmit_work);
	pr_err("stcp-lifetime: RETX-STOP-EXIT id=%llu ssk=%px active=%d pending=%d teardown=%d\n",
	       READ_ONCE(ssk->lifetime_id), ssk,
	       atomic_read(&ssk->retransmit_callbacks),
	       delayed_work_pending(&ssk->retransmit_work),
	       READ_ONCE(ssk->teardown_started));
	WARN_ON_ONCE(atomic_read(&ssk->retransmit_callbacks) != 0);
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
	/* Preserve the selected STCP wire protocol for accept/connect paths. */
	sk->sk_protocol = protocol;
	sock->ops = &stcp_proto_ops;
	sock->state = SS_UNCONNECTED;

	ssk = stcp_sk(sk);
	ssk->rust_ctx = NULL;
	ssk->carrier = NULL;
	init_waitqueue_head(&ssk->accept_wq);
	init_waitqueue_head(&ssk->recv_wq);
	INIT_DELAYED_WORK(&ssk->retransmit_work, stcp_retransmit_workfn);
	ssk->retransmit_work_started = false;
	ssk->lifetime_id = (u64)atomic64_inc_return(&stcp_lifetime_seq);
	ssk->teardown_started = false;
	atomic_set(&ssk->retransmit_callbacks, 0);
	pr_err("stcp-lifetime: SOCK-CREATE id=%llu ssk=%px sk=%px sock=%px pid=%d comm=%s\n",
	       ssk->lifetime_id, ssk, sk, sock, current->pid, current->comm);
	mutex_init(&ssk->tx_lock);
	mutex_init(&ssk->rx_lock);
	ssk->tx_buffer = NULL;
	ssk->tx_buffer_size = 0;
	ssk->rx_buffer = NULL;
	ssk->rx_buffer_size = 0;

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

	stcp_user_register(ssk);
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
	ssk->carrier = NULL;
	pr_err("stcp-debug: child-alloc newsock=%px newsk=%px ssk=%px "
	       "sk_security=%px sock_state=%d sk_state=%u pid=%d comm=%s\n",
	       newsock, newsk, ssk,
#ifdef CONFIG_SECURITY
	       READ_ONCE(newsk->sk_security),
#else
	       NULL,
#endif
	       READ_ONCE(newsock->state), READ_ONCE(newsk->sk_state),
	       current->pid, current->comm);
	init_waitqueue_head(&ssk->accept_wq);
	init_waitqueue_head(&ssk->recv_wq);
	INIT_DELAYED_WORK(&ssk->retransmit_work, stcp_retransmit_workfn);
	ssk->retransmit_work_started = false;
	ssk->lifetime_id = (u64)atomic64_inc_return(&stcp_lifetime_seq);
	ssk->teardown_started = false;
	atomic_set(&ssk->retransmit_callbacks, 0);
	pr_err("stcp-lifetime: CHILD-CREATE id=%llu ssk=%px sk=%px sock=%px pid=%d comm=%s\n",
	       ssk->lifetime_id, ssk, newsk, newsock, current->pid, current->comm);
	mutex_init(&ssk->tx_lock);
	mutex_init(&ssk->rx_lock);
	ssk->tx_buffer = NULL;
	ssk->tx_buffer_size = 0;
	ssk->rx_buffer = NULL;
	ssk->rx_buffer_size = 0;

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
