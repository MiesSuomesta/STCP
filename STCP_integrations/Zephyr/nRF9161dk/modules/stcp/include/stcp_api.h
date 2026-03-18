#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <zephyr/net/socket.h>
#include <stdbool.h>
#include <errno.h>

#include "stcp_generated.h"

#ifdef __cplusplus
extern "C" {
#endif

struct stcp_api;

// Main entrypoint...
int stcp_library_init(void);

int stcp_api_get_handshake_status(struct stcp_api *api);

int stcp_api_wait_until_reached_stcp_init_ready(int timeout_sec);
int stcp_api_resolve(const char *host, const char *port, struct zsock_addrinfo **result);
    
// Semaforien odottelut per instanssi
int stcp_api_wait_until_reached_ip_network_up(struct stcp_api *api, int timeout);
int stcp_api_wait_until_reached_lte_ready    (struct stcp_api *api, int timeout);
int stcp_api_wait_until_reached_pdn_ready    (struct stcp_api *api, int timeout);
int stcp_api_wait_until_reached_connect_ready(struct stcp_api *api, int timeout);

/* Lifecycle */
int     stcp_api_init(struct stcp_api **api);
int     stcp_api_init_with_fd(struct stcp_api **api, int fd);

int     stcp_api_close(struct stcp_api *api);

/* Server */
int     stcp_api_bind(struct stcp_api *api,
                      const struct zsock_addrinfo *addr,
                      socklen_t addrlen);

int     stcp_api_listen(struct stcp_api *api,
                        int backlog);

int     stcp_api_accept(struct stcp_api *api,
                        struct stcp_api **new_api,
                        struct zsock_addrinfo *peer,
                        socklen_t *peer_len);

/* Client */
int     stcp_api_connect(struct stcp_api *api,
                         const struct zsock_addrinfo *addr,
                         socklen_t addrlen);


int stcp_api_connection_reset(struct stcp_api *api);


/* IO */
int stcp_api_set_io_timeout(struct stcp_api *api, int timeout_ms);

ssize_t stcp_api_send(struct stcp_api *api,
                       const void *buf,
                       size_t len,
                       int flags);

ssize_t stcp_api_recv(struct stcp_api *api,
                      void *buf,
                      size_t len,
                      int flags);

ssize_t stcp_api_sendmsg(struct stcp_api *api,
                         const struct msghdr *msg);

/* Nonblocking */
int     stcp_api_set_nonblocking(struct stcp_api *api,
                                 bool enable);

/* Poll */
int stcp_api_poll(struct stcp_api *api,
                  int events,
                  int timeout_ms,
                  int *revents);

/* Optional FD access */
int     stcp_api_get_fd(struct stcp_api *api);

/* State */
int     stcp_api_state(struct stcp_api *api);


#ifdef __cplusplus
}
#endif
