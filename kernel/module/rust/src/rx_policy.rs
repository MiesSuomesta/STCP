use core::sync::atomic::{AtomicUsize, Ordering};
use crate::{byte_queue::{RX_WIRE_BUDGET, RX_APP_BUDGET, RX_REORDER_BUDGET, RX_CHUNK_LIMIT, RX_FRAME_DATA_LIMIT}, error::StcpError};

// Conservative admission reservations, not eager allocations or an allocator
// replacement. Covers both connection wire directions, app/reorder buffers,
// parser/decompression temporaries and a metadata allowance. TX and C socket
// buffers are outside this RX policy. Reserving upfront avoids hot-path atomics.
pub const SESSION_RX_RESERVATION: usize = 2 * RX_WIRE_BUDGET + RX_APP_BUDGET
    + RX_REORDER_BUDGET + 2 * (RX_FRAME_DATA_LIMIT + 64)
    + 3 * RX_CHUNK_LIMIT * 8 * core::mem::size_of::<usize>() + 4096;
pub const GLOBAL_RX_RESERVATIONS: usize = 2 * 1024 * 1024 * 1024;
pub const PARTIAL_FRAME_TIMEOUT_MS: u64 = 300_000;
static RESERVED: AtomicUsize = AtomicUsize::new(0);

pub(crate) struct SessionLease;
impl SessionLease {
    pub(crate) fn acquire() -> Result<Self, StcpError> {
        let mut current = RESERVED.load(Ordering::Acquire);
        loop {
            let next = current.checked_add(SESSION_RX_RESERVATION).ok_or(StcpError::NoMem)?;
            if next > GLOBAL_RX_RESERVATIONS { return Err(StcpError::NoMem); }
            match RESERVED.compare_exchange_weak(current,next,Ordering::AcqRel,Ordering::Acquire) {
                Ok(_) => return Ok(Self), Err(observed) => current=observed,
            }
        }
    }
}
impl Drop for SessionLease {
    fn drop(&mut self) { RESERVED.fetch_sub(SESSION_RX_RESERVATION,Ordering::AcqRel); }
}
#[cfg(test)]
pub(crate) fn reserved() -> usize { RESERVED.load(Ordering::Acquire) }
unsafe extern "C" { fn stcp_kernel_monotonic_ms() -> u64; }
pub(crate) fn now_ms() -> u64 { unsafe { stcp_kernel_monotonic_ms() } }
