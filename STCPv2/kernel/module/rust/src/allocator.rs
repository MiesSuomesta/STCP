use core::{
    alloc::{GlobalAlloc, Layout},
    ffi::c_void,
    ptr,
};

unsafe extern "C" {
    fn stcp_rust_kernel_alloc(size: usize) -> *mut c_void;
    fn stcp_rust_kernel_free(ptr: *mut c_void);
}

struct KernelAllocator;

unsafe impl GlobalAlloc for KernelAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        let ptr = unsafe {
            stcp_rust_kernel_alloc(layout.size().max(1))
        };

        ptr.cast()
    }

    unsafe fn alloc_zeroed(&self, layout: Layout) -> *mut u8 {
        let ptr = unsafe { self.alloc(layout) };

        if !ptr.is_null() {
            unsafe {
                ptr::write_bytes(ptr, 0, layout.size());
            }
        }

        ptr
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        if !ptr.is_null() {
            unsafe {
                stcp_rust_kernel_free(ptr.cast());
            }
        }
    }
}

#[global_allocator]
static GLOBAL_ALLOCATOR: KernelAllocator = KernelAllocator;

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo<'_>) -> ! {
    loop {
        core::hint::spin_loop();
    }
}
