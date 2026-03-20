
use core::ffi::c_void;
use crate::errorit::*;
//use core::panic::Location;
use alloc::boxed::Box;
use crate::proto_session::ProtoSession;
use crate::proto_session::SESSION_MAGIC;

use crate::stcp_dbg;



#[unsafe(no_mangle)]
pub extern "C" fn rust_session_create(
    out_sess: *mut *mut c_void,
    transport: *mut core::ffi::c_void,
) -> i32 {

    stcp_dbg!("SESSION/CREATE start");

    if out_sess.is_null() {
        return -EBADF;
    }

    let sess = Box::new(ProtoSession::new(false, transport as *mut c_void));

    unsafe {
        *out_sess = Box::into_raw(sess) as *mut c_void;
    }

    stcp_dbg!("SESSION/CREATE OK");
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_session_is_valid(sess: *mut c_void) -> i32 {

    if sess.is_null() {
        stcp_dbg!("No session!");
        return 0;
    }
    
    let s = unsafe { &mut *(sess as *mut ProtoSession) };
    let rv = s.magic == SESSION_MAGIC;
    stcp_dbg!("Is RUST session valid: {}", rv);
    
    if rv {
        return 1;
    }

    return 0;
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_session_destroy(sess: *mut c_void) -> i32 {

    stcp_dbg!("RUST Session destroy called with: {:?}", sess);
    unsafe { drop(Box::from_raw(sess as *mut ProtoSession)); }
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_session_reset_everything_now(sess: *mut c_void) -> i32 {

    if sess.is_null() {
        return -EBADF;
    }
    
    let s = unsafe { &mut *(sess as *mut ProtoSession) };

    stcp_dbg!("======================= SESSION {:?} RESET ==============================", sess);   
    let rv: i32 = s.reset_everything_now();
    stcp_dbg!("=================== SESSION RESET COMPLETE =========================");   

    rv
}
