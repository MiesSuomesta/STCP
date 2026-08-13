// SPDX-License-Identifier: GPL-2.0

#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/in.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/net.h>
#include <linux/refcount.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/socket.h>
#include <linux/wait.h>
#include <linux/atomic.h>

#include <net/net_namespace.h>
#include <net/sock.h>
#include <linux/tcp.h>

#include "stcp_carrier.h"
#include "stcp_kernel_compat.h"
#include "stcp_test.h"

#define STCP_CARRIER_TCP_RX_BUFFER_SIZE (8 * 1024 * 1024)
#define STCP_CARRIER_UDP_RX_BUFFER_SIZE (64 * 1024)
#define STCP_TCP_SOCKET_BUFFER_SIZE (16 * 1024 * 1024)
#define STCP_UDP_SOCKET_BUFFER_SIZE (16 * 1024 * 1024)

static atomic64_t stcp_tcp_close_started = ATOMIC64_INIT(0);
static atomic64_t stcp_tcp_close_drained = ATOMIC64_INIT(0);
static atomic64_t stcp_tcp_close_drain_timeouts = ATOMIC64_INIT(0);
static atomic64_t stcp_tcp_close_fin_acked = ATOMIC64_INIT(0);
static atomic64_t stcp_tcp_close_fin_timeouts = ATOMIC64_INIT(0);

struct stcp_carrier {
	enum stcp_carrier_kind kind;
	u64 debug_id;
	atomic_t rx_callbacks;
	bool destroy_started;
	struct socket *socket;
	struct task_struct *receiver;
	struct stcp_carrier *parent;
	refcount_t refs;

	/* Serializes receiver start/stop and prevents stale task pointers. */
	struct mutex lifecycle_lock;
	struct completion stop_done;
	bool stopping;
	bool stopped;

	/*
	 * UDP children share the listener/root socket.  Do not hold
	 * lifecycle_lock across kernel_sendmsg(): doing so serializes every
	 * connection and makes pipelined bursts slower than stop-and-wait.
	 * stopping is published under lifecycle_lock; active_sends keeps the
	 * root socket alive until all in-flight sends have returned.
	 */
	atomic_t active_sends;
	wait_queue_head_t send_wait;

	void *rust_ctx;
	void *owner;

	struct sockaddr_storage peer;
	bool has_peer;
	bool listening;
	bool connected;

	/* Deterministic reliability test state, protected per carrier. */
	struct mutex test_lock;
	u8 *held_frame;
	size_t held_frame_len;
};

static atomic64_t stcp_carrier_debug_seq = ATOMIC64_INIT(0);

extern int stcp_rust_carrier_receive_from(
	void *rust_ctx,
	const u8 *data,
	size_t len,
	u32 peer_addr,
	u16 peer_port
);

extern void *stcp_rust_get_carrier(void *rust_ctx);

extern int stcp_rust_get_udp_peer(
	void *rust_ctx,
	u32 *out_addr,
	u16 *out_port
);

extern void stcp_kernel_wake_recv(void *owner);

static struct stcp_carrier *stcp_carrier_root(
	struct stcp_carrier *carrier
)
{
	if (!carrier)
		return NULL;

	return carrier->parent ? carrier->parent : carrier;
}



static void stcp_tune_tcp_socket(struct socket *socket)
{
	struct sock *sk;

	if (!socket || !socket->sk)
		return;

	sk = socket->sk;
	tcp_sock_set_nodelay(sk);

	/*
	 * STCP carries multi-megabyte encrypted frames.  The kernel defaults are
	 * often too small for eight pipelined 1 MiB messages and force frequent
	 * sender stalls.  Keep enough queued data per direction to cover the
	 * benchmark bandwidth-delay product without changing global sysctls.
	 */
	WRITE_ONCE(sk->sk_sndbuf, max_t(int, READ_ONCE(sk->sk_sndbuf),
		STCP_TCP_SOCKET_BUFFER_SIZE));
	WRITE_ONCE(sk->sk_rcvbuf, max_t(int, READ_ONCE(sk->sk_rcvbuf),
		STCP_TCP_SOCKET_BUFFER_SIZE));
}


static void stcp_tune_udp_socket(struct socket *socket)
{
	struct sock *sk;

	if (!socket || !socket->sk)
		return;

	sk = socket->sk;
	WRITE_ONCE(sk->sk_sndbuf, max_t(int, READ_ONCE(sk->sk_sndbuf),
		STCP_UDP_SOCKET_BUFFER_SIZE));
	WRITE_ONCE(sk->sk_rcvbuf, max_t(int, READ_ONCE(sk->sk_rcvbuf),
		STCP_UDP_SOCKET_BUFFER_SIZE));
}

