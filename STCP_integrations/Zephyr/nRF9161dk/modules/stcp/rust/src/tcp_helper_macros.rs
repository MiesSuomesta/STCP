#![allow(dead_code)]

use core::ffi::c_int;
use crate::tcp_io::stcp_tcp_recv;
use crate::errorit::EAGAIN;

#[macro_export]
macro_rules! stcp_tcp_op {
    ($op_name:expr, $call:expr) => {{
        $crate::debug::stcp_log_fmt(
            $crate::debug::LogLevel::Debug,
            core::format_args!(
                "TCP_OP {file}:{line} {func}(): {op} ..",
                file = core::file!(),
                line = core::line!(),
                func = core::module_path!(),
                op   = ($op_name),
            )
        );

        let ret = $call;

        $crate::debug::stcp_log_fmt(
            $crate::debug::LogLevel::Debug,
            core::format_args!(
                "TCP_OP {file}:{line} {func}(): {op} DONE ret={ret}",
                file = core::file!(),
                line = core::line!(),
                func = core::module_path!(),
                op   = ($op_name),
                ret  = ret,
            )
        );

        ret
    }}
}

// SEND ALL
#[macro_export]
macro_rules! stcp_tcp_send_all {
    ($sock:expr, $data:expr) => {{
        $crate::stcp_tcp_op!(
            "SEND_ALL",
            $crate::helpers::tcp_send_all($sock, $data)
        )
    }};
}

// PEEK MAX
#[macro_export]
macro_rules! stcp_tcp_peek_max {
    ($sock:expr, $buf:expr, $no_blocking:expr) => {{
        $crate::stcp_tcp_op!(
            "PEEK_MAX",
            $crate::helpers::tcp_peek_max($sock, $buf, $no_blocking)
        )
    }};
}

// RECV UNTIL BUFFER FULL
#[macro_export]
macro_rules! stcp_tcp_recv_until_buffer_full {
    ($sock:expr, $buf:expr, $no_blocking:expr) => {{
        $crate::stcp_tcp_op!(
            "RECV_UNTIL_FULL",
            $crate::helpers::tcp_recv_until_buffer_full($sock, $buf, $no_blocking)
        )
    }};
}

// RECV ONCE
#[macro_export]
macro_rules! stcp_tcp_recv_once {
    ($sock:expr, $buf:expr, $no_blocking:expr) => {{
        $crate::stcp_tcp_op!(
            "RECV_ONCE",
            $crate::helpers::tcp_recv_once($sock, $buf, $no_blocking)
        )
    }};
}


// RECV EXACT
pub fn stcp_tcp_recv_exact(
    sock: *mut core::ffi::c_void,
    buf: &mut [u8],
    non_blocking: i32,
) -> isize {

    let mut offset = 0;

    while offset < buf.len() {

        let mut got: c_int = 0;

        let rc = unsafe {
            stcp_tcp_recv(
                sock,
                buf[offset..].as_mut_ptr(),
                buf.len() - offset,
                non_blocking,
                0,
                &mut got,
            )
        };

        if rc == -EAGAIN as isize {

            // non-blocking socket → ei dataa vielä
            if non_blocking != 0 {
                return 0;
            }

            continue;
        }

        if rc < 0 {
            return rc;
        }

        if got == 0 {

            // peer closed
            return -1;
        }

        offset += got as usize;
    }

    offset as isize
}