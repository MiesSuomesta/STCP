
//use core::ffi::c_void;

use crate::types::{
  //    ProtoOps,
        ProtoSession,
    };
    
use crate::errorit::*;

//use crate::helpers::{tcp_send_all, tcp_recv_exact};
//use crate::tcp_io;
use alloc::boxed::Box;
//use crate::stcp_handshake::{client_handshake, server_handshake};
//use crate::crypto::Crypto;


use crate::types::{kernel_socket};
use crate::stcp_dbg;

#[unsafe(no_mangle)]
pub extern "C" fn rust_session_create(out_sess: *mut *mut ProtoSession, transport: *mut kernel_socket) -> i32 {

  stcp_dbg!("SESSION/CREATE: Create session starts");   
    
    if out_sess.is_null() {
      stcp_dbg!("No place to put");   
        return -EBADF;
    }

    let sess = ProtoSession::new(false, transport);
    let boxed = Box::new(sess);
    let raw = Box::into_raw(boxed);

    stcp_dbg!("SESSION/CREATE: setting new session into");   
    unsafe { *out_sess = raw; }
    stcp_dbg!("SESSION/CREATE: OK");   
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_session_destroy(sess: *mut ProtoSession) -> i32 {

    if sess.is_null() {
        return -EBADF;
    }
    
    unsafe { drop(Box::from_raw(sess)); }
    0
}

