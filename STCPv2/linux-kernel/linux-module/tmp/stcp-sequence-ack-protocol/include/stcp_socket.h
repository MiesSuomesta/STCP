#ifndef STCP_SOCKET_H
#define STCP_SOCKET_H

#include <linux/wait.h>
#include <net/sock.h>

struct stcp_sock {
	struct sock sk;
	void *rust_ctx;
	wait_queue_head_t accept_wq;
	wait_queue_head_t recv_wq;
};

static inline struct stcp_sock *stcp_sk(struct sock *sk)
{
	return container_of(sk, struct stcp_sock, sk);
}

#endif
