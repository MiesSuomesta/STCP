extern crate alloc;

use alloc::boxed::Box;
use alloc::vec::Vec;
use spin::Mutex;

pub use stcpcrypto::aes_lib::StcpAesCodec;
pub use stcptypes::types::*;

pub use crate::listener::StcpListener;
pub use crate::stream::StcpStream;

#[repr(C)]
pub struct StcpConnection {
    pub stream: Mutex<StcpStream>,
    pub aes: StcpAesCodec,
    pub shared_key: Vec<u8>,
}

#[repr(C)]
pub struct StcpServer {
    pub listener: StcpListener,
    pub port: u16,
    pub callback: ServerMessageProcessCB,
}

#[repr(C)]
pub struct ServerThreadContext {
    pub stream: Mutex<StcpStream>,
    pub cb: ServerMessageProcessCB,
}

