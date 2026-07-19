#include "stcp_socket.h"

#include <linux/gfp.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/types.h>

void stcp_kernel_random_bytes(void *buffer, size_t len)
{
	if (buffer && len)
		get_random_bytes(buffer, len);
}

void stcp_kernel_wake_accept(void *owner)
{
	struct stcp_sock *ssk = owner;

	if (ssk && waitqueue_active(&ssk->accept_wq))
		wake_up_interruptible(&ssk->accept_wq);
}

void stcp_kernel_wake_recv(void *owner)
{
	struct stcp_sock *ssk = owner;

	if (ssk && waitqueue_active(&ssk->recv_wq))
		wake_up_interruptible(&ssk->recv_wq);
}
