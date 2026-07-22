#ifndef STCP_SOCKET_H
#define STCP_SOCKET_H

#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/types.h>
#include <net/sock.h>

struct stcp_sock {
	struct sock sk;
	void *rust_ctx;
	struct stcp_carrier *carrier;

	wait_queue_head_t accept_wq;
	wait_queue_head_t recv_wq;

	struct delayed_work retransmit_work;
	bool retransmit_work_started;

	struct mutex tx_lock;
	struct mutex rx_lock;
	u8 *tx_buffer;
	size_t tx_buffer_size;
	u8 *rx_buffer;
	size_t rx_buffer_size;

	/* Active-users registry used by the graceful unload helper. */
	struct list_head user_node;
	pid_t owner_tgid;
	bool user_registered;
};

static inline struct stcp_sock *stcp_sk(struct sock *sk)
{
	return container_of(sk, struct stcp_sock, sk);
}

#endif
