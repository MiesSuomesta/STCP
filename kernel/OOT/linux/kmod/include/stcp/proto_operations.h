#pragma once
#include <linux/net.h>
#include <linux/socket.h>
#include <linux/inet.h>
#include <linux/tcp.h>
#include <linux/slab.h>

#include <stcp/proto_layer.h>   // Rust proto_ops API
#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>

/* ----------------------------------------- */
/*  CONNECT                                  */
/* ----------------------------------------- */
/*
int stcp_rust_glue_proto_op_connect(struct sock *sk,
                          struct sockaddr *addr,
                          int addr_len);
*/
/* ----------------------------------------- */
/*  ACCEPT                                   */
/* ----------------------------------------- */
/*
struct sock *stcp_rust_glue_proto_op_accept(
    struct sock *sk,
    struct proto_accept_arg *arg);

*/

/* ----------------------------------------- */
/*  SENDMSG (STCP encrypted send)            */
/* ----------------------------------------- */

int stcp_rust_glue_proto_op_sendmsg(struct sock *sk,
                          struct msghdr *msg,
                          size_t len);

/* ----------------------------------------- */
/*  RECVMSG (STCP encrypted recv)            */
/* ----------------------------------------- */

int stcp_rust_glue_proto_op_recvmsg(
    struct sock *sk,
    struct msghdr *msg,
    size_t len,
    int flags,
    int *recv_len);

/* ----------------------------------------- */
/*  RELEASE (STCP release)                   */
/* ----------------------------------------- */

int stcp_rust_glue_proto_op_release(struct sock *sk);

/* ----------------------------------------- */
/*  Destroy (STCP destroy)                   */
/* ----------------------------------------- */

void stcp_rust_glue_proto_op_destroy(struct sock *sk);
