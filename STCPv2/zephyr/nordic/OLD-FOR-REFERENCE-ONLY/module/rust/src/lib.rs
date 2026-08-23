#![no_std]

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
    stcp_rust_create_accepted_stream,
    stcp_rust_is_connected,
    stcp_rust_recv,
    stcp_rust_release,
    stcp_rust_send,
    stcp_rust_start_handshake,
};

#[cfg(feature = "userspace")]
pub use carrier::stcp_rust_carrier_receive;
