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

// POOL_FLOW_FIX: transient reservations never hold the context spinlock.
unsafe extern "C" {
    fn stcp_buffer_tx_begin(frames: usize) -> bool;
    fn stcp_buffer_tx_end();
    fn stcp_buffer_tx_ready(frames: usize) -> bool;
    fn stcp_buffer_rx_ready() -> bool;
}
pub(crate) struct TxPoolReservation;
impl TxPoolReservation {
    pub(crate) fn acquire(frames: usize) -> Result<Self, crate::error::StcpError> {
        if unsafe { stcp_buffer_tx_begin(frames) } { Ok(Self) }
        else { Err(crate::error::StcpError::Again) }
    }
}
impl Drop for TxPoolReservation {
    fn drop(&mut self) { unsafe { stcp_buffer_tx_end() }; }
}
pub(crate) fn tx_pool_ready(frames: usize) -> bool {
    unsafe { stcp_buffer_tx_ready(frames) }
}
pub(crate) fn rx_pool_ready() -> bool {
    unsafe { stcp_buffer_rx_ready() }
}
