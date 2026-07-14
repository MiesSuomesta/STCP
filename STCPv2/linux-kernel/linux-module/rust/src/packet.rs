use alloc::vec::Vec;
use crate::error::StcpError;

pub const STCP_MAGIC: [u8; 4] = *b"STCP";
pub const STCP_VERSION: u8 = 2;
pub const STCP_HEADER_LEN: usize = 16;
pub const STCP_IV_LEN: usize = 16;
pub const STCP_PUBLIC_KEY_LEN: usize = 64;
pub const STCP_MAX_PAYLOAD_LEN: usize = 64 * 1024 * 1024;
pub const STCP_FRAME_PAYLOAD_LEN: usize = 128 * 1024;

#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum StcpPacketType { PublicKey=1, Data=2, DataChunk=3, DataChunkEnd=4, Close=5, Error=6 }

#[derive(Debug, Clone, Copy)]
pub struct StcpHeader { pub packet_type: StcpPacketType, pub flags: u16, pub payload_len: u64 }
impl StcpHeader {
 pub fn encode(self)->[u8;16]{ let mut o=[0;16]; o[..4].copy_from_slice(&STCP_MAGIC); o[4]=self.packet_type as u8; o[5]=STCP_VERSION; o[6..8].copy_from_slice(&self.flags.to_be_bytes()); o[8..].copy_from_slice(&self.payload_len.to_be_bytes()); o }
 pub fn decode(b:&[u8;16])->Result<Self,StcpError>{ if b[..4]!=STCP_MAGIC || b[5]!=STCP_VERSION{return Err(StcpError::Protocol)}; let t=match b[4]{1=>StcpPacketType::PublicKey,2=>StcpPacketType::Data,3=>StcpPacketType::DataChunk,4=>StcpPacketType::DataChunkEnd,5=>StcpPacketType::Close,6=>StcpPacketType::Error,_=>return Err(StcpError::Protocol)}; let n=u64::from_be_bytes(b[8..16].try_into().map_err(|_|StcpError::Protocol)?); if n as usize>STCP_MAX_PAYLOAD_LEN{return Err(StcpError::Protocol)}; Ok(Self{packet_type:t,flags:u16::from_be_bytes([b[6],b[7]]),payload_len:n}) }
}
pub fn encode_packet(t:StcpPacketType,p:&[u8])->Vec<u8>{let mut v=Vec::with_capacity(16+p.len());v.extend_from_slice(&StcpHeader{packet_type:t,flags:0,payload_len:p.len() as u64}.encode());v.extend_from_slice(p);v}
