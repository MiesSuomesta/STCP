#include <zephyr/logging/log.h>
#include "debug.h"
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/heap_listener.h>
#include <zephyr/kernel.h>
#include <stddef.h>
#include <stdint.h>
#include <zephyr/sys/heap_listener.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/kernel.h>

#include "stcp_alloc.h"
#include "stcp_struct.h"
#include "stcp_bridge.h"
#include "stcp_net.h"
#include "utils.h"
#include "stcp_operations_zephyr.h"

#include "stcp_rust_exported_functions.h"

LOG_MODULE_REGISTER(stcp_brige_operations, LOG_LEVEL_INF);

#define STCP_DEBUG_DUMP_ENABLED             1
#define STCP_DEBUG_DUMP_MAX_DUMP_LENGTH     512

#include <zephyr/sys/heap_listener.h>

static void on_heap_alloc(uintptr_t heap_id, void *mem, size_t bytes)
{
    LOG_ERR("HEAP ALLOC: heap=%lu ptr=%p size=%u", heap_id, mem, (unsigned)bytes);
}

static void on_heap_free(uintptr_t heap_id, void *mem, size_t bytes)
{
    LOG_ERR("HEAP FREE: heap=%lu ptr=%p size=%u", heap_id, mem, (unsigned)bytes);
}

static void on_heap_resize(uintptr_t heap_id, void *mem, size_t bytes)
{
    LOG_ERR("HEAP RESIZE: heap=%lu ptr=%p size=%u", heap_id, mem, (unsigned)bytes);
}

HEAP_LISTENER_ALLOC_DEFINE(stcp_alloc_listener, HEAP_ID_LIBC, on_heap_alloc);
HEAP_LISTENER_FREE_DEFINE(stcp_free_listener, HEAP_ID_LIBC, on_heap_free);
HEAP_LISTENER_RESIZE_DEFINE(stcp_resize_listener, HEAP_ID_LIBC, on_heap_resize);

void heap_debug_init(void)
{
    heap_listener_register(&stcp_alloc_listener);
    heap_listener_register(&stcp_free_listener);
    heap_listener_register(&stcp_resize_listener);
}


int stcp_exported_rust_ctx_alive_count(void) 
{
    // Not enabled
    return -1;
}

void stcp_hexdump_ascii(const char *prefix, const uint8_t *buf, size_t len)
{

    if (!stcp_config_debug_enabled()) {
        return ;
    }

#if STCP_DEBUG_DUMP_ENABLED
    const size_t cols = 16;

    if (len > STCP_DEBUG_DUMP_MAX_DUMP_LENGTH) {
        len = STCP_DEBUG_DUMP_MAX_DUMP_LENGTH;
    }

    if (prefix) {
        printk("%s (len=%u):\n", prefix, (unsigned)len);
    }

    for (size_t i = 0; i < len; i += cols) {
        printk("%04x  ", (unsigned)i);
        /* Hex */
        for (size_t j = 0; j < cols; j++) {
            if (i + j < len) {
                printk("%02x ", buf[i + j]);
            } else {
                printk("   ");
            }
        }

        printk(" |");

        /* ASCII */
        for (size_t j = 0; j < cols && i + j < len; j++) {
            uint8_t c = buf[i + j];
            if (c >= 32 && c <= 126) {
                printk("%c", c);
            } else {
                printk(".");
            }
        }

        printk("|\n");
    }
#endif
}

int stcp_is_context_valid(void *vpCtx) {
    struct stcp_ctx * ctx = vpCtx;
    
    if (!ctx) {
        LDBG("STCP Context null");
        return -EINVAL;
    } else {
        LDBG("STCP Context check: ctx ptr ok? %p", ctx);
    }

    if (!ctx->session) {
        LDBG("STCP RUST Session null!\n");
        return -EINVAL;
    } else {
        LDBG("STCP Context check: session ptr ok....");
    }

    if (!ctx->handshake_done) {
        LDBG("Not handshake done, not checking the FD's...");
        return 1;
    }
    
    // Checking FD's
    if (ctx->ks.fd > 0) {
        LDBG("STCP Context check: Transport FD....");
        int rc2 = stcp_is_file_desc_alive(ctx->ks.fd);
        if (rc2 < 0) {
            ctx->ks.fd = -1;
            LDBG("STCP RUST FD for Transport not ok!, RC: %d\n", rc2);
            return -EINVAL;
        }
    } else {
        LDBG("FD for transport is set -1 => NOT checking");
    }

    return 1;
}

int stcp_is_file_desc_alive(int fd) {
    uint8_t tmp;
    int ret = zsock_recv(fd, &tmp, 1, ZSOCK_MSG_PEEK | ZSOCK_MSG_DONTWAIT);

    if (ret == 0) {
        return 0;
    } else if (ret < 0 && errno == EAGAIN) {
        return 1;
    }
    return -1;
}

int stcp_get_pending_fd_error(int fd)
{
    
    if (fd < 0 ) {
        LDBG("STCP Context is destroyed!\n");
        return -EBADFD;
    }

    int err = 0;
    socklen_t len = sizeof(err);

    zsock_getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);


    return err;
}


void stcp_sleep_ms(uint32_t ms)
{
    k_msleep(ms);
}


int stcp_tcp_timeout_set_to_fd(int fd, int timeout_ms) {
    LDBG("Setting timeout of %d seconds for reading from fd: %d",
        timeout_ms / 1000, fd);
    int rc = zsock_setsockopt(fd,
                            SOL_SOCKET,
                            SO_RCVTIMEO,
                            &timeout_ms,
                            sizeof(timeout_ms));
    return rc;
}

