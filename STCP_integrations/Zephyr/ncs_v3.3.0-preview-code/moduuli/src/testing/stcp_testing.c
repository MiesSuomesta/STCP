
#include "include/stcp_testing.h"

#define LOGTAG     "[STCP/Testing] "
#include <stcp_testing_bplate.h>

#include <stcp_testing_common.h>

void stcp_testing_resume(void) {
#if CONFIG_STCP_TESTING
    TDBG("Torture testin resume, server %s:%d. Torture, mode %d!", 
        CONFIG_STCP_TESTING_PEER_HOSTNAME_TO_CONNECT, 
        CONFIG_STCP_TESTING_PEER_PORT_TO_CONNECT,
        CONFIG_STCP_TESTING_MODE
    );

    stcp_torture_resume();
    TDBG("Torture resumed...");
#endif    
}
