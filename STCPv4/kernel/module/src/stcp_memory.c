#include "stcp_socket.h"

#include <linux/gfp.h>
#include <linux/printk.h>
#include <linux/random.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>

u64 stcp_kernel_benchmark_now_ns(void);

void stcp_kernel_benchmark_record(
    const u8 *file,
    size_t file_len,
    u32 line,
    u32 column,
    u64 elapsed_ns
);

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
	if (ssk) {
		u64 bench_wake_start = stcp_kernel_benchmark_now_ns();
		wake_up_interruptible(&ssk->recv_wq);
		stcp_kernel_benchmark_record((const u8 *)"C:WAKE_RECV",
		                             sizeof("C:WAKE_RECV") - 1, 0, 0,
		                             stcp_kernel_benchmark_now_ns() - bench_wake_start);
	}
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
	pr_err("stcp-demux: %s event=%u ctx=%px arg0=%#lx arg1=%#lx pid=%d comm=%s\n",
	       name, event, (void *)ctx, arg0, arg1, current->pid, current->comm);
	if (event == 223)
		dump_stack();
}

/*
 * STCP hot-path benchmark accumulator.
 *
 * start/stop must not printk: doing so would become part of enclosing timing
 * intervals and badly distort the result. Rust stop() records into this fixed
 * table; stcp_rust_exit() dumps it once when the module is unloaded.
 */
#include <linux/ktime.h>
#include <linux/math64.h>
#include <linux/spinlock.h>

#define STCP_BENCHMARK_SLOTS 64

struct stcp_benchmark_slot {
	const u8 *file;
	size_t file_len;
	u32 line;
	u32 column;
	u64 calls;
	u64 total_ns;
	u64 min_ns;
	u64 max_ns;
};

static DEFINE_SPINLOCK(stcp_benchmark_lock);
static struct stcp_benchmark_slot stcp_benchmark_slots[STCP_BENCHMARK_SLOTS];

u64 stcp_kernel_benchmark_now_ns(void)
{
	return ktime_get_mono_fast_ns();
}

void stcp_kernel_benchmark_record(const u8 *file, size_t file_len,
				  u32 line, u32 column, u64 elapsed_ns)
{
	unsigned long flags;
	struct stcp_benchmark_slot *slot = NULL;
	int i;

	spin_lock_irqsave(&stcp_benchmark_lock, flags);
	for (i = 0; i < STCP_BENCHMARK_SLOTS; i++) {
		struct stcp_benchmark_slot *s = &stcp_benchmark_slots[i];

		if (s->calls && s->file == file && s->line == line && s->column == column) {
			slot = s;
			break;
		}
		if (!s->calls && !slot)
			slot = s;
	}

	if (slot) {
		if (!slot->calls) {
			slot->file = file;
			slot->file_len = file_len;
			slot->line = line;
			slot->column = column;
			slot->min_ns = elapsed_ns;
			slot->max_ns = elapsed_ns;
		} else {
			if (elapsed_ns < slot->min_ns)
				slot->min_ns = elapsed_ns;
			if (elapsed_ns > slot->max_ns)
				slot->max_ns = elapsed_ns;
		}
		slot->calls++;
		slot->total_ns += elapsed_ns;
	}
	spin_unlock_irqrestore(&stcp_benchmark_lock, flags);
}

void stcp_kernel_benchmark_check(void)
{
	int i;

	pr_info("STCP-BENCH BEGIN\n");
	for (i = 0; i < STCP_BENCHMARK_SLOTS; i++) {
		struct stcp_benchmark_slot snapshot;
		unsigned long flags;
		u64 avg_ns;

		spin_lock_irqsave(&stcp_benchmark_lock, flags);
		snapshot = stcp_benchmark_slots[i];
		spin_unlock_irqrestore(&stcp_benchmark_lock, flags);

		if (!snapshot.calls)
			continue;
		avg_ns = div64_u64(snapshot.total_ns, snapshot.calls);
		pr_info("STCP-BENCH %.*s:%u:%u calls=%llu total_ns=%llu avg_ns=%llu min_ns=%llu max_ns=%llu\n",
			(int)snapshot.file_len, snapshot.file, snapshot.line, snapshot.column,
			(unsigned long long)snapshot.calls,
			(unsigned long long)snapshot.total_ns,
			(unsigned long long)avg_ns,
			(unsigned long long)snapshot.min_ns,
			(unsigned long long)snapshot.max_ns);
	}
	pr_info("STCP-BENCH END\n");
}
