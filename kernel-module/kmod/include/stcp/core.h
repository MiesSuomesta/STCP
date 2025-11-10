#ifndef STCP_CORE_H
#define STCP_CORE_H

#include <linux/module.h>
#include <linux/net.h>
#include <linux/socket.h>
#include <linux/skbuff.h>
#include <net/sock.h>

enum stcp_life_state {
	STCP_STATE_INIT = 0,
	STCP_STATE_READY,
	STCP_STATE_CLOSING,
};

struct stcp_inner {
	struct socket *sock; /* inner TCP (or other) socket; may be NULL in INIT */
};

struct stcp_state {
	struct sock *sk;            /* always valid while st exists */
	struct stcp_inner *inner;   /* may be NULL while INIT */
	int state;                  /* enum stcp_life_state */
	/* add your fields here */
};

/* If you keep stcp_state via sk_user_data, provide this helper */
static inline struct stcp_state *stcp_sk(const struct sock *sk)
{
	return (struct stcp_state *)(sk ? sk->sk_user_data : NULL);
}

#define ST_GUARD_RET(x, msg, ret) \
	do { if (!(x)) { pr_err("stcp: %s\n", (msg)); return (ret); } } while (0)

#endif /* STCP_CORE_H */
