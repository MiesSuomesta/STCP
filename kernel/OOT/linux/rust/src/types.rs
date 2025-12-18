use alloc::vec::Vec;

use crate::{stcp_dbg /* , stcp_dump */ };

use crate::stcp_message::{stcp_message_get_header_size_in_bytes};

#[repr(u8)]
#[derive(PartialEq, Eq, Copy, Clone, Debug)]
pub enum HandshakeStatus {
    Init =  0,
    Public = 1,
    Complete = 2,
    Aes = 3,
    Error = 4,
}

impl HandshakeStatus {

    pub fn from_raw(raw: u32) -> Option<Self> {
        match raw {
            0 => Some(Self::Init),
            1 => Some(Self::Public),
            2 => Some(Self::Complete),
            3 => Some(Self::Aes),
            _ => Some(Self::Error),
        }
    }

    pub fn next_step(self) -> Option<Self> {
        match self {
            Self::Init      => Some(Self::Public),
            Self::Public    => Some(Self::Complete),
            Self::Complete  => Some(Self::Aes),
            Self::Aes       => Some(Self::Aes),   // ei seuraavaa
            _               => Some(Self::Error), // ei seuraavaa
        }
    }
   
    pub fn to_raw(self) -> u32 {
        self as u32
    }
}

#[repr(u8)]
#[derive(PartialEq, Eq, Copy, Clone, Debug)]
pub enum StcpMsgType {
    Unknown = 0,
    Error = 1,
    Public = 2,        // Public key
    Aes = 3,

}

impl StcpMsgType {

    pub fn from_raw(raw: u32) -> Option<Self> {
        match raw {
            0 => Some(Self::Unknown),
            1 => Some(Self::Error),
            2 => Some(Self::Public),
            3 => Some(Self::Aes),
            _ => Some(Self::Unknown),
        }
    }

    pub fn next_step(self) -> Option<Self> {
        match self {
            Self::Unknown => Some(Self::Error),
            Self::Error   => Some(Self::Public),
            Self::Public  => Some(Self::Aes),
            Self::Aes     => None, // ei seuraavaa
        }
    }
    
    pub fn to_raw(self) -> u32 {
        self as u32
    }
}

// Teoriassa TCP-paketin maksimi koko, ~65KB
pub const STCP_MAX_TCP_PAYLOAD_SIZE: u16 = 65495;

pub const STCP_TCP_RECV_BLOCK: i32 = 0;
pub const STCP_TCP_RECV_NO_BLOCK: i32 = 1;

pub const MSG_PEEK : u32 = 0x02;

pub const STCP_TAG_BYTES: u64 = 0x53544350; // STCP heksana

#[repr(C)]
#[derive(PartialEq, Eq, Copy, Clone, Debug)]
pub struct StcpMessageHeader {
    pub version  : u32,               // Version
    pub tag      : u64,               // STCP
    pub msg_type : StcpMsgType,       // type (Init at start)
    pub msg_len  : u32,               // (AES) payload size
}

pub const STCP_VERSION: u32 = 1;

impl StcpMessageHeader {

    pub fn new() -> Self {
        Self {
            version: STCP_VERSION,
            tag: STCP_TAG_BYTES,
            msg_type: StcpMsgType::Unknown,
            msg_len:0,
        }
    }

    pub fn to_bytes_be(&self) -> Vec<u8> {
        let header_size = stcp_message_get_header_size_in_bytes();
        let mut buffer: Vec<u8> = Vec::with_capacity(header_size);
        buffer.resize(header_size, 0);
        let mut i: usize = 0;

        // Versio
        buffer[i..i+4].copy_from_slice(&self.version.to_be_bytes());
        i += 4;

        // tag
        buffer[i..i+8].copy_from_slice(&self.tag.to_be_bytes());
        i += 8;

        // Msg type
        buffer[i] = self.msg_type as u8;
        i += 1;

        // msg len
        buffer[i..i+4].copy_from_slice(&self.msg_len.to_be_bytes());

        buffer
    }

