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
#include <linux/uaccess.h>

#include <net/sock.h>

#include "stcp.h"
#include "stcp_kernel_compat.h"
#include "stcp_carrier.h"
#include "stcp_proto.h"
#include "stcp_rust_ffi.h"
#include "stcp_socket.h"
#include "stcp_users.h"

#define STCP_IO_BUFFER_MAX (2 * 1024 * 1024)
#define STCP_CONNECT_TIMEOUT_MS 5000
#define STCP_SEND_READY_TIMEOUT_MS 5000
#define STCP_CLOSE_DRAIN_TIMEOUT_MS 750
#define STCP_CLOSE_FIN_TIMEOUT_MS 1250

/*
 * Crash-debug instrumentation for the socket lifetime / LSM recvmsg race.
 *
 * security_socket_recvmsg() is called before ->recvmsg(), so a crash inside
 * AppArmor can happen before stcp_recvmsg() gets control.  The release trace
 * is therefore intentionally detailed: it records exactly when sock->sk is
 * detached and what STCP state still exists at that moment.
 */
static void stcp_debug_socket_state(const char *where, struct socket *sock)
{
    struct sock *sk;
    struct stcp_sock *ssk;
    void *security = NULL;

    if (!sock) {
        pr_err("stcp-debug: %s sock=NULL pid=%d comm=%s\n",
               where, current->pid, current->comm);
        return;
    }

    sk = READ_ONCE(sock->sk);
    if (!sk) {
        pr_err("stcp-debug: %s sock=%px sk=NULL sock_state=%d pid=%d comm=%s\n",
               where, sock, READ_ONCE(sock->state), current->pid, current->comm);
        return;
    }

    ssk = stcp_sk(sk);
#ifdef CONFIG_SECURITY
    security = READ_ONCE(sk->sk_security);
#endif

    pr_err("stcp-debug: %s sock=%px sk=%px ssk=%px sk_security=%px "
           "sock_state=%d sk_state=%u ctx=%px carrier=%px "
           "txbuf=%px rxbuf=%px pid=%d comm=%s\n",
           where,
           sock,
           sk,
           ssk,
           security,
           READ_ONCE(sock->state),
           READ_ONCE(sk->sk_state),
           READ_ONCE(ssk->rust_ctx),
           READ_ONCE(ssk->carrier),
           READ_ONCE(ssk->tx_buffer),
           READ_ONCE(ssk->rx_buffer),
           current->pid,
           current->comm);
}


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
	u64 lifetime_id;
	int retransmit_callbacks;
	u8 *tx_buffer;
	u8 *rx_buffer;
	size_t tx_buffer_size;
	size_t rx_buffer_size;

	stcp_debug_socket_state("release-enter", sock);

	if (!sock)
		return -EINVAL;

	sk = READ_ONCE(sock->sk);
	if (!sk) {
		pr_err("stcp-debug: release-no-sk sock=%px pid=%d comm=%s\n",
		       sock, current->pid, current->comm);
		return 0;
	}

	ssk = stcp_sk(sk);
	if (xchg(&ssk->teardown_started, true))
		pr_err("STCP-LIFETIME-BUG: duplicate/concurrent release id=%llu sock=%px sk=%px ssk=%px pid=%d comm=%s\n",
		       READ_ONCE(ssk->lifetime_id), sock, sk, ssk, current->pid, current->comm);
	pr_err("stcp-lifetime: RELEASE-MARK id=%llu sock=%px sk=%px ssk=%px ctx=%px carrier=%px retx_active=%d retx_pending=%d pid=%d comm=%s\n",
	       READ_ONCE(ssk->lifetime_id), sock, sk, ssk,
	       READ_ONCE(ssk->rust_ctx), READ_ONCE(ssk->carrier),
	       atomic_read(&ssk->retransmit_callbacks),
	       delayed_work_pending(&ssk->retransmit_work),
	       current->pid, current->comm);
	stcp_debug_socket_state("release-before-unregister", sock);

	/* Remove it from /proc/stcp/users before freeing the socket. */
	stcp_user_unregister(ssk);

	/*
	 * Keep sock->sk attached while STCP teardown is still in progress.
	 * Generic socket/LSM hooks may still observe this socket concurrently, so
	 * detaching it here creates a long NULL window during graceful close.
	 * The final detach is done below, after sk_common_release(), matching the
	 * ordering used by the normal INET release path.
	 */
	stcp_debug_socket_state("release-sk-kept-attached", sock);

	pr_err("stcp-debug: release-stop-retransmit sk=%px ssk=%px ctx=%px carrier=%px\n",
	       sk, ssk, READ_ONCE(ssk->rust_ctx), READ_ONCE(ssk->carrier));
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
	if (READ_ONCE(ssk->rust_ctx)) {
		int carrier_error = READ_ONCE(ssk->carrier)
			? stcp_carrier_last_error(READ_ONCE(ssk->carrier)) : 0;

		if (!carrier_error) {
			pr_emerg("stcp-xconnect: REL01 protocol-close-send id=%llu ctx=%px carrier=%px\n",
				 READ_ONCE(ssk->lifetime_id), READ_ONCE(ssk->rust_ctx),
				 READ_ONCE(ssk->carrier));
			stcp_rust_shutdown(READ_ONCE(ssk->rust_ctx), SHUT_RDWR);
		} else {
			/* The peer has already reset/closed the TCP carrier.  Sending the
			 * STCP CLOSE frame only re-enters a dead transport during release.
			 * Rust final release below still performs local state teardown. */
			pr_emerg("stcp-xconnect: REL01 protocol-close-skip id=%llu ctx=%px carrier=%px terminal=%d\n",
				 READ_ONCE(ssk->lifetime_id), READ_ONCE(ssk->rust_ctx),
				 READ_ONCE(ssk->carrier), carrier_error);
		}
	}

	/*
	 * Keep the TCP carrier alive long enough for queued STCP data and the
	 * protocol CLOSE frame to leave the write queue.  Then perform SHUT_WR and
	 * wait a bounded time for the peer to acknowledge FIN.  This prevents
	 * churn runs from accumulating sockets in FIN-WAIT-1 while still keeping
	 * close latency bounded when a peer disappears.
	 *
	 * UDP has its own reliability/teardown path and is intentionally skipped.
	 */
	if (READ_ONCE(ssk->carrier))
		stcp_carrier_graceful_close(
			READ_ONCE(ssk->carrier),
			STCP_CLOSE_DRAIN_TIMEOUT_MS,
			STCP_CLOSE_FIN_TIMEOUT_MS
		);

	/* Detach pointers exactly once, including concurrent error teardown. */
	pr_err("stcp-debug: release-before-xchg sk=%px ssk=%px ctx=%px carrier=%px\n",
	       sk, ssk, READ_ONCE(ssk->rust_ctx), READ_ONCE(ssk->carrier));
	rust_ctx = xchg(&ssk->rust_ctx, NULL);
	carrier = xchg(&ssk->carrier, NULL);
	pr_err("stcp-debug: release-after-xchg sk=%px ssk=%px old_ctx=%px old_carrier=%px "
	       "ctx_now=%px carrier_now=%px\n",
	       sk, ssk, rust_ctx, carrier,
	       READ_ONCE(ssk->rust_ctx), READ_ONCE(ssk->carrier));

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

	/*
	 * sk_common_release() may drop the final reference to @sk and free the
	 * containing struct stcp_sock.  Snapshot diagnostic fields before that
	 * point; nothing after sk_common_release() may dereference sk/ssk.
	 */
	lifetime_id = READ_ONCE(ssk->lifetime_id);
	retransmit_callbacks = atomic_read(&ssk->retransmit_callbacks);

	pr_err("stcp-debug: release-before-sock-orphan sock=%px sk=%px ssk=%px pid=%d comm=%s\n",
	       sock, sk, ssk, current->pid, current->comm);
	sock_orphan(sk);
	pr_err("stcp-debug: release-before-sk-common-release sock=%px sk=%px ssk=%px\n",
	       sock, sk, ssk);
	sk_common_release(sk);

	/*
	 * STCP teardown is now complete.  Only at this point detach the VFS socket
	 * from the protocol sock.  __sock_release() also clears this field after
	 * ->release() returns, so this assignment is intentionally idempotent.
	 */
	pr_err("stcp-debug: release-before-final-sk-null sock=%px old_sk=%px pid=%d comm=%s\n",
	       sock, sk, current->pid, current->comm);
	WRITE_ONCE(sock->sk, NULL);
	pr_err("stcp-debug: release-after-final-sk-null sock=%px old_sk=%px pid=%d comm=%s\n",
	       sock, sk, current->pid, current->comm);

	pr_err("stcp-lifetime: RELEASE-EXIT id=%llu sock=%px old_sk=%px retx_active=%d pid=%d comm=%s\n",
	       lifetime_id, sock, sk, retransmit_callbacks,
	       current->pid, current->comm);
	pr_err("stcp-debug: release-exit sock=%px old_sk=%px pid=%d comm=%s\n",
	       sock, sk, current->pid, current->comm);

	return 0;
}

