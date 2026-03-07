use core::ffi::c_int;
use core::ffi::c_void;
use crate::types::{kernel_socket};
use core::panic::Location;

unsafe extern "C" {
    pub fn stcp_tcp_send(sock: *mut kernel_socket,
                     buf: *const u8,
                     len: usize) -> isize;

    pub fn stcp_tcp_send_iovec(
        sock: *mut c_void,
        msg_vp: *mut core::ffi::c_void,
        flags: core::ffi::c_int,
    ) -> isize;

    pub fn stcp_tcp_recv(sock: *mut kernel_socket,
                     buf: *mut u8,
                     len: usize,
                     non_blocking: i32,
                     flags: u32,
                     recv_len: &mut c_int) -> isize;
/*
    pub fn stcp_tcp_send_locked(sock: *mut kernel_socket,
                     buf: *const u8,
                     len: usize) -> isize;

    pub fn stcp_tcp_recv_locked(sock: *mut kernel_socket,
                     buf: *mut u8,
                     len: usize) -> isize;
*/
}
