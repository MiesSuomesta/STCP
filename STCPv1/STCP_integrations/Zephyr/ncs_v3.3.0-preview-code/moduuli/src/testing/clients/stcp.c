
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stdbool.h>
#include <errno.h>
#include <zephyr/random/random.h>

#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/stcp_tcp_low_level_operations.h>
#include <stcp/debug.h>
#include <status_monitor.h>
#include <stcp/dns.h>

#define LOGTAG     "[STCP/Torture/STCP] "
#include <stcp_testing_bplate.h>

#include <stcp_testing_common.h>

#define STCP_API_CLOSE(api) \
        TDBG("Closing api? %p", api);       \
        if (api) {                          \
            TDBG("Closing api %p", api);    \
            stcp_api_close(api);            \
            (api) = NULL;                   \
            TDBG("Closed api %p", api);     \
        }


void stcp_torture_worker(void *p1, void *p2, void *p3) 
{
    TDBG("Start STCP thread...");

    uint8_t *payload;
    uint8_t *rx_buff;
    struct stcp_api *api = NULL;
    int rc;

    payload = stcp_alloc(STCP_TORTURE_BUF_SIZE);
    rx_buff = stcp_alloc(STCP_TORTURE_BUF_SIZE);

    TDBG("Waiting for network.....");
    stcp_api_wait_until_reached_ip_network_up(NULL, 60*60);
    TDBG("Network UP! Starting STCP testing while..");

    /* -------------------------------------------------- */
    /* Resolve                                            */
    /* -------------------------------------------------- */
   
    while (1) {
/*
        TDBG("Starting to do DNS? %p", the_test_server_addr_resolved);
        while ( the_test_server_addr_resolved == NULL ) {
            TDBG("Resolving %s:%d", pTestingHostName, pTestingHostPort);

            rc = stcp_dns_resolve(
                pTestingHostName, 
                pTestingHostPort,
                &the_test_server_addr_resolved
            );

            TDBG("resolver ret %d, %p",
                rc, the_test_server_addr_resolved);

            if (rc >= 0 && the_test_server_addr_resolved != NULL) {
                LDBG("Got resolved, %p!", the_test_server_addr_resolved);
                break;
            }
            sleep_ms_jitter(2500, 300);
        }
*/
        /* -------------------------------------------------- */
        /* API & Connect && looop                             */
        /* -------------------------------------------------- */
    
        TDBG("Start at creationg while..");
        /* -------------------------------------------------- */
        /* create context if missing                          */
        /* -------------------------------------------------- */
        if (!api) {
/*
          int fd = stcp_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            TDBG("Got pristine FD: %d", fd);
            if (fd < 0) {
                k_sleep(K_MSEC(500));
                continue;;
            }
*/
            rc = stcp_api_init(&api);
            TDBG("Got API %p, rc: %d", api, rc);

            if (!api) {
                k_sleep(K_MSEC(500));
                continue;
            }

            TDBG("Got api %p with FD after create: %d", api, stcp_api_get_fd(api));
        }

        struct zsock_addrinfo *dns = NULL;
        rc = stcp_api_get_dns_info(api, &dns);
        if (rc < 0) {
            TERR("DNS resolfing error, rc: %d, errno: %d", rc, errno);
            stcp_api_close(api);
            api = NULL;
            sleep_ms_jitter(1000,300);
            continue;
        }
        /* -------------------------------------------------- */
        /* connect                                            */
        /* -------------------------------------------------- */

        TDBG("Connecting API: %p", api);
        TDBG("Resolved : %p", dns);

        if (dns == NULL) {
            stcp_api_close(api);
            api = NULL;
            sleep_ms_jitter(1000,300);
            continue;
        }

        rc = stcp_api_connect(api,
                              dns->ai_addr,
                              dns->ai_addrlen);

        TDBG("Connected? %d", rc);
        if (rc < 0) {

            TWRN("connect failed rc=%d", rc);

            STCP_API_CLOSE(api);

            k_sleep(K_MSEC(1000));
            continue;
        }

        if (!stcp_api_get_handshake_status(api)) {
            TWRN("API[%p] Handshake not done!", api);

            STCP_API_CLOSE(api);

            k_sleep(K_MSEC(1000));
            continue;
        }

        TINF("connected");

        /* -------------------------------------------------- */
        /* traffic loop                                       */
        /* -------------------------------------------------- */
        stcp_statistics_inc(STAT_RUNNING, 1);
        int keepSending = atomic_get(&connection_good);
        TDBG("to sending loop...");
        while (keepSending) {
            TDBG("in sending loop...");
            /* random payload */
//            sys_rand_get(payload, sizeof(payload));
            generate_strftime_payload(payload, sizeof(payload));

            //rc = stcp_send(ctx, payload, sizeof(payload), 0);
            rc = stcp_api_send(api, payload, sizeof(payload), 0);

            if (stcp_testing_is_connection_dead(rc))
            {
                TWRN("context dead, recreating");

                STCP_API_CLOSE(api);

                k_sleep(K_MSEC(200));
                keepSending = 0;
                continue;
            }

            if (rc == -EINPROGRESS) {
                k_sleep(K_MSEC(sys_rand32_get() % 20));
                continue;
            }

            rc = stcp_api_recv(api, rx_buff, sizeof(rx_buff), 0);

            if (stcp_testing_is_connection_dead(rc))
            {
                TWRN("context dead, recreating");

                STCP_API_CLOSE(api);

                k_sleep(K_MSEC(200));
                keepSending = 0;
                continue;
            }

            if (rc == -EINPROGRESS) {
                k_sleep(K_MSEC(sys_rand32_get() % 20));
                continue;
            }

            /* random reconnect */
            keepSending = atomic_get(&connection_good);
            if ((sys_rand32_get() % 100) < 5) {
                TDBG("random reconnect");
                keepSending = 0;
                continue;
            }
            
        }

        LDBG("Exit from sending: %d", keepSending);
        stcp_statistics_dec(STAT_RUNNING, 1);

        /* -------------------------------------------------- */
        /* disconnect                                         */
        /* -------------------------------------------------- */

        STCP_API_CLOSE(api);

        k_sleep(K_MSEC(100));

    }

    stcp_free(payload); 
    stcp_free(rx_buff); 

}
