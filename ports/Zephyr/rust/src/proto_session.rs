extern crate alloc;

use core::ffi::c_int;
use core::ffi::c_void;
use crate::tcp_io::stcp_tcp_recv;
use crate::stcp_tcp_recv_once;
use alloc::vec;
use alloc::vec::Vec;
use crate::stcp_dbg;
use crate::stcp_dump;
use crate::aes;
use crate::aes::StcpAesCodec;
use core::panic::Location;
use crate::session_handler::rust_session_destroy;
use crate::slice_helpers::StcpError;
use crate::types::kernel_socket;
use crate::types::*;
use crate::errorit::*;

use crate::helpers::*;
use crate::helpers;


use crate::slice_helpers::*;

#[cfg(feature = "std")]
use std::net::{TcpListener, TcpStream};

use crate::stcp_message::build_frame_and_send;
use crate::stcp_message::stcp_message_get_header_size_in_bytes;


/* ========================= SESSION ========================= */

pub struct ProtoSession {
    pub private_key: StcpEcdhSecret,
    pub public_key: StcpEcdhPubKey,
    pub shared_key: StcpEcdhSecret,
    pub status_raw: u32,
    pub is_server: bool,
    pub is_freed: bool,
    pub transport: *mut kernel_socket,
    pub rx_buff: PeekRxBuff,
    pub aes: Option<StcpAesCodec>,
}

impl ProtoSession {

    pub fn new(is_server: bool, transport_vp: *mut c_void) -> Self {
        let transp = transport_vp as *mut kernel_socket;

        Self {
            private_key: StcpEcdhSecret::new(),
            public_key: StcpEcdhPubKey::new(),
            shared_key: StcpEcdhSecret::new(),
            status_raw: HandshakeStatus::Init.to_raw(),
            is_server: false,
            is_freed: false,
            transport: transp,
            rx_buff: PeekRxBuff::new(),
            aes: None,
        }
    }

    pub fn init_aes_with(&mut self, theSK: StcpEcdhSecret) {
        stcp_dbg!("Initialising with AES....");
        let sk = theSK.to_bytes_be();
        self.aes = Some(StcpAesCodec::new(&sk));
        self.set_status(HandshakeStatus::Aes);
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



    pub fn set_is_server(&mut self, v: bool) { self.is_server = v; }
    pub fn get_is_server(&self) -> bool { self.is_server }
    pub fn in_aes_mode(&self) -> bool {
        self.aes.is_some()
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

    pub fn set_transport_to(&mut self, newTransport: *mut c_void) {
        stcp_dbg!("Setting transport to {:?}", newTransport);
        self.transport = newTransport as *mut kernel_socket;
    }

    pub fn recv_header_and_payload(
        &mut self,
        transport: *mut kernel_socket,
        hdr: &mut [u8],
        payload: &mut [u8],
    ) -> Result<i32, StcpError> {
        stcp_dbg!("recv_header_and_payload: reading header ({} bytes)", hdr.len());

        //let rc = self.rx_buff.read_data_to(self.transport, hdr);
        let rc1 = stcp_tcp_recv_once!(transport, hdr, hdr.len() as i32);
        if rc1 <= 0 {
            stcp_dbg!("recv_header_and_payload: header read failed rc={}", rc1);
            return Err(StcpError::HeaderSizeMismatch);
        }

        let header = StcpMessageHeader::try_from_bytes_be(hdr).unwrap();

        let msgLen = header.msg_len;
        stcp_dbg!("recv_header_and_payload: header ok, reading payload ({}/{} bytes)", msgLen, payload.len());

        if msgLen > 64*1024 {
            stcp_dbg!("recv_header_and_payload: Insane payload length: {}, rc: {}", msgLen, rc1);
            return Err(StcpError::HeaderSizeMismatch);
        }

        //let rc2 = self.rx_buff.read_data_to(transport, payload);
        let rc2 = stcp_tcp_recv_once!(transport, payload, msgLen as i32);
        if rc2 <= 0 {
            stcp_dbg!("recv_header_and_payload: payload read failed rc={}", rc2);
            return Err(StcpError::PayloadSizeMismatch);
        }

        stcp_dbg!("Got payload RC {} // ML {}", rc2 , msgLen);
        let data_in: Vec<u8> = payload[..msgLen as usize].to_vec();
        stcp_dump!("DataIN", &data_in);

        let plain: Vec<u8> = if self.in_aes_mode() {
            match self.aes.as_ref().and_then(|aes| aes.decrypt(&data_in)) {
                Some(p) => p,
                None => {
                    stcp_dbg!("AES decrypt failed");
                    return Err(StcpError::Invalid);
                }
            }
        } else {
            data_in
        };

        let dl = plain.len();
        payload[..dl].copy_from_slice(&plain[..dl]);

        stcp_dbg!("DECRYPTED: Plain data {} bytes", plain.len());
        stcp_dump!("DECRYPTED: Plain data", &plain);

        stcp_dbg!("Payload data out {} bytes", dl);
        stcp_dump!("Payload data out", &payload[..dl]);
        Ok(dl as i32)
    }

    pub fn send_message(
        &mut self,
        data: &[u8],
    ) -> Result<usize, StcpError> {

        let (mtype, dout): (StcpMsgType, Vec<u8>) = if self.in_aes_mode() {
            stcp_dbg!("Sending AES message...");
            let aes = self.get_aes().unwrap();
            let enc = aes.encrypt(data);
            (StcpMsgType::Aes, enc)
        } else {
            stcp_dbg!("Sending plain message...");
            (StcpMsgType::Public, data.to_vec())
        };

        stcp_dbg!("Sending data {} bytes", dout.len());
        stcp_dump!("Sending data", &dout);

        let frame_len = build_frame_and_send(self.transport, mtype, &dout);
        Ok(frame_len as usize)
    }

    pub fn recv_message(
        &mut self,
    ) -> Result<Vec<u8>, StcpError> {

        let hdr_sz = stcp_message_get_header_size_in_bytes();
        let payload_sz = STCP_MAX_TCP_PAYLOAD_SIZE;

        let mut hdr_buf     = vec![0u8; hdr_sz];
        let mut payload_buf = vec![0u8; payload_sz];

        stcp_dbg!("Starting to receive......");
        match self.recv_header_and_payload(self.transport, &mut hdr_buf, &mut payload_buf) {
            Ok(paylen) => {
                 stcp_dbg!("Payload length: {}", paylen);
                stcp_dump!("Received header ", &hdr_buf);
                stcp_dump!("Received payload", &payload_buf[..paylen as usize]);
                let actual_len = core::cmp::min(
                    paylen as usize,
                    payload_buf.len()
                ) as usize;

                let mut out = Vec::with_capacity(actual_len);
                out.extend_from_slice(&payload_buf[..actual_len]);
                stcp_dump!("Final payload   ", &out);
                stcp_dbg!("Final: {} bytes.", out.len());
                Ok(out)
            },
            Err(e) => {
                stcp_dbg!("Error while fetching message: {:?}", e);
                Err(e)
            }
        }
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