static int stcp_sockaddr(
	u32 address,
	u16 port,
	struct sockaddr_storage *storage
)
{
	struct sockaddr_in *result;

	if (!storage)
		return -EINVAL;

	memset(storage, 0, sizeof(*storage));
	result = (struct sockaddr_in *)storage;
	result->sin_family = AF_INET;
	result->sin_addr.s_addr = (__force __be32)address;
	result->sin_port = (__force __be16)port;
	return 0;
}

static bool stcp_is_data_frame(const u8 *data, size_t len)
{
	if (!data || len < 5)
		return false;

	return data[0] == 'S' && data[1] == 'T' &&
	       data[2] == 'C' && data[3] == 'P' &&
	       (data[4] == 3 || data[4] == 4);
}

static ssize_t stcp_udp_send_one(
	struct stcp_carrier *carrier,
	const u8 *data,
	size_t len,
	int flags
)
{
	struct stcp_carrier *root = stcp_carrier_root(carrier);
	struct msghdr message;
	struct kvec vector;
	int ret;

	if (!root || !data)
		return -EINVAL;

	memset(&message, 0, sizeof(message));
	message.msg_flags = flags;
	message.msg_name = &carrier->peer;
	message.msg_namelen = sizeof(struct sockaddr_in);

	vector.iov_base = (void *)data;
	vector.iov_len = len;

	/*
	 * Grab a short-lived send reference while holding lifecycle_lock, then
	 * release the mutex before kernel_sendmsg().  The UDP socket itself is
	 * safe for concurrent sendmsg calls; keeping the lifecycle mutex held
	 * here unnecessarily serialized every accepted UDP child.
	 */
	mutex_lock(&root->lifecycle_lock);
	if (root->stopping || !root->socket) {
		mutex_unlock(&root->lifecycle_lock);
		return -ESHUTDOWN;
	}
	atomic_inc(&root->active_sends);
	mutex_unlock(&root->lifecycle_lock);

	ret = kernel_sendmsg(root->socket, &message, &vector, 1, len);
	if (ret >= 0 && (size_t)ret != len)
		ret = -EIO;

	if (atomic_dec_and_test(&root->active_sends))
		wake_up_all(&root->send_wait);

	return ret;
}

static bool stcp_carrier_stop_root(struct stcp_carrier *carrier)
{
	struct task_struct *receiver;
	struct socket *socket;
	bool wait_for_other = false;

	if (!carrier || carrier->parent)
		return false;

	pr_err("stcp-lifetime: CARRIER-STOP-ENTER cid=%llu carrier=%px ctx=%px owner=%px receiver=%px rx_active=%d stopping=%d stopped=%d pid=%d comm=%s\n",
	       READ_ONCE(carrier->debug_id), carrier, READ_ONCE(carrier->rust_ctx),
	       READ_ONCE(carrier->owner), READ_ONCE(carrier->receiver),
	       atomic_read(&carrier->rx_callbacks), READ_ONCE(carrier->stopping),
	       READ_ONCE(carrier->stopped), current->pid, current->comm);

	/*
	 * Exactly one caller performs shutdown.  Concurrent destroy/free paths
	 * wait for stop_done before they are allowed to release the root memory.
	 */
	mutex_lock(&carrier->lifecycle_lock);

	if (carrier->stopped) {
		mutex_unlock(&carrier->lifecycle_lock);
		return true;
	}

	if (carrier->stopping) {
		wait_for_other = true;
		receiver = NULL;
		socket = NULL;
	} else {
		carrier->stopping = true;
		receiver = carrier->receiver;
		carrier->receiver = NULL;
		socket = carrier->socket;
	}

	mutex_unlock(&carrier->lifecycle_lock);

	if (!wait_for_other)
		wait_event(carrier->send_wait,
			   atomic_read(&carrier->active_sends) == 0);

	if (wait_for_other) {
		wait_for_completion(&carrier->stop_done);
		return true;
	}

	/* Wake kernel_recvmsg() before joining the thread. */
	if (receiver && socket)
		kernel_sock_shutdown(socket, SHUT_RDWR);

	if (receiver && !IS_ERR(receiver)) {
		if (WARN_ON_ONCE(receiver == current)) {
			/* This path is forbidden; leak rather than free under RX. */
			mutex_lock(&carrier->lifecycle_lock);
			carrier->receiver = receiver;
			carrier->stopping = false;
			mutex_unlock(&carrier->lifecycle_lock);
			return false;
		}

		kthread_stop(receiver);
	}

	/* No receiver callback can run after kthread_stop() returns. */
	mutex_lock(&carrier->lifecycle_lock);
	carrier->rust_ctx = NULL;
	carrier->owner = NULL;
	carrier->connected = false;
	carrier->listening = false;
	carrier->has_peer = false;
	carrier->stopped = true;
	mutex_unlock(&carrier->lifecycle_lock);

	complete_all(&carrier->stop_done);
	pr_err("stcp-lifetime: CARRIER-STOP-EXIT cid=%llu carrier=%px ctx=%px owner=%px rx_active=%d stopped=%d pid=%d comm=%s\n",
	       READ_ONCE(carrier->debug_id), carrier, READ_ONCE(carrier->rust_ctx),
	       READ_ONCE(carrier->owner), atomic_read(&carrier->rx_callbacks),
	       READ_ONCE(carrier->stopped), current->pid, current->comm);
	WARN_ON_ONCE(atomic_read(&carrier->rx_callbacks) != 0);
	return true;
}

