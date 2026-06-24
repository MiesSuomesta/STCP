#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>

#include <errno.h>

#include <stcp/debug.h>
#include <stcp/stcp_struct.h>
#include <stcp/fsm.h>
#include <stcp/utils.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/stcp_rust_exported_functions.h>
#include <stcp/stcp_alloc.h>
#include <stcp/dns.h>
#include <stcp/low_level_pointer.h>

#include <stcp/low_level_refcount_tracker.h>

#include <stcp/lifespan.h>

#include <stcp/sanity.h>

int is_prt_at_text_addr(uintptr_t addr)
{
    return (
        addr >= CONFIG_FLASH_BASE_ADDRESS &&
        addr < (CONFIG_FLASH_BASE_ADDRESS + CONFIG_FLASH_SIZE * 1024)
    );
}

int is_stack_ptr_valid(uintptr_t sp)
{
    k_tid_t tid = k_current_get();

    if (!tid) {
        return 0;
    }

    uintptr_t start = (uintptr_t)tid->stack_info.start;
    uintptr_t end   = start + tid->stack_info.size;

    if (sp < start || sp >= end) {
        return 0;
    }

    if (sp & (sizeof(uintptr_t)-1)) {
        return 0;
    }

    return 1;
}

int stcp_api_sanity_check(struct stcp_api *api, enum stcp_sanity_phase phase)
{
    if (!api) {
        LERR("Sanity: API NULL");
        return -EINVAL;
    }

    struct stcp_ctx *ctx = api->ctx;

    if (!ctx) {
        LERR("Sanity: CTX NULL");
        return -EINVAL;
    }

    if ((uintptr_t)ctx < 0x20000000) {
        LERR("Sanity: CTX pointer invalid (%p)", ctx);
        return -EFAULT;
    }

    if ( ctx->magic == STCP_CTX_MAGIC_POISON ) {
        LERR("Sanity: Context header magic is poisoned already");
        return -EFAULT;
    }

    if ( ctx->magic_footer == STCP_CTX_MAGIC_POISON ) {
        LERR("Sanity: Context footer magic is poisoned already");
        return -EFAULT;
    }

    if (ctx->magic != STCP_CTX_MAGIC_ALIVE) {
        LERR("Sanity: Context %p invalid magic at header", ctx);
        return -EFAULT;
    }

    if (ctx->magic_footer != STCP_CTX_MAGIC_ALIVE_FOOTER) {
        LERR("Sanity: Context %p invalid magic at footer", ctx);
        return -EFAULT;
    }

    if (ctx->destroyed) {
        LERR("Sanity: CTX destroyed (%p)", ctx);
        return -EPIPE;
    }

    // 🔹 API link
    if (phase >= STCP_SANITY_API_LINKED) {
        if (ctx->api != api) {
            LERR("Sanity: CTX => API mismatch (%p => %p)", api, ctx);
            return -EBADR;
        }

        if (api->ctx != ctx) {
            LERR("Sanity: API => CTX mismatch (%p => %p)", ctx, api);
            return -EBADR;
        }
    }

    // 🔹 READY vaihe
    if (phase >= STCP_SANITY_READY) {
        LDBG("Sanity: ctx not READY yet (state=%d)", ctx->state);
        int valid = atomic_get(&api->alive);
        LDBG("Alive? %d", valid);
        if (!valid) {
            LDBG("Sanity: API %p is not valid", api);
            return -EFAULT;
        }
    }

    // 🔹 CONNECTED vaihe
    if (phase >= STCP_SANITY_CONNECTED) {
        if (ctx->ks.fd < 0) {
            LERR("Sanity: FD invalid (%d)", ctx->ks.fd);
            return -EBADFD;
        }

        int valid =  stcp_api_is_valid(api);
        if (!valid) {
            LDBG("Sanity: API %p is not valid", api);
            return -EPROTO;
        }
    }

    if (phase >= STCP_SANITY_USABLE) {
        int usable =  stcp_api_is_usable(api);
        if (!usable) {
            LDBG("Sanity: API %p not usable (state=%d fd=%d closing=%d)",
                api,
                api->ctx->state,
                api->ctx->ks.fd,
                api->ctx->closing);
            return -EACCES;
        }
    }

    return 0;
}