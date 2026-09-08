use alloc::vec::Vec;

use crate::error::StcpError;

pub const STCP_VERSION: u8 = 2;
/*
 * Compact STCPv2 wire header.
 *
 * byte 0: bits 0..3 packet type, bits 4..5 version, bits 6..7 flags
 * bytes 1..4: payload length (u32, network byte order)
 * bytes 5..8: sequence (u32 on wire, widened to u64 internally)
 * bytes 9..12: acknowledgment (u32 on wire, widened to u64 internally)
 * bytes 13..16: connection id (u32 on wire, widened to u64 internally)
 *
 * The old 4-byte "STCP" magic and byte-aligned type/version/16-bit flags
 * were redundant once a carrier/session is already speaking STCP. Keeping
 * the counters as u64 internally avoids invasive reliability changes while
 * the wire representation remains compact. Header::with_numbers() rejects a
 * session before a value can overflow its compact u32 wire representation.
 */
pub const STCP_HEADER_LEN: usize = 17;
pub const STCP_PUBLIC_KEY_LEN: usize = 64;
pub const STCP_NONCE_LEN: usize = 8;
pub const STCP_AUTH_TAG_LEN: usize = 16;
pub const STCP_UDP_FRAME_PAYLOAD_LEN: usize = 60 * 1024;
pub const STCP_STREAM_FRAME_PAYLOAD_LEN: usize = 2 * 1024 * 1024;
/* Compatibility default for code paths without a context. */
pub const STCP_FRAME_PAYLOAD_LEN: usize = STCP_UDP_FRAME_PAYLOAD_LEN;
pub const STCP_MAX_PAYLOAD_LEN: usize = 64 * 1024 * 1024;

const TYPE_MASK: u8 = 0x0f;
const VERSION_MASK: u8 = 0x03;
const FLAGS_MASK: u8 = 0x03;
const VERSION_SHIFT: u8 = 4;
const FLAGS_SHIFT: u8 = 6;
const MAX_WIRE_COUNTER: u64 = u32::MAX as u64;

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
    /* Two wire flag bits are currently available; all existing callers use 0. */
    pub flags: u8,
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
        if payload_len > STCP_MAX_PAYLOAD_LEN ||
           payload_len > u32::MAX as usize ||
           sequence > MAX_WIRE_COUNTER ||
           acknowledgment > MAX_WIRE_COUNTER ||
           connection_id > MAX_WIRE_COUNTER
        {
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
        let descriptor = (self.packet_type as u8 & TYPE_MASK)
            | ((STCP_VERSION & VERSION_MASK) << VERSION_SHIFT)
            | ((self.flags & FLAGS_MASK) << FLAGS_SHIFT);

        output[0] = descriptor;
        output[1..5].copy_from_slice(&(self.payload_len as u32).to_be_bytes());
        output[5..9].copy_from_slice(&(self.sequence as u32).to_be_bytes());
        output[9..13].copy_from_slice(&(self.acknowledgment as u32).to_be_bytes());
        output[13..17].copy_from_slice(&(self.connection_id as u32).to_be_bytes());

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
        let sequence = u32::from_be_bytes(
            input[5..9].try_into().map_err(|_| StcpError::Protocol)?,
        ) as u64;
        let acknowledgment = u32::from_be_bytes(
            input[9..13].try_into().map_err(|_| StcpError::Protocol)?,
        ) as u64;
        let connection_id = u32::from_be_bytes(
            input[13..17].try_into().map_err(|_| StcpError::Protocol)?,
        ) as u64;

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
