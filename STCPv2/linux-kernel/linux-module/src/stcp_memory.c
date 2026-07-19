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

	/*
	 * Do not guard wake_up() with waitqueue_active().  That helper is a
	 * lockless hint and using it as a correctness condition creates a lost
	 * wakeup race with wait_event_interruptible(): the producer may observe
	 * no waiter immediately before the consumer links itself to the queue.
	 */
	if (ssk)
		wake_up_interruptible_all(&ssk->accept_wq);
}

void stcp_kernel_wake_recv(void *owner)
{
	struct stcp_sock *ssk = owner;

	/* See stcp_kernel_wake_accept(): wake_up itself is safe and cheap. */
	if (ssk)
		wake_up_interruptible_all(&ssk->recv_wq);
}
