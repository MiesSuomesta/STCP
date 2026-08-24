use std::{
    ffi::c_void,
    io::Read,
    net::{IpAddr, SocketAddr, TcpListener, TcpStream},
    os::fd::{AsRawFd, RawFd},
    ptr,
    slice,
    sync::Mutex,
    time::{Duration, Instant},
};

use chacha20poly1305::{
    aead::{AeadInPlace, KeyInit},
    ChaCha20Poly1305, Key, Nonce, Tag,
};
use rand_core::{OsRng, RngCore};
use hmac::{Hmac, Mac};
use sha2::{Digest, Sha256};
use stcp_kernel_core::{
    stcp_rust_bind,
    stcp_rust_create,
    stcp_rust_create_external_tcp_child,
    stcp_rust_is_connected,
    stcp_rust_listen,
    stcp_rust_recv,
    stcp_rust_release,
    stcp_rust_send,
    stcp_rust_set_carrier,
    stcp_rust_start_handshake,
};
use x25519_dalek::{PublicKey, StaticSecret};

const STCP_PROTO_TCP: u8 = 253;
const DEFAULT_LISTEN: &str = "0.0.0.0:19000";
const HANDSHAKE_TIMEOUT: Duration = Duration::from_secs(10);

unsafe extern "C" {
    fn stcp_rust_carrier_receive_from(
        ctx: *mut c_void,
        data: *const u8,
        len: usize,
        peer_addr: u32,
        peer_port: u16,
    ) -> i32;
}

#[repr(C)]
struct UserspaceCarrier {
    fd: RawFd,
    tx_lock: Mutex<()>,
}

