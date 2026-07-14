use alloc::boxed::Box;

use core::{
    ffi::{c_int, c_void},
    ptr,
    slice,
};

use crate::{
    error::StcpError,
    state::StcpContext,
    transport,
};

const EAGAIN: c_int = -11;
const EINVAL: c_int = -22;

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
) -> *mut c_void {
    Box::into_raw(
        Box::new(StcpContext::new(proto)),
    )
    .cast()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn stcp_rust_release(
    raw: *mut c_void,
) {
    if raw.is_null() {
        return;
    }

    let ctx = unsafe { &*raw.cast::<StcpContext>() };
    transport::release(ctx);

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
        transport::set_owner(ctx, owner as usize);
    });
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_bind(
    raw: *mut c_void,
    addr: u32,
    port: u16,
) -> c_int {
    with_ctx_result(raw, |ctx| {
        transport::bind(ctx, addr, port)
    })
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_listen(
    raw: *mut c_void,
    backlog: c_int,
) -> c_int {
    with_ctx_result(raw, |ctx| {
        transport::listen(ctx, backlog)
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
        transport::connect(ctx, addr, port)
    })
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_accept(
    raw: *mut c_void,
    out_ctx: *mut *mut c_void,
    _flags: c_int,
) -> c_int {
    if out_ctx.is_null() {
        return EINVAL;
    }

    match with_ctx(raw, transport::accept) {
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

    match with_ctx(raw, |ctx| transport::send(ctx, data)) {
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
        unsafe {
            slice::from_raw_parts_mut(buffer, len)
        }
    };

    match with_ctx(raw, |ctx| transport::recv(ctx, output)) {
        Ok(Ok(bytes_received)) => bytes_received as isize,
        Ok(Err(error)) => error.errno() as isize,
        Err(errno) => errno as isize,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_has_accept(
    raw: *mut c_void,
) -> c_int {
    match with_ctx(raw, transport::has_accept) {
        Ok(true) => 1,
        Ok(false) => 0,
        Err(errno) => errno,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_has_data(
    raw: *mut c_void,
) -> c_int {
    match with_ctx(raw, transport::has_data) {
        Ok(true) => 1,
        Ok(false) => 0,
        Err(errno) => errno,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_is_connected(
    raw: *mut c_void,
) -> c_int {
    match with_ctx(raw, transport::is_connected) {
        Ok(true) => 1,
        Ok(false) => 0,
        Err(errno) => errno,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_shutdown(
    raw: *mut c_void,
    how: c_int,
) {
    let _ = with_ctx(raw, |ctx| {
        transport::shutdown(ctx, how);
    });
}
