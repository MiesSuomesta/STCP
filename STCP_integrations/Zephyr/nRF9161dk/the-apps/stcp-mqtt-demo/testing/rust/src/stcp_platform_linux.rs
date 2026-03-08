use std::net::TcpStream;
use std::io::{Read, Write};
use std::io::ErrorKind;
use std::slice;

use core::ffi::{c_int, c_void};

use p256::{
    SecretKey,
    PublicKey,
    ecdh::diffie_hellman,
    elliptic_curve::sec1::ToEncodedPoint,
};

use rand::rngs::OsRng;

const EAGAIN: i32 = 11;
const EINVAL: i32 = 22;

#[repr(C)]
pub struct stcp_crypto_pubkey {
    pub x: [u8; 32],
    pub y: [u8; 32],
}

#[repr(C)]
pub struct stcp_crypto_secret {
    pub data: [u8; 32],
}

//
// TCP RECV
//

#[unsafe(no_mangle)]
pub extern "C" fn stcp_tcp_recv(
    sock_ptr: *mut c_void,
    buf: *mut u8,
    len: usize,
    _non_blocking: i32,
    _flags: u32,
    recv_len: *mut i32,
) -> isize {

    if sock_ptr.is_null() || buf.is_null() || recv_len.is_null() {
        return -EINVAL as isize;
    }

    let stream = unsafe { &mut *(sock_ptr as *mut TcpStream) };
    let slice = unsafe { slice::from_raw_parts_mut(buf, len) };

    match stream.read(slice) {

        Ok(n) => {
            unsafe { *recv_len = n as i32; }
            n as isize
        }

        Err(ref e) if e.kind() == ErrorKind::WouldBlock => {
            -EAGAIN as isize
        }

        Err(_) => {
            -1
        }
    }
}

//
// TCP SEND
//

#[unsafe(no_mangle)]
pub extern "C" fn stcp_tcp_send(
    sock_ptr: *mut c_void,
    buf: *const u8,
    len: usize,
) -> isize {

    if sock_ptr.is_null() || buf.is_null() {
        return -EINVAL as isize;
    }

    let stream = unsafe { &mut *(sock_ptr as *mut TcpStream) };
    let slice = unsafe { slice::from_raw_parts(buf, len) };

    match stream.write_all(slice) {

        Ok(_) => len as isize,

        Err(ref e) if e.kind() == ErrorKind::WouldBlock => {
            -EAGAIN as isize
        }

        Err(_) => {
            -1
        }
    }
}

//
// CRYPTO KEYPAIR
//

#[unsafe(no_mangle)]
pub extern "C" fn stcp_crypto_generate_keypair(
    out_pub: *mut stcp_crypto_pubkey,
    out_priv: *mut stcp_crypto_secret,
) -> i32 {

    if out_pub.is_null() || out_priv.is_null() {
        return -1;
    }

    let secret = SecretKey::random(&mut OsRng);
    let public = secret.public_key();

    let encoded = public.to_encoded_point(false);
    let bytes = encoded.as_bytes();

    unsafe {

        (*out_pub).x.copy_from_slice(&bytes[1..33]);
        (*out_pub).y.copy_from_slice(&bytes[33..65]);

        (*out_priv).data.copy_from_slice(&secret.to_bytes());
    }

    0
}

//
// CRYPTO SHARED SECRET
//

#[unsafe(no_mangle)]
pub extern "C" fn stcp_crypto_compute_shared(
    priv_key: *const stcp_crypto_secret,
    peer_pub: *const stcp_crypto_pubkey,
    out_shared: *mut stcp_crypto_secret,
) -> i32 {

    if priv_key.is_null() || peer_pub.is_null() || out_shared.is_null() {
        return -1;
    }

    unsafe {

        let mut full_pub = [0u8; 65];

        full_pub[0] = 0x04;
        full_pub[1..33].copy_from_slice(&(*peer_pub).x);
        full_pub[33..65].copy_from_slice(&(*peer_pub).y);

        let peer_public =
            PublicKey::from_sec1_bytes(&full_pub).unwrap();

        let secret =
            SecretKey::from_slice(&(*priv_key).data).unwrap();

        let shared = diffie_hellman(
            secret.to_nonzero_scalar(),
            peer_public.as_affine(),
        );

        (*out_shared)
            .data
            .copy_from_slice(shared.raw_secret_bytes().as_slice());
    }

    0
}

//
// LOGGER
//

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_log(
    level: c_int,
    buf: *const u8,
    len: usize,
) {

    if buf.is_null() || len == 0 {
        return;
    }

    let bytes = unsafe { slice::from_raw_parts(buf, len) };

    let msg = match std::str::from_utf8(bytes) {
        Ok(s) => s,
        Err(_) => {
            eprintln!("[STCP][{}] <non utf8>", level);
            return;
        }
    };

    let lvl = match level {
        0 => "ERR",
        1 => "WRN",
        2 => "INF",
        3 => "DBG",
        _ => "UNK",
    };

    eprintln!("[STCP][{}] {}", lvl, msg);
}