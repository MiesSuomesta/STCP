#include "stcp_socket.h"
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/types.h>

void *stcp_rust_kernel_alloc(size_t size)
{
	if (!size)
		size = 1;

	return kmalloc(size, GFP_KERNEL);
}

void stcp_rust_kernel_free(void *ptr)
{
	kfree(ptr);
}

void stcp_kernel_wake_accept(void *owner)
{
	struct stcp_sock *ssk = owner;

	if (ssk)
		wake_up_interruptible_all(&ssk->accept_wq);
}

void stcp_kernel_wake_recv(void *owner)
{
	struct stcp_sock *ssk = owner;

	if (ssk)
		wake_up_interruptible_all(&ssk->recv_wq);
}
