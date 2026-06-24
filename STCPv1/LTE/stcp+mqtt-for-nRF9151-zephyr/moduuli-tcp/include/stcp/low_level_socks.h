#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <stcp/stcp_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_net.h>
#include <stcp/workers.h>
#include <stcp/utils.h>
#include <stcp/stcp_operations_zephyr.h>

#include <stcp/low_level_operations.h>

#if STCP_SOCKET_TRACKING 
struct stcp_socket_item {
    int fd;
    int alive;
    void *lr;
};

struct stcp_socket_item * stcp_socket_list_get_slot_ptr(int slot);
int stcp_socket_list_get_free_slot();
int stcp_socket_list_get_slot_with_fd(int fd);
int stcp_socket_list_use_slot(int slot, int fd, void *lr);
int stcp_socket_list_free_slot(int slot);
void stcp_socket_dump_status();
int stcp_open_sock(int family, int type, int proto);
int stcp_close_sock(int fd);

#define STCP_SOCKET_OPEN    stcp_open_sock
#define STCP_SOCKET_CLOSE   stcp_close_sock

#else
#define STCP_SOCKET_OPEN    zsock_socket
#define STCP_SOCKET_CLOSE   zsock_close
#endif

