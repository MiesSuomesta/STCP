
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stdbool.h>
#include <errno.h>
#include <zephyr/random/random.h>

#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/stcp_tcp_low_level_operations.h>
#include <stcp/debug.h>

struct zsock_addrinfo *the_test_target_addr_resolved = NULL;
struct zsock_addrinfo *the_stcp_target_addr_resolved = NULL;

static const char *theTestingHostName = CONFIG_STCP_TESTING_PEER_HOSTNAME_TO_CONNECT;
static const int   theTestingHostPort = CONFIG_STCP_TESTING_PEER_PORT_TO_CONNECT;
static const char *theStcpTargetHostName  = CONFIG_STCP_CONNECT_TO_HOST;
static const int   theStcpTargetHostPort = CONFIG_STCP_CONNECT_TO_PORT;

int stcp_dns_resolve(const char *pHost, const int pPort, struct zsock_addrinfo **res) {

    char port_buf[10];
    struct zsock_addrinfo hints = { 0 };
    hints.ai_family = AF_INET;

    LDBG("Resolving %s:%d", pHost, pPort);

    if (!res) {
        return -EINVAL;
    }

    snprintk(port_buf, sizeof(port_buf), "%d", pPort);

    int rc = zsock_getaddrinfo(pHost, (const char*)port_buf, &hints, res);

    if (*res) {
        struct sockaddr_in *sin = (struct sockaddr_in *)((*res)->ai_addr);

        LDBG("Resolved %s:%d => ip=%d.%d.%d.%d port=%d",
            pHost,
            pPort,
            sin->sin_addr.s4_addr[0],
            sin->sin_addr.s4_addr[1],
            sin->sin_addr.s4_addr[2],
            sin->sin_addr.s4_addr[3],
            ntohs(sin->sin_port)
        );
    }

    LDBG("Resolved %s:%d => %p // %d", pHost, pPort, *res, rc);
    return rc;
}

int stcp_dns_resolve_testing_target() {
#if CONFIG_STCP_TESTING
    if (the_test_target_addr_resolved != NULL) {
        LWRN("Resolved test target already..");
        return -EALREADY;
    }

    int rc = stcp_dns_resolve(
        theTestingHostName, 
        theTestingHostPort,
        &the_test_target_addr_resolved
    );
    
    return rc;
#else
    return -EAFNOSUPPORT
#endif
}   

int stcp_dns_resolve_stcp_target() {

    if (the_stcp_target_addr_resolved != NULL) {
        LWRN("Resolved STCP target already..");
        return -EALREADY;
    }

    int rc = stcp_dns_resolve(
        theStcpTargetHostName, 
        theStcpTargetHostPort,
        &the_stcp_target_addr_resolved
    );
    return rc;
}   

int stcp_dns_free(struct zsock_addrinfo *ptr) {
    LDBG("Freeing DNS info from %p", ptr);
    if (!ptr) {
        return -EBADFD;
    }

    zsock_freeaddrinfo(ptr);
    return 0;
}   

int stcp_dns_free_testing_target() {
#if CONFIG_STCP_TESTING
    int rc = stcp_dns_free(the_test_target_addr_resolved);
    return rc;
#else
    return -EAFNOSUPPORT
#endif
}   

int stcp_dns_free_stcp_target() {
    int rc = stcp_dns_free(the_stcp_target_addr_resolved);
    return rc;
}   

int stcp_dns_resolve_all() {
    stcp_dns_resolve_stcp_target();
#if CONFIG_STCP_TESTING
    stcp_dns_resolve_testing_target();
#endif
    return 0;
}


