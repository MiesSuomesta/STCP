#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>
#include <stdint.h>
#include <errno.h>

#include "stcp/stcp_api_internal.h"
#include "stcp/stcp_struct.h"

intptr_t stcp_tcp_send_iovec(void *sock, void *msg_vp, int flags);

intptr_t stcp_tcp_send(void *sock_vp, const uint8_t *buf, uintptr_t len);

intptr_t stcp_tcp_recv(void *sock_vp,
                       uint8_t *buf,
                       uintptr_t len,
                       int32_t non_blocking,
                       uint32_t flags,
                       int *recv_len);