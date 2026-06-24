#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "stcp_client.h"

int main() {
    printf("🟢 Starting up STCP C Client...\n");

    long unsigned int recv_bytes = 0;

    const char *server_addr = "127.0.0.1";
    void *conn = stcp_client_connect(server_addr, 5555);
    if (!conn) {
        printf("❌ Connection failed!\n");
        return 1;
    }

    int counter = 0;
    while (1) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Message #%d from C-client", counter++);
        printf("⟶ Sendgin: %s\n", msg);

        if (stcp_client_send(conn, msg, strlen(msg)) < 0) {
            printf("❌ send message error!\n");
            break;
        }

        uint8_t buf[1024];
        int rc = stcp_client_recv(conn, buf, sizeof(buf), &recv_bytes);
        if (rc == 0) {
            printf("⬅ Received: %.*s\n", (int)recv_bytes, buf);
        } else {
            printf("⚠ Ei response or error: %d\n", rc);
        }

        sleep(1);
    }

    stcp_client_disconnect(conn);
    return 0;
}
