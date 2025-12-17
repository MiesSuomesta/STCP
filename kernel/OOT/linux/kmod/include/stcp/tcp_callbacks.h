#pragma once 
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

// Signatuurit OK, tarkastettu 14.12.2025 klo 20:10
extern int           (*orginal_tcp_sendmsg)(struct sock *sk, struct msghdr *msg, size_t len);
extern int           (*orginal_tcp_recvmsg)(struct sock *sk, struct msghdr *msg, size_t len, int flags, int *recv_len);
extern int           (*orginal_tcp_listen) (struct socket *sk, int backlog);
extern int           (*orginal_tcp_connect)(struct sock *sk, struct sockaddr *uaddr, int addr_len);
extern struct sock * (*orginal_tcp_accept) (struct sock *sk, struct proto_accept_arg *arg);
extern int           (*orginal_tcp_bind)   (struct sock *sk, struct sockaddr *uaddr, int addr_len);
extern void          (*orginal_tcp_destroy) (struct sock *sk);