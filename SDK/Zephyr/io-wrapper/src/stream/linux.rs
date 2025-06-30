extern crate alloc;
use core::result::Result;
use alloc::vec::Vec;
use std::net::{SocketAddr, IpAddr, Ipv4Addr};
use std::os::unix::io::RawFd;
use std::mem;
use libc::*;


pub struct StcpStream {
    pub fd: libc::c_int,
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
            
		let myfd = libc::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
	
		let res = connect(
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

    pub fn close(&mut self) {
      unsafe { libc::close(self.fd); }
    }

    pub fn write(&mut self, buf: &[u8]) -> Result<usize, ()> {
        let ret = unsafe {
            libc::send(
                self.fd,
                buf.as_ptr() as *const _,
                buf.len(),
                0,
            )
        };
        if ret < 0 {
            return Err(());
        }
        Ok(ret as usize)
    }

    pub fn write_all(&mut self, buf: &[u8]) -> Result<usize, ()> {
        let ret = unsafe {
            libc::send(
                self.fd,
                buf.as_ptr() as *const _,
                buf.len(),
                0,
            )
        };
        if ret < 0 {
            return Err(());
        }
        Ok(ret as usize)
    }

    pub fn read(&mut self, buf: &mut [u8]) -> Result<usize, ()> {
        let ret = unsafe {
            libc::recv(
                self.fd,
                buf.as_mut_ptr() as *mut _,
                buf.len(),
                0,
            )
        };
        if ret < 0 {
            return Err(());
        }
        Ok(ret as usize)
    }
}
