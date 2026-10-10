#include <stdio.h>
#include <stdarg.h>
struct shell {int dummy;};
static inline void shell_print(const struct shell*s,const char*f,...) {(void)s;(void)f;}
static inline void shell_error(const struct shell*s,const char*f,...) {(void)s;(void)f;}
#define snprintk snprintf
#define SHELL_CMD(...) 0
#define SHELL_CMD_ARG(...) 0
#define SHELL_STATIC_SUBCMD_SET_CREATE(name,...) static int name[]={__VA_ARGS__}
#define SHELL_SUBCMD_SET_END 0
#define SHELL_CMD_REGISTER(...)
