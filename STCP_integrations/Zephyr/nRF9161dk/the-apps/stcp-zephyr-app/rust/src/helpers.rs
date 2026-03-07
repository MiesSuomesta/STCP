
use core::ffi::c_int;
use core::ffi::c_void;
use crate::types::{kernel_socket};
use crate::tcp_io;
use crate::stcp_dbg;
use crate::stcp_dump;
use crate::errorit::*;
use alloc::vec::Vec;
use core::panic::Location;
use crate::proto_session::ProtoSession;


// sleep wrapper
pub fn stcp_sleep_msec(ms: u32) {
    unsafe {
        crate::abi::stcp_sleep_ms(ms);
    }
}


#[cfg(not(feature = "std"))]
#[cfg(target_os = "none")]   // Zephyr
const STCP_PEEK_FLAGS: u32 = 0;

#[cfg(not(feature = "std"))]
#[cfg(not(target_os = "none"))] // Linux / userspace
const STCP_PEEK_FLAGS: u32 = MSG_PEEK;

#[cfg(feature = "std")]
const STCP_PEEK_FLAGS: u32 = 0; // TODO: katso oikee const tähän

pub fn tcp_send_all(sock: *mut kernel_socket, data: &[u8]) -> isize {

  stcp_dbg!("TCP SENDing {} bytes..", data.len());   

    let mut total = 0usize;
    let len = data.len();

    while total < len {
        let ptr = unsafe { data.as_ptr().add(total) };
        let left = len - total;

        let n = unsafe { tcp_io::stcp_tcp_send(sock, ptr, left) };
        if n < 0 {
            stcp_dbg!("TCP SEND Error: {}", n);   
            return n;
        }
        if n == 0 {
            stcp_dbg!("TCP SEND peer closed");   
            break;
        }
        total += n as usize;
    }

    stcp_dbg!("TCP SEND Done, {} bytes sent.", total);   
    total as isize
}

pub fn tcp_recv_exact(sock: *mut kernel_socket, buf: &mut [u8], no_blocking: i32) -> isize {

  stcp_dbg!("TCP RECV exact {} bytes.", buf.len());   

    let mut total = 0usize;
    let len = buf.len();

    while total < len {
        let ptr = unsafe { buf.as_mut_ptr().add(total) };
        let left = len - total;

        let mut recv_len: c_int = 0;
        let n = unsafe { tcp_io::stcp_tcp_recv(sock, ptr, left, no_blocking, 0, &mut recv_len) };
        if n < 0 {
            stcp_dbg!("TCP RECV Sock peer error");   
            return n;
        }
        if n == 0 {
            // peer sulki yhteyden
            stcp_dbg!("TCP RECV Sock peer closed received bytes");   
            return 0;
        }
        total += n as usize;
    }

    stcp_dbg!("TCP RECV Done, {} bytes recvd.", total);   
    total as isize
}

pub fn tcp_peek_max(sock: *mut kernel_socket, buf: &mut [u8], no_bloking: i32) -> isize {

  stcp_dbg!("TCP PEEK Sock Peeking traffic");   

    let len = buf.len();

    // Zephyr buildissa rikki PEEK:kki, ei voi käyttää luotettavasti..
    // joten: Peek kuluttaa Zephyrillä => Voi aiheuttaa sen ettei toimi
    //        niin luotettavasti aina!
    let flags: u32 = STCP_PEEK_FLAGS;

    stcp_dbg!("TCP PEEK Sock Start to peek while, flags: {}", flags);   

    let ptr =  buf.as_mut_ptr();

    let mut recv_len: c_int = 0;
    stcp_dbg!("TCP PEEK Sock Start to peek bytes max");   
    let n = unsafe { tcp_io::stcp_tcp_recv(sock, ptr, len, no_bloking, flags, &mut recv_len) };
    stcp_dbg!("STCP TCP recv returned {}", n);

    if n < 0 {
        stcp_dbg!("TCP PEEK Sock peer error");   
    }
    if n == 0 {
        // peer sulki yhteyden
        stcp_dbg!("TCP PEEK Sock peer closed received bytes");   
    }

    stcp_dbg!("TCP PEEK Sock Received {} bytes", n);   
    n as isize
}

