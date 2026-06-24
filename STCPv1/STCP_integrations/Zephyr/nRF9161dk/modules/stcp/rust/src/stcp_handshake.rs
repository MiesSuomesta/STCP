use core::ffi::c_int;

use crate::types::{
  //    ProtoOps,
        HandshakeStatus,
        //StcpMsgType,
        StcpEcdhPubKey,
        //StcpMessageHeader,
        STCP_ECDH_PUB_LEN,
    };

//use crate::aes::StcpAesCodec;
use crate::tcp_io;
//use alloc::boxed::Box;
//use crate::stcp_handshake::{client_handshake, server_handshake};
use crate::crypto::Crypto;
use crate::{stcp_dbg, stcp_dump};
use crate::errorit::{EBADF,EAGAIN,ECONNABORTED,EINVAL};
//use alloc::vec::Vec;
//use alloc::vec;
//use core::panic::Location;
use crate::proto_session::ProtoSession;
//use crate::helpers::stcp_sleep_msec;

//use crate::abi::stcp_exported_rust_ctx_alive_count;

// TCP helpperi makrot
use crate::tcp_io::stcp_tcp_recv;
//use crate::stcp_tcp_send_all;
//use crate::stcp_tcp_recv_until_buffer_full;

/// Clientti handshake 

pub fn handshake_generate_keys(sess: *mut ProtoSession) -> i32 {

    // Eksplisiittinen &mut-viite, EI generiikkaa
    let s: &mut ProtoSession = unsafe { &mut *sess };
    stcp_dbg!("Generate keys ===================================");   

    let ret = Crypto::generate_keypair(s);
    if s.is_server {
      stcp_dbg!("Server Generate keys");   
    } else {
      stcp_dbg!("Client Generate keys");   
    }

    ret
}

pub fn handshake_send_public_key(sess: *mut ProtoSession, transport: *mut core::ffi::c_void) -> isize {

    // Eksplisiittinen &mut-viite, EI generiikkaa
    let s: &mut ProtoSession = unsafe { &mut *sess };

    let _sock = transport as *mut core::ffi::c_void;
    
    stcp_dbg!("Sending as server {}", s.get_is_server());
    stcp_dbg!("Sending public key ==================================");   

    // Raakana pitää lähettää
    let the_public_key_bytes = s.public_key.to_bytes_be();
    stcp_dump!("TX PUBKEY FRAME", &the_public_key_bytes);
    let sent = unsafe { tcp_io::stcp_tcp_send(transport, the_public_key_bytes.as_ptr(), the_public_key_bytes.len()) };
    stcp_dbg!("Public key sent: {:?}, {:?}", sent, the_public_key_bytes.len());   

    sent
}

#[inline(never)]
pub fn handshake_recv_public_key(
    transport: *mut core::ffi::c_void,
    out_pubkey: &mut StcpEcdhPubKey,
) -> isize {

    let mut buf = [0u8; STCP_ECDH_PUB_LEN];
    let mut off = 0;

    while off < STCP_ECDH_PUB_LEN {

        let mut ret_len: c_int = 0;

        let rc = unsafe {
            stcp_tcp_recv(
                transport,
                buf[off..].as_mut_ptr(),
                (STCP_ECDH_PUB_LEN - off) as usize,
                0,
                0,
                &mut ret_len
            )
        };

        if rc == -EAGAIN as isize {
            return -EAGAIN as isize;
        }

        if rc <= 0 {
            return rc;
        }

        off += ret_len as usize;
    }

    stcp_dump!("RX PUBKEY FRAME", &buf);

    *out_pubkey = StcpEcdhPubKey::from_bytes_be(&buf);

    STCP_ECDH_PUB_LEN as isize
}

pub fn stcp_do_handshake_pub_keys(
    sess: *mut ProtoSession,
    transport: *mut core::ffi::c_void,
    out_pubkey: &mut StcpEcdhPubKey,

) -> isize {
  
  stcp_dbg!("Client handshake: Public key sending ....");
  let rc1 = handshake_send_public_key(sess, transport);
  if rc1 < 0 {
    stcp_dbg!("[client] Error while sending PK: {}", rc1);
    return rc1;
  }
  stcp_dbg!("[client] Public key sent: {}", rc1);

  stcp_dbg!("Client handshake: Public key receiving ....");
  let rc2 = handshake_recv_public_key(transport, out_pubkey);
  if rc2 < 0 {
    stcp_dbg!("[client] Error while receiving PK: {}", rc2);
    return rc2;
  }

  stcp_dbg!("[client] Public key recv: {}", rc2);

  return 1;
}



#[unsafe(no_mangle)]
pub extern "C" fn rust_session_handshake_lte(sess_vp: *mut core::ffi::c_void, transport: *mut core::ffi::c_void) -> i32 {

  stcp_dbg!("========================================================");   
  stcp_dbg!("=== START HANDSHAKE ====================================");   
  stcp_dbg!("========================================================");   
  let sock = transport as *mut core::ffi::c_void;

  if sess_vp.is_null() {
    return -EBADF;
  }

  if sock.is_null() {
    stcp_dbg!("Client Worker NO transport");   
    return -EBADF;
  }

  let s = unsafe { &mut *(sess_vp as *mut ProtoSession) };

  let mut incoming_pubkey = StcpEcdhPubKey::new();
  let mut rc = -EAGAIN;
  let mut tries = 0;

  stcp_dbg!("Starting to generate keys.....");
  let ret_gen = handshake_generate_keys(s);
  stcp_dbg!("=HS= Generated Keys... ret: {}", ret_gen);

  while  (rc == -EAGAIN) && (tries < 10) {
    rc = stcp_do_handshake_pub_keys(s, transport, &mut incoming_pubkey) as i32;
    tries += 1;
    stcp_dbg!("Handshake PK try: {} => {}", tries, rc);   
  } 

  if rc < 0 {
    stcp_dbg!("Handshake: error in hs: {}", rc);   
    return rc;
  }

  let incoming_pubkey_bytes = incoming_pubkey.to_bytes_be();
  let incoming_pubkey_bytes_64: &[u8; 64] = 
      incoming_pubkey_bytes.as_slice().try_into().expect("pubkey wrong length");

  stcp_dump!("=HS= Peer Public key", incoming_pubkey_bytes_64);   

  let ret_shared = Crypto::compute_shared_from_bytes(
                s, incoming_pubkey_bytes_64);

  stcp_dbg!("=HS= Shared key calculation returned: {}", ret_shared);   

  s.set_status(HandshakeStatus::Aes);
  s.init_aes_with(s.shared_key.clone());
  
  stcp_dbg!("==== HS complete => AES MODE =====");
  stcp_dbg!("==== HS complete => AES MODE =====");
  stcp_dbg!("==== HS complete => AES MODE =====");
  return 1; // EI nolla, vaan ykkönen!
}
