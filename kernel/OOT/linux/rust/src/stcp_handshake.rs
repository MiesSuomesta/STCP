
use crate::types::{
  //    ProtoOps,
        ProtoSession,
        HandshakeStatus,
        StcpMsgType,
        StcpEcdhPubKey,
        StcpMessageHeader,
        STCP_ECDH_PUB_LEN,
        kernel_socket,
        STCP_TAG_BYTES,
        STCP_TCP_RECV_BLOCK,
        STCP_TCP_RECV_NO_BLOCK,
    };


//use crate::helpers::{tcp_send_all, tcp_recv_once, tcp_recv_exact, tcp_peek_max, get_session};
//use crate::tcp_io;
//use alloc::boxed::Box;
//use crate::stcp_handshake::{client_handshake, server_handshake};
use crate::crypto::Crypto;
use crate::{stcp_dbg, stcp_dump};
use crate::errorit::{EBADF,EAGAIN,ECONNABORTED,EINVAL};
use crate::stcp_message::*;
use alloc::vec::Vec;
use alloc::vec;

//use crate::abi::stcp_exported_rust_ctx_alive_count;

// TCP helpperi makrot
//use crate::stcp_tcp_recv_once;
//use crate::stcp_tcp_send_all;
use crate::stcp_tcp_recv_exact;
use crate::stcp_tcp_peek_max;
//use crate::stcp_tcp_recv_until_buffer_full;

/// Clientti handshake 

pub fn handshake_generate_keys(sess: *mut ProtoSession) -> i32 {

    // Eksplisiittinen &mut-viite, EI generiikkaa
    let s: &mut ProtoSession = unsafe { &mut *sess };

    let ret = Crypto::generate_keypair(s);
    if s.is_server {
      stcp_dbg!("Server Generate keys");   
    } else {
      stcp_dbg!("Client Generate keys");   
    }

    ret
}

pub fn handshake_send_public_key(sess: *mut ProtoSession, transport: *mut kernel_socket) -> isize {

    // Eksplisiittinen &mut-viite, EI generiikkaa
    let s: &mut ProtoSession = unsafe { &mut *sess };

    let sock = transport as *mut kernel_socket;
    
    if s.is_server {
      stcp_dbg!("Server Sending public key");   
    } else {
      stcp_dbg!("Client Sending public key");   
    }

    let mut the_public_key_bytes = s.public_key.to_bytes_be();
    let sent = stcp_message_send_frame(s, sock, StcpMsgType::Public, &mut the_public_key_bytes);

    if s.is_server {
      stcp_dbg!("Server Public key sent ret");   
    } else {
      stcp_dbg!("Client Public key sent ret");   
    }

    sent as isize
}

#[inline(never)]
pub fn handshake_recv_public_key(
    _sess: *mut ProtoSession,
    transport: *mut kernel_socket,
    out_header: &mut StcpMessageHeader,
    out_pubkey: &mut StcpEcdhPubKey,
    no_blocking: i32,
  ) -> isize {

  let hdr_size = stcp_message_get_header_size_in_bytes();
  let flags = if no_blocking != 0 { STCP_TCP_RECV_NO_BLOCK } else { STCP_TCP_RECV_BLOCK };

  // 1) Lue header tarkasti
  let mut hdr_buf: Vec<u8> = vec![0u8; hdr_size];
  let r1 = stcp_tcp_peek_max!(transport, &mut hdr_buf, flags);
  if r1 < 0 {
      // typillisesti -EAGAIN nonblockissa → worker resched
      *out_header = StcpMessageHeader::new();
      *out_pubkey = StcpEcdhPubKey::new();
      return r1 as isize;
  }

  let header = stcp_message_form_a_header_from_data(&hdr_buf);
  if header.tag != STCP_TAG_BYTES {
      stcp_dbg!("Recv PK: invalid tag in header");
      *out_header = StcpMessageHeader::new();
      *out_pubkey = StcpEcdhPubKey::new();
      return -EAGAIN as isize;
  }


  let payload_len = header.msg_len as usize;
  if payload_len != STCP_ECDH_PUB_LEN {
      stcp_dbg!("Recv PK: invalid payload_len {}, expected {}", payload_len, STCP_ECDH_PUB_LEN);
      *out_header = StcpMessageHeader::new();
      *out_pubkey = StcpEcdhPubKey::new();
      return -EINVAL as isize;
  }

  // TÄRKEÄ: Syödään headeri RX jonosta => payloadi seuraava
  stcp_dbg!("Eatign up header...");
  _ = stcp_tcp_recv_exact!(transport, &mut hdr_buf, flags);

  // 2) Lue payload tarkasti palyod + hdr_size
  let mut payload: Vec<u8> = vec![0u8; payload_len];
  let r2 = stcp_tcp_recv_exact!(transport, &mut payload, flags);
  if r2 < 0 {
      *out_header = StcpMessageHeader::new();
      *out_pubkey = StcpEcdhPubKey::new();
      return r2 as isize;
  }

  stcp_dump!("Recv/PK payload", &payload);

  *out_header = header;
  *out_pubkey = StcpEcdhPubKey::from_bytes_be(&payload);
  payload_len as isize

}

