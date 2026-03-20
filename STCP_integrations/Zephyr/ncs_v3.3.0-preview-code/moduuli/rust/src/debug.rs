#![allow(dead_code)]
use core::ffi::c_int;
use core::fmt::{self, Write};

use crate::abi::stcp_rust_log;

/*
    // Check ratelimitter
    static const char *lvl[] = {
        "??",   // 0
        "ERR",  // 1
        "WRN",  // 2
        "INF",  // 3
        "DBG",  // 4
        "TRC",  // 5
    };*/

#[repr(i32)]
#[derive(Copy, Clone)]
pub enum LogLevel {
    Err   = 1,
    Warn  = 2,
    Info  = 3,
    Debug = 4,
}

#[cfg(feature = "stcp_debug")]
pub fn stcp_log_fmt(level: LogLevel, args: fmt::Arguments<'_>) {

    struct Buf {
        buf: [u8; 256],
        len: usize,
    }

    impl Write for Buf {
        fn write_str(&mut self, s: &str) -> fmt::Result {

            let bytes = s.as_bytes();
            let space = self.buf.len() - self.len;
            let n = core::cmp::min(space, bytes.len());

            self.buf[self.len..self.len+n].copy_from_slice(&bytes[..n]);
            self.len += n;

            Ok(())
        }
    }

    let mut b = Buf {
        buf: [0;256],
        len: 0,
    };

    let _ = b.write_fmt(args);

    unsafe {
        #[cfg(feature = "stcp_debug")]
        stcp_rust_log(level as c_int, b.buf.as_ptr(), b.len);
    }
}

#[cfg(feature = "stcp_debug")]
pub fn stcp_log_fmt_big(level: LogLevel, args: fmt::Arguments<'_>) {

    struct Buf {
        buf: [u8; 256],
        len: usize,
    }

    impl Write for Buf {
        fn write_str(&mut self, s: &str) -> fmt::Result {

            let bytes = s.as_bytes();
            let space = self.buf.len() - self.len;
            let n = core::cmp::min(space, bytes.len());

            self.buf[self.len..self.len+n].copy_from_slice(&bytes[..n]);
            self.len += n;

            Ok(())
        }
    }

    let mut b = Buf {
        buf: [0;256],
        len: 0,
    };

    let _ = b.write_str(".--------------------------------------------------------------->\n");
    let _ = b.write_str("| ");
    let _ = b.write_fmt(args);
    let _ = b.write_str("'----------------------------------------->\n");

    unsafe {
        #[cfg(feature = "stcp_debug")]
        stcp_rust_log(level as c_int, b.buf.as_ptr(), b.len);
    }
}

#[cfg(not(feature = "stcp_debug"))]
pub fn stcp_log_fmt(_: LogLevel, _: fmt::Arguments<'_>) {}

#[cfg(not(feature = "stcp_debug"))]
pub fn stcp_log_fmt_big(_: LogLevel, _: fmt::Arguments<'_>) {}

pub fn stcp_dump_bytes(level: LogLevel, info: &str, data: &[u8]) {

    let mut buf = [0u8; 256];
    let mut pos = 0;

    fn push(buf: &mut [u8], pos: &mut usize, s: &[u8]) {
        let space = buf.len() - *pos;
        let n = core::cmp::min(space, s.len());
        buf[*pos..*pos+n].copy_from_slice(&s[..n]);
        *pos += n;
    }

    push(&mut buf, &mut pos, info.as_bytes());
    push(&mut buf, &mut pos, b": ");

    for b in data {

        let hex = [
            b"0123456789ABCDEF"[(*b >> 4) as usize],
            b"0123456789ABCDEF"[(*b & 0xF) as usize],
        ];

        push(&mut buf, &mut pos, &hex);
        push(&mut buf, &mut pos, b" ");
    }

    unsafe {
        crate::abi::stcp_rust_log(
            level as core::ffi::c_int,
            buf.as_ptr(),
            pos
        );
    }
}


#[macro_export]
macro_rules! stcp_dump {
    ($info:expr, $buf:expr) => {
        $crate::debug::stcp_dump_bytes(
            $crate::debug::LogLevel::Debug,
            $info,
            $buf
        );
    };
}

#[macro_export]
macro_rules! stcp_dbg {
    ($($arg:tt)*) => {
        $crate::debug::stcp_log_fmt(
            $crate::debug::LogLevel::Debug,
            core::format_args!($($arg)*)
        );
    };
}

#[macro_export]
macro_rules! stcp_info {
    ($($arg:tt)*) => {
        $crate::debug::stcp_log_fmt(
            $crate::debug::LogLevel::Info,
            core::format_args!($($arg)*)
        );
    };
}

#[macro_export]
macro_rules! stcp_err {
    ($($arg:tt)*) => {
        $crate::debug::stcp_log_fmt(
            $crate::debug::LogLevel::Err,
            core::format_args!($($arg)*)
        );
    };
}

#[macro_export]
macro_rules! stcp_dbg_big {
    ($($arg:tt)*) => {
        $crate::debug::stcp_log_fmt_big(
            $crate::debug::LogLevel::Debug,
            core::format_args!($($arg)*)
        );
    };
}

#[macro_export]
macro_rules! stcp_info_big {
    ($($arg:tt)*) => {
        $crate::debug::stcp_log_fmt_big(
            $crate::debug::LogLevel::Info,
            core::format_args!($($arg)*)
        );
    };
}

#[macro_export]
macro_rules! stcp_err_big {
    ($($arg:tt)*) => {
        $crate::debug::stcp_log_fmt_big(
            $crate::debug::LogLevel::Err,
            core::format_args!($($arg)*)
        );
    };
}

#[macro_export]
macro_rules! stcp_warn_big {
    ($($arg:tt)*) => {
        $crate::debug::stcp_log_fmt_big(
            $crate::debug::LogLevel::Warn,
            core::format_args!($($arg)*)
        );
    };
}