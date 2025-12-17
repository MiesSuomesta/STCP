#pragma once
#include <linux/net.h>
#include <linux/socket.h>
#include <linux/inet.h>
#include <linux/tcp.h>
#include <linux/slab.h>

#include <stcp/proto_layer.h>   // Rust proto_ops API
#include <stcp/stcp_socket_struct.h>

// Public FFI from Rust
extern int rust_exported_session_create(void **out_sess, void *transport);
extern int rust_exported_session_destroy(void *sess);
extern int rust_exported_session_client_handshake(void *sess);
extern int rust_exported_session_server_handshake(void *sess);

extern ssize_t rust_exported_session_sendmsg(void *sess, void *transport, void *msg, size_t msglen);
extern ssize_t rust_exported_session_recvmsg(void *sess, void *transport, void *buffer, size_t maxlen, int bloking);

// Functions to be clear of the scope ...
extern int rust_exported_data_client_ready_worker(void *sess, void *transport);
extern int rust_exported_data_server_ready_worker(void *sess, void *transport);
extern int rust_exported_session_handshake_done(void *sess);
extern int rust_exported_session_handshake_pump(void *sess, void *transport /* struct sock* */, int reason);

// Module init / exit
extern void stcp_module_rust_enter (void);
extern void stcp_module_rust_exit (void);

// Instance counter
extern void stcp_exported_rust_sockets_alive_get(void);
extern void stcp_exported_rust_sockets_alive_put(void);
extern int  stcp_exported_rust_ctx_alive_count(void);

// RUST initialised 
inline int is_rust_init_done(void);