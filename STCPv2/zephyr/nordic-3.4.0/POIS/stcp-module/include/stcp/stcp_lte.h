#ifndef STCP_LTE_H
#define STCP_LTE_H

#include <stdbool.h>

struct stcp_lte_state {
    bool registered;
    bool pdn_active;
    bool ip_ready;
    bool radio_connected;
};

#if defined(CONFIG_STCP_LTE)
int stcp_lte_init_and_connect(int timeout_seconds);
int stcp_lte_wait_until_ready(int timeout_seconds);
int stcp_lte_get_state(struct stcp_lte_state *state);
int stcp_lte_dump_status(void);
#else
static inline int stcp_lte_init_and_connect(int timeout_seconds)
{
    (void)timeout_seconds;
    return 0;
}
static inline int stcp_lte_wait_until_ready(int timeout_seconds)
{
    (void)timeout_seconds;
    return 0;
}
static inline int stcp_lte_get_state(struct stcp_lte_state *state)
{
    if (state != NULL) {
        state->registered = false;
        state->pdn_active = false;
        state->ip_ready = false;
        state->radio_connected = false;
    }
    return 0;
}
static inline int stcp_lte_dump_status(void) { return 0; }
#endif

#endif
