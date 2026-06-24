extern crate alloc;
use core::ffi::c_int;
use core::result::Result;
use core::net::*;
use core::mem;
use zephyrsys::net::*;
use alloc::vec::Vec;
use alloc::format;


use zephyrsys::net as zn;

use debug::zprint;
use debug::dbg;

use utils::htons;

pub struct StcpStream {
	pub fd: c_int,
}

impl StcpStream {

    fn get_ip_info(addr: SocketAddr) -> Result<(Ipv4Addr, u16), ()> {

        match addr {
                SocketAddr::V4(sav4) => {
                        let ip  = sav4.ip().clone();
                        let port = sav4.port();
                        Ok((ip, port))
                }

                SocketAddr::V6(sav6) => {
                        Err(())
                }
        }
    }

    pub fn connect(addr: SocketAddr) -> Result<Self, ()> {
        unsafe {
            
                let myfd = zn::stcp_modem_socket(zn::AF_INET, zn::SOCK_STREAM, zn::IPPROTO_TCP);
                if myfd < 0 {
                        return Err(());
                }
                
                let results = Self::get_ip_info(addr);

                let (ip, port) = match results {
                        Ok(v) => v,
                        Err(e) => {
                                dbg!("Unable to connect: {:?}", e);
                                return Err(e);
                        },
                };


                let ip_bytes = ip.octets();

                let mut raw_addr: sockaddr_in = mem::zeroed();
                        raw_addr.sin_family = AF_INET as u16;
                        raw_addr.sin_port = htons(port);
                        raw_addr.sin_addr = in_addr {
                                s_addr: u32::from_ne_bytes(ip_bytes),
                        };
        
                let res = zn::stcp_modem_connect(
                                myfd,
                                &raw_addr as *const sockaddr_in as *const sockaddr,
                                mem::size_of::<sockaddr_in>() as u32,
                        );
                    
                if res < 0 {
                        return Err(());
                }
            
            Ok(Self{ fd: myfd })
        } // Unsafe loppuu
    }

    pub fn write(&mut self, buf: &[u8]) -> Result<isize, ()> {
	let ret = unsafe {
		zn::stcp_modem_send(
			self.fd,
			buf.as_ptr() as *const _,
			buf.len(),
			0,
		)
	};

	if ret < 0 {
		return Err(());
	}

	Ok(ret)
    }

    pub fn write_all(&mut self, buf: &[u8]) -> Result<isize, ()> {
	let ret = unsafe {
		zn::stcp_modem_send(
			self.fd,
			buf.as_ptr() as *const _,
			buf.len(),
			0,
		)
	};

	if ret < 0 {
		return Err(());
	}

	Ok(ret)
    }

    pub fn read(&mut self, buf: &mut [u8]) -> Result<isize, ()> {

	let ret = unsafe {
		zn::stcp_modem_recv(
			self.fd,
			buf.as_mut_ptr() as *mut _,
			buf.len(),
			0,
		)
	};

	if ret < 0 {
		return Err(());
	}

	Ok(ret)
    }

    pub fn close(&mut self) -> Result<isize, ()> {

	let ret = unsafe {
		zn::stcp_modem_close(self.fd)
	};

	if ret < 0 {
		return Err(());
	}

	Ok(0)
    }

}

