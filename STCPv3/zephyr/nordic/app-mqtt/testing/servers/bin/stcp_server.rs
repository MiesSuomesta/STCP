use std::net::{TcpListener, TcpStream};
use std::thread;
use the_stcp_kernel_module::stcp_message::build_frame_and_send;
use the_stcp_kernel_module::proto_session::ProtoSession;
use the_stcp_kernel_module::debug::*;
use the_stcp_kernel_module::debug;
use the_stcp_kernel_module::stcp_dbg;
use the_stcp_kernel_module::stcp_dump;
use the_stcp_kernel_module::errorit::*;
use the_stcp_kernel_module::stcp_handshake;
use the_stcp_kernel_module::types::kernel_socket;
use the_stcp_kernel_module::slice_helpers::StcpError;
use std::time::Duration;
use std::thread::sleep;


use std::io::ErrorKind;

use core::ptr;
use core::ffi::c_int;
use core::ffi::c_void;
use std::io::{Read, Write};
use std::os::unix::io::{AsRawFd, FromRawFd};
use std::panic::Location;
use p256::{
    SecretKey,
    PublicKey,
    ecdh::diffie_hellman,
    elliptic_curve::sec1::ToEncodedPoint,
};

use rand::rngs::OsRng;

#[repr(C)]
pub struct stcp_crypto_pubkey {
    pub x: [u8; 32],
    pub y: [u8; 32],
}

#[repr(C)]
pub struct stcp_crypto_secret {
    pub data: [u8; 32],
}

/*
 * NET functions for x86
 */
const FORWARD_HOST: &str = "127.0.0.1";
const FORWARD_PORT: u16 = 1883;