#[unsafe(no_mangle)]
pub extern "C" fn rust_session_handshake_done(sess: *mut ProtoSession) -> i32 {

    if sess.is_null() {
        stcp_dbg!("NO SESSION");   
        return -EBADF;
    }

    let s = unsafe { &mut *sess };

    let is_done = s.is_handshake(HandshakeStatus::Complete);
    stcp_dbg!("Session is handshake done");   

    if is_done {
       return 1;
    }

    return 0;
}



#[unsafe(no_mangle)]
pub extern "C" fn rust_session_client_handshake(sess: *mut ProtoSession, transport: *mut kernel_socket) -> i32 {
  stcp_dbg!("Client Starting");   
  // Check alive count
/*
  let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
  stcp_dbg!("There are alive instances");   
  if alive < 1 {
    stcp_dbg!("Should not be alive anymore");   
    return -500;
  }
*/
  stcp_dbg!("Client Worker Client handshake starting");   
  let sock = transport as *mut kernel_socket;

  if sess.is_null() {
    stcp_dbg!("Client Worker NO SESSION");   
    return -EBADF;
  }

  stcp_dbg!("Client Worker Server CP 1");   

  if sock.is_null() {
    stcp_dbg!("Client Worker NO transport");   
    return -EBADF;
  }

  stcp_dbg!("Client Worker Checks passed starting");   
  let s = unsafe { &mut *sess };
  let status = s.get_status();

    // Generoi omat avaimet
  stcp_dbg!("Client Worker State machine: {}", status as i32);   

  match status {
      HandshakeStatus::Init => {
        stcp_dbg!("*C*");
        stcp_dbg!("*C* Client STATE: Init");
        stcp_dbg!("*C*");
        let ret_gen = handshake_generate_keys(s);
        stcp_dbg!("=C= Generated Keys... ret: {}", ret_gen);

        let ret_pub = handshake_send_public_key(sess, transport);
        stcp_dbg!("=C= Sent public key... ret: {}", ret_pub);

        s.set_status(HandshakeStatus::Public);
        stcp_dbg!("=C= Status set to Public");
        return -EAGAIN; // Uudelleen workkeri käynnistymään
      }

      HandshakeStatus::Public => {
        stcp_dbg!("*C*");
        stcp_dbg!("*C* Client STATE: Public");
        stcp_dbg!("*C*");
        let mut incoming_header = StcpMessageHeader::new();
        let mut incoming_pubkey = StcpEcdhPubKey::new();

        let ret_recv = handshake_recv_public_key(sess, transport, &mut incoming_header, &mut incoming_pubkey, STCP_TCP_RECV_NO_BLOCK);
        stcp_dbg!("=C= Receiving of peer public key returned: {}", ret_recv);   

        // EI edetä ennen kuin oikea frame on oikeasti saatu
        if ret_recv == -EAGAIN as isize {
            return -EAGAIN;
        }

        if ret_recv < 0 {
            s.set_status(HandshakeStatus::Error);
            return ret_recv as i32;
        }

        if incoming_header.msg_type != StcpMsgType::Public {
            stcp_dbg!("=C= ERROR: Expected Public frame, got {}", incoming_header.msg_type.to_raw());
            return -EAGAIN as i32;
        }

        let incoming_pubkey_bytes = incoming_pubkey.to_bytes_be();
        let incoming_pubkey_bytes_64: &[u8; 64] = 
            incoming_pubkey_bytes.as_slice().try_into().expect("pubkey wrong length");

        stcp_dump!("=C= Peer Public key", incoming_pubkey_bytes_64);   

        let ret_shared = Crypto::compute_shared_from_bytes(
                      s, incoming_pubkey_bytes_64);

        stcp_dbg!("=C= Shared key calculation returned: {}", ret_shared);   

        s.set_status(HandshakeStatus::Complete);
        stcp_dbg!("=C= Client Worker Hanshake complete");   
        return -EAGAIN; // Uudelleen workkeri käynnistymään
      }

      HandshakeStatus::Complete => {
        stcp_dbg!("***");
        stcp_dbg!("*** Client STATE: Complete");
        stcp_dbg!("***");

        s.set_status(HandshakeStatus::Aes);
        stcp_dbg!("Client Worker Hanshake complete");   
        return 0;
      }

      HandshakeStatus::Aes => {
        stcp_dbg!("***");
        stcp_dbg!("*** Client STATE: AES");
        stcp_dbg!("***");

        stcp_dbg!("Client Worker AES MODE");   
        return 1;
      }

      HandshakeStatus::Error => {
        stcp_dbg!("***");
        stcp_dbg!("*** Client STATE: Error");
        stcp_dbg!("***");

        stcp_dbg!("Client Worker STATE Error");   
        return -5;
      }
    };
}

