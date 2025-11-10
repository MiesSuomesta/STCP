 // hello_rust.rs
#![no_std]
#![no_main]

use kernel::prelude::*;
use kernel::ThisModule;

module! {
    type: Stcp,
    name: "stcp",
    authors: [ "Lauri Jakku <lauri.jakku@paxsudos.fi>" ],
    description: "SecureTCP by Paxsudos IT: Enables Secure TCP connections.",
    license: "GPL",
}

struct Stcp;

extern "C" {
    fn stcp_proto_register() -> core::ffi::c_int;
    fn stcp_proto_unregister();
}

impl kernel::Module for Stcp {
    fn init(_module: &'static ThisModule) -> Result<Stcp> {
        pr_info!("STCP: Registering protocol....!\n");
        let rc = unsafe { stcp_proto_register() };
        pr_info!("STCP: the RC from registering: %d!\n", rc);
        if rc != 0 {
            pr_err!("stcp: proto_register failed: {}\n", rc);
            return Err(Error::from_errno(rc));
        }
        pr_info!("stcp: registered IPPROTO_STCP=253\n");
        Ok(Stcp)
    }
}

impl Drop for Stcp {
    fn drop(&mut self) {
        pr_info!("STCP: Unloading..\n");
        unsafe { stcp_proto_unregister() };
        pr_info!("stcp: unregistered IPPROTO_STCP\n");
    }
}
