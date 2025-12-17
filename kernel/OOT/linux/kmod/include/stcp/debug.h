#pragma once
#include <linux/printk.h>
#include <linux/ratelimit.h>

extern int stcp_debug_mode; /* define in some .c and module_param it */
#ifndef DISABLE_DEBUG
#  define SDBG(fmt, ...) \
	  do { pr_info("stcp %s:%d [%s]: " fmt "\n",__FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (0)
#else
#  define SDBG(fmt, ...) /* NOP */
#endif
