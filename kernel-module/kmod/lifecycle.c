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


#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

// Omat headerit
#include <stcp/kmod.h>
#include <stcp/lifecycle.h>

inline unsigned long stcp_sock_phase_get(struct stcp_sock *st)
{
    unsigned long p;
    if (!st) { return -1; }
    if (!st->phase)  { return -2; }

    spin_lock(&st->phase->phase_lock);
    p = st->phase->phase;
    spin_unlock(&st->phase->phase_lock);
    return p;
}

inline void stcp_sock_phase_set(struct stcp_sock *st, unsigned long p)
{
    if (!st) { 
        pr_err("stcp: %s: Error: no st", __func__);
        return;
    }
    if (!st->phase)  { 
        pr_err("stcp: %s: Error: no phase", __func__);
        return;
    }

    spin_lock(&st->phase->phase_lock);
    st->phase->phase = p;
    spin_unlock(&st->phase->phase_lock);
}

inline unsigned long stcp_sock_state_get(struct stcp_sock *st)
{
    unsigned long p;
    if (!st) {
        pr_err("stcp: %s: Error: no st", __func__);
        return (unsigned long)-22;
    }

    if (!st->phase) {
        pr_err("stcp: %s: Error: no phase", __func__);
        return (unsigned long)-22;
    }

    spin_lock(&st->phase->phase_lock);
    p = st->state;
    spin_unlock(&st->phase->phase_lock);
    return p;
}

inline void stcp_sock_state_set(struct stcp_sock *st, unsigned long p)
{
    if (!st) { 
        pr_err("stcp: %s: Error: no st", __func__);
        return;
    }

    spin_lock(&st->phase->phase_lock);
    st->state = p;
    spin_unlock(&st->phase->phase_lock);
}

/* RUST toteutukseen !
inline unsigned long stcp_security_layer_phase_get(struct stcp_sock *st)
{
    unsigned long p;
    spin_lock(&st->phase->phase_lock);
    p = st->phase->security_layer_phase;
    spin_unlock(&st->phase->phase_lock);
    return p;
}

inline void stcp_security_layer_phase_set(struct stcp_sock *st, unsigned long p)
{
    spin_lock(&st->phase->phase_lock);
    st->phase->security_layer_phase = p;
    spin_unlock(&st->phase->phase_lock);
}
*/
