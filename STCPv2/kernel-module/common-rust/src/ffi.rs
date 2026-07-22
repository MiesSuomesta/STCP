use alloc::boxed::Box;

use core::{
    ffi::{c_int, c_void},
    ptr,
    slice,
};

use crate::{
    error::StcpError,
    state::StcpContext,
    session,
};

const EINVAL: c_int = -22;

#[repr(C)]
pub struct StcpReliabilityStats {
    pub srtt_ms: u32,
    pub rttvar_ms: u32,
    pub rto_ms: u32,
    pub sent_frames: u64,
    pub acknowledged_frames: u64,
    pub retransmitted_frames: u64,
    pub duplicate_frames: u64,
    pub reordered_frames: u64,
    pub timeout_failures: u64,
    pub rtt_samples: u64,
}


#[inline]
fn with_ctx<R>(
    raw: *mut c_void,
    operation: impl FnOnce(&StcpContext) -> R,
) -> Result<R, c_int> {
    let ctx_ptr = raw.cast::<StcpContext>();

    if ctx_ptr.is_null() {
        return Err(EINVAL);
    }

    let ctx = unsafe { &*ctx_ptr };
    Ok(operation(ctx))
}

#[inline]
fn with_ctx_result(
    raw: *mut c_void,
    operation: impl FnOnce(&StcpContext) -> Result<(), StcpError>,
) -> c_int {
    match with_ctx(raw, operation) {
        Ok(Ok(())) => 0,
        Ok(Err(error)) => error.errno(),
        Err(errno) => errno,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_init() -> c_int {
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_exit() {}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_create(
    proto: u8,
    out_ctx: *mut *mut c_void,
) -> c_int {
    if out_ctx.is_null() {
        return EINVAL;
    }

    unsafe {
        ptr::write(out_ctx, ptr::null_mut());
    }

    let ctx = match StcpContext::new(proto) {
        Ok(ctx) => ctx,
        Err(error) => return error.errno(),
    };

    let raw = Box::into_raw(Box::new(ctx)).cast();

    unsafe {
        ptr::write(out_ctx, raw);
    }

    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn stcp_rust_release(
    raw: *mut c_void,
) {
    if raw.is_null() {
        return;
    }

    let ctx = unsafe { &*raw.cast::<StcpContext>() };
    session::release(ctx);

    unsafe {
        drop(Box::from_raw(
            raw.cast::<StcpContext>(),
        ));
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_set_owner(
    raw: *mut c_void,
    owner: *mut c_void,
) {
    let _ = with_ctx(raw, |ctx| {
        session::set_owner(ctx, owner as usize);
    });
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_set_carrier(
    raw: *mut c_void,
    carrier: *mut c_void,
) {
    let _ = with_ctx(raw, |ctx| {
        crate::carrier::set_carrier(
            ctx,
            carrier as usize,
        );
    });
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_get_carrier(
    raw: *mut c_void,
) -> *mut c_void {
    match with_ctx(raw, |ctx| ctx.inner.lock().carrier) {
        Ok(carrier) => carrier as *mut c_void,
        Err(_) => core::ptr::null_mut(),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_bind(
    raw: *mut c_void,
    addr: u32,
    port: u16,
) -> c_int {
    with_ctx_result(raw, |ctx| {
        session::bind(ctx, addr, port)
    })
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_listen(
    raw: *mut c_void,
    backlog: c_int,
) -> c_int {
    with_ctx_result(raw, |ctx| {
        session::listen(ctx, backlog)
    })
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_connect(
    raw: *mut c_void,
    addr: u32,
    port: u16,
    _flags: c_int,
) -> c_int {
    with_ctx_result(raw, |ctx| {
        session::connect(ctx, addr, port)
    })
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_start_handshake(
    raw: *mut c_void,
) -> c_int {
    with_ctx_result(raw, session::start_handshake)
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_create_stream_accepted_child_ptr(
    listener_raw: *mut c_void,
) -> *mut c_void {
    match with_ctx(listener_raw, session::create_stream_accepted_child) {
        Ok(Ok(child)) => Box::into_raw(child).cast(),
        Ok(Err(_)) | Err(_) => ptr::null_mut(),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_create_stream_accepted_child(
    listener_raw: *mut c_void,
    out_ctx: *mut *mut c_void,
) -> c_int {
    if out_ctx.is_null() {
        return EINVAL;
    }

    unsafe {
        ptr::write(out_ctx, ptr::null_mut());
    }

    match with_ctx(listener_raw, session::create_stream_accepted_child) {
        Ok(Ok(child)) => {
            let child_ptr = Box::into_raw(child).cast();
            unsafe {
                ptr::write(out_ctx, child_ptr);
            }
            0
        }
        Ok(Err(error)) => error.errno(),
        Err(errno) => errno,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_accept(
    raw: *mut c_void,
    out_ctx: *mut *mut c_void,
    flags: c_int,
) -> c_int {
    if out_ctx.is_null() {
        return EINVAL;
    }

    match with_ctx(raw, |ctx| session::accept(ctx, flags)) {
        Ok(Ok(child)) => {
            let child_ptr = Box::into_raw(child).cast();

            unsafe {
                ptr::write(out_ctx, child_ptr);
            }

            0
        }
        Ok(Err(error)) => error.errno(),
        Err(errno) => errno,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_send(
    raw: *mut c_void,
    buffer: *const u8,
    len: usize,
    _flags: c_int,
) -> isize {
    if buffer.is_null() && len != 0 {
        return EINVAL as isize;
    }

    let data = if len == 0 {
        &[]
    } else {
        unsafe {
            slice::from_raw_parts(buffer, len)
        }
    };

    match with_ctx(raw, |ctx| session::send(ctx, data)) {
        Ok(Ok(bytes_sent)) => bytes_sent as isize,
        Ok(Err(error)) => error.errno() as isize,
        Err(errno) => errno as isize,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_recv(
    raw: *mut c_void,
    buffer: *mut u8,
    len: usize,
    _flags: c_int,
) -> isize {
    if buffer.is_null() && len != 0 {
        return EINVAL as isize;
    }

    let output = if len == 0 {
        &mut []
    } else {
        unsafe { slice::from_raw_parts_mut(buffer, len) }
    };

    match with_ctx(raw, |ctx| {
        crate::carrier::debug_event(100, ctx, len, 0);
        let result = session::recv(ctx, output);
        let code = match &result {
            Ok(n) => *n,
            Err(e) => (-e.errno()) as usize,
        };
        crate::carrier::debug_event(199, ctx, code, result.is_ok() as usize);
        result
    }) {
        Ok(Ok(bytes_received)) => bytes_received as isize,
        Ok(Err(error)) => error.errno() as isize,
        Err(errno) => errno as isize,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_has_accept(
    raw: *mut c_void,
) -> c_int {
    match with_ctx(raw, session::has_accept) {
        Ok(true) => 1,
        Ok(false) => 0,
        Err(errno) => errno,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_has_data(
    raw: *mut c_void,
) -> c_int {
    match with_ctx(raw, session::has_data) {
        Ok(true) => 1,
        Ok(false) => 0,
        Err(errno) => errno,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_is_connected(
    raw: *mut c_void,
) -> c_int {
    match with_ctx(raw, session::is_connected) {
        Ok(true) => 1,
        Ok(false) => 0,
        Err(errno) => errno,
    }
}


#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_can_send(
    raw: *mut c_void,
    len: usize,
) -> c_int {
    match with_ctx(raw, |ctx| session::can_send(ctx, len)) {
        Ok(true) => 1,
        Ok(false) => 0,
        Err(errno) => errno,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_tick(
    raw: *mut c_void,
) -> c_int {
    match with_ctx(raw, session::tick) {
        Ok(Ok(true)) => 1,
        Ok(Ok(false)) => 0,
        Ok(Err(error)) => error.errno(),
        Err(errno) => errno,
    }
}


#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_get_reliability_stats(
    raw: *mut c_void,
    out_stats: *mut StcpReliabilityStats,
) -> c_int {
    if out_stats.is_null() {
        return EINVAL;
    }

    match with_ctx(raw, session::reliability_snapshot) {
        Ok((srtt_ms, rttvar_ms, rto_ms, stats)) => {
            unsafe {
                ptr::write(
                    out_stats,
                    StcpReliabilityStats {
                        srtt_ms,
                        rttvar_ms,
                        rto_ms,
                        sent_frames: stats.sent_frames,
                        acknowledged_frames: stats.acknowledged_frames,
                        retransmitted_frames: stats.retransmitted_frames,
                        duplicate_frames: stats.duplicate_frames,
                        reordered_frames: stats.reordered_frames,
                        timeout_failures: stats.timeout_failures,
                        rtt_samples: stats.rtt_samples,
                    },
                );
            }
            0
        }
        Err(errno) => errno,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_shutdown(
    raw: *mut c_void,
    how: c_int,
) {
    let _ = with_ctx(raw, |ctx| {
        session::shutdown(ctx, how);
    });
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_crypto_selftest() -> c_int {
    match crate::crypto::selftest() { Ok(()) => 0, Err(e) => e.errno() }
}
