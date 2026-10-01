//! Minimal Linux userspace wrapper for the public STCP socket ABI.
//!
//! This crate intentionally has no dependency on the private/full STCP SDK.

use std::{io, mem, net::{Ipv4Addr, SocketAddrV4}};

pub const AF_STCP: libc::c_int = 45;
pub const STCP_TCP: libc::c_int = 253;
pub const STCP_UDP: libc::c_int = 254;

pub struct StcpStream {
    fd: libc::c_int,
}

impl StcpStream {
    pub fn connect(addr: SocketAddrV4) -> io::Result<Self> {
        let fd = socket()?;
        let sa = sockaddr(addr);
        if unsafe {
            libc::connect(
                fd,
                &sa as *const libc::sockaddr_in as *const libc::sockaddr,
                mem::size_of::<libc::sockaddr_in>() as libc::socklen_t,
            )
        } < 0 {
            let e = io::Error::last_os_error();
            unsafe { libc::close(fd); }
            return Err(e);
        }
        Ok(Self { fd })
    }

    pub fn send_all(&mut self, mut data: &[u8]) -> io::Result<()> {
        while !data.is_empty() {
            let n = unsafe { libc::send(self.fd, data.as_ptr().cast(), data.len(), 0) };
            if n < 0 { return Err(io::Error::last_os_error()); }
            if n == 0 { return Err(io::Error::new(io::ErrorKind::WriteZero, "STCP send returned zero")); }
            data = &data[n as usize..];
        }
        Ok(())
    }

    pub fn recv(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        let n = unsafe { libc::recv(self.fd, buf.as_mut_ptr().cast(), buf.len(), 0) };
        if n < 0 { Err(io::Error::last_os_error()) } else { Ok(n as usize) }
    }
}

impl Drop for StcpStream {
    fn drop(&mut self) { unsafe { libc::close(self.fd); } }
}

pub struct StcpListener {
    fd: libc::c_int,
}

impl StcpListener {
    pub fn bind(addr: SocketAddrV4) -> io::Result<Self> {
        let fd = socket()?;
        let sa = sockaddr(addr);
        if unsafe {
            libc::bind(
                fd,
                &sa as *const libc::sockaddr_in as *const libc::sockaddr,
                mem::size_of::<libc::sockaddr_in>() as libc::socklen_t,
            )
        } < 0 {
            let e = io::Error::last_os_error();
            unsafe { libc::close(fd); }
            return Err(e);
        }
        if unsafe { libc::listen(fd, 32) } < 0 {
            let e = io::Error::last_os_error();
            unsafe { libc::close(fd); }
            return Err(e);
        }
        Ok(Self { fd })
    }

    pub fn accept(&self) -> io::Result<StcpStream> {
        let fd = unsafe { libc::accept(self.fd, std::ptr::null_mut(), std::ptr::null_mut()) };
        if fd < 0 { Err(io::Error::last_os_error()) } else { Ok(StcpStream { fd }) }
    }
}

impl Drop for StcpListener {
    fn drop(&mut self) { unsafe { libc::close(self.fd); } }
}

pub fn parse_ipv4(s: &str) -> io::Result<SocketAddrV4> {
    s.parse().map_err(|e| io::Error::new(io::ErrorKind::InvalidInput, format!("bad IPv4 endpoint {s}: {e}")))
}

fn socket() -> io::Result<libc::c_int> {
    let fd = unsafe { libc::socket(AF_STCP, libc::SOCK_STREAM, STCP_TCP) };
    if fd < 0 { Err(io::Error::last_os_error()) } else { Ok(fd) }
}

fn sockaddr(addr: SocketAddrV4) -> libc::sockaddr_in {
    libc::sockaddr_in {
        sin_family: libc::AF_INET as libc::sa_family_t,
        sin_port: addr.port().to_be(),
        sin_addr: libc::in_addr { s_addr: u32::from_ne_bytes(addr.ip().octets()) },
        sin_zero: [0; 8],
    }
}

pub fn any(port: u16) -> SocketAddrV4 { SocketAddrV4::new(Ipv4Addr::UNSPECIFIED, port) }
