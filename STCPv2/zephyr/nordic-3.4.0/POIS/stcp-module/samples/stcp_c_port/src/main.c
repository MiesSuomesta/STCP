#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp.h>

#include <stcp/stcp_lte.h>

static int send_all(int fd, const uint8_t *data, size_t len)
{
    size_t offset = 0;

    while (offset < len) {
        ssize_t ret = zsock_send(fd, data + offset, len - offset, 0);
        if (ret < 0) {
            printk("send failed: errno=%d\n", errno);
            return -errno;
        }
        if (ret == 0) {
            printk("send returned zero\n");
            return -EPIPE;
        }
        offset += (size_t)ret;
    }

    return 0;
}

int main(void)
{
    static const char message[] = CONFIG_STCP_TEST_MESSAGE;
    struct sockaddr_in peer = {0};
    uint8_t reply[256];
    int fd;
    int ret;

    printk("STCP Zephyr C carrier test\n");
    printk("remote=%s:%d protocol=%d\n",
           CONFIG_STCP_TEST_REMOTE_IPV4,
           CONFIG_STCP_TEST_REMOTE_PORT,
           STCP_PROTO_TCP);

#if defined(CONFIG_STCP_LTE)
    printk("initializing STCP LTE transport\n");
    ret = stcp_lte_init_and_connect(CONFIG_STCP_LTE_READY_TIMEOUT_SECONDS);
    if (ret != 0) {
        printk("STCP LTE initialization failed: %d\n", ret);
        (void)stcp_lte_dump_status();
        return 0;
    }
    printk("LTE, PDN and IP data path ready\n");
#endif

    fd = zsock_socket(AF_STCP, SOCK_STREAM, STCP_PROTO_TCP);
    if (fd < 0) {
        printk("socket(AF_STCP) failed: errno=%d\n", errno);
        return 0;
    }
    printk("socket OK, fd=%d\n", fd);

    peer.sin_family = AF_INET;
    peer.sin_port = htons(CONFIG_STCP_TEST_REMOTE_PORT);
    ret = zsock_inet_pton(AF_INET, CONFIG_STCP_TEST_REMOTE_IPV4,
                          &peer.sin_addr);
    if (ret != 1) {
        printk("invalid remote IPv4 address\n");
        goto out;
    }

    ret = zsock_connect(fd, (struct sockaddr *)&peer, sizeof(peer));
    if (ret < 0) {
        printk("connect failed: errno=%d\n", errno);
        goto out;
    }
    printk("connect OK\n");

    ret = send_all(fd, (const uint8_t *)message, strlen(message));
    if (ret < 0) goto out;
    printk("send OK, bytes=%u\n", (unsigned int)strlen(message));

    ssize_t received = zsock_recv(fd, reply, sizeof(reply) - 1, 0);
    if (received < 0) {
        printk("recv failed: errno=%d\n", errno);
        goto out;
    }
    if (received == 0) {
        printk("peer closed connection\n");
        goto out;
    }

    reply[received] = '\0';
    printk("recv OK, bytes=%d, data=\"%s\"\n", (int)received, reply);

    if (zsock_shutdown(fd, ZSOCK_SHUT_RDWR) < 0) {
        printk("shutdown failed: errno=%d\n", errno);
    } else {
        printk("shutdown OK\n");
    }

out:
    if (zsock_close(fd) < 0) {
        printk("close failed: errno=%d\n", errno);
    } else {
        printk("close OK\n");
    }
    return 0;
}
