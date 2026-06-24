use std::ffi::c_void;
use std::net::TcpStream;

pub struct ProxyConnection {
    pub transport: *mut c_void,
    pub mqtt_socket: TcpStream,
}

pub const STCP_RECV_FLAG_EXACT_MODE: i32        = (1 << 20);
pub const STCP_RECV_FLAG_NON_BLOCKING: i32      = (1 << 21);
