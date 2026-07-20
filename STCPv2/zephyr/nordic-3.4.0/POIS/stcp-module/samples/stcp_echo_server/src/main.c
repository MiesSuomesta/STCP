#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/printk.h>

#include <stcp/stcp.h>
#include <stcp/stcp_lte.h>

static int send_all(int fd, const uint8_t *data, size_t length)
{
    size_t sent_total = 0;

    while (sent_total < length) {
        ssize_t sent = zsock_send(fd,
                                  data + sent_total,
                                  length - sent_total,
                                  0);
        if (sent < 0) {
            return -errno;
        }
        if (sent == 0) {
            return -EPIPE;
        }
        sent_total += (size_t)sent;
    }

    return 0;
}

static void serve_client(int client_fd, const struct sockaddr_in *peer)
{
    static uint8_t buffer[CONFIG_STCP_ECHO_BUFFER_SIZE];
    char peer_addr[NET_IPV4_ADDR_LEN] = {0};

    if (peer != NULL) {
        (void)zsock_inet_ntop(AF_INET, &peer->sin_addr,
                              peer_addr, sizeof(peer_addr));
        printk("client connected: %s:%u fd=%d\n",
               peer_addr[0] != '\0' ? peer_addr : "?",
               ntohs(peer->sin_port), client_fd);
    } else {
        printk("client connected: fd=%d\n", client_fd);
    }

    for (;;) {
        ssize_t received = zsock_recv(client_fd, buffer, sizeof(buffer), 0);

        if (received == 0) {
            printk("client closed: fd=%d\n", client_fd);
            break;
        }

        if (received < 0) {
            int saved_errno = errno;
            if (saved_errno == EINTR) {
                continue;
            }
            printk("recv failed: fd=%d errno=%d\n",
                   client_fd, saved_errno);
            break;
        }

        printk("rx: fd=%d bytes=%d\n", client_fd, (int)received);

        int ret = send_all(client_fd, buffer, (size_t)received);
        if (ret < 0) {
            printk("send failed: fd=%d errno=%d\n",
                   client_fd, -ret);
            break;
        }

        printk("echoed: fd=%d bytes=%d\n", client_fd, (int)received);
    }

    (void)zsock_shutdown(client_fd, ZSOCK_SHUT_RDWR);
    (void)zsock_close(client_fd);
}

int main(void)
{
    struct sockaddr_in local = {
        .sin_family = AF_INET,
        .sin_port = htons(CONFIG_STCP_ECHO_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };
    int listener_fd;
    int ret;

    printk("STCP echo server starting\n");

    ret = stcp_lte_init_and_connect(CONFIG_STCP_ECHO_LTE_TIMEOUT_SECONDS);
    if (ret < 0) {
        printk("LTE startup failed: %d\n", ret);
        return 0;
    }

    printk("LTE/PDN/IP ready\n");

    listener_fd = zsock_socket(AF_STCP, SOCK_STREAM, STCP_PROTO_TCP);
    if (listener_fd < 0) {
        printk("socket failed: errno=%d\n", errno);
        return 0;
    }

    printk("socket OK, listener fd=%d\n", listener_fd);

    ret = zsock_bind(listener_fd,
                     (const struct sockaddr *)&local,
                     sizeof(local));
    if (ret < 0) {
        printk("bind failed: errno=%d\n", errno);
        (void)zsock_close(listener_fd);
        return 0;
    }

    printk("bind OK, port=%d\n", CONFIG_STCP_ECHO_PORT);

    ret = zsock_listen(listener_fd, CONFIG_STCP_ECHO_BACKLOG);
    if (ret < 0) {
        printk("listen failed: errno=%d\n", errno);
        (void)zsock_close(listener_fd);
        return 0;
    }

    printk("listening, backlog=%d\n", CONFIG_STCP_ECHO_BACKLOG);

    for (;;) {
        struct sockaddr_in peer = {0};
        socklen_t peer_len = sizeof(peer);

        printk("waiting for STCP client...\n");

        int client_fd = zsock_accept(listener_fd,
                                     (struct sockaddr *)&peer,
                                     &peer_len);
        if (client_fd < 0) {
            int saved_errno = errno;
            if (saved_errno == EINTR || saved_errno == EAGAIN) {
                continue;
            }
            printk("accept failed: errno=%d\n", saved_errno);
            k_sleep(K_SECONDS(1));
            continue;
        }

        serve_client(client_fd, &peer);
    }

    return 0;
}
