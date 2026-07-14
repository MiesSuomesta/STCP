#ifndef STCP_PROTO_H
#define STCP_PROTO_H

#include <linux/net.h>
#include <net/sock.h>

extern const struct proto_ops stcp_proto_ops;
extern struct proto stcp_proto;
int stcp_proto_register(void);
void stcp_proto_unregister(void);
struct sock *stcp_alloc_child_sock(struct net *net, struct socket *newsock);

#endif
