
use tokio::net::TcpStream;
use tokio::net::TcpListener;
use tokio::sync::Mutex;
use crate::aes_lib::StcpAesCodec;

#[repr(C)]
pub struct StcpConnection {
    pub stream: Mutex<TcpStream>,
    pub aes: StcpAesCodec,
    pub shared_key: Vec<u8>,
}


#[repr(C)]
pub struct StcpServer {
    pub listener: TcpListener,
    pub port: u16,
    pub callback: ServerMessageProcessCB,
}

pub type ServerMessageProcessCB = extern "C" fn(
    input_ptr: *const u8,
    input_len: usize,
    output_buf: *mut u8,
    max_output_len: usize,
    actual_output_len: *mut usize,
);

// Protokollan maksimi paketin koko (IPv4)
pub const STCP_IPv4_PACKET_PAYLOAD_MAX_SIZE: usize = (1024 * 64) - 1;
pub const STCP_IPv4_PACKET_HEADERS_MAX_SIZE: usize = 60; // 60 bytes
pub const STCP_IPv4_PACKET_MAX_SIZE: usize = STCP_IPv4_PACKET_PAYLOAD_MAX_SIZE - STCP_IPv4_PACKET_HEADERS_MAX_SIZE;
