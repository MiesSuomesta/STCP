// dns_helper.c
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

int resolve_domain(const char *domain, char *out_ip, size_t max_len) {
    struct hostent *he = gethostbyname(domain);
    if (!he || !he->h_addr_list[0]) return -1;

    const char *ip = inet_ntoa(*(struct in_addr *)he->h_addr_list[0]);
    strncpy(out_ip, ip, max_len);
    return 0;
}
