
use core::ffi::c_void;
use core::ffi::c_int;
use crate::{stcp_dbg, stcp_worker_dbg /* , stcp_dump, stcp_sess_transp */ };

use crate::slice_helpers::{stcp_make_mut_slice, StcpError};
use crate::session_handler::{rust_session_create, rust_session_destroy};
use crate::stcp_handshake::{
    rust_session_server_handshake,
    rust_session_client_handshake,
    rust_session_handshake_done,
};

use crate::stcp_message::*;

//use alloc::boxed::Box;

use crate::types::{
  //    ProtoOps,
        ProtoSession,
        kernel_socket,
        HandshakeStatus,
        StcpMsgType,
        StcpMessageHeader,
        STCP_TAG_BYTES,
    };

use crate::errorit::*;


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

    if sess_void_ptr.is_null() {
        stcp_dbg!("No session");   
        return -EBADF;
    }

    let sess = sess_void_ptr as *mut ProtoSession;

    rust_session_handshake_done(sess)
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_data_client_ready_worker(sess_void_ptr: *mut c_void, transport_void_ptr: *mut c_void) -> c_int {
    stcp_worker_dbg!(sess_void_ptr, transport_void_ptr, "woken up by data ready.");

    let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
    stcp_dbg!("There are {} alive instances", alive);   
    if alive < 1 {
      return -500;
    }

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
    stcp_worker_dbg!(sess, transport, "Is server: {}, Status: {:?}", is_server, status);

    stcp_worker_dbg!(sess, transport, "Client handshake, before status: {:?}", status as i32);

    ret = rust_session_client_handshake(sess, transport);
    status = s.get_status();
    stcp_worker_dbg!(sess, transport, "Client handshake,ret : {:?} // {:?}", ret, status);
    ret
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_data_server_ready_worker(sess_void_ptr: *mut c_void, transport_void_ptr: *mut c_void) -> c_int {
    // Check alive count
    let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
    stcp_dbg!("There are {} alive instances", alive);   
    if alive < 1 {
      return -500;
    }

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

    ret = rust_session_server_handshake(sess, transport);
    status = s.get_status();
    stcp_worker_dbg!(sess, transport, "Server handshake,ret : {:?} // {:?}", ret, status);
    
    ret
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_session_create(out: *mut *mut c_void, transport: *mut c_void) -> c_int {

  stcp_dbg!("SESSION/Checkpoint 1");
  // Check alive count
  let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
  if alive < 0 {
    stcp_dbg!("Module is not alive!");
    return -500;
  }

  if out.is_null()        { return -EBADF; }
  if transport.is_null()  { return -EBADF; }

  let sess_ptr = out as *mut *mut ProtoSession;
  // allokoi sessio ..jos ei onnistu s != 0
  stcp_dbg!("SESSION/Checkpoint 2");
  let s = rust_session_create(sess_ptr, transport as *mut kernel_socket);
  stcp_dbg!("SESSION/Checkpoint 3");

  s
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_session_client_handshake(sess_void_ptr: *mut c_void) -> c_int {
  // Check alive count
  let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
  if alive < 1 {
    return -500;
  }

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
  let ret = rust_session_client_handshake(sess, transport);

  ret
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_session_server_handshake(sess_void_ptr: *mut c_void) -> c_int {
    // Check alive count
    let alive = unsafe { stcp_exported_rust_ctx_alive_count() };
    if alive < 1 {
      return -5;
    }

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
    let ret = rust_session_server_handshake(sess, transport);
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
    let buffer: &mut [u8] = match stcp_make_mut_slice(cast_buffer, len, len) {
        Ok(buf) => buf,
        Err(e) => {
          match e {
              StcpError::NullPointer => return -EBADF as isize,
              StcpError::LengthTooBig { .. } => return -EINVAL as isize,
          }              
        }
      };
    stcp_dbg!("After unsafe parts A");   

    let aesmode = s.in_aes_mode();

    if aesmode {
      stcp_dbg!("In AES mode..");
      stcp_message_send_frame(sess, transport, StcpMsgType::Aes, buffer) as isize
    } else {
      stcp_dbg!("In Public-key mode..");
      stcp_message_send_frame(sess, transport, StcpMsgType::Public, buffer) as isize
    } 
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_exported_session_recvmsg(
    sess_void_ptr: *mut c_void,
    transport_void_ptr: *mut c_void,
    out_buf_void_ptr: *mut c_void,
    out_maxlen: usize,
    non_blocking: i32,
) -> isize {
    if sess_void_ptr.is_null() || transport_void_ptr.is_null() || out_buf_void_ptr.is_null() {
        return -EBADF as isize;
    }

    let sock = transport_void_ptr as *mut kernel_socket;
    let out = out_buf_void_ptr as *mut u8;
    let out_slice = match stcp_make_mut_slice(out, out_maxlen, out_maxlen) {
        Ok(s) => s,
        Err(_) => return -EINVAL as isize,
    };

    let hdr_sz = stcp_message_get_header_size_in_bytes();
    let mut hdr_buf = [0u8; 32]; // varmista >= header size
    if hdr_sz > hdr_buf.len() { return -EINVAL as isize; }

    let mut got: c_int = 0;

    // 1) read header
    let r1 = unsafe { stcp_tcp_recv(sock, hdr_buf.as_mut_ptr(), hdr_sz, non_blocking, 0, &mut got) };
    if r1 < 0 { return r1; }
    if got as usize != hdr_sz { return -EAGAIN as isize; } // tai -EMSGSIZE / partial handling

    let hdr = StcpMessageHeader::from_bytes_be(&hdr_buf[..hdr_sz]);
    if hdr.tag != STCP_TAG_BYTES { return -EPROTO as isize; }

    let pay_len = hdr.msg_len as usize;

    // 2) sanity
    if pay_len > (1024*1024) { return -EMSGSIZE as isize; } // järkevä yläraja

    // 3) read payload
    let mut payload = alloc::vec![0u8; pay_len];
    got = 0;
    let r2 = unsafe { stcp_tcp_recv(sock, payload.as_mut_ptr(), pay_len, non_blocking, 0, &mut got) };
    if r2 < 0 { return r2; }
    if got as usize != pay_len { return -EAGAIN as isize; }

    // 4) copy to user out buffer (truncate or error)
    let n = core::cmp::min(pay_len, out_slice.len());
    out_slice[..n].copy_from_slice(&payload[..n]);
    return n as isize;
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

    // 1) Raaka pointteri sessioon
    let sess = sess_void_ptr as *mut ProtoSession;

    // 2) Eksplisiittinen &mut-viite, EI generiikkaa
    //let s: &mut ProtoSession = unsafe { &mut *sess };

    rust_session_destroy(sess);

    0
}