static void stcp_carrier_free_root(struct stcp_carrier *carrier)
{
	struct socket *socket;

	/* Idempotent: listener release normally stopped it already. */
	if (!stcp_carrier_stop_root(carrier))
		return;

	mutex_lock(&carrier->lifecycle_lock);
	socket = carrier->socket;
	carrier->socket = NULL;
	mutex_unlock(&carrier->lifecycle_lock);

	kfree_sensitive(carrier->held_frame);
	carrier->held_frame = NULL;
	carrier->held_frame_len = 0;

	if (socket)
		sock_release(socket);

	kfree(carrier);
}

static void stcp_carrier_put_root(struct stcp_carrier *carrier)
{
	if (carrier && refcount_dec_and_test(&carrier->refs))
		stcp_carrier_free_root(carrier);
}

static int stcp_receiver_thread(void *argument)
{
	struct stcp_carrier *carrier = argument;
	size_t buffer_size;
	u8 *buffer;

	buffer_size = carrier->kind == STCP_CARRIER_UDP
		? STCP_CARRIER_UDP_RX_BUFFER_SIZE
		: STCP_CARRIER_TCP_RX_BUFFER_SIZE;

	buffer = kvmalloc(buffer_size, GFP_KERNEL);
	if (!buffer) {
		/*
		 * Do not let the task disappear while carrier->receiver still
		 * references it.  Keep the kthread alive until its owner stops it.
		 */
		pr_err("stcp: carrier RX buffer allocation failed\n");
		while (!kthread_should_stop())
			schedule_timeout_interruptible(1);
		return -ENOMEM;
	}

	while (!kthread_should_stop()) {
		struct sockaddr_storage peer;
		struct msghdr message = {
			.msg_flags = 0,
			.msg_name = &peer,
			.msg_namelen = sizeof(peer),
		};
		struct kvec vector = {
			.iov_base = buffer,
			.iov_len = buffer_size,
		};
		u32 peer_addr = 0;
		u16 peer_port = 0;
		ssize_t received_len;
		int ret;

		memset(&peer, 0, sizeof(peer));
		ret = kernel_recvmsg(
			carrier->socket,
			&message,
			&vector,
			1,
			buffer_size,
			0
		);

		if (ret <= 0) {
			if (kthread_should_stop())
				break;

			/*
			 * TCP EOF and permanent disconnect errors are terminal.  The old
			 * code kept one kthread and its RX buffer alive forever for every
			 * closed stress-test connection.
			 */
			if (carrier->kind == STCP_CARRIER_TCP &&
			    (ret == 0 || ret == -ESHUTDOWN || ret == -ENOTCONN ||
			     ret == -ECONNRESET || ret == -EPIPE)) {
				/*
				 * Never let the receiver task exit on its own.
				 * carrier->receiver is consumed later by kthread_stop();
				 * returning here would leave a stale task pointer once the
				 * kthread bookkeeping has been released. Park until the
				 * owner performs the authoritative stop.
				 */
				while (!kthread_should_stop())
					schedule_timeout_interruptible(HZ / 10 ?: 1);
				break;
			}

			if (ret < 0 && ret != -EINTR && ret != -ERESTARTSYS &&
			    ret != -EAGAIN)
				pr_err_ratelimited("stcp: carrier recv failed: %d\n", ret);

			/* Only transient errors retry; yield instead of spinning. */
			schedule_timeout_interruptible(1);
			continue;
		}

		received_len = ret;

		if (peer.ss_family == AF_INET) {
			struct sockaddr_in *sin = (struct sockaddr_in *)&peer;
			peer_addr = (__force u32)sin->sin_addr.s_addr;
			peer_port = (__force u16)sin->sin_port;
		}


		{
			void *callback_ctx = READ_ONCE(carrier->rust_ctx);
			void *callback_owner = READ_ONCE(carrier->owner);
			int active = atomic_inc_return(&carrier->rx_callbacks);

			pr_err("stcp-lifetime: RX-CB-ENTER cid=%llu carrier=%px ctx=%px owner=%px stopped=%d stopping=%d destroy=%d active=%d len=%zd pid=%d comm=%s\n",
			       READ_ONCE(carrier->debug_id), carrier, callback_ctx,
			       callback_owner, READ_ONCE(carrier->stopped),
			       READ_ONCE(carrier->stopping), READ_ONCE(carrier->destroy_started),
			       active, received_len, current->pid, current->comm);

			if (unlikely(READ_ONCE(carrier->destroy_started) || !callback_ctx))
				pr_err("STCP-LIFETIME-BUG: RX callback after detach/teardown cid=%llu carrier=%px ctx=%px owner=%px destroy=%d stopping=%d\n",
				       READ_ONCE(carrier->debug_id), carrier, callback_ctx,
				       callback_owner, READ_ONCE(carrier->destroy_started),
				       READ_ONCE(carrier->stopping));

			if (callback_ctx)
				ret = stcp_rust_carrier_receive_from(
					callback_ctx, buffer, (size_t)received_len,
					peer_addr, peer_port);
			else
				ret = -ESHUTDOWN;

			active = atomic_dec_return(&carrier->rx_callbacks);
			pr_err("stcp-lifetime: RX-CB-EXIT cid=%llu carrier=%px ctx_now=%px owner_now=%px active=%d ret=%d pid=%d comm=%s\n",
			       READ_ONCE(carrier->debug_id), carrier,
			       READ_ONCE(carrier->rust_ctx), READ_ONCE(carrier->owner),
			       active, ret, current->pid, current->comm);
		}

		cond_resched();
		if (ret)
			pr_err_ratelimited("stcp: Rust carrier receive failed: %d\n", ret);

		/* Rust queue_to_context() performs the normal wake.  Keep only the
		 * pre-attachment fallback used during handshake/accept. */
		if (!carrier->rust_ctx)
			stcp_kernel_wake_recv(carrier->owner);
	}

	kvfree_sensitive(buffer, buffer_size);
	return 0;
}

