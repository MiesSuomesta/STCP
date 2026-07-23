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
void stcp_carrier_set_rust_ctx(struct stcp_carrier *carrier, void *rust_ctx);
void *stcp_rust_create_stream_accepted_child_ptr(void *listener_ctx);
int stcp_rust_create_stream_accepted_child(void *listener_ctx, void **out_ctx);

#define STCP_IO_BUFFER_MAX (2 * 1024 * 1024)
#define STCP_CONNECT_TIMEOUT_MS 30000
#define STCP_SEND_READY_TIMEOUT_MS 30000


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
	pr_info("stcp-debug-v7: connect receiver start begin sk=%px ctx=%px carrier=%px\n",
		sock->sk, ssk->rust_ctx, ssk->carrier);
	ret = stcp_carrier_start_receiver_thread(ssk->carrier);
	pr_info("stcp-debug-v7: connect receiver start result sk=%px carrier=%px ret=%d\n",
		sock->sk, ssk->carrier, ret);
	if (ret)
		return ret;

	pr_info("stcp-debug-v7: connect handshake start begin sk=%px ctx=%px carrier=%px connected=%d\n",
		sock->sk, ssk->rust_ctx, ssk->carrier,
		stcp_rust_is_connected(ssk->rust_ctx));
	ret = stcp_rust_start_handshake(ssk->rust_ctx);
	pr_info("stcp-debug-v7: connect handshake start result sk=%px ctx=%px ret=%d connected=%d\n",
		sock->sk, ssk->rust_ctx, ret,
		stcp_rust_is_connected(ssk->rust_ctx));
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
	struct stcp_sock *child = NULL;
	struct stcp_carrier *accepted_carrier = NULL;
	struct sock *newsk = NULL;
	void *accepted_ctx = NULL;
	int flags = arg ? arg->flags : 0;
	int ret;

	pr_info("stcp-debug-v7: ACCEPT ENTER sock=%px sk=%px newsock=%px flags=0x%x\n",
		sock, sock ? sock->sk : NULL, newsock, flags);

	if (!sock || !sock->sk || !newsock)
		return -EINVAL;

	listener = stcp_sk(sock->sk);
	pr_info("stcp-debug-v7: accept listener=%px ctx=%px carrier=%px udp=%d\n",
		listener, listener ? listener->rust_ctx : NULL,
		listener ? listener->carrier : NULL,
		listener && listener->carrier ? stcp_carrier_is_udp(listener->carrier) : -1);
	if (!listener || !listener->rust_ctx || !listener->carrier)
		return -EINVAL;

	if (stcp_carrier_is_udp(listener->carrier)) {
		for (;;) {
			pr_info("stcp-debug-v7: accept UDP rust_accept begin listener_ctx=%px flags=0x%x\n",
				listener->rust_ctx, flags);
			ret = stcp_rust_accept(listener->rust_ctx, &accepted_ctx, flags);
			pr_info("stcp-debug-v7: accept UDP rust_accept result listener_ctx=%px child_ctx=%px ret=%d\n",
				listener->rust_ctx, accepted_ctx, ret);
			if (ret != -EAGAIN)
				break;
			if (flags & O_NONBLOCK)
				return -EAGAIN;
			ret = wait_event_interruptible(listener->accept_wq,
				stcp_rust_has_accept(listener->rust_ctx) > 0);
			if (ret)
				return ret;
		}
		if (ret)
			return ret;
	} else {
		/* Accept the real TCP socket first. This removes the circular
		 * dependency where a Rust child was created before kernel_accept(). */
		pr_info("stcp-debug-v7: accept TCP carrier_accept begin listener=%px carrier=%px flags=0x%x\n",
			listener, listener->carrier, flags);
		ret = stcp_carrier_accept(listener->carrier, NULL, NULL,
			&accepted_carrier, flags & O_NONBLOCK ? O_NONBLOCK : 0);
		pr_info("stcp-debug-v7: accept TCP carrier_accept result listener=%px accepted_carrier=%px ret=%d\n",
			listener, accepted_carrier, ret);
		if (ret)
			return ret;

		accepted_ctx = stcp_rust_create_stream_accepted_child_ptr(listener->rust_ctx);
		pr_info("stcp-debug-v7: accept TCP rust child create listener_ctx=%px child_ctx=%px\n",
			listener->rust_ctx, accepted_ctx);
		if (!accepted_ctx) {
			stcp_carrier_destroy(accepted_carrier);
			return -ENOMEM;
		}
	}

	newsk = stcp_alloc_child_sock(sock_net(sock->sk), newsock);
	pr_info("stcp-debug-v7: accept alloc child newsock=%px newsk=%px err=%ld\n",
		newsock, IS_ERR(newsk) ? NULL : newsk,
		IS_ERR(newsk) ? PTR_ERR(newsk) : 0L);
	if (IS_ERR(newsk)) {
		if (accepted_carrier)
			stcp_carrier_destroy(accepted_carrier);
		stcp_rust_release(accepted_ctx);
		return PTR_ERR(newsk);
	}

	child = stcp_sk(newsk);
	child->rust_ctx = accepted_ctx;

	if (accepted_carrier) {
		child->carrier = accepted_carrier;
		/* TCP accept creates the carrier before the Rust child exists.
		 * Wire both pointers before starting the RX thread, otherwise
		 * incoming handshake frames are dispatched with a NULL context. */
		stcp_carrier_set_owner(child->carrier, child);
		stcp_carrier_set_rust_ctx(child->carrier, child->rust_ctx);
	} else {
		ret = stcp_carrier_accept(listener->carrier, child->rust_ctx, child,
			&child->carrier, flags & O_NONBLOCK ? O_NONBLOCK : 0);
		pr_info("stcp-debug-v7: accept UDP carrier attach child=%px carrier=%px ret=%d\n",
			child, child->carrier, ret);
		if (ret)
			goto fail_child;
	}

	pr_info("stcp-debug-v7: accept wire child=%px ctx=%px carrier=%px\n",
		child, child->rust_ctx, child->carrier);
	stcp_rust_set_carrier(child->rust_ctx, child->carrier);
	stcp_rust_set_owner(child->rust_ctx, child);

	ret = stcp_carrier_start_receiver_thread(child->carrier);
	pr_info("stcp-debug-v7: accept receiver start child=%px carrier=%px ret=%d\n",
		child, child->carrier, ret);
	if (ret)
		goto fail_carrier;

	pr_info("stcp-debug-v7: accept handshake start begin child=%px ctx=%px\n",
		child, child->rust_ctx);
	ret = stcp_rust_start_handshake(child->rust_ctx);
	pr_info("stcp-debug-v7: accept handshake start result child=%px ctx=%px ret=%d connected=%d\n",
		child, child->rust_ctx, ret, stcp_rust_is_connected(child->rust_ctx));
	if (ret)
		goto fail_carrier;

	if (stcp_rust_is_connected(child->rust_ctx) <= 0) {
		pr_info("stcp-debug-v7: accept waiting handshake child=%px ctx=%px timeout_ms=%u\n",
			child, child->rust_ctx, STCP_CONNECT_TIMEOUT_MS);
		ret = wait_event_interruptible_timeout(child->recv_wq,
			stcp_rust_is_connected(child->rust_ctx) > 0,
			msecs_to_jiffies(STCP_CONNECT_TIMEOUT_MS));
		pr_info("stcp-debug-v7: accept wait result child=%px ctx=%px wait_ret=%d connected=%d\n",
			child, child->rust_ctx, ret, stcp_rust_is_connected(child->rust_ctx));
		if (ret < 0)
			goto fail_carrier;
		if (ret == 0) {
			ret = -ETIMEDOUT;
			goto fail_carrier;
		}
	}

	stcp_start_retransmit_work(child);
	stcp_user_register(child);
	newsock->state = SS_CONNECTED;
	pr_info("stcp-debug-v7: ACCEPT COMPLETE child=%px ctx=%px carrier=%px sk=%px\n",
		child, child->rust_ctx, child->carrier, newsk);
	return 0;

fail_carrier:
	pr_err("stcp-debug-v7: accept fail_carrier child=%px ctx=%px carrier=%px ret=%d\n",
		child, child ? child->rust_ctx : NULL,
		child ? child->carrier : NULL, ret);
	if (child && child->rust_ctx) {
		stcp_rust_set_owner(child->rust_ctx, NULL);
		stcp_rust_set_carrier(child->rust_ctx, NULL);
	}
	if (child && child->carrier) {
		stcp_carrier_destroy(child->carrier);
		child->carrier = NULL;
	}
fail_child:
	if (child && child->rust_ctx) {
		stcp_rust_release(child->rust_ctx);
		child->rust_ctx = NULL;
	}
	if (newsk) {
		newsock->sk = NULL;
		sk_free(newsk);
	}
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
