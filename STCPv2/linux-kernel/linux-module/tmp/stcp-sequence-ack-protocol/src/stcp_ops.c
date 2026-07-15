#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/in.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/socket.h>
#include <linux/sockptr.h>
#include <linux/uio.h>

#include <net/sock.h>

#include "stcp.h"
#include "stcp_proto.h"
#include "stcp_rust_ffi.h"
#include "stcp_socket.h"

#define STCP_IO_BUFFER_MAX (128 * 1024)

static int stcp_release(struct socket *sock)
{
	struct sock *sk;
	struct stcp_sock *ssk;

	if (!sock)
		return -EINVAL;

	sk = sock->sk;
	if (!sk)
		return 0;

	ssk = stcp_sk(sk);

	if (ssk->rust_ctx) {
		stcp_rust_set_owner(ssk->rust_ctx, NULL);
		stcp_rust_release(ssk->rust_ctx);
		ssk->rust_ctx = NULL;
	}

	wake_up_interruptible_all(&ssk->accept_wq);
	wake_up_interruptible_all(&ssk->recv_wq);

	sock_orphan(sk);
	sock->sk = NULL;
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

	if (!sock || !sock->sk || !addr)
		return -EINVAL;

	if (addr_len < sizeof(*sin))
		return -EINVAL;

	sin = (struct sockaddr_in *)addr;
	if (sin->sin_family != AF_INET)
		return -EAFNOSUPPORT;

	ssk = stcp_sk(sock->sk);
	if (!ssk->rust_ctx)
		return -EINVAL;

	return stcp_rust_bind(
		ssk->rust_ctx,
		(__force u32)sin->sin_addr.s_addr,
		(__force u16)sin->sin_port
	);
}

static int stcp_listen(struct socket *sock, int backlog)
{
	struct stcp_sock *ssk;

	if (!sock || !sock->sk)
		return -EINVAL;

	ssk = stcp_sk(sock->sk);
	if (!ssk->rust_ctx)
		return -EINVAL;

	return stcp_rust_listen(ssk->rust_ctx, backlog);
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
	if (!ssk->rust_ctx)
		return -EINVAL;

	ret = stcp_rust_connect(
		ssk->rust_ctx,
		(__force u32)sin->sin_addr.s_addr,
		(__force u16)sin->sin_port,
		flags
	);

	if (!ret)
		sock->state = SS_CONNECTED;

	return ret;
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
	stcp_rust_set_owner(child->rust_ctx, child);

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
	ssize_t ret;

	if (!sock || !sock->sk || !msg)
		return -EINVAL;

	if (!len)
		return 0;

	if (len > STCP_IO_BUFFER_MAX)
		return -EMSGSIZE;

	ssk = stcp_sk(sock->sk);
	if (!ssk->rust_ctx)
		return -EINVAL;

	buffer = kmalloc(len, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	if (!copy_from_iter_full(buffer, len, &msg->msg_iter)) {
		kfree(buffer);
		return -EFAULT;
	}

	ret = stcp_rust_send(
		ssk->rust_ctx,
		buffer,
		len,
		msg->msg_flags
	);

	kfree(buffer);
	return ret;
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

	buffer = kmalloc(len, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

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

	kfree(buffer);
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

	stcp_rust_shutdown(ssk->rust_ctx, how);
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
