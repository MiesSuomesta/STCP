#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/in.h>
#include <linux/jiffies.h>
#include <linux/log2.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/socket.h>
#include <linux/sockptr.h>
#include <linux/uio.h>

#include <net/sock.h>

#include "stcp.h"
#include "stcp_carrier.h"
#include "stcp_proto.h"
#include "stcp_rust_ffi.h"
#include "stcp_socket.h"
#include "stcp_users.h"

/*
 * Keep these adapter declarations local as well as in the public headers.
 * This protects out-of-tree builds from stale generated include trees and
 * makes implicit-declaration failures impossible during incremental builds.
 */
bool stcp_carrier_is_udp(const struct stcp_carrier *carrier);
void *stcp_rust_create_stream_accepted_child_ptr(void *listener_ctx);
int stcp_rust_create_stream_accepted_child(void *listener_ctx, void **out_ctx);

#define STCP_IO_BUFFER_MAX (2 * 1024 * 1024)
#define STCP_CONNECT_TIMEOUT_MS 5000
#define STCP_SEND_READY_TIMEOUT_MS 5000


/*
 * Multi-megabyte kmalloc() allocations require physically contiguous pages
 * and become unreliable on fragmented/KASAN kernels.  Grow each per-socket
 * scratch buffer only to the amount actually needed and use kvmalloc(), which
 * can transparently fall back to virtually contiguous memory.
 *
 * Caller must hold the corresponding tx_lock/rx_lock.
 */
static int stcp_ensure_io_buffer(
	u8 **buffer,
	size_t *capacity,
	size_t needed
)
{
	u8 *new_buffer;

	if (*buffer && *capacity >= needed)
		return 0;

	/* Grow geometrically to avoid repeated realloc/copy cycles as applications
	 * move from smoke-test sized I/O to multi-megabyte throughput buffers. */
	needed = roundup_pow_of_two(max_t(size_t, needed, PAGE_SIZE));
	needed = min_t(size_t, needed, STCP_IO_BUFFER_MAX);

	new_buffer = kvmalloc(needed, GFP_KERNEL | __GFP_NOWARN);
	if (!new_buffer)
		return -ENOMEM;

	if (*buffer)
		kvfree_sensitive(*buffer, *capacity);

	*buffer = new_buffer;
	*capacity = needed;
	return 0;
}

