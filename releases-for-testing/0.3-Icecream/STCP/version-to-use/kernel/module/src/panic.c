#include <linux/bug.h>
#include <linux/kernel.h>

__noreturn void stcp_kernel_panic(void)
{
	pr_emerg("stcp: Rust panic detected\n");
	BUG();

	unreachable();
}