//! Linux STCP-TCP echo client; standard library only.
use std::{env, io::{self, Read, Write}, net::{Ipv4Addr, TcpStream}, os::fd::{AsRawFd, FromRawFd, OwnedFd}};
#[repr(C)]
struct Address { family: u16, port: u16, ip: [u8; 4], padding: [u8; 8] }
extern "C" {
    fn socket(domain: i32, kind: i32, protocol: i32) -> i32;
    fn connect(fd: i32, address: *const Address, length: u32) -> i32;
}
fn check(result: i32) -> io::Result<i32> {
    if result < 0 { Err(io::Error::last_os_error()) } else { Ok(result) }
}
fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<_> = env::args().collect();
    if !(2..=4).contains(&args.len()) { return Err("usage: echo-client IPv4 [port] [message]".into()); }
    let ip: Ipv4Addr = args[1].parse()?;
    let port: u16 = args.get(2).map(String::as_str).unwrap_or("7000").parse()?;
    if port == 0 { return Err("port must be 1..65535".into()); }
    let payload = args.get(3).map(String::as_str).unwrap_or("Hello STCP!").as_bytes();
    if payload.is_empty() { return Err("message must not be empty".into()); }
    let fd = unsafe { OwnedFd::from_raw_fd(check(socket(45, 1, 253))?) };
    let address = Address { family: 2, port: port.to_be(), ip: ip.octets(), padding: [0; 8] };
    unsafe { check(connect(fd.as_raw_fd(), &address, std::mem::size_of::<Address>() as u32))?; }
    let mut client = TcpStream::from(fd);
    for part in payload.chunks(4096) {
        client.write_all(part)?;
        let mut reply = vec![0; part.len()];
        client.read_exact(&mut reply)?;
        if reply != part { return Err("echo mismatch".into()); }
    }
    println!("PASS: {} bytes echoed unchanged", payload.len());
    Ok(())
}
