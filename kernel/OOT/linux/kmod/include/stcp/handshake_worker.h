#pragma once

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/net.h>
#include <linux/errno.h>

#include <net/sock.h>
#include <net/inet_common.h>
#include <net/inet_connection_sock.h>
#include <net/protocol.h>
#include <net/tcp.h>
#include <linux/inet.h>

#include <stcp/debug.h>
#include <stcp/proto_layer.h>   // Rust proto_ops API

#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/proto_operations.h>

#define HS_PUMP_REASON_MANUAL         1
#define HS_PUMP_REASON_DATA_READY     2
#define HS_PUMP_REASON_NEXT_STEP      3
#define HS_PUMP_REASON_COMPLETE      255



int stcp_queue_work_for_stcp_hanshake(struct stcp_sock *st, unsigned int delayMS, int reason);
int stcp_work_queue_init(void);
void stcp_handshake_worker(struct work_struct *work);
int destroy_the_work_queue(struct stcp_sock *st);
void stcp_handshake_start(struct stcp_sock *st, int server_side);