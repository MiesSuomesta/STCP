#![allow(dead_code)]

use core::ffi::c_int;
use core::fmt::{self, Write as FmtWrite};
use crate::debug::*;

#[macro_export]
macro_rules! stcp_tcp_op {
    ($op_name:expr, $call:expr) => {{
        // Ennen varsinaista kutsua
        $crate::debug::stcp_log_fmt(
            $crate::debug::LogLevel::Debug,
            core::format_args!(
            "TCP_OP {file}:{line} {func}(): {op} ..",
            file   = core::file!(),
            line   = core::line!(),
            func   = core::module_path!(),
            op     = $op_name,
        ));

        let ret = $call;

        // Jälkeen
        $crate::debug::stcp_log_fmt(
            $crate::debug::LogLevel::Debug,
            core::format_args!(
                "TCP_OP {file}:{line} {func}(): {op} DONE ret={ret}",
                file   = core::file!(),
                line   = core::line!(),
                func   = core::module_path!(),
                op     = $op_name,
                ret    = ret,
            ));

        ret
    }};
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

// RECV EXACT
#[macro_export]
macro_rules! stcp_tcp_recv_exact {
    ($sock:expr, $buf:expr, $no_blocking:expr) => {{
        $crate::stcp_tcp_op!(
            "RECV_EXACT",
            $crate::helpers::tcp_recv_exact($sock, $buf, $no_blocking)
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

