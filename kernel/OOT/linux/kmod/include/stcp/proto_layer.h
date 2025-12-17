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
#include <stcp/stcp_misc.h>

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

#define STCP_STATUS_HANDSHAKE_PENDING       (1 <<  0)
#define STCP_STATUS_HANDSHAKE_FAILED        (1 <<  1)
#define STCP_STATUS_HANDSHAKE_COMPLETE      (1 <<  2)
#define STCP_STATUS_HANDSHAKE_SERVER        (1 <<  3)
#define STCP_STATUS_HANDSHAKE_CLIENT        (1 <<  4)
#define STCP_STATUS_HANDSHAKE_EXIT_MODE     (1 <<  5)
#define STCP_STATUS_SOCKET_BOUND            (1 <<  6)
#define STCP_STATUS_SOCKET_LISTENING        (1 <<  7)
#define STCP_STATUS_SOCKET_CONNECTED        (1 <<  8)
#define STCP_STATUS_SOCKET_FATAL_ERROR      (1 <<  9)
#define STCP_STATUS_HANDSHAKE_STARTED       (1 << 10)
#define STCP_STATUS_STATECHANGE_HOOKED      (1 << 11)

#define STCP_STATUS_HS_QUEUED_BIT           (12)
#define STCP_STATUS_HS_QUEUED               (1 << STCP_STATUS_HS_QUEUED_BIT)


#define START_HANDSHAKE_SIDE_SERVER          1
#define START_HANDSHAKE_SIDE_CLIENT          0

#define STCP_STATUS_HANDSHAKE_TYPE_MASK (STCP_STATUS_HANDSHAKE_SERVER | STCP_STATUS_HANDSHAKE_CLIENT)

struct stcp_sock {
    unsigned int              magic;
    struct sock              *sk;
    
    proto_session_t          *session;
    unsigned long             status;
    int                       is_server;
    
    struct delayed_work       handshake_work;
    struct workqueue_struct  *the_wq;
    void                    (*orginal_data_ready)(struct sock *sk);

    // Bind addr & lenght
    struct sockaddr_storage   bind_addr;
    int                       bind_addr_len;

    struct completion         hs_done;   // Käytössä
    int                       hs_result; // käytössä

    int                       hs_running;
    int                       hs_need_wakeup;

    // Moduulin sisäinen TCP yhteys
    struct socket            *transport;
    struct socket            *listener;

    void                    (*orginal_state_change)(struct sock *sk);
    

};

#ifdef __cplusplus
}
#endif

#endif /* PROTO_LAYER_H */
