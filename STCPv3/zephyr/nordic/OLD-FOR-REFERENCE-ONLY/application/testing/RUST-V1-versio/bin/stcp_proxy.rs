use std::{
    ffi::c_void,
    io::{Read, Write},
    net::{Shutdown, TcpListener, TcpStream},
    os::fd::{AsRawFd, RawFd},
    ptr,
    slice,
    sync::{
        atomic::{AtomicBool, Ordering},
        Arc, Mutex,
    },
    thread,
    time::{Duration, Instant},
};

use chacha20poly1305::{
    aead::{AeadInPlace, KeyInit},
    ChaCha20Poly1305, Key, Nonce, Tag,
};
use rand_core::{OsRng, RngCore};
use stcp_kernel_core::{
    stcp_rust_carrier_receive,
    stcp_rust_create_accepted_stream,
    stcp_rust_is_connected,
    stcp_rust_recv,
    stcp_rust_release,
    stcp_rust_send,
    stcp_rust_start_handshake,
};
use x25519_dalek::{PublicKey, StaticSecret};

const STCP_PROTO_TCP: u8 = 253;
const LISTEN_ADDR: &str = "0.0.0.0:7777";
const MQTT_BACKEND: &str = "127.0.0.1:1883";
const HANDSHAKE_TIMEOUT: Duration = Duration::from_secs(10);

#[repr(C)]
struct UserspaceCarrier {
    fd: RawFd,
    tx_lock: Mutex<()>,
}

impl UserspaceCarrier {
    fn new(fd: RawFd) -> Self {
        Self {
            fd,
            tx_lock: Mutex::new(()),
        }
    }
}