static int stcp_release(struct socket *sock)
{
	struct stcp_carrier *carrier;
	struct stcp_sock *ssk;
	struct sock *sk;
	void *rust_ctx;
	struct stcp_reliability_stats stats;
	u8 *tx_buffer;
	u8 *rx_buffer;
	size_t tx_buffer_size;
	size_t rx_buffer_size;

	if (!sock)
		return -EINVAL;

	sk = sock->sk;
	if (!sk)
		return 0;

	ssk = stcp_sk(sk);

	/* Remove it from /proc/stcp/users before freeing the socket. */
	stcp_user_unregister(ssk);

	/* Prevent new operations from finding this socket during teardown. */
	sock->sk = NULL;

	stcp_stop_retransmit_work(ssk);

	/*
	 * Send protocol CLOSE before detaching the Rust context or stopping the
	 * carrier.  close(fd) normally reaches .release directly without an
	 * explicit shutdown(2), so omitting this leaves the accepted peer blocked
	 * in recv() forever and leaks churn handlers/file descriptors.
	 *
	 * shutdown() is idempotent in Rust: if userspace already called it, this
	 * becomes a no-op.  The carrier is still fully alive here, therefore the
	 * CLOSE frame can be queued synchronously before teardown begins.
	 */
	if (READ_ONCE(ssk->rust_ctx))
		stcp_rust_shutdown(READ_ONCE(ssk->rust_ctx), SHUT_RDWR);

	/*
	 * stcp_rust_shutdown() queues CLOSE synchronously through the carrier.
	 * Do not impose a fixed sleep on every close: carrier teardown already
	 * waits for in-flight UDP sendmsg calls, and TCP kernel_sendmsg has
	 * returned before shutdown returns.  A fixed delay accumulates thousands
	 * of closing sockets during churn and lowers connections/second.
	 */

	/* Detach pointers exactly once, including concurrent error teardown. */
	rust_ctx = xchg(&ssk->rust_ctx, NULL);
	carrier = xchg(&ssk->carrier, NULL);

	if (rust_ctx &&
	    stcp_rust_get_reliability_stats(rust_ctx, &stats) == 0 &&
	    stats.sent_frames) {
		pr_debug(
			"stcp: reliability srtt=%ums rttvar=%ums rto=%ums "
			"sent=%llu acked=%llu retx=%llu dup=%llu reorder=%llu "
			"timeouts=%llu samples=%llu\n",
			stats.srtt_ms,
			stats.rttvar_ms,
			stats.rto_ms,
			stats.sent_frames,
			stats.acknowledged_frames,
			stats.retransmitted_frames,
			stats.duplicate_frames,
			stats.reordered_frames,
			stats.timeout_failures,
			stats.rtt_samples
		);
	}

	if (rust_ctx) {
		/*
		 * Detach every C pointer from Rust before stopping/freeing the
		 * carrier.  Rust release is a local teardown and sends no frames.
		 */
		stcp_rust_set_owner(rust_ctx, NULL);
		stcp_rust_set_carrier(rust_ctx, NULL);
	}

	/*
	 * Root destroy stops the receiver immediately, even if UDP children keep
	 * an inert reference to the root.  Therefore no callback can touch the
	 * Rust context after this returns.
	 */
	if (carrier)
		stcp_carrier_destroy(carrier);

	if (rust_ctx)
		stcp_rust_release(rust_ctx);

	tx_buffer = xchg(&ssk->tx_buffer, NULL);
	tx_buffer_size = xchg(&ssk->tx_buffer_size, 0);
	rx_buffer = xchg(&ssk->rx_buffer, NULL);
	rx_buffer_size = xchg(&ssk->rx_buffer_size, 0);
	if (tx_buffer)
		kvfree_sensitive(tx_buffer, tx_buffer_size);
	if (rx_buffer)
		kvfree_sensitive(rx_buffer, rx_buffer_size);

	wake_up_interruptible_all(&ssk->accept_wq);
	wake_up_interruptible_all(&ssk->recv_wq);

	sock_orphan(sk);
	sk_common_release(sk);

	return 0;
}

static int stcp_bind(
	struct socket *sock,
	struct sockaddr *addr,
	int addr_len
)
{
	struct sockaddr_in *sin;
	struct stcp_sock *ssk;
	int ret;

	if (!sock || !sock->sk || !addr)
		return -EINVAL;

	if (addr_len < sizeof(*sin))
		return -EINVAL;

	sin = (struct sockaddr_in *)addr;
	if (sin->sin_family != AF_INET)
		return -EAFNOSUPPORT;

	ssk = stcp_sk(sock->sk);
	if (!ssk->rust_ctx || !ssk->carrier)
		return -EINVAL;

	pr_info("stcp: bind enter sk=%px ctx=%px carrier=%px addr=%pI4:%u raw_addr=0x%08x raw_port=0x%04x\n",
		sock->sk, ssk->rust_ctx, ssk->carrier, &sin->sin_addr.s_addr,
		ntohs(sin->sin_port), (__force u32)sin->sin_addr.s_addr,
		(__force u16)sin->sin_port);
	ret = stcp_carrier_bind(
		ssk->carrier,
		(__force u32)sin->sin_addr.s_addr,
		(__force u16)sin->sin_port
	);

	pr_info("stcp: bind carrier result sk=%px carrier=%px ret=%d\n", sock->sk, ssk->carrier, ret);
	if (ret)
		return ret;

	ret = stcp_rust_bind(
		ssk->rust_ctx,
		(__force u32)sin->sin_addr.s_addr,
		(__force u16)sin->sin_port
	);
	pr_info("stcp: bind rust result sk=%px ctx=%px ret=%d\n", sock->sk, ssk->rust_ctx, ret);
	return ret;
}

