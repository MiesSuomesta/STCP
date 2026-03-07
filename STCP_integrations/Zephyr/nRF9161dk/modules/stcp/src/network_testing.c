#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/init.h>
#include <modem/lte_lc.h>

#include <zephyr/sys/heap_listener.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/kernel.h>

#include <zephyr/net/socket.h>
#include <zephyr/net/net_ip.h>
#include <errno.h>
#include <zephyr/sys/atomic.h>
#include <modem/nrf_modem_lib.h>

#include "stcp/stcp_struct.h"
#include "stcp/stcp_net.h"
#include "stcp/workers.h"
#include "stcp/utils.h"
#include "stcp/fsm.h"
#include "stcp/utils.h"
#include "stcp/stcp_operations_zephyr.h"

#include "stcp/stcp_rust_exported_functions.h"

LOG_MODULE_REGISTER(stcp_tcp_tester, LOG_LEVEL_INF);

int stcp_network_test_dns(const char *host, const char *port,
                               struct sockaddr_in *out)
{
    struct addrinfo hints = { 0 };
    struct addrinfo *res = NULL;
    int rc;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    LINF("DNS test: resolving %s:%s", host, port);

    rc = zsock_getaddrinfo(host, port, &hints, &res);
    if (rc != 0) {
        LERR("getaddrinfo failed rc=%d errno=%d", rc, errno);
        return -errno;
    }

    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;

    char ip[NET_IPV4_ADDR_LEN];
    net_addr_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));

    LINF("DNS OK: %s -> %s:%d", host, ip, ntohs(addr->sin_port));

    if (out) {
        memcpy(out, addr, sizeof(*out));
    }

    zsock_freeaddrinfo(res);
    return 0;
}

int stcp_network_ping_ip(const struct sockaddr_in *addr)
{
    int fd = zsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (fd < 0) {
        LERR("ping socket failed errno=%d", errno);
        return -errno;
    }

    char dummy = 0;
    int rc = zsock_sendto(fd, &dummy, 1, 0,
                          (struct sockaddr *)addr, sizeof(*addr));
// pois     zsock_close(fd);

    LDBG("Ping send rc=%d errno=%d", rc, errno);
    return rc;
}

int stcp_network_test_tcp_connect(const struct sockaddr_in *addr)
{
    int fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        LERR("socket failed errno=%d", errno);
        return -errno;
    }

    struct timeval tv = {
        .tv_sec = 5,
        .tv_usec = 0,
    };
    zsock_setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    zsock_setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    LDBG("TCP connect test...");

    int rc = zsock_connect(fd, (struct sockaddr *)addr, sizeof(*addr));
    if (rc < 0) {
        LERR("connect failed errno=%d", errno);
// pois         zsock_close(fd);
        return -errno;
    }

    LDBG("TCP connect OK!");
// pois     zsock_close(fd);
    return 0;
}

int stcp_network_test_network_avalability(const char *host, const char *port) {
    struct sockaddr_in theAddr;
    int err = stcp_network_test_dns(host, port, &theAddr);
    if ( err < 0 ) {
        return -1;
    }
    
/* EI LTE verkossa toimi
    err = stcp_network_ping_ip(&theAddr);
    if ( err < 0 ) {
        return -2;
    }
*/
    err = stcp_network_test_tcp_connect(&theAddr);
    if ( err < 0 ) {
        return -3;
    }

    return 0;
}

