#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>

#include <errno.h>

#include "stcp.h"
#include "stcp_struct.h"
#include "stcp_net.h"
#include "stcp_operations_zephyr.h"

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

/*

 */
int stcp_close(void *obj);
int stcp_context_connect(struct stcp_ctx *ctx,
                         struct sockaddr *addr,
                         socklen_t addrlen,
                         int timeout_ms);

int stcp_net_close_fd(int *fd);

bool stcp_is_supported(int family, int type, int proto);
int stcp_socket(int family, int type, int proto);