static int stcp_carrier_start_receiver(struct stcp_carrier *carrier)
{
	struct task_struct *receiver;
	int ret = 0;

	if (!carrier || !carrier->socket)
		return -EINVAL;

	mutex_lock(&carrier->lifecycle_lock);

	if (carrier->stopping) {
		ret = -ESHUTDOWN;
		goto out_unlock;
	}

	if (carrier->receiver)
		goto out_unlock;

	receiver = kthread_run(
		stcp_receiver_thread,
		carrier,
		"stcp-rx/%p",
		carrier
	);

	if (IS_ERR(receiver)) {
		ret = PTR_ERR(receiver);
		goto out_unlock;
	}

	carrier->receiver = receiver;

 out_unlock:
	mutex_unlock(&carrier->lifecycle_lock);
	return ret;
}

struct stcp_carrier *stcp_carrier_create(
	enum stcp_carrier_kind kind,
	void *rust_ctx,
	void *owner
)
{
	struct stcp_carrier *carrier;
	int socket_type;
	int protocol;
	int ret;

	carrier = kzalloc(sizeof(*carrier), GFP_KERNEL);
	if (!carrier)
		return ERR_PTR(-ENOMEM);

	carrier->kind = kind;
	carrier->debug_id = (u64)atomic64_inc_return(&stcp_carrier_debug_seq);
	atomic_set(&carrier->rx_callbacks, 0);
	carrier->destroy_started = false;
	carrier->rust_ctx = rust_ctx;
	carrier->owner = owner;
	refcount_set(&carrier->refs, 1);
	mutex_init(&carrier->lifecycle_lock);
	init_completion(&carrier->stop_done);
	atomic_set(&carrier->active_sends, 0);
	init_waitqueue_head(&carrier->send_wait);
	mutex_init(&carrier->test_lock);

	switch (kind) {
	case STCP_CARRIER_TCP:
		socket_type = SOCK_STREAM;
		protocol = IPPROTO_TCP;
		break;
	case STCP_CARRIER_UDP:
		socket_type = SOCK_DGRAM;
		protocol = IPPROTO_UDP;
		break;
	default:
		kfree(carrier);
		return ERR_PTR(-EPROTONOSUPPORT);
	}

	ret = sock_create_kern(&init_net, AF_INET, socket_type, protocol, &carrier->socket);
	if (ret) {
		kfree(carrier);
		return ERR_PTR(ret);
	}
	if (kind == STCP_CARRIER_TCP)
		stcp_tune_tcp_socket(carrier->socket);
	else
		stcp_tune_udp_socket(carrier->socket);
	return carrier;
}