impl UserspaceCarrier {
    fn new(fd: RawFd) -> Self {
        Self { fd, tx_lock: Mutex::new(()) }
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
pub extern "C" fn stcp_carrier_destroy(_carrier: *mut c_void) {}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_wake_accept(_owner: *mut c_void) {}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_wake_recv(_owner: *mut c_void) {}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_debug_event(
    _event: u32, _ctx: usize, _arg0: usize, _arg1: usize,
) {}

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

    let carrier = unsafe { &*carrier.cast::<UserspaceCarrier>() };
    let _guard = match carrier.tx_lock.lock() {
        Ok(g) => g,
        Err(_) => return -(libc::EIO as isize),
    };

    let bytes = if len == 0 { &[] } else { unsafe { slice::from_raw_parts(data, len) } };
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
            if e == libc::EINTR { continue; }
            return -(e as isize);
        }
        if rc == 0 { return -(libc::EPIPE as isize); }
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
pub extern "C" fn stcp_kernel_x25519_keypair(secret: *mut u8, public_key: *mut u8) -> i32 {
    if secret.is_null() || public_key.is_null() { return -libc::EINVAL; }

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
    shared: *mut u8, secret: *const u8, peer: *const u8,
) -> i32 {
    if shared.is_null() || secret.is_null() || peer.is_null() { return -libc::EINVAL; }

    let mut secret_bytes = [0u8; 32];
    let mut peer_bytes = [0u8; 32];
    unsafe {
        ptr::copy_nonoverlapping(secret, secret_bytes.as_mut_ptr(), 32);
        ptr::copy_nonoverlapping(peer, peer_bytes.as_mut_ptr(), 32);
    }

    let private = StaticSecret::from(secret_bytes);
    let public = PublicKey::from(peer_bytes);
    let result = private.diffie_hellman(&public);
    if result.as_bytes().iter().all(|b| *b == 0) { return -libc::EKEYREJECTED; }

    unsafe { ptr::copy_nonoverlapping(result.as_bytes().as_ptr(), shared, 32); }
    0
}


#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_derive_session_keys(
    shared: *const u8,
    client_pub: *const u8,
    server_pub: *const u8,
    client_to_server: *mut u8,
    server_to_client: *mut u8,
) -> i32 {
    if shared.is_null()
        || client_pub.is_null()
        || server_pub.is_null()
        || client_to_server.is_null()
        || server_to_client.is_null()
    {
        return -libc::EINVAL;
    }

    let shared = unsafe { slice::from_raw_parts(shared, 32) };
    let client_pub = unsafe { slice::from_raw_parts(client_pub, 32) };
    let server_pub = unsafe { slice::from_raw_parts(server_pub, 32) };

    /*
     * Keep this byte-for-byte compatible with the canonical platform KDF:
     *
     * salt = SHA256("STCPv2-HKDF-SHA256" || client_pub || server_pub)
     * prk  = HMAC-SHA256(salt, shared)
     * c2s  = HMAC-SHA256(prk, "STCPv2 client to server key" || 0x01)
     * s2c  = HMAC-SHA256(prk, "STCPv2 server to client key" || 0x01)
     */
    let mut hasher = Sha256::new();
    hasher.update(b"STCPv2-HKDF-SHA256");
    hasher.update(client_pub);
    hasher.update(server_pub);
    let salt = hasher.finalize();

    type HmacSha256 = Hmac<Sha256>;

    let mut extract = match <HmacSha256 as Mac>::new_from_slice(&salt) {
        Ok(v) => v,
        Err(_) => return -libc::EIO,
    };
    extract.update(shared);
    let prk = extract.finalize().into_bytes();

    let mut c2s_mac = match <HmacSha256 as Mac>::new_from_slice(&prk) {
        Ok(v) => v,
        Err(_) => return -libc::EIO,
    };
    c2s_mac.update(b"STCPv2 client to server key");
    c2s_mac.update(&[1u8]);
    let c2s = c2s_mac.finalize().into_bytes();

    let mut s2c_mac = match <HmacSha256 as Mac>::new_from_slice(&prk) {
        Ok(v) => v,
        Err(_) => return -libc::EIO,
    };
    s2c_mac.update(b"STCPv2 server to client key");
    s2c_mac.update(&[1u8]);
    let s2c = s2c_mac.finalize().into_bytes();

    if c2s.as_slice() == s2c.as_slice() {
        return -libc::EKEYREJECTED;
    }

    unsafe {
        ptr::copy_nonoverlapping(c2s.as_ptr(), client_to_server, 32);
        ptr::copy_nonoverlapping(s2c.as_ptr(), server_to_client, 32);
    }

    0
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_chacha_encrypt(
    key: *const u8, nonce: u64,
    aad: *const u8, aad_len: usize,
    plain: *const u8, plain_len: usize,
    out: *mut u8, out_len: usize,
) -> i32 {
    if key.is_null() || out.is_null() || out_len < plain_len + 16 { return -libc::EINVAL; }
    if plain_len != 0 && plain.is_null() { return -libc::EINVAL; }
    if aad_len != 0 && aad.is_null() { return -libc::EINVAL; }

    let key_bytes = unsafe { slice::from_raw_parts(key, 32) };
    let aad_bytes = if aad_len == 0 { &[] } else { unsafe { slice::from_raw_parts(aad, aad_len) } };
    let plain_bytes = if plain_len == 0 { &[] } else { unsafe { slice::from_raw_parts(plain, plain_len) } };

    let cipher = ChaCha20Poly1305::new(Key::from_slice(key_bytes));
    let nonce_buf = nonce_bytes(nonce);
    let mut buffer = plain_bytes.to_vec();

    let tag = match cipher.encrypt_in_place_detached(
        Nonce::from_slice(&nonce_buf), aad_bytes, &mut buffer
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
    key: *const u8, nonce: u64,
    aad: *const u8, aad_len: usize,
    cipher_text: *const u8, cipher_len: usize,
    out: *mut u8, out_len: usize,
) -> i32 {
    if key.is_null() || cipher_text.is_null() || out.is_null() || cipher_len < 16 {
        return -libc::EINVAL;
    }

    let plain_len = cipher_len - 16;
    if out_len < plain_len { return -libc::EINVAL; }

    let key_bytes = unsafe { slice::from_raw_parts(key, 32) };
    let aad_bytes = if aad_len == 0 { &[] } else { unsafe { slice::from_raw_parts(aad, aad_len) } };
    let input = unsafe { slice::from_raw_parts(cipher_text, cipher_len) };
    let mut buffer = input[..plain_len].to_vec();
    let tag = Tag::from_slice(&input[plain_len..]);
    let nonce_buf = nonce_bytes(nonce);

    let cipher = ChaCha20Poly1305::new(Key::from_slice(key_bytes));
    if cipher.decrypt_in_place_detached(
        Nonce::from_slice(&nonce_buf), aad_bytes, &mut buffer, tag
    ).is_err() {
        return -libc::EBADMSG;
    }

    unsafe { ptr::copy_nonoverlapping(buffer.as_ptr(), out, buffer.len()); }
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_chacha_decrypt_in_place(
    key: *const u8, nonce: u64,
    aad: *const u8, aad_len: usize,
    cipher_text: *mut u8, cipher_len: usize,
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
        Nonce::from_slice(&nonce_buf), aad_bytes, body, &tag
    ).is_err() {
        return -libc::EBADMSG;
    }
    0
}

fn ipv4_u32(addr: SocketAddr) -> u32 {
    match addr.ip() {
        IpAddr::V4(ip) => u32::from_be_bytes(ip.octets()),
        IpAddr::V6(_) => 0,
    }
}

fn create_listener(port: u16) -> Result<*mut c_void, String> {
    let mut ctx = ptr::null_mut();
    let rc = stcp_rust_create(STCP_PROTO_TCP, &mut ctx);
    if rc < 0 || ctx.is_null() {
        return Err(format!("stcp_rust_create rc={rc}"));
    }

    let rc = stcp_rust_bind(ctx, 0, port);
    if rc < 0 {
        unsafe { stcp_rust_release(ctx) };
        return Err(format!("stcp_rust_bind rc={rc}"));
    }

    let rc = stcp_rust_listen(ctx, 1);
    if rc < 0 {
        unsafe { stcp_rust_release(ctx) };
        return Err(format!("stcp_rust_listen rc={rc}"));
    }

    Ok(ctx)
}

fn run_connection(listener_ctx: *mut c_void, mut stream: TcpStream) -> Result<(), String> {
    let local = stream.local_addr().map_err(|e| e.to_string())?;
    let peer = stream.peer_addr().map_err(|e| e.to_string())?;
    stream.set_nodelay(true).ok();
    stream.set_read_timeout(Some(Duration::from_millis(20))).map_err(|e| e.to_string())?;

    let carrier = Box::new(UserspaceCarrier::new(stream.as_raw_fd()));
    let carrier_ptr = Box::into_raw(carrier);

    let mut ctx = ptr::null_mut();
    let rc = stcp_rust_create_external_tcp_child(
        listener_ctx,
        ipv4_u32(local),
        local.port(),
        ipv4_u32(peer),
        peer.port(),
        &mut ctx,
    );
    if rc < 0 || ctx.is_null() {
        unsafe { drop(Box::from_raw(carrier_ptr)); }
        return Err(format!("stcp_rust_create_external_tcp_child rc={rc}"));
    }

    stcp_rust_set_carrier(ctx, carrier_ptr.cast());

    let result = (|| {
        let rc = stcp_rust_start_handshake(ctx);
        if rc < 0 {
            return Err(format!("stcp_rust_start_handshake rc={rc}"));
        }

        let deadline = Instant::now() + HANDSHAKE_TIMEOUT;
        let mut wire = [0u8; 16384];
        let mut plain = [0u8; 16384];

        while stcp_rust_is_connected(ctx) != 1 {
            if Instant::now() >= deadline {
                return Err("STCPv2 handshake timeout".into());
            }
            match stream.read(&mut wire) {
                Ok(0) => return Err("peer closed during handshake".into()),
                Ok(n) => {
                    let rc = unsafe { stcp_rust_carrier_receive_from(ctx, wire.as_ptr(), n, 0, 0) };
                    if rc < 0 {
                        return Err(format!("stcp_rust_carrier_receive_from rc={rc}"));
                    }
                }
                Err(ref e) if e.kind() == std::io::ErrorKind::Interrupted => continue,
                Err(ref e) if e.kind() == std::io::ErrorKind::WouldBlock
                    || e.kind() == std::io::ErrorKind::TimedOut => {}
                Err(e) => return Err(format!("carrier read: {e}")),
            }
        }

        println!("[server] STCPv2 connected: {peer}");

        loop {
            loop {
                let n = stcp_rust_recv(ctx, plain.as_mut_ptr(), plain.len(), 0);
                if n > 0 {
                    let n = n as usize;
                    let sent = stcp_rust_send(ctx, plain.as_ptr(), n, 0);
                    if sent < 0 {
                        return Err(format!("stcp_rust_send rc={sent}"));
                    }
                    if sent as usize != n {
                        return Err(format!("short STCP send: {sent}/{n}"));
                    }
                    continue;
                }
                if n == -(libc::EAGAIN as isize) {
                    break;
                }
                if n < 0 {
                    return Err(format!("stcp_rust_recv rc={n}"));
                }
                break;
            }

            match stream.read(&mut wire) {
                Ok(0) => break,
                Ok(n) => {
                    let rc = unsafe { stcp_rust_carrier_receive_from(ctx, wire.as_ptr(), n, 0, 0) };
                    if rc < 0 {
                        return Err(format!("stcp_rust_carrier_receive_from rc={rc}"));
                    }
                }
                Err(ref e) if e.kind() == std::io::ErrorKind::Interrupted => continue,
                Err(ref e) if e.kind() == std::io::ErrorKind::WouldBlock
                    || e.kind() == std::io::ErrorKind::TimedOut => {}
                Err(e) => return Err(format!("carrier read: {e}")),
            }
        }
        Ok(())
    })();

    unsafe { stcp_rust_release(ctx); }
    unsafe { drop(Box::from_raw(carrier_ptr)); }
    result
}

fn main() -> Result<(), String> {
    let listen = std::env::var("STCP_LISTEN").unwrap_or_else(|_| DEFAULT_LISTEN.into());
    let socket_addr: SocketAddr = listen.parse().map_err(|e| format!("bad STCP_LISTEN: {e}"))?;
    let listener = TcpListener::bind(socket_addr).map_err(|e| e.to_string())?;
    let listener_ctx = create_listener(socket_addr.port())?;

    println!("STCPv2 shared-core echo server");
    println!("  listen : {listen}");
    println!("  core   : kernel/module/rust");
    println!("  model  : one TCP connection at a time");

    for conn in listener.incoming() {
        match conn {
            Ok(stream) => {
                if let Err(e) = run_connection(listener_ctx, stream) {
                    eprintln!("[server] connection failed: {e}");
                } else {
                    println!("[server] client disconnected");
                }
            }
            Err(e) => eprintln!("[server] accept error: {e}"),
        }
    }

    unsafe { stcp_rust_release(listener_ctx); }
    Ok(())
}
