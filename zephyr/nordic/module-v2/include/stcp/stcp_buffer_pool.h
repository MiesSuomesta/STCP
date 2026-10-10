#ifndef STCP_BUFFER_POOL_H
#define STCP_BUFFER_POOL_H
#include <stddef.h>
#include <stdbool.h>
/* Nonblocking: NULL means the bounded pool is full or request too large. */
void *stcp_buffer_alloc(size_t size);
bool stcp_buffer_free(void *ptr);
void stcp_buffer_pool_report(void);
/* One serialized, nonblocking TX reservation; release on every return. */
bool stcp_buffer_tx_begin(size_t frame_count);
void stcp_buffer_tx_end(void);
bool stcp_buffer_tx_ready(size_t frame_count);
/* DATA admission leaves parser/decrypt workspace. Control frames bypass it. */
bool stcp_buffer_rx_ready(void);
#endif
