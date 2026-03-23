use core::ffi::c_int;
use crate::types::{kernel_socket};

unsafe extern "C" {
    pub fn stcp_tcp_send(sock: *mut kernel_socket,
                     buf: *const u8,
                     len: usize) -> isize;

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
