use std::env;
use std::io;
use std::mem::{size_of, zeroed};
use std::net::{Ipv4Addr, SocketAddr, UdpSocket};
use std::os::fd::RawFd;
use std::thread;
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};

const AF_STCP: libc::c_int = 45;
const IPPROTO_STCP_UDP: libc::c_int = 254;

const DEFAULT_LISTEN_IP: &str = "0.0.0.0";
const DEFAULT_LISTEN_PORT: u16 = 56830;
const DEFAULT_BACKEND: &str = "127.0.0.1:5683";

fn wall_us() -> u128 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default()
        .as_micros()
}

fn diag(event: &str) {
    eprintln!("[HSWIRE wall_us={}] {event}", wall_us());
}

struct StcpFd {
    fd: RawFd,
}

impl Drop for StcpFd {
    fn drop(&mut self) {
        unsafe { libc::close(self.fd); }
    }
}

fn parse_args() -> Result<(Ipv4Addr, u16, SocketAddr), String> {
    let mut args = env::args().skip(1);

    let listen_ip = args
        .next()
        .unwrap_or_else(|| DEFAULT_LISTEN_IP.to_string())
        .parse()
        .map_err(|e| format!("invalid listen IPv4: {e}"))?;

    let listen_port = args
        .next()
        .unwrap_or_else(|| DEFAULT_LISTEN_PORT.to_string())
        .parse()
        .map_err(|e| format!("invalid listen port: {e}"))?;

    let backend = args
        .next()
        .unwrap_or_else(|| DEFAULT_BACKEND.to_string())
        .parse()
        .map_err(|e| format!("invalid backend address: {e}"))?;

    Ok((listen_ip, listen_port, backend))
}

fn stcp_listener(ip: Ipv4Addr, port: u16) -> io::Result<StcpFd> {
    /*
     * Native STCP-UDP public ABI restored in the kernel:
     * AF_STCP + SOCK_DGRAM + protocol 254.
     *
     * STCP still exposes bind/listen/accept at the logical socket layer;
     * internally protocol 254 owns a real UDP carrier and accepted children
     * share the listener's UDP carrier.
     */
    let fd = unsafe {
        libc::socket(AF_STCP, libc::SOCK_DGRAM, IPPROTO_STCP_UDP)
    };
    if fd < 0 {
        return Err(io::Error::last_os_error());
    }

    let listener = StcpFd { fd };

    let one: libc::c_int = 1;
    unsafe {
        libc::setsockopt(
            listener.fd,
            libc::SOL_SOCKET,
            libc::SO_REUSEADDR,
            (&one as *const libc::c_int).cast(),
            size_of::<libc::c_int>() as libc::socklen_t,
        );
    }

    let mut addr: libc::sockaddr_in = unsafe { zeroed() };
    addr.sin_family = libc::AF_INET as libc::sa_family_t;
    addr.sin_port = port.to_be();
    addr.sin_addr = libc::in_addr {
        s_addr: u32::from_ne_bytes(ip.octets()),
    };

    if unsafe {
        libc::bind(
            listener.fd,
            (&addr as *const libc::sockaddr_in).cast(),
            size_of::<libc::sockaddr_in>() as libc::socklen_t,
        )
    } < 0 {
        return Err(io::Error::last_os_error());
    }

    if unsafe { libc::listen(listener.fd, 32) } < 0 {
        return Err(io::Error::last_os_error());
    }

    Ok(listener)
}

fn accept_stcp(listener: &StcpFd) -> io::Result<StcpFd> {
    loop {
        let fd = unsafe {
            libc::accept(
                listener.fd,
                std::ptr::null_mut(),
                std::ptr::null_mut(),
            )
        };

        if fd >= 0 {
            diag(&format!("ACCEPT_RETURN child_fd={fd}"));
            return Ok(StcpFd { fd });
        }

        let e = io::Error::last_os_error();
        if e.kind() == io::ErrorKind::Interrupted {
            continue;
        }
        return Err(e);
    }
}

