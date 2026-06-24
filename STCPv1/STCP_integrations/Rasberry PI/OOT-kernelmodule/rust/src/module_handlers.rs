
use crate::stcp_dbg;

#[unsafe(no_mangle)]
pub extern "C" fn stcp_module_rust_enter () {
  stcp_dbg!("STCP rust module enter");   
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_module_rust_exit () {
  stcp_dbg!("STCP rust module exit");   
}
