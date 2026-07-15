use alloc::{
    collections::VecDeque,
    sync::Arc,
};

use core::ffi::{
    c_int,
    c_void,
};

use crate::{
    error::StcpError,
    spinlock::SpinLock,
    state::{
        Connection,
        Side,
        StcpContext,
    },
};

unsafe extern "C" {
    fn stcp_carrier_needs_reliability(
        carrier: *const c_void,
    ) -> bool;

    fn stcp_carrier_send(
        carrier: *mut c_void,
        data: *const u8,
        len: usize,
        flags: c_int,
    ) -> isize;

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

pub(crate) fn reliability_required(
    carrier: usize,
) -> bool {
    if carrier == 0 {
        return true;
    }

    unsafe {
        stcp_carrier_needs_reliability(
            carrier as *const c_void,
        )
    }
}

pub(crate) fn set_carrier(
    ctx: &StcpContext,
    carrier: usize,
) {
    let mut inner = ctx.inner.lock();
    inner.carrier = carrier;
}

pub(crate) fn transmit(
    shared: &Arc<Connection>,
    side: Side,
    carrier: usize,
    bytes: &[u8],
    flags: c_int,
) -> Result<(), StcpError> {
    if carrier == 0 {
        outgoing_queue(shared, side)
            .lock()
            .extend(bytes.iter().copied());

        wake_recv(shared.peer_owner(side));
        return Ok(());
    }

    let ret = unsafe {
        stcp_carrier_send(
            carrier as *mut c_void,
            bytes.as_ptr(),
            bytes.len(),
            flags,
        )
    };

    if ret < 0 {
        return Err(StcpError::Kernel(ret as i32));
    }

    if ret as usize != bytes.len() {
        return Err(StcpError::Kernel(-5));
    }

    Ok(())
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_carrier_receive(
    raw_ctx: *mut c_void,
    data: *const u8,
    len: usize,
) -> c_int {
    if raw_ctx.is_null() {
        return -22;
    }

    if data.is_null() && len != 0 {
        return -22;
    }

    let ctx = unsafe {
        &*(raw_ctx as *const crate::state::StcpContext)
    };

    let bytes = if len == 0 {
        &[]
    } else {
        unsafe {
            core::slice::from_raw_parts(data, len)
        }
    };

    let (shared, side, owner) = {
        let inner = ctx.inner.lock();

        let Some(endpoint) = &inner.connection else {
            return -107;
        };

        (
            endpoint.shared.clone(),
            endpoint.side,
            inner.owner,
        )
    };

    incoming_queue(&shared, side)
        .lock()
        .extend(bytes.iter().copied());

    wake_recv(owner);
    0
}