static int stcp_listen(struct socket *sock, int backlog)
{
	struct stcp_sock *ssk;
	int ret;

	if (!sock || !sock->sk)
		return -EINVAL;

	ssk = stcp_sk(sock->sk);
	if (!ssk->rust_ctx || !ssk->carrier)
		return -EINVAL;

	pr_info("stcp: listen enter sk=%px ctx=%px carrier=%px backlog=%d\n",
		sock->sk, ssk->rust_ctx, ssk->carrier, backlog);
	ret = stcp_carrier_listen(
		ssk->carrier,
		backlog
	);

	pr_info("stcp: listen carrier result sk=%px carrier=%px ret=%d\n", sock->sk, ssk->carrier, ret);
	if (ret)
		return ret;

	ret = stcp_rust_listen(
		ssk->rust_ctx,
		backlog
	);
	pr_info("stcp: listen rust result sk=%px ctx=%px ret=%d\n", sock->sk, ssk->rust_ctx, ret);
	return ret;
}

static int stcp_connect(
	struct socket *sock,
	struct sockaddr *addr,
	int addr_len,
	int flags
)
{
	struct sockaddr_in *sin;
	struct stcp_sock *ssk;
	int ret;

	if (!sock || !sock->sk || !addr)
		return -EINVAL;

	if (addr_len < sizeof(*sin))
		return -EINVAL;

	sin = (struct sockaddr_in *)addr;
	if (sin->sin_family != AF_INET)
		return -EAFNOSUPPORT;

	ssk = stcp_sk(sock->sk);
	if (!ssk->rust_ctx || !ssk->carrier)
		return -EINVAL;

	pr_info("stcp: connect request sk=%px ctx=%px carrier=%px dst=%pI4:%u raw_addr=0x%08x raw_port=0x%04x flags=0x%x\n",
		sock->sk, ssk->rust_ctx, ssk->carrier, &sin->sin_addr.s_addr,
		ntohs(sin->sin_port), (__force u32)sin->sin_addr.s_addr,
		(__force u16)sin->sin_port, flags);
	ret = stcp_carrier_connect(
		ssk->carrier,
		(__force u32)sin->sin_addr.s_addr,
		(__force u16)sin->sin_port,
		flags
	);

	pr_info("stcp: connect carrier result sk=%px carrier=%px ret=%d\n", sock->sk, ssk->carrier, ret);
	if (ret)
		return ret;

	pr_info("stcp: connect enter sk=%px ctx=%px carrier=%px flags=0x%x\n",
		sock->sk, ssk->rust_ctx, ssk->carrier, flags);

	ret = stcp_rust_connect(
		ssk->rust_ctx,
		(__force u32)sin->sin_addr.s_addr,
		(__force u16)sin->sin_port,
		flags
	);		

	pr_info("stcp: connect rust result ctx=%px ret=%d\n",
		ssk->rust_ctx, ret);
	if (ret)
		return ret;

	/*
	 * The receiver must be running before the handshake can emit frames.
	 * Otherwise the peer may send PublicKey/HandshakeDone and application data
	 * while no carrier RX worker exists, leaving recv() asleep forever.
	 * At this point connect(), carrier attachment and owner setup are complete.
	 */
	ret = stcp_carrier_start_receiver_thread(ssk->carrier);
	if (ret)
		return ret;

	ret = stcp_rust_start_handshake(ssk->rust_ctx);
	if (ret) {
		stcp_carrier_shutdown(ssk->carrier, SHUT_RDWR);
		return ret;
	}

	/*
	 * connect() must not report success before the cryptographic handshake is
	 * complete.  Returning 0 while can_send()==0 creates a race where the first
	 * userspace send is accepted as a connected socket operation but returns
	 * -EAGAIN, leaving an echo peer blocked in recv().
	 *
	 * The accept side runs independently and carrier RX wakes recv_wq whenever
	 * handshake state changes.  Nonblocking connect keeps normal socket
	 * semantics and reports -EINPROGRESS until poll() observes Ready.
	 */
	stcp_start_retransmit_work(ssk);

	if (stcp_rust_is_connected(ssk->rust_ctx) > 0) {
		sock->state = SS_CONNECTED;
		pr_info("stcp: connect ready immediately ctx=%px\n", ssk->rust_ctx);
		return 0;
	}

	if (flags & O_NONBLOCK) {
		sock->state = SS_CONNECTING;
		pr_info("stcp: connect in progress ctx=%px\n", ssk->rust_ctx);
		return -EINPROGRESS;
	}

	ret = wait_event_interruptible_timeout(
		ssk->recv_wq,
		stcp_rust_is_connected(ssk->rust_ctx) > 0,
		msecs_to_jiffies(STCP_CONNECT_TIMEOUT_MS)
	);
	if (ret < 0) {
		pr_info("stcp: connect interrupted ctx=%px ret=%d\n",
			ssk->rust_ctx, ret);
		return ret;
	}
	if (ret == 0) {
		pr_info("stcp: connect handshake timeout ctx=%px\n", ssk->rust_ctx);
		stcp_carrier_shutdown(ssk->carrier, SHUT_RDWR);
		return -ETIMEDOUT;
	}

	sock->state = SS_CONNECTED;
	pr_info("stcp: connect handshake complete ctx=%px\n", ssk->rust_ctx);
	return 0;
}

