#include <linux/init.h>
#include <linux/module.h>
#include <linux/atomic.h>

#include "stcp_proto.h"
#include "stcp_rust_ffi.h"

static bool drop_first_data;
static atomic_t drop_budget = ATOMIC_INIT(0);

module_param(drop_first_data, bool, 0644);
MODULE_PARM_DESC(drop_first_data, "Drop the first DATA frame to test retransmission");

bool stcp_kernel_should_drop_data(void)
{
	return atomic_cmpxchg(&drop_budget, 1, 0) == 1;
}

static int __init stcp_module_init(void)
{
	int ret;

	atomic_set(&drop_budget, drop_first_data ? 1 : 0);

	ret = stcp_rust_init();
	if (ret)
		return ret;

	ret = stcp_rust_crypto_selftest();
	if (ret) {
		pr_err("stcp: crypto selftest failed: %d\n", ret);
		stcp_rust_exit();
		return ret;
	}

	pr_info("stcp: directional crypto selftest passed\n");

	ret = stcp_proto_register();
	if (ret) {
		stcp_rust_exit();
		return ret;
	}

	pr_info("stcp: loopback BSD transport loaded\n");
	return 0;
}

static void __exit stcp_module_exit(void)
{
	stcp_proto_unregister();
	stcp_rust_exit();
	pr_info("stcp: loopback BSD transport unloaded\n");
}

module_init(stcp_module_init);
module_exit(stcp_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("STCPv2");
MODULE_DESCRIPTION("Rust-backed STCP BSD socket loopback implementation");
