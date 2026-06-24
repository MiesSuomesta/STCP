
use core::ffi::c_void;
use crate::errorit::*;
//use core::panic::Location;
use alloc::boxed::Box;
use crate::proto_session::ProtoSession;
use crate::proto_session::SESSION_MAGIC;
use crate::kernel_socket;
use crate::stcp_dbg;



#[unsafe(no_mangle)]
pub extern "C" fn rust_session_create(
    out_sess: *mut *mut c_void,
    transport_void_ptr: *mut core::ffi::c_void,
    bypass_enabled: core::ffi::c_int,
) -> i32 {

    stcp_dbg!("SESSION/CREATE start");

    if out_sess.is_null() {
        stcp_dbg!("SESSION/CREATE No session");
        return -EBADF;
    }

    let transport = transport_void_ptr as *const kernel_socket;
    let transport_copy = unsafe { (*transport).clone() };
    
    let boxed = Box::new(transport_copy);
    let transport_vp = Box::into_raw(boxed) as *mut c_void;

    stcp_dbg!("SESSION/CREATE session create new..");
    let mut sess = Box::new(ProtoSession::new(transport_vp));
    stcp_dbg!("SESSION/CREATE session done..");

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
    let s = sess as *mut ProtoSession;
    stcp_dbg!("RUST Session destroy...");

    unsafe {
        if s.is_null() {
            stcp_dbg!("RUST Session destroy return -1");
            return -1;
        }

        stcp_dbg!("RUST Session destroy called with: {:?}", s);
        drop(Box::from_raw(s));
        stcp_dbg!("RUST Session destroy done");
    }

    stcp_dbg!("RUST Session destroy return");
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
