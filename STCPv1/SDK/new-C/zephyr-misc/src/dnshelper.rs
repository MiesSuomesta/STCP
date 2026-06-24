extern crate alloc;
use alloc::ffi::CString;

use core::net::Ipv4Addr;

use utils;

extern "C" {
    fn resolve_domain(domain: *const u8, out_ip: *mut u8, max_len: usize) -> i32;
}

pub fn resolve_domain_rust(domain: &str) -> Option<(Ipv4Addr, u16)> {
    let mut buf = [0u8; 64];
    let cstr = CString::new(domain).ok()?;
    let res = unsafe {
        resolve_domain(cstr.as_ptr(), buf.as_mut_ptr(), buf.len())
    };
    if res != 0 {
        return None;
    }

    let ip_str = core::str::from_utf8(&buf).ok()?.trim_end_matches(char::from(0));
    let parsedInfo = utils::create_ipaddr_and_port_from_host(ip_str);

    let (ipaddr, port) = match parsedInfo {
                        Some((ip, port)) => (ip, port),
                        None => return None,
                };

    Some( (ipaddr, port) )
}
