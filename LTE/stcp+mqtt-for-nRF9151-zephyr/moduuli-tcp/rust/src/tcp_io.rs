use core::ffi::c_int;
use core::ffi::c_void;

unsafe extern "C" {

    pub fn stcp_tcp_send_iovec(
        sock: *mut c_void,
        msg_vp: *mut core::ffi::c_void,
        flags: core::ffi::c_int,
    ) -> isize;

    pub fn stcp_tcp_recv(
            sock: *mut c_void,
            buf: *mut u8,
            len: usize,
            non_bloking: i32,
            flags: i32,
            recv_len: *mut c_int
        ) -> isize;


    pub fn stcp_tcp_send(
            sock: *mut c_void,
            buf: *const u8,
            len: usize
        ) -> isize;

}