/// Serveri handshake 
#[unsafe(no_mangle)]
pub extern "C" fn rust_session_server_handshake(sess: *mut ProtoSession, transport: *mut kernel_socket) -> i32 {
  // Check alive count
  stcp_dbg!("Server Starting");   
/*
  let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
  stcp_dbg!("There are alive instances");   
  if alive < 1 {
    stcp_dbg!("Should not be alive anymore");   
    return -500;
  }
*/

  if sess.is_null() {
    stcp_dbg!("Server NO SESSION");   
    return -EBADF;
  }

  stcp_dbg!("Server Server CP 1");   

  if transport.is_null() {
    stcp_dbg!("Server NO transport");   
    return -EBADF;
  }


  stcp_dbg!("Server Server CP 2");   

  //let sock = transport as *mut kernel_socket;
  let s = unsafe { &mut *sess };


  let status = s.get_status();
  stcp_dbg!("Server Server CP 3");   

  stcp_dbg!("Server Worker State machine: {}", status as i32);   
  let mut incoming_header = StcpMessageHeader::new();
  let mut incoming_pubkey = StcpEcdhPubKey::new();

  match status {
      HandshakeStatus::Init => {
        stcp_dbg!("*S*");
        stcp_dbg!("*S* Server STATE: Init");
        stcp_dbg!("*S*");
        let ret_gen = handshake_generate_keys(s);
        stcp_dbg!("=S= Generated Keys... ret: {}", ret_gen);

        s.set_status(HandshakeStatus::Public);
        stcp_dbg!("=S= Status set to Public");
        return -EAGAIN; // Uudelleen workkeri käynnistymään
      }

      HandshakeStatus::Public => {
        stcp_dbg!("*S*");
        stcp_dbg!("*S* Server STATE: Public");
        stcp_dbg!("*S*");

        let ret_recv = handshake_recv_public_key(sess, transport, &mut incoming_header, &mut incoming_pubkey, STCP_TCP_RECV_NO_BLOCK);
        stcp_dbg!("=S= Receiving of peer public key returned: {}", ret_recv);   

        if ret_recv == -EAGAIN as isize {
            return -EAGAIN;
        }

        if ret_recv < 0 {
            s.set_status(HandshakeStatus::Error);
            return ret_recv as i32;
        }

        if incoming_header.msg_type != StcpMsgType::Public {
            stcp_dbg!("=S= ERROR: Expected Public frame, got {}", incoming_header.msg_type.to_raw());
            return -EAGAIN;
        }

        let incoming_pubkey_bytes = incoming_pubkey.to_bytes_be();
        let incoming_pubkey_bytes_64: &[u8; 64] = 
            incoming_pubkey_bytes.as_slice().try_into().expect("pubkey wrong length");

        stcp_dump!("=C= Peer Public key", incoming_pubkey_bytes_64);   

        let ret_shared = Crypto::compute_shared_from_bytes(
                      s, incoming_pubkey_bytes_64);
                      
        stcp_dbg!("=S= Shared key calculation returned: {}", ret_shared);   

        let ret_pub = handshake_send_public_key(sess, transport);
        stcp_dbg!("=S= Sent public key... ret: {}", ret_pub);

        
        s.set_status(HandshakeStatus::Complete);
        stcp_dbg!("=S= Server Worker Hanshake complete");   
        return -EAGAIN; // Uudelleen workkeri käynnistymään
      }

      HandshakeStatus::Complete => {
        stcp_dbg!("*S*");
        stcp_dbg!("*S* Server STATE: Handshake Complete");
        stcp_dbg!("*S*");

        s.set_status(HandshakeStatus::Aes);
        stcp_dbg!("Client Worker Hanshake complete");   
        return 0;
      }

      HandshakeStatus::Aes => {
        stcp_dbg!("*S*");
        stcp_dbg!("*S* Server STATE: AES mode");
        stcp_dbg!("*S*");
        return 1; // Marking complete => no worker resched after this point
      }

      HandshakeStatus::Error => {
        stcp_dbg!("***");
        stcp_dbg!("*** Server STATE: Error");
        stcp_dbg!("***");

        stcp_dbg!("Client Worker STATE Error");   
        return -ECONNABORTED;
      }
    };
}
