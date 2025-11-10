
extern crate alloc;
use alloc::vec::Vec;
use core::str::FromStr;
use core::net::Ipv4Addr;

pub fn create_ipaddr_and_port_from_host(addr: &str) -> Option<(Ipv4Addr, u16)> {

    let parts: Vec<&str> = addr.split(":").collect();

    if parts.len() != 2 {
         return None;
    }

    let port = parts[1].parse::<u16>().ok()?;

    let ipaddress = Ipv4Addr::from_str(parts[0]).ok()?;
    Some((ipaddress, port))
}

pub fn htons(value: u16) -> u16 {
	value.to_be()
}

