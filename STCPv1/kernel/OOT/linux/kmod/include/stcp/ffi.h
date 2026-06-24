

// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/stdarg.h>

#ifndef STCP_FFI_H
#define STCP_FFI_H 1

extern void stcp_kgetrandom(void *buf, size_t len);

#endif
