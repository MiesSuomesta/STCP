#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

struct stcp_api;

#ifndef __aligned
#define __aligned(x) __attribute__((aligned(x)))
#endif

struct kernel_socket {
	int fd;
	void *kctx;
} __aligned(16);

