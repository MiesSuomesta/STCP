use std::env;
use std::io::{self, Read, Write};
use std::mem::{size_of, zeroed};
use std::net::{Ipv4Addr, TcpStream};
use std::os::fd::RawFd;
use std::thread;

const AF_STCP: libc::c_int = 45;
const IPPROTO_STCP_TCP: libc::c_int = 253;

const DEFAULT_LISTEN_IP: &str = "0.0.0.0";
const DEFAULT_LISTEN_PORT: u16 = 18830;
const DEFAULT_BACKEND_HOST: &str = "127.0.0.1";
const DEFAULT_BACKEND_PORT: u16 = 1883;

struct StcpFd {
    fd: RawFd,
}

impl StcpFd {
    fn new(fd: RawFd) -> Self {
        Self { fd }
    }

    fn try_clone(&self) -> io::Result<Self> {
        let fd = unsafe { libc::dup(self.fd) };
        if fd < 0 {
            return Err(io::Error::last_os_error());
        }
        Ok(Self::new(fd))
    }

    fn shutdown(&self) {
        unsafe {
            libc::shutdown(self.fd, libc::SHUT_RDWR);
        }
    }
}

impl Read for StcpFd {
    fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        loop {
            let n = unsafe {
                libc::recv(
                    self.fd,
                    buf.as_mut_ptr().cast(),
                    buf.len(),
                    0,
                )
            };

            if n >= 0 {
                return Ok(n as usize);
            }

            let err = io::Error::last_os_error();
            if err.kind() == io::ErrorKind::Interrupted {
                continue;
            }
            return Err(err);
        }
    }
}

impl Write for StcpFd {
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        loop {
            let n = unsafe {
                libc::send(
                    self.fd,
                    buf.as_ptr().cast(),
                    buf.len(),
                    libc::MSG_NOSIGNAL,
                )
            };

            if n >= 0 {
                return Ok(n as usize);
            }

            let err = io::Error::last_os_error();
            if err.kind() == io::ErrorKind::Interrupted {
                continue;
            }
            return Err(err);
        }
    }

    fn flush(&mut self) -> io::Result<()> {
        Ok(())
    }
}

impl Drop for StcpFd {
    fn drop(&mut self) {
        unsafe {
            libc::close(self.fd);
        }
    }
}

fn parse_args() -> Result<(Ipv4Addr, u16, String, u16), String> {
    let mut args = env::args().skip(1);

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

    let backend_host = args
        .next()
        .unwrap_or_else(|| DEFAULT_BACKEND_HOST.to_string());

    let backend_port: u16 = args
        .next()
        .unwrap_or_else(|| DEFAULT_BACKEND_PORT.to_string())
        .parse()
        .map_err(|e| format!("invalid backend port: {e}"))?;

    Ok((listen_ip, listen_port, backend_host, backend_port))
}

fn stcp_listener(ip: Ipv4Addr, port: u16) -> io::Result<StcpFd> {
    let fd = unsafe {
        libc::socket(AF_STCP, libc::SOCK_STREAM, IPPROTO_STCP_TCP)
    };

    if fd < 0 {
        return Err(io::Error::last_os_error());
    }

    let listener = StcpFd::new(fd);

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
    addr.sin_family = AF_STCP as libc::sa_family_t;
    addr.sin_port = port.to_be();
    addr.sin_addr = libc::in_addr {
        s_addr: u32::from_ne_bytes(ip.octets()),
    };

    let rc = unsafe {
        libc::bind(
            listener.fd,
            (&addr as *const libc::sockaddr_in).cast(),
            size_of::<libc::sockaddr_in>() as libc::socklen_t,
        )
    };

    if rc < 0 {
        return Err(io::Error::last_os_error());
    }

    let rc = unsafe { libc::listen(listener.fd, 32) };
    if rc < 0 {
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
            return Ok(StcpFd::new(fd));
        }

        let err = io::Error::last_os_error();
        if err.kind() == io::ErrorKind::Interrupted {
            continue;
        }
        return Err(err);
    }
}

fn pipe<R, W>(mut src: R, mut dst: W, label: &'static str) -> io::Result<u64>
where
    R: Read,
    W: Write,
{
    let mut buf = [0u8; 16 * 1024];
    let mut total = 0u64;

    loop {
        let n = src.read(&mut buf)?;

        if n == 0 {
            return Ok(total);
        }

        dst.write_all(&buf[..n])?;
        total += n as u64;

        if total <= 64 * 1024 {
            eprintln!("[gateway] {label}: +{n} bytes total={total}");
        }
    }
}

fn handle_client(
    client: StcpFd,
    backend_host: String,
    backend_port: u16,
) -> io::Result<()> {
    eprintln!(
        "[gateway] STCP client accepted; connecting MQTT backend {}:{}",
        backend_host,
        backend_port,
    );

    let backend = TcpStream::connect((backend_host.as_str(), backend_port))?;
    backend.set_nodelay(true)?;

    eprintln!("[gateway] MQTT backend connected");

    let stcp_rx = client.try_clone()?;
    let stcp_tx = client;
    let tcp_rx = backend.try_clone()?;
    let tcp_tx = backend;

    let up = thread::spawn(move || {
        pipe(stcp_rx, tcp_tx, "STCP->MQTT")
    });

    let down = thread::spawn(move || {
        pipe(tcp_rx, stcp_tx, "MQTT->STCP")
    });

    let up_result = up.join().unwrap_or_else(|_| {
        Err(io::Error::other("STCP->MQTT thread panicked"))
    });

    let down_result = down.join().unwrap_or_else(|_| {
        Err(io::Error::other("MQTT->STCP thread panicked"))
    });

    match (&up_result, &down_result) {
        (Ok(up_bytes), Ok(down_bytes)) => {
            eprintln!(
                "[gateway] session ended cleanly: up={} down={}",
                up_bytes,
                down_bytes,
            );
        }
        _ => {
            eprintln!(
                "[gateway] session ended: up={:?} down={:?}",
                up_result,
                down_result,
            );
        }
    }

    up_result?;
    down_result?;
    Ok(())
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let (listen_ip, listen_port, backend_host, backend_port) =
        parse_args().map_err(io::Error::other)?;

    eprintln!("==================================================");
    eprintln!(" STCPv2 MQTT Gateway");
    eprintln!(
        " STCP listen : {}:{}  AF_STCP={} proto={}",
        listen_ip,
        listen_port,
        AF_STCP,
        IPPROTO_STCP_TCP,
    );
    eprintln!(
        " MQTT target : {}:{}",
        backend_host,
        backend_port,
    );
    eprintln!("==================================================");

    let listener = stcp_listener(listen_ip, listen_port)?;

    loop {
        match accept_stcp(&listener) {
            Ok(client) => {
                let host = backend_host.clone();

                thread::spawn(move || {
                    if let Err(e) =
                        handle_client(client, host, backend_port)
                    {
                        eprintln!("[gateway] client error: {e}");
                    }
                });
            }

            Err(e) => {
                eprintln!("[gateway] accept failed: {e}");
                thread::sleep(std::time::Duration::from_millis(250));
            }
        }
    }
}
