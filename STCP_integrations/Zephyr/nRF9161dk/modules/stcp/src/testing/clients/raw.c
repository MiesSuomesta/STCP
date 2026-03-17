
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stdbool.h>
#include <errno.h>
#include <zephyr/random/random.h>

#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/debug.h>
#include <stcp/stcp_tcp_low_level_operations.h>

#include <status_monitor.h>

#define LOGTAG     "[STCP/Torture/TCP] "
#include <stcp_testing_bplate.h>
#include <stcp_testing_common.h>


void tcp_torture_worker(void *p1, void *p2, void *p3) {
    struct stcp_ctx *ctx = NULL;
    int rc;

    uint8_t payload[STCP_TORTURE_BUF_SIZE];
    uint8_t rx_buff[STCP_TORTURE_BUF_SIZE];

    while (1) {

        /* -------------------------------------------------- */
        /* create context if missing                          */
        /* -------------------------------------------------- */

        if (!ctx) {

            int fd = stcp_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (fd < 0) {
                k_sleep(K_MSEC(500));
                continue;
            }

            ctx = stcp_create_new_context(fd);

            if (!ctx) {
                stcp_net_close_fd(&fd);
                k_sleep(K_MSEC(500));
                continue;
            }
        }

        /* -------------------------------------------------- */
        /* connect                                            */
        /* -------------------------------------------------- */

        rc = stcp_connect(ctx,
                the_test_server_addr_resolved->ai_addr,
                the_test_server_addr_resolved->ai_addrlen);

        if (rc < 0) {

            LWRN("connect failed rc=%d", rc);

            stcp_close(ctx);
            ctx = NULL;

            k_sleep(K_MSEC(1000));
            continue;
        }

        LINF("connected");

        /* -------------------------------------------------- */
        /* traffic loop                                       */
        /* -------------------------------------------------- */
        stcp_statistics_inc(STAT_RUNNING, 1);
        int my_fd = ctx->ks.fd;

        while (1) {

            /* random payload */
            sys_rand_get(payload, sizeof(payload));

            //rc = stcp_send(ctx, payload, sizeof(payload), 0);
            rc = stcp_tcp_send_via_fd(my_fd, payload, sizeof(payload));

            if (rc == -ENOTRECOVERABLE ||
                rc == -EBADFD ||
                rc == -ENOTCONN)
            {
                LWRN("context dead, recreating");

                if (ctx) {
                    stcp_close(ctx);
                    ctx = NULL;
                }

                k_sleep(K_MSEC(500));
                continue;
            }

            rc = stcp_tcp_recv_via_fd(my_fd, rx_buff, sizeof(rx_buff), 0, 0 , NULL);

            if (rc == -ENOTRECOVERABLE ||
                rc == -EBADFD ||
                rc == -ENOTCONN)
            {
                LWRN("context dead, recreating");

                if (ctx) {
                    stcp_close(ctx);
                    ctx = NULL;
                }

                k_sleep(K_MSEC(500));
                continue;
            }

            /* random delay 10–200 ms */
            k_sleep(K_MSEC(sys_rand32_get() % 200));

            /* random reconnect */
            if ((sys_rand32_get() % 100) < 5) {
                LDBG("random reconnect");
                break;
            }

        }

        stcp_statistics_dec(STAT_RUNNING, 1);

        /* -------------------------------------------------- */
        /* disconnect                                         */
        /* -------------------------------------------------- */

        if (ctx) {
            stcp_close(ctx);
            ctx = NULL;
        }

        k_sleep(K_MSEC(200));

    }
}

