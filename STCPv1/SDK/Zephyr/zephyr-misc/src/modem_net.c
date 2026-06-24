// zephyrmisc/modem_net.c
#include <nrf_socket.h>
#include "stcp_types.h"

int stcp_modem_socket(int family, int type, int proto) {
    return nrf_socket(family, type, proto);
}

int stcp_modem_connect(int sock, const struct nrf_sockaddr *addr, nrf_socklen_t addrlen) {
    return nrf_connect(sock, addr, addrlen);
}

int stcp_modem_send(int sock, const void *buf, size_t len, int flags) {
    return nrf_send(sock, buf, len, flags);
}

int stcp_modem_recv(int sock, void *buf, size_t maxlen, int flags) {
    return nrf_recv(sock, buf, maxlen, flags);
}

int stcp_modem_close(int sock) {
    return nrf_close(sock);
}

int stcp_modem_bind(int sock, const struct nrf_sockaddr *addr, nrf_socklen_t addrlen) {
    return nrf_bind(sock, addr, addrlen);
}

int stcp_modem_listen(int sock, int backlog) {
    return nrf_listen(sock, backlog);
}

int stcp_modem_accept(int sock, struct nrf_sockaddr *addr, nrf_socklen_t *addrlen) {
    return nrf_accept(sock, addr, addrlen);
}
