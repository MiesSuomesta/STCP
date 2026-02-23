#include <zephyr/kernel.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include "debug.h"
LOG_MODULE_REGISTER(stcp_c_memory_allocations, LOG_LEVEL_INF);

#define STCP_MEM_DEBUG 0 

/*
 * Toteutus 1: Zephyr system heap (k_malloc/k_free)
 * -> vaatii CONFIG_HEAP_MEM_POOL_SIZE > 0
 */

void *stcp_alloc(size_t n)
{
    if (n == 0) n = 1;
    void *p = k_malloc(n);
#if STCP_MEM_DEBUG
    SDBG("MEM: Alloc %d => %p", n, p);
#endif
    return p;
}

void stcp_free(void *p)
{
    if (p) {

#if STCP_MEM_DEBUG
        SDBG("MEM: Free %p", p);
#endif
        k_free(p);
    }
}

/*
 * Optionaalinen: aligned alloc.
 * Jos Rust/crypto joskus tarvitsee isompaa alignia.
 * Demossa voi jättää pois ja palata tähän jos tulee alignment-hardfault.
 */
void *stcp_alloc_aligned(size_t align, size_t n)
{
    if (n == 0) n = 1;
#if defined(CONFIG_COMMON_LIBC_MALLOC) || defined(CONFIG_MINIMAL_LIBC_MALLOC) || defined(CONFIG_NEWLIB_LIBC)
    /* jos libc tarjoaa aligned_alloc */
    /* HUOM: C11 aligned_alloc vaatii size multiple of align */
    size_t sz = (n + (align - 1)) & ~(align - 1);
    extern void *aligned_alloc(size_t, size_t);
    return aligned_alloc(align, sz);
#else
    /* fallback: ei toteutettu */
    (void)align;
    return NULL;
#endif
}

void stcp_free_aligned(void *p)
{
    stcp_free(p);
}

