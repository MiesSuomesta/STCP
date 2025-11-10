#pragma once
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

#define WITH_STATE_HISTORY 0

#define STCP_SOCK_MAGIC 0x53544350u /* 'STCP' */
#define STCP_BM_WORDS(x) BITS_TO_LONGS((x))

#include <stcp/lifecycle.h>

#define STCP_H_STRUCTURES 1

#define STCP_STATE_ESTABLISHED   TCP_ESTABLISHED
#define STCP_STATE_CLOSING       TCP_CLOSING

extern int stcp_safe_mode;

/* Sisäinen rakenne: wrapataan “alaprotokollan” socket tähän */
struct stcp_inner {
    struct socket *sock;   /* esim. TCP/UDP/… johon STCP kääriytyy */
};

/* Per-socket STCP state, johon viitataan sk->sk_user_data:n kautta */
struct stcp_state {
    enum stcp_sock_state_flag_idx state;
    struct stcp_inner *inner;
    /* halutessa tänne mutex/spinlock/counters/debug jne. */
    int cleanup_done; 
};

/* stcp_sock **alkaa** struct sock:lla */
struct stcp_sock {
    struct inet_connection_sock  icsk;
    struct socket *parent;
    struct stcp_inner *inner;
    struct stcp_connection_phase *phase; // missä vaiheessa mennään
    int state; // soketin tila

    /* Accept-välivarasto lapsen innerille (yläkerros siirtää sen oikeaan stcp_sockiin) */
    struct socket *accept_pending;

    spinlock_t       lock;
    spinlock_t       inner_lock;
    struct list_head accept_q;
    wait_queue_head_t waitq;
    u32              magic;

    /* ÄLÄ KOSKE DATAAN, rust puolen dataa, kernelin ei tarvitse välittää
       siitä ollenkaan .. sen elinkaaren hallinta on rust puolella täysin
       Ainoa joka kernelipuolen pitää tehdä on releasessa vapauttaa lukko.
     */
    void              *rust;
    spinlock_t         rust_lock;

};

struct stcp_connection_phase {
    /* oma protokollan tila */
    unsigned long int phase;  // Enum stcp_sock_state_flag_idx
// Rust toteutukseen:    unsigned long int security_layer_phase; // Enum stcp_security_layer_phase_flag_idx
    spinlock_t phase_lock;   
};

/* Per-socket STCP-konteksti */
struct stcp_sock_ctx {
    /* turvamaski */
    u32                magic;

    /* linkitys */
    struct socket     *sock;       /* käyttäjän näkyvä stcp-socket */
    struct sock       *sk;         /* sock-ydinrakenne (sock->sk) */

    /* sisäinen (wrapattu) kuljetus-socket, esim TCP tms. */
    struct stcp_inner *inner;

    /* fine-grained tila ja historia */
    u32                current_state;  /* enum stcp_sock_state_flag_idx */
    u32                current_phase;  /* enum stcp_security_layer_phase_flag_idx */

#if WITH_STATE_HISTORY
    unsigned long      state_hist[STCP_BM_WORDS(STCPF_SOCKET_PHASE_MAX)];
    unsigned long      phase_hist[STCP_BM_WORDS(STCPF_SECURITY_LAYER_PHASE_MAX)];
#endif

    /* elinkaari */
    atomic_t           refcnt;

    /* kevyt lukko state/phase päivityksille */
    spinlock_t         lock;
};
