use alloc::boxed::Box;

use core::{
    ffi::{c_int, c_void},
    ptr,
    slice,
};

use crate::{
    state::StcpContext,
    transport,
};

const EAGAIN: c_int = -11;
const EINVAL: c_int = -22;

#[inline]
fn ctx_from_ptr<'a>(
    ptr: *mut c_void,
) -> Result<&'a mut StcpContext, c_int> {
    unsafe {
        ptr.cast::<StcpContext>()
            .as_mut()
            .ok_or(EINVAL)
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
    let ctx = Box::new(StcpContext::new(proto));

    Box::into_raw(ctx).cast()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn stcp_rust_release(
    ptr: *mut c_void,
) {
    if ptr.is_null() {
        return;
    }

    let ctx = unsafe {
            Box::from_raw(ptr.cast::<StcpContext>())
        };

    drop(ctx);
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_bind(
    ptr: *mut c_void,
    addr: u32,
    port: u16,
) -> c_int {
    let result = ctx_from_ptr(ptr).and_then(|ctx| {
        transport::bind(ctx, addr, port)
            .map_err(|err| err.errno())
    });

    match result {
        Ok(()) => 0,
        Err(errno) => errno,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_listen(
    ptr: *mut c_void,
    backlog: c_int,
) -> c_int {
    let result = ctx_from_ptr(ptr).and_then(|ctx| {
        transport::listen(ctx, backlog)
            .map_err(|err| err.errno())
    });

    match result {
        Ok(()) => 0,
        Err(errno) => errno,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_connect(
    ptr: *mut c_void,
    addr: u32,
    port: u16,
    _flags: c_int,
) -> c_int {
    let result = ctx_from_ptr(ptr).and_then(|ctx| {
        transport::connect(ctx, addr, port)
            .map_err(|err| err.errno())
    });

    match result {
        Ok(()) => 0,
        Err(errno) => errno,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_accept(
    ptr: *mut c_void,
    out_ctx: *mut *mut c_void,
    _flags: c_int,
) -> c_int {
    if out_ctx.is_null() {
        return EINVAL;
    }

    let ctx = match ctx_from_ptr(ptr) {
        Ok(ctx) => ctx,
        Err(errno) => return errno,
    };

    match ctx.accept_queue.pop_front() {
        Some(child) => {
            let child_ptr = Box::into_raw(child).cast();

            unsafe {
                ptr::write(out_ctx, child_ptr);
            }

            0
        }

        None => EAGAIN,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_send(
    ptr: *mut c_void,
    buffer: *const u8,
    len: usize,
    _flags: c_int,
) -> isize {
    if buffer.is_null() && len != 0 {
        return EINVAL as isize;
    }

    let ctx = match ctx_from_ptr(ptr) {
        Ok(ctx) => ctx,
        Err(errno) => return errno as isize,
    };

    let data = unsafe {
        slice::from_raw_parts(buffer, len)
    };

    match transport::send(ctx, data) {
        Ok(bytes_sent) => bytes_sent as isize,
        Err(err) => err.errno() as isize,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_recv(
    ptr: *mut c_void,
    buffer: *mut u8,
    len: usize,
    _flags: c_int,
) -> isize {
    if buffer.is_null() && len != 0 {
        return EINVAL as isize;
    }

    let ctx = match ctx_from_ptr(ptr) {
        Ok(ctx) => ctx,
        Err(errno) => return errno as isize,
    };

    let output = unsafe {
        slice::from_raw_parts_mut(buffer, len)
    };

    match transport::recv(ctx, output) {
        Ok(bytes_received) => bytes_received as isize,
        Err(err) => err.errno() as isize,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_shutdown(
    _ptr: *mut c_void,
    _how: c_int,
) {
}
