pub mod server;

pub use server::{
    stcp_internal_server_bind,
    stcp_internal_server_listen,
    stcp_internal_server_stop,
};

use stcpcommon::defines_etc::{StcpServer, ServerMessageProcessCB};



#[no_mangle]
pub extern "C" fn stcp_server_bind(
    ip_ptr: *const u8,
    ip_len: usize,
    port: u16,
    cb: ServerMessageProcessCB,
) -> *mut StcpServer {
    if ip_ptr.is_null() {
        return std::ptr::null_mut();
    }
    let ip_slice = unsafe { std::slice::from_raw_parts(ip_ptr, ip_len) };
    let ip_str = match std::str::from_utf8(ip_slice) {
        Ok(s) => s,
        Err(_) => return std::ptr::null_mut(),
    };

    match stcp_internal_server_bind(ip_str, port, cb) {
        Ok(server) => Box::into_raw(Box::new(server)),
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn stcp_server_listen(server_ptr: *mut StcpServer) -> i32 {
    if server_ptr.is_null() {
        return -1;
    }
    let server = unsafe { &mut *server_ptr };
    match stcp_internal_server_listen(server) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

#[no_mangle]
pub extern "C" fn stcp_server_stop(server_ptr: *mut StcpServer) {
    stcp_internal_server_stop(server_ptr);
}

