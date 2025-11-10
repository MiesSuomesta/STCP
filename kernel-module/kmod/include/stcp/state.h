#pragma once
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/bug.h>
#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/bitmap.h>
#include <linux/bitops.h>
#include <linux/rcupdate.h>
#include <linux/rculist.h>
#include <linux/printk.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <net/sock.h>
#include <net/inet_connection_sock.h>

#include <stcp/lifecycle.h>
#include <stcp/structures.h>
#include <stcp/kmod.h>  // Includaa structit...

/* --- API --- */

/* allo/ini/free */
struct stcp_sock_ctx *stcp_sock_ctx_alloc(struct socket *sock);
void stcp_ctx_sock_get(struct stcp_sock_ctx *st);
void stcp_ctx_sock_put(struct stcp_sock_ctx *st);

/* user_data <-> stcp_sock */
static inline void stcp_ctx_set_user_data(struct sock *sk, struct stcp_sock_ctx *st)
{
    WRITE_ONCE(sk->sk_user_data, st);
}
static inline struct stcp_sock_ctx *stcp_ctx_get_user_data(const struct sock *sk)
{
    return (struct stcp_sock_ctx *)READ_ONCE(sk->sk_user_data);
}

/* haku socketista/sockista */
static inline struct stcp_sock_ctx *stcp_ctx_from_socket(const struct socket *sock)
{
    if (!sock || !sock->sk) return NULL;
    return stcp_ctx_get_user_data(sock->sk);
}
static inline struct stcp_sock_ctx *stcp_ctx_from_sk(const struct sock *sk)
{
    if (!sk) return NULL;
    return stcp_ctx_get_user_data(sk);
}

/* phase/state – get/set + hist */
static inline u32 stcp_sock_ctx_state_get(const struct stcp_sock_ctx *st)
{
    return READ_ONCE(st->current_state);
}
static inline u32 stcp_sock_ctx_phase_get(const struct stcp_sock_ctx *st)
{
    return READ_ONCE(st->current_phase);
}

/* aseta ja kirjaa historiaan; palauttaa vanhan arvon */
static inline u32 stcp_sock_ctx_state_set(struct stcp_sock_ctx *st, u32 s)
{
    unsigned long flags;

    u32 old;

    spin_lock_irqsave(&st->lock, flags);

    old = st->current_state;
    st->current_state = s;

#if WITH_STATE_HISTORY
    if (s < STCPF_SOCKET_PHASE_MAX)
        __set_bit(s, st->state_hist);
#endif

    spin_unlock_irqrestore(&st->lock, flags);
    return old;
}
static inline u32 stcp_sock_ctx_phase_set(struct stcp_sock_ctx *st, u32 p)
{
    unsigned long flags;
    u32 old;

    spin_lock_irqsave(&st->lock, flags);

    old = st->current_phase;

    st->current_phase = p;

#if WITH_STATE_HISTORY
    if (p < STCPF_SECURITY_LAYER_PHASE_MAX)
        __set_bit(p, st->phase_hist);
#endif 

    spin_unlock_irqrestore(&st->lock, flags);

    return old;
}

/* testaa onko joskus käyty tietyssä tilassa/fasessa */
extern inline bool stcp_sock_ctx_state_ever(const struct stcp_sock_ctx *st, u32 s);
extern inline bool stcp_sock_ctx_phase_ever(const struct stcp_sock_ctx *st, u32 p);

extern void stcp_sock_ctx_get(struct stcp_sock_ctx *theContext);
extern void stcp_sock_ctx_put(struct stcp_sock_ctx *theContext);
extern void stcp_state_debug_dump(const struct stcp_sock_ctx *st, const char *tag);
extern void stcp_ctx_state_debug_dump(const struct stcp_sock_ctx *st, const char *tag);

