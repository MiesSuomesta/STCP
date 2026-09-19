//! Temporary STCPv4 protocol lifecycle profiling.
//! Uses the existing C benchmark accumulator; no printk on hot path.
use core::ffi::c_void;

unsafe extern "C" {
    fn stcp_kernel_benchmark_now_ns() -> u64;
    fn stcp_kernel_benchmark_record(file: *const u8, file_len: usize,
        line: u32, column: u32, elapsed_ns: u64);
}

#[inline(always)]
pub(crate) fn now() -> u64 { unsafe { stcp_kernel_benchmark_now_ns() } }

#[inline(always)]
pub(crate) fn record(label: &'static [u8], start: u64) {
    let elapsed = now().saturating_sub(start);
    unsafe { stcp_kernel_benchmark_record(label.as_ptr(), label.len(), 0, 0, elapsed); }
}

#[inline(always)]
pub(crate) fn value(label: &'static [u8], value: u64) {
    unsafe { stcp_kernel_benchmark_record(label.as_ptr(), label.len(), 0, 0, value); }
}