static int stcp_bind(
	struct socket *sock,
	stcp_sockaddr_t *addr,
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
	stcp_sockaddr_t *addr,
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

	pr_emerg("stcp-xconnect: C01 carrier-connect-enter sk=%px ctx=%px carrier=%px addr=%pI4 port=%u flags=0x%x pid=%d comm=%s\n",
		sock->sk, ssk->rust_ctx, ssk->carrier, &sin->sin_addr.s_addr,
		ntohs(sin->sin_port), flags, current->pid, current->comm);

	ret = stcp_carrier_connect(
		ssk->carrier,
		(__force u32)sin->sin_addr.s_addr,
		(__force u16)sin->sin_port,
		flags
	);

	pr_emerg("stcp-xconnect: C02 carrier-connect-exit sk=%px ctx=%px carrier=%px ret=%d\n",
		sock->sk, ssk->rust_ctx, ssk->carrier, ret);
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
	pr_emerg("stcp-xconnect: C03 rust-connect-exit sk=%px ctx=%px carrier=%px ret=%d\n",
		sock->sk, ssk->rust_ctx, ssk->carrier, ret);
	if (ret)
		return ret;

	/*
	 * The receiver must be running before the handshake can emit frames.
	 * Otherwise the peer may send PublicKey/HandshakeDone and application data
	 * while no carrier RX worker exists, leaving recv() asleep forever.
	 * At this point connect(), carrier attachment and owner setup are complete.
	 */
	pr_emerg("stcp-xconnect: C04 rx-start-enter ctx=%px carrier=%px\n",
		ssk->rust_ctx, ssk->carrier);
	ret = stcp_carrier_start_receiver_thread(ssk->carrier);
	pr_emerg("stcp-xconnect: C05 rx-start-exit ctx=%px carrier=%px ret=%d\n",
		ssk->rust_ctx, ssk->carrier, ret);
	if (ret)
		return ret;

	pr_emerg("stcp-xconnect: C06 handshake-start-enter ctx=%px carrier=%px\n",
		ssk->rust_ctx, ssk->carrier);
	ret = stcp_rust_start_handshake(ssk->rust_ctx);
	pr_emerg("stcp-xconnect: C07 handshake-start-exit ctx=%px carrier=%px ret=%d\n",
		ssk->rust_ctx, ssk->carrier, ret);
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
	pr_emerg("stcp-xconnect: C08 retx-start-enter ctx=%px carrier=%px\n",
		ssk->rust_ctx, ssk->carrier);
	stcp_start_retransmit_work(ssk);
	pr_emerg("stcp-xconnect: C09 wait-ready-enter ctx=%px carrier=%px\n",
		ssk->rust_ctx, ssk->carrier);

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

	pr_emerg("stcp-xconnect: C10 wait-event-enter ctx=%px carrier=%px timeout_ms=%u\n",
		ssk->rust_ctx, ssk->carrier, STCP_CONNECT_TIMEOUT_MS);
	ret = wait_event_interruptible_timeout(
		ssk->recv_wq,
		stcp_rust_is_connected(ssk->rust_ctx) > 0 ||
		stcp_carrier_last_error(ssk->carrier) != 0,
		msecs_to_jiffies(STCP_CONNECT_TIMEOUT_MS)
	);
	pr_emerg("stcp-xconnect: C11 wait-event-exit ctx=%px carrier=%px ret=%d terminal=%d\n",
		ssk->rust_ctx, ssk->carrier, ret,
		stcp_carrier_last_error(ssk->carrier));
	if (ret < 0) {
		pr_info("stcp: connect interrupted ctx=%px ret=%d\n",
			ssk->rust_ctx, ret);
		return ret;
	}
	if (stcp_carrier_last_error(ssk->carrier) != 0) {
		int carrier_error = stcp_carrier_last_error(ssk->carrier);

		pr_info("stcp: connect carrier failed ctx=%px error=%d\n",
			ssk->rust_ctx, carrier_error);
		return carrier_error;
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

static void stcp_accept_cleanup_child(
	struct socket *newsock,
	struct sock *newsk,
	struct stcp_sock *child
)
{
	if (child) {
		if (child->rust_ctx) {
			stcp_rust_set_owner(child->rust_ctx, NULL);
			stcp_rust_set_carrier(child->rust_ctx, NULL);
		}
		if (child->carrier) {
			stcp_carrier_destroy(child->carrier);
			child->carrier = NULL;
		}
		if (child->rust_ctx) {
			stcp_rust_release(child->rust_ctx);
			child->rust_ctx = NULL;
		}
	}
	if (newsock)
		newsock->sk = NULL;
	if (newsk)
		sk_free(newsk);
}

static int stcp_accept(
	struct socket *sock,
	struct socket *newsock,
	struct proto_accept_arg *arg
)
{
	struct stcp_sock *listener;
	struct stcp_sock *child;
	struct sock *newsk = NULL;
	struct stcp_carrier *accepted_carrier = NULL;
	void *accepted_ctx = NULL;
	u32 local_addr = 0, peer_addr = 0;
	u16 local_port = 0, peer_port = 0;
	bool external_tcp = false;
	int flags = arg ? arg->flags : 0;
	int ret;

	if (!sock || !sock->sk || !newsock)
		return -EINVAL;

	listener = stcp_sk(sock->sk);
	if (!listener->rust_ctx || !listener->carrier)
		return -EINVAL;

	pr_emerg("stcp-xconnect: A01 accept-enter listener=%px ctx=%px carrier=%px flags=0x%x newsock=%px\n",
		listener, listener->rust_ctx, listener->carrier, flags, newsock);

	/* Keep the mature Raspberry Pi UDP accept path unchanged. */
	if (stcp_carrier_get_kind(listener->carrier) == STCP_CARRIER_UDP) {
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
		/*
		 * TCP must accept the real carrier first.  A local Linux/Raspberry
		 * client also enqueues a Rust child through the in-kernel listener
		 * registry, while an external Zephyr client cannot.  After accepting
		 * the carrier, briefly give the local path a chance to provide its
		 * already-created child; otherwise create an external server child.
		 */
		pr_emerg("stcp-xconnect: A02 kernel-accept-enter listener_carrier=%px\n",
			listener->carrier);
		ret = stcp_carrier_accept_unattached(
			listener->carrier,
			&accepted_carrier,
			flags
		);
		pr_emerg("stcp-xconnect: A03 kernel-accept-exit ret=%d accepted_carrier=%px\n",
			ret, accepted_carrier);
		if (ret)
			return ret;

		if (stcp_rust_has_accept(listener->rust_ctx) <= 0 &&
		    !(flags & O_NONBLOCK)) {
			wait_event_timeout(
				listener->accept_wq,
				stcp_rust_has_accept(listener->rust_ctx) > 0,
				msecs_to_jiffies(100)
			);
		}

		ret = stcp_rust_accept(listener->rust_ctx, &accepted_ctx, O_NONBLOCK);
		pr_emerg("stcp-xconnect: A04 rust-accept ret=%d accepted_ctx=%px accepted_carrier=%px\n",
			ret, accepted_ctx, accepted_carrier);
		if (ret == -EAGAIN) {
			ret = stcp_carrier_get_endpoints(
				accepted_carrier,
				&local_addr, &local_port,
				&peer_addr, &peer_port
			);
			pr_emerg("stcp-xconnect: A05 endpoints ret=%d local=%pI4:%u peer=%pI4:%u carrier=%px\n",
				ret, &local_addr, ntohs(local_port), &peer_addr, ntohs(peer_port),
				accepted_carrier);
			if (ret) {
				stcp_carrier_destroy(accepted_carrier);
				return ret;
			}

			ret = stcp_rust_create_external_tcp_child(
				listener->rust_ctx,
				local_addr, local_port, peer_addr, peer_port,
				&accepted_ctx
			);
			pr_emerg("stcp-xconnect: A06 external-child-create ret=%d ctx=%px carrier=%px\n",
				ret, accepted_ctx, accepted_carrier);
			if (ret) {
				stcp_carrier_destroy(accepted_carrier);
				return ret;
			}
			external_tcp = true;
			pr_info(
				"stcp: external TCP child created listener_ctx=%px child_ctx=%px\n",
				listener->rust_ctx, accepted_ctx
			);
		} else if (ret) {
			stcp_carrier_destroy(accepted_carrier);
			return ret;
		}
	}

	if (!accepted_ctx)
		return -EIO;

	pr_emerg("stcp-xconnect: A07 child-sock-alloc-enter ctx=%px carrier=%px newsock=%px\n",
		accepted_ctx, accepted_carrier, newsock);
	newsk = stcp_alloc_child_sock(sock_net(sock->sk), newsock);
	pr_emerg("stcp-xconnect: A08 child-sock-alloc-exit newsk=%px newsock_sk=%px\n",
		newsk, READ_ONCE(newsock->sk));
	if (IS_ERR(newsk)) {
		if (accepted_carrier)
			stcp_carrier_destroy(accepted_carrier);
		stcp_rust_release(accepted_ctx);
		return PTR_ERR(newsk);
	}

	child = stcp_sk(newsk);
	child->rust_ctx = accepted_ctx;
	child->compression_enabled = listener->compression_enabled;
	child->compression_threshold = listener->compression_threshold;
	ret = stcp_rust_set_compression_threshold(
		child->rust_ctx, child->compression_threshold);
	if (!ret)
		ret = stcp_rust_set_compression(
			child->rust_ctx, child->compression_enabled ? 1 : 0);
	if (ret) {
		stcp_accept_cleanup_child(newsock, newsk, child);
		return ret;
	}

	if (stcp_carrier_get_kind(listener->carrier) == STCP_CARRIER_UDP) {
		ret = stcp_carrier_accept(
			listener->carrier,
			child->rust_ctx,
			child,
			&child->carrier,
			0
		);
		if (ret) {
			stcp_accept_cleanup_child(newsock, newsk, child);
			return ret;
		}
	} else {
		child->carrier = accepted_carrier;
		stcp_carrier_attach(child->carrier, child->rust_ctx, child);
		pr_emerg("stcp-xconnect: A09 carrier-attached child=%px ctx=%px carrier=%px external=%d\n",
			child, child->rust_ctx, child->carrier, external_tcp);
	}

	stcp_rust_set_carrier(child->rust_ctx, child->carrier);
	stcp_rust_set_owner(child->rust_ctx, child);

	pr_emerg("stcp-xconnect: A10 rx-start-enter child=%px ctx=%px carrier=%px\n",
		child, child->rust_ctx, child->carrier);
	ret = stcp_carrier_start_receiver_thread(child->carrier);
	pr_emerg("stcp-xconnect: A11 rx-start-exit ret=%d child=%px ctx=%px carrier=%px\n",
		ret, child, child->rust_ctx, child->carrier);
	if (ret) {
		stcp_accept_cleanup_child(newsock, newsk, child);
		return ret;
	}

	if (external_tcp) {
		/* The RX worker consumes the already queued Zephyr PublicKey frame and
		 * adopts its connection id before the server emits its own PublicKey. */
		pr_emerg("stcp-xconnect: A12 connection-id-wait-enter child=%px ctx=%px carrier=%px\n",
			child, child->rust_ctx, child->carrier);
		ret = wait_event_interruptible_timeout(
			child->recv_wq,
			stcp_rust_connection_id(child->rust_ctx) != 0 ||
			stcp_carrier_last_error(child->carrier) != 0,
			msecs_to_jiffies(STCP_CONNECT_TIMEOUT_MS)
		);
		pr_emerg("stcp-xconnect: A13 connection-id-wait-exit ret=%d cid=%llu terminal=%d\n",
			ret, (unsigned long long)stcp_rust_connection_id(child->rust_ctx),
			stcp_carrier_last_error(child->carrier));
		if (stcp_carrier_last_error(child->carrier) != 0) {
			ret = stcp_carrier_last_error(child->carrier);
			stcp_accept_cleanup_child(newsock, newsk, child);
			return ret;
		}
		if (ret <= 0) {
			pr_info("stcp: external TCP connection-id wait failed ctx=%px ret=%d\n",
				child->rust_ctx, ret);
			stcp_accept_cleanup_child(newsock, newsk, child);
			return ret < 0 ? ret : -ETIMEDOUT;
		}
	}

	pr_emerg("stcp-xconnect: A14 handshake-start-enter child=%px ctx=%px carrier=%px external=%d\n",
		child, child->rust_ctx, child->carrier, external_tcp);
	ret = stcp_rust_start_handshake(child->rust_ctx);
	pr_emerg("stcp-xconnect: A15 handshake-start-exit ret=%d child=%px ctx=%px carrier=%px cid=%llu\n",
		ret, child, child->rust_ctx, child->carrier,
		(unsigned long long)stcp_rust_connection_id(child->rust_ctx));

	/*
	 * The accepted carrier RX worker is started before this point.  On a fast
	 * same-host TCP connection (notably RPi -> RPi) it can consume the peer's
	 * PublicKey and advance the Rust handshake before stcp_accept() reaches
	 * this call.  start_handshake() then correctly reports -EINVAL because the
	 * state is no longer the initial handshake state.
	 *
	 * A non-zero connection id proves that RX has already accepted a valid
	 * STCP handshake frame for this child, so treat only that specific -EINVAL
	 * as the benign "already started by RX" race.  Keep every other error
	 * fatal so genuine invalid-state bugs are not hidden.
	 */
	if (ret == -EINVAL && stcp_rust_connection_id(child->rust_ctx) != 0) {
		pr_emerg("stcp-xconnect: A15B handshake already advanced by RX; continuing child=%px ctx=%px cid=%llu\n",
			child, child->rust_ctx,
			(unsigned long long)stcp_rust_connection_id(child->rust_ctx));
		ret = 0;
	}

	if (ret) {
		stcp_accept_cleanup_child(newsock, newsk, child);
		return ret;
	}

	pr_emerg("stcp-xconnect: A16 handshake-wait-enter child=%px ctx=%px carrier=%px\n",
		child, child->rust_ctx, child->carrier);
	ret = wait_event_interruptible_timeout(
		child->recv_wq,
		stcp_rust_is_connected(child->rust_ctx) > 0 ||
		stcp_carrier_last_error(child->carrier) != 0,
		msecs_to_jiffies(STCP_CONNECT_TIMEOUT_MS)
	);
	pr_emerg("stcp-xconnect: A17 handshake-wait-exit ret=%d connected=%d terminal=%d\n",
		ret, stcp_rust_is_connected(child->rust_ctx),
		stcp_carrier_last_error(child->carrier));
	if (stcp_carrier_last_error(child->carrier) != 0) {
		ret = stcp_carrier_last_error(child->carrier);
		stcp_accept_cleanup_child(newsock, newsk, child);
		return ret;
	}
	if (ret <= 0) {
		pr_info("stcp: accepted handshake wait failed ctx=%px external=%d ret=%d\n",
			child->rust_ctx, external_tcp, ret);
		stcp_accept_cleanup_child(newsock, newsk, child);
		return ret < 0 ? ret : -ETIMEDOUT;
	}

	stcp_start_retransmit_work(child);
	stcp_user_register(child);
	newsock->state = SS_CONNECTED;
	pr_info(
		"stcp: accept complete child=%px ctx=%px carrier=%px external=%d connection_id=%llu\n",
		child, child->rust_ctx, child->carrier, external_tcp,
		(unsigned long long)stcp_rust_connection_id(child->rust_ctx)
	);
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
	struct sock *sk;
	u8 *buffer;
	ssize_t ret;
	int wait_ret;

	stcp_debug_socket_state("recvmsg-enter", sock);

	if (!sock || !msg)
		return -EINVAL;

	sk = READ_ONCE(sock->sk);
	if (!sk) {
		pr_err("stcp-debug: recvmsg-no-sk sock=%px msg=%px len=%zu flags=0x%x "
		       "pid=%d comm=%s\n",
		       sock, msg, len, flags, current->pid, current->comm);
		return -EINVAL;
	}

	if (!len)
		return 0;

	/*
	 * SOCK_STREAM recv() may return fewer bytes than requested.
	 * Do not reject a large userspace buffer with -EMSGSIZE;
	 * cap only the temporary kernel allocation.
	 */
	len = min_t(size_t, len, STCP_IO_BUFFER_MAX);

	ssk = stcp_sk(sk);
	if (!READ_ONCE(ssk->rust_ctx)) {
		pr_err("stcp-debug: recvmsg-no-ctx sock=%px sk=%px ssk=%px carrier=%px\n",
		       sock, sk, ssk, READ_ONCE(ssk->carrier));
		return -EINVAL;
	}

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
	pr_err("stcp-debug: recvmsg-exit sock=%px sk=%px ssk=%px ctx=%px carrier=%px "
	       "ret=%zd len=%zu flags=0x%x pid=%d comm=%s\n",
	       sock, sk, ssk, READ_ONCE(ssk->rust_ctx), READ_ONCE(ssk->carrier),
	       ret, len, flags, current->pid, current->comm);
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
	struct stcp_sock *ssk;
	int value;
	int ret;

	if (!sock || !sock->sk)
		return -EINVAL;
	if (level != SOL_STCP)
		return -ENOPROTOOPT;
	if (optlen < sizeof(value))
		return -EINVAL;
	if (copy_from_sockptr(&value, optval, sizeof(value)))
		return -EFAULT;

	ssk = stcp_sk(sock->sk);
	if (!ssk->rust_ctx)
		return -EINVAL;

	switch (optname) {
	case STCP_SO_COMPRESSION:
		if (value != 0 && value != 1)
			return -EINVAL;
		ret = stcp_rust_set_compression(ssk->rust_ctx, value);
		if (ret)
			return ret;
		ssk->compression_enabled = value != 0;
		return 0;

	case STCP_SO_COMPRESSION_THRESHOLD:
		if (value < 0)
			return -EINVAL;
		ret = stcp_rust_set_compression_threshold(ssk->rust_ctx, (u32)value);
		if (ret)
			return ret;
		ssk->compression_threshold = (u32)value;
		return 0;

	default:
		return -ENOPROTOOPT;
	}
}

static int stcp_getsockopt(
	struct socket *sock,
	int level,
	int optname,
	char *optval,
	int *optlen
)
{
	struct stcp_sock *ssk;
	int value;
	int len;

	if (!sock || !sock->sk || !optval || !optlen)
		return -EINVAL;
	if (level != SOL_STCP)
		return -ENOPROTOOPT;
	if (get_user(len, optlen))
		return -EFAULT;
	if (len < sizeof(value))
		return -EINVAL;

	ssk = stcp_sk(sock->sk);
	switch (optname) {
	case STCP_SO_COMPRESSION:
		value = ssk->compression_enabled ? 1 : 0;
		break;
	case STCP_SO_COMPRESSION_THRESHOLD:
		value = (int)ssk->compression_threshold;
		break;
	default:
		return -ENOPROTOOPT;
	}

	if (copy_to_user(optval, &value, sizeof(value)))
		return -EFAULT;
	if (put_user(sizeof(value), optlen))
		return -EFAULT;
	return 0;
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
