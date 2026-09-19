//! Lightweight STCP kernel hot-path benchmark instrumentation.
//!
//! `start()` records the Rust caller location automatically. `stop()` records
//! the elapsed time into a small C-side accumulator. `check()` dumps the
//! accumulated call-site statistics. No allocation is performed on the hot
//! path and no printk is emitted by start/stop.

use core::panic::Location;

unsafe extern "C" {
    fn stcp_kernel_benchmark_now_ns() -> u64;
    fn stcp_kernel_benchmark_record(
        file: *const u8,
        file_len: usize,
        line: u32,
        column: u32,
        elapsed_ns: u64,
    );
    fn stcp_kernel_benchmark_check();
}

#[derive(Clone, Copy, Debug)]
pub struct Benchmark {
    start_ns: u64,
    file: &'static str,
    line: u32,
    column: u32,
}

#[derive(Clone, Copy, Debug)]
pub struct BenchmarkResult {
    pub file: &'static str,
    pub line: u32,
    pub column: u32,
    pub start_ns: u64,
    pub stop_ns: u64,
    pub elapsed_ns: u64,
}

#[track_caller]
#[inline(always)]
pub fn start() -> Benchmark {
    let caller = Location::caller();
    Benchmark {
        start_ns: unsafe { stcp_kernel_benchmark_now_ns() },
        file: caller.file(),
        line: caller.line(),
        column: caller.column(),
    }
}

#[inline(always)]
pub fn stop(benchmark: Benchmark) -> BenchmarkResult {
    let stop_ns = unsafe { stcp_kernel_benchmark_now_ns() };
    let elapsed_ns = stop_ns.saturating_sub(benchmark.start_ns);

    unsafe {
        stcp_kernel_benchmark_record(
            benchmark.file.as_ptr(),
            benchmark.file.len(),
            benchmark.line,
            benchmark.column,
            elapsed_ns,
        );
    }

    BenchmarkResult {
        file: benchmark.file,
        line: benchmark.line,
        column: benchmark.column,
        start_ns: benchmark.start_ns,
        stop_ns,
        elapsed_ns,
    }
}

/// Dump all accumulated benchmark call sites to the kernel log.
#[inline]
pub fn check() {
    unsafe { stcp_kernel_benchmark_check() };
}
