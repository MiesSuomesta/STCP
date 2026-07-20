#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp.h>
#include <stcp/stcp_lte.h>

#define PROXY_HOST "lja.fi"
#define PROXY_PORT "7777"
#define RECONNECT_DELAY K_SECONDS(10)
#define RX_BUFFER_SIZE 1024

static int send_all(int fd, const void *data, size_t len)
{
    const uint8_t *p = data;
    size_t sent = 0;

    while (sent < len) {
        ssize_t ret = zsock_send(fd, p + sent, len - sent, 0);
        if (ret < 0) {
            return -errno;
        }
        if (ret == 0) {
            return -EPIPE;
        }
        sent += (size_t)ret;
    }
    return 0;
}

static int resolve_proxy(struct sockaddr_in *remote)
{
    struct zsock_addrinfo hints = {0};
    struct zsock_addrinfo *result = NULL;
    int ret;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    ret = zsock_getaddrinfo(PROXY_HOST, PROXY_PORT, &hints, &result);
    if (ret != 0 || result == NULL) {
        printk("DNS resolve failed for %s:%s, rc=%d\n",
               PROXY_HOST, PROXY_PORT, ret);
        return -EHOSTUNREACH;
    }

    if (result->ai_addrlen < sizeof(*remote)) {
        zsock_freeaddrinfo(result);
        return -EINVAL;
    }

    memcpy(remote, result->ai_addr, sizeof(*remote));
    zsock_freeaddrinfo(result);
    return 0;
}

static int connect_proxy(void)
{
    struct sockaddr_in remote = {0};
    int fd;
    int ret;

    ret = resolve_proxy(&remote);
    if (ret < 0) {
        return ret;
    }

    fd = zsock_socket(AF_STCP, SOCK_STREAM, STCP_PROTO_TCP);
    if (fd < 0) {
        printk("socket(AF_STCP) failed: errno=%d\n", errno);
        return -errno;
    }

    printk("connecting STCP2 proxy to %s:%s, fd=%d\n",
           PROXY_HOST, PROXY_PORT, fd);

    ret = zsock_connect(fd, (struct sockaddr *)&remote, sizeof(remote));
    if (ret < 0) {
        ret = -errno;
        printk("proxy connect failed: errno=%d\n", errno);
        zsock_close(fd);
        return ret;
    }

    printk("STCP2 proxy tunnel connected to %s:%s\n",
           PROXY_HOST, PROXY_PORT);
    return fd;
}

static int proxy_session(int fd)
{
    static const char hello[] =
        "STCP2 PROXY ONLINE proto=raw future=coap,mqtt\n";
    uint8_t rx[RX_BUFFER_SIZE + 1];
    int ret;

    ret = send_all(fd, hello, sizeof(hello) - 1);
    if (ret < 0) {
        printk("proxy hello send failed: %d\n", ret);
        return ret;
    }

    for (;;) {
        ssize_t received = zsock_recv(fd, rx, RX_BUFFER_SIZE, 0);
        if (received < 0) {
            printk("proxy recv failed: errno=%d\n", errno);
            return -errno;
        }
        if (received == 0) {
            printk("proxy peer closed connection\n");
            return -ECONNRESET;
        }

        rx[received] = '\0';
        printk("proxy rx %d bytes: %s\n", (int)received, rx);

        /* Raw test mode: echo received payload back once.
         * Replace this later with CoAP/MQTT channel dispatch.
         */
        ret = send_all(fd, rx, (size_t)received);
        if (ret < 0) {
            printk("proxy echo send failed: %d\n", ret);
            return ret;
        }
    }
}

int main(void)
{
    int ret;

    printk("STCP2 internet proxy client starting\n");
    printk("remote=%s:%s\n", PROXY_HOST, PROXY_PORT);

    ret = stcp_lte_init_and_connect(300);
    if (ret < 0) {
        printk("LTE/PDN/IP initialization failed: %d\n", ret);
        (void)stcp_lte_dump_status();
        return 0;
    }

    printk("LTE/PDN/IP ready\n");

    for (;;) {
        int fd = connect_proxy();
        if (fd < 0) {
            printk("retrying proxy connection in 10 seconds\n");
            k_sleep(RECONNECT_DELAY);
            continue;
        }

        (void)proxy_session(fd);
        (void)zsock_shutdown(fd, ZSOCK_SHUT_RDWR);
        (void)zsock_close(fd);

        printk("proxy tunnel disconnected; reconnecting in 10 seconds\n");
        k_sleep(RECONNECT_DELAY);
    }
}
