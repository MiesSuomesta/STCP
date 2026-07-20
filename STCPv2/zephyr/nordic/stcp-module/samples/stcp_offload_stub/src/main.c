#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp.h>

int main(void)
{
    struct sockaddr_stcp peer = {
        .stcp_family = AF_STCP,
        .stcp_port = 253,
        .stcp_addr = { 0x01 },
    };
    const char message[] = "STCP Zephyr socket-offload stub";
    int fd;
    ssize_t sent;

    printk("STCP offload stub sample\n");
    fd = zsock_socket(AF_STCP, SOCK_STREAM, STCP_PROTO);
    if (fd < 0) {
        printk("socket failed: errno=%d\n", errno);
        return 0;
    }
    printk("socket fd=%d\n", fd);
    if (zsock_connect(fd, (struct sockaddr *)&peer, sizeof(peer)) < 0) {
        printk("connect failed: errno=%d\n", errno);
        zsock_close(fd);
        return 0;
    }
    sent = zsock_send(fd, message, strlen(message), 0);
    printk("send returned %d, errno=%d\n", (int)sent, errno);
    zsock_close(fd);
    printk("stub test complete\n");
    return 0;
}
