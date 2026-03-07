#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>
#include <stdint.h>
#include <errno.h>

#include "stcp/stcp_struct.h"

ssize_t stcp_tcp_sendmsg(int fd,
                         struct zsock_msghdr *msg,
                         int flags);

intptr_t stcp_tcp_send(struct kernel_socket *sock, const uint8_t *buf, uintptr_t len);

intptr_t stcp_tcp_recv(struct kernel_socket *sock,
                       uint8_t *buf,
                       uintptr_t len,
                       int32_t non_blocking,
                       uint32_t flags,
                       int *recv_len);