fn stcp_recv(fd: RawFd, buf: &mut [u8]) -> io::Result<usize> {
    loop {
        let n = unsafe {
            libc::recv(fd, buf.as_mut_ptr().cast(), buf.len(), 0)
        };
        if n >= 0 {
            return Ok(n as usize);
        }

        let e = io::Error::last_os_error();
        if e.kind() == io::ErrorKind::Interrupted {
            continue;
        }
        return Err(e);
    }
}

fn stcp_send_all(fd: RawFd, buf: &[u8]) -> io::Result<()> {
    let mut off = 0usize;
    while off < buf.len() {
        let n = unsafe {
            libc::send(
                fd,
                buf[off..].as_ptr().cast(),
                buf.len() - off,
                libc::MSG_NOSIGNAL,
            )
        };

        if n < 0 {
            let e = io::Error::last_os_error();
            if e.kind() == io::ErrorKind::Interrupted {
                continue;
            }
            return Err(e);
        }
        if n == 0 {
            return Err(io::Error::new(
                io::ErrorKind::WriteZero,
                "STCP send returned 0",
            ));
        }

        off += n as usize;
    }
    Ok(())
}

fn handle_client(client: StcpFd, backend: SocketAddr) -> io::Result<()> {
    eprintln!("[gateway] STCP-UDP client accepted; backend={backend}");
    diag(&format!("CLIENT_HANDLER_START fd={}", client.fd));

    let udp = UdpSocket::bind("127.0.0.1:0")?;
    udp.connect(backend)?;
    udp.set_read_timeout(Some(Duration::from_secs(10)))?;

    let mut req = [0u8; 4096];
    let mut rsp = [0u8; 4096];

    loop {
        let n = stcp_recv(client.fd, &mut req)?;
        if n == 0 {
            eprintln!("[gateway] STCP-UDP client closed");
            return Ok(());
        }

        eprintln!("[gateway] STCP-UDP -> CoAP UDP: {n} bytes");
        diag(&format!("APP_STCP_RX fd={} bytes={n}", client.fd));

        let t = Instant::now();
        let sent = udp.send(&req[..n])?;
        diag(&format!("BACKEND_UDP_TX bytes={sent} elapsed_us={}", t.elapsed().as_micros()));
        if sent != n {
            return Err(io::Error::new(
                io::ErrorKind::WriteZero,
                format!("short UDP send {sent}/{n}"),
            ));
        }

        match udp.recv(&mut rsp) {
            Ok(m) => {
                eprintln!("[gateway] CoAP UDP -> STCP-UDP: {m} bytes");
                diag(&format!("BACKEND_UDP_RX bytes={m}"));
                let t = Instant::now();
                stcp_send_all(client.fd, &rsp[..m])?;
                diag(&format!("APP_STCP_TX fd={} bytes={m} elapsed_us={}", client.fd, t.elapsed().as_micros()));
            }
            Err(e)
                if e.kind() == io::ErrorKind::TimedOut
                    || e.kind() == io::ErrorKind::WouldBlock =>
            {
                eprintln!("[gateway] CoAP backend response timeout");
            }
            Err(e) => return Err(e),
        }
    }
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let (listen_ip, listen_port, backend) =
        parse_args().map_err(io::Error::other)?;

    eprintln!("==================================================");
    eprintln!(" STCPv2 CoAP Gateway");
    eprintln!(
        " STCP listen : {}:{} AF_STCP={} SOCK_DGRAM proto={}",
        listen_ip, listen_port, AF_STCP, IPPROTO_STCP_UDP
    );
    eprintln!(" CoAP target : {backend}");
    eprintln!("==================================================");

    let listener = stcp_listener(listen_ip, listen_port)?;
    diag(&format!("LISTENER_READY port={listen_port}"));

    loop {
        match accept_stcp(&listener) {
            Ok(client) => {
                thread::spawn(move || {
                    if let Err(e) = handle_client(client, backend) {
                        eprintln!("[gateway] client error: {e}");
                    }
                });
            }
            Err(e) => {
                eprintln!("[gateway] accept failed: {e}");
                thread::sleep(Duration::from_millis(250));
            }
        }
    }
}
