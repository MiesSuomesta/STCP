
use stcpcommon::defines_etc::{StcpServer, ServerMessageProcessCB};


use stcpserverlib::{
    stcp_internal_server_bind,
    stcp_internal_server_listen,
    stcp_internal_server_stop,
};


#[no_mangle]
pub extern "C" fn stcp_server_bind(ip_ptr: *const u8, ip_len: usize, port: u16, cb: ServerMessageProcessCB) -> *mut std::ffi::c_void {
    if ip_ptr.is_null() {
        return std::ptr::null_mut();
    }

    let ip_slice = unsafe { std::slice::from_raw_parts(ip_ptr, ip_len) };

    let ip_str = match std::str::from_utf8(ip_slice) {
        Ok(s) => s,
        Err(_) => return std::ptr::null_mut(),
    };

    match stcp_internal_server_bind(ip_str, port, cb) {
        Ok(server) => Box::into_raw(Box::new(server)) as *mut std::ffi::c_void,
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn stcp_server_listen(server_ptr: *mut std::ffi::c_void) -> i32 {
    if server_ptr.is_null() {
        return -1;
    }
    let server = unsafe { &mut *(server_ptr as *mut StcpServer) };
    match stcp_internal_server_listen(server) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

#[no_mangle]
pub extern "C" fn stcp_server_stop(server_ptr: *mut std::ffi::c_void) {
        let server = unsafe { &mut *(server_ptr as *mut StcpServer) };
	stcp_internal_server_stop(server);
}

#[allow(dead_code)]
fn _force_link() {
//    let _ = stcp_internal_server_bind as usize;
}

