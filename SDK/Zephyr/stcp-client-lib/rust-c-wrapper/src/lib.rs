#![no_std]

extern crate alloc;
use alloc::vec::Vec;
use alloc::boxed::Box;

use core::result::Result;
use core::ffi::CStr;
use core::ffi::{c_char, c_int};
use core::ptr;


use stcpclientlib::client::{
    stcp_client_internal_connect,
    stcp_client_internal_send,
    stcp_client_internal_recv,
};


use iowrapper::types::StcpConnection;


#[no_mangle]
pub extern "C" fn stcp_client_connect(ip: *const c_char, port: u16) -> *mut core::ffi::c_void {
    if ip.is_null() {
        return ptr::null_mut();
    }

    let c_str = unsafe { CStr::from_ptr(ip) };
    let ip_str = match c_str.to_str() {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };

    let client = stcp_client_internal_connect(ip_str, port);

    client as *mut core::ffi::c_void
}

#[no_mangle]
pub extern "C" fn stcp_client_send(
    client_ptr: *mut core::ffi::c_void,
    data_ptr: *const u8,
    data_len: usize,
) -> c_int {
    if client_ptr.is_null() || data_ptr.is_null() {
        return -1 as c_int;
    }

    let client = unsafe { &mut *(client_ptr as *mut StcpConnection) };
//    let data = unsafe { core::slice::from_raw_parts(data_ptr, data_len) };

    match stcp_client_internal_send(client, data_ptr, data_len) {
	Ok(sentlen) => {
           sentlen as c_int
        }
	Err(code) => {
            code as c_int
        }
    }
}

#[no_mangle]
pub extern "C" fn stcp_client_recv(
    client_ptr: *mut core::ffi::c_void,
    out_buf: *mut u8,
    max_len: usize,
    recv_bytes: *mut usize,
) -> c_int {

    if client_ptr.is_null() || out_buf.is_null() || recv_bytes.is_null() {
        return -1 as c_int;
    }

    let client = unsafe { &mut *(client_ptr as *mut StcpConnection) };

    match stcp_client_internal_recv(client, out_buf, max_len) {
	Ok(outlen) => {
            unsafe {
                *recv_bytes = outlen;
            }
            0 as c_int
        }

	Err(code) => {
            unsafe {
                *recv_bytes = 0;
            }
            code as c_int
        }
    }
}

#[no_mangle]
pub extern "C" fn stcp_client_disconnect(client_ptr: *mut core::ffi::c_void) {
    if client_ptr.is_null() {
        return;
    }

    unsafe {
        let _ = Box::from_raw(client_ptr as *mut StcpConnection);
    }
}

#[allow(dead_code)]
fn _force_link() {
//    let _ = stcp_internal_server_bind as usize;
}
