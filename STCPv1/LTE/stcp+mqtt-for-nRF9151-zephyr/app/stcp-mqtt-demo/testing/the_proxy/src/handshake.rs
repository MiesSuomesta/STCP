use std::ffi::c_void;

use crate::debug::stcp_uptime_ms;
use crate::stcp_dbg;

use the_stcp_kernel_module::{
      proto_session::ProtoSession,
      stcp_handshake::rust_session_handshake_lte,
};

pub fn run_handshake(
    mut session: &mut ProtoSession,
    transport: *mut c_void
) -> Result<(), i32> {

    stcp_dbg!(
        "Running handshake session={:?} transport={:?}",
        session as *mut _ as *mut c_void,
        transport
    );
    let mut i: i32 = 0;
    loop {

        i += 1;
        stcp_dbg!(
            "Running handshake ({:?} time) session={:?} transport={:?}",
            i,
            session as *mut _ as *mut c_void,
            transport
        );
        
        let rc = unsafe { 
                rust_session_handshake_lte(
                                    session as *mut _ as *mut c_void,
                                    transport
                                )
            };    

        stcp_dbg!(
            "Ran handshake session={:?} transport={:?} => {:?}",
            session as *mut _ as *mut c_void,
            transport,
            rc
        );

        if rc == 1 {
            return Ok(());
        }

        if rc < 0 {
            return Err((rc));
        }

        std::thread::sleep(
            std::time::Duration::from_millis(50)
        );
    }
}

