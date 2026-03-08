#include <zephyr/kernel.h>
#include "stcp/debug.h"
LOG_MODULE_REGISTER(stcp_rust_memory_allocations, LOG_LEVEL_INF);

/*
 * Yksinkertaiset wrapperit Rustin globaalille allokaattorille.
 * Tällä hetkellä käytetään GFP_KERNEL.
 * Jos tarvitset myöhemmin ATOMIC tms, voidaan lisätä erillinen API.
 */
// Lippu, jos laittasi moduulin argumentteihin mukaan ??
#define STCP_MEM_DEBUG 0

#if STCP_MEM_DEBUG
int debug_memory_alloc = 0;
#endif

void *stcp_rust_kernel_alloc(size_t pSize)
{
    void *p = k_malloc(pSize);
    if (p) {
        memset(p, 0, pSize);
    }

#if STCP_MEM_DEBUG
    if (debug_memory_alloc) {
        SDBG("MEM: Alloc %d bytes => %p", pSize, p);
    }
#endif
    return p;
}

void stcp_rust_kernel_free(void *pFrom)
{

#if STCP_MEM_DEBUG
    if (debug_memory_alloc) {
        SDBG("MEM: Free %p", pFrom);
    }
#endif

    k_free(pFrom);
}
