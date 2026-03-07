
use crate::stcp_dbg;
use core::panic::Location;

#[unsafe(no_mangle)]
pub extern "C" fn stcp_module_rust_enter () {

  let msg = b"HELLO FROM RUST\n";

  unsafe {
      crate::abi::stcp_rust_log(
          1,
          msg.as_ptr(),
          msg.len(),
      );
  }
  
  stcp_dbg!("STCP rust module enter");   
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_module_rust_exit () {
  stcp_dbg!("STCP rust module exit");   
}
