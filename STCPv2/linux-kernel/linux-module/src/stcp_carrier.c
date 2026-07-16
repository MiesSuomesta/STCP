// SPDX-License-Identifier: GPL-2.0

#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/in.h>
#include <linux/kthread.h>
#include <linux/net.h>
#include <linux/refcount.h>
#include <linux/slab.h>
#include <linux/socket.h>

#include <net/net_namespace.h>
#include <net/sock.h>

#include "stcp_carrier.h"

#define STCP_CARRIER_RX_BUFFER_SIZE (128 * 1024)

struct stcp_carrier {
	enum stcp_carrier_kind kind;
	struct socket *socket;
	struct task_struct *receiver;
	struct stcp_carrier *parent;
	refcount_t refs;

	void *rust_ctx;
	void *owner;

	struct sockaddr_storage peer;
	bool has_peer;
	bool listening;
	bool connected;
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

static void stcp_carrier_free_root(struct stcp_carrier *carrier)
{
	if (carrier->receiver) {
		kthread_stop(carrier->receiver);
		carrier->receiver = NULL;
	}

	if (carrier->socket) {
		sock_release(carrier->socket);
		carrier->socket = NULL;
	}

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
	u8 *buffer;

	buffer = kmalloc(STCP_CARRIER_RX_BUFFER_SIZE, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	while (!kthread_should_stop()) {
		struct sockaddr_storage peer;
		struct msghdr message = {
			.msg_flags = MSG_DONTWAIT,
			.msg_name = &peer,
			.msg_namelen = sizeof(peer),
		};
		struct kvec vector = {
			.iov_base = buffer,
			.iov_len = STCP_CARRIER_RX_BUFFER_SIZE,
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
			STCP_CARRIER_RX_BUFFER_SIZE,
			MSG_DONTWAIT
		);

		if (ret == -EAGAIN || ret == -EWOULDBLOCK) {
			msleep_interruptible(2);
			continue;
		}
		if (ret == 0) {
			if (carrier->kind == STCP_CARRIER_TCP)
				break;
			msleep_interruptible(2);
			continue;
		}
		if (ret < 0) {
			if (ret != -EINTR)
				pr_err_ratelimited("stcp: carrier recv failed: %d\n", ret);
			msleep_interruptible(10);
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
		if (ret)
			pr_err_ratelimited("stcp: Rust carrier receive failed: %d\n", ret);

		stcp_kernel_wake_recv(carrier->owner);
	}

	kfree_sensitive(buffer);
	return 0;
}

static int stcp_carrier_start_receiver(struct stcp_carrier *carrier)
{
	if (!carrier || !carrier->socket)
		return -EINVAL;
	if (carrier->receiver)
		return 0;

	carrier->receiver = kthread_run(
		stcp_receiver_thread,
		carrier,
		"stcp-rx/%p",
		carrier
	);
	if (IS_ERR(carrier->receiver)) {
		int ret = PTR_ERR(carrier->receiver);
		carrier->receiver = NULL;
		return ret;
	}
	return 0;
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
	    !listener->socket || !child_rust_ctx || !peer_port)
		return ERR_PTR(-EINVAL);

	child = kzalloc(sizeof(*child), GFP_KERNEL);
	if (!child)
		return ERR_PTR(-ENOMEM);

	refcount_inc(&listener->refs);
	child->kind = STCP_CARRIER_UDP;
	child->socket = listener->socket;
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
		kfree(carrier);
		stcp_carrier_put_root(parent);
		return;
	}

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

	if (!carrier)
		return -EINVAL;
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
	if (!carrier)
		return -EINVAL;

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

	if (!carrier)
		return -EINVAL;
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
	return stcp_carrier_start_receiver(carrier);
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
	refcount_set(&child->refs, 1);

	ret = stcp_carrier_start_receiver(child);
	if (ret) {
		stcp_carrier_destroy(child);
		return ret;
	}
	*out_child = child;
	return 0;
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

	if (!carrier || !carrier->socket)
		return -EINVAL;
	if (!carrier->connected)
		return -ENOTCONN;
	if (!data && len)
		return -EINVAL;

	if (carrier->kind == STCP_CARRIER_UDP) {
		int ret;
		if (!carrier->has_peer)
			return -ENOTCONN;
		message.msg_name = &carrier->peer;
		message.msg_namelen = sizeof(struct sockaddr_in);
		vector.iov_base = (void *)data;
		vector.iov_len = len;
		ret = kernel_sendmsg(carrier->socket, &message, &vector, 1, len);
		if (ret < 0)
			return ret;
		return (size_t)ret == len ? ret : -EIO;
	}

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
