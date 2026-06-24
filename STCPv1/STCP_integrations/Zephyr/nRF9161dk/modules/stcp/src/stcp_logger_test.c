#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <string.h>


#define TEST_HOST "192.168.1.20"
#define TEST_PORT 7755

static void paxlogd_test_thread(void)
{
    while (1) {
        LINF("paxlogd test: connecting...");

        int fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fd < 0) {
            LERR("socket() failed: %d", errno);
            k_sleep(K_SECONDS(5));
            continue;
        }

        struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port = htons(TEST_PORT),
        };

        if (zsock_inet_pton(AF_INET, TEST_HOST, &addr.sin_addr) != 1) {
            LERR("inet_pton failed");
// pois             zsock_close(fd);
            k_sleep(K_SECONDS(5));
            continue;
        }

        if (zsock_connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            LERR("connect() failed: %d", errno);
// pois             zsock_close(fd);
            k_sleep(K_SECONDS(5));
            continue;
        }

        const char *msg = "HELLO FROM MODEM\n";
        ssize_t rc = zsock_send(fd, msg, strlen(msg), 0);

        if (rc < 0) {
            LERR("send() failed: %d", errno);
        } else {
            LINF("sent %d bytes", rc);
        }

// pois         zsock_close(fd);
        k_sleep(K_SECONDS(10));
    }
}

K_THREAD_DEFINE(paxlogd_test_tid, 2048, paxlogd_test_thread, NULL, NULL, NULL, 7, 0, 0);
