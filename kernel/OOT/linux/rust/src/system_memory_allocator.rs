#![allow(dead_code)]

use core::alloc::{GlobalAlloc, Layout};
use core::ptr::null_mut;
use core::ffi::c_void;
use core::panic::PanicInfo;
use crate::abi::{stcp_rust_kernel_alloc, stcp_rust_kernel_free};

// Globaalin allokaattorin toteutus kernelille
struct KernelAlloc;

unsafe impl GlobalAlloc for KernelAlloc {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        let size = if layout.size() == 0 { 1 } else { layout.size() };
        let ptr = unsafe { stcp_rust_kernel_alloc(size) } as *mut u8;

        if ptr.is_null() {
            null_mut()
        } else {
            ptr
        }
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        if !ptr.is_null() {
            unsafe { stcp_rust_kernel_free(ptr as *mut c_void) };
        }
    }
}

#[global_allocator]
static GLOBAL_ALLOC: KernelAlloc = KernelAlloc;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

