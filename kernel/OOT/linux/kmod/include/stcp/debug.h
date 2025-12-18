#pragma once
#include <linux/printk.h>
#include <linux/ratelimit.h>

extern int stcp_debug_mode; /* define in some .c and module_param it */
#ifdef STCP_RELEASE
#  define SDBG(fmt, ...) /* NOP */
#else
#  define SDBG(fmt, ...) \
	  do { pr_info("stcp %s:%d [%s]: " fmt "\n",__FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (0)
#endif