static int stcp_accept(
	struct socket *sock,
	struct socket *newsock,
	struct proto_accept_arg *arg
)
{
	struct stcp_sock *listener;
	struct stcp_sock *child;
	struct sock *newsk;
	void *accepted_ctx = NULL;
	int flags = arg ? arg->flags : 0;
	int ret;

	if (!sock || !sock->sk || !newsock)
		return -EINVAL;

	listener = stcp_sk(sock->sk);
	if (!listener->rust_ctx || !listener->carrier)
		return -EINVAL;

	pr_info("stcp: accept enter listener=%px ctx=%px carrier=%px flags=0x%x\n",
		listener, listener->rust_ctx, listener->carrier, flags);

	/*
	 * UDP children are created by Rust when the first datagram arrives and are
	 * still delivered through the logical accept queue. Stream carriers are
	 * different: accept the real TCP socket first and construct a dedicated
	 * server child around it. This avoids the old circular dependency where
	 * Rust waited for a child that could only be created after kernel_accept().
	 */
	if (stcp_carrier_is_udp(listener->carrier)) {
		for (;;) {
			ret = stcp_rust_accept(listener->rust_ctx, &accepted_ctx, flags);
			if (ret != -EAGAIN)
				break;
			if (flags & O_NONBLOCK)
				return -EAGAIN;
			ret = wait_event_interruptible(
				listener->accept_wq,
				stcp_rust_has_accept(listener->rust_ctx) > 0
			);
			if (ret)
				return ret;
		}
		if (ret)
			return ret;
	} else {
		accepted_ctx = stcp_rust_create_stream_accepted_child_ptr(
			listener->rust_ctx
		);
		pr_info("stcp: accept provisional child listener_ctx=%px child_ctx=%px via=direct-ptr\n",
			listener->rust_ctx, accepted_ctx);
		if (!accepted_ctx)
			return -ENOMEM;
	}

	if (!accepted_ctx)
		return -EIO;

	newsk = stcp_alloc_child_sock(sock_net(sock->sk), newsock);
	if (IS_ERR(newsk)) {
		stcp_rust_release(accepted_ctx);
		return PTR_ERR(newsk);
	}

	child = stcp_sk(newsk);
	child->rust_ctx = accepted_ctx;
	pr_info("stcp: accept rust child listener_ctx=%px child=%px child_ctx=%px\n",
		listener->rust_ctx, child, child->rust_ctx);

	ret = stcp_carrier_accept(
		listener->carrier,
		child->rust_ctx,
		child,
		&child->carrier,
		flags & O_NONBLOCK ? O_NONBLOCK : 0
	);
	pr_info("stcp: accept carrier result listener=%px child=%px carrier=%px ret=%d\n",
		listener, child, child->carrier, ret);
	if (ret)
		goto fail_child;

	stcp_rust_set_carrier(child->rust_ctx, child->carrier);
	stcp_rust_set_owner(child->rust_ctx, child);

	ret = stcp_carrier_start_receiver_thread(child->carrier);
	pr_info("stcp: accept receiver start child=%px carrier=%px ret=%d\n",
		child, child->carrier, ret);
	if (ret)
		goto fail_carrier;

	ret = stcp_rust_start_handshake(child->rust_ctx);
	pr_info("stcp: accept handshake start child=%px ctx=%px ret=%d\n",
		child, child->rust_ctx, ret);
	if (ret)
		goto fail_carrier;

	/* Do not expose the accepted socket before both peers completed crypto. */
	if (stcp_rust_is_connected(child->rust_ctx) <= 0) {
		ret = wait_event_interruptible_timeout(
			child->recv_wq,
			stcp_rust_is_connected(child->rust_ctx) > 0,
			msecs_to_jiffies(STCP_CONNECT_TIMEOUT_MS)
		);
		if (ret < 0)
			goto fail_carrier;
		if (ret == 0) {
			pr_info("stcp: accept handshake timeout child=%px ctx=%px\n",
				child, child->rust_ctx);
			ret = -ETIMEDOUT;
			goto fail_carrier;
		}
	}

	stcp_start_retransmit_work(child);
	stcp_user_register(child);
	newsock->state = SS_CONNECTED;
	pr_info("stcp: accept complete child=%px ctx=%px carrier=%px owner=%px\n",
		child, child->rust_ctx, child->carrier, child);
	return 0;

fail_carrier:
	stcp_rust_set_owner(child->rust_ctx, NULL);
	stcp_rust_set_carrier(child->rust_ctx, NULL);
	if (child->carrier) {
		stcp_carrier_destroy(child->carrier);
		child->carrier = NULL;
	}
fail_child:
	if (child->rust_ctx) {
		stcp_rust_release(child->rust_ctx);
		child->rust_ctx = NULL;
	}
	newsock->sk = NULL;
	sk_free(newsk);
	return ret;
}

