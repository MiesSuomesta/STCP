#ifndef STCP_DEBUG_H
#define STCP_DEBUG_H

#include <linux/printk.h>

extern int stcp_debug_ref; /* define in some .c and module_param it */

#define STCP_DBG(fmt, ...) \
	do { if (READ_ONCE(stcp_debug_ref)) \
		pr_debug("stcp: " fmt "\n", ##__VA_ARGS__); } while (0)

#define STCP_INFO(fmt, ...) pr_info("stcp: " fmt "\n", ##__VA_ARGS__)
#define STCP_ERR(fmt, ...)  pr_err ("stcp: " fmt "\n", ##__VA_ARGS__)

#endif
