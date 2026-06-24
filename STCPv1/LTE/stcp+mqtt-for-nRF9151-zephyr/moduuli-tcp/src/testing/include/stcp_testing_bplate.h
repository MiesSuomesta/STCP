#include <zephyr/kernel.h>

// Muista määritellä filekseen LOGTAG
#if CONFIG_STCP_TESTING_LOG_COMPLETELY_OFF
#  define STCP_TEST_LOG_MACRO(...)
#else
#  if CONFIG_STCP_TESTING_LOG
#    define STCP_TEST_LOG_MACRO(...) printk(__VA_ARGS__)
#  else
#     define STCP_TEST_LOG_MACRO(...) LDBG(__VA_ARGS__)
#  endif
#endif

#define TDBG(msg, ...)  STCP_TEST_LOG_MACRO(LOGTAG msg "\n", ##__VA_ARGS__)
#define TWRN(msg, ...)  STCP_TEST_LOG_MACRO(LOGTAG msg "\n", ##__VA_ARGS__)
#define TINF(msg, ...)  STCP_TEST_LOG_MACRO(LOGTAG msg "\n", ##__VA_ARGS__)
#define TERR(msg, ...)  STCP_TEST_LOG_MACRO(LOGTAG msg "\n", ##__VA_ARGS__)


#define TDBGBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(TDBG, LOGTAG, ##__VA_ARGS__)
#define TWRNBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(TWRN, LOGTAG, ##__VA_ARGS__)
#define TINFBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(TINF, LOGTAG, ##__VA_ARGS__)
#define TERRBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(TERR, LOGTAG, ##__VA_ARGS__)
#define TENTER() TDBG("ENTER")
#define TEXIT()  TDBG("EXIT")

typedef enum {
    TEST_MODE_IDLE = 0,
    TEST_MODE_TCP_MAX_THROUGHPUT,
    TEST_MODE_STCP_MAX_THROUGHPUT,
    TEST_MODE_MQTT_MAX_THROUGHPUT
} test_mode_t;


// Headerissä testi entrypointit jne.
void stcp_echo_server_start(void);
void stcp_test_server_start(void);

