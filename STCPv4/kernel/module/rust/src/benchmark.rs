#![no_std]

//! Minimal STCP profiling/benchmark helper.
//!
//! No allocation, std, locks, or global mutable state.
//! `start()` automatically records its caller location.

use core::panic::Location;

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
pub fn start(now_ns: u64) -> Benchmark {
    let caller = Location::caller();

    Benchmark {
        start_ns: now_ns,
        file: caller.file(),
        line: caller.line(),
        column: caller.column(),
    }
}

#[inline(always)]
pub fn stop(benchmark: Benchmark, now_ns: u64) -> BenchmarkResult {
    BenchmarkResult {
        file: benchmark.file,
        line: benchmark.line,
        column: benchmark.column,
        start_ns: benchmark.start_ns,
        stop_ns: now_ns,
        elapsed_ns: now_ns.saturating_sub(benchmark.start_ns),
    }
}

/// Returns true when elapsed time is at least `limit_ns`.
#[inline(always)]
pub fn check(result: &BenchmarkResult, limit_ns: u64) -> bool {
    result.elapsed_ns >= limit_ns
}
