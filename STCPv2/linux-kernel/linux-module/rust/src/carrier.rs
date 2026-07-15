use alloc::{
    collections::VecDeque,
    sync::Arc,
};

use core::ffi::c_void;

use crate::{
    spinlock::SpinLock,
    state::{Connection, Side},
};

unsafe extern "C" {
    fn stcp_kernel_should_drop_data() -> bool;
    fn stcp_kernel_wake_accept(owner: *mut c_void);
    fn stcp_kernel_wake_recv(owner: *mut c_void);
}

pub(crate) fn wake_accept(owner: usize) {
    if owner != 0 {
        unsafe {
            stcp_kernel_wake_accept(owner as *mut c_void);
        }
    }
}

pub(crate) fn wake_recv(owner: usize) {
    if owner != 0 {
        unsafe {
            stcp_kernel_wake_recv(owner as *mut c_void);
        }
    }
}

pub(crate) fn outgoing_queue(
    shared: &Arc<Connection>,
    side: Side,
) -> &SpinLock<VecDeque<u8>> {
    match side {
        Side::A => &shared.a_to_b,
        Side::B => &shared.b_to_a,
    }
}

pub(crate) fn incoming_queue(
    shared: &Arc<Connection>,
    side: Side,
) -> &SpinLock<VecDeque<u8>> {
    match side {
        Side::A => &shared.b_to_a,
        Side::B => &shared.a_to_b,
    }
}


pub(crate) fn transmit(
    shared: &Arc<Connection>,
    side: Side,
    bytes: &[u8],
    may_drop_for_test: bool,
) {
    if may_drop_for_test {
        let drop_frame = unsafe {
            stcp_kernel_should_drop_data()
        };

        if drop_frame {
            return;
        }
    }

    outgoing_queue(shared, side)
        .lock()
        .extend(bytes.iter().copied());

    wake_recv(shared.peer_owner(side));
}
