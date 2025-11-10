#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

StcpConnection *stcp_internal_connect(const char *addr);

int32_t stcp_internal_send(StcpConnection *conn, const uint8_t *data, uintptr_t len);

uintptr_t stcp_internal_recv(StcpConnection *conn, uint8_t *buf, uintptr_t max_len);

void stcp_internal_disconnect(StcpConnection *conn);
