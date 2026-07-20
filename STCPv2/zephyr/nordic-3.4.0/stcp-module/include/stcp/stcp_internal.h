#ifndef STCP2_INTERNAL_H
#define STCP2_INTERNAL_H
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp.h>
enum stcp_state { STCP_FREE=0, STCP_CREATED, STCP_BOUND, STCP_LISTENING, STCP_CONNECTING, STCP_CONNECTED, STCP_CLOSED };
struct stcp_ctx {
    bool used;
    int fd;
    int protocol;
    int carrier_fd;
    int last_error;
    enum stcp_state state;
    struct sockaddr_in local;
    struct sockaddr_in peer;
    struct k_mutex lock;
};
struct stcp_ctx *stcp_ctx_alloc(void);
void stcp_ctx_free(struct stcp_ctx *ctx);
int stcp_carrier_open(int protocol);
int stcp_carrier_wait_connected(int fd, int timeout_ms);
#endif
