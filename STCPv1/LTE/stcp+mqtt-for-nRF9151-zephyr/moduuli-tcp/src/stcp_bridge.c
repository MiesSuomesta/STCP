#include <zephyr/logging/log.h>
#include "stcp/debug.h"
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/heap_listener.h>
#include <zephyr/kernel.h>
#include <stddef.h>
#include <stdint.h>
#include <zephyr/sys/heap_listener.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/kernel.h>

#include "stcp/settings.h"
#include "stcp/stcp_alloc.h"
#include "stcp/utils.h"
#include "stcp/stcp_net.h"
#include "stcp/utils.h"
#include "stcp/stcp_operations_zephyr.h"

#include <stcp/workers.h>
#include <stcp/debug.h>

#include <stcp/sanity.h>

#include "stcp/stcp_rust_exported_functions.h"


#define STCP_DEBUG_DUMP_ENABLED             1
#define STCP_DEBUG_DUMP_MAX_DUMP_LENGTH     512

int stcp_is_file_desc_alive(int fd);

int stcp_exported_rust_ctx_alive_count(void) 
{
    // Not enabled
    return -errno;
}

void stcp_hexdump_ascii(const char *prefix, const uint8_t *buf, int len)
{
#if CONFIG_STCP_DEBUG
#if STCP_DEBUG_DUMP_ENABLED
    if (!stcp_config_debug_enabled()) {
        return ;
    }

    if (len < 0) {
        LERR("[%s] Error, neg len.", prefix);
        return;
    }

    const size_t cols = 16;

    if (len > STCP_DEBUG_DUMP_MAX_DUMP_LENGTH) {
        LWRN("[%s] Length way big, %d bytes.", prefix, len);
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
# endif // Dumps enabled
#endif // CONFIG_STCP_DEBUG
}

inline int stcp_api_pointer_valid(struct stcp_api *api)
{
    uintptr_t p = (uintptr_t)api;

    if (p < 0x20000000 || p > 0x30000000) {
        LDBGBIG("STCP: Pointer not in valid range: %p", api);
        stcp_dump_bt();
        return -EBADMSG;
    }

    if (api->magic != STCP_CTX_MAGIC_ALIVE) {
        LDBGBIG("STCP: Magic (header), mismatch %08x != %08x", api->magic, STCP_CTX_MAGIC_ALIVE);
        stcp_dump_bt();
        return -EBADMSG;
    }

    if (api->magic_footer != STCP_CTX_MAGIC_ALIVE_FOOTER) {
        LDBGBIG("STCP: Magic (footer), mismatch %08x != %08x", api->magic_footer, STCP_CTX_MAGIC_ALIVE_FOOTER);
        stcp_dump_bt();
        return -EBADMSG;
    }

    if (api->magic != STCP_CTX_MAGIC_ALIVE) {
        LWRN("CTX %p invalid magic at header", api);
        stcp_dump_bt();
        return -EBADMSG;
    }

    return 1;
}

inline int stcp_ctx_pointer_valid(struct stcp_ctx *ctx)
{
    uintptr_t p = (uintptr_t)ctx;

    if (((uintptr_t)ctx) & 0x3) {
        LDBGBIG("CTX not aligned! %p", ctx);
        stcp_dump_bt();
        return -EBADMSG;
    }

    if (p < 0x20000000 || p > 0x30000000) {
        LDBGBIG("STCP: Pointer not in valid range: %p", ctx);
        stcp_dump_bt();
        return -EBADMSG;
    }

    if (ctx->magic != STCP_CTX_MAGIC_ALIVE) {
        LDBGBIG("STCP: Magic (header), mismatch %08x != %08x", ctx->magic, STCP_CTX_MAGIC_ALIVE);
        stcp_dump_bt();
        return -EBADMSG;
    }

    if (ctx->magic_footer != STCP_CTX_MAGIC_ALIVE_FOOTER) {
        LDBGBIG("STCP: Magic (footer), mismatch %08x != %08x", ctx->magic_footer, STCP_CTX_MAGIC_ALIVE_FOOTER);
        stcp_dump_bt();
        return -EBADMSG;
    }

    if (ctx->magic != STCP_CTX_MAGIC_ALIVE) {
        LWRN("CTX %p invalid magic at header", ctx);
        stcp_dump_bt();
        return -EBADMSG;
    }

    return 1;
}

// Mikään ei saa ottaa refereenssiä täällä!
int stcp_is_context_valid_no_ref(void *vpCtx) {
    struct stcp_ctx * ctx = vpCtx;
    int ret = 1;

    if (((uintptr_t)ctx) & 0x3) {
        LDBGBIG("CTX not aligned! %p", ctx);
        stcp_dump_bt();
        return -EBADMSG;
    }

    if (!ctx) {
        LERR("STCP Context null");
        stcp_dump_bt();
        ret = -EINVAL;
        goto out;
    } 

    if (!stcp_ctx_pointer_valid(ctx)) {
        LERR("STCP Context pointer not valid!");
        stcp_dump_bt();
        ret = -EINVAL;
        goto out;
    }

    if (atomic_get(&ctx->closing)) {
        LDBG("STCP Context closing..");
        ret = -EINVAL;
        goto out;
    }

    // Checking FD's
#if 0
    if (ctx->ks.fd >= 0) {
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
#endif
out:
    return ret;
}

int stcp_is_context_valid(void *vpCtx) {
    struct stcp_ctx * ctx = vpCtx;
    int ret = 0;

    STCP_REF_COUNT_GET(ctx, "@ is context valid", return -EACCES; );
        ret = stcp_is_context_valid_no_ref(vpCtx);
        if (ret == 0) {

            if (!ctx->session) {
                LDBG("STCP RUST Session null!\n");
                ret = -EINVAL;
                goto out;
            }

            if ( worker_is_context_scheduled_for_cleanup(ctx)) {
                LINF("Trying to use after closing set....");
                ret = -ENOTRECOVERABLE;
                goto out;
            }
        }

out:
    STCP_REF_COUNT_PUT(ctx, "@ end of is context valid");
    return ret;
}

int stcp_is_file_descq_alive(int fd) {
    uint8_t tmp;
    int ret = zsock_recv(fd, &tmp, 1, ZSOCK_MSG_PEEK | ZSOCK_MSG_DONTWAIT);

    if (ret == 0) {
        return 0;
    } else if (ret < 0 && errno == EAGAIN) {
        return 1;
    }
    return -errno;
}

void stcp_sleep_ms(uint32_t ms)
{
    k_msleep(ms);
}

int stcp_tcp_timeout_set_to_fd(int fd, int timeout_ms)
{
    int rc;

    LDBG("Setting timeout of %d ms for fd: %d", timeout_ms, fd);

    struct timeval timeout = {
        .tv_sec  = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000
    };

    rc = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    if (rc < 0) {
        LERR("Failed to set SO_SNDTIMEO: %d", errno);
        return -errno;
    }

    rc = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (rc < 0) {
        LERR("Failed to set SO_RCVTIMEO: %d", errno);
        return -errno;
    }

    return 0;
}
