#ifndef STCP_ZEPHYR_INTERNAL_H
#define STCP_ZEPHYR_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp.h>

enum stcp_state {
    STCP_STATE_FREE = 0,
    STCP_STATE_CREATED,
    STCP_STATE_BOUND,
    STCP_STATE_LISTENING,
    STCP_STATE_CONNECTING,
    STCP_STATE_CONNECTED,
    STCP_STATE_CLOSED,
};

enum stcp_carrier_kind {
    STCP_CARRIER_TCP = 1,
    STCP_CARRIER_UDP = 2,
};

struct stcp_carrier {
    enum stcp_carrier_kind kind;
    int fd;
    bool bound;
    bool listening;
    bool connected;
};

struct stcp_socket_ctx {
    bool allocated;
    int fd;
    int type;
    int protocol;
    int last_error;
    enum stcp_state state;
    struct sockaddr_in local;
    struct sockaddr_in peer;
    struct k_mutex lock;
    struct stcp_carrier carrier;
};

struct stcp_socket_ctx *stcp_ctx_alloc(void);
void stcp_ctx_release(struct stcp_socket_ctx *ctx);

int stcp_carrier_init(struct stcp_carrier *carrier, int protocol);
void stcp_carrier_close(struct stcp_carrier *carrier);
int stcp_carrier_bind(struct stcp_carrier *carrier, const struct sockaddr_in *addr);
int stcp_carrier_connect(struct stcp_carrier *carrier, const struct sockaddr_in *addr);
int stcp_carrier_listen(struct stcp_carrier *carrier, int backlog);
int stcp_carrier_accept(struct stcp_carrier *listener, struct stcp_carrier *child,
                        struct sockaddr_in *peer, socklen_t *peer_len);
ssize_t stcp_carrier_send(struct stcp_carrier *carrier, const void *buf,
                          size_t len, int flags);
ssize_t stcp_carrier_recv(struct stcp_carrier *carrier, void *buf,
                          size_t len, int flags);
int stcp_carrier_shutdown(struct stcp_carrier *carrier, int how);
int stcp_carrier_getsockname(struct stcp_carrier *carrier,
                             struct sockaddr_in *addr, socklen_t *len);
int stcp_carrier_getpeername(struct stcp_carrier *carrier,
                             struct sockaddr_in *addr, socklen_t *len);
int stcp_carrier_getsockopt(struct stcp_carrier *carrier, int level, int optname,
                            void *optval, socklen_t *optlen);
int stcp_carrier_setsockopt(struct stcp_carrier *carrier, int level, int optname,
                            const void *optval, socklen_t optlen);

int stcp_core_bind(struct stcp_socket_ctx *ctx, const struct sockaddr_in *addr);
int stcp_core_connect(struct stcp_socket_ctx *ctx, const struct sockaddr_in *addr);
int stcp_core_listen(struct stcp_socket_ctx *ctx, int backlog);
int stcp_core_accept(struct stcp_socket_ctx *listener, struct stcp_socket_ctx *child,
                     struct sockaddr_in *peer, socklen_t *peer_len);
ssize_t stcp_core_send(struct stcp_socket_ctx *ctx, const void *buf,
                       size_t len, int flags);
ssize_t stcp_core_recv(struct stcp_socket_ctx *ctx, void *buf,
                       size_t len, int flags);
int stcp_core_shutdown(struct stcp_socket_ctx *ctx, int how);

#endif
