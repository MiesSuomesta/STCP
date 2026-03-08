#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stddef.h>
#include <stdint.h>


struct kernel_socket {
	int fd;
	void *kctx;
	void *resolved_host; // Kättelyssä täytetään, pitää closessa vapauttaa..
};

enum {
    CTX_STATE_INIT = 0,
    CTX_STATE_CONNECTING,
    CTX_STATE_CONNECTED,
    CTX_STATE_CLOSING,
};

typedef enum {
    STCP_FSM_INIT = 0,
    STCP_FSM_WAIT_LTE,
    STCP_FSM_WAIT_PDN,
    STCP_FSM_WAIT_IP,
    STCP_FSM_WAIT_STABLE,
    STCP_FSM_TCP_CONNECT,
    STCP_FSM_STCP_HANDSHAKE,
    STCP_FSM_RUN,
    STCP_FSM_TCP_RECONNECT,
    STCP_FSM_FATAL
} stcp_fsm_state_t;

struct stcp_ctx;

struct stcp_fsm {
    stcp_fsm_state_t state;
    struct k_sem connection_ready;
    struct stcp_ctx *ctx;
    bool stop;
};

// 2KB 
#define STCP_RECV_STREAM_BUF_SIZE (2*1024)

// 2KB 
#define STCP_RECV_FRAME_BUF_SIZE (2*1024)

#define STCP_MAX_HOSTNAME_LEN   128
#define STCP_MAX_PORT_LEN       8

struct stcp_recv_stream {
    uint8_t *buffer;
    size_t pos;
    size_t len;
};

struct stcp_ctx {
	void *session;   // Rust STCP session pointer
	struct stcp_fsm fsm;
	char hostname_str[STCP_MAX_HOSTNAME_LEN];
	char port_str[STCP_MAX_PORT_LEN];
	int handshake_done;
	atomic_t closing;
	atomic_t ctx_state;
	struct k_mutex lock;
	struct k_work cleanup_work;
	struct k_work connect_work;
	struct kernel_socket ks;

    // RX bufferi per konteksti
    char rx_stream_buffer[STCP_RECV_STREAM_BUF_SIZE];
    struct stcp_recv_stream rx_stream;
  
    // RX STCP frame puskuri
    char rx_frame[STCP_RECV_FRAME_BUF_SIZE];
    size_t rx_frame_len;

    // Koonti puskuri
    uint8_t rx_payload[STCP_RECV_FRAME_BUF_SIZE];
    size_t rx_payload_pos;
    size_t rx_payload_len;

};
