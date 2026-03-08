extern crate alloc;

use core::ffi::c_int;
use core::ffi::c_void;
use alloc::vec;
use alloc::vec::Vec;
use crate::stcp_dbg;

/* ========================= RX BUFFER ========================= */

#[repr(C)]
#[derive(Clone, Debug)]
pub struct PeekRxBuff {
    pub buf: [u8; 4096],
    pub start: usize,
    pub end: usize,
}

impl PeekRxBuff {

    pub fn new() -> Self {
        Self {
            buf: [0;4096],
            start: 0,
            end: 0,
        }
    }

    pub fn available(&self) -> usize {
        self.end - self.start
    }

    pub fn push(&mut self, data: &[u8]) {

        let len = data.len();

        self.buf[self.end..self.end + len].copy_from_slice(data);

        self.end += len;
    }

    pub fn pop(&mut self, out: &mut [u8]) -> usize {

        let available = self.available();

        if available == 0 {
            return 0;
        }

        let n = core::cmp::min(out.len(), available);

        out[..n].copy_from_slice(&self.buf[self.start..self.start + n]);

        self.start += n;

        if self.start == self.end {
            self.start = 0;
            self.end = 0;
        }

        n
    }
}

/* ========================= HANDSHAKE ========================= */

#[repr(u8)]
#[derive(PartialEq, Eq, Copy, Clone, Debug)]
pub enum HandshakeStatus {
    Init = 0,
    Public = 1,
    Complete = 2,
    Aes = 3,
    Error = 4,
}

impl HandshakeStatus {
    pub fn from_raw(raw: u32) -> Self {
        match raw {
            0 => Self::Init,
            1 => Self::Public,
            2 => Self::Complete,
            3 => Self::Aes,
            _ => Self::Error,
        }
    }

    pub fn to_raw(self) -> u32 {
        self as u32
    }

    pub fn next_step(self) -> Option<Self> {
        match self {
            Self::Init => Some(Self::Public),
            Self::Public => Some(Self::Complete),
            Self::Complete => Some(Self::Aes),
            Self::Aes => None,
            _ => Some(Self::Error),
        }
    }
}

/* ========================= MSG TYPE ========================= */

#[repr(u32)]
#[derive(PartialEq, Eq, Copy, Clone, Debug)]
pub enum StcpMsgType {
    Unknown = 0,
    Error = 1,
    Public = 2,
    Aes = 3,
}

impl StcpMsgType {
    pub fn from_raw(raw: u32) -> Self {
        match raw {
            1 => Self::Error,
            2 => Self::Public,
            3 => Self::Aes,
            _ => Self::Unknown,
        }
    }

    pub fn to_raw(self) -> u32 {
        self as u32
    }
}

/* ========================= HEADER ========================= */

pub const STCP_TAG_BYTES: u32 = 0x53544350;
pub const STCP_VERSION: u32 = 1;

#[repr(C)]
#[derive(PartialEq, Eq, Copy, Clone, Debug)]
pub struct StcpMessageHeader {
    pub version: u32,
    pub tag: u32,
    pub msg_type: StcpMsgType,      // u32!
    pub msg_len: u32,
}

impl StcpMessageHeader {
    pub const HEADER_LEN: usize = 4 + 4 + 4 + 4; // 16 bytes 

    pub fn new() -> Self {
        Self {
            version:  STCP_VERSION,
            tag:      STCP_TAG_BYTES,
            msg_type: StcpMsgType::Unknown,
            msg_len:  0,
        }
    }

    pub fn new_from(p_ver: u32, p_tag: u32, p_type: StcpMsgType, p_len: u32) -> Self {
        Self {
            version: p_ver,
            tag: p_tag,
            msg_type: p_type,
            msg_len: p_len,
        }
    }

    pub fn to_be_bytes(&self) -> Vec<u8> {
        let mut out = vec![0u8; Self::HEADER_LEN];

        let mut i = 0;
        out[i..i+4].copy_from_slice(&self.version.to_be_bytes()); i += 4;
        out[i..i+4].copy_from_slice(&self.tag.to_be_bytes()); i += 4;
        out[i..i+4].copy_from_slice(&(self.msg_type as u32).to_be_bytes()); i += 4;
        out[i..i+4].copy_from_slice(&self.msg_len.to_be_bytes());

        out
    }

    pub fn validate(&self) -> bool {
        stcp_dbg!("Checkpoint");
        if self.version != STCP_VERSION {
            stcp_dbg!("Checkpoint");
            return false;
        }
        stcp_dbg!("Checkpoint");

        if self.tag != STCP_TAG_BYTES {
            stcp_dbg!("Checkpoint");
            return false;
        }

        stcp_dbg!("Checkpoint");
        true
    }

