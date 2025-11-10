#![no_std]
pub mod client;

use core::ffi::c_char;
use core::ffi::c_int;
use core::ffi::c_void;
use core::ffi::CStr;
use core::ptr;
use heapless::String;

pub use iowrapper::types::StcpConnection;

use crate::client::{
    stcp_client_internal_connect,
    stcp_client_internal_send,
    stcp_client_internal_recv,
    stcp_client_internal_disconnect,
};

#[no_mangle]
pub extern "C" fn stcp_client_send(conn_ptr: *mut StcpConnection, data_ptr: *const u8, len: usize) -> c_int {
    if conn_ptr.is_null() || data_ptr.is_null() || len == 0 {
        return -1 as c_int;
    }

    match stcp_client_internal_send(conn_ptr, data_ptr, len) {
		Ok(nlen)  => { nlen as c_int }
		Err(code) => { code }
	}
}

#[no_mangle]
pub extern "C" fn stcp_client_recv(conn_ptr: *mut StcpConnection, data_ptr: *mut u8, max_len: usize) -> c_int {
    if conn_ptr.is_null() || data_ptr.is_null() || max_len == 0 {
        return -1 as c_int;
    }

    match stcp_client_internal_recv(conn_ptr, data_ptr, max_len) {
		Ok(nlen)  => { nlen as c_int }
		Err(code) => { code }
	}
}

#[no_mangle]
pub extern "C" fn stcp_client_disconnect(conn: *mut StcpConnection) {
    stcp_client_internal_disconnect(conn);
}

#[no_mangle]
pub extern "C" fn stcp_client_connect(addr: *const c_char, port: u16) -> *mut StcpConnection {
    if addr.is_null() {
        return ptr::null_mut();
    }

    let c_str = unsafe { CStr::from_ptr(addr) };
    let addr_str = match c_str.to_str() {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };
 
   stcp_client_internal_connect(addr_str, port)
}







