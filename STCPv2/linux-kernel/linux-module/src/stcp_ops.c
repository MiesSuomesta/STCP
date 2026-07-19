#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/in.h>
#include <linux/jiffies.h>
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

#define STCP_IO_BUFFER_MAX (1024 * 1024)

static int stcp_release(struct socket *sock)
{
	struct stcp_carrier *carrier;
	struct stcp_sock *ssk;
	struct sock *sk;
	void *rust_ctx;
	struct stcp_reliability_stats stats;
	u8 *tx_buffer;
	u8 *rx_buffer;

	if (!sock)
		return -EINVAL;

	sk = sock->sk;
	if (!sk)
		return 0;

	ssk = stcp_sk(sk);

	/* Prevent new operations from finding this socket during teardown. */
	sock->sk = NULL;

	stcp_stop_retransmit_work(ssk);

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
	rx_buffer = xchg(&ssk->rx_buffer, NULL);
	kfree_sensitive(tx_buffer);
	kfree_sensitive(rx_buffer);

	wake_up_interruptible_all(&ssk->accept_wq);
	wake_up_interruptible_all(&ssk->recv_wq);

	sock_orphan(sk);
	sk_common_release(sk);

	return 0;
}

static int stcp_bind(
	struct socket *sock,
	struct sockaddr_unsized *addr,
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

	ret = stcp_carrier_bind(
		ssk->carrier,
		(__force u32)sin->sin_addr.s_addr,
		(__force u16)sin->sin_port
	);

	if (ret)
		return ret;

	return stcp_rust_bind(
		ssk->rust_ctx,
		(__force u32)sin->sin_addr.s_addr,
		(__force u16)sin->sin_port
	);
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

	ret = stcp_carrier_listen(
		ssk->carrier,
		backlog
	);

	if (ret)
		return ret;

	return stcp_rust_listen(
		ssk->rust_ctx,
		backlog
	);
}

static int stcp_connect(
	struct socket *sock,
	struct sockaddr_unsized *addr,
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

	ret = stcp_carrier_connect(
		ssk->carrier,
		(__force u32)sin->sin_addr.s_addr,
		(__force u16)sin->sin_port,
		flags
	);

	if (ret)
		return ret;

	ret = stcp_rust_connect(
		ssk->rust_ctx,
		(__force u32)sin->sin_addr.s_addr,
		(__force u16)sin->sin_port,
		flags
	);		

	if (ret)
		return ret;

	ret = stcp_rust_start_handshake(ssk->rust_ctx);
	if (ret)
		return ret;

	/*
	 * Start RX only after Rust has a complete connection, carrier and owner.
	 * Otherwise a queued PublicKey can be consumed against a half-initialized
	 * context and the handshake never reaches Ready.
	 */
	ret = stcp_carrier_start_receiver_thread(ssk->carrier);
	if (ret)
		return ret;

	/*
	 * Do not wait for the crypto handshake inside connect().  The listener
	 * application must first run accept() so the accepted carrier and Rust
	 * owner can be attached; waiting here caused the stress clients to stall
	 * at connect() with zero completed connections.
	 *
	 * The first send() remains correctly gated by stcp_rust_send(): it returns
	 * -EAGAIN until the state reaches Ready.  Nonblocking userspace then waits
	 * for EPOLLOUT on recv_wq, which the carrier wakes as handshake bytes arrive.
	 */
	stcp_start_retransmit_work(ssk);
	sock->state = SS_CONNECTED;
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
	if (!listener->rust_ctx)
		return -EINVAL;

	for (;;) {
		ret = stcp_rust_accept(
			listener->rust_ctx,
			&accepted_ctx,
			flags
		);

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

	if (!accepted_ctx)
		return -EIO;

	newsk = stcp_alloc_child_sock(
		sock_net(sock->sk),
		newsock
	);

	if (IS_ERR(newsk)) {
		stcp_rust_release(accepted_ctx);
		return PTR_ERR(newsk);
	}

	child = stcp_sk(newsk);
	child->rust_ctx = accepted_ctx;

	/*
	 * accepted_ctx has already been removed from the Rust listener queue.
	 * Therefore the matching TCP child must be consumed atomically here.
	 * Passing the outer listener's O_NONBLOCK flag can return -EAGAIN after
	 * the Rust child was popped, permanently desynchronising the two queues
	 * and leaving all clients stuck before handshake completion.
	 *
	 * kernel_connect() succeeded before the Rust child was queued, so the
	 * corresponding TCP child is guaranteed to arrive. Use a blocking inner
	 * accept even when the public STCP listener itself is nonblocking.
	 */
	ret = stcp_carrier_accept(
		listener->carrier,
		child->rust_ctx,
		child,
		&child->carrier,
		0
	);

	if (ret) {
		stcp_rust_set_owner(child->rust_ctx, NULL);
		stcp_rust_set_carrier(child->rust_ctx, NULL);
		stcp_rust_release(child->rust_ctx);
		child->rust_ctx = NULL;
		newsock->sk = NULL;
		sk_free(newsk);
		return ret;
	}

	stcp_rust_set_carrier(
		child->rust_ctx,
		child->carrier
	);

	stcp_rust_set_owner(child->rust_ctx, child);

	ret = stcp_rust_start_handshake(child->rust_ctx);
	if (ret) {
		stcp_rust_set_owner(child->rust_ctx, NULL);
		stcp_rust_set_carrier(child->rust_ctx, NULL);
		stcp_carrier_destroy(child->carrier);
		child->carrier = NULL;
		stcp_rust_release(child->rust_ctx);
		child->rust_ctx = NULL;
		newsock->sk = NULL;
		sk_free(newsk);
		return ret;
	}

	ret = stcp_carrier_start_receiver_thread(child->carrier);
	if (ret) {
		stcp_rust_set_owner(child->rust_ctx, NULL);
		stcp_rust_set_carrier(child->rust_ctx, NULL);
		stcp_carrier_destroy(child->carrier);
		child->carrier = NULL;
		stcp_rust_release(child->rust_ctx);
		child->rust_ctx = NULL;
		newsock->sk = NULL;
		sk_free(newsk);
		return ret;
	}

	stcp_start_retransmit_work(child);

	newsock->state = SS_CONNECTED;
	return 0;
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
	if (!ssk->tx_buffer)
		ssk->tx_buffer = kmalloc(STCP_IO_BUFFER_MAX, GFP_KERNEL);
	buffer = ssk->tx_buffer;
	if (!buffer) {
		mutex_unlock(&ssk->tx_lock);
		return -ENOMEM;
	}

	while (total < len) {
		size_t chunk = min_t(size_t, len - total, STCP_IO_BUFFER_MAX);
		int wait_ret;

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
			wait_ret = wait_event_interruptible(
				ssk->recv_wq,
				stcp_rust_can_send(ssk->rust_ctx, chunk) > 0
			);
			if (wait_ret < 0) {
				ret = wait_ret;
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
	if (!ssk->rx_buffer)
		ssk->rx_buffer = kmalloc(STCP_IO_BUFFER_MAX, GFP_KERNEL);
	buffer = ssk->rx_buffer;
	if (!buffer) {
		mutex_unlock(&ssk->rx_lock);
		return -ENOMEM;
	}

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
