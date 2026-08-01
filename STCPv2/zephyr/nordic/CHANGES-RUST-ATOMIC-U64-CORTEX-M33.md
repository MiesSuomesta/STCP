# Rust STCP core: Cortex-M33 atomic compatibility

The nRF9151 target `thumbv8m.main-none-eabi` does not expose 64-bit atomic
operations, so `core::sync::atomic::AtomicU64` is unavailable.

This patch keeps the Linux/Raspberry Pi build unchanged with `AtomicU64` and
uses `AtomicU32` on targets without 64-bit atomics. The generated connection ID
is widened to the existing `u64` wire field, preserving the STCP frame format.
Zero remains reserved and is skipped after counter wraparound.

No wire-format changes are introduced.
