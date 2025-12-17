#pragma once
#include <linux/net.h>
#include <linux/socket.h>

#include <linux/uio.h>
#include <linux/kernel.h>

// Init.c:ssä alustus näille
extern int (*orginal_tcp_data_ready)(struct sock *sk);
extern int (*orginal_tcp_sendmsg)(struct sock *sk, struct msghdr *msg, size_t len);
extern int (*orginal_tcp_recvmsg)(struct sock *sk, struct msghdr *msg, size_t len, int flags, int *recv_len);

ssize_t stcp_tcp_recv(
    struct sock *sk,
    u8 *buf,
    size_t len,
    int non_blocking,
    u32 flags,
    int *recv_len);

ssize_t stcp_tcp_send(struct sock *sk, u8 *buf, size_t len);

