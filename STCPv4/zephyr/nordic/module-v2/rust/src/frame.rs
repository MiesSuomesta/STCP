use alloc::vec::Vec;

use crate::error::StcpError;

pub const STCP_MAGIC: [u8; 4] = *b"STCP";
pub const STCP_VERSION: u8 = 2;
pub const STCP_HEADER_LEN: usize = 40;
pub const STCP_PUBLIC_KEY_LEN: usize = 64;
pub const STCP_NONCE_LEN: usize = 8;
pub const STCP_AUTH_TAG_LEN: usize = 16;
pub const STCP_UDP_FRAME_PAYLOAD_LEN: usize = 60 * 1024;
pub const STCP_STREAM_FRAME_PAYLOAD_LEN: usize = 2 * 1024 * 1024;
/* Compatibility default for code paths without a context. */
pub const STCP_FRAME_PAYLOAD_LEN: usize = STCP_UDP_FRAME_PAYLOAD_LEN;
pub const STCP_MAX_PAYLOAD_LEN: usize = 64 * 1024 * 1024;

#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PacketType {
    PublicKey = 1,
    HandshakeDone = 2,
    DataChunk = 3,
    DataChunkEnd = 4,
    Ack = 5,
    Ping = 6,
    Pong = 7,
    Close = 8,
    Reset = 9,
}

impl PacketType {
    pub fn from_u8(value: u8) -> Result<Self, StcpError> {
        match value {
            1 => Ok(Self::PublicKey),
            2 => Ok(Self::HandshakeDone),
            3 => Ok(Self::DataChunk),
            4 => Ok(Self::DataChunkEnd),
            5 => Ok(Self::Ack),
            6 => Ok(Self::Ping),
            7 => Ok(Self::Pong),
            8 => Ok(Self::Close),
            9 => Ok(Self::Reset),
            _ => Err(StcpError::Protocol),
        }
    }
}

#[derive(Debug, Clone, Copy)]
pub struct Header {
    pub packet_type: PacketType,
    pub flags: u16,
    pub payload_len: usize,
    pub sequence: u64,
    pub acknowledgment: u64,
    pub connection_id: u64,
}

impl Header {
    pub fn new(
        packet_type: PacketType,
        payload_len: usize,
    ) -> Result<Self, StcpError> {
        Self::with_numbers(packet_type, payload_len, 0, 0, 0)
    }

    pub fn with_numbers(
        packet_type: PacketType,
        payload_len: usize,
        sequence: u64,
        acknowledgment: u64,
        connection_id: u64,
    ) -> Result<Self, StcpError> {
        if payload_len > STCP_MAX_PAYLOAD_LEN {
            return Err(StcpError::Protocol);
        }

        Ok(Self {
            packet_type,
            flags: 0,
            payload_len,
            sequence,
            acknowledgment,
            connection_id,
        })
    }

    pub fn encode(self) -> [u8; STCP_HEADER_LEN] {
        let mut output = [0u8; STCP_HEADER_LEN];

        output[0..4].copy_from_slice(&STCP_MAGIC);
        output[4] = self.packet_type as u8;
        output[5] = STCP_VERSION;
        output[6..8].copy_from_slice(&self.flags.to_be_bytes());
        output[8..16].copy_from_slice(&(self.payload_len as u64).to_be_bytes());
        output[16..24].copy_from_slice(&self.sequence.to_be_bytes());
        output[24..32].copy_from_slice(&self.acknowledgment.to_be_bytes());
        output[32..40].copy_from_slice(&self.connection_id.to_be_bytes());

        output
    }

    pub fn decode(input: &[u8]) -> Result<Self, StcpError> {
        if input.len() < STCP_HEADER_LEN {
            return Err(StcpError::Again);
        }
        if input[0..4] != STCP_MAGIC || input[5] != STCP_VERSION {
            return Err(StcpError::Protocol);
        }

        let packet_type = PacketType::from_u8(input[4])?;
        let flags = u16::from_be_bytes([input[6], input[7]]);
        let payload_len = u64::from_be_bytes(
            input[8..16].try_into().map_err(|_| StcpError::Protocol)?,
        ) as usize;
        let sequence = u64::from_be_bytes(
            input[16..24].try_into().map_err(|_| StcpError::Protocol)?,
        );
        let acknowledgment = u64::from_be_bytes(
            input[24..32].try_into().map_err(|_| StcpError::Protocol)?,
        );
        let connection_id = u64::from_be_bytes(
            input[32..40].try_into().map_err(|_| StcpError::Protocol)?,
        );

        if payload_len > STCP_MAX_PAYLOAD_LEN {
            return Err(StcpError::Protocol);
        }

        Ok(Self {
            packet_type,
            flags,
            payload_len,
            sequence,
            acknowledgment,
            connection_id,
        })
    }
}

pub fn encode_frame(
    packet_type: PacketType,
    connection_id: u64,
    payload: &[u8],
) -> Result<Vec<u8>, StcpError> {
    encode_control_frame(packet_type, 0, 0, connection_id, payload)
}

pub fn encode_control_frame(
    packet_type: PacketType,
    sequence: u64,
    acknowledgment: u64,
    connection_id: u64,
    payload: &[u8],
) -> Result<Vec<u8>, StcpError> {
    let header = Header::with_numbers(
        packet_type,
        payload.len(),
        sequence,
        acknowledgment,
        connection_id,
    )?.encode();
    let mut frame = Vec::new();
    frame.try_reserve_exact(STCP_HEADER_LEN + payload.len())
        .map_err(|_| StcpError::NoMem)?;
    frame.extend_from_slice(&header);
    frame.extend_from_slice(payload);
    Ok(frame)
}

pub fn encode_encrypted_frame(
    packet_type: PacketType,
    sequence: u64,
    acknowledgment: u64,
    connection_id: u64,
    nonce: u64,
    ciphertext: &[u8],
) -> Result<Vec<u8>, StcpError> {
    let payload_len = STCP_NONCE_LEN
        .checked_add(ciphertext.len())
        .ok_or(StcpError::Protocol)?;
    let header = Header::with_numbers(
        packet_type,
        payload_len,
        sequence,
        acknowledgment,
        connection_id,
    )?.encode();
    let mut frame = Vec::new();
    frame.try_reserve_exact(STCP_HEADER_LEN + payload_len)
        .map_err(|_| StcpError::NoMem)?;
    frame.extend_from_slice(&header);
    frame.extend_from_slice(&nonce.to_be_bytes());
    frame.extend_from_slice(ciphertext);
    Ok(frame)
}
