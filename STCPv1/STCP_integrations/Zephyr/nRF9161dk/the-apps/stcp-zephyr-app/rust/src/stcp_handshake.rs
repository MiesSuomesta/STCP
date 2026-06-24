use core::ffi::c_int;
use crate::types::{
  //    ProtoOps,
        HandshakeStatus,
        StcpMsgType,
        StcpEcdhPubKey,
        StcpMessageHeader,
        STCP_ECDH_PUB_LEN,
        kernel_socket,
    };

use crate::aes::StcpAesCodec;
use crate::tcp_io;
//use alloc::boxed::Box;
//use crate::stcp_handshake::{client_handshake, server_handshake};
use crate::crypto::Crypto;
use crate::{stcp_dbg, stcp_dump};
use crate::errorit::{EBADF,EAGAIN,ECONNABORTED,EINVAL};
use alloc::vec::Vec;
use alloc::vec;
use core::panic::Location;
use crate::proto_session::ProtoSession;
use crate::helpers::stcp_sleep_msec;

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

pub fn handshake_send_public_key(sess: *mut ProtoSession, transport: *mut kernel_socket) -> isize {

    // Eksplisiittinen &mut-viite, EI generiikkaa
    let s: &mut ProtoSession = unsafe { &mut *sess };

    let _sock = transport as *mut kernel_socket;
    
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
    sess: *mut ProtoSession,
    transport: *mut kernel_socket,
    out_pubkey: &mut StcpEcdhPubKey,
) -> isize {

    let s = unsafe { &mut *sess };
    let mut pubkey_buf = [0u8; STCP_ECDH_PUB_LEN];
    let mut ret_len: c_int = 0;

    let rc = unsafe {
        stcp_tcp_recv(transport, pubkey_buf.as_mut_ptr(), STCP_ECDH_PUB_LEN, 0, 0, &mut ret_len)
    };

    stcp_dbg!("Received public key, rc {}, ret_len {}", rc, ret_len);

    // 1️⃣ Peer sulki
    if rc == 0 {
        stcp_dbg!("❌ peer closed connection before pubkey");
        return -ECONNABORTED as isize;
    }

    // 2️⃣ Nonblock: ei vielä dataa
    if rc == -EAGAIN as isize {
        return -EAGAIN as isize;
    }

    // 3️⃣ Muu virhe
    if rc < 0 {
        stcp_dbg!("❌ recv error {}", rc);
        return rc as isize;
    }

    // 4️⃣ Tarkista oikea pituus
    if ret_len as isize != STCP_ECDH_PUB_LEN as isize {
        stcp_dbg!("❌ invalid pubkey length {}, expected {}", ret_len, STCP_ECDH_PUB_LEN);
        return -EINVAL as isize;
    }

    stcp_dump!("RX PUBKEY FRAME", &pubkey_buf);

    *out_pubkey = StcpEcdhPubKey::from_bytes_be(&pubkey_buf);
    STCP_ECDH_PUB_LEN as isize
}

pub fn client_handshake_pub_keys(
    sess: *mut ProtoSession,
    transport: *mut kernel_socket,
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
  let rc2 = handshake_recv_public_key(sess, transport, out_pubkey);
  if rc2 < 0 {
    stcp_dbg!("[client] Error while receiving PK: {}", rc2);
    return rc2;
  }

  stcp_dbg!("[client] Public key recv: {}", rc2);

  return 1;
}

