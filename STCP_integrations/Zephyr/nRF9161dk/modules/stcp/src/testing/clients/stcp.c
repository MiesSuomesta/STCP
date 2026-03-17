
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

#define LOGTAG     "[STCP/Torture/STCP] "
#include <stcp_testing_bplate.h>

#include <stcp_testing_common.h>

#if 0
void stcp_torture_worker(void *p1, void *p2, void *p3) 
{
    int fd;
    int rc;

    uint8_t rxbuf[STCP_TORTURE_BUF_SIZE];

    LINF("[WORKER %d] Starting...", worker_id);

    fd = stcp_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        LERR("socket failed: %d errno=%d", fd, errno);
        return;
    }

    rc = stcp_connect(fd, "lja.fi", 7777);
    if (rc < 0) {
        LERR("connect failed: %d errno=%d", rc, errno);
        stcp_close(fd);
        return;
    }

    LINF("[WORKER %d] Connected, entering loop", worker_id);

    while (1) {

        // =========================
        // 📡 RX (tämä ajaa handshakea!)
        // =========================
        int len = stcp_recv(fd, rxbuf, sizeof(rxbuf), 0);

        if (len > 0) {
            LDBG("[WORKER %d] RX %d bytes", worker_id, len);

        } else if (len == 0) {
            LINF("[WORKER %d] Connection closed", worker_id);
            break;

        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                LERR("[WORKER %d] recv error: %d errno=%d", worker_id, len, errno);
                break;
            }
        }

        // =========================
        // 🚀 TX (kun handshake valmis)
        // =========================
        // HUOM: stcp_send EI saa mennä ennen handshakea!
        if (stcp_is_connected(fd)) {

            static uint32_t counter = 0;

            char msg[64];
            int mlen = snprintk(msg, sizeof(msg),
                "worker %d msg %u", worker_id, counter++);

            int s = stcp_send(fd, msg, mlen, 0);

            if (s < 0) {
                LERR("[WORKER %d] send error: %d errno=%d", worker_id, s, errno);
                break;
            }

            LDBG("[WORKER %d] TX %d bytes", worker_id, s);
        }

        k_msleep(10);
    }

    stcp_close(fd);
    LINF("[WORKER %d] EXIT", worker_id);
}
#endif 

void stcp_torture_worker(void *p1, void *p2, void *p3) 
{
    TDBGBIG("Start STCP thread...");

    uint8_t *payload;
    uint8_t *rx_buff;
    struct stcp_api *api = NULL;
    int rc;

    payload = stcp_alloc(STCP_TORTURE_BUF_SIZE);
    rx_buff = stcp_alloc(STCP_TORTURE_BUF_SIZE);

    TDBGBIG("Start STCP testing while..");
        
    while (1) {
        TDBGBIG("Start at creationg while..");
        /* -------------------------------------------------- */
        /* create context if missing                          */
        /* -------------------------------------------------- */

        if (!api) {

            int fd = stcp_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            LDBG("Got pristine FD: %d", fd);
            if (fd < 0) {
                k_sleep(K_MSEC(500));
                break;
            }
 
            rc = stcp_api_init_with_fd(&api, fd);

            if (!api) {
                stcp_net_close_fd(&fd);
                k_sleep(K_MSEC(500));
                continue;
            }

            LDBG("Got api %p with FD after create: %d", api, stcp_api_get_fd(api));
        }

        /* -------------------------------------------------- */
        /* connect                                            */
        /* -------------------------------------------------- */

        rc = stcp_api_connect(api,
                the_test_server_addr_resolved->ai_addr,
                the_test_server_addr_resolved->ai_addrlen);

        TDBGBIG("Connected? %d", rc);

        if (rc < 0) {

            TWRN("connect failed rc=%d", rc);

            stcp_api_close(api);
            api = NULL;

            k_sleep(K_MSEC(1000));
            continue;
        }

        if (!stcp_api_get_handshake_status(api)) {
            LWRN("API[%p] Handshake not done!", api);

            stcp_api_close(api);
            api = NULL;

            k_sleep(K_MSEC(1000));
            continue;
        }

        TINF("connected");

        /* -------------------------------------------------- */
        /* traffic loop                                       */
        /* -------------------------------------------------- */
        stcp_statistics_inc(STAT_RUNNING, 1);

        while (1) {

            /* random payload */
            sys_rand_get(payload, sizeof(payload));

            //rc = stcp_send(ctx, payload, sizeof(payload), 0);
            rc = stcp_api_send(api, payload, sizeof(payload), 0);

            if (rc == -ENOTRECOVERABLE ||
                rc == -EBADFD ||
                rc == -ESTALE ||
                rc == -ENOTCONN)
            {
                TWRN("context dead, recreating");

                if (api) {
                    stcp_api_close(api);
                    api = NULL;
                }

                k_sleep(K_MSEC(500));
                break;
            }

            if (rc == -EINPROGRESS) {
                k_sleep(K_MSEC(sys_rand32_get() % 200));
                continue;
            }

            rc = stcp_api_recv(api, rx_buff, sizeof(rx_buff), 0);

            if (rc == -ENOTRECOVERABLE ||
                rc == -EBADFD ||
                rc == -ESTALE ||
                rc == -ENOTCONN)
            {
                TWRN("context dead, recreating");

                if (api) {
                    stcp_api_close(api);
                    api = NULL;
                }

                k_sleep(K_MSEC(500));
                break;
            }

            if (rc == -EINPROGRESS) {
                k_sleep(K_MSEC(sys_rand32_get() % 200));
                continue;
            }

            /* random delay 10–200 ms */
            k_sleep(K_MSEC(sys_rand32_get() % 200));

            /* random reconnect */
            if ((sys_rand32_get() % 100) < 5) {
                TDBG("random reconnect");
                break;
            }

        }

        stcp_statistics_dec(STAT_RUNNING, 1);

        /* -------------------------------------------------- */
        /* disconnect                                         */
        /* -------------------------------------------------- */

        if (api) {
            stcp_close(api);
            api = NULL;
        }

        k_sleep(K_MSEC(1));

    }

    stcp_free(payload); 
    stcp_free(rx_buff); 

}
