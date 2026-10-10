#include <stdarg.h>
#define LOG_LEVEL_INF 3
#define LOG_MODULE_REGISTER(...)
static inline void host_log(const char *format,...) {(void)format;}
#define LOG_INF(...) host_log(__VA_ARGS__)
#define LOG_ERR(...) host_log(__VA_ARGS__)
#define LOG_WRN(...) host_log(__VA_ARGS__)
#define LOG_DBG(...) host_log(__VA_ARGS__)
