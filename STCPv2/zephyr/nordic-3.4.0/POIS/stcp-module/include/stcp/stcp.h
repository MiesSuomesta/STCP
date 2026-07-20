#ifndef STCP_ZEPHYR_H
#define STCP_ZEPHYR_H

#include <zephyr/net/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Kept identical to the Linux module. */
#ifndef AF_STCP
#define AF_STCP 45
#endif
#ifndef PF_STCP
#define PF_STCP AF_STCP
#endif

#define STCP_PROTO_DEFAULT 0
#define STCP_PROTO_TCP     253
#define STCP_PROTO_UDP     254

/*
 * Address ABI intentionally matches the Linux module: socket family is
 * AF_STCP, but bind/connect/getpeername/getsockname use sockaddr_in.
 */

#ifdef __cplusplus
}
#endif

#endif
