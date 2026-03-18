#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stdbool.h>
#include <errno.h>

#include <zephyr/net/mqtt.h>
#include <stdint.h>
#include <time.h>
#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>

#define LOGTAG     "[STCP/Torture/Common] "
#include <stcp_testing_bplate.h>

#include <stcp_testing_common.h>


struct zsock_addrinfo *the_test_server_addr_resolved = NULL;

int stcp_testing_is_connection_dead(int rc) {
    if (rc == -ENOTRECOVERABLE ||
        rc == -EBADFD ||
        rc == -ESTALE ||
        rc == -ENOTCONN)
    {
        return 1;
    }
    return 0;
}

int generate_strftime_payload(uint8_t *buf, size_t len)
{
#if CONFIG_DATE_TIME
    time_t now;
    struct tm tm;

    now = time(NULL);

    if (gmtime_r(&now, &tm) == NULL) {
        return 0;
    }

    return strftime(
        (char *)buf,
        len,
        CONFIG_STCP_TESTING_PAYLOAD_FORMAT,
        &tm
    );
#else
    uint64_t ms = k_uptime_get();

    uint32_t total_sec = ms / 1000;

    uint32_t h = (total_sec / 3600) % 24;
    uint32_t m = (total_sec / 60) % 60;
    uint32_t s = total_sec % 60;

    int pl = snprintk((char *)buf, len, "%02u:%02u:%02u %s", 
        h, m, s, CONFIG_STCP_TESTING_PAYLOAD_FORMAT);

    return pl;
#endif
}

int stcp_testing_resolve_test_host_address() {
    static int dns_done = 0;
    if (!dns_done) {

        int rc = zsock_getaddrinfo(
            CONFIG_STCP_TESTING_PEER_HOSTNAME_TO_CONNECT, 
            CONFIG_STCP_TESTING_PEER_PORT_TO_CONNECT,
            NULL, 
            &the_test_server_addr_resolved);

        if (!the_test_server_addr_resolved) {
            TERR("DNS address not resolved");
            return -EINVAL;
        }

        if (rc == 0) {
            TINF("DNS resolved for %s:%s",
                CONFIG_STCP_TESTING_PEER_HOSTNAME_TO_CONNECT, 
                CONFIG_STCP_TESTING_PEER_PORT_TO_CONNECT
            );
            dns_done = 1;
        } else {
            TWRN("DNS resolve failed!");
        }
    }
    return dns_done;
}

int stcp_testing_connect_peer(struct stcp_api *api, struct stcp_ctx *ctx) {
    int rc = 0;
    if (api) {
        TDBG("HAS API => connectin via stcp_api_connect...");
        // STCP or MQTT

        if (api->ctx && api->ctx->closing) {
            TWRN("CTX closing, skipping connect");
            return -EBADFD;
        }

        rc = stcp_api_connect(api, 
            the_test_server_addr_resolved->ai_addr, 
            the_test_server_addr_resolved->ai_addrlen);

    }
    
    if (ctx) {
        TDBG("HAS CTX => connectin via stcp_connect...");

        if (ctx->closing) {
            TWRN("CTX closing, skipping connect");
            return -EBADFD;
        }

        rc = stcp_connect(ctx,
                the_test_server_addr_resolved->ai_addr, 
                the_test_server_addr_resolved->ai_addrlen);
    }

    TDBG("connect peer, rc: %d", rc);
    return rc;
}

static K_MUTEX_DEFINE(g_peer_socket);
int stcp_testing_get_peer_socket(struct stcp_api **apiTo)
{
    static int init = 0;
    int fd;
    struct stcp_api *theApi = NULL;
    
    if (!init) {
        k_mutex_init(&g_peer_socket);
    }

    k_mutex_lock(&g_peer_socket, K_FOREVER);

        fd = stcp_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fd < 0) {
            TERR("Got no socket!, %d / %d", fd, errno);
            k_mutex_unlock(&g_peer_socket);
            return fd;
        }

        if (apiTo) {
            *apiTo = NULL;

            int rc = stcp_api_init_with_fd(&theApi, fd);
            if (rc < 0) {
                TERR("Got error %d", rc);
                if (theApi) {
                    stcp_api_close(theApi);
                }
            } else {
                *apiTo = theApi;
            }
        }

    k_mutex_unlock(&g_peer_socket);
    TINF("Created contex %p with fd %d", theApi, fd);
    return fd;
}
