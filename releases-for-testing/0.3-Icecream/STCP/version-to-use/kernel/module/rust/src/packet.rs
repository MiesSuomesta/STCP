use alloc::vec::Vec;

use crate::error::StcpError;

pub const STCP_VERSION: u8 = 2;
/* Compact legacy packet header: descriptor + u32 payload length. */
pub const STCP_HEADER_LEN: usize = 5;
pub const STCP_PUBLIC_KEY_LEN: usize = 64;
pub const STCP_NONCE_LEN: usize = 8;
pub const STCP_AUTH_TAG_LEN: usize = 16;
pub const STCP_FRAME_PAYLOAD_LEN: usize = 60 * 1024;
pub const STCP_MAX_PAYLOAD_LEN: usize = 64 * 1024 * 1024;

const TYPE_MASK: u8 = 0x0f;
const VERSION_MASK: u8 = 0x03;
const FLAGS_MASK: u8 = 0x03;
const VERSION_SHIFT: u8 = 4;
const FLAGS_SHIFT: u8 = 6;

#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PacketType {
    PublicKey = 1,
    HandshakeDone = 2,
    DataChunk = 3,
    DataChunkEnd = 4,
    Close = 5,
}

impl PacketType {
    pub fn from_u8(value: u8) -> Result<Self, StcpError> {
        match value {
            1 => Ok(Self::PublicKey),
            2 => Ok(Self::HandshakeDone),
            3 => Ok(Self::DataChunk),
            4 => Ok(Self::DataChunkEnd),
            5 => Ok(Self::Close),
            _ => Err(StcpError::Protocol),
        }
    }
}

#[derive(Debug, Clone, Copy)]
pub struct Header {
    pub packet_type: PacketType,
    pub flags: u8,
    pub payload_len: usize,
}

impl Header {
    pub fn new(packet_type: PacketType, payload_len: usize) -> Result<Self, StcpError> {
        if payload_len > STCP_MAX_PAYLOAD_LEN || payload_len > u32::MAX as usize {
            return Err(StcpError::Protocol);
        }

        Ok(Self {
            packet_type,
            flags: 0,
            payload_len,
        })
    }

    pub fn encode(self) -> [u8; STCP_HEADER_LEN] {
        let mut output = [0u8; STCP_HEADER_LEN];
        let descriptor = (self.packet_type as u8 & TYPE_MASK)
            | ((STCP_VERSION & VERSION_MASK) << VERSION_SHIFT)
            | ((self.flags & FLAGS_MASK) << FLAGS_SHIFT);

        output[0] = descriptor;
        output[1..5].copy_from_slice(&(self.payload_len as u32).to_be_bytes());

        output
    }

    pub fn decode(input: &[u8]) -> Result<Self, StcpError> {
        if input.len() < STCP_HEADER_LEN {
            return Err(StcpError::Again);
        }

        let descriptor = input[0];
        let version = (descriptor >> VERSION_SHIFT) & VERSION_MASK;
        if version != STCP_VERSION {
            return Err(StcpError::Protocol);
        }

        let packet_type = PacketType::from_u8(descriptor & TYPE_MASK)?;
        let flags = (descriptor >> FLAGS_SHIFT) & FLAGS_MASK;
        let payload_len = u32::from_be_bytes(
            input[1..5].try_into().map_err(|_| StcpError::Protocol)?,
        ) as usize;

        if payload_len > STCP_MAX_PAYLOAD_LEN {
            return Err(StcpError::Protocol);
        }

        Ok(Self {
            packet_type,
            flags,
            payload_len,
        })
    }
}

pub fn encode_frame(
    packet_type: PacketType,
    payload: &[u8],
) -> Result<Vec<u8>, StcpError> {
    let header = Header::new(packet_type, payload.len())?.encode();
    let mut frame = Vec::with_capacity(STCP_HEADER_LEN + payload.len());

    frame.extend_from_slice(&header);
    frame.extend_from_slice(payload);

    Ok(frame)
}

pub fn encode_encrypted_frame(
    packet_type: PacketType,
    nonce: u64,
    ciphertext: &[u8],
) -> Result<Vec<u8>, StcpError> {
    let payload_len = STCP_NONCE_LEN
        .checked_add(ciphertext.len())
        .ok_or(StcpError::Protocol)?;

    let header = Header::new(packet_type, payload_len)?.encode();
    let mut frame = Vec::new();

    frame
        .try_reserve_exact(STCP_HEADER_LEN + payload_len)
        .map_err(|_| StcpError::NoMem)?;

    frame.extend_from_slice(&header);
    frame.extend_from_slice(&nonce.to_be_bytes());
    frame.extend_from_slice(ciphertext);

    Ok(frame)
}
