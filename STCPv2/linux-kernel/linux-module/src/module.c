#include <linux/init.h>
#include <linux/module.h>
#include "stcp.h"
#include "stcp_rust.h"

static int __init stcp_init(void)
{
    int ret = stcp_rust_init();
    if (ret)
        return ret;
    ret = stcp_proto_register();
    if (ret)
        stcp_rust_exit();
    return ret;
}

static void __exit stcp_exit(void)
{
    stcp_proto_unregister();
    stcp_rust_exit();
}

module_init(stcp_init);
module_exit(stcp_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("STCPv2");
MODULE_DESCRIPTION("STCP BSD socket transport skeleton");
