#include <zephyr/kernel.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include "stcp/debug.h"
#include <stcp/stcp_struct.h>
#include <stcp/low_level_pointer.h>
#include <stcp/stcp_alloc.h>

#define STCP_MEM_DEBUG 1 

#ifdef STCP_POINTER_TRACKING_USAGE_FROM_RUST
#undef STCP_POINTER_TRACKING_USAGE_FROM_RUST
#endif

#define STCP_POINTER_TRACKING_USAGE_FROM_RUST 1

/*
 * Toteutus 1: Zephyr system heap (k_malloc/k_free)
 * -> vaatii CONFIG_HEAP_MEM_POOL_SIZE > 0
 */

void *stcp_alloc(size_t n)
{
    if (n == 0) n = 1;
    void *p = STCP_MEMORY_ALLOC(n);
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
        STCP_MEMORY_DEALLOC(p);
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
    stcp_rust_kernel_free(p);
}
