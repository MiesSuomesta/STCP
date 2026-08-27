use std::io;
use std::mem::{size_of, zeroed};
use std::net::{Ipv4Addr, SocketAddr, UdpSocket};
use std::os::fd::RawFd;
use std::time::Duration;

const AF_STCP: libc::c_int = 45;
const IPPROTO_STCP: libc::c_int = 253;

const DEFAULT_LISTEN_IP: &str = "0.0.0.0";
const DEFAULT_LISTEN_PORT: u16 = 56830;
const DEFAULT_BACKEND: &str = "127.0.0.1:5683";

struct Fd(RawFd);

impl Drop for Fd {
    fn drop(&mut self) {
        unsafe {
            libc::close(self.0);
        }
    }
}

fn parse_args() -> Result<(Ipv4Addr, u16, SocketAddr), String> {
    let mut args = std::env::args().skip(1);

    let listen_ip: Ipv4Addr = args
        .next()
        .unwrap_or_else(|| DEFAULT_LISTEN_IP.to_string())
        .parse()
        .map_err(|e| format!("invalid listen IPv4: {e}"))?;

    let listen_port: u16 = args
        .next()
        .unwrap_or_else(|| DEFAULT_LISTEN_PORT.to_string())
        .parse()
        .map_err(|e| format!("invalid listen port: {e}"))?;

    let backend: SocketAddr = args
        .next()
        .unwrap_or_else(|| DEFAULT_BACKEND.to_string())
        .parse()
        .map_err(|e| format!("invalid backend address: {e}"))?;

    Ok((listen_ip, listen_port, backend))
}

fn stcp_udp_socket(ip: Ipv4Addr, port: u16) -> io::Result<Fd> {
    let fd = unsafe {
        libc::socket(AF_STCP, libc::SOCK_DGRAM, IPPROTO_STCP)
    };

    if fd < 0 {
        return Err(io::Error::last_os_error());
    }

    let fd = Fd(fd);

    let mut addr: libc::sockaddr_in = unsafe { zeroed() };
    addr.sin_family = libc::AF_INET as libc::sa_family_t;
    addr.sin_port = port.to_be();
    addr.sin_addr = libc::in_addr {
        s_addr: u32::from_ne_bytes(ip.octets()),
    };

    let rc = unsafe {
        libc::bind(
            fd.0,
            (&addr as *const libc::sockaddr_in).cast(),
            size_of::<libc::sockaddr_in>() as libc::socklen_t,
        )
    };

    if rc < 0 {
        return Err(io::Error::last_os_error());
    }

    Ok(fd)
}

fn sockaddr_to_string(addr: &libc::sockaddr_in) -> String {
    let ip = Ipv4Addr::from(addr.sin_addr.s_addr.to_ne_bytes());
    let port = u16::from_be(addr.sin_port);
    format!("{ip}:{port}")
}

fn recv_stcp_from(
    fd: RawFd,
    buf: &mut [u8],
) -> io::Result<(usize, libc::sockaddr_in, libc::socklen_t)> {
    let mut peer: libc::sockaddr_in = unsafe { zeroed() };
    let mut peer_len = size_of::<libc::sockaddr_in>() as libc::socklen_t;

    let n = unsafe {
        libc::recvfrom(
            fd,
            buf.as_mut_ptr().cast(),
            buf.len(),
            0,
            (&mut peer as *mut libc::sockaddr_in).cast(),
            &mut peer_len,
        )
    };

    if n < 0 {
        return Err(io::Error::last_os_error());
    }

    Ok((n as usize, peer, peer_len))
}

fn send_stcp_to(
    fd: RawFd,
    buf: &[u8],
    peer: &libc::sockaddr_in,
    peer_len: libc::socklen_t,
) -> io::Result<()> {
    let n = unsafe {
        libc::sendto(
            fd,
            buf.as_ptr().cast(),
            buf.len(),
            libc::MSG_NOSIGNAL,
            (peer as *const libc::sockaddr_in).cast(),
            peer_len,
        )
    };

    if n < 0 {
        return Err(io::Error::last_os_error());
    }

    if n as usize != buf.len() {
        return Err(io::Error::new(
            io::ErrorKind::WriteZero,
            format!("short STCP datagram send: {n}/{}", buf.len()),
        ));
    }

    Ok(())
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let (listen_ip, listen_port, backend) =
        parse_args().map_err(io::Error::other)?;

    eprintln!("==================================================");
    eprintln!(" STCPv2 CoAP UDP Gateway");
    eprintln!(
        " STCP listen : {}:{} AF_STCP={} SOCK_DGRAM proto={}",
        listen_ip,
        listen_port,
        AF_STCP,
        IPPROTO_STCP,
    );
    eprintln!(" CoAP target : {}", backend);
    eprintln!("==================================================");

    let stcp = stcp_udp_socket(listen_ip, listen_port)?;
    let udp = UdpSocket::bind("127.0.0.1:0")?;
    udp.set_read_timeout(Some(Duration::from_secs(5)))?;

    let mut buf = [0u8; 4096];

    loop {
        let (n, peer, peer_len) = recv_stcp_from(stcp.0, &mut buf)?;
        let peer_str = sockaddr_to_string(&peer);

        if n == 0 {
            continue;
        }

        eprintln!(
            "[gateway] STCP-UDP -> CoAP UDP: {} bytes from {}",
            n,
            peer_str,
        );

        udp.send_to(&buf[..n], backend)?;

        match udp.recv_from(&mut buf) {
            Ok((m, from)) => {
                eprintln!(
                    "[gateway] CoAP UDP -> STCP-UDP: {} bytes from {} to {}",
                    m,
                    from,
                    peer_str,
                );

                send_stcp_to(stcp.0, &buf[..m], &peer, peer_len)?;
            }

            Err(e)
                if e.kind() == io::ErrorKind::WouldBlock
                    || e.kind() == io::ErrorKind::TimedOut =>
            {
                eprintln!(
                    "[gateway] backend timeout waiting CoAP response for {}",
                    peer_str
                );
            }

            Err(e) => return Err(e.into()),
        }
    }
}