struct stcp_carrier *stcp_carrier_create_udp_child(
	struct stcp_carrier *listener,
	void *child_rust_ctx,
	u32 peer_addr,
	u16 peer_port
)
{
	struct stcp_carrier *child;

	if (!listener || listener->kind != STCP_CARRIER_UDP ||
	    !child_rust_ctx || !peer_port)
		return ERR_PTR(-EINVAL);

	child = kzalloc(sizeof(*child), GFP_KERNEL);
	if (!child)
		return ERR_PTR(-ENOMEM);

	/*
	 * Acquire the parent reference only while the listener is live.  This
	 * prevents accept racing root teardown from resurrecting a zero refcount.
	 */
	mutex_lock(&listener->lifecycle_lock);
	if (listener->stopping || !listener->socket ||
	    !refcount_inc_not_zero(&listener->refs)) {
		mutex_unlock(&listener->lifecycle_lock);
		kfree(child);
		return ERR_PTR(-ESHUTDOWN);
	}
	mutex_unlock(&listener->lifecycle_lock);
	mutex_init(&child->lifecycle_lock);
	init_completion(&child->stop_done);
	atomic_set(&child->active_sends, 0);
	init_waitqueue_head(&child->send_wait);
	mutex_init(&child->test_lock);
	child->kind = STCP_CARRIER_UDP;
	child->debug_id = (u64)atomic64_inc_return(&stcp_carrier_debug_seq);
	atomic_set(&child->rx_callbacks, 0);
	child->destroy_started = false;
	/* UDP children borrow the root socket through parent; never copy it. */
	child->socket = NULL;
	child->parent = listener;
	child->rust_ctx = child_rust_ctx;
	child->owner = NULL;
	child->connected = true;
	child->has_peer = true;
	stcp_sockaddr(peer_addr, peer_port, &child->peer);

	return child;
}

void stcp_carrier_set_owner(
	struct stcp_carrier *carrier,
	void *owner
)
{
	if (carrier)
		carrier->owner = owner;
}

int stcp_carrier_get_endpoints(
	struct stcp_carrier *carrier,
	u32 *local_addr,
	u16 *local_port,
	u32 *peer_addr,
	u16 *peer_port
)
{
	struct sockaddr_storage local = { 0 };
	struct sockaddr_storage peer = { 0 };
	struct sockaddr_in *local4 = (struct sockaddr_in *)&local;
	struct sockaddr_in *peer4 = (struct sockaddr_in *)&peer;
	int ret;

	if (!carrier || !carrier->socket || !local_addr || !local_port ||
	    !peer_addr || !peer_port)
		return -EINVAL;

	ret = kernel_getsockname(carrier->socket, (struct sockaddr *)&local);
	if (ret)
		return ret;
	ret = kernel_getpeername(carrier->socket, (struct sockaddr *)&peer);
	if (ret)
		return ret;
	if (local4->sin_family != AF_INET || peer4->sin_family != AF_INET)
		return -EAFNOSUPPORT;

	*local_addr = (__force u32)local4->sin_addr.s_addr;
	*local_port = (__force u16)local4->sin_port;
	*peer_addr = (__force u32)peer4->sin_addr.s_addr;
	*peer_port = (__force u16)peer4->sin_port;
	return 0;
}

void stcp_carrier_destroy(struct stcp_carrier *carrier)
{
	if (!carrier)
		return;

	WRITE_ONCE(carrier->destroy_started, true);
	pr_err("stcp-lifetime: CARRIER-DESTROY cid=%llu carrier=%px parent=%px ctx=%px owner=%px receiver=%px rx_active=%d pid=%d comm=%s\n",
	       READ_ONCE(carrier->debug_id), carrier, READ_ONCE(carrier->parent),
	       READ_ONCE(carrier->rust_ctx), READ_ONCE(carrier->owner),
	       READ_ONCE(carrier->receiver), atomic_read(&carrier->rx_callbacks),
	       current->pid, current->comm);

	if (carrier->parent) {
		struct stcp_carrier *parent = carrier->parent;
		carrier->parent = NULL;
		carrier->socket = NULL;
		kfree_sensitive(carrier->held_frame);
		carrier->held_frame = NULL;
		carrier->held_frame_len = 0;
		kfree(carrier);
		stcp_carrier_put_root(parent);
		return;
	}

	/*
	 * A root/listener release must stop RX immediately, before its Rust
	 * context is freed.  Child references keep only the inert root object
	 * alive until their own release.
	 */
	stcp_carrier_stop_root(carrier);
	stcp_carrier_put_root(carrier);
}

