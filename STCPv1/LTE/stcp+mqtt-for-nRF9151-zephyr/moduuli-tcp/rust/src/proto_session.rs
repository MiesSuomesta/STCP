extern crate alloc;

use core::ffi::c_void;
use crate::stcp_tcp_recv_once;
use alloc::vec;
use alloc::vec::Vec;
use crate::stcp_dbg;
use crate::stcp_dump;
use crate::aes::StcpAesCodec;
use crate::slice_helpers::StcpError;
use crate::types::*;
use crate::errorit::*;
use alloc::boxed::Box;

#[cfg(feature = "std")]
use std::net::{TcpListener, TcpStream};

use crate::stcp_message::build_frame_and_send;
use crate::stcp_message::stcp_message_get_header_size_in_bytes;
use crate::abi::stcp_rust_api_transport_get_fd;

/* ========================= SESSION ========================= */
pub const SESSION_MAGIC: u32 = 0x53544350;
pub const STCP_HEADER_SIZE: usize = 16;
pub const STCP_MAX_FRAME: usize = 1024 * 1024; // 1MB

pub enum RxState {
    Header,
    Payload,
}


pub struct ProtoSession {
    pub private_key: StcpEcdhSecret,
    pub public_key: StcpEcdhPubKey,
    pub shared_key: StcpEcdhSecret,
    pub status_raw: u32,
    pub is_in_aes_mode: bool,
    pub is_aes_bypass_requested: bool,
    pub is_freed: bool,
    pub transport: *mut core::ffi::c_void,
    pub rx_buff: PeekRxBuff,
    pub aes: Option<StcpAesCodec>,
    pub magic: u32,
    pub rx_state: RxState,

    pub header_buf: [u8; STCP_HEADER_SIZE],
    pub header_pos: usize,

    pub payload_buf: Vec<u8>,
    pub payload_pos: usize,
    pub payload_len: usize,

    pub hs_rx_buf: [u8; 64],
    pub hs_rx_off: isize,

    pub is_my_public_key_sent: bool,
    pub are_keys_generated: bool,
}

impl Drop for ProtoSession {
    fn drop(&mut self) {
        if !self.transport.is_null() {
            unsafe {
                let ptr = self.transport as *mut kernel_socket;
                drop(Box::from_raw(ptr));
            }
        }
    }
}

impl ProtoSession {

    pub fn new(transport_vp: *mut c_void) -> Self {
        let transp = transport_vp as *mut core::ffi::c_void;
        Self {
            magic: SESSION_MAGIC,
            private_key: StcpEcdhSecret::new(),
            public_key: StcpEcdhPubKey::new(),
            shared_key: StcpEcdhSecret::new(),
            status_raw: HandshakeStatus::Init.to_raw(),

            // Ei nykysin taideta käyttää mihinkään.. TODO: poista jos näin on
            // is_server: is_server,

            // Defaulttina ei päällä, EI SAA OLLA.
            is_in_aes_mode: false,
            
            // Defaulttina bypass ei päällä. Laitetaan erikseen session teossa RUST puolella.
            is_aes_bypass_requested: false, 
            is_freed: false,
            transport: transp,
            rx_buff: PeekRxBuff::new(),
            aes: None,

            rx_state: RxState::Header,
            header_buf: [0; STCP_HEADER_SIZE],
            header_pos: 0,

            payload_buf: Vec::new(),
            payload_pos: 0,
            payload_len: 0,

            hs_rx_buf:  [0; 64],
            hs_rx_off: 0,
            is_my_public_key_sent: false,
            are_keys_generated: false,
        }
    }

    pub fn get_transport_fd(&mut self) -> i32 {
        let fd: i32 = unsafe { stcp_rust_api_transport_get_fd(self.transport) };
        stcp_dbg!("Got transport FD: {:?}", fd);
        fd
    }

