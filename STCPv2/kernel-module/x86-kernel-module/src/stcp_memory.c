#include "stcp_socket.h"

#include <linux/gfp.h>
#include <linux/printk.h>
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

	/* Keep the hot wake path free of printk and wake only the socket queue. */
	if (ssk)
		wake_up_interruptible(&ssk->recv_wq);
}

/* Events below 300 are intentionally suppressed because they are hot-path
 * tracing. Events 300+ are low-rate pressure/failure diagnostics. */
void stcp_kernel_debug_event(u32 event, unsigned long ctx,
                             unsigned long arg0, unsigned long arg1)
{
	if (event < 300)
		return;

	pr_info_ratelimited(
		"stcp-debug: event=%u ctx=%px arg0=%lu arg1=%lu\n",
		event, (void *)ctx, arg0, arg1);
}
