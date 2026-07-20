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

	/* See stcp_kernel_wake_accept(): wake_up itself is safe and cheap. */
	if (ssk) {
		pr_info("stcp: wake_recv owner=%px ctx=%px waiters=%d\n",
			ssk, ssk->rust_ctx, waitqueue_active(&ssk->recv_wq));
		wake_up_interruptible_all(&ssk->recv_wq);
	} else {
		pr_info("stcp: wake_recv owner=NULL\n");
	}
}

/* Numeric Rust datapath tracing is disabled in performance builds. */
void stcp_kernel_debug_event(u32 event, unsigned long ctx,
                             unsigned long arg0, unsigned long arg1)
{
	(void)event;
	(void)ctx;
	(void)arg0;
	(void)arg1;
}
