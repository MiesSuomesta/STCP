#ifndef STCP_ZEPHYR_INTERNAL_H_
#define STCP_ZEPHYR_INTERNAL_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/ring_buffer.h>
#include <stcp/stcp.h>

enum stcp_socket_state {
    STCP_STATE_FREE = 0,
    STCP_STATE_CREATED,
    STCP_STATE_BOUND,
    STCP_STATE_LISTENING,
    STCP_STATE_CONNECTING,
    STCP_STATE_CONNECTED,
    STCP_STATE_CLOSED,
};

struct stcp_socket_ctx {
    void *fifo_reserved;
    bool allocated;
    bool nonblocking;
    int fd;
    int type;
    int protocol;
    int last_error;
    int backlog;
    enum stcp_socket_state state;
    struct sockaddr_stcp local;
    struct sockaddr_stcp peer;
    struct k_mutex lock;
    struct k_sem rx_ready;
    struct k_sem connect_ready;
    struct k_fifo accept_queue;
    struct ring_buf rx_ring;
    uint8_t rx_storage[CONFIG_STCP_RX_BUFFER_SIZE];
};

struct stcp_socket_ctx *stcp_ctx_alloc(void);
void stcp_ctx_release(struct stcp_socket_ctx *ctx);

int stcp_core_bind(struct stcp_socket_ctx *ctx, const struct sockaddr_stcp *addr);
int stcp_core_listen(struct stcp_socket_ctx *ctx, int backlog);
int stcp_core_connect(struct stcp_socket_ctx *ctx, const struct sockaddr_stcp *addr);
ssize_t stcp_core_send(struct stcp_socket_ctx *ctx, const void *buf, size_t len, int flags);
ssize_t stcp_core_recv(struct stcp_socket_ctx *ctx, void *buf, size_t len, int flags);
int stcp_core_shutdown(struct stcp_socket_ctx *ctx, int how);

#endif
