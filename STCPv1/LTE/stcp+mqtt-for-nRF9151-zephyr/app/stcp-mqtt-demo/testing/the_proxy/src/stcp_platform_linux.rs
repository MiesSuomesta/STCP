use std::net::TcpStream;
use std::io::{Read, Write};
use std::io::ErrorKind;
use std::slice;
use the_stcp_kernel_module::slice_helpers::StcpError;
use the_stcp_kernel_module::// stcp_dbg;
use core::ffi::{c_int, c_void};
use p256::SecretKey;
use p256::PublicKey;
use p256::ecdh::diffie_hellman;
use rand_core::OsRng;
use p256::elliptic_curve::sec1::ToEncodedPoint;

const EAGAIN: i32 = 11;
const EINVAL: i32 = 22;


#[unsafe(no_mangle)]
pub fn stcp_transport_wait_until_ready(seconds: i32) {
    println!("Wait for {} seconds here...", seconds);
}

unsafe extern "C" {
    pub fn stcp_rust_api_transport_get_fd(
        p: *mut c_void,
    ) -> i32;
}

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

        let peer_public = match PublicKey::from_sec1_bytes(&full_pub) {
            Ok(v) => v,
            Err(_) => return -1,
        };

        let secret = match SecretKey::from_slice(&(*priv_key).data) {
            Ok(v) => v,
            Err(_) => return -2,
        };

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