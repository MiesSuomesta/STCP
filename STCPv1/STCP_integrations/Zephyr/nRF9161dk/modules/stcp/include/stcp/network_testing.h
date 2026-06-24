#pragma once 
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/init.h>
#include <modem/lte_lc.h>

#include <zephyr/sys/heap_listener.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/kernel.h>

#include <zephyr/net/socket.h>
#include <zephyr/net/net_ip.h>
#include <errno.h>
#include <zephyr/sys/atomic.h>
#include <modem/nrf_modem_lib.h>

#include "stcp_struct.h"
#include "stcp_net.h"
#include "workers.h"
#include "utils.h"
#include "fsm.h"
#include "utils.h"
#include "stcp_operations_zephyr.h"

#include "stcp_rust_exported_functions.h"

int stcp_network_test_dns(const char *host, const char *port,
                               struct sockaddr_in *out);

int stcp_network_ping_ip(const struct sockaddr_in *addr);

int stcp_network_test_tcp_connect(const struct sockaddr_in *addr);

int stcp_network_test_network_avalability(const char *host, const char *port);
