#pragma once
#include <linux/net.h>
#include <linux/socket.h>
#include <linux/inet.h>
#include <linux/tcp.h>
#include <linux/slab.h>

#include <stcp/proto_layer.h>   // Rust proto_ops API
#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>

enum stcp_hs_reason {
    REASON_SEND_BEFORE_SESSION = 1,
    REASON_SEND_BEFORE_HS      = 2,
    REASON_RECV_BEFORE_HS      = 3,
};

inline bool stcp_glue_call_is_nonblock(const struct sock *sk, int flags);

/* Palauttaa 0 jos HS ok, tai negatiivisen errno:n */
inline int stcp_glue_ensure_handshake(struct stcp_sock *st,
                                 struct sock *sk,
                                 int flags,
                                 int reason);