    pub fn init_aes_with(&mut self, the_sk: StcpEcdhSecret) {
        stcp_dbg!("Initialising with AES....");
        let sk = the_sk.to_bytes_be();
        self.aes = Some(StcpAesCodec::new(&sk));
        self.set_status(HandshakeStatus::Aes);
        self.set_aes_mode(true);
        stcp_dbg!("====== WITH AES ======");
    }

    pub fn get_aes(&self) -> Result<&StcpAesCodec, StcpError> {
        self.aes.as_ref().ok_or(StcpError::Invalid)
    }

    pub fn reset_everything_now(&mut self) -> i32 {
        self.private_key = StcpEcdhSecret::new();
        self.public_key = StcpEcdhPubKey::new();
        self.shared_key = StcpEcdhSecret::new();
        self.status_raw = HandshakeStatus::Init.to_raw();
        self.rx_buff = PeekRxBuff::new();
        0
    }

    pub fn set_aes_mode(&mut self, v: bool) {
        self.is_in_aes_mode = v;
    }

    pub fn in_aes_mode(&self) -> bool {
        self.is_in_aes_mode
    }  

    pub fn set_status(&mut self, s: HandshakeStatus) {
        self.status_raw = s.to_raw();
    }

    pub fn get_status(&self) -> HandshakeStatus {
        HandshakeStatus::from_raw(self.status_raw)
    }

    pub fn is_handshake(&self, tgt: HandshakeStatus) -> bool {
        HandshakeStatus::from_raw(self.status_raw) == tgt
    }

    pub fn set_transport_to(&mut self, new_transport: *mut c_void) {
        stcp_dbg!("Setting transport to {:?}", new_transport);
        self.transport = new_transport as *mut core::ffi::c_void;
    }

    pub fn parse_payload_len(header: &[u8]) -> Result<usize, StcpError> {

        let mut len_bytes = [0u8;4];
        len_bytes.copy_from_slice(&header[0..4]);

        let payload_len = u32::from_be_bytes(len_bytes) as usize;

        if payload_len > STCP_MAX_FRAME {
            return Err(StcpError::HeaderSizeMismatch);
        }

        Ok(payload_len)

    }

    pub fn recv_exact(
        &mut self,
        transport: *mut core::ffi::c_void,
        buf: &mut [u8],
        want: usize,
    ) -> Result<(), StcpError> {

        let mut off = 0;

        while off < want {

            let rc = stcp_tcp_recv_once!(
                transport,
                &mut buf[off..off + (want - off)],
                0 // <= Blokkaa, tärkeä
            );
            stcp_dbg!("recv_once @ recv_exact, rc={}", rc);
            if rc <= 0 {

                if rc == -EAGAIN as isize {
                    return Err(StcpError::Again);
                }

                stcp_dbg!("recv_exact: rc={}", rc);
                return Err(StcpError::ProtoError);
            }

            off += rc as usize;
        }

        Ok(())
    }

    /// Lue STCP frame header ja payload
    pub fn recv_header_and_payload(
        &mut self,
        transport: *mut core::ffi::c_void,
        hdr: &mut [u8],
        payload: &mut [u8],
    ) -> Result<i32, StcpError> {

        stcp_dbg!("recv_header_and_payload: reading header ({} bytes)", hdr.len());

        match self.recv_exact(transport, hdr, hdr.len()) {

            Ok(_) => {}

            Err(StcpError::WouldBlock) => {
                stcp_dbg!("recv_header_and_payload: header WouldBlock");
                return Ok(0);
            }

            Err(e) => return Err(e),
        }

        let header = StcpMessageHeader::try_from_bytes_be(hdr)
            .ok_or(StcpError::HeaderSizeMismatch)?;

        let msg_len = header.msg_len as usize;

        stcp_dbg!(
            "recv_header_and_payload: header ok, reading payload ({} bytes)",
            msg_len
        );

        if msg_len > STCP_MAX_TCP_PAYLOAD_SIZE {
            stcp_dbg!("recv_header_and_payload: payload too large: {}", msg_len);
            return Err(StcpError::HeaderSizeMismatch);
        }

        match self.recv_exact(transport, payload, msg_len) {

            Ok(_) => {}

            Err(StcpError::WouldBlock) => {
                stcp_dbg!("recv_header_and_payload: payload WouldBlock");
                return Ok(0);
            }

            Err(e) => return Err(e),
        }

        let data_in: Vec<u8> = payload[..msg_len].to_vec();
        stcp_dump!("DataIN", &data_in);

        stcp_dbg!("RAW RX: {:02X?}", data_in);
        let plain = if self.aes.is_some() {
            self.aes
                .as_ref()
                .and_then(|aes| aes.decrypt(&data_in))
                .ok_or(StcpError::ProtoError)?

        } else {

            data_in

        };

        let dl = plain.len();

        payload[..dl].copy_from_slice(&plain[..dl]);

        stcp_dbg!("DECRYPTED: {} bytes", dl);
        stcp_dump!("DECRYPTED payload", &plain);

        Ok(dl as i32)
    }

