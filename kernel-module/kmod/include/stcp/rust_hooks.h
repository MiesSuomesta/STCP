// SPDX-License-Identifier: GPL
#pragma once
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/types.h>   // bbool
#include <linux/net.h>
#include <net/sock.h>
#include <net/inet_common.h>
#include <net/protocol.h>
#include <net/inet_connection_sock.h>

/* Kaikki palauttavat 0.. tai -Exxx; jos ei toteutettu, palauta -ENOSYS */

int stcp_rust_bind(struct socket *sock, struct sockaddr *addr, int addrlen);
int stcp_rust_connect(struct socket *sock, struct sockaddr *addr, int addrlen, int flags);
int stcp_rust_listen(struct socket *sock, int backlog);
void stcp_rust_close(struct sock *sk, long timeout);

/* accept: Rust voi halutessaan hoitaa new_sockin täytön; jos ei, palauta -ENOSYS */
int stcp_rust_accept(struct socket *parent, struct socket *child, int flags, bool kern);

/* perinteiset viestit */
int stcp_rust_sendmsg(struct socket *sock, struct msghdr *msg, size_t len);
int stcp_rust_recvmsg(struct socket *sock, struct msghdr *msg, size_t len, int flags);

/* muut */
int stcp_rust_shutdown(struct socket *sock, int how);
int stcp_rust_getname(struct socket *sock, struct sockaddr *addr, int *addrlen, int peer);
int stcp_rust_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg);
int stcp_rust_poll(struct file *file, struct socket *sock, void *wait); /* voit palauttaa -ENOSYS */
int stcp_rust_setsockopt(struct socket *sock, int level, int optname, sockptr_t optval, unsigned int optlen);
int stcp_rust_getsockopt(struct socket *sock, int level, int optname, char __user *optval, int __user *optlen);

