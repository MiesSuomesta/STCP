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
    LDBG("[API %p] Handshake done: %d", api, api->ctx->handshake_done);
    return api->ctx->handshake_done;
}

int stcp_api_wait_until_reached_ip_network_up(struct stcp_api *api, int timeout) {
    ARG_UNUSED(api);
    return stcp_fsm_wait_until_reached_ip_network_up(NULL, timeout);
}

int stcp_api_wait_until_reached_lte_ready(struct stcp_api *api, int timeout) {
    ARG_UNUSED(api);
    return stcp_fsm_wait_until_reached_lte_ready(NULL, timeout);
}

int stcp_api_wait_until_reached_pdn_ready(struct stcp_api *api, int timeout) {
    ARG_UNUSED(api);
    return stcp_fsm_wait_until_reached_pdn_ready(NULL, timeout);
}

int stcp_api_wait_until_reached_connect_ready(struct stcp_api *api, int timeout) {

    if (!api || !api->ctx) {
        LERR("No api..");
        return -EINVAL;
    }

    return stcp_fsm_wait_until_reached_connect_ready(&api->ctx->fsm, timeout);
}
