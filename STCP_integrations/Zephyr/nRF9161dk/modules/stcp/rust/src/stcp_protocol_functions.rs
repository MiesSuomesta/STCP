
use core::ffi::c_void;
use core::ffi::c_int;
use crate::{stcp_dbg /* , stcp_dump, stcp_sess_transp */ };
use core::panic::Location;

use crate::slice_helpers::{stcp_make_mut_slice, StcpError};
use crate::session_handler::{rust_session_create, rust_session_destroy};
use crate::stcp_handshake::{
    rust_session_server_handshake_lte,
    rust_session_client_handshake_lte,
};
use crate::types::STCP_MAX_TCP_PAYLOAD_SIZE;
use crate::stcp_message::*;
use alloc::vec::Vec;
use crate::stcp_dump;
use crate::helpers;

//use alloc::boxed::Box;

use crate::types::{
  //    ProtoOps,
        kernel_socket,
        HandshakeStatus,
        StcpMsgType,
        StcpMessageHeader,
        STCP_TAG_BYTES,
        zsock_iovec,
        zsock_msghdr,
    };

use crate::errorit::*;
use crate::proto_session::ProtoSession;

use crate::tcp_io::stcp_tcp_send;
use crate::tcp_io::stcp_tcp_send_iovec;
use crate::tcp_io::stcp_tcp_recv;

//use crate::helpers::{tcp_recv_once, tcp_send_all, get_session};
//use crate::abi::{stcp_end_of_life_for_sk};
use crate::abi::stcp_exported_rust_ctx_alive_count;

// TCP helpperi makrot
//use crate::stcp_tcp_recv_once;
//use crate::stcp_tcp_send_all;
//use crate::stcp_tcp_recv_exact;
//use crate::stcp_tcp_peek_max;
//use crate::stcp_tcp_recv_until_buffer_full;

