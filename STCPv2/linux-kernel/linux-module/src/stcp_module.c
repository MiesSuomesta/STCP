#include <linux/init.h>
#include <linux/module.h>

#include "stcp_proto.h"
#include "stcp_rust_ffi.h"

static int __init stcp_module_init(void)
{
	int ret;

	ret = stcp_rust_init();
	if (ret)
		return ret;

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