    pub fn try_from_bytes_be(buf: &[u8]) -> Option<Self> {

        if buf.len() < Self::HEADER_LEN {
            return None;
        }

        let mut i = 0;

        let version = u32::from_be_bytes(buf[i..i+4].try_into().ok()?);
        i += 4;

        let tag = u32::from_be_bytes(buf[i..i+4].try_into().ok()?);
        i += 4;

        let msg_type_raw = u32::from_be_bytes(buf[i..i+4].try_into().ok()?);
        let msg_type = StcpMsgType::from_raw(msg_type_raw);
        i += 4;

        let msg_len = u32::from_be_bytes(buf[i..i+4].try_into().ok()?);
        stcp_dbg!("Header from 0x{:08X}", msg_len);
        stcp_dbg!("Header for {} bytes payload", msg_len);

        Some(Self {
            version,
            tag,
            msg_type,
            msg_len,
        })
    }

    pub fn extract_payload<'a>(&self, frame: &'a [u8]) -> Option<&'a [u8]> {

        let start = Self::HEADER_LEN;
        let end = start + self.msg_len as usize;

        if frame.len() < end {
            return None;
        }

        Some(&frame[start..end])
    }

    pub fn parse_and_validate(buf: &[u8]) -> Option<Self> {
        stcp_dbg!("Checkpoint");
        let h = Self::try_from_bytes_be(buf)?;
        
        stcp_dbg!("Checkpoint");
        if !h.validate() {
            stcp_dbg!("Checkpoint");
            return None;
        }
        
        stcp_dbg!("Checkpoint");
        Some(h)
    }


    pub fn from_bytes_be(buf: &[u8]) -> Option<Self> {
        let mut i = 0;

        if buf.len() < 16 {
            stcp_dbg!("Header too short: {} / {} bytes", buf.len(), 16);
            return None;
        }

        let version = u32::from_be_bytes(buf[i..i+4].try_into().ok()?); 
        i += 4;

        let stcp_magic = u32::from_be_bytes(*b"STCP");
        let magic = u32::from_be_bytes(buf[i..i+4].try_into().ok()?);
        i += 4;

        if magic != stcp_magic {
            stcp_dbg!("Bad magic: 0x{:08X?} vs 0x{:08X?}", magic,  stcp_magic);
            return None;
        }

        let msg_type_raw = u32::from_be_bytes(buf[i..i+4].try_into().ok()?);
        let msg_type = StcpMsgType::from_raw(msg_type_raw);
        i += 4;

        let msg_len = u32::from_be_bytes(buf[i..i+4].try_into().ok()?);
        //i += 4;

        if msg_len > 64 * 1024 {
            stcp_dbg!("Insane msg_len: {}", msg_len);
            return None;
        }

        Some(Self {
            version,
            tag: magic,
            msg_type: msg_type,
            msg_len,
        })
    }
}

/* ========================= SOCKET ========================= */

/* iovec ja msgheader */

#[repr(C)]
pub struct zsock_iovec {
    pub iov_base: *mut c_void,
    pub iov_len: usize,
}

#[repr(C)]
pub struct zsock_msghdr {
    pub msg_name: *mut c_void,
    pub msg_namelen: u32,
    pub msg_iov: *mut zsock_iovec,
    pub msg_iovlen: usize,
    pub msg_control: *mut c_void,
    pub msg_controllen: usize,
    pub msg_flags: c_int,
}

/* ========================= ECDH ========================= */
pub const STCP_ECDH_SHARED_LEN: usize = 32;
pub const STCP_ECDH_PUB_XY_LEN: usize = 32;
pub const STCP_ECDH_PUB_LEN: usize = 64;
pub const STCP_MAX_TCP_PAYLOAD_SIZE: usize = 65495;

#[repr(C)]
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct StcpEcdhPubKey {
    pub x: [u8; STCP_ECDH_PUB_XY_LEN],
    pub y: [u8; STCP_ECDH_PUB_XY_LEN],
}


impl StcpEcdhPubKey {
    pub fn new() -> Self {
        Self {
            x: [0u8; STCP_ECDH_PUB_XY_LEN],
            y: [0u8; STCP_ECDH_PUB_XY_LEN],
        }
    }

    pub fn from_bytes_be(buf: &[u8]) -> Self {
        let mut out = Self::new();
        out.x.copy_from_slice(&buf[0..32]);
        out.y.copy_from_slice(&buf[32..64]);
        out
    }
    
    pub fn to_bytes_be(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(STCP_ECDH_PUB_LEN);
        out.extend_from_slice(&self.x);
        out.extend_from_slice(&self.y);
        out
    }
}

#[repr(C)]
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct StcpEcdhSecret {
    pub data: [u8; 32],
}

impl StcpEcdhSecret {
    pub fn new() -> Self {
        Self {
            data: [0u8; STCP_ECDH_SHARED_LEN],
        }
    }

    pub fn to_bytes_be(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(STCP_ECDH_SHARED_LEN);
        out.extend_from_slice(&self.data);
        out
    }

    pub fn set_from_slice(&mut self, src: &[u8]) {
        let n = core::cmp::min(src.len(), STCP_ECDH_SHARED_LEN);
        self.data[..n].copy_from_slice(&src[..n]);
    }
}
