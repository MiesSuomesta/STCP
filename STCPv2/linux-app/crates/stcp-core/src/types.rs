use std::os::fd::RawFd;
use crate::crypto::CryptoContext;
use std::net::{SocketAddr};

#[derive(Debug)]
pub struct StcpContext {
    pub sk: RawFd,
    pub crypto: CryptoContext,
    pub rx_payload_buf: Vec<u8>,
}

#[derive(Debug)]
pub enum StcpState {
    New(StcpContext),
    Bound { ctx: StcpContext, at_where: SocketAddr },
    Listening { ctx: StcpContext, at_where: SocketAddr },
    Connected { ctx: StcpContext, to_where: SocketAddr },
    Handshake { ctx: StcpContext },
    Ready { ctx: StcpContext },
    Closing { ctx: StcpContext, reason: String },
    Closed { ctx: StcpContext, reason: String },
    Error { ctx: StcpContext, reason: String },
}
