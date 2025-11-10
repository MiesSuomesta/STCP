#![allow(dead_code)]
#![allow(improper_ctypes)]

use core::ffi::c_void;
use core::ffi::c_char;

extern "C" {
    // Context functions
    pub fn stcp_rust_blob_get(sk: *mut c_void) -> *mut c_void;
    pub fn stcp_rust_blob_set(sk: *mut c_void, ctx: *mut c_void);
    pub fn stcp_rust_blob_lock(sk: *mut c_void);
    pub fn stcp_rust_blob_unlock(sk: *mut c_void);

    // Memory allocation
    pub fn stcp_rust_kernel_alloc(size: usize) -> *mut c_void;
    pub fn stcp_rust_kernel_free(ptr: *mut c_void);

    // printk
    pub fn stcp_rust_kernel_printk(fmt: *const c_char, ...);
}
