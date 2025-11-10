#![no_std]
#![feature(c_variadic)]

pub mod system_memory_allocator;

pub mod abi;
pub mod error;
pub mod debug;
pub mod ctx;
pub mod ffi;
pub mod encrypted_io;

use core::ffi::c_void;

/// Yhteinen debug-print-funktio, jota makrot kutsuvat.
///
/// TÄLLÄ HETKELLÄ NOP, jotta ei tarvita `kernel`-cratea eikä FFI:tä.
/// Kun haluat oikeat printk-logit, toteutetaan tämä C-bridgen kautta.
#[inline(always)]
pub fn stcp_trace_print(loc: &str, ptr: *const c_void) {
    let _ = (loc, ptr);
}