pub fn tcp_recv_until_buffer_full(sock: *mut kernel_socket, buf: &mut [u8], no_blocking: i32) -> isize {

  stcp_dbg!("TCP RECV Sock Buffer bytes");   

    let mut total = 0usize;
    let max_len = buf.len();

    while total < max_len {
        let ptr = unsafe { buf.as_mut_ptr().add(total) };
        let left = max_len - total;

        let mut recv_len: c_int = 0;
        let n = unsafe { tcp_io::stcp_tcp_recv(sock, ptr, left, no_blocking, 0, &mut recv_len) };
        if n < 0 {
            return n;
        }
        if n == 0 {
            // peer sulki yhteyden
            return total as isize;
        }
        total += n as usize;
    }

  stcp_dbg!("TCP RECV Sock Received bytes");   
    total as isize
}


pub fn tcp_recv_once(sock: *mut kernel_socket, buf: &mut [u8], no_blocking: i32) -> isize {

  stcp_dbg!("TCP RECV Sock once");   
  let mut recv_len: c_int = 0;
  let n = unsafe { tcp_io::stcp_tcp_recv(sock, buf.as_mut_ptr(), buf.len(), no_blocking, 0, &mut recv_len) };
  stcp_dbg!("TCP RECV Sock Received");   
  n as isize
}

// Sessio hakeminen
pub fn get_session<'a>(ptr: *mut c_void) -> Result<(&'a mut ProtoSession, *mut ProtoSession), isize> {
    if ptr.is_null() {
        return Err(-EBADF as isize);
    }
    let raw = ptr as *mut ProtoSession;
    let s: &mut ProtoSession = unsafe { &mut *raw };
    Ok((s, raw))
}

//
// Misc: keys alloc from C => Rust buff
//
use crate::abi::*;
use core::slice;

pub fn stcp_helper_get_heap_alloc_from(keyp: *mut c_void, keys: usize) -> Vec<u8> {
    if keyp.is_null() || keys == 0 {
        return Vec::new();
    }

    unsafe {
        // Luo read-only slice kernelin muistista
        let src: &[u8] = slice::from_raw_parts(keyp as *const u8, keys);

        // Luo Rust-puolen bufferi ja kopioi data
        let mut rbuff = Vec::with_capacity(keys);
        rbuff.set_len(keys);
        rbuff.copy_from_slice(src);

        // Vapauta kernel-muisti
        stcp_misc_ecdh_key_free(keyp);

        rbuff
    }
}

pub fn stcp_helper_get_ecdh_shared_key_new() -> Vec<u8> {
    unsafe {
        let keyp = stcp_misc_ecdh_shared_key_new();
        let keys = stcp_misc_ecdh_shared_key_size() as usize;
        let rbuff: Vec<u8> = stcp_helper_get_heap_alloc_from(keyp, keys);
        rbuff
    }

}

pub fn stcp_helper_get_ecdh_public_key_new() -> Vec<u8> {
    unsafe {
        let keyp = stcp_misc_ecdh_public_key_new();
        let keys = stcp_misc_ecdh_public_key_size() as usize;
        let rbuff: Vec<u8> = stcp_helper_get_heap_alloc_from(keyp, keys);
        rbuff
    }
}

pub fn stcp_helper_get_ecdh_private_key_new() -> Vec<u8> {
    unsafe {
        let keyp = stcp_misc_ecdh_private_key_new();
        let keys = stcp_misc_ecdh_private_key_size() as usize;
        let rbuff: Vec<u8> = stcp_helper_get_heap_alloc_from(keyp, keys);
        rbuff
    }
}

