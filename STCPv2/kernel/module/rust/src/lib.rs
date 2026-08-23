#![cfg_attr(not(feature = "userspace"), no_std)]

extern crate alloc;

#[cfg(not(feature = "userspace"))]
mod allocator;
mod byte_queue;
mod crypto;
mod error;
mod ffi;
mod kdf;
mod carrier;
mod frame;
mod spinlock;
mod state;
mod session;

pub use error::StcpError;
pub use state::StcpContext;

#[cfg(feature = "userspace")]
pub use ffi::{
    stcp_rust_bind,
    stcp_rust_create,
    stcp_rust_create_external_tcp_child,
    stcp_rust_is_connected,
    stcp_rust_listen,
    stcp_rust_recv,
    stcp_rust_send,
    stcp_rust_set_carrier,
    stcp_rust_start_handshake,
};

#[cfg(feature = "userspace")]
pub use carrier::{
    stcp_rust_carrier_receive,
    stcp_rust_carrier_receive_from,
};

#[cfg(feature = "userspace")]
pub use ffi::{
    stcp_rust_accept,
    stcp_rust_can_send,
    stcp_rust_connect,
    stcp_rust_connection_id,
    stcp_rust_crypto_selftest,
    stcp_rust_exit,
    stcp_rust_get_carrier,
    stcp_rust_get_reliability_stats,
    stcp_rust_has_accept,
    stcp_rust_has_data,
    stcp_rust_init,
    stcp_rust_release,
    stcp_rust_set_owner,
    stcp_rust_shutdown,
    stcp_rust_tick,
};

