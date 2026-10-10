#ifndef STCP_BUFFER_POOL_H
#define STCP_BUFFER_POOL_H
#include <stddef.h>
#include <stdbool.h>
/* Nonblocking: NULL means the bounded pool is full or request too large. */
void *stcp_buffer_alloc(size_t size);
bool stcp_buffer_free(void *ptr);
void stcp_buffer_pool_report(void);
#endif
