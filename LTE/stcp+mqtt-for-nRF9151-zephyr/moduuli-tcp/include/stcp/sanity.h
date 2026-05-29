#pragma once

enum stcp_sanity_phase {
    STCP_SANITY_INIT = 0,
    STCP_SANITY_API_LINKED,
    STCP_SANITY_READY,
    STCP_SANITY_CONNECTED,
    STCP_SANITY_USABLE,
};

int is_prt_at_text_addr(uintptr_t addr);
int is_stack_ptr_valid(uintptr_t sp);
int stcp_api_sanity_check(struct stcp_api *api, enum stcp_sanity_phase phase);
