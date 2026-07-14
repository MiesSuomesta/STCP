#![no_std]
extern crate alloc;
mod error;
mod ffi;
mod state;
mod transport;
pub use error::StcpError;
pub use state::StcpContext;

mod system_memory_allocator;
