use alloc::{boxed::Box, collections::VecDeque, vec::Vec};
use crate::crypto::CryptoContext;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum StcpState { New, Bound, Listening, Connected, Handshake, Ready, Closing, Closed, Error }

pub struct StcpContext {
    pub state: StcpState,
    pub proto_id: u8,
    pub local_addr: u32,
    pub local_port: u16,
    pub peer_addr: u32,
    pub peer_port: u16,
    pub crypto: CryptoContext,
    pub rx_payload: Vec<u8>,
    pub accept_queue: VecDeque<Box<StcpContext>>,
}
impl StcpContext { pub fn new(proto_id:u8)->Self{Self{state:StcpState::New,proto_id,local_addr:0,local_port:0,peer_addr:0,peer_port:0,crypto:CryptoContext::new(),rx_payload:Vec::new(),accept_queue:VecDeque::new()}} }
