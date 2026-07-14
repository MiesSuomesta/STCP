use crate::error::StcpError;

pub const STCP_MAGIC: [u8; 4] = *b"STCP";
pub const STCP_VERSION: u8 = 2;
pub const STCP_HEADER_LEN: usize = 16;
pub const STCP_IV_LEN: usize = 16;

pub const STCP_AES_GCM_TAG_LEN: usize = 16;
pub const STCP_AES_GCM_NONCE_LEN: usize = 12;

pub const STCP_PUBLIC_KEY_LEN: usize = 64;

pub const STCP_INITIAL_RX_BUFFER_LEN: usize = 128;

pub const STCP_MAX_PAYLOAD_LEN: usize = 64 * 1024 * 1024;

// On wire paljonko maksimi
pub const STCP_FRAME_PAYLOAD_LEN: usize = 128 * 1024;

// Muutaman framen kokonen bufferi testityökaluille 
pub const STCP_RECEIVE_BUFFER_LEN: usize = 1 * 1024 * 1024;



// Moduuliin tämä:
//pub const STCP_SOCKET_TYPE: i32 = 253;
pub const STCP_SOCKET_TYPE: i32 = 6;

#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum StcpPacketType {
    PublicKey = 1,
    Data = 2,
    DataChunk = 3,
    DataChunkEnd = 4,
    Close = 5,
    Error = 6,
}

impl StcpPacketType {
    pub fn from_u8(v: u8) -> Result<Self, StcpError> {
        match v {
            1 => Ok(Self::PublicKey),
            2 => Ok(Self::Data),
            3 => Ok(Self::DataChunk),
            4 => Ok(Self::DataChunkEnd),
            5 => Ok(Self::Close),
            6 => Ok(Self::Error),
            _ => Err(StcpError::Protocol(format!("unknown packet type: {v}"))),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct StcpHeader {
    pub packet_type: StcpPacketType,
    pub version: u8,
    pub flags: u16,
    pub payload_len: u64,
}

impl StcpHeader {
    pub fn new(packet_type: StcpPacketType, flags: u16, payload_len: u64) -> Self {
        Self { packet_type, version: STCP_VERSION, flags, payload_len }
    }

    pub fn encode(&self) -> [u8; STCP_HEADER_LEN] {
        let mut out = [0u8; STCP_HEADER_LEN];
        out[0..4].copy_from_slice(&STCP_MAGIC);
        out[4] = self.packet_type as u8;
        out[5] = self.version;
        out[6..8].copy_from_slice(&self.flags.to_be_bytes());
        out[8..16].copy_from_slice(&self.payload_len.to_be_bytes());
        out
    }

    pub fn decode(buf: &[u8; STCP_HEADER_LEN]) -> Result<Self, StcpError> {
        if buf[0..4] != STCP_MAGIC {
            return Err(StcpError::Protocol("bad STCP magic".to_string()));
        }

        let packet_type = StcpPacketType::from_u8(buf[4])?;
        let version = buf[5];
        if version != STCP_VERSION {
            return Err(StcpError::Protocol(format!("unsupported STCP version: {version}")));
        }

        let flags = u16::from_be_bytes([buf[6], buf[7]]);
        let payload_len = u64::from_be_bytes(buf[8..16].try_into().unwrap());

        if payload_len as usize > STCP_MAX_PAYLOAD_LEN {
            return Err(StcpError::Protocol(format!("payload too large: {payload_len}")));
        }

        Ok(Self { packet_type, version, flags, payload_len })
    }
}

pub fn encode_packet(packet_type: StcpPacketType, payload: &[u8]) -> Vec<u8> {
    let header = StcpHeader::new(packet_type, 0, payload.len() as u64).encode();
    let mut out = Vec::with_capacity(STCP_HEADER_LEN + payload.len());
    out.extend_from_slice(&header);
    out.extend_from_slice(payload);
    out
}

pub fn decode_packet(
    packet: &[u8],
) -> Result<(StcpHeader, &[u8]), StcpError> {

    if packet.len() < STCP_HEADER_LEN {
        return Err(
            StcpError::Protocol(
                "packet too short".to_string()
            )
        );
    }

    let header_buf: &[u8; STCP_HEADER_LEN] =
        packet[0..STCP_HEADER_LEN]
            .try_into()
            .map_err(|_| {
                StcpError::Protocol(
                    "invalid header size".to_string()
                )
            })?;

    let header = StcpHeader::decode(header_buf)?;

    let payload_len = header.payload_len as usize;

    if packet.len() < STCP_HEADER_LEN + payload_len {
        return Err(
            StcpError::Protocol(
                "payload truncated".to_string()
            )
        );
    }

    let payload =
        &packet[STCP_HEADER_LEN..STCP_HEADER_LEN + payload_len];

    Ok((header, payload))
}

pub fn encode_public_key_packet(public_key: &[u8; STCP_PUBLIC_KEY_LEN]) -> Vec<u8> {
    encode_packet(StcpPacketType::PublicKey, public_key)
}

pub fn encode_data_packet(data_type: StcpPacketType, iv: [u8; STCP_IV_LEN], ciphertext: &[u8]) -> Vec<u8> {
    let mut payload = Vec::with_capacity(STCP_IV_LEN + ciphertext.len());
    payload.extend_from_slice(&iv);
    payload.extend_from_slice(ciphertext);
    encode_packet(data_type, &payload)
}

pub fn split_data_payload(payload: &[u8]) -> Result<([u8; STCP_IV_LEN], &[u8]), StcpError> {
    if payload.len() < STCP_IV_LEN {
        return Err(StcpError::Protocol(format!("data payload too small: {}", payload.len())));
    }

    let mut iv = [0u8; STCP_IV_LEN];
    iv.copy_from_slice(&payload[..STCP_IV_LEN]);
    Ok((iv, &payload[STCP_IV_LEN..]))
}
