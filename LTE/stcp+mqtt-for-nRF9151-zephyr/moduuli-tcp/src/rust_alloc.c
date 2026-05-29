#include <zephyr/kernel.h>
#include "stcp/debug.h"
#include <stcp/low_level_pointer.h>

/*
 * Yksinkertaiset wrapperit Rustin globaalille allokaattorille.
 * Tällä hetkellä käytetään GFP_KERNEL.
 * Jos tarvitset myöhemmin ATOMIC tms, voidaan lisätä erillinen API.
 */
// Lippu, jos laittasi moduulin argumentteihin mukaan ??
#define STCP_MEM_DEBUG 1

#if STCP_MEM_DEBUG
int debug_memory_alloc = 1;
#endif

atomic_t alloc_cnt = ATOMIC_INIT(0);
atomic_t free_cnt  = ATOMIC_INIT(0);

void* k_stcp_alloc(size_t n) {
    
    if (n < 1) {
        LWRNBIG("ALLOC: %d sized => NULL", n);
        return NULL;
    }
    
    void* p = k_malloc(n);
    
    if (p != NULL) {
        memset(p, 0, n);
        return p;
    }

    return NULL;
}

void k_stcp_free(void* p) {
    if (p) {
        k_free(p);
    }
}

void *stcp_rust_kernel_alloc(uintptr_t pSize)
{
    LDBG("RUST ALLOC of %d at %p", pSize, stcp_get_lr());   
    void *p = STCP_MEMORY_ALLOC(pSize);
    if (p) {
        memset(p, 0, pSize);
#if STCP_MEM_DEBUG || STCP_POINTER_TRACKING
        atomic_inc(&alloc_cnt);
#endif
    } else {
        LERRBIG("OOM, Out of mana....");
#if STCP_POINTER_TRACKING
        LDBG("Pointer dump");
        stcp_pointer_dump_status();
#endif
    }

#if STCP_MEM_DEBUG || STCP_POINTER_TRACKING
    if (debug_memory_alloc) {
        SDBG("MEM: Alloc %d bytes => %p", pSize, p);
    }

    SDBG("ALLOC %d => %p (alloc=%d free=%d)",
         pSize, p,
         atomic_get(&alloc_cnt),
         atomic_get(&free_cnt));

#endif

    return p;
}

void stcp_rust_kernel_free(void *pFrom)
{
    LDBG("RUST FREE %p at %p", pFrom, stcp_get_lr());   

#if STCP_MEM_DEBUG || STCP_POINTER_TRACKING
        if (pFrom) {
            atomic_inc(&free_cnt);
        }
#endif

#if STCP_MEM_DEBUG || STCP_POINTER_TRACKING
    if (debug_memory_alloc) {
        SDBG("MEM: Free %p", pFrom);
    }
    
    SDBG("FREE %p (alloc=%d free=%d)",
         pFrom,
         atomic_get(&alloc_cnt),
         atomic_get(&free_cnt));
#endif

    STCP_MEMORY_DEALLOC(pFrom);
}
