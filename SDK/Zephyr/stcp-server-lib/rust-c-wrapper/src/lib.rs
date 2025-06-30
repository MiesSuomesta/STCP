#![no_std]

extern crate alloc;
use alloc::vec::Vec;
use alloc::boxed::Box;

use stcpserverlib::{
    stcp_internal_server_bind,
    stcp_internal_server_listen,
    stcp_internal_server_stop,
};

use core::result::Result;
use stcpdefines::*;
use stcptypes::types::ServerMessageProcessCB;
use iowrapper::types::StcpServer;

#[no_mangle]
pub extern "C" fn stcp_server_bind(ip_ptr: *const u8, ip_len: usize, port: u16, cb: ServerMessageProcessCB) -> *mut core::ffi::c_void {
    if ip_ptr.is_null() {
        return core::ptr::null_mut();
    }

    let ip_slice = unsafe { core::slice::from_raw_parts(ip_ptr, ip_len) };

    let ip_str = match core::str::from_utf8(ip_slice) {
        Ok(s) => s,
        Err(_) => return core::ptr::null_mut(),
    };

    match stcp_internal_server_bind(ip_str, port, cb) {
        Ok(server) => Box::into_raw(Box::new(server)) as *mut core::ffi::c_void,
        Err(_) => core::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn stcp_server_listen(server_ptr: *mut core::ffi::c_void) -> i32 {
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
pub extern "C" fn stcp_server_stop(server_ptr: *mut core::ffi::c_void) {
        let server = unsafe { &mut *(server_ptr as *mut StcpServer) };
	stcp_internal_server_stop(server);
}

#[allow(dead_code)]
fn _force_link() {
//    let _ = stcp_internal_server_bind as usize;
}