    pub fn from_bytes_be(buffer: &[u8]) -> StcpMessageHeader {
        let header_size = stcp_message_get_header_size_in_bytes();
        if buffer.len() < header_size {
          stcp_dbg!("No enough data");   
            return StcpMessageHeader {
                                    version: 0,
                                    tag: 0,
                                    msg_type: StcpMsgType::Error,
                                    msg_len: 0,
                                };
        }

        let mut i: usize = 0;

        // Versio
        let got_version = u32::from_be_bytes( buffer[i..i+4].try_into().unwrap() );
        i += 4;

        // tag
        let got_tag = u64::from_be_bytes( buffer[i..i+8].try_into().unwrap() );
        i += 8;

        // Msg type 
        /*
            Error = 0,
            Unknown = 1,
            Public = 2,        // Public key
            Aes = 3,
        */
        let got_type = match buffer[i] {
            // Mäppäys
            0 => StcpMsgType::Error,
            1 => StcpMsgType::Unknown,
            2 => StcpMsgType::Public,
            3 => StcpMsgType::Aes,
            // Defaultti
            _ => StcpMsgType::Unknown,
        };

        i += 1;

        // msg len
        // tag
        let got_lenght = u32::from_be_bytes( buffer[i..i+4].try_into().unwrap() );
        
        StcpMessageHeader {
            version: got_version,
            tag: got_tag,
            msg_type: got_type,
            msg_len: got_lenght,
        }
    }

}

#[repr(C)]
pub struct kernel_socket {
    _private: [u8; 0],
}

#[repr(C)]
#[derive(PartialEq, Eq, Clone, Debug)]
pub struct ProtoSession {
    pub private_key         : StcpEcdhSecret,
    pub public_key          : StcpEcdhPubKey,
    pub shared_key          : StcpEcdhSecret,
    pub status_raw          : u32,
    pub is_server           : bool,
    pub transport           : *mut kernel_socket,
}

impl ProtoSession {

    pub fn new(p_server:bool, p_transport: *mut kernel_socket) -> Self {
        stcp_dbg!("ProtoSession/new: Start ...");
        let out = Self {
            private_key: StcpEcdhSecret::new(),
            shared_key:  StcpEcdhSecret::new(),
            public_key:  StcpEcdhPubKey::new(),
            status_raw: HandshakeStatus::Init.to_raw(),
            is_server: p_server,
            transport: p_transport,
        };
        stcp_dbg!("ProtoSession/new: end ...");
        out
    }

    pub fn get_status(&self) -> HandshakeStatus {
        match HandshakeStatus::from_raw(self.status_raw) {
            Some(s) => s,
            None => HandshakeStatus::Error,
        }
    }

    pub fn set_status(&mut self, state: HandshakeStatus) {
        self.status_raw = state.to_raw();
    }

    pub fn is_handshake(&self, tgt: HandshakeStatus) -> bool {
        let now = self.status_raw;
        let other = tgt.to_raw();
        let ret: bool = now == other;
        stcp_dbg!("Is handshake status {} vs {} => {}", now, other, ret);   
        return ret;
    }

    pub fn in_aes_mode(&self) -> bool {
        self.is_handshake(HandshakeStatus::Aes)
    }

    pub fn get_is_server(&self) -> bool {
        self.is_server
    }

    pub fn set_is_server(&mut self, t: bool) {
        self.is_server = t;
    }

}

pub const STCP_RECV_BLOCK: i32     = 0;
pub const STCP_RECV_NON_BLOCK: i32 = 1;

pub const STCP_ECDH_PRIV_LEN: usize   = 32;
pub const STCP_ECDH_PUB_LEN: usize    = 64;
pub const STCP_ECDH_PUB_XY_LEN: usize = 32;
pub const STCP_ECDH_SHARED_LEN: usize = 32;

#[repr(C)]
#[derive(PartialEq, Eq, Clone, Debug)]
pub struct StcpEcdhPubKey {
    pub x: [u8; STCP_ECDH_PUB_XY_LEN],
    pub y: [u8; STCP_ECDH_PUB_XY_LEN],
}

