#ifndef STCP_V2_INTERNAL_H
#define STCP_V2_INTERNAL_H

#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/atomic.h>
#include <stcp/stcp.h>

struct stcp_v2_carrier {
    int fd;
    int socket_type;
    bool owns_fd;
    bool peer_valid;
    struct sockaddr_in peer;
    struct k_mutex tx_lock;
};

struct stcp_v2_socket {
    bool used;
    int fd;
    int socket_type;
    int protocol;
    void *rust_ctx;
    struct stcp_v2_carrier *carrier;
    struct sockaddr_in local;
    struct sockaddr_in peer;
    struct k_sem event;
    struct k_thread rx_thread;
    K_KERNEL_STACK_MEMBER(rx_stack, CONFIG_STCP_V2_RX_STACK_SIZE);

    /*
     * Native carrier RX scratch storage must NOT live on rx_thread stack.
     * The Rust receive path has a non-trivial call chain, so consuming a
     * multi-kilobyte automatic array here leaves too little stack headroom.
     */
    uint8_t *rx_buffer;
    size_t rx_buffer_size;

    atomic_t rx_running;
    atomic_t rx_stop;
};

struct stcp_v2_socket *stcp_v2_socket_alloc(void);
void stcp_v2_socket_free(struct stcp_v2_socket *sock);

struct stcp_v2_carrier *stcp_v2_carrier_open(int socket_type);
struct stcp_v2_carrier *stcp_v2_carrier_udp_child(struct stcp_v2_carrier *listener,
                                                   uint32_t peer_addr,
                                                   uint16_t peer_port);
void stcp_v2_carrier_free(struct stcp_v2_carrier *carrier);
ssize_t stcp_v2_carrier_send_wire(struct stcp_v2_carrier *carrier,
                                  const uint8_t *data, size_t len, int flags);

int stcp_v2_rx_start(struct stcp_v2_socket *sock);
void stcp_v2_rx_stop(struct stcp_v2_socket *sock);
void stcp_v2_signal(struct stcp_v2_socket *sock);

#endif
