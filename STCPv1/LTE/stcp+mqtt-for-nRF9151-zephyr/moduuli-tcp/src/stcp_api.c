#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <stdbool.h>
#include <errno.h>

#include <stcp/stcp_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_api_internal.h>

extern atomic_t g_modem_status_online;

int stcp_api_check_magic(struct stcp_api *api) {
    int rv = -EBADMSG;

    if (api) {

        rv = 0;
        uintptr_t p = (uintptr_t)api;

        if (p < 0x20000000 || p > 0x30000000) {
            LDBGBIG("STCP: API Pointer not in valid range: %p", api);
            rv = -ENODEV;
        }

        if (api->magic != STCP_CTX_MAGIC_ALIVE) {
            LERRBIG(
                "API DEAD/CORRUPTED %p magic=0x%x",
                api,
                api->magic
            );
            rv = -ENOMEDIUM;
        } else if (api->magic_footer != STCP_CTX_MAGIC_ALIVE_FOOTER) {
            LERRBIG(
                "API DEAD/CORRUPTED %p footer magic=0x%x",
                api,
                api->magic_footer
            );
            rv = -ENOMEDIUM;
        }
    }

    return rv;
}

void* stcp_api_context_get_lock(struct stcp_api *api) {

    if (stcp_api_check_magic(api) < 0) {
        LERR("GOT NO VALID API!");
#if STCP_STCP_FSM_TRACKING
        LDBGBIG("Dumping FSM status....");
        stcp_trace_fsm_dump_status();
#endif
        stcp_dump_bt();
        k_panic();
    } else {
        return &(VOID_TO_API(api)->lock);
    }
    return NULL;
};

int stcp_api_get_errno() {
    return errno;
}
__attribute__((used))
void *stcp_api_get_errno_keep = (void *)&stcp_api_get_errno;

void stcp_api_request_reset() {
    LINFBIG("[API] Requested reset..");
    stcp_set_reset_requested();
}

int stcp_api_is_open_for_io(struct stcp_api *api)
{
    if (!api) {
        return -EINVAL;
    }

    return stcp_is_context_open_for_fd_io(api->ctx);
}

int stcp_api_get_handshake_status(struct stcp_api *api)
{
    if (!api || !api->ctx) {
        LERR("No api..");
        return -EINVAL;
    }
    API_LOCK(api, "HS Status");
        LDBG("[API %p] Handshake done: %d", api, api->ctx->handshake_done);
        int val = api->ctx->handshake_done;
    API_UNLOCK(api, "HS Status");
    return val;
}

int stcp_api_wait_until_stcp_handshake_is_done(struct stcp_api *api, int timeout_ms) {

    LDBG("[API %p] Waiting for hanshake...", api);
    int rv = stcp_wait_for_hanshake_done(api->ctx, timeout_ms);
    LDBG("[API %p] Handshake waited, ret: %d...", api, rv);
    return rv;

}

int stcp_api_wait_until_reached_ip_network_up(struct stcp_api *api, int timeout) {
    API_LOCK(api, __func__);
        int rv = stcp_fsm_wait_until_reached_ip_network_up(NULL, timeout);
    API_UNLOCK(api, __func__);
    return rv;
}

int stcp_api_wait_until_reached_lte_ready(struct stcp_api *api, int timeout) {
    API_LOCK(api, __func__);
        int rv = stcp_fsm_wait_until_reached_lte_ready(NULL, timeout);
    API_UNLOCK(api, __func__);
    return rv;
}

int stcp_api_wait_until_reached_pdn_ready(struct stcp_api *api, int timeout) {
    API_LOCK(api, __func__);
        int rv = stcp_fsm_wait_until_reached_pdn_ready(NULL, timeout);
    API_UNLOCK(api, __func__);
    return rv;
}

int stcp_api_wait_for_radio_connected(int seconds) {
    return stcp_transport_wait_for_radio_connected(seconds);
}

int stcp_api_get_modem_state(int *lte, int *pdn, int *ip, int *radio, int *connection_ok) {
    return stcp_transport_get_modem_states(lte, pdn, ip, radio, connection_ok);
}
/*
int stcp_api_get_modem_online_status() {
    int rv = atomic_get(&g_modem_status_online);
    return rv;
}
*/

int stcp_api_acquire(struct stcp_api *api)
{
    if (!api || !stcp_api_is_alive(api))
        return -EINVAL;

    STCP_REF_COUNT_GET(api->ctx, "api acquire", return -EACCES;);

    if (!stcp_api_is_alive(api)) {
        STCP_REF_COUNT_PUT(api->ctx, "api acquire fail");
        return -EPIPE;
    }

    return 0;
}

void stcp_api_release(struct stcp_api *api)
{
    if (!api) return;
    STCP_REF_COUNT_PUT(api->ctx, "api release");
}



int stcp_api_is_open_for_fd_io(struct stcp_api *api)
{
    if (!api) return;
    int open = stcp_is_context_open_for_fd_io(api->ctx);
    return open;
}

int stcp_api_is_alive(struct stcp_api *api) {

    LDBG("Called with API: %p", api);
    stcp_dump_bt();

    if (!api) {
        return -EINVAL;
    }

    if ((uintptr_t)api < 0x20000000) {
        LWRN("API POINTER INVALID %p", api);
        return -EBADMSG;
    }

    if ( api->magic == STCP_CTX_MAGIC_POISON ) {
        LWRN("API header magic is poisoned already");
        return -EBADMSG;
    }

    if (api->magic_footer == STCP_CTX_MAGIC_POISON ) {
        LWRN("API footer magic is poisoned already");
        return -EBADMSG;
    }

    if (api->magic != STCP_CTX_MAGIC_ALIVE) {
        LWRN("API %p invalid magic at header", api);
        return -EBADMSG;
    }

    if (api->magic_footer != STCP_CTX_MAGIC_ALIVE_FOOTER) {
        LWRN("API %p invalid magic at footer", api);
        return -EBADMSG;
    }

    if (!api->ctx) {
        return -EINVAL;
    }
    
    struct stcp_ctx* ctx = api->ctx;

    if (!ctx) {
        LWRN("CTX POINTER NULL %p", ctx);
        return -EBADMSG;
    }


    if ((uintptr_t)ctx < 0x20000000) {
        LWRN("CTX POINTER INVALID %p", ctx);
        return -EBADMSG;
    }

    if ( ctx->magic == STCP_CTX_MAGIC_POISON ) {
        LWRN("Context header magic is poisoned already");
        return -EBADMSG;
    }

    if ( ctx->magic_footer == STCP_CTX_MAGIC_POISON ) {
        LWRN("Context footer magic is poisoned already");
        return -EBADMSG;
    }

    if (ctx->magic != STCP_CTX_MAGIC_ALIVE) {
        LWRN("CTX %p invalid magic at header", ctx);
        return -EBADMSG;
    }

    if (ctx->magic_footer != STCP_CTX_MAGIC_ALIVE_FOOTER) {
        LWRN("CTX %p invalid magic at footer", ctx);
        return -EBADMSG;
    }
        
    int rv = (int)atomic_get(&api->alive);

    return rv;
}
void stcp_api_try_to_wakeup_radio(void) {
    stcp_transport_wakeup_radio();
}

/*
    Misc juttuja RUST Puolelle käyttöön
*/

int stcp_rust_api_transport_get_fd(void *pks_void) {
    struct kernel_socket *pks = pks_void;
    if (!pks) { return -EBADMSG; }
    int fd = pks->fd;
    LDBG("STCP: Transport[%p] has FD: %d", fd);
    return fd;
}


