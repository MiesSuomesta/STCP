#include <stdio.h>
#include <string.h>
#include "stcp_types.h"
#include "stcp_client.h"

int main(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        printf("%s v%d.%d.%d\n",
            STCP_PROJECT_NAME,
            STCP_VERSION_MAJOR,
            STCP_VERSION_MINOR,
            STCP_VERSION_PATCH);
        return 0;
    }

    printf("STCP-demo-client käynnistyy....\n");
    stcp_client_start();
    printf("STCP-demo-client käynnissä\n");
    return 0;
}
