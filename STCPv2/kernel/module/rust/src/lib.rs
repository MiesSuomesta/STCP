#![no_std]

extern crate alloc;

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
