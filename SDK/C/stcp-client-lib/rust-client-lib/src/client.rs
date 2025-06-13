
use std::ptr;
use stcpcommon::aes_lib::StcpAesCodec;
use stcpcommon::stcp_elliptic_codec::StcpEllipticCodec;
use stcpcommon::utils::StcpUtils;
use stcpcommon::defines_etc::StcpConnection;
use tokio::net::TcpStream;
use tokio::io::AsyncWriteExt;
use tokio::sync::Mutex;
use std::net::{IpAddr, SocketAddr};

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
        let rt = match tokio::runtime::Runtime::new() {
            Ok(rt) => rt,
            Err(_) => return ptr::null_mut(),
        };

        let result = rt.block_on(async {
            let ec = StcpEllipticCodec::new();
            let aes = StcpAesCodec::new();
            let util = StcpUtils::new();

            let ip: IpAddr = addr_str.parse().ok()?;

            let mut stream = match TcpStream::connect(SocketAddr::new(ip, port)).await {
                Ok(s) => s,
                Err(_) => return None,
            };

            let (_peer_key, shared_key) = util.do_the_stcp_handshake_client(&mut stream, ec).await;
            if shared_key.is_empty() {
                return None;
            }

            Some(Box::new(StcpConnection {
                stream: Mutex::new(stream),
                aes,
                shared_key,
            }))
        });

        match result {
            Some(conn) => Box::into_raw(conn),
            None => ptr::null_mut(),
        }
}

pub fn stcp_client_internal_send(conn: *mut StcpConnection, data: *const u8, len: usize) -> Result<usize, i32> {
    if conn.is_null() || data.is_null() || len == 0 {
        return Err(-2);
    }

    let msg = unsafe { std::slice::from_raw_parts(data, len).to_vec() };
    let conn = unsafe { &*conn };

    let rt = tokio::runtime::Runtime::new().unwrap();
    let rv : usize = 0;
    let ec : i32 = 0;

    rt.block_on(async {
        let mut guard = conn.stream.lock().await;
        let stream = &mut *guard;

        let encrypted = conn.aes.handle_aes_outgoing_message(&msg, &conn.shared_key);

        stream
            .write_all(&encrypted)
            .await
            .map(|_| encrypted.len())   // Ok(n)
            .map_err(|_| -2)            
    })
}

pub fn stcp_client_internal_recv(conn: *mut StcpConnection, buf: *mut u8, max_len: usize) -> Result<usize, i32> {
    if conn.is_null() || buf.is_null() || max_len == 0 {
        return Err(-2);
    }

    let conn = unsafe { &*conn };

    let rt = tokio::runtime::Runtime::new().unwrap();

    rt.block_on(async {
        let mut guard = conn.stream.lock().await;
        let stream = &mut *guard;

        let utils = StcpUtils::new();

        // Tässä odotetaan viestiä:
	let (_, encrypted) = utils.read_from(stream).await;

        let decrypted = conn.aes.handle_aes_incoming_message(&encrypted, &conn.shared_key);

        let to_copy = std::cmp::min(max_len, decrypted.len());

        unsafe {
            std::ptr::copy_nonoverlapping(decrypted.as_ptr(), buf, to_copy);
        }

        Ok(to_copy)
    })
}

pub fn stcp_client_internal_disconnect(conn: *mut StcpConnection) {

    if conn.is_null() {
        return;
    }

    unsafe {
        drop(Box::from_raw(conn));
    }

}

