#ifndef STCP_H
#define STCP_H

#include <linux/net.h>
#include <linux/skbuff.h>
#include <linux/socket.h>
#include <net/sock.h>

#define AF_STCP 45
#define PF_STCP AF_STCP
#define STCP_PROTO_ID 253
#define STCP_MAX_BACKLOG 128

struct stcp_sock {
    struct sock sk;
    void *rust_ctx;
    wait_queue_head_t accept_wq;
};

static inline struct stcp_sock *stcp_sk(const struct sock *sk)
{
    return container_of(sk, struct stcp_sock, sk);
}

int stcp_proto_register(void);
void stcp_proto_unregister(void);

#endif
