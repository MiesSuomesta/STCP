#![cfg(target_os = "linux")]

use core::ffi::c_int;
use core::ptr;

#[unsafe(no_mangle)]
pub extern "C" fn stcp_random_get(buf: *mut u8, len: usize) -> c_int {
    if buf.is_null() || len == 0 {
        return -libc::EINVAL;
    }

    let mut filled = 0usize;

    while filled < len {
        let rc = unsafe {
            libc::syscall(
                libc::SYS_getrandom,
                buf.add(filled) as *mut libc::c_void,
                len - filled,
                0,
            )
        };

        if rc < 0 {
            let err = unsafe { *libc::__errno_location() };
            if err == libc::EINTR {
                continue; // retry
            }
            return -err;
        }

        filled += rc as usize;
    }

    0
}