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

// pahse flagsit
enum stcp_security_layer_phase_flag_idx {
    STCPF_PHASE_INIT = 0,
    /*
        Bitti päällä => Public key moodissa.
     */
    STCPF_PHASE_IN_PUBLIC_KEY_MODE, // Bitti päällä jos pubkey moodissa

    /*
        Bitti päällä => AES moodissa.
     */
    STCPF_PHASE_IN_AES_TRAFFIC_MODE,

    /*
        Bitti päällä => Publick keyt ok.
     */
    STCPF_PHASE_GOT_PUBLIC_KEYS,
    /*
        Bitti päällä => jaettu avain on ok.
     */
    STCPF_PHASE_GOT_SHARED_SECRET,
    /*
        Bitti päällä => Molemmat public key ja AES on päällä
     */
    STCPF_PHASE_CONNECTED,

    /*
        Bitti päällä => sokettia ajetaan alas.
     */
    STCPF_PHASE_CLOSING,
    /*
        Bitti päällä => soketti suljettu.
     */
    STCPF_PHASE_CLOSED,
    /*
        Bitti päällä => soketti kuollu.
     */
    STCPF_PHASE_DEAD,
    STCPF_SECURITY_LAYER_PHASE_MAX
};

enum stcp_sock_state_flag_idx {
    STCPF_STATE_INIT = 0,
    STCPF_STATE_INNER_CREATING,
    STCPF_STATE_READY,
    STCPF_STATE_CLOSING,
    STCPF_STATE_CLOSED,
    STCPF_STATE_DEAD,
    STCPF_SOCKET_PHASE_MAX
};

struct stcp_sock;

extern inline unsigned long stcp_sock_phase_get(struct stcp_sock *st);
extern inline void stcp_sock_phase_set(struct stcp_sock *st, unsigned long p);

extern inline unsigned long stcp_sock_state_get(struct stcp_sock *st);
extern inline void stcp_sock_state_set(struct stcp_sock *st, unsigned long p);
// Rusti puolella noit...
//inline unsigned long stcp_security_layer_phase_get(struct stcp_sock *st);
//inline void stcp_security_layer_phase_set(struct stcp_sock *st, unsigned long p);
