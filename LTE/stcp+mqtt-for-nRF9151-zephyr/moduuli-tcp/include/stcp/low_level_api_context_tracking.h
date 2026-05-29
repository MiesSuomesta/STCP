#pragma once
#include <stcp/stcp_alloc.h>
#include <stcp/low_level_operations.h>

#if STCP_API_CONTEXT_TRACKING 

struct stcp_api_context_item {
    void *ptr;
    int alive;
    void *first_lr;
    void *lr;
    struct stcp_debug_info *alloc_dip;
    struct stcp_debug_info *dealloc_dip;
};


struct stcp_api_context_item* stcp_api_context_list_get_slot_ptr(int slot);
int stcp_api_context_list_get_free_slot();
int stcp_api_context_list_get_slot_with_pointer(void* pPrt);
int stcp_api_context_list_use_slot(int slot, void* pPtr, void *lr);
int stcp_api_context_list_free_slot(int slot, void *freeLR);
void stcp_pointer_dump_status();
int stcp_track_api_context(void *pPtr, void *lr);
int stcp_untrack_api_context(void* pPtr, void* freelr);

#define STCP_API_CONTEXT_TRACK(val)       stcp_track_api_context(val, stcp_get_lr())
#define STCP_API_CONTEXT_UNTRACK(val)     stcp_untrack_api_context(val, stcp_get_lr())
#else
// Nothing ..
#define STCP_API_CONTEXT_TRACK(val)       stcp_context_do_assert_check(val)
#define STCP_API_CONTEXT_UNTRACK(val)     stcp_context_do_assert_check(val)
#endif

