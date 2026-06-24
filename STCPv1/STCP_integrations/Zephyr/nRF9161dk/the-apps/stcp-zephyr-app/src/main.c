#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/init.h>
#include <modem/lte_lc.h>

#include <zephyr/sys/heap_listener.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/kernel.h>

#include <stcp_api.h>
#include <stcp/stcp_socket.h>

#define SERVER_IP "lja.fi"
#define SERVER_PORT 7777

void main(void)
{

    int rc = stcp_library_init();
    printk("STCP library init: %d, errno: %d\n", rc, errno);

    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    int fd = stcp_socket(AF_INET, SOCK_STREAM, 253);
    if (fd < 0) {
        printk("Socket failed: %d, %d\n", fd, errno);
        return;
    }

    if (stcp_connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printk("Connect failed\n");
        return;
    }

    printk("Said hello\n");
    stcp_send(fd, "hello", 5, 0);

    char buf[64];
    stcp_recv(fd, buf, sizeof(buf), 0);
    printk("Recvd: %s\n", buf);

    stcp_close(fd);
}