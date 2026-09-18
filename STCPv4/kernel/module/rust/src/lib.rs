#![cfg_attr(not(feature = "userspace"), no_std)]

extern crate alloc;

#[cfg(not(feature = "userspace"))]
mod allocator;

pub mod benchmark;
mod byte_queue;
mod crypto;
mod error;
mod ffi;
mod carrier;
mod compression;
mod frame;
mod kdf;
mod spinlock;
mod state;
mod session;
mod p2p;

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
    stcp_rust_release,
    stcp_rust_send,
    stcp_rust_set_carrier,
    stcp_rust_start_handshake,
};