#[unsafe(no_mangle)]
pub extern "C" fn stcp_tcp_recv(
    sock_ptr: *mut std::ffi::c_void,
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
    let slice = unsafe { std::slice::from_raw_parts_mut(buf, len) };

    match stream.read(slice) {
        Ok(n) => {
            unsafe { *recv_len = n as i32 };
            n as isize
        }
        Err(e) => {
            -e.raw_os_error().unwrap_or(EAGAIN) as isize
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_tcp_send(
    sock_ptr: *mut std::ffi::c_void,
    buf: *const u8,
    len: usize,
) -> isize {

    if sock_ptr.is_null() || buf.is_null() {
        return -EINVAL as isize;
    }

    let stream = unsafe { &mut *(sock_ptr as *mut TcpStream) };
    let slice = unsafe { std::slice::from_raw_parts(buf, len) };
    stcp_dump!("x86 TX buffer", &slice);
    match stream.write(slice) {
        Ok(n) => n as isize,
        Err(e) => -e.raw_os_error().unwrap_or(-EAGAIN) as isize,
    }
}

/*
 * Crypto for x86 ....
 */

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
    let encoded = public.to_encoded_point(false); // uncompressed

    let pub_bytes = encoded.as_bytes(); // 65 bytes: 0x04 || X || Y

    unsafe {
        // X
        (*out_pub).x.copy_from_slice(&pub_bytes[1..33]);

        // Y
        (*out_pub).y.copy_from_slice(&pub_bytes[33..65]);

        // private
        (*out_priv).data.copy_from_slice(&secret.to_bytes());
    }

    0
}

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

        // Rakenna 0x04 || X || Y
        let mut full_pub = [0u8; 65];
        full_pub[0] = 0x04;
        full_pub[1..33].copy_from_slice(&(*peer_pub).x);
        full_pub[33..65].copy_from_slice(&(*peer_pub).y);

        let peer_public = PublicKey::from_sec1_bytes(&full_pub).unwrap();

        let secret = SecretKey::from_slice(&(*priv_key).data).unwrap();

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

/*
 * Logger function for x86
 */
 
#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_log(level: c_int, buf: *const u8, len: usize) {
      if buf.is_null() || len == 0 {
        return;
    }

    let bytes = unsafe { core::slice::from_raw_parts(buf, len) };

    // Yritetään tulostaa UTF-8:na, fallback hexiin
    let msg = match core::str::from_utf8(bytes) {
        Ok(s) => s,
        Err(_) => {
            eprintln!("[STCP][{:?}] <non-utf8 {} bytes>", level, len);
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


/*
 * Main functions for x86
 */

fn main() {
    let args: Vec<String> = env::args().collect();

    let aes_enabled = if args.len() > 1 {
        match args[1].as_str() {
            "aes" => true,
            "plain" => false,
            _ => {
                println!("Usage: ./stcp_server [aes|plain]");
                return;
            }
        }
    } else {
        true // default: AES ON
    };

    let listener = TcpListener::bind("0.0.0.0:7777").unwrap();

    println!("=========================================");
    println!(" STCP Rust Test Server");
    println!(" Listening on 0.0.0.0:7777");
    println!(" AES MODE: {}", if aes_enabled { "ENABLED" } else { "DISABLED" });
    println!(" Forward to: {}:{}", FORWARD_HOST, FORWARD_PORT);
    println!("=========================================");

    for stream in listener.incoming() {
        match stream {
            Ok(stream) => {
                thread::spawn({
                    let aes_enabled = aes_enabled;
                    move || handle_client(stream, aes_enabled)
                });
            }
            Err(e) => {
                println!("Connection error: {e:?}");
            }
        }
    }
}

use std::net::Shutdown;


pub fn handle_client(mut stream: TcpStream, aes_enabled: bool) -> Result<(), StcpError> {

    let transport = &mut stream as *mut _ as *mut c_void;

    println!("=========================================");
    println!(" New STCP client: {:?}", stream.peer_addr());
    println!(" Transport ptr: {:?}", transport);
    println!("=========================================");

    let mut session = ProtoSession::new(true, transport);
    session.set_aes_mode(aes_enabled);
    // ===============================
    // HANDSHAKE
    // ===============================
    loop {
        let hsret = crate::stcp_handshake::rust_session_server_handshake_lte(
            &mut session as *mut ProtoSession,
            transport as *mut kernel_socket,
        );

        if hsret == 1 {
            stcp_dbg!("=== HANDSHAKE COMPLETE ===");
            break;
        }

        if hsret == -EAGAIN {
            continue;
        }

        stcp_dbg!("Handshake failed: {}", hsret);
        return Err(StcpError::Invalid);
    }

    println!("🔐 AES session established.");

    // ===============================
    // CONNECT BACKEND (X:Y)
    // ===============================
    let mut backend = TcpStream::connect((FORWARD_HOST, FORWARD_PORT))
        .map_err(|_| StcpError::Invalid)?;

    backend.set_nodelay(true).ok();
    stream.set_nodelay(true).ok();
    backend.set_nonblocking(true).ok();
    stream.set_nonblocking(true).ok();

    // tee non-blocking jotta ei jumitu

    println!("🔁 Forwarding to {}:{}", FORWARD_HOST, FORWARD_PORT);

    let mut backend_buf = [0u8; 4096];

    loop {

        // ===============================
        // STCP -> BACKEND
        // ===============================
        match session.recv_message() {
            Ok(data) => {
                if !data.is_empty() {
                    stcp_dbg!("Forwarding {} bytes to Mosquitto", data.len());
                    stcp_dump!("Forwarding data", &data);
                    match backend.write(&data) {

                        Ok(n) => {
                            stcp_dbg!("Forwarded {} bytes to backend", n);
                        }

                        Err(ref e) if e.kind() == ErrorKind::WouldBlock => {
                            stcp_dbg!("Backend would block");
                        }

                        Err(e) => {
                            stcp_dbg!("Backend write error: {:?}", e);
                            break;
                        }
                    }
                }
            }
            Err(StcpError::Again) => {
                stcp_dbg!("STCP recv again..");
            }
            Err(StcpError::WouldBlock) => {
                stcp_dbg!("STCP recv would block..");
            }
            Err(StcpError::Invalid) => {}
            Err(e) => {
                stcp_dbg!("STCP recv error: {:?}", e);
                break;
            }
        }

        // ===============================
        // BACKEND -> STCP
        // ===============================
        match backend.read(&mut backend_buf) {
            Ok(0) => {
                stcp_dbg!("Backend closed");
                break;
            }
            Ok(n) => {
                stcp_dbg!("MOSQUITTO REPLY {} bytes", n);
                stcp_dbg!("{:02x?}", &backend_buf[..n]);
                stcp_dbg!("Forwarding {} bytes back to STCP", n);
                if session.send_message(&backend_buf[..n]).is_err() {
                    stcp_dbg!("STCP send failed");
                    break;
                }
                stcp_dbg!("Sent {} bytes over STCP", n);
            }
            Err(ref e) if e.kind() == ErrorKind::WouldBlock => {}
            Err(e) => {
                stcp_dbg!("Backend read error: {:?}", e);
                break;
            }
        }

        sleep(Duration::from_millis(1));
    }
    stcp_dbg!("🔌 Proxy closed.");
    Ok(())
}
