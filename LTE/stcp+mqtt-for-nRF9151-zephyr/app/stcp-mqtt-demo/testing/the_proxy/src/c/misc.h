#pragma once
#include <poll.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

struct stcp_api;

// Pitää olla sellainen ettei kilahda MSG_ lippujen päälle
#define STCP_RECV_FLAG_EXACT_MODE       (1 << 20)
#define STCP_RECV_FLAG_NON_BLOCKING     (1 << 21)

int stcp_transport_wait_until_ready(int seconds);
int stcp_api_is_open_for_io(
    struct stcp_api *api
);
int64_t stcp_uptime_ms(void);

intptr_t stcp_tcp_recv(
    void *sock_vp,
    uint8_t *buf,
    uintptr_t len,
    uint32_t non_bloking,
    uint32_t flags,
    uintptr_t* recv_len
);

intptr_t stcp_tcp_send(
    void *sock_vp,
    const uint8_t *buf,
    uintptr_t len
);

