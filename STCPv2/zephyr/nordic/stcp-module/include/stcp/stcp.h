#ifndef STCP_ZEPHYR_STCP_H_
#define STCP_ZEPHYR_STCP_H_

#include <stdint.h>
#include <zephyr/net/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Keep these aligned with the Linux STCP implementation for source portability. */
#ifndef AF_STCP
#define AF_STCP 45
#endif
#ifndef PF_STCP
#define PF_STCP AF_STCP
#endif
#define STCP_PROTO 253
#define STCP_ADDR_LEN 16

struct sockaddr_stcp {
    sa_family_t stcp_family;
    uint16_t stcp_port;
    uint8_t stcp_addr[STCP_ADDR_LEN];
};

#ifdef __cplusplus
}
#endif

#endif
