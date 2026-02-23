#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>
#include <errno.h>
#include "stcp.h"

#ifndef STCP_H
struct kernel_socket {
	int fd;
	void *kctx;
	void *resolved_host;
};
#endif

enum {
    CTX_STATE_INIT = 0,
    CTX_STATE_CONNECTING,
    CTX_STATE_CONNECTED,
    CTX_STATE_CLOSING,
};

struct stcp_ctx {
	int tcp_fd;
	void *session;   // Rust STCP session pointer
    int handshake_done;
	atomic_t closing;
	atomic_t ctx_state;
	struct k_mutex lock;
	struct k_work cleanup_work;
	struct k_work connect_work;
	struct kernel_socket ks;
};