static int stcp_sendmsg(
	struct socket *sock,
	struct msghdr *msg,
	size_t len
)
{
	struct stcp_sock *ssk;
	u8 *buffer;
	size_t total = 0;
	ssize_t ret = 0;

	if (!sock || !sock->sk || !msg)
		return -EINVAL;
	if (!len)
		return 0;

	ssk = stcp_sk(sock->sk);
	if (!ssk->rust_ctx)
		return -EINVAL;

	mutex_lock(&ssk->tx_lock);

	while (total < len) {
		size_t chunk = min_t(size_t, len - total, STCP_IO_BUFFER_MAX);
		int wait_ret;

		ret = stcp_ensure_io_buffer(
			&ssk->tx_buffer,
			&ssk->tx_buffer_size,
			chunk
		);
		if (ret < 0)
			break;
		buffer = ssk->tx_buffer;

		if (!copy_from_iter_full(buffer, chunk, &msg->msg_iter)) {
			ret = total ? (ssize_t)total : -EFAULT;
			break;
		}


		for (;;) {
			ret = stcp_rust_send(ssk->rust_ctx, buffer, chunk,
						 msg->msg_flags);
			if (ret != -EAGAIN)
				break;
			if (msg->msg_flags & MSG_DONTWAIT)
				break;
			wait_ret = wait_event_interruptible_timeout(
				ssk->recv_wq,
				stcp_rust_can_send(ssk->rust_ctx, chunk) > 0,
				msecs_to_jiffies(STCP_SEND_READY_TIMEOUT_MS)
			);
			if (wait_ret < 0) {
				ret = wait_ret;
				break;
			}
			if (wait_ret == 0) {
				ret = -ETIMEDOUT;
				break;
			}
		}

		if (ret < 0) {
			if (total)
				ret = total;
			break;
		}
		if (!ret)
			break;
		total += ret;
		if ((size_t)ret < chunk)
			break;
	}

	mutex_unlock(&ssk->tx_lock);
	return total ? (int)total : (int)ret;
}

