#ifndef STCP2_ZEPHYR_H
#define STCP2_ZEPHYR_H

#include <zephyr/net/socket.h>

/* STCPv2 public socket ABI. */
#ifndef AF_STCP
#define AF_STCP 45
#endif

#ifndef PF_STCP
#define PF_STCP AF_STCP
#endif

/* STCP has one protocol number. Carrier selection comes from socket type:
 *   SOCK_STREAM -> TCP carrier
 *   SOCK_DGRAM  -> UDP carrier
 */
#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

#endif /* STCP2_ZEPHYR_H */
