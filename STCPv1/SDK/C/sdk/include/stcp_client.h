/* AUTOGENEROITU, älä muokkaa käsin! */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

void *stcp_client_connect(const char *ip, uint16_t port);

int stcp_client_send(void *client_ptr, const uint8_t *data_ptr, uintptr_t data_len);

int stcp_client_recv(void *client_ptr, uint8_t *out_buf, uintptr_t max_len, uintptr_t *recv_bytes);

void stcp_client_disconnect(void *client_ptr);
