#pragma once
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/preempt.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>

#include <linux/printk.h>
#include <linux/ratelimit.h>
#include <stcp/proto_layer.h>   // Rust proto_ops API
#include <stcp/stcp_socket_struct.h>


extern int stcp_debug_mode; /* define in some .c and module_param it */
#ifdef STCP_RELEASE
#  define SDBG(fmt, ...) /* NOP */
#else
#  define SDBG(fmt, ...) \
	  do { pr_err("stcp %s:%d [%s]: " fmt "\n",__FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (0)
#endif

#define DEBUG_INCOMING_STCP_STATUS(st)                          \
    stcp_log_ctx(__FILE__, (st), (st) ? (st)->sk : NULL )

unsigned long stcp_read_ul(const unsigned long *p);

 void stcp_log_ctx(const char *tag, const void *st_ptr, const void *sk_ptr);

/* Ratelimit-versio (hyvä steady/churniin) */
void stcp_log_ctx_rl(const char *tag, const void *st_ptr, const void *sk_ptr);

/* Kun sulla on struct stcp_sock *st ja haluat tulostaa kenttiä */
void stcp_log_st_fields(const char *tag,
                                        const struct stcp_sock *st,
                                        const struct sock *sk);

void stcp_log_st_fields_rl(const char *tag,
                                           const struct stcp_sock *st,
                                           const struct sock *sk);