impl StcpEcdhPubKey {

    pub fn new() -> Self {
        stcp_dbg!("StcpEcdhPubKey/new_boxed: Start ...");
        let mut xvec = Vec::with_capacity(STCP_ECDH_PUB_XY_LEN);
            xvec.resize(STCP_ECDH_PUB_XY_LEN, 0);

        let mut yvec = Vec::with_capacity(STCP_ECDH_PUB_XY_LEN);
            yvec.resize(STCP_ECDH_PUB_XY_LEN, 0);

        let out = Self {
            x: xvec[..STCP_ECDH_PUB_XY_LEN].try_into().unwrap(),
            y: yvec[..STCP_ECDH_PUB_XY_LEN].try_into().unwrap(),
        };
        stcp_dbg!("StcpEcdhPubKey/new_boxed: End ...");

        out
    }

    pub fn to_bytes_be(&self) -> Vec<u8> {
        let mut buffer: Vec<u8> = Vec::with_capacity(STCP_ECDH_PUB_LEN);
        buffer.resize(STCP_ECDH_PUB_LEN, 0);

        // X 
        buffer[0..STCP_ECDH_PUB_XY_LEN].copy_from_slice(&self.x[0..STCP_ECDH_PUB_XY_LEN]);

        // Y 
        buffer[STCP_ECDH_PUB_XY_LEN..].copy_from_slice(&self.y[0..STCP_ECDH_PUB_XY_LEN]);
        
        buffer
    }

    pub fn from_bytes_be(buffer: &[u8]) -> StcpEcdhPubKey {
        stcp_dbg!("StcpEcdhPubKey/from_bytes_be: Start ...");
        if buffer.len() < STCP_ECDH_PUB_LEN {
            stcp_dbg!("No enough data");   
            return StcpEcdhPubKey::new();
        }

        let mut sepk = StcpEcdhPubKey::new();

        // x & y
        for i in 0..STCP_ECDH_PUB_XY_LEN {
            sepk.x[i] = buffer[i];
            sepk.y[i] = buffer[STCP_ECDH_PUB_XY_LEN + i];
        }
        stcp_dbg!("StcpEcdhPubKey/from_bytes_be: End ...");
        
        sepk
    }

}



#[repr(C)]
#[derive(PartialEq, Eq, Clone, Debug)]
pub struct StcpEcdhSecret {
    pub data: [u8; STCP_ECDH_SHARED_LEN],
    pub len: usize,
}

impl StcpEcdhSecret {
    pub fn new() -> Self {
        stcp_dbg!("StcpEcdhSecret/new: Start ...");
        let mut dvec = Vec::with_capacity(STCP_ECDH_PUB_XY_LEN);
                dvec.resize(STCP_ECDH_PUB_XY_LEN, 0);

        let out = Self {
            data: dvec[..STCP_ECDH_PUB_XY_LEN].try_into().unwrap(),
            len: 0,
        };
        stcp_dbg!("StcpEcdhSecret/new: End ...");
        out
    }

    pub fn set_from_slice(&mut self, src: &[u8]) {
        let n = core::cmp::min(src.len(), STCP_ECDH_SHARED_LEN);
        self.data[..n].copy_from_slice(&src[..n]);
        self.len = n;
    }


    pub fn from_bytes_be(buffer: &[u8]) -> StcpEcdhSecret {
        stcp_dbg!("StcpEcdhSecret/from_bytes_be: Start ...");

        if buffer.len() < STCP_ECDH_SHARED_LEN {
            stcp_dbg!("No enough data");   
            return StcpEcdhSecret::new();
        }

        let mut shkey = StcpEcdhSecret::new();

        // x & y
        for i in 0..STCP_ECDH_SHARED_LEN {
            shkey.data[i] = buffer[i];
        }
        stcp_dbg!("StcpEcdhSecret/from_bytes_be: End ...");
        shkey
    }

}
