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

/* Linux/SDK compatible public selector for the STCP-UDP variant.
 *
 * Public ABI:
 *   AF_STCP + SOCK_STREAM + IPPROTO_STCP      -> STCP-TCP
 *   AF_STCP + SOCK_STREAM + IPPROTO_STCP_UDP  -> STCP-UDP
 *
 * Zephyr keeps backwards compatibility with the older
 * AF_STCP + SOCK_DGRAM + IPPROTO_STCP spelling as well.
 */
#ifndef IPPROTO_STCP_UDP
#define IPPROTO_STCP_UDP 254
#endif

#endif /* STCP2_ZEPHYR_H */
