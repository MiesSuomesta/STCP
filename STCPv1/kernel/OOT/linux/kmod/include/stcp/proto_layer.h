#ifndef PROTO_LAYER_H
#define PROTO_LAYER_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/net.h>
#include <linux/errno.h>

#include <net/sock.h>
#include <net/inet_common.h>
#include <net/inet_connection_sock.h>
#include <net/protocol.h>
#include <net/tcp.h>
#include <linux/inet.h>
#include <linux/rcupdate.h>
#include <linux/refcount.h>

#include <stcp/stcp_misc.h>
#include <stcp/state.h>

#ifdef __cplusplus
extern "C" {
#endif

// STCP
#define STCP_MAGIC_ALIVE      0x53544350u
#define STCP_MAGIC_DEAD       0xDEADBEEFu


/* 
 * Opaakki sessiotyyppi protokollalle.
 * Todellinen sisältö on C- tai Rust-implementeissä.
 */
typedef struct proto_session proto_session_t;

#define START_HANDSHAKE_SIDE_SERVER          1
#define START_HANDSHAKE_SIDE_CLIENT          0

struct stcp_sock {
    unsigned int                           magic;
    struct sock                           *sk;
    
    proto_session_t                       *session;
    enum stcp_handshake_status             handshake_status;
    unsigned long                          flags; // Bitti fieldi;
    int                                    is_server;
    
    struct delayed_work                    handshake_work;
    struct workqueue_struct               *the_wq;
    void                                 (*orginal_data_ready)(struct sock *sk);

    struct work_struct                     session_destroy_work;
    struct workqueue_struct               *the_wq_session_destroy;

    // Bind addr & lenght
    struct sockaddr_storage                bind_addr;
    int                                    bind_addr_len;

    struct completion                      hs_done;   // Käytössä
    int                                    hs_result; // käytössä
    int                                    hs_running;
    int                                    hs_need_wakeup;

    // Moduulin sisäinen TCP yhteys
    struct socket                         *transport;
    struct socket                         *listener;

    void                                 (*orginal_state_change)(struct sock *sk);
    struct proto                          *orig_sk_prot;

    int                                    pump_counter;
    struct rcu_head                        rcu;
    refcount_t                             refcnt;
    rwlock_t                               sk_callback_lock;

};

#ifdef __cplusplus
}
#endif

#endif /* PROTO_LAYER_H */
