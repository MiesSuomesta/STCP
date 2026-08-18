#ifndef STCP2_INTERNAL_H
#define STCP2_INTERNAL_H
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp.h>

enum stcp_state {
    STCP_FREE=0, STCP_CREATED, STCP_BOUND, STCP_LISTENING,
    STCP_CONNECTING, STCP_CONNECTED, STCP_CLOSED
};

struct stcp_ctx {
    bool used;
    int fd;
    int socket_type;
    int protocol;
    int carrier_fd;
    int last_error;
    enum stcp_state state;
    struct sockaddr_in local;
    struct sockaddr_in peer;
    struct k_mutex lock;
#ifdef CONFIG_STCP_RUST_CORE
    void *rust_ctx;
    struct k_sem rust_event;
    struct k_thread rust_rx_thread;
    K_KERNEL_STACK_MEMBER(rust_rx_stack, CONFIG_STCP_RUST_RX_STACK_SIZE);
    volatile bool rust_rx_stop;
    volatile bool rust_rx_running;
#endif
};

struct stcp_ctx *stcp_ctx_alloc(void);
void stcp_ctx_free(struct stcp_ctx *ctx);
int stcp_carrier_open(int socket_type);
int stcp_carrier_wait_connected(int fd, int timeout_ms);
#ifdef CONFIG_STCP_RUST_CORE
int stcp_rust_rx_start(struct stcp_ctx *ctx);
void stcp_rust_rx_stop(struct stcp_ctx *ctx);
#endif
#endif
