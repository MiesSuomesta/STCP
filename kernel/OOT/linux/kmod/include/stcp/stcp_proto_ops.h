#pragma once
#include <linux/net.h>
#include <linux/socket.h>
#include <linux/inet.h>
#include <linux/tcp.h>
#include <linux/slab.h>

#include <stcp/proto_layer.h>   // Rust proto_ops API
#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

int stcp_proto_socket_ops_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len);
int stcp_proto_socket_ops_listen(struct socket *sock, int backlog);
int stcp_proto_socket_ops_connect(struct socket *sock, struct sockaddr *uaddr, int addr_len, int flags);
int stcp_proto_socket_ops_accept(struct socket *sock, struct socket *newsock, struct proto_accept_arg *arg);
int stcp_proto_socket_ops_release(struct socket *sock);

// stcp_proto_ops.c
#include <linux/kernel.h>
#include <linux/net.h>
#include <net/sock.h>
#include <net/inet_common.h>
#include <net/tcp.h>

#include <stcp/debug.h>
#include <stcp/settings.h>
#include <stcp/tcp_callbacks.h>
#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/handshake_worker.h>
#include <stcp/stcp_proto_ops.h>

/* ========== BIND ========== */
int stcp_proto_socket_ops_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len);

/* ========== LISTEN ========== */
int stcp_proto_socket_ops_listen(struct socket *sock, int backlog);

/* ========== ACCEPT ========== */
int stcp_proto_socket_ops_accept(struct socket *sock, struct socket *newsock, struct proto_accept_arg *arg);

/* ========== CONNECT ========== */
int stcp_proto_socket_ops_connect(struct socket *sock, struct sockaddr *uaddr, int addr_len, int flags);

/* ========== RELEASE ========== */
int stcp_proto_socket_ops_release(struct socket *sock);