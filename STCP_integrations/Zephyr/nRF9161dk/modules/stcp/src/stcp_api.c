#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <stdbool.h>
#include <errno.h>

#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>


int stcp_api_get_handshake_status(struct stcp_api *api)
{
    if (!api || !api->ctx) {
        LERR("No api..");
        return -EINVAL;
    }
    API_LOCK(api);
        LDBG("[API %p] Handshake done: %d", api, api->ctx->handshake_done);
        int val = api->ctx->handshake_done;
    API_UNLOCK(api);
    return val;
}

int stcp_api_wait_until_reached_ip_network_up(struct stcp_api *api, int timeout) {
    API_LOCK(api);
        int rv = stcp_fsm_wait_until_reached_ip_network_up(NULL, timeout);
    API_UNLOCK(api);
    return rv;
}

int stcp_api_wait_until_reached_lte_ready(struct stcp_api *api, int timeout) {
    API_LOCK(api);
        int rv = stcp_fsm_wait_until_reached_lte_ready(NULL, timeout);
    API_UNLOCK(api);
    return rv;
}

int stcp_api_wait_until_reached_pdn_ready(struct stcp_api *api, int timeout) {
    API_LOCK(api);
        int rv = stcp_fsm_wait_until_reached_pdn_ready(NULL, timeout);
    API_UNLOCK(api);
    return rv;
}

int stcp_api_wait_until_reached_connect_ready(struct stcp_api *api, int timeout) {

    if (!api || !api->ctx) {
        LERR("No api..");
        return -EINVAL;
    }

    API_LOCK(api);
        int rv = stcp_fsm_wait_until_reached_connect_ready(&api->ctx->fsm, timeout);
    API_UNLOCK(api);
    return rv;
}
