#include <net/sock.h>
#include <linux/printk.h>
#include <linux/socket.h>
#include <linux/list.h>
#include <linux/version.h>
#include <linux/printk.h>
#include <linux/socket.h>  
#include <linux/kernel.h>  // container_of
#include <net/sock.h>      // struct sock
#include <linux/net.h>     // struct socket
#include <net/inet_connection_sock.h>

// Omat headerit
#include <stcp/kmod.h>
#include <stcp/lifecycle.h>
#include <stcp/helpers.h>

inline struct stcp_state *stcp_state_from_socket(const struct socket *sock)
{
    struct sock *sk;

    if (!sock)
        return NULL;

    sk = READ_ONCE(sock->sk);
    if (!sk)
        return NULL;

    return (struct stcp_state *)READ_ONCE(sk->sk_user_data);
}

/* Kiinnitä state sk:hen */
inline void stcp_state_attach_state(struct sock *sk, struct stcp_state *st)
{
    WRITE_ONCE(sk->sk_user_data, (void *)st);
}

/* Irrota state sk:stä */
inline void stcp_state_detach_state(struct sock *sk)
{
    WRITE_ONCE(sk->sk_user_data, NULL);
}

/* Vapauta inner (idempotentti) */
inline void stcp_state_free_inner(struct stcp_state *st)
{
    if (!st)
        return;

    if (st->inner) {
        if (st->inner->sock) {
            sock_release(st->inner->sock);
            st->inner->sock = NULL;
        }
        kfree(st->inner);
        st->inner = NULL;
    }
}


//
// STCP helppereitä  (hmm .. eriyttää omaan fileensä?)
//
inline struct stcp_sock *stcp_from_sk(struct sock *sk)
{
    struct inet_connection_sock *icsk;

    if (!sk)
        return NULL;

    icsk = inet_csk(sk);
    if (!icsk)
        return NULL;

    /* stcp_sock EMBEDDOI icsk:n kentällä 'icsk' */
    return container_of(icsk, struct stcp_sock, icsk);
}

inline const struct stcp_sock *stcp_from_sk_const(const struct sock *sk)
{
    /* castataan pois const vain väliaikaisesti, koska inet_csk ei tarjoa const-versiota */
    return stcp_from_sk((struct sock *)sk);
}

inline struct stcp_sock *stcp_from_socket(const struct socket *sock)
{
    struct sock *sk;
    struct inet_connection_sock *icsk;

    if (!sock)
        return NULL;

    sk = sock->sk;
    if (!sk)
        return NULL;

    // inet_csk() palauttaa icsk:n (sis. inet_sock → sk)
    icsk = inet_csk(sk);
    if (!icsk)
        return NULL;

    // stcp_sock  EMBEDDOI  icsk:n nimellä 'icsk'
    return container_of(icsk, struct stcp_sock, icsk);
}

inline struct stcp_connection_phase *stcp_connection_phase_from_socket(const struct socket *sock)
{
    struct stcp_sock *stsk = stcp_from_socket(sock);
    if (unlikely(!stsk)) {
        pr_err("stcp: %s: stsk NULL\n", __func__);
        return NULL;
    }
    return stsk->phase;
}

inline void stcp_flag_set(struct stcp_connection_phase *st, unsigned int idx)
{
    if (unlikely(!st)) {
        pr_err("stcp: %s: st NULL\n", __func__);
        return ;
    }
    // optionaalinen rajatarkistus kehitysvaiheessa:
    if (WARN_ON_ONCE(idx >= STCPF_SOCKET_PHASE_MAX)) return;
    set_bit(idx, &st->phase);
}

inline void stcp_flag_clear(struct stcp_connection_phase *st, unsigned int idx)
{
    if (unlikely(!st)) {
        pr_err("stcp: %s: st NULL\n", __func__);
        return ;
    }
    if (WARN_ON_ONCE(idx >= STCPF_SOCKET_PHASE_MAX)) return;
    clear_bit(idx, &st->phase);
}

inline bool stcp_flag_is_set(const struct stcp_connection_phase *st, unsigned int idx)
{
    if (unlikely(!st)) {
        pr_err("stcp: %s: st NULL\n", __func__);
        return false;
    }
    if (WARN_ON_ONCE(idx >= STCPF_SOCKET_PHASE_MAX)) return false;
    return test_bit(idx, &st->phase);
}


