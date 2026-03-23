#pragma once

// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/stdarg.h>

#include <linux/types.h>

void stcp_rust_log(int level, const char *buf, size_t len); 