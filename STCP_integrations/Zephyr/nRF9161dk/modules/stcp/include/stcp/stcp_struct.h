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
#define STCP_RECV_STREAM_BUF_SIZE (512)

// 2KB 
#define STCP_RECV_FRAME_BUF_SIZE (512)

#define STCP_MAX_HOSTNAME_LEN   128
#define STCP_MAX_PORT_LEN       8

#define STCP_CTX_MAGIC_ALIVE    0x53544350
#define STCP_CTX_MAGIC_POISON   0xDEADBEEF

struct stcp_recv_stream {
    uint8_t *buffer;
    size_t pos;
    size_t len;
};

struct stcp_ctx {
    uint32_t magic; // Conteksti magikki

	struct k_mutex lock;
	struct k_work cleanup_work;
	struct stcp_fsm fsm;
    atomic_t refcnt;
    atomic_t closing;
    atomic_t connection_closed;
    atomic_t destroyed;
	int handshake_done;
    int poll_timeouts;
    size_t rx_frame_len;
    size_t rx_payload_pos;
    size_t rx_payload_len;

    void *session;   // Rust STCP session pointer
	void *api;       // Api instance this conntext belongs to. 
	struct kernel_socket ks;

    // Bufferit loppuun ... 
	char hostname_str[STCP_MAX_HOSTNAME_LEN];
	char port_str[STCP_MAX_PORT_LEN];

    // RX bufferi per konteksti
    char rx_frame[STCP_RECV_FRAME_BUF_SIZE];
    // RX STCP frame puskuri
    char rx_stream_buffer[STCP_RECV_STREAM_BUF_SIZE];
    struct stcp_recv_stream rx_stream;

    // Koonti puskuri
    uint8_t rx_payload[STCP_RECV_FRAME_BUF_SIZE];

};
