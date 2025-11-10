#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "stcp_random.h"
#include "stcp_types.h"
#include "stcp_server.h"

void process_message_callback(const uint8_t *input, long unsigned int input_len,
                              uint8_t *output_buf, long unsigned int max_output_len,
                              long unsigned int *actual_output_len) {
    printf("📝 [C SERVER] Message received: %.*s\n", (int)input_len, input);

    const char *reply = "[C SERVER] message received!";
    long unsigned int reply_len = strlen(reply);
    if (reply_len > max_output_len) reply_len = max_output_len;

    memcpy(output_buf, reply, reply_len);
    *actual_output_len = reply_len;
}

int main() {
    printf("🟢 Käynnistetään STCP C Server...\n");

    const char *bind_ip = "0.0.0.0";
    uint16_t port = 5555;

    void *server = stcp_server_bind((const uint8_t*)bind_ip, strlen(bind_ip), port, process_message_callback);
    if (!server) {
        printf("❌ Bind failed.\n");
        return 1;
    }

    printf("🪝 Callback registered. Starting listening...\n");
    stcp_server_listen(server);

    printf("🛑 Server stopped. doign Cleanup.\n");
    stcp_server_stop(server);

    return 0;
}
