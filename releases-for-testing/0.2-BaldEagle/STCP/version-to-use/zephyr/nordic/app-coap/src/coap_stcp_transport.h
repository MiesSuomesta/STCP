#ifndef COAP_STCP_TRANSPORT_H
#define COAP_STCP_TRANSPORT_H

#include <stddef.h>
#include <stdint.h>

int coap_stcp_connect(void);
void coap_stcp_close(void);
int coap_stcp_send(const uint8_t *data, size_t len);
int coap_stcp_recv(uint8_t *data, size_t len);
int coap_stcp_fd(void);

#endif
