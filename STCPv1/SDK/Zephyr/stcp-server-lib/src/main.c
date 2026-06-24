#include <stdio.h>
#include <string.h>

#include "stcp_random.h"
#include "stcp_types.h"
#include "stcp_server.h"

void message_processing_callback(const uint8_t *input, long unsigned int input_len,
                      uint8_t *output_buf, long unsigned int max_output_len,
                      long unsigned int *actual_output_len) {
    printf("🧠 C-callback received: %.*s\n", (int)input_len, input);
    const char *response = "✅ C server had your message!";
    long unsigned int response_len = strlen(response);
    if (response_len < max_output_len) {
        memcpy(output_buf, response, response_len);
        *actual_output_len = response_len;
    } else {
        *actual_output_len = 0; // buffer liian pieni
    }
}

int main(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        printf("%s v%d.%d.%d\n",
            STCP_PROJECT_NAME,
            STCP_VERSION_MAJOR,
            STCP_VERSION_MINOR,
            STCP_VERSION_PATCH);
        return 0;
    }

    printf("STCP-demo-server käynnistyy....\n");
    stcp_server_register_callback(message_processing_callback);
    stcp_server_start();
    printf("STCP-demo-server käynnissä\n");
    return 0;
}
