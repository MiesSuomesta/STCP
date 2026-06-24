#pragma once
#include <stcp/stcp_alloc.h>
#include <stcp/low_level_operations.h>

#if STCP_POINTER_TRACKING 
#define STCP_POINTER_TRACKING_USAGE_FROM_RUST 0

struct stcp_pointer_item {
    void *ptr;
    int alive;
    int from_rust;
    size_t bytes;
    void *first_lr;
    void *lr;
    struct stcp_debug_info *alloc_dip;
    struct stcp_debug_info *dealloc_dip;
};


struct stcp_pointer_item * stcp_pointer_list_get_slot_ptr(int slot);
int stcp_pointer_list_get_free_slot();
int stcp_pointer_list_get_slot_with_pointer(void* pPrt);
int stcp_pointer_list_use_slot(int slot, void* pPtr, size_t bytes, void *lr, int rust);
int stcp_pointer_list_free_slot(int slot, void *freeLR);
void stcp_pointer_dump_status();
int stcp_track_pointer(void *pPtr, void *lr, size_t size, int rust);
int stcp_untrack_pointer(void* pPtr, void* freelr);

void *stcp_low_level_mem_alloc(size_t n, int fromRust);
void  stcp_low_level_mem_dealloc(void *p, int fromRust);

#define STCP_MEMORY_ALLOC(val)       stcp_low_level_mem_alloc(val, STCP_POINTER_TRACKING_USAGE_FROM_RUST)
#define STCP_MEMORY_DEALLOC(val)     stcp_low_level_mem_dealloc(val, STCP_POINTER_TRACKING_USAGE_FROM_RUST)
#else
#define STCP_MEMORY_ALLOC(val)      k_stcp_alloc(val)
#define STCP_MEMORY_DEALLOC(val)    k_stcp_free(val)
#endif

