// ============================================================================
// Zephyr moodi
// ============================================================================
extern crate alloc;
use alloc::vec::Vec;
use core::str::FromStr;
use core::net::Ipv4Addr;

use crate::stream::StcpStream;
use core::ffi::c_int;
use zephyrsys::net::*;

use utils;

pub struct Incoming<'a> {
        listener: &'a StcpListener
}

impl<'a> Iterator for Incoming<'a> {
        type Item = Result<StcpStream, ()>;
        fn next(&mut self) -> Option<Self::Item> {
                Some(self.listener.accept())
        }
}

pub struct StcpListener {
    fd: c_int,
}

impl StcpListener {

    pub fn bind(addr: &str) -> Result<Self, ()> {
        let fd = unsafe { stcp_modem_socket (
			AF_INET as i32,
			SOCK_STREAM as i32,
			IPPROTO_TCP as i32,
		) };

	if fd < 0 {
		return Err(());
	}

	let res = utils::create_ipaddr_and_port_from_host(addr);

	let (ipaddr, port) = match res {
			Some((ip, port)) => (ip, port),
			None => return Err(()),
		};

	let port_bigendian = port.to_be();
	let ip_bytes = ipaddr.octets();

	let saddr = sockaddr_in {
			sin_family: AF_INET as u16,
			sin_port: port_bigendian,
			sin_addr: zephyrsys::net::in_addr {
					s_addr: u32::from_le_bytes(ip_bytes),
				},
			sin_zero:[0; 8],
		};

	let rv = unsafe {
		stcp_modem_bind(
			fd,
			&saddr as *const sockaddr_in as *const sockaddr,
			core::mem::size_of::<sockaddr_in>() as u32,

		);
	};

	if rv == () {
		return Err(());
	}

	Ok( Self {fd} )

   }

    pub fn accept(&self) -> Result<StcpStream, ()> {

	let mut addr = core::mem::MaybeUninit::<sockaddr_in>::uninit();
	let mut addrlen = core::mem::size_of::<sockaddr_in>() as u32;

	let client = unsafe {
		stcp_modem_accept(
			self.fd,
			(addr.as_mut_ptr() as *mut sockaddr),
			&mut addrlen,
		)
	};

	if client < 0 {
		return Err(());
	}

	Ok( StcpStream { fd: client } )
    }

    pub fn incoming(&self) -> Incoming {
        Incoming { listener: self }
    }
}
