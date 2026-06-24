#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <zephyr/init.h>
#include <zephyr/sys/heap_listener.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/sys/atomic.h>

#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>


#include <stcp_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_net.h>
#include <stcp/workers.h>
#include <stcp/utils.h>
#include <stcp/stcp_mqtt.h>
#include <stcp/utils.h>
#include <stcp/stcp_operations_zephyr.h>
#include <stcp/low_level_socks.h>

static int g_stcp_open_socks = 0;
static struct stcp_socket_item fd_list[STCP_SOCKET_FD_LIST_SIZE] = { 0 };

struct stcp_socket_item * stcp_socket_list_get_slot_ptr(int slot) {
    if (slot >= STCP_SOCKET_FD_LIST_SIZE) return NULL;
    if (slot < 0) return NULL;
    return &(fd_list[slot]);
}

int stcp_socket_list_get_free_slot() {
    int i = 0;
    struct stcp_socket_item *pItem = fd_list;

    while (i < STCP_SOCKET_FD_LIST_SIZE) {
        pItem = stcp_socket_list_get_slot_ptr(i);
        if (pItem == NULL) {
            return -EINVAL;
        }

        if (!pItem->alive) {
            return i;
        }
        i++;
    }

    return -1;
}


int stcp_socket_list_get_slot_with_fd(int fd) {
    int i = 0;
    struct stcp_socket_item *pItem = fd_list;
    LDBG("Searching for %d ...", fd);
    while (i < STCP_SOCKET_FD_LIST_SIZE) {
        pItem = stcp_socket_list_get_slot_ptr(i);
        if (pItem == NULL) {
            return -EINVAL;
        }

        if (pItem->fd == fd) {
            LDBG("Got slot %d for %d ...", i, fd);
            return i;
        }
        i++;
    }

    return -1;
}



int stcp_socket_list_use_slot(int slot, int fd, void *lr) {
    struct stcp_socket_item *pItem = NULL;

    pItem = stcp_socket_list_get_slot_ptr(slot);
    if (pItem == NULL) {
        return -EINVAL;
    }

    pItem->alive    = 1;
    pItem->fd       = fd;
    pItem->lr       = lr;
    LDBG("SLOT[%d] set alive { fd:%d, lr %p }", slot, fd, lr);
    return 1;
}

int stcp_socket_list_free_slot(int slot) {
    struct stcp_socket_item *pItem = NULL;

    pItem = stcp_socket_list_get_slot_ptr(slot);
    if (pItem == NULL) {
        return -EINVAL;
    }

    pItem->alive    = 0;
    pItem->fd       = -1;
    pItem->lr       = NULL;
    LDBG("SLOT[%d] Freed", slot);
    return 1;
}

void stcp_socket_dump_status() {
    int i = 0;
    struct stcp_socket_item *pItem = fd_list;

    while (i < STCP_SOCKET_FD_LIST_SIZE) {
        pItem = stcp_socket_list_get_slot_ptr(i);
        if (pItem == NULL) {
            return;
        }

        LINF("Socket %d (alive: %d) => LR: %p",
            pItem->fd,
            pItem->alive,
            pItem->lr
        );
        i++;
    }
}

int stcp_open_sock(int family, int type, int proto) {
        
    void *lr = __builtin_return_address(0);
    int idx = stcp_socket_list_get_free_slot();
    if (idx >= STCP_SOCKET_FD_LIST_SIZE) {
        stcp_socket_dump_status();
        return -ENOBUFS;
    }

    if (idx < 0) {
        stcp_socket_dump_status();
        return -ENOBUFS;
    }

    int fd = zsock_socket(family, type, proto);
    if (fd >= 0) {
        g_stcp_open_socks++;
        stcp_socket_list_use_slot(idx, fd, lr);
    }

    stcp_socket_dump_status();
    return fd;
}

int stcp_close_sock(int fd) {
    if (fd >= 0) {
        zsock_close(fd);

        int idx = stcp_socket_list_get_slot_with_fd(fd);
        
        if (idx < 0) {
            LERR("FD NOT RECORDED");
            stcp_socket_dump_status();
            stcp_dump_bt();
        } else {
            stcp_socket_list_free_slot(idx);
        }

        g_stcp_open_socks--;
        if (g_stcp_open_socks < 0) {
            LERR("Unbalanced socket operations!");
            stcp_socket_dump_status();
            stcp_dump_bt();
        }
    }

    return g_stcp_open_socks;
}
