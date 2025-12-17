#pragma once
// stcp_prot_impl.c
#include <net/tcp.h>
#include <net/sock.h>
#include <linux/errno.h>

#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/handshake_worker.h>
#include <stcp/tcp_callbacks.h>

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

// Ota talteen TCP:n originaalit

int stcp_proto_setup(void);
int is_stcp_magic_ok(struct stcp_sock* st);
