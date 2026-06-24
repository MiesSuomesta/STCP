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
use crate::errorit::{EBADF,EAGAIN,EALREADY,ECONNABORTED,EINVAL,ETIMEDOUT};
//use alloc::vec::Vec;
//use alloc::vec;
//use core::panic::Location;
use crate::proto_session::ProtoSession;
//use crate::helpers::stcp_sleep_msec;

//use crate::abi::stcp_exported_rust_ctx_alive_count;
use core::time::Duration;

// TCP helpperi makrot
use crate::tcp_io::stcp_tcp_recv;
//use crate::stcp_tcp_send_all;
//use crate::stcp_tcp_recv_until_buffer_full;

// Clientti handshake 

unsafe extern "C" {
  pub fn stcp_api_get_errno() -> i32;
}

pub fn get_errno() -> i32 {
  unsafe { stcp_api_get_errno() }
}

pub fn handshake_generate_keys(sess: *mut ProtoSession) -> i32 {

    // Eksplisiittinen &mut-viite, EI generiikkaa
    let s: &mut ProtoSession = unsafe { &mut *sess };
    stcp_dbg!("Generate keys ===================================");   

    let ret = Crypto::generate_keypair(s);
    ret
}

pub fn handshake_send_public_key(sess: *mut ProtoSession, transport: *mut core::ffi::c_void) -> isize {

    stcp_dbg!("CLIENT ABOUT TO SEND PUBKEY");
    // Eksplisiittinen &mut-viite, EI generiikkaa
    let s: &mut ProtoSession = unsafe { &mut *sess };

    let _sock = transport as *mut core::ffi::c_void;

    if s.is_my_public_key_sent {
      stcp_dbg!("[Handshake] Public key already sent ==================================");   
      return -EALREADY as isize
    }
    s.is_my_public_key_sent = true;

    stcp_dbg!("[Handshake] Sending public key ==================================");   

    // Raakana pitää lähettää
    let the_public_key_bytes = s.public_key.to_bytes_be();
    stcp_dump!("[Handshake] TX PUBKEY FRAME", &the_public_key_bytes);
    let sent = unsafe { tcp_io::stcp_tcp_send(transport, the_public_key_bytes.as_ptr(), the_public_key_bytes.len()) };
    stcp_dbg!("[Handshake] Public key sent: {:?}, {:?}", sent, the_public_key_bytes.len());
    sent
}

#[inline(never)]
pub fn handshake_recv_public_key(
    sess: *mut ProtoSession,
    transport: *mut core::ffi::c_void,
    out_pubkey: &mut StcpEcdhPubKey,
) -> isize {

  stcp_dbg!("[Handshake] handshake_recv_public_key({:?}, {:?}, {:?})",
    sess, transport, out_pubkey);

  let s: &mut ProtoSession = unsafe { &mut *sess };
    stcp_dbg!("[Handshake] handshake_recv_public_key before while {:?} / {:?}",
      s.hs_rx_off,
      STCP_ECDH_PUB_LEN
    );

    stcp_dbg!(
        "[Handshake] SESSION PTR {:p} hs_rx_off BEFORE={}",
        s,
        s.hs_rx_off
    );

    while s.hs_rx_off < STCP_ECDH_PUB_LEN as isize {

      let remaining = (STCP_ECDH_PUB_LEN as isize - s.hs_rx_off) as usize;

      stcp_dbg!("[Handshake] handshake_recv_public_key while {:?} / {:?} Remaining: {:?}",
        s.hs_rx_off,
        STCP_ECDH_PUB_LEN,
        remaining
      );


      let slice: &mut [u8] =
          &mut s.hs_rx_buf[
              (s.hs_rx_off as usize)
              ..
              (s.hs_rx_off as usize + remaining)
          ];

      let mut ret_len: c_int = 0;

      let rc = unsafe {
          stcp_tcp_recv(
              transport,
              slice.as_mut_ptr(),
              slice.len(),
              0,
              (1<<20), // EXACT MODE 20th bit
              &mut ret_len
          )
      };

      stcp_dbg!("[Handshake] Recv ret {} => lenn: {}", rc, ret_len);

      if rc < 0 {

        if (rc == -EAGAIN as isize) {
          stcp_dbg!(
              "[Handshake] Public key recv: EAGAIN"
          );

          
          continue;
        }

        if (rc == -ETIMEDOUT as isize) {
          stcp_dbg!(
              "[Handshake] Public key recv: ETIMEDOUT"
          );
          continue;
        }

        stcp_dbg!(
            "[Handshake] Public key recv: no progress rc={} ret_len={}",
            rc,
            ret_len
        );
        return rc as isize;
      }
      s.hs_rx_off += ret_len as isize;

      stcp_dbg!(
          "[Handshake] SESSION PTR {:p} hs_rx_off AFTER={} ret_len={}",
          s,
          s.hs_rx_off,
          ret_len
      );
    }

    stcp_dump!("[Handshake] RX PUBKEY FRAME", &s.hs_rx_buf);

    *out_pubkey = StcpEcdhPubKey::from_bytes_be(&s.hs_rx_buf);
    
    // Init buffer...
    s.hs_rx_off = 0;
    s.hs_rx_buf.fill(0);
    
    STCP_ECDH_PUB_LEN as isize
}

