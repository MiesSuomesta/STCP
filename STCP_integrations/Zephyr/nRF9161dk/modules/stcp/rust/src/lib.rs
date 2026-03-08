#![cfg_attr(not(feature = "std"), no_std)]

#[cfg(feature = "std")]
extern crate std;

extern crate alloc;


#[cfg(target_os = "linux")]
pub mod linux_random;

pub mod abi;
pub mod aes;
pub mod crypto;
pub mod debug;
pub mod debug_helpers;
pub mod helpers;
pub mod system_memory_allocator;
pub mod session_handler;
pub mod stcp_handshake;
pub mod stcp_message;
pub mod stcp_protocol_functions;
pub mod slice_helpers;
pub mod tcp_helper_macros;
pub mod tcp_io;
pub mod proto_session;
pub mod settings;
pub mod module_handlers;

#[allow(dead_code)]
mod errorit;

pub mod types;
pub use types::*;

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_alive() -> i32 {
    0x53544350  // "STCP"
}
