#include <stdint.h>
#include <time.h>

#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/debug.h>



#define STCP_TORTURE_BUF_SIZE   (4*1024)
#define STCP_TORTURE_WORKERS    1
#define STCP_TORTURE_STACK      (12*1024)
#define STCP_TORTURE_PRIO       6


extern struct zsock_addrinfo *the_test_server_addr_resolved;

void tcp_torture_worker(void *p1, void *p2, void *p3);
void stcp_torture_worker(void *p1, void *p2, void *p3);
void mqtt_torture_worker(void *p1, void *p2, void *p3);

int generate_strftime_payload(uint8_t *buf, size_t len);
int stcp_testing_connect_peer(struct stcp_api *api, struct stcp_ctx *ctx);
int stcp_testing_get_peer_socket(struct stcp_api **apiTo);


