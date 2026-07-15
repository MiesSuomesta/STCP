#![no_std]

extern crate alloc;

mod allocator;
mod crypto;
mod error;
mod ffi;
mod kdf;
mod packet;
mod spinlock;
mod state;
mod transport;

pub use error::StcpError;
pub use state::StcpContext;
