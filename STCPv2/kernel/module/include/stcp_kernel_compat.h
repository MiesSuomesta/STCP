/* SPDX-License-Identifier: GPL-2.0 */
#ifndef STCP_KERNEL_COMPAT_H
#define STCP_KERNEL_COMPAT_H

/*
 * Socket address API compatibility.
 *
 * The module Makefile probes the target kernel's include/linux/net.h and
 * passes STCP_HAVE_SOCKADDR_UNSIZED=1 through Kbuild when struct proto_ops
 * uses struct sockaddr_unsized *.  Raspberry Pi 6.18 currently uses the
 * older struct sockaddr * callbacks, while newer linux-next uses
 * sockaddr_unsized.  Keep all casts and callback prototypes in one place.
 */
#include <linux/net.h>
#include <linux/socket.h>

#ifdef STCP_HAVE_SOCKADDR_UNSIZED
typedef struct sockaddr_unsized stcp_sockaddr_t;
#define STCP_KERNEL_SOCKADDR(addr) ((struct sockaddr_unsized *)(addr))
#else
typedef struct sockaddr stcp_sockaddr_t;
#define STCP_KERNEL_SOCKADDR(addr) ((struct sockaddr *)(addr))
#endif

#endif /* STCP_KERNEL_COMPAT_H */