enum stcp_carrier_kind stcp_carrier_get_kind(
	const struct stcp_carrier *carrier
)
{
	return carrier ? carrier->kind : 0;
}

bool stcp_carrier_needs_reliability(const struct stcp_carrier *carrier)
{
	return carrier && carrier->kind == STCP_CARRIER_UDP;
}

int stcp_carrier_bind(struct stcp_carrier *carrier, u32 address, u16 port)
{
	struct sockaddr_storage socket_address;
	int ret;

	if (!carrier || carrier->parent)
		return -EINVAL;
	if (carrier->stopping || !carrier->socket)
		return -ESHUTDOWN;
	ret = stcp_sockaddr(address, port, &socket_address);
	if (ret)
		return ret;
	return kernel_bind(
		carrier->socket,
		STCP_KERNEL_SOCKADDR(&socket_address),
		sizeof(struct sockaddr_in)
	);
}

int stcp_carrier_listen(struct stcp_carrier *carrier, int backlog)
{
	if (!carrier || carrier->parent)
		return -EINVAL;
	if (carrier->stopping || !carrier->socket)
		return -ESHUTDOWN;

	carrier->listening = true;
	if (carrier->kind == STCP_CARRIER_UDP)
		return stcp_carrier_start_receiver(carrier);

	return kernel_listen(carrier->socket, backlog);
}

int stcp_carrier_connect(
	struct stcp_carrier *carrier,
	u32 address,
	u16 port,
	int flags
)
{
	struct sockaddr_storage socket_address;
	int ret;

	if (!carrier || carrier->parent)
		return -EINVAL;
	if (carrier->stopping || !carrier->socket)
		return -ESHUTDOWN;
	ret = stcp_sockaddr(address, port, &socket_address);
	if (ret)
		return ret;

	ret = kernel_connect(
		carrier->socket,
		STCP_KERNEL_SOCKADDR(&socket_address),
		sizeof(struct sockaddr_in),
		flags
	);
	if (ret)
		return ret;

	carrier->peer = socket_address;
	carrier->has_peer = true;
	/* The socket was already tuned immediately after creation. Repeating
	 * tcp_sock_set_nodelay() here races teardown after failed/aborted connects
	 * and was observed by KASAN as a slab-use-after-free. */
	carrier->connected = true;
	return 0;
}


int stcp_carrier_accept_unattached(
	struct stcp_carrier *listener,
	struct stcp_carrier **out_child,
	int flags
)
{
	struct stcp_carrier *child;
	struct socket *accepted = NULL;
	int ret;

	if (!listener || !out_child)
		return -EINVAL;
	*out_child = NULL;
	if (listener->kind != STCP_CARRIER_TCP)
		return -EOPNOTSUPP;

	ret = kernel_accept(listener->socket, &accepted, flags);
	if (ret)
		return ret;

	child = kzalloc(sizeof(*child), GFP_KERNEL);
	if (!child) {
		sock_release(accepted);
		return -ENOMEM;
	}

	child->kind = STCP_CARRIER_TCP;
	child->debug_id = (u64)atomic64_inc_return(&stcp_carrier_debug_seq);
	atomic_set(&child->rx_callbacks, 0);
	child->destroy_started = false;
	child->socket = accepted;
	child->connected = true;
	stcp_tune_tcp_socket(child->socket);
	refcount_set(&child->refs, 1);
	mutex_init(&child->lifecycle_lock);
	init_completion(&child->stop_done);
	atomic_set(&child->active_sends, 0);
	init_waitqueue_head(&child->send_wait);
	mutex_init(&child->test_lock);

	*out_child = child;
	return 0;
}

void stcp_carrier_attach(
	struct stcp_carrier *carrier,
	void *rust_ctx,
	void *owner
)
{
	if (!carrier)
		return;
	WRITE_ONCE(carrier->rust_ctx, rust_ctx);
	WRITE_ONCE(carrier->owner, owner);
}