pub const STCP_REASON_NEXT_STEP : i32 = 3;

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_session_handshake_pump(sess: *mut ProtoSession, transport: *mut kernel_socket, reason: i32) -> i32 {

  if sess.is_null() {
    stcp_dbg!("Pump: no session");   
    return -EBADF;
  }

  if transport.is_null() {
    stcp_dbg!("Pump: NO transport");   
    return -EBADF;
  }

  let s = unsafe { &mut *sess };

  let server     = s.is_server;
  let status     = s.get_status();

  let sess_casted = sess as *mut c_void;
  let transport_casted = transport as *mut c_void;

  if reason == STCP_REASON_NEXT_STEP {
    let now = status.to_raw();
    let nxt = status.next_step().unwrap();
    s.set_status(nxt);
    let rust_worker_return: i32;

/*
    >0 = edistyi (state vaihtui / lähetti jotain / purki framia) → C saa schedulettaa heti uudestaan jos haluaa
     0 = ei edistystä (tarvitsee lisää dataa / odottaa herätettä) → C ei schedulea; odottaa data_ready
    <0 = fatal (protokolla rikki tms) → C merkitsee fatal ja alas
*/
    if server {
      stcp_dbg!("SERVER: Changing state from {} to {} ...", now, nxt.to_raw());
      rust_worker_return = rust_exported_data_server_ready_worker(sess_casted, transport_casted);
    } else {
      stcp_dbg!("CLIENT: Changing state from {} to {} ...", now, nxt.to_raw());
      rust_worker_return = rust_exported_data_client_ready_worker(sess_casted, transport_casted);
    }

    return rust_worker_return;
  }

  return 0; // ei edistystä, odottelee uutta herätettä
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_session_handshake_done(sess_void_ptr: *mut c_void) -> c_int {
  let mut rc = 0;
  let sess = unsafe { &mut *(sess_void_ptr as *mut ProtoSession) };
  
  if sess.in_aes_mode() {
    rc = 1;
  }
  stcp_dbg!("Is handshake compelte: {}", rc);
  rc
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_data_client_ready_worker(sess_void_ptr: *mut c_void, transport_void_ptr: *mut c_void) -> c_int {

   /* TODO: Lauttaa featuren taakse ?
    let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
    stcp_dbg!("There are {} alive instances", alive);   
    if alive < 1 {
      return -500;
    }
    */

    if sess_void_ptr.is_null()      { return -EBADF; }
    if transport_void_ptr.is_null() { return -EBADF; }

    // 1) Raaka pointteri sessioon
    let sess = sess_void_ptr as *mut ProtoSession;

    // 2) Eksplisiittinen &mut-viite, EI generiikkaa
    let s: &mut ProtoSession = unsafe { &mut *sess };

    let transport = s.transport as *mut kernel_socket;
    
    stcp_dbg!("Worker Client Setting server false");   
    s.set_is_server(false);

    stcp_dbg!("Worker Client Starting worker");   
    if transport.is_null() {
      stcp_dbg!("Worker Client No transport");   
      return -EBADF;
    }

    let is_server = s.get_is_server();
    let mut status = s.get_status();
    let ret: c_int;

    ret = rust_session_client_handshake_lte(sess, transport);

    status = s.get_status();
    ret
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_data_server_ready_worker(sess_void_ptr: *mut c_void, transport_void_ptr: *mut c_void) -> c_int {
    // Check alive count
/*
    let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
    stcp_dbg!("There are {} alive instances", alive);   
    if alive < 1 {
      return -500;
    }
*/

    if sess_void_ptr.is_null()      { return -EBADF; }
    if transport_void_ptr.is_null() { return -EBADF; }

    // 1) Raaka pointteri sessioon
    let sess = sess_void_ptr as *mut ProtoSession;

    // 2) Eksplisiittinen &mut-viite, EI generiikkaa
    let s: &mut ProtoSession = unsafe { &mut *sess };
    let transport = transport_void_ptr as *mut kernel_socket;
    s.set_is_server(true);

    //let is_server = s.get_is_server();
    let status: HandshakeStatus;

    let ret: c_int;

    ret = rust_session_server_handshake_lte(sess, transport);
    status = s.get_status();
    
    ret
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_session_create(out: *mut *mut c_void, transport: *mut c_void) -> c_int {

  stcp_dbg!("SESSION/Checkpoint 1");
  // Check alive count
/*
  let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
  if alive < 0 {
    stcp_dbg!("Module is not alive!");
    return -500;
  }
*/
  if out.is_null()        { return -EBADF; }
  if transport.is_null()  { return -EBADF; }
  
  stcp_dbg!("SESSION/Checkpoint 2");

  let s = rust_session_create(out, transport as *mut kernel_socket);
 
  stcp_dbg!("SESSION/Checkpoint 3");

  s
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_session_client_handshake(sess_void_ptr: *mut c_void) -> c_int {
  // Check alive count
/*
  let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
  if alive < 1 {
    return -500;
  }
*/

  if sess_void_ptr.is_null() {
    return -EBADF;
  }

  // 1) Raaka pointteri sessioon
  let sess = sess_void_ptr as *mut ProtoSession;

  // 2) Eksplisiittinen &mut-viite, EI generiikkaa
  let s: &mut ProtoSession = unsafe { &mut *sess };

  let transport = s.transport as *mut kernel_socket;
  if transport.is_null() {
    return -EBADF;
  }

  s.set_is_server(false);
  let ret = rust_session_client_handshake_lte(sess, transport);

  ret
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_session_server_handshake(sess_void_ptr: *mut c_void) -> c_int {
    // Check alive count
/*
    let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
    if alive < 1 {
      return -5;
    }
*/

    if sess_void_ptr.is_null() {
      return -EBADF;
    }

    // 1) Raaka pointteri sessioon
    let sess = sess_void_ptr as *mut ProtoSession;

    // 2) Eksplisiittinen &mut-viite, EI generiikkaa
    let s: &mut ProtoSession = unsafe { &mut *sess };

    let transport = s.transport as *mut kernel_socket;

    // tämä on struct sock pointteri
    if transport.is_null() {
        return -EBADF;
    }

    s.set_is_server(true);
    let ret = rust_session_server_handshake_lte(sess, transport);
    ret
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_session_sendmsg(sess_void_ptr: *const c_void,
                                 transport_void_ptr: *const c_void,
                                 buf: *const c_void,
                                 len: usize) -> isize {

    // Check alive count
    //let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
    //if (alive < 1) {
    //  return -5;
    //}

    if sess_void_ptr.is_null() {
      return -EBADF as isize;
    }

    // tämä on struct sock pointteri
    if transport_void_ptr.is_null() {
      return -EBADF as isize;
    }

    // 1) Raaka pointteri sessioon
    let sess = sess_void_ptr as *mut ProtoSession;

    // 2) Eksplisiittinen &mut-viite, EI generiikkaa
    let s: &mut ProtoSession = unsafe { &mut *sess };
  
    let transport = transport_void_ptr as *mut kernel_socket;

    // Connecti ei mee solmuun tällä tavalla ...
    let cast_buffer = buf as *mut u8;

    stcp_dbg!("Before unsafe parts A");   
    let buffer: &[u8] = match stcp_make_mut_slice(cast_buffer, len, len) {
        Ok(buf) => {
          stcp_dbg!("Got buffer ({} bytes): {:?}", buf.len(), buf);
          buf
        },
        Err(e) => {
          match e {
              StcpError::NullPointer => return -EBADF as isize,
              StcpError::LengthTooBig { .. } => return -EINVAL as isize,
              _ => return -EINVAL as isize,
          }              
        }
      };
    stcp_dbg!("After unsafe parts A");   

    stcp_dbg!("SENDING IN RUST =====================================");   
    let aesmode = s.in_aes_mode();
    let ret: i32;

    stcp_dbg!("Sending encrypted: {}, {} bytes", s.in_aes_mode(), buffer.len());
    stcp_dbg!("BUFFER HEX: {:02x?}", buffer);

    if aesmode {
      stcp_dbg!("Sending in AES mode..");
      ret = stcp_message_send_frame(s, transport, StcpMsgType::Aes, buffer);
    } else {
      stcp_dbg!("Sending in Public-key mode..");
      ret = stcp_message_send_frame(s, transport, StcpMsgType::Public, buffer);
    } 
    stcp_dbg!("Got rc: {} at exit.", ret);
    ret as isize
}

unsafe fn internal_iovec_total_len(msg: &zsock_msghdr) -> usize {
    let mut sum = 0;
    for i in 0..msg.msg_iovlen {
        let iov = &*msg.msg_iov.add(i);
        sum += iov.iov_len;
    }
    sum
}


unsafe fn internal_iovec_flatten(msg: &zsock_msghdr, out: &mut [u8]) {
    let mut offset = 0;

    for i in 0..msg.msg_iovlen {
        let iov = &*msg.msg_iov.add(i);

        let slice = core::slice::from_raw_parts(
            iov.iov_base as *const u8,
            iov.iov_len,
        );

        out[offset..offset + iov.iov_len]
            .copy_from_slice(slice);

        offset += iov.iov_len;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_session_sendmsg_iovec(
    sess_void_ptr: *const c_void,
    transport_void_ptr: *const c_void,
    msg_void_ptr: *mut c_void,
    flags: i32,
    encrypted: bool,
) -> isize {

      if sess_void_ptr.is_null() {
      return -EBADF as isize;
    }

    // tämä on struct sock pointteri
    if transport_void_ptr.is_null() {
      return -EBADF as isize;
    }

    // tämä on struct sock pointteri
    if msg_void_ptr.is_null() {
      return -EBADF as isize;
    }

    // 1) Raaka pointteri sessioon
    let sess = sess_void_ptr as *mut ProtoSession;

    // 2) Eksplisiittinen &mut-viite, EI generiikkaa
    let s: &mut ProtoSession = unsafe { &mut *sess };
  
    let transport = transport_void_ptr as *mut kernel_socket;
    let msg = msg_void_ptr as *mut zsock_msghdr;

  if !encrypted {
    
    return unsafe { stcp_tcp_send_iovec(transport as *mut c_void, msg_void_ptr  as *mut c_void, flags) as isize };
  }

  // Salattu versio.. lasketaan vektorin data...
  let msg_ref: &zsock_msghdr = unsafe { &*msg };
  let plaintext_len = unsafe { internal_iovec_total_len(msg_ref) };

  let mut plaintext = Vec::<u8>::new();
  plaintext.resize(plaintext_len, 0);

  // Laitetaan kaikki yhteen puskuriin ...
  unsafe { 
    internal_iovec_flatten(msg_ref, &mut plaintext);
  }

  let aes = s.get_aes();
  let mut encrypted = aes.expect("not cryptable").encrypt(&plaintext);
  // Nyt on kryptattuna ...
  
  // Muodostetaan oma iovec
  let mut header = [0u8; 8];
  let frame_len: u64 = encrypted.len() as u64;

  header.copy_from_slice(&frame_len.to_be_bytes());

  let mut send_iov = [
      zsock_iovec {
          iov_base: header.as_mut_ptr() as *mut _,
          iov_len: 8,
      },
      zsock_iovec {
          iov_base: encrypted.as_mut_ptr() as *mut _,
          iov_len: encrypted.len(),
      },
  ];

  let mut send_msg = zsock_msghdr {
      msg_name: core::ptr::null_mut(),
      msg_namelen: 0,
      msg_iov: send_iov.as_mut_ptr(),
      msg_iovlen: 2,
      msg_control: core::ptr::null_mut(),
      msg_controllen: 0,
      msg_flags: 0,
  };

  // Laitetaan piuhalle!
  let msg_ptr: *mut zsock_msghdr = &mut send_msg;
  let msg_void = msg_ptr as *mut c_void;
  unsafe {
     stcp_tcp_send_iovec(sess_void_ptr as *mut c_void, msg_void, flags) as isize
  }
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_session_recvmsg(
    sess_void_ptr: *mut c_void,
    payload_ptr: *const c_void,
    payload_len: usize,
    out_buf_void_ptr: *mut c_void,
    out_maxlen: usize,
) -> isize {

    use core::cmp::min;

    if sess_void_ptr.is_null()
        || payload_ptr.is_null()
        || out_buf_void_ptr.is_null()
    {
        return -EBADF as isize;
    }

    if payload_len == 0 {
        return 0;
    }

    if out_maxlen == 0 {
        return -EINVAL as isize;
    }

    let sess = unsafe { &mut *(sess_void_ptr as *mut ProtoSession) };

    let payload = unsafe {
        core::slice::from_raw_parts(
            payload_ptr as *const u8,
            payload_len
        )
    };

    let out_slice = match stcp_make_mut_slice(
        out_buf_void_ptr as *mut u8,
        out_maxlen,
        out_maxlen,
    ) {
        Ok(s) => s,
        Err(_) => return -EINVAL as isize,
    };

    stcp_dbg!(
        "STCP recvmsg payload_len={} aes_mode={}",
        payload_len,
        sess.in_aes_mode()
    );

    stcp_dump!("RX payload from C", payload);

    /*
        AES MODE
    */
    if sess.in_aes_mode() {

        let decrypted = match sess
            .aes
            .as_ref()
            .and_then(|aes| aes.decrypt(payload))
        {
            Some(d) => d,
            None => {
                stcp_dbg!("AES decrypt failed");
                return -EPROTO as isize;
            }
        };

        let dec_len = decrypted.len();
        let n = min(dec_len, out_slice.len());

        out_slice[..n].copy_from_slice(&decrypted[..n]);

        stcp_dbg!("AES decrypted {} bytes", n);

        stcp_dump!("AES decrypted payload", &out_slice[..n]);

        return n as isize;
    }

    /*
        PUBLIC MODE
    */
    let n = min(payload_len, out_slice.len());

    out_slice[..n].copy_from_slice(&payload[..n]);

    stcp_dbg!("Plain payload {} bytes", n);

    stcp_dump!("PLAIN payload", &out_slice[..n]);

    n as isize
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_session_destroy(sess_void_ptr: *mut c_void) -> isize {
    // Check alive count
/*
    let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
    stcp_dbg!("There are alive instances");   
    if alive < 1 {
      stcp_dbg!("Should not be alive anymore");   
      return -5;
    }
*/
    if sess_void_ptr.is_null() {
        return -EBADF as isize;
    }

    // 2) Eksplisiittinen &mut-viite, EI generiikkaa
    //let s: &mut ProtoSession = unsafe { &mut *sess };

    rust_session_destroy(sess_void_ptr);

    0
}
