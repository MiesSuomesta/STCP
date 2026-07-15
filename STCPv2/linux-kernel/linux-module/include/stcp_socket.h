#ifndef STCP_SOCKET_H
#define STCP_SOCKET_H

#include <linux/wait.h>
#include <linux/workqueue.h>
#include <net/sock.h>

struct stcp_sock {
	struct sock sk;
	void *rust_ctx;
	struct stcp_carrier *carrier;

	wait_queue_head_t accept_wq;
	wait_queue_head_t recv_wq;

	struct delayed_work retransmit_work;
	bool retransmit_work_started;
};

static inline struct stcp_sock *stcp_sk(struct sock *sk)
{
	return container_of(sk, struct stcp_sock, sk);
}

#endif