fn errno() -> i32 {
    std::io::Error::last_os_error().raw_os_error().unwrap_or(libc::EIO)
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_carrier_needs_reliability(_carrier: *const c_void) -> bool {
    false
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_carrier_create_udp_child(
    _listener: *mut c_void,
    _child_rust_ctx: *mut c_void,
    _peer_addr: u32,
    _peer_port: u16,
) -> *mut c_void {
    ptr::null_mut()
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_carrier_destroy(_carrier: *mut c_void) {
    // TcpStream owns the fd. The proxy releases it after the Rust context.
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_wake_accept(_owner: *mut c_void) {}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_wake_recv(_owner: *mut c_void) {}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_debug_event(
    _event: u32,
    _ctx: usize,
    _arg0: usize,
    _arg1: usize,
) {
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_carrier_send(
    carrier: *mut c_void,
    data: *const u8,
    len: usize,
    flags: i32,
) -> isize {
    if carrier.is_null() || (data.is_null() && len != 0) {
        return -(libc::EINVAL as isize);
    }

    let carrier = unsafe { &*(carrier.cast::<UserspaceCarrier>()) };
    let _guard = match carrier.tx_lock.lock() {
        Ok(g) => g,
        Err(_) => return -(libc::EIO as isize),
    };

    let bytes = if len == 0 {
        &[]
    } else {
        unsafe { slice::from_raw_parts(data, len) }
    };

    let mut done = 0usize;
    while done < bytes.len() {
        let rc = unsafe {
            libc::send(
                carrier.fd,
                bytes[done..].as_ptr().cast(),
                bytes.len() - done,
                flags | libc::MSG_NOSIGNAL,
            )
        };

        if rc < 0 {
            let e = errno();
            if e == libc::EINTR {
                continue;
            }
            return -(e as isize);
        }

        if rc == 0 {
            return -(libc::EPIPE as isize);
        }

        done += rc as usize;
    }

    done as isize
}

fn nonce_bytes(nonce: u64) -> [u8; 12] {
    let mut out = [0u8; 12];
    out[4..].copy_from_slice(&nonce.to_le_bytes());
    out
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_x25519_keypair(
    secret: *mut u8,
    public_key: *mut u8,
) -> i32 {
    if secret.is_null() || public_key.is_null() {
        return -libc::EINVAL;
    }

    let mut secret_bytes = [0u8; 32];
    OsRng.fill_bytes(&mut secret_bytes);
    secret_bytes[0] &= 248;
    secret_bytes[31] &= 127;
    secret_bytes[31] |= 64;

    let private = StaticSecret::from(secret_bytes);
    let public = PublicKey::from(&private);

    unsafe {
        ptr::copy_nonoverlapping(secret_bytes.as_ptr(), secret, 32);
        ptr::copy_nonoverlapping(public.as_bytes().as_ptr(), public_key, 32);
    }
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_x25519_shared(
    shared: *mut u8,
    secret: *const u8,
    peer: *const u8,
) -> i32 {
    if shared.is_null() || secret.is_null() || peer.is_null() {
        return -libc::EINVAL;
    }

    let mut secret_bytes = [0u8; 32];
    let mut peer_bytes = [0u8; 32];
    unsafe {
        ptr::copy_nonoverlapping(secret, secret_bytes.as_mut_ptr(), 32);
        ptr::copy_nonoverlapping(peer, peer_bytes.as_mut_ptr(), 32);
    }

    let private = StaticSecret::from(secret_bytes);
    let public = PublicKey::from(peer_bytes);
    let result = private.diffie_hellman(&public);

    if result.as_bytes().iter().all(|b| *b == 0) {
        return -libc::EKEYREJECTED;
    }

    unsafe {
        ptr::copy_nonoverlapping(result.as_bytes().as_ptr(), shared, 32);
    }
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_chacha_encrypt(
    key: *const u8,
    nonce: u64,
    aad: *const u8,
    aad_len: usize,
    plain: *const u8,
    plain_len: usize,
    out: *mut u8,
    out_len: usize,
) -> i32 {
    if key.is_null() || out.is_null() || out_len < plain_len + 16 {
        return -libc::EINVAL;
    }
    if plain_len != 0 && plain.is_null() {
        return -libc::EINVAL;
    }
    if aad_len != 0 && aad.is_null() {
        return -libc::EINVAL;
    }

    let key_bytes = unsafe { slice::from_raw_parts(key, 32) };
    let aad_bytes = if aad_len == 0 { &[] } else { unsafe { slice::from_raw_parts(aad, aad_len) } };
    let plain_bytes = if plain_len == 0 { &[] } else { unsafe { slice::from_raw_parts(plain, plain_len) } };

    let cipher = ChaCha20Poly1305::new(Key::from_slice(key_bytes));
    let nonce_buf = nonce_bytes(nonce);
    let mut buffer = plain_bytes.to_vec();

    let tag = match cipher.encrypt_in_place_detached(
        Nonce::from_slice(&nonce_buf),
        aad_bytes,
        &mut buffer,
    ) {
        Ok(tag) => tag,
        Err(_) => return -libc::EIO,
    };

    unsafe {
        ptr::copy_nonoverlapping(buffer.as_ptr(), out, buffer.len());
        ptr::copy_nonoverlapping(tag.as_slice().as_ptr(), out.add(buffer.len()), 16);
    }
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_chacha_decrypt(
    key: *const u8,
    nonce: u64,
    aad: *const u8,
    aad_len: usize,
    cipher_text: *const u8,
    cipher_len: usize,
    out: *mut u8,
    out_len: usize,
) -> i32 {
    if key.is_null() || cipher_text.is_null() || out.is_null() || cipher_len < 16 {
        return -libc::EINVAL;
    }

    let plain_len = cipher_len - 16;
    if out_len < plain_len {
        return -libc::EINVAL;
    }

    let key_bytes = unsafe { slice::from_raw_parts(key, 32) };
    let aad_bytes = if aad_len == 0 { &[] } else { unsafe { slice::from_raw_parts(aad, aad_len) } };
    let input = unsafe { slice::from_raw_parts(cipher_text, cipher_len) };
    let mut buffer = input[..plain_len].to_vec();
    let tag = Tag::from_slice(&input[plain_len..]);
    let nonce_buf = nonce_bytes(nonce);

    let cipher = ChaCha20Poly1305::new(Key::from_slice(key_bytes));
    if cipher.decrypt_in_place_detached(
        Nonce::from_slice(&nonce_buf),
        aad_bytes,
        &mut buffer,
        tag,
    ).is_err() {
        return -libc::EBADMSG;
    }

    unsafe {
        ptr::copy_nonoverlapping(buffer.as_ptr(), out, buffer.len());
    }
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_chacha_decrypt_in_place(
    key: *const u8,
    nonce: u64,
    aad: *const u8,
    aad_len: usize,
    cipher_text: *mut u8,
    cipher_len: usize,
) -> i32 {
    if key.is_null() || cipher_text.is_null() || cipher_len < 16 {
        return -libc::EINVAL;
    }

    let plain_len = cipher_len - 16;
    let key_bytes = unsafe { slice::from_raw_parts(key, 32) };
    let aad_bytes = if aad_len == 0 { &[] } else { unsafe { slice::from_raw_parts(aad, aad_len) } };
    let input = unsafe { slice::from_raw_parts_mut(cipher_text, cipher_len) };
    let (body, tag_bytes) = input.split_at_mut(plain_len);
    let tag = Tag::clone_from_slice(tag_bytes);
    let nonce_buf = nonce_bytes(nonce);

    let cipher = ChaCha20Poly1305::new(Key::from_slice(key_bytes));
    if cipher.decrypt_in_place_detached(
        Nonce::from_slice(&nonce_buf),
        aad_bytes,
        body,
        &tag,
    ).is_err() {
        return -libc::EBADMSG;
    }
    0
}

fn wait_connected(ctx: *mut c_void) -> Result<(), String> {
    let deadline = Instant::now() + HANDSHAKE_TIMEOUT;
    loop {
        let rc = stcp_rust_is_connected(ctx);
        if rc == 1 {
            return Ok(());
        }
        if rc < 0 {
            return Err(format!("stcp_rust_is_connected rc={rc}"));
        }
        if Instant::now() >= deadline {
            return Err("STCP handshake timeout".to_string());
        }
        thread::sleep(Duration::from_millis(5));
    }
}

fn rx_worker(mut stream: TcpStream, rust_ctx: usize, running: Arc<AtomicBool>) {
    let ctx = rust_ctx as *mut c_void;
    let mut buf = [0u8; 4096];

    while running.load(Ordering::Acquire) {
        match stream.read(&mut buf) {
            Ok(0) => break,
            Ok(n) => {
                let rc = stcp_rust_carrier_receive(ctx, buf.as_ptr(), n);
                if rc < 0 {
                    eprintln!("[proxy] STCP carrier RX rc={rc}");
                    break;
                }
            }
            Err(ref e) if e.kind() == std::io::ErrorKind::Interrupted => continue,
            Err(ref e) if e.kind() == std::io::ErrorKind::WouldBlock => {
                thread::sleep(Duration::from_millis(2));
            }
            Err(e) => {
                eprintln!("[proxy] carrier recv failed: {e}");
                break;
            }
        }
    }

    running.store(false, Ordering::Release);
}

fn handle_client(stream: TcpStream) {
    let peer = stream.peer_addr().ok();
    println!("[proxy] carrier connected from {:?}", peer);

    stream.set_nodelay(true).ok();

    let rx_stream = match stream.try_clone() {
        Ok(s) => s,
        Err(e) => {
            eprintln!("[proxy] clone failed: {e}");
            return;
        }
    };

    let carrier = Box::new(UserspaceCarrier::new(stream.as_raw_fd()));
    let carrier_ptr = Box::into_raw(carrier);

    let mut rust_ctx: *mut c_void = ptr::null_mut();
    let rc = stcp_rust_create_accepted_stream(
        STCP_PROTO_TCP,
        carrier_ptr.cast(),
        &mut rust_ctx,
    );
    if rc < 0 || rust_ctx.is_null() {
        eprintln!("[proxy] stcp_rust_create_accepted_stream rc={rc}");
        unsafe { drop(Box::from_raw(carrier_ptr)); }
        return;
    }

    let running = Arc::new(AtomicBool::new(true));
    let rx_running = running.clone();
    let ctx_for_thread = rust_ctx as usize;
    let rx_thread = thread::spawn(move || rx_worker(rx_stream, ctx_for_thread, rx_running));

    let hs = stcp_rust_start_handshake(rust_ctx);
    if hs < 0 {
        eprintln!("[proxy] handshake start rc={hs}");
        running.store(false, Ordering::Release);
    } else if let Err(e) = wait_connected(rust_ctx) {
        eprintln!("[proxy] {e}");
        running.store(false, Ordering::Release);
    } else {
        println!("[proxy] STCP handshake complete");

        match TcpStream::connect(MQTT_BACKEND) {
            Ok(mut backend) => {
                backend.set_nodelay(true).ok();
                backend.set_read_timeout(Some(Duration::from_millis(20))).ok();

                let mut stcp_buf = [0u8; 8192];
                let mut mqtt_buf = [0u8; 8192];

                while running.load(Ordering::Acquire) {
                    let n = stcp_rust_recv(
                        rust_ctx,
                        stcp_buf.as_mut_ptr(),
                        stcp_buf.len(),
                        0,
                    );

                    if n > 0 {
                        if let Err(e) = backend.write_all(&stcp_buf[..n as usize]) {
                            eprintln!("[proxy] Mosquitto write failed: {e}");
                            break;
                        }
                    } else if n < 0 && n != -(libc::EAGAIN as isize) {
                        eprintln!("[proxy] STCP recv rc={n}");
                        break;
                    }

                    match backend.read(&mut mqtt_buf) {
                        Ok(0) => {
                            println!("[proxy] Mosquitto closed");
                            break;
                        }
                        Ok(n) => {
                            let sent = stcp_rust_send(
                                rust_ctx,
                                mqtt_buf.as_ptr(),
                                n,
                                0,
                            );
                            if sent < 0 {
                                eprintln!("[proxy] STCP send rc={sent}");
                                break;
                            }
                        }
                        Err(ref e)
                            if e.kind() == std::io::ErrorKind::WouldBlock
                                || e.kind() == std::io::ErrorKind::TimedOut => {}
                        Err(e) => {
                            eprintln!("[proxy] Mosquitto read failed: {e}");
                            break;
                        }
                    }

                    thread::sleep(Duration::from_millis(2));
                }
            }
            Err(e) => eprintln!("[proxy] Mosquitto connect failed: {e}"),
        }
    }

    running.store(false, Ordering::Release);
    let _ = stream.shutdown(Shutdown::Both);
    let _ = rx_thread.join();

    unsafe {
        stcp_rust_release(rust_ctx);
        drop(Box::from_raw(carrier_ptr));
    }

    println!("[proxy] client disconnected");
}

fn main() -> std::io::Result<()> {
    let listener = TcpListener::bind(LISTEN_ADDR)?;

    println!("STCPv2 MQTT proxy");
    println!("  listen  : {LISTEN_ADDR}");
    println!("  backend : {MQTT_BACKEND}");
    println!("  core    : ../../../module/rust (stcp-kernel-core)");

    for conn in listener.incoming() {
        match conn {
            Ok(stream) => {
                thread::spawn(move || handle_client(stream));
            }
            Err(e) => eprintln!("[proxy] accept error: {e}"),
        }
    }

    Ok(())
}
