// debug_bridge_linux.c

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define LOG_BUFFER_SIZE_BYTES       (1024*4)
#define STCP_DEFAULT_RUST_LOG_LEVEL 1

// Estetään rekursio (thread-local Linuxissa!)
static __thread int stcp_in_log_call = 0;

__attribute__((used))
__attribute__((noinline))
void stcp_rust_log(int level, const uint8_t *buf, uintptr_t len)
{
    if (stcp_in_log_call) {
        return;
    }

    stcp_in_log_call = 1;

    static const char *lvl[] = {
        "??",   // 0
        "ERR",  // 1
        "WRN",  // 2
        "INF",  // 3
        "DBG",  // 4
        "TRC",  // 5
    };

    char tmp[LOG_BUFFER_SIZE_BYTES];
    size_t n;

    if (!buf || !len) {
        stcp_in_log_call = 0;
        return;
    }

    if (len >= sizeof(tmp))
        n = sizeof(tmp) - 1;
    else
        n = len;

    memcpy(tmp, buf, n);
    tmp[n] = '\0';

    if (level < 0 || level > 5)
        level = STCP_DEFAULT_RUST_LOG_LEVEL;

    // Linux: stdout (tai stderr)
    switch(level) {
        case 1: // ERR
        case 2: // WRN
            fprintf(stderr, "STCP/RUST[%s]: %s\n", lvl[level], tmp);
            break;

        case 3: // INF
        case 4: // DBG
        case 5: // TRC
        default:
            fprintf(stdout, "STCP/RUST[%s]: %s\n", lvl[level], tmp);
            break;
    }

    fflush(stdout);
    fflush(stderr);

    stcp_in_log_call = 0;
}

// Estää dead code eliminationin
__attribute__((used))
void *stcp_rust_log_keep = (void *)&stcp_rust_log;