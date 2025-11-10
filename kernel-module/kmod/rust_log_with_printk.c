// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/stdarg.h>

/*
 * Yksi yhtenäinen funktio, jossa on __VA_ARGS__-tuki.
 * Tätä voi kutsua Rustista turvallisesti FFI:n kautta.
 */
void stcp_rust_kernel_printk(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}
