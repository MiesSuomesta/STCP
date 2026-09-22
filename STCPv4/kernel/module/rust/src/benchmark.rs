//! Lightweight STCP kernel benchmarking.
//!
//! Linux/RPi kernel builds use the C-side benchmark accumulator.
//! Bare-metal targets such as Zephyr (`target_os = "none"`) compile
//! the benchmark backend into no-ops.
//!
//! No allocation is performed on the hot path and no printk is emitted
//! by start/stop.

use core::panic::Location;

#[cfg(not(target_os = "none"))]
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

    #[cfg(not(target_os = "none"))]
    let start_ns = unsafe { stcp_kernel_benchmark_now_ns() };

    #[cfg(target_os = "none")]
    let start_ns = 0;

    Benchmark {
        start_ns,
        file: caller.file(),
        line: caller.line(),
        column: caller.column(),
    }
}

#[inline(always)]
pub fn stop(benchmark: Benchmark) -> BenchmarkResult {
    #[cfg(not(target_os = "none"))]
    let stop_ns = unsafe { stcp_kernel_benchmark_now_ns() };

    #[cfg(target_os = "none")]
    let stop_ns = benchmark.start_ns;

    let elapsed_ns = stop_ns.saturating_sub(benchmark.start_ns);

    #[cfg(not(target_os = "none"))]
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
///
/// Bare-metal targets such as Zephyr have no kernel-side benchmark
/// accumulator, so this is intentionally a no-op there.
#[inline]
pub fn check() {
    #[cfg(not(target_os = "none"))]
    unsafe {
        stcp_kernel_benchmark_check();
    }
}