static int stcp_recvmsg(
	struct socket *sock,
	struct msghdr *msg,
	size_t len,
	int flags
)
{
	struct stcp_sock *ssk;
	u8 *buffer;
	ssize_t ret;
	int wait_ret;

	if (!sock || !sock->sk || !msg)
		return -EINVAL;

	if (!len)
		return 0;

	/*
	 * SOCK_STREAM recv() may return fewer bytes than requested.
	 * Do not reject a large userspace buffer with -EMSGSIZE;
	 * cap only the temporary kernel allocation.
	 */
	len = min_t(size_t, len, STCP_IO_BUFFER_MAX);

	ssk = stcp_sk(sock->sk);
	if (!ssk->rust_ctx)
		return -EINVAL;

	mutex_lock(&ssk->rx_lock);
	ret = stcp_ensure_io_buffer(
		&ssk->rx_buffer,
		&ssk->rx_buffer_size,
		len
	);
	if (ret < 0) {
		mutex_unlock(&ssk->rx_lock);
		return ret;
	}
	buffer = ssk->rx_buffer;

	for (;;) {
		ret = stcp_rust_recv(
			ssk->rust_ctx,
			buffer,
			len,
			flags
		);


		if (ret != -EAGAIN)
			break;

		if (flags & MSG_DONTWAIT)
			break;


		wait_ret = wait_event_interruptible(
			ssk->recv_wq,
			stcp_rust_has_data(ssk->rust_ctx) != 0
		);

		if (wait_ret) {
			ret = wait_ret;
			break;
		}
	}

	if (ret > 0 &&
	    copy_to_iter(buffer, ret, &msg->msg_iter) != ret)
		ret = -EFAULT;

	mutex_unlock(&ssk->rx_lock);
	return ret;
}

static int stcp_shutdown(struct socket *sock, int how)
{
	struct stcp_sock *ssk;

	if (!sock || !sock->sk)
		return -EINVAL;

	ssk = stcp_sk(sock->sk);
	if (!ssk->rust_ctx)
		return -EINVAL;

	/* Send the protocol Close frame while the carrier is still alive. */
	stcp_rust_shutdown(ssk->rust_ctx, how);

	stcp_carrier_shutdown(
		ssk->carrier,
		how
	);
	wake_up_interruptible_all(&ssk->recv_wq);

	return 0;
}

static __poll_t stcp_poll(
	struct file *file,
	struct socket *sock,
	struct poll_table_struct *wait
)
{
	struct stcp_sock *ssk;
	__poll_t mask = 0;

	if (!sock || !sock->sk)
		return EPOLLERR;

	ssk = stcp_sk(sock->sk);
	if (!ssk->rust_ctx)
		return EPOLLERR;

	poll_wait(file, &ssk->accept_wq, wait);
	poll_wait(file, &ssk->recv_wq, wait);

	if (stcp_rust_has_accept(ssk->rust_ctx) > 0)
		mask |= EPOLLIN | EPOLLRDNORM;

	if (stcp_rust_has_data(ssk->rust_ctx) != 0)
		mask |= EPOLLIN | EPOLLRDNORM;

	if (stcp_rust_is_connected(ssk->rust_ctx) > 0)
		mask |= EPOLLOUT | EPOLLWRNORM;

	return mask;
}

static int stcp_setsockopt(
	struct socket *sock,
	int level,
	int optname,
	sockptr_t optval,
	unsigned int optlen
)
{
	return -ENOPROTOOPT;
}

static int stcp_getsockopt(
	struct socket *sock,
	int level,
	int optname,
	char *optval,
	int *optlen
)
{
	return -ENOPROTOOPT;
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
	.poll          = stcp_poll,
	.ioctl         = sock_no_ioctl,
	.gettstamp     = sock_gettstamp,
	.listen        = stcp_listen,
	.shutdown      = stcp_shutdown,
	.setsockopt    = stcp_setsockopt,
	.getsockopt    = stcp_getsockopt,
	.sendmsg       = stcp_sendmsg,
	.recvmsg       = stcp_recvmsg,
	.mmap          = sock_no_mmap,
};
