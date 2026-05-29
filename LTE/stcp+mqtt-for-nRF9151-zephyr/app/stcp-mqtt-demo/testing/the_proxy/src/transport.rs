use core::ffi::c_void;

use crate::debug::stcp_uptime_ms;
use crate::stcp_dbg;

use the_stcp_kernel_module::{
    proto_session::ProtoSession,
    stcp_protocol_functions::{
        rust_exported_session_recvmsg,
        rust_exported_session_sendmsg,
    },
};

unsafe extern "C" {

    pub fn stcp_tcp_send_iovec(
        sock: *mut c_void,
        msg_vp: *mut c_void,
        flags: i32,
    ) -> isize;

    pub fn stcp_tcp_recv(
        sock: *mut c_void,
        buf: *mut u8,
        len: usize,
        non_blocking: i32,
        flags: i32,
        recv_len: *mut usize,
    ) -> isize;

    pub fn stcp_tcp_send(
        sock: *mut c_void,
        buf: *const u8,
        len: usize,
    ) -> isize;

    fn stcp_rust_kernel_socket_create(
        fd: i32,
    ) -> *mut c_void;

    fn stcp_rust_kernel_socket_destroy(
        p: *mut c_void,
    );
}

pub fn create_transport(
    fd: i32,
) -> *mut c_void {

    let ret =
        unsafe {
            stcp_rust_kernel_socket_create(fd)
        };

    stcp_dbg!(
        "Created transport {:?} for fd={}",
        ret,
        fd
    );

    ret
}

pub fn destroy_transport(
    transport: *mut c_void,
) {

    stcp_dbg!(
        "Destroying transport {:?}",
        transport
    );

    unsafe {
        stcp_rust_kernel_socket_destroy(
            transport
        )
    }
}

pub fn recv_from_transport(
    session: &mut ProtoSession,
    encrypted_payload: &[u8],
    out_buf: &mut [u8],
) -> Result<usize, i32> {

    stcp_dbg!(
        "Decrypting payload {} bytes",
        encrypted_payload.len()
    );

    let rc =
        unsafe {
            rust_exported_session_recvmsg(
                session as *mut ProtoSession as *mut c_void,

                encrypted_payload.as_ptr()
                    as *const c_void,

                encrypted_payload.len(),

                out_buf.as_mut_ptr()
                    as *mut c_void,

                out_buf.len(),
            )
        };

    stcp_dbg!(
        "recvmsg rc={}",
        rc
    );

    if rc < 0 {
        return Err(rc as i32);
    }

    Ok(rc as usize)
}

pub fn send_to_transport(
    session: &mut ProtoSession,
    transport: *mut c_void,
    plain_buf: &[u8],
) -> Result<usize, i32> {

    stcp_dbg!(
        "Encrypting + sending {} bytes",
        plain_buf.len()
    );

    let rc =
        unsafe {
            rust_exported_session_sendmsg(
                session as *mut ProtoSession as *mut c_void,

                transport,

                plain_buf.as_ptr()
                    as *const c_void,

                plain_buf.len(),
            )
        };

    stcp_dbg!(
        "sendmsg rc={}",
        rc
    );

    if rc < 0 {
        return Err(rc as i32);
    }

    Ok(rc as usize)
}