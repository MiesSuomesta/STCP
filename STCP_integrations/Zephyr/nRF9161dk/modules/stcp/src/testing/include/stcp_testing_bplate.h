
// Muista määritellä filekseen LOGTAG
#define TDBG(msg, ...)  LDBG(LOGTAG msg, ##__VA_ARGS__)
#define TWRN(msg, ...)  LWRN(LOGTAG msg, ##__VA_ARGS__)
#define TINF(msg, ...)  LINF(LOGTAG msg, ##__VA_ARGS__)
#define TERR(msg, ...)  LERR(LOGTAG msg, ##__VA_ARGS__)

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

