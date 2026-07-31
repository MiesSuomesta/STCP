/* SPDX-License-Identifier: GPL-2.0 */
#ifndef STCP_LOG_H
#define STCP_LOG_H

#include <linux/printk.h>

/*
 * Release builds compile verbose tracing out completely.  This is important
 * for small-payload/high-IOPS benchmarks: even ratelimited printk call sites
 * add hot-path overhead and can perturb wakeup timing.
 *
 * Build debug tracing explicitly with:
 *   make STCP_RELEASE=0 ...
 */
#ifdef STCP_RELEASE
#define STCP_TRACE(fmt, ...) do { } while (0)
#define STCP_TRACE_RATELIMITED(fmt, ...) do { } while (0)
#else
#define STCP_TRACE(fmt, ...) \
	pr_info("stcp-debug: " fmt, ##__VA_ARGS__)
#define STCP_TRACE_RATELIMITED(fmt, ...) \
	pr_info_ratelimited("stcp-debug: " fmt, ##__VA_ARGS__)
#endif

#endif /* STCP_LOG_H */
