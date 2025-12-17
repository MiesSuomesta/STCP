#pragma once
#include <linux/net.h>
#include <linux/socket.h>
#include <linux/inet.h>
#include <linux/tcp.h>
#include <linux/slab.h>

    
#include <stcp/stcp_socket_struct.h>
#include "proto_layer.h"


struct stcp_sock *       stcp_struct_alloc_st(void);
void                     stcp_struct_free_st(struct stcp_sock *st);
void                     stcp_detach_from_sock(struct stcp_sock *st);

int                      stcp_struct_attach_st_to_socket(struct stcp_sock *st, struct socket *sock);
int                      stcp_struct_attach_st_to_sk(struct stcp_sock *st, struct sock *sk);

inline void              stcp_rust_glue_socket_op_data_ready(struct sock *sk);
inline struct stcp_sock *stcp_struct_get_st_from_socket(struct socket *sock);
inline struct stcp_sock *stcp_struct_get_st_from_sk(struct sock *sk);
inline struct stcp_sock *stcp_struct_get_st_from_sk_for_destroy(struct sock *sk);

inline struct stcp_sock *stcp_struct_get_or_alloc_st_from_sk(struct sock *sk);
inline struct stcp_sock *stcp_struct_get_or_alloc_st_from_socket(struct socket *sock);