    /// Lähetä STCP frame
    pub fn send_message(&mut self, data: &[u8]) -> Result<usize, StcpError> {
        let (mtype, dout) = if self.aes.is_some() {
            stcp_dbg!("Sending AES message...");
            let aes = self.get_aes()?;
            let enc = aes.encrypt(data);
            (StcpMsgType::Aes, enc)
        } else {
            stcp_dbg!("Sending plain message...");
            (StcpMsgType::Public, data.to_vec())
        };

        stcp_dbg!("Sending {} bytes", dout.len());
        stcp_dump!("Sending data", &dout);

        let frame_len = build_frame_and_send(self.transport, mtype, &dout);
        Ok(frame_len as usize)
    }

    pub fn parse_header(&mut self, hdr: &[u8]) -> (StcpMsgType, usize) {
        let msg_type = u32::from_be_bytes([hdr[8], hdr[9], hdr[10], hdr[11]]);
        let len = u32::from_be_bytes([hdr[12], hdr[13], hdr[14], hdr[15]]);

        let mtype = match msg_type {
            0 => StcpMsgType::Unknown,
            1 => StcpMsgType::Error,
            2 => StcpMsgType::Public,
            3 => StcpMsgType::Aes,
            _ => StcpMsgType::Unknown,
        };

        (mtype, len as usize)
    }

    /// Lue kokonainen STCP-viesti
    pub fn recv_message(&mut self) -> Result<Vec<u8>, StcpError> {
        let hdr_sz = stcp_message_get_header_size_in_bytes();
        let payload_sz = STCP_MAX_TCP_PAYLOAD_SIZE;

        let mut hdr_buf = vec![0u8; hdr_sz];
        let mut payload_buf = vec![0u8; payload_sz];

        stcp_dbg!("Starting to receive STCP message...");

        let payload_len = self.recv_header_and_payload(
            self.transport,
            &mut hdr_buf,
            &mut payload_buf
        )?;

        let (mtype, _) = self.parse_header(&hdr_buf);
        let payload = &payload_buf[..payload_len as usize];

        stcp_dbg!("Payload length: {}", payload_len);
        stcp_dbg!("Message type: {:?}", mtype);
        stcp_dump!("Header", &hdr_buf);
        stcp_dump!("Payload", payload);

        let out = payload.to_vec();

        stcp_dump!("Final payload", &out);
        stcp_dbg!("Final length: {} bytes", out.len());

        Ok(out)
    }    

    // Funktio on vain testikäyttöön!
   #[cfg(target_arch = "x86_64")] 
   pub fn recv_from_stream(&mut self, stream: &TcpStream) -> Result<Vec<u8>, StcpError>  {
        stcp_dbg!("Recv Start!");
        let msg = match self.recv_message() {
            Ok(v) => {
                stcp_dbg!("Received message: {:?}", v);
                Ok(v)
            },
            Err(e) => {
                stcp_dbg!("Error while receiving message: {:?}", e);
                Err(e)
            },
        };
        msg
    }
}

