//! Linux STCP-TCP echo server. Standard library and Linux libc ABI only.
use std::{env, io::{self, Read, Write}, net::TcpStream, os::fd::{FromRawFd, OwnedFd, AsRawFd}};

#[repr(C)]
struct Address { family: u16, port: u16, ip: [u8; 4], padding: [u8; 8] }
extern "C" {
    fn socket(domain: i32, kind: i32, protocol: i32) -> i32;
    fn bind(fd: i32, address: *const Address, length: u32) -> i32;
    fn listen(fd: i32, backlog: i32) -> i32;
    fn accept(fd: i32, address: *mut std::ffi::c_void, length: *mut u32) -> i32;
}
fn check(result: i32) -> io::Result<i32> {
    if result < 0 { Err(io::Error::last_os_error()) } else { Ok(result) }
}
fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<_> = env::args().collect();
    if args.len() > 2 { return Err("usage: stcp-free-echo [port]".into()); }
    let port: u16 = args.get(1).map(String::as_str).unwrap_or("7000").parse()?;
    if port == 0 { return Err("port must be 1..65535".into()); }
    let fd = unsafe { OwnedFd::from_raw_fd(check(socket(45, 1, 253))?) };
    let address = Address { family: 2, port: port.to_be(), ip: [0; 4], padding: [0; 8] };
    unsafe {
        check(bind(fd.as_raw_fd(), &address, std::mem::size_of::<Address>() as u32))?;
        check(listen(fd.as_raw_fd(), 16))?;
    }
    println!("STCP-TCP echo listening on 0.0.0.0:{port}");
    let mut buffer = [0u8; 65536];
    loop {
        // No peer-address lookup: a disconnect during accept may invalidate it.
        let child = match check(unsafe { accept(fd.as_raw_fd(), std::ptr::null_mut(), std::ptr::null_mut()) }) {
            Ok(value) => value,
            Err(e) if matches!(e.raw_os_error(), Some(4 | 32 | 103 | 104 | 107 | 110 | 111)) => continue,
            Err(e) => return Err(e.into()),
        };
        let mut client = TcpStream::from(unsafe { OwnedFd::from_raw_fd(child) });
        let result = (|| -> io::Result<()> {
            loop {
                let count = match client.read(&mut buffer) {
                    Err(e) if e.kind() == io::ErrorKind::Interrupted => continue,
                    result => result?,
                };
                if count == 0 { return Ok(()); }
                client.write_all(&buffer[..count])?;
            }
        })();
        if let Err(e) = result { eprintln!("client: {e}"); }
    }
}