int stcp_carrier_accept(
	struct stcp_carrier *listener,
	void *child_rust_ctx,
	void *child_owner,
	struct stcp_carrier **out_child,
	int flags
)
{
	struct stcp_carrier *child;
	int ret;

	if (!listener || !out_child)
		return -EINVAL;
	*out_child = NULL;

	if (listener->kind == STCP_CARRIER_UDP) {
		child = stcp_rust_get_carrier(child_rust_ctx);
		if (child) {
			stcp_carrier_set_owner(child, child_owner);
			*out_child = child;
			return 0;
		}

		/* Fallback for legacy children created before early UDP carrier setup. */
		{
			u32 peer_addr;
			u16 peer_port;

			ret = stcp_rust_get_udp_peer(child_rust_ctx, &peer_addr, &peer_port);
			if (ret)
				return ret;

			child = stcp_carrier_create_udp_child(
				listener, child_rust_ctx, peer_addr, peer_port);
			if (IS_ERR(child))
				return PTR_ERR(child);

			stcp_carrier_set_owner(child, child_owner);
			*out_child = child;
			return 0;
		}
	}

	ret = stcp_carrier_accept_unattached(listener, &child, flags);
	if (ret)
		return ret;

	stcp_carrier_attach(child, child_rust_ctx, child_owner);
	*out_child = child;
	return 0;
}

int stcp_carrier_start_receiver_thread(struct stcp_carrier *carrier)
{
	if (!carrier)
		return -EINVAL;
	if (carrier->kind == STCP_CARRIER_UDP && carrier->parent)
		return 0;
	return stcp_carrier_start_receiver(carrier);
}

ssize_t stcp_carrier_send(
	struct stcp_carrier *carrier,
	const u8 *data,
	size_t len,
	int flags
)
{
	struct msghdr message = { .msg_flags = flags | MSG_NOSIGNAL };
	struct kvec vector;
	size_t position = 0;
	ssize_t send_result = 0;

	if (!carrier)
		return -EINVAL;
	if (!carrier->connected)
		return -ENOTCONN;
	if (!data && len)
		return -EINVAL;

	if (carrier->kind == STCP_CARRIER_UDP) {
		ssize_t ret;
		bool data_frame;

		if (!carrier->has_peer)
			return -ENOTCONN;

		data_frame = stcp_is_data_frame(data, len);

		/* Normal production path: avoid the fault-injection mutex entirely. */
		if (!stcp_test_active())
			return stcp_udp_send_one(carrier, data, len, flags);

		/* Pretend success: reliability must recover intentionally lost frames. */
		if (data_frame &&
		    (stcp_test_should_drop_data() ||
		     stcp_test_should_drop_percent()))
			return (ssize_t)len;

		if (data_frame) {
			u32 delay_ms = stcp_test_take_delay_ms();

			if (delay_ms)
				msleep(delay_ms);
		}

		mutex_lock(&carrier->test_lock);

		/*
		 * Hold the first DATA frame. The next DATA frame is sent first and the
		 * held frame second, creating deterministic packet reordering.
		 */
		if (data_frame && !carrier->held_frame &&
		    stcp_test_should_reorder_data()) {
			carrier->held_frame = kmemdup(data, len, GFP_KERNEL);
			if (!carrier->held_frame) {
				mutex_unlock(&carrier->test_lock);
				return -ENOMEM;
			}
			carrier->held_frame_len = len;
			mutex_unlock(&carrier->test_lock);
			return (ssize_t)len;
		}

		ret = stcp_udp_send_one(carrier, data, len, flags);
		if (ret >= 0 && data_frame && carrier->held_frame) {
			ssize_t held_ret = stcp_udp_send_one(
				carrier,
				carrier->held_frame,
				carrier->held_frame_len,
				flags
			);
			kfree_sensitive(carrier->held_frame);
			carrier->held_frame = NULL;
			carrier->held_frame_len = 0;
			if (held_ret < 0)
				ret = held_ret;
		}

		if (ret >= 0 && data_frame && stcp_test_should_duplicate_data()) {
			ssize_t duplicate_ret = stcp_udp_send_one(carrier, data, len, flags);
			if (duplicate_ret < 0)
				ret = duplicate_ret;
		}

		mutex_unlock(&carrier->test_lock);
		return ret;
	}

	/*
	 * Protect the TCP socket against concurrent close.  release() waits for
	 * active_sends to reach zero before beginning the FIN sequence.
	 */
	mutex_lock(&carrier->lifecycle_lock);
	if (carrier->stopping || !carrier->socket) {
		mutex_unlock(&carrier->lifecycle_lock);
		return -ESHUTDOWN;
	}
	atomic_inc(&carrier->active_sends);
	mutex_unlock(&carrier->lifecycle_lock);

	while (position < len) {
		int ret;
		vector.iov_base = (void *)(data + position);
		vector.iov_len = len - position;
		ret = kernel_sendmsg(
			carrier->socket,
			&message,
			&vector,
			1,
			len - position
		);
		if (ret < 0) {
			send_result = ret;
			break;
		}
		if (ret == 0) {
			send_result = -EPIPE;
			break;
		}
		position += (size_t)ret;
	}

	if (atomic_dec_and_test(&carrier->active_sends))
		wake_up_all(&carrier->send_wait);

	if (send_result < 0)
		return send_result;
	return (ssize_t)position;
}

