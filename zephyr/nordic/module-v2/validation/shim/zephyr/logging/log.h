#include <stdio.h>
#define LOG_MODULE_REGISTER(...)
#define LOG_ERR(...) do {fprintf(stderr,__VA_ARGS__);fputc('\n',stderr);}while(0)
