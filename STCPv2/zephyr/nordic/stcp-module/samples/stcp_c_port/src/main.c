#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp.h>

int main(void)
{
    int fd = zsock_socket(AF_STCP, SOCK_STREAM, STCP_PROTO_TCP);

    printk("STCP Zephyr C port\n");
    if (fd < 0) {
        printk("socket(AF_STCP) failed: errno=%d\n", errno);
        return 0;
    }

    printk("socket(AF_STCP) OK, fd=%d\n", fd);
    zsock_close(fd);
    printk("close OK\n");
    return 0;
}