static bool stcp_tcp_tx_drained(struct sock *sk)
{
	if (!sk)
		return true;

	/* sk_wmem_alloc includes one reference owned by the socket itself. */
	return sk_wmem_alloc_get(sk) <= 1;
}

static bool stcp_tcp_fin_acknowledged(struct sock *sk)
{
	u8 state;

	if (!sk)
		return true;

	state = READ_ONCE(sk->sk_state);
	return state != TCP_FIN_WAIT1 &&
	       state != TCP_CLOSING &&
	       state != TCP_LAST_ACK;
}

int stcp_carrier_graceful_close(
	struct stcp_carrier *carrier,
	unsigned int drain_timeout_ms,
	unsigned int fin_timeout_ms
)
{
	struct socket *socket;
	struct sock *sk;
	long sends_idle;
	long drained;
	long fin_acked;
	int shutdown_ret;

	if (!carrier || carrier->parent || carrier->kind != STCP_CARRIER_TCP)
		return 0;

	mutex_lock(&carrier->lifecycle_lock);
	if (carrier->stopping || carrier->stopped || !carrier->socket ||
	    carrier->listening || !carrier->connected) {
		mutex_unlock(&carrier->lifecycle_lock);
		return 0;
	}
	socket = carrier->socket;
	sk = socket->sk;
	mutex_unlock(&carrier->lifecycle_lock);

	atomic64_inc(&stcp_tcp_close_started);

	/* First wait for all STCP kernel_sendmsg() callers to return. */
	sends_idle = wait_event_timeout(
		carrier->send_wait,
		atomic_read(&carrier->active_sends) == 0,
		msecs_to_jiffies(max_t(unsigned int, drain_timeout_ms, 1))
	);
	if (!sends_idle)
		pr_warn_ratelimited(
			"stcp: TCP close active-send timeout active_sends=%d\n",
			atomic_read(&carrier->active_sends)
		);

	/* Then wait for the TCP write queue to be acknowledged before FIN. */
	drained = wait_event_timeout(
		*sk_sleep(sk),
		stcp_tcp_tx_drained(sk),
		msecs_to_jiffies(max_t(unsigned int, drain_timeout_ms, 1))
	);
	if (drained) {
		atomic64_inc(&stcp_tcp_close_drained);
	} else {
		atomic64_inc(&stcp_tcp_close_drain_timeouts);
		pr_warn_ratelimited(
			"stcp: TCP close drain timeout state=%u wmem=%d active_sends=%d\n",
			READ_ONCE(sk->sk_state),
			sk_wmem_alloc_get(sk),
			atomic_read(&carrier->active_sends)
		);
	}

	shutdown_ret = kernel_sock_shutdown(socket, SHUT_WR);
	if (shutdown_ret && shutdown_ret != -ENOTCONN)
		pr_debug("stcp: TCP SHUT_WR returned %d\n", shutdown_ret);

	/* Wait only for FIN acknowledgement. FIN-WAIT-2 is acceptable here. */
	fin_acked = wait_event_timeout(
		*sk_sleep(sk),
		stcp_tcp_fin_acknowledged(sk),
		msecs_to_jiffies(max_t(unsigned int, fin_timeout_ms, 1))
	);
	if (fin_acked) {
		atomic64_inc(&stcp_tcp_close_fin_acked);
	} else {
		atomic64_inc(&stcp_tcp_close_fin_timeouts);
		pr_warn_ratelimited(
			"stcp: TCP FIN acknowledgement timeout state=%u wmem=%d; forcing shutdown\n",
			READ_ONCE(sk->sk_state),
			sk_wmem_alloc_get(sk)
		);
		kernel_sock_shutdown(socket, SHUT_RDWR);
	}

	pr_debug(
		"stcp: TCP close stats started=%lld drained=%lld drain_to=%lld fin_ack=%lld fin_to=%lld\n",
		(long long)atomic64_read(&stcp_tcp_close_started),
		(long long)atomic64_read(&stcp_tcp_close_drained),
		(long long)atomic64_read(&stcp_tcp_close_drain_timeouts),
		(long long)atomic64_read(&stcp_tcp_close_fin_acked),
		(long long)atomic64_read(&stcp_tcp_close_fin_timeouts)
	);

	return shutdown_ret;
}

void stcp_carrier_shutdown(struct stcp_carrier *carrier, int how)
{
	if (!carrier || !carrier->socket)
		return;
	if (carrier->kind == STCP_CARRIER_UDP)
		return;
	kernel_sock_shutdown(carrier->socket, how);
}
