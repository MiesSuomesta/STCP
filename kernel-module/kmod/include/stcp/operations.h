#pragma once
#include <net/sock.h>
#include <linux/printk.h>
#include <linux/socket.h>
#include <linux/list.h>
#include <linux/version.h>
#include <linux/printk.h>
#include <linux/net.h>
#include <linux/kernel.h>  // container_of
#include <net/sock.h>      // struct sock
#include <linux/net.h>     // struct socket
#include <linux/poll.h>  // __poll_t

#include "kmod.h"
#include "structures.h"

// Operaatiot ja niiden esittely...
int stcp_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len);
int stcp_connect(struct socket *sock, struct sockaddr *addr, int addr_len, int flags);
void stcp_close(struct sock *sk, long timeout);
int stcp_listen(struct socket *sock, int backlog);
int stcp_sendmsg(struct socket *sock, struct msghdr *msg, size_t len);
int stcp_recvmsg(struct socket *sock, struct msghdr *msg, size_t len, int flags);
int stcp_shutdown(struct socket *sock, int how);
int stcp_getname(struct socket *sock, struct sockaddr *uaddr, int peer);
int stcp_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg);
int stcp_setsockopt(struct socket *sock, int level, int optname, sockptr_t optval, unsigned int optlen);
int stcp_getsockopt(struct socket *sock, int level, int optname, char __user *optval, int __user *optlen);

/* Yhtenäinen accept-glue kaikille kernelihaarille */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,8,0)
/* Kernel 6.8+: accept(struct socket *sock, struct socket *newsock, struct proto_accept_arg *arg) */
int stcp_accept_glue(struct socket *sock, struct socket *newsock,
                     struct proto_accept_arg *arg);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(5,9,0)
/* Kernel 5.9+ .. <6.8: accept(struct socket *sock, struct socket *newsock, int flags, bool kern) */
int stcp_accept_glue(struct socket *sock, struct socket *newsock,
                     int flags, bool kern);
#else
/* Vanhat puut: accept(struct socket *sock, struct socket *newsock, int flags) — ei kern-paramia */
int stcp_accept_glue(struct socket *sock, struct socket *newsock,
                     int flags);

#endif /* version split */