pub fn stcp_do_handshake_pub_keys(
    sess: *mut ProtoSession,
    transport: *mut core::ffi::c_void,
    out_pubkey: &mut StcpEcdhPubKey,

) -> isize {
  
  let rc1 = handshake_send_public_key(sess, transport);
  stcp_dbg!("[Handshake] Public key sent, rc: {}", rc1);
  if rc1 < 0 {
    if rc1 == -EALREADY as isize {
      stcp_dbg!("[Handshake] Sent of public key already done");
    } else {
      stcp_dbg!("[Handshake] Sending error of public key: {}", rc1);
      return rc1;

    }
  }

  stcp_dbg!("[Handshake] Client handshake: Public key receiving ....");
  let rc2 = handshake_recv_public_key(sess, transport, out_pubkey);
  if rc2 < 0 {
    stcp_dbg!("[Handshake] Error while receiving PK: {}", rc2);
    return rc2;
  }

  stcp_dbg!("[Handshake] Public key recv: {}", rc2);

  return 1;
}



#[unsafe(no_mangle)]
pub extern "C" fn rust_session_handshake_lte(sess_vp: *mut core::ffi::c_void, transport: *mut core::ffi::c_void) -> i32 {

  stcp_dbg!("========================================================");   
  stcp_dbg!("=== START HANDSHAKE ====================================");   
  stcp_dbg!("========================================================");   
  stcp_dbg!("HS Args: Sess {:?}, Transport {:?}", sess_vp, transport);   
  let sock = transport as *mut core::ffi::c_void;

  if sess_vp.is_null() {
    stcp_dbg!("No session");   
    return -EBADF;
  }

  if sock.is_null() {
    stcp_dbg!("No transport");   
    return -EBADF;
  }

  let s = unsafe { &mut *(sess_vp as *mut ProtoSession) };

  let mut incoming_pubkey = StcpEcdhPubKey::new();
  let mut rc = -EAGAIN;

  if !s.are_keys_generated {
    stcp_dbg!("Starting to generate keys.....");
    let ret_gen = handshake_generate_keys(s);
    stcp_dbg!("=HS= Generated Keys... ret: {}", ret_gen);
  } else {
    stcp_dbg!("=HS= Generated Keys... Already done!");
  }
  s.are_keys_generated = true;

  stcp_dbg!("=HS= Doing key exchange....");   
  rc = stcp_do_handshake_pub_keys(s, transport, &mut incoming_pubkey) as i32;
  stcp_dbg!("Handshake Public keys rc: {}", rc);   

  if rc < 0 {
    stcp_dbg!("Handshake: error in hs: {}", rc);   
    return rc;
  }

  stcp_dbg!("Handshake: Processing incoming key...");   
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
