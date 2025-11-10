#![no_std]

extern crate alloc;
use alloc::vec::Vec;
use alloc::boxed::Box;

use core::ptr;
use core::slice;
use core::net::{IpAddr, SocketAddr};

use iowrapper::stream::{StcpStream};
use iowrapper::types::StcpConnection;
use spin::Mutex;


use stcpcrypto::aes_lib::StcpAesCodec;
use stcpcrypto::stcp_elliptic_codec::StcpEllipticCodec;
use iowrapper::handshake::StcpUtils;

use utils;

use debug::zprint;
use debug::dbg;


// Debugging amcros


pub fn make_addr(ip_str: &str, port: u16) -> Option<SocketAddr> {
    let ip: IpAddr = ip_str.parse().ok()?;
    Some(SocketAddr::new(ip, port))
}

pub fn stcp_client_internal_connect(addr: &str, port: u16) -> *mut StcpConnection {
    if addr.is_empty() {
        return ptr::null_mut();
    }

    let addr_str = unsafe { addr };

  //  if let Some(full_addr) = make_addr(addr_str, port) {
  //
    let ec = StcpEllipticCodec::New();
    let aes = StcpAesCodec::new();
    let util = StcpUtils::new();

    let ip: IpAddr = match addr_str.parse() {
		Ok(ip) => ip,
		Err(_) => return  ptr::null_mut(),
	};

    let mut stream = match StcpStream::connect(SocketAddr::new(ip, port)) {
        Ok(s) => s,
	Err(_) => return  ptr::null_mut(),
    };

    let keys = util.do_the_stcp_handshake_client(&mut stream, ec);
    
    let shared_key = keys.1;

    if shared_key.is_empty() {
	return  ptr::null_mut();
    }

    let conn = Box::new(StcpConnection {
            stream: Mutex::new(stream),
            aes,
            shared_key,
        });

    Box::into_raw(conn)
}

pub fn stcp_client_internal_send(conn: *mut StcpConnection, data: *const u8, len: usize) -> Result<usize, i32> {
    if conn.is_null() || data.is_null() || len == 0 {
        return Err(-2);
    }

    let msg = unsafe { slice::from_raw_parts(data, len).to_vec() };
    let conn = unsafe { &*conn };

    let rv : usize = 0;
    let ec : i32 = 0;

    let mut guard = conn.stream.lock();
    let stream = &mut *guard;

    let encrypted = conn.aes.handle_aes_outgoing_message(&msg, &conn.shared_key);

    match stream.write_all(&encrypted) {
        Ok(_) => Ok(encrypted.len()),
        Err(_) => Err(-2)
    }
}

pub fn stcp_client_internal_recv(conn: *mut StcpConnection, data: *mut u8, len: usize) -> Result<usize, i32> {
    if conn.is_null() || data.is_null() || len == 0 {
        return Err(-2);
    }

    let msg = unsafe { slice::from_raw_parts(data, len).to_vec() };
    let conn = unsafe { &*conn };

    let rv : usize = 0;
    let ec : i32 = 0;

    let mut guard = conn.stream.lock();

    let stream = &mut *guard;

    let utils = StcpUtils::new();

        // Tässä odotetaan viestiä:
    let (_, encrypted) = utils.read_from(stream);

    let decrypted = conn.aes.handle_aes_incoming_message(&encrypted, &conn.shared_key);

    let to_copy = core::cmp::min(len, decrypted.len());

    unsafe {
        core::ptr::copy_nonoverlapping(decrypted.as_ptr(), data, to_copy);
    }

    Ok(to_copy)
}

pub fn stcp_client_internal_disconnect(conn: *mut StcpConnection) {

    let conn_in = unsafe { &mut *conn };
    unsafe {
        drop(Box::from_raw(conn_in));
    }

}

