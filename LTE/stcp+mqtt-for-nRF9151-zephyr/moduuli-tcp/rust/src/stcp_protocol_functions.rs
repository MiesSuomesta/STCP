
use core::ffi::c_void;
use core::ffi::c_int;
use crate::{stcp_dbg /* , stcp_dump, stcp_sess_transp */ };

use crate::slice_helpers::{stcp_make_mut_slice, StcpError};
use crate::session_handler::{rust_session_create, rust_session_destroy};
use crate::stcp_handshake::{
    rust_session_handshake_lte,
};
use crate::stcp_message::*;
use alloc::vec::Vec;
use crate::stcp_dump;

//use alloc::boxed::Box;

use crate::types::{
  //    ProtoOps,
        StcpMsgType,
        zsock_iovec,
        zsock_msghdr,
    };

use crate::errorit::*;
use crate::proto_session::ProtoSession;

use crate::tcp_io::stcp_tcp_send_iovec;

//use crate::helpers::{tcp_recv_once, tcp_send_all, get_session};
//use crate::abi::{stcp_end_of_life_for_sk};

// TCP helpperi makrot
//use crate::stcp_tcp_recv_once;
//use crate::stcp_tcp_send_all;
//use crate::stcp_tcp_recv_exact;
//use crate::stcp_tcp_peek_max;
//use crate::stcp_tcp_recv_until_buffer_full;

pub const STCP_REASON_NEXT_STEP : i32 = 3;

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
pub extern "C" fn rust_exported_session_create(out: *mut *mut c_void, transport: *mut c_void, bypass: c_int) -> c_int {

  stcp_dbg!("Session creation starts, aes bypassed: {}", bypass != 0);
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

  let s = rust_session_create(out, transport as *mut core::ffi::c_void, bypass);
 
  stcp_dbg!("SESSION/Checkpoint 3");

  s
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
  
    let transport = transport_void_ptr as *mut core::ffi::c_void;


    stcp_dbg!(
        "SEND sess={:?} fd={} transport={:?}",
        sess_void_ptr,
        s.get_transport_fd(),
        transport
    );

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
  
  if msg.msg_iov.is_null() {
      return 0;
  }

  let base = msg.msg_iov;

  for i in 0..msg.msg_iovlen as usize {
      let iov = unsafe { &*base.add(i) };
      sum += iov.iov_len;
  }

  sum
}

unsafe fn internal_iovec_flatten(msg: &zsock_msghdr, out: &mut [u8]) {
    if msg.msg_iov.is_null() {
        return;
    }

    let mut offset = 0;

    let iovecs = unsafe {
        core::slice::from_raw_parts(
            msg.msg_iov,
            msg.msg_iovlen,
        )
    };

    for iov in iovecs {
        let slice = unsafe {
            core::slice::from_raw_parts(
                iov.iov_base as *const u8,
                iov.iov_len,
            )
        };

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

    let _ = flags;

    if sess_void_ptr.is_null() {
        stcp_dbg!("sendmsg_iovec: session NULL");
        return -1;
    }

    if msg_void_ptr.is_null() {
        stcp_dbg!("sendmsg_iovec: msg NULL");
        return -1;
    }

    let sess = unsafe {
        &mut *(sess_void_ptr as *mut ProtoSession)
    };

    let transport =
        transport_void_ptr as *mut core::ffi::c_void;

    let msg = unsafe {
        &*(msg_void_ptr as *const zsock_msghdr)
    };

    let s: &mut ProtoSession = unsafe { &mut *sess };
    stcp_dbg!(
        "SEND VEC sess={:?} fd={} transport={:?}",
        sess_void_ptr,
        s.get_transport_fd(),
        transport
    );


    if msg.msg_iov.is_null() || msg.msg_iovlen == 0 {
        stcp_dbg!("sendmsg_iovec: invalid iov");
        return -1;
    }

    stcp_dbg!(
        "sendmsg_iovec: sess={:?} iovlen={} encrypted={}",
        sess_void_ptr,
        msg.msg_iovlen,
        encrypted
    );

    //
    // Flatten iovec -> plaintext
    //
    let mut plaintext: Vec<u8> = Vec::new();

    for idx in 0..msg.msg_iovlen {

        let iov_ref = unsafe {
            &*msg.msg_iov.add(idx)
        };

        if iov_ref.iov_base.is_null() || iov_ref.iov_len == 0 {
            continue;
        }

        let slice = unsafe {
            core::slice::from_raw_parts(
                iov_ref.iov_base as *const u8,
                iov_ref.iov_len
            )
        };

        plaintext.extend_from_slice(slice);
    }

    stcp_dbg!(
        "sendmsg_iovec plaintext len={}",
        plaintext.len()
    );

    if plaintext.is_empty() {
        stcp_dbg!("sendmsg_iovec: plaintext empty");
        return -1;
    }

    //
    // SAME framing path as ProtoSession::send_message()
    //
    let msg_type = if encrypted {
        StcpMsgType::Aes
    } else {
        StcpMsgType::Public
    };

    let rc = stcp_message_send_frame(
        sess,
        transport,
        msg_type,
        &plaintext
    );

    stcp_dbg!(
        "sendmsg_iovec send rc={}",
        rc
    );

    rc as isize
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
        stcp_dbg!("STCP recvmsg error: Null seen as params...");
        return -EBADF as isize;
    }

    if payload_len == 0 {
        stcp_dbg!("STCP recvmsg error: no payload len");
        return 0;
    }

    if out_maxlen == 0 {
      stcp_dbg!("STCP recvmsg error: no max output len");
      return -ENOSPC as isize;
    }

    let sess = unsafe { &mut *(sess_void_ptr as *mut ProtoSession) };
    let s: &mut ProtoSession = unsafe { &mut *sess };
    stcp_dbg!(
        "RECV VEC sess={:?} fd={} transport={:?}",
        sess_void_ptr,
        s.get_transport_fd(),
        sess.transport
    );

    stcp_dbg!(
        "STCP recvmsg payload_len={}",
        payload_len
    );

    let payload = unsafe {
        core::slice::from_raw_parts(
            payload_ptr as *const u8,
            payload_len
        )
    };

    stcp_dbg!(
        "STCP recvmsg out_buf={:?} out_maxlen={}",
        out_buf_void_ptr,
        out_maxlen
    );

    let out_slice = match stcp_make_mut_slice(
        out_buf_void_ptr as *mut u8,
        out_maxlen,
        out_maxlen,
    ) {
        Ok(s) => s,
        Err(e) => { 
          return -EINVAL as isize
        },
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
