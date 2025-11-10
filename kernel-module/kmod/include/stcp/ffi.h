

// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/stdarg.h>

#ifndef STCP_FFI_H
#define STCP_FFI_H 1

extern void stcp_kgetrandom(void *buf, size_t len);

/*
 * Yksi yhtenäinen funktio, jossa on __VA_ARGS__-tuki.
 * Tätä voi kutsua Rustista turvallisesti FFI:n kautta.
 */
extern void stcp_rust_kernel_printk(const char *fmt, ...);

#endif
