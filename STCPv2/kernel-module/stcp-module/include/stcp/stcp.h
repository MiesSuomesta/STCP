#ifndef STCP2_ZEPHYR_H
#define STCP2_ZEPHYR_H
#include <zephyr/net/socket.h>
#ifndef AF_STCP
#define AF_STCP 45
#endif
#ifndef PF_STCP
#define PF_STCP AF_STCP
#endif
#define STCP_PROTO_TCP 253
#define STCP_PROTO_UDP 254
#endif
