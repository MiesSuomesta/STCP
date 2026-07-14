#![no_std]

extern crate alloc;

mod allocator;
mod error;
mod ffi;
mod spinlock;
mod state;
mod transport;

pub use error::StcpError;
pub use state::StcpContext;
