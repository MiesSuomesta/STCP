
use crate::self_testing::stcp_do_selftests;
use crate::stcp_dbg;
use crate::stcp_dbg_big;
use crate::stcp_info;
use crate::stcp_info_big;
use crate::stcp_err_big;

#[unsafe(no_mangle)]
pub extern "C" fn stcp_module_rust_enter () {
/* logituksen tarkistukseen...
  let msg = b"HELLO FROM RUST\n";

  unsafe {
      crate::abi::stcp_rust_log(
          1,
          msg.as_ptr(),
          msg.len(),
      );
  }
  */
  stcp_dbg!("STCP rust module enter");   
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_module_rust_exit () {
  stcp_dbg!("STCP rust module exit");   
}
