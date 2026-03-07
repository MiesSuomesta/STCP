// debug.rs

#![allow(dead_code)]
use core::panic::Location;

use core::ffi::c_int;
use core::fmt::{self};

#[cfg(feature="stcp_debug")]
use core::fmt::{Write};

unsafe extern "C" {
    fn stcp_rust_log(level: c_int, buf: *const u8, len: usize);
}

/// Logitasot – matchaa C-puolen printk-prioihin
#[repr(i32)]
#[derive(Copy, Clone, Debug)]
pub enum LogLevel {
    Emerg   = 0,
    Alert   = 1,
    Crit    = 2,
    Err     = 3,
    Warn    = 4,
    Notice  = 5,
    Info    = 6,
    Debug   = 7,
}

/// Kuinka iso puskuri per logirivi heapilla.
/// Jos loppuu kesken, loppu katkaistaan hiljaa.
#[cfg(feature="stcp_debug")]
const STCP_LOG_BUF: usize = 1024;

/// Pieni wrapper, joka kerää fmt!-outputin Vec<u8>:iin (heap).
#[cfg(feature="stcp_debug")]
struct KernelLogBuf {
    buf: alloc::vec::Vec<u8>,
}

#[cfg(feature="stcp_debug")]
impl KernelLogBuf {
    fn new() -> Self {
        let v = alloc::vec::Vec::with_capacity(STCP_LOG_BUF);
        KernelLogBuf { buf: v }
    }

    #[inline(always)]
    fn as_bytes(&self) -> &[u8] {
        &self.buf
    }
}

/*
 *
 */


#[inline(always)]
pub fn get_lr_register() -> usize {

    #[cfg(target_arch = "arm")]
    unsafe {
        let lr: u32;
        core::arch::asm!(
            "mov {0}, lr",
            out(reg) lr,
            options(nomem, nostack, preserves_flags)
        );
        return lr as usize;
    }

    #[cfg(target_arch = "x86_64")]
    unsafe {
        let lr: usize;
        core::arch::asm!(
            "mov {0}, [rsp]",
            out(reg) lr,
            options(nomem, preserves_flags)
        );
        return lr as usize;
    }
}


#[inline(always)]
pub fn get_pc_register() -> usize {

    #[cfg(target_arch = "arm")]
    unsafe {
        let pc: u32;
        core::arch::asm!(
            "mov {0}, pc",
            out(reg) pc,
            options(nomem, nostack, preserves_flags)
        );
        return pc as usize;
    }

    #[cfg(target_arch = "x86_64")]
    unsafe {
        let rip: usize;
        unsafe {
            core::arch::asm!(
                "lea {0}, [rip]",
                out(reg) rip,
                options(nomem, nostack, preserves_flags)
            );
        }
        return rip as usize;
    }
}

//
// Registeri hommat loppuu
//


#[cfg(feature="stcp_debug")]
impl Write for KernelLogBuf {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        let bytes = s.as_bytes();
        let space_left = STCP_LOG_BUF.saturating_sub(self.buf.len());
        if space_left == 0 {
            // Ei lisää dataa jos täynnä, mutta ei kaadeta fmt:ää.
            return Ok(());
        }

        let to_copy = core::cmp::min(space_left, bytes.len());
        self.buf.extend_from_slice(&bytes[..to_copy]);
        Ok(())
    }
}

#[cfg(all(feature = "stcp_debug", not(target_os = "none")))]
#[inline(always)]
pub fn stcp_log_fmt(level: LogLevel, args: fmt::Arguments<'_>) {
    use alloc::string::ToString;

    let s = args.to_string();
    if s.is_empty() {
        return;
    }

    unsafe {
        stcp_rust_log(level as c_int, s.as_bytes().as_ptr(), s.len());
    }
}


/// Perus "fmt!" → C-log-funktioon.
#[cfg(all(feature = "stcp_debug", target_os = "none"))]
pub fn stcp_log_fmt(level: LogLevel, args: fmt::Arguments<'_>) {
    let mut kbuf = KernelLogBuf::new();
    // Jos formatointi failaa, emme tee mitään – ei panic kernelissä.
    let _ = kbuf.write_fmt(args);

    let slice = kbuf.as_bytes();
    if slice.is_empty() {
        return;
    }

    unsafe {
        stcp_rust_log(level as c_int, slice.as_ptr(), slice.len());
    }
}

#[cfg(not(feature="stcp_debug"))]
pub fn stcp_log_fmt(level: LogLevel, args: fmt::Arguments<'_>) {
    let _ = level;
    let _ = args;
}


/// Yksinkertainen suora string-logi ilman formatointia.
#[cfg(feature="stcp_debug")]
pub fn stcp_log_str(level: LogLevel, msg: &str) {
    let bytes = msg.as_bytes();
    if bytes.is_empty() {
        return;
    }

    unsafe {
        stcp_rust_log(level as c_int, bytes.as_ptr(), bytes.len());
    }
}

#[cfg(not(feature="stcp_debug"))]
pub fn stcp_log_str(level: LogLevel, msg: &str) {
    let _ = level;
    let _ = msg;
}

