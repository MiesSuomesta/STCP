
#include <linux/slab.h>
#include <linux/types.h>
#include <stcp/rust_alloc.h>
/*
 * Yksinkertaiset wrapperit Rustin globaalille allokaattorille.
 * Tällä hetkellä käytetään GFP_KERNEL.
 * Jos tarvitset myöhemmin ATOMIC tms, voidaan lisätä erillinen API.
 */
// Lippu, jos laittasi moduulin argumentteihin mukaan ??
static int debug_memory_alloc = 0;

void *stcp_rust_kernel_alloc(size_t pSize)
{
    void *p = kmalloc(pSize, GFP_KERNEL);
    if (debug_memory_alloc) {
        pr_debug("stcp: Allocating %zu (%px)", pSize, p);
    }
    return p;
}

void stcp_rust_kernel_free(const void *pFrom)
{
    if (debug_memory_alloc) {
        pr_debug("stcp: Freeing %px", pFrom);
    }
    kfree(pFrom);
}
