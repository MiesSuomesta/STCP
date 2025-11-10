#![allow(unused_imports)]
use core::ffi::{c_char, c_void};

use crate::abi::stcp_rust_kernel_printk;

/// Peruslogi:
///   stcp_log!("hello");
///   stcp_log!("err=%d", err);
#[macro_export]
macro_rules! stcp_log {
    ($fmt:literal $(, $arg:expr)* $(,)?) => {{
        unsafe {
            $crate::abi::stcp_rust_kernel_printk(
                concat!("stcp: ", $fmt, "\n\0").as_ptr()
                    as *const core::ffi::c_char,
                $($arg),*
            );
        }
    }};
}

/// Konteksti-logi:
///   stcp_trace_ctx!("stcp_rust_connect", sk);
///   stcp_trace_ctx!("stcp_rust_connect", sk, "err=%d", rc);
#[macro_export]
macro_rules! stcp_trace_ctx {
    // Vain ctx
    ($name:literal, $ctx:expr) => {{
        unsafe {
            $crate::abi::stcp_rust_kernel_printk(
                concat!("stcp: ", $name, " ctx=%p\n\0").as_ptr()
                    as *const core::ffi::c_char,
                $ctx as *const core::ffi::c_void,
            );
        }
    }};
    // ctx + lisäargumentteja (printf-tyyli)
    ($name:literal, $ctx:expr, $fmt:literal $(, $arg:expr)* $(,)?) => {{
        unsafe {
            $crate::abi::stcp_rust_kernel_printk(
                concat!("stcp: ", $name, " ctx=%p ", $fmt, "\n\0").as_ptr()
                    as *const core::ffi::c_char,
                $ctx as *const core::ffi::c_void,
                $($arg),*
            );
        }
    }};
}

/// Yksinkertainen wrapperi jos haluat tulostaa valmiin &str:n
pub fn kprint(msg: &str) {
    unsafe {
        let bytes = msg.as_bytes();
        let mut buf = [0u8; 256];

        let len = if bytes.len() >= buf.len() - 1 {
            buf.len() - 1
        } else {
            bytes.len()
        };

        buf[..len].copy_from_slice(&bytes[..len]);
        buf[len] = 0; // NUL

        stcp_rust_kernel_printk(buf.as_ptr() as *const c_char);
    }
}
