#pragma once
#include <zephyr/net/socket.h>

// Public FFI from Rust
//extern int rust_exported_session_create(void **out_sess, void *transport);
//extern int rust_exported_session_destroy(void *sess);

//exte
int rust_session_handshake_lte(void *sess, void *transport);

#if 0
extern int rust_session_client_handshake_lte(void *sess, void *transport);
extern int rust_session_server_handshake_lte(void *sess, void *transport);
//
extern ssize_t rust_exported_session_sendmsg(void *sess, void *transport, void *msg, size_t msglen);
extern ssize_t rust_exported_session_recvmsg(void *sess, void *transport, void *buffer, size_t maxlen, int bloking);

extern ssize_t rust_exported_session_sendmsg_iovec(
    const void *sess_void_ptr,
    const void *transport_void_ptr,
    void *msg_void_ptr,
    int flags,
    bool encrypted
);
#endif

//
//// Functions to be clear of the scope ...
//extern int rust_exported_data_client_ready_worker(void *sess, void *transport);
//extern int rust_exported_data_server_ready_worker(void *sess, void *transport);
//extern int rust_exported_session_handshake_done(void *sess);
//extern int rust_exported_session_handshake_pump(void *sess, void *transport /* struct sock* */, int reason);
//
//// Module init / exit
extern void stcp_module_rust_enter (void);
extern void stcp_module_rust_exit (void);
//
//// Instance counter
//extern void stcp_exported_rust_sockets_alive_get(void);
//extern void stcp_exported_rust_sockets_alive_put(void);
//extern int  stcp_exported_rust_ctx_alive_count(void);

// RUST initialised 
//inline int is_rust_init_done(void);