pub fn server_handshake_pub_keys(
    sess: *mut ProtoSession,
    transport: *mut kernel_socket,
    out_pubkey: &mut StcpEcdhPubKey,

) -> isize {

  stcp_dbg!("Server handshake: Public key receiving ....");
  let rc2 = handshake_recv_public_key(sess, transport, out_pubkey);
  if rc2 < 0 {
    stcp_dbg!("[server] Error while receiving PK: {}", rc2);
    return rc2;
  }
  stcp_dbg!("[server] Public key recv: {}", rc2);

  
  stcp_dbg!("Server handshake: Public key sending ....");
  let rc1 = handshake_send_public_key(sess, transport);
  if rc1 < 0 {
    stcp_dbg!("[server] Error while sending PK: {}", rc1);
    return rc1;
  }
  stcp_dbg!("[server] Public key sent: {}", rc1);
  return 1;
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_session_client_handshake_lte(sess: *mut ProtoSession, transport: *mut kernel_socket) -> i32 {

  stcp_dbg!("========================================================");   
  stcp_dbg!("=== START CLIENT =======================================");   
  stcp_dbg!("========================================================");   
  let sock = transport as *mut kernel_socket;

  if sess.is_null() {
    stcp_dbg!("Client Worker NO SESSION");   
    return -EBADF;
  }

  stcp_dbg!("Client Worker CP 1");   

  if sock.is_null() {
    stcp_dbg!("Client Worker NO transport");   
    return -EBADF;
  }

  stcp_dbg!("Client Worker Checks passed starting");   

  let s = unsafe { &mut *sess };

  s.set_is_server(false);

  let mut incoming_pubkey = StcpEcdhPubKey::new();
  let mut rc = -EAGAIN;
  let mut tries = 0;

  stcp_dbg!("Starting to generate keys.....");
  let ret_gen = handshake_generate_keys(s);
  stcp_dbg!("=C= Generated Keys... ret: {}", ret_gen);

  while ( (rc == -EAGAIN) && (tries < 10) ) {
    rc = client_handshake_pub_keys(sess, transport, &mut incoming_pubkey) as i32;
    tries += 1;
    stcp_dbg!("Client PK try: {} => {}", tries, rc);   
  } 

  if (rc < 0) {
    stcp_dbg!("Client: error in hs: {}", rc);   
    return rc;
  }

  let incoming_pubkey_bytes = incoming_pubkey.to_bytes_be();
  let incoming_pubkey_bytes_64: &[u8; 64] = 
      incoming_pubkey_bytes.as_slice().try_into().expect("pubkey wrong length");

  stcp_dump!("=C= Peer Public key", incoming_pubkey_bytes_64);   

  let ret_shared = Crypto::compute_shared_from_bytes(
                s, incoming_pubkey_bytes_64);

  stcp_dbg!("=C= Shared key calculation returned: {}", ret_shared);   

  s.set_status(HandshakeStatus::Aes);
  let skBytes = s.shared_key.to_bytes_be();
  s.init_aes_with(s.shared_key.clone());
  stcp_dbg!("==== Client HS complete => AES MODE =====");
  stcp_dbg!("==== Client HS complete => AES MODE =====");
  stcp_dbg!("==== Client HS complete => AES MODE =====");
  return 1; // EI nolla, vaan ykkönen!
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_session_server_handshake_lte(sess: *mut ProtoSession, transport: *mut kernel_socket) -> i32 {

  stcp_dbg!("========================================================");   
  stcp_dbg!("=== START SERVER =======================================");   
  stcp_dbg!("========================================================");   
  let sock = transport as *mut kernel_socket;

  if sess.is_null() {
    return -EBADF;
  }


  if sock.is_null() {
    return -EBADF;
  }

  stcp_dbg!("Server Checks passed starting");   
  let s = unsafe { &mut *sess };

  s.set_is_server(true);

  let mut incoming_pubkey = StcpEcdhPubKey::new();
  let mut rc = -EAGAIN;
  let mut tries = 0;

  let ret_gen = handshake_generate_keys(s);
  stcp_dbg!("=S= Generated Keys... ret: {}", ret_gen);

  while ( (rc == -EAGAIN) && (tries < 10) ) {
    rc = server_handshake_pub_keys(sess, transport, &mut incoming_pubkey) as i32;
    tries += 1;
    stcp_dbg!("Client PK try: {} => {}", tries, rc);   
  } 

  if (rc < 0) {
    return rc;
  }

  let incoming_pubkey_bytes = incoming_pubkey.to_bytes_be();
  let incoming_pubkey_bytes_64: &[u8; 64] = 
      incoming_pubkey_bytes.as_slice().try_into().expect("pubkey wrong length");

  stcp_dump!("=S= Peer Public key", incoming_pubkey_bytes_64);   

  let ret_shared = Crypto::compute_shared_from_bytes(
                s, incoming_pubkey_bytes_64);

  stcp_dbg!("=S= Shared key calculation returned: {}", ret_shared);   

  s.set_status(HandshakeStatus::Aes);
  let skBytes = s.shared_key.to_bytes_be();
  s.init_aes_with(s.shared_key.clone());
  stcp_dbg!("==== Server HS complete => AES MODE =====");   
  stcp_dbg!("==== Server HS complete => AES MODE =====");   
  stcp_dbg!("==== Server HS complete => AES MODE =====");   
  return 1; // EI nolla, vaan ykkönen!
}
