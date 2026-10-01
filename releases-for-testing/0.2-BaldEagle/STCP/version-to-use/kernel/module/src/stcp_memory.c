#include "stcp_socket.h"
#include "stcp_carrier.h"

#include <linux/gfp.h>
#include <linux/printk.h>
#include <linux/random.h>
#include <linux/sched.h>
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

/* Low-volume Rust demux/lifetime trace bridge. */
void stcp_kernel_debug_event(u32 event, unsigned long ctx,
                             unsigned long arg0, unsigned long arg1)
{
	const char *name;
	switch (event) {
	case 210: name = "UDP-UNREG-ENTER"; break;
	case 211: name = "UDP-UNREG-EXIT"; break;
	case 220: name = "UDP-DEMUX-LOOKUP"; break;
	case 221: name = "UDP-DEMUX-HIT"; break;
	case 222: name = "UDP-DEMUX-MISS"; break;
	case 223: name = "UDP-DEMUX-BEFORE-CHILD-DEREF"; break;
	case 230: name = "LISTENER-REGISTER"; break;
	case 231: name = "LISTENER-LOOKUP"; break;
	case 232: name = "LISTENER-UNREG-ENTER"; break;
	case 233: name = "LISTENER-UNREG-EXIT"; break;
	case 253: name = "HS-DONE-BOUNDARY"; break;
	case 300: name = "HS-START-ENTER"; break;
	case 301: name = "HS-START-LOCKED"; break;
	case 302: name = "HS-PUBKEY-ENTER"; break;
	case 303: name = "HS-PUBKEY-ENCODED"; break;
	case 304: name = "FRAME-SEND-ENTER"; break;
	case 305: name = "FRAME-SEND-SNAPSHOT"; break;
	case 306: name = "CARRIER-TX-ENTER"; break;
	case 307: name = "CARRIER-TX-EXIT"; break;
	case 308: name = "HS-PUBKEY-SENT"; break;
	case 309: name = "HS-START-EXIT"; break;
	case 310: name = "RX-QUEUE-ENTER"; break;
	case 311: name = "RX-QUEUE-PUSHED"; break;
	case 312: name = "RX-PROGRESS-ENTER"; break;
	case 313: name = "RX-PROGRESS-EXIT"; break;
	default: return;
	}
	if (READ_ONCE(stcp_verbose_debug))
	pr_err("stcp-demux: %s event=%u ctx=%px arg0=%#lx arg1=%#lx pid=%d comm=%s\n",
	       name, event, (void *)ctx, arg0, arg1, current->pid, current->comm);
	if (READ_ONCE(stcp_verbose_debug) && event == 223)
		dump_stack();
}