/// Yleislogi, ilman sessiota / sockia
#[macro_export]
#[track_caller]
macro_rules! stcp_log {
    ($level:expr, $($arg:tt)*) => {{
        //let loc = Location::caller();

        #[cfg(all(feature = "stcp_debug", target_os = "none"))]
        $crate::debug::stcp_log_fmt(
            $level,
            core::format_args!(
                "stcp/logging at {file}:{line} Registers[LR:{theLR:#010x}, PC:{thePC:#010x}]",
                file = core::file!(),
                line = core::line!(),
                theLR = crate::debug::get_lr_register(),
                thePC = crate::debug::get_pc_register(),
            )
        );

        $crate::debug::stcp_log_fmt(
            $level,
            core::format_args!(
                "stcp/RUST {file}:{line} {func}(): {msg}",
                file = core::file!(),
                line = core::line!(),
                func = core::module_path!(),
                msg  = core::format_args!($($arg)*),
            )
        );
    }};
}

/// Pelkkä debug-taso (lyhenne)
#[macro_export]
macro_rules! stcp_dbg {
    ($($arg:tt)*) => {{
        $crate::stcp_log!($crate::debug::LogLevel::Debug, $($arg)*);
    }};
}

#[macro_export]
macro_rules! stcp_info {
    ($($arg:tt)*) => {
        $crate::stcp_log!($crate::debug::LogLevel::Info, $($arg)*);
    };
}

#[macro_export]
macro_rules! stcp_err {
    ($($arg:tt)*) => {
        $crate::stcp_log!($crate::debug::LogLevel::Err, $($arg)*);
    };
}
/// Logi jossa käytössä sessio-pointeri
#[macro_export]
macro_rules! stcp_sess {
    ($sess:expr, $($arg:tt)*) => {
        let sess_ptr = $sess as *const core::ffi::c_void;
        $crate::debug::stcp_log_fmt(
            $crate::debug::LogLevel::Debug,
            core::format_args!(
                "stcp/RUST {file}:{line} {func}(): Session[{sess:p}]: {msg}",
                file = core::file!(),
                line = core::line!(),
                func = core::module_path!(),
                sess = sess_ptr,
                msg  = core::format_args!($($arg)*),
            )
        );
    };
}


#[macro_export]
macro_rules! stcp_dump {
    ($info:expr, $buf:expr) => {
        $crate::debug::stcp_log_fmt(
            $crate::debug::LogLevel::Debug,
            core::format_args!(
                "stcp/RUST {file}:{line} {func}(): DUMP[ {info} ]: {len} bytes: {dump} // {ascii}",
                file = core::file!(),
                line = core::line!(),
                func = core::module_path!(),
                info = $info,
                len  = ($buf).len(),
                dump = $crate::debug::HexDump($buf),
                ascii = $crate::debug::AsciiDump($buf),
            )
        );
    };
}

#[macro_export]
macro_rules! stcp_sess_transp {
    ($sess:expr, $transp:expr, $($arg:tt)*) => {
        $crate::debug::stcp_log_fmt(
            $crate::debug::LogLevel::Debug,
            core::format_args!(
                "stcp/RUST {file}:{line} {func}(): Session[{sess:p}//{transp:p}]: {msg}",
                file = core::file!(),
                line = core::line!(),
                func = core::module_path!(),
                sess = $sess,
                transp = $transp,
                msg  = core::format_args!($($arg)*),
            )
        );
    };
}

/// Worker + transport -debug, pohjana siitä versiosta jonka lähetit.
#[macro_export]
macro_rules! stcp_worker_dbg {
    ($worker:expr, $transp:expr, $($arg:tt)*) => {
        $crate::debug::stcp_log_fmt(
            $crate::debug::LogLevel::Debug,
            core::format_args!(
                "stcp/RUST {file}:{line} {func}(): Worker[{worker:p}//{transp:p}]: {msg}",
                file = core::file!(),
                line = core::line!(),
                func = core::module_path!(),
                worker = $worker,
                transp = $transp,
                msg  = core::format_args!($($arg)*),
            )
        );
    };
}



// debug.rs jatkuu

pub struct AsciiDump<'a>(pub &'a [u8]);

impl<'a> fmt::Display for AsciiDump<'a> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let bytes = self.0;

        for (i, b) in bytes.iter().enumerate() {
            let mut ch = *b as char;

            if *b != 32 {
                if !ch.is_ascii_graphic() {
                    ch = '.';
                }
            }

            write!(f, "{}", ch)?;

            if i + 1 < bytes.len() {
                f.write_str(" ")?;
            }
        }

        Ok(())
    }
}

pub struct HexDump<'a>(pub &'a [u8]);

impl<'a> fmt::Display for HexDump<'a> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let bytes = self.0;

        for (i, b) in bytes.iter().enumerate() {
            write!(f, "{:02X}", b)?;

            if i + 1 < bytes.len() {
                f.write_str(" ")?;
            }
        }

        Ok(())
    }
}
