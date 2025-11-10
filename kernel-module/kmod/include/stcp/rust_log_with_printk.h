#ifndef STCP_RUST_LOG_HELPER_H
#define STCP_RUST_LOG_HELPER_H 1

// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/stdarg.h>

/*
 * Yksi yhtenäinen funktio, jossa on __VA_ARGS__-tuki.
 * Tätä voi kutsua Rustista turvallisesti FFI:n kautta.
 */
void stcp_rust_kernel_printk(const char *fmt, ...);

#endif