inline struct stcp_sock *stcp_get_st(struct socket *sock, const char *fn)
{
    if (!sock) { pr_err("stcp: %s: sock NULL\n", fn); return NULL; }
    return stcp_from_socket(sock);
}

inline int stcp_guard_inner_required(struct stcp_sock *st, const char *fn)
{
    if (!st) { pr_err("stcp: %s: state NULL\n", fn); return -EINVAL; }
    if (!st->inner) { pr_err("stcp: %s: no inner\n", fn); return -EAGAIN; }
    if (!st->inner->sock) { pr_err("stcp: %s: no inner sock\n", fn); return -EAGAIN; }
    return 0;
}

inline int stcp_ensure_inner(struct stcp_sock *st, const char *fn)
{
    int err = 0;

    if (unlikely(!st)) { pr_err("stcp: %s: Not Connected?\n", fn); return -EINVAL; }

    // Kaikki on ok ?
    if (READ_ONCE(st->inner) && READ_ONCE(st->inner->sock)) return 0;

    pr_debug("stcp: Create inner?\n");
    spin_lock(&st->inner_lock);
    pr_debug("stcp: in locked area\n");

    if ((!st->inner) || (!st->inner->sock)) {
        pr_debug("stcp: Creating inner...\n");
        err = stcp_inner_create(st);
        pr_debug("stcp:Inned done? %d\n", err);
    }

    pr_debug("stcp: Going out of locked area\n");
    spin_unlock(&st->inner_lock);
    pr_debug("stcp: Out of lock... %d\n", err);

    return err;
}

inline int stcp_ensure_phase(struct stcp_sock *st, const char *fn)
{
    int err = 0;

    if (unlikely(!st)) { pr_err("stcp: %s: Not Connected?\n", fn); return -EINVAL; }

    // Kaikki on ok ?
    if (READ_ONCE(st->phase)) { pr_debug("stcp: %s: All ok\n", fn); return 0; };

    if (!st->phase) {
        pr_debug("stcp: Creating phase...\n");
            unsigned long flags;
            spin_lock_irqsave(&st->inner_lock, flags);
            if (!st->phase) {
                struct stcp_connection_phase *ph = kzalloc(sizeof(*ph), GFP_KERNEL);
                if (!ph) {
                    spin_unlock_irqrestore(&st->inner_lock, flags);
                    pr_err("stcp: %s No memory?\n", __func__);
                    return -ENOMEM;
                }
                spin_lock_init(&ph->phase_lock);
                ph->phase = 0;
                st->phase = ph;
                pr_debug("stcp: Phase init done\n");
            }
            spin_unlock_irqrestore(&st->inner_lock, flags);
        pr_debug("stcp: Phase done?\n");
    }

    return err;
}

// State apureita: stcp_sock_ctx 
inline struct stcp_sock_ctx *stcp_get_context_from_socket(const struct socket *sock)
{
    struct sock *sk = sock ? sock->sk : NULL;
    return (struct stcp_sock_ctx *)(sk ? sk->sk_user_data : NULL);
}

inline int stcp_need_context_from_socket(const struct socket *sock,
                                  struct stcp_sock_ctx **out)
{
    struct stcp_sock_ctx *st = stcp_get_context_from_socket(sock);
    if (unlikely(!st)) return -EINVAL;
    *out = st;
    return 0;
}

/* Palauttaa 0 kun kaikki on kunnossa ja *out_st osoittaa validiin st:hen.
 * Voi allokoida/ottaa lukkoja; ÄLÄ kutsu atomisessa kontekstissa. */
inline int stcp_ensure_all_ok(struct socket *sock,
                                     struct stcp_sock **out_st,
                                     const char *fn)
{
    int r = 0;
    struct stcp_sock *st;

    if (!sock || !sock->sk)
        return -ENOTCONN;

    st = stcp_from_socket(sock);            /* mielellään READ_ONCE sisällä */
    if (!st)
        return -ENOTCONN;

    pr_debug("stcp: %s: have st=%p\n", fn, st);

    /* varmista inner ennen kuin kosket phaseen */
    r = stcp_ensure_inner(st, fn);
    pr_debug("stcp: %s: ensure_inner -> %d\n", fn, r);
    if (r)
        return r;

    /* varmista phase ennen kuin käytät *_phase/_state apureita */
    r = stcp_ensure_phase(st, fn);
    pr_debug("stcp: %s: ensure_phase -> %d\n", fn, r);
    if (r)
        return r;

    if (out_st)
        *out_st = st;

    return 0;
}

