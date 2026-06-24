
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stdbool.h>
#include <errno.h>
#include <zephyr/random/random.h>

#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/stcp_socket.h>
#include <stcp/utils.h>
#include <stcp/debug.h>
#include <stcp/stcp_tcp_low_level_operations.h>


#define LOGTAG     "[STCP/Torture/TCP] "
#include <stcp_testing_bplate.h>
#include <stcp_testing_common.h>
#include <status_monitor.h>

void tcp_torture_worker(void *p1, void *p2, void *p3) 
{
    TDBG("Start TCP thread...");

    uint8_t *payload;
    uint8_t *rx_buff;
    struct stcp_api *api = NULL;
    int rc;

    payload = stcp_alloc(STCP_TORTURE_BUF_SIZE);
    rx_buff = stcp_alloc(STCP_TORTURE_BUF_SIZE);

    TDBG("Waiting for network.....");
    stcp_api_wait_until_reached_ip_network_up(NULL, 60*60);
    TDBG("Network UP! Starting STCP testing while..");

    while (1) {
        TDBG("Start at creationg while..");
        /* -------------------------------------------------- */
        /* create context if missing                          */
        /* -------------------------------------------------- */

        if (!api) {

            int fd = stcp_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            TDBG("Got pristine FD: %d", fd);
            if (fd < 0) {
                k_sleep(K_MSEC(500));
                continue;;
            }
 
            rc = stcp_api_init_with_fd(&api, fd);

            if (!api) {
                stcp_net_close_fd(&fd);
                k_sleep(K_MSEC(500));
                continue;
            }

            TDBG("Got api %p with FD after create: %d", api, stcp_api_get_fd(api));
        }

        /* -------------------------------------------------- */
        /* connect                                            */
        /* -------------------------------------------------- */

        rc = stcp_api_connect(api,
                the_test_server_addr_resolved->ai_addr,
                the_test_server_addr_resolved->ai_addrlen);

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
        int keepSending = 1;
        TDBG("to sending loop...");
        int connection_fd = stcp_api_get_fd(api);
        while (keepSending) {
            TDBG("in sending loop...");
            /* random payload */
            generate_strftime_payload(payload, sizeof(payload));

            rc = stcp_tcp_send_via_fd(connection_fd, payload, sizeof(payload));

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

            rc = stcp_tcp_recv_via_fd(connection_fd, rx_buff, sizeof(rx_buff), 1, 0, NULL);

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
            if ((sys_rand32_get() % 100) < 5) {
                TDBG("random reconnect");
                keepSending = 0;
                continue;
            }
        }

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

