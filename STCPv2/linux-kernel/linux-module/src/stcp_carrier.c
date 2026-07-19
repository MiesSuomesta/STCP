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

#include <net/net_namespace.h>
#include <net/sock.h>
#include <linux/tcp.h>

#include "stcp_carrier.h"
#include "stcp_test.h"

#define STCP_CARRIER_TCP_RX_BUFFER_SIZE (2 * 1024 * 1024)
#define STCP_CARRIER_UDP_RX_BUFFER_SIZE (64 * 1024)

struct stcp_carrier {
	enum stcp_carrier_kind kind;
	struct socket *socket;
	struct task_struct *receiver;
	struct stcp_carrier *parent;
	refcount_t refs;

	/* Serializes receiver start/stop and prevents stale task pointers. */
	struct mutex lifecycle_lock;
	struct completion stop_done;
	bool stopping;
	bool stopped;

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
	 * Root shutdown and UDP sends are serialized by lifecycle_lock.  Child
	 * carriers never own a socket pointer; they always use the refcounted
	 * parent's socket while this lock is held.
	 */
	mutex_lock(&root->lifecycle_lock);

	if (root->stopping || !root->socket) {
		ret = -ESHUTDOWN;
		goto out_unlock;
	}

	ret = kernel_sendmsg(root->socket, &message, &vector, 1, len);
	if (ret >= 0 && (size_t)ret != len)
		ret = -EIO;

 out_unlock:
	mutex_unlock(&root->lifecycle_lock);
	return ret;
}

static bool stcp_carrier_stop_root(struct stcp_carrier *carrier)
{
	struct task_struct *receiver;
	struct socket *socket;
	bool wait_for_other = false;

	if (!carrier || carrier->parent)
		return false;

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

		if (peer.ss_family == AF_INET) {
			struct sockaddr_in *sin = (struct sockaddr_in *)&peer;
			peer_addr = (__force u32)sin->sin_addr.s_addr;
			peer_port = (__force u16)sin->sin_port;
		}

		ret = stcp_rust_carrier_receive_from(
			carrier->rust_ctx,
			buffer,
			(size_t)ret,
			peer_addr,
			peer_port
		);

		cond_resched();
		if (ret)
			pr_err_ratelimited("stcp: Rust carrier receive failed: %d\n", ret);

		/*
		 * Keep the carrier-side wake as a correctness fallback. During
		 * handshake/accept the Rust owner may not yet be attached, so the
		 * Rust wake can legitimately be a no-op. waitqueue_active() inside
		 * stcp_kernel_wake_recv() keeps this cheap when nobody sleeps.
		 */
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
	carrier->rust_ctx = rust_ctx;
	carrier->owner = owner;
	refcount_set(&carrier->refs, 1);
	mutex_init(&carrier->lifecycle_lock);
	init_completion(&carrier->stop_done);
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
		tcp_sock_set_nodelay(carrier->socket->sk);
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
	mutex_init(&child->test_lock);
	child->kind = STCP_CARRIER_UDP;
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

void stcp_carrier_destroy(struct stcp_carrier *carrier)
{
	if (!carrier)
		return;

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
		(struct sockaddr_unsized *)&socket_address,
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
		(struct sockaddr_unsized *)&socket_address,
		sizeof(struct sockaddr_in),
		flags
	);
	if (ret)
		return ret;

	carrier->peer = socket_address;
	carrier->has_peer = true;
	carrier->connected = true;
	return 0;
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
	struct socket *accepted = NULL;
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

	ret = kernel_accept(listener->socket, &accepted, flags);
	if (ret)
		return ret;

	child = kzalloc(sizeof(*child), GFP_KERNEL);
	if (!child) {
		sock_release(accepted);
		return -ENOMEM;
	}

	child->kind = STCP_CARRIER_TCP;
	child->socket = accepted;
	child->rust_ctx = child_rust_ctx;
	child->owner = child_owner;
	child->connected = true;
	tcp_sock_set_nodelay(child->socket->sk);
	refcount_set(&child->refs, 1);
	mutex_init(&child->lifecycle_lock);
	init_completion(&child->stop_done);
	mutex_init(&child->test_lock);

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
	struct msghdr message = { .msg_flags = flags };
	struct kvec vector;
	size_t position = 0;

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

	if (!carrier->socket)
		return -ESHUTDOWN;

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
		if (ret < 0)
			return ret;
		if (ret == 0)
			return -EPIPE;
		position += (size_t)ret;
	}
	return (ssize_t)position;
}

void stcp_carrier_shutdown(struct stcp_carrier *carrier, int how)
{
	if (!carrier || !carrier->socket)
		return;
	if (carrier->kind == STCP_CARRIER_UDP)
		return;
	kernel_sock_shutdown(carrier->socket, how);
}
