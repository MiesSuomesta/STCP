#ifndef STCP_RUST_ALLOC_H
#define STCP_RUST_ALLOC_H

#include <linux/slab.h>
#include <linux/types.h>

/*
 * Yksinkertaiset wrapperit Rustin globaalille allokaattorille.
 * Tällä hetkellä käytetään GFP_KERNEL.
 * Jos tarvitset myöhemmin ATOMIC tms, voidaan lisätä erillinen API.
 */

extern void *stcp_rust_kernel_alloc(size_t size);
extern void  stcp_rust_kernel_free(const void *ptr);

#endif /* STCP_RUST_ALLOC_H */
