#ifndef STCP_LTE_TRANSPORT_H_
#define STCP_LTE_TRANSPORT_H_

#include <stdbool.h>
#include <stdint.h>

struct stcp_lte_transport_state {
    bool initialized;
    bool registered;
    bool pdn_active;
    bool ip_active;
    bool rrc_connected;
    bool roaming;
    bool custom_pdn_active;
    uint8_t pdn_cid;
    int pdn_id;
    int last_esm_error;
};

int stcp_lte_transport_init(void);
int stcp_lte_transport_wait_ready(int timeout_seconds);
int stcp_lte_transport_get_state(struct stcp_lte_transport_state *state);
bool stcp_lte_transport_is_ready(void);
uint8_t stcp_lte_transport_pdn_cid(void);
int stcp_lte_transport_pdn_id(void);
int stcp_lte_transport_bind_socket(int fd);

#endif /* STCP_LTE_TRANSPORT_H_ */
