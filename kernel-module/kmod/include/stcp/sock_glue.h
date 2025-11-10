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

// Omat headerit
#define USE_SAFEGUARD 1
#include "kmod.h"
#include "lifecycle.h"
#include "helpers.h"

// Moduulin exporttaamat liput.. (proto.c definoi)
extern int stcp_safe_mode;
extern int stcp_debug_mode;

/* Palauta STCP:n oma stcp_sock runko kernelin socketista.
 * Edellytys: struct stcp_sock alkaa kentällä `struct sock sk;`
 */
/* Perusmuunnos sk -> stcp_sock */

/* ---- Delegoitavat operaatiofunktiot (toteutus stcp_sock_glue.c:ssä) ---- */
extern void stcp_inner_destroy(struct stcp_sock *st);

extern int stcp_release(struct socket *sock);
extern int stcp_bind(struct socket *sock, struct sockaddr *uaddr, int addrlen);
extern int stcp_listen(struct socket *sock, int backlog);
extern void stcp_close(struct sock *sk, long timeout);

/* accept – eri signatuurit kerneliversion mukaan */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,8,0)
extern int stcp_accept_glue(struct socket *sock, struct socket *newsock,
                            struct proto_accept_arg *arg);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(5,9,0)
extern int stcp_accept_glue(struct socket *sock, struct socket *newsock,
                            int flags, bool kern);
#else
extern int stcp_accept_glue(struct socket *sock, struct socket *newsock,
                            int flags);
#endif

extern int stcp_getname(struct socket *sock, struct sockaddr *uaddr, int peer);
extern int stcp_setsockopt(struct socket *sock, int level, int optname,
                    sockptr_t optval, unsigned int optlen);
extern int stcp_getsockopt(struct socket *sock, int level, int optname,
                    char __user *optval, int __user *optlen);
extern int stcp_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg);
extern int stcp_shutdown(struct socket *sock, int how);
extern __poll_t stcp_poll(struct file *file, struct socket *sock, poll_table *wait);
extern int stcp_connect(struct socket *sock, struct sockaddr *uaddr, int addrlen, int flags);
extern int stcp_sendmsg(struct socket *sock, struct msghdr *msg, size_t size);
extern int stcp_recvmsg(struct socket *sock, struct msghdr *msg, size_t size, int flags);

/* API: per-socket inner elinkaari */
int stcp_inner_create(struct stcp_sock *st);  // ← VAIN yksi parametri
int stcp_inner_release(struct stcp_sock *st);
int stcp_inner_free(struct stcp_sock *st);
void stcp_inner_destroy(struct stcp_sock *st);

int stcp_inner_bind(struct stcp_sock *st, struct sockaddr *uaddr, int addr_len);
int stcp_inner_listen(struct stcp_sock *st, int backlog);
int stcp_inner_connect(struct stcp_sock *st, struct sockaddr *addr, int addr_len, int flags);
void stcp_inner_close(struct sock *sk, long timeout);

int stcp_inner_accept(struct stcp_sock *parent, struct stcp_sock **out_child, int flags);
int stcp_inner_set(struct stcp_sock *st, struct socket *sock);
int stcp_inner_shutdown(struct stcp_sock *st, int how);
int stcp_inner_sendmsg(struct stcp_sock *st, struct msghdr *msg, size_t len);
int stcp_inner_recvmsg(struct stcp_sock *st, struct msghdr *msg, size_t len, int flags);
int stcp_inner_destroy_ex(struct stcp_sock *st);
int stcp_inner_getname(struct stcp_sock *st, struct sockaddr *uaddr, int *uaddr_len, int peer);
