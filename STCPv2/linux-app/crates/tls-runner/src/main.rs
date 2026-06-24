use std::env;
use std::fs::File;
use std::io::{self, BufReader, Read, Write};
use std::net::{SocketAddr, TcpListener, TcpStream};
use std::sync::Arc;
use std::thread;
use std::time::{Duration, Instant};

use rustls::client::danger::{
    HandshakeSignatureValid,
    ServerCertVerified,
    ServerCertVerifier,
}; 

use rustls::{
    ClientConfig, ClientConnection, RootCertStore, ServerConfig, ServerConnection, StreamOwned,
    DigitallySignedStruct, SignatureScheme,
};
use rustls::pki_types::{CertificateDer, PrivateKeyDer, ServerName, UnixTime};

use stcp_core::packet::{
    STCP_FRAME_PAYLOAD_LEN,
    STCP_RECEIVE_BUFFER_LEN
};

const DEFAULT_CHUNK: usize = 64 * 1024;
const RECV_BUF: usize = STCP_RECEIVE_BUFFER_LEN;

type R<T> = Result<T, Box<dyn std::error::Error + Send + Sync>>;

#[derive(Debug)]
struct ClientStats {
    id: usize,
    sent: usize,
    recv: usize,
    secs: f64,
    conns: usize,
}

#[derive(Debug)]
struct NoCertVerifier;

impl ServerCertVerifier for NoCertVerifier {
    fn verify_server_cert(
        &self,
        _end_entity: &CertificateDer<'_>,
        _intermediates: &[CertificateDer<'_>],
        _server_name: &ServerName<'_>,
        _ocsp_response: &[u8],
        _now: UnixTime,
    ) -> Result<ServerCertVerified, rustls::Error> {
        Ok(ServerCertVerified::assertion())
    }

    fn verify_tls12_signature(
        &self,
        _message: &[u8],
        _cert: &CertificateDer<'_>,
        _dss: &DigitallySignedStruct,
    ) -> Result<HandshakeSignatureValid, rustls::Error> {
        Ok(HandshakeSignatureValid::assertion())
    }

    fn verify_tls13_signature(
        &self,
        _message: &[u8],
        _cert: &CertificateDer<'_>,
        _dss: &DigitallySignedStruct,
    ) -> Result<HandshakeSignatureValid, rustls::Error> {
        Ok(HandshakeSignatureValid::assertion())
    }

    fn supported_verify_schemes(&self) -> Vec<SignatureScheme> {
        vec![
            SignatureScheme::RSA_PKCS1_SHA256,
            SignatureScheme::RSA_PKCS1_SHA384,
            SignatureScheme::RSA_PKCS1_SHA512,
            SignatureScheme::RSA_PSS_SHA256,
            SignatureScheme::RSA_PSS_SHA384,
            SignatureScheme::RSA_PSS_SHA512,
            SignatureScheme::ECDSA_NISTP256_SHA256,
            SignatureScheme::ECDSA_NISTP384_SHA384,
            SignatureScheme::ED25519,
        ]
    }
}

fn connect_tls(
    addr: SocketAddr,
    _cert_path: &str,
) -> R<StreamOwned<ClientConnection, TcpStream>> {
    let config = ClientConfig::builder()
        .dangerous()
        .with_custom_certificate_verifier(Arc::new(NoCertVerifier))
        .with_no_client_auth();

    let server_name = ServerName::try_from("localhost")?;
    let conn = ClientConnection::new(Arc::new(config), server_name)?;
    let tcp = TcpStream::connect(addr)?;

    Ok(StreamOwned::new(conn, tcp))
}

fn main() {
    if let Err(e) = real_main() {
        eprintln!("error: {e}");
        std::process::exit(1);
    }
}

fn real_main() -> R<()> {
    let mut args = env::args().skip(1);
    let cmd = args.next().ok_or("missing command")?;

    match cmd.as_str() {
        "server" => {
            let addr: SocketAddr = args.next().ok_or("missing addr")?.parse()?;
            let cert = args.next().ok_or("missing cert.pem")?;
            let key = args.next().ok_or("missing key.pem")?;
            run_server(addr, &cert, &key)?;
        }

        "client" => {
            let addr: SocketAddr = args.next().ok_or("missing addr")?.parse()?;
            let cert = args.next().ok_or("missing cert.pem")?;
            let msg = args.next().ok_or("missing message")?;
            run_client(addr, &cert, msg.as_bytes())?;
        }

        "send" => {
            let addr: SocketAddr = args.next().ok_or("missing addr")?.parse()?;
            let cert = args.next().ok_or("missing cert.pem")?;
            let bytes: usize = args.next().ok_or("missing byte count")?.parse()?;
            let chunk = args.next().map(|v| v.parse()).transpose()?.unwrap_or(DEFAULT_CHUNK);

            let _ = run_send_client(0, addr, &cert, bytes, chunk)?;
        }

        "send-many" => {
            let addr: SocketAddr = args.next().ok_or("missing addr")?.parse()?;
            let cert = args.next().ok_or("missing cert.pem")?;
            let clients: usize = args.next().ok_or("missing client count")?.parse()?;
            let bytes: usize = args.next().ok_or("missing byte count")?.parse()?;
            let chunk = args.next().map(|v| v.parse()).transpose()?.unwrap_or(DEFAULT_CHUNK);

            run_many_clients(addr, cert, clients, bytes, chunk)?;
        }

        "steady" => {
            let addr: SocketAddr = args.next().ok_or("missing addr")?.parse()?;
            let cert = args.next().ok_or("missing cert.pem")?;
            let clients: usize = args.next().ok_or("missing client count")?.parse()?;
            let seconds: u64 = args.next().ok_or("missing seconds")?.parse()?;
            let chunk = args.next().map(|v| v.parse()).transpose()?.unwrap_or(DEFAULT_CHUNK);

            run_steady(addr, cert, clients, seconds, chunk)?;
        }

        "churn" => {
            let addr: SocketAddr = args.next().ok_or("missing addr")?.parse()?;
            let cert = args.next().ok_or("missing cert.pem")?;
            let clients: usize = args.next().ok_or("missing client count")?.parse()?;
            let seconds: u64 = args.next().ok_or("missing seconds")?.parse()?;
            let bytes: usize = args.next().ok_or("missing bytes per connection")?.parse()?;
            let chunk = args.next().map(|v| v.parse()).transpose()?.unwrap_or(DEFAULT_CHUNK);

            run_churn(addr, cert, clients, seconds, bytes, chunk)?;
        }

        _ => print_usage(),
    }

    Ok(())
}

fn run_server(addr: SocketAddr, cert_path: &str, key_path: &str) -> R<()> {
    let config = Arc::new(load_server_config(cert_path, key_path)?);
    let listener = TcpListener::bind(addr)?;

    println!("TLS server listening on {addr}");

    let mut id = 0usize;

    loop {
        let (tcp, _) = listener.accept()?;
        id += 1;

        let cfg = config.clone();
        let client_id = id;

        thread::spawn(move || {
            handle_client(client_id, tcp, cfg)
        });
    }
}

fn handle_client(
    id: usize,
    tcp: TcpStream,
    config: Arc<ServerConfig>,
) -> R<()> {
    let conn = ServerConnection::new(config)?;
    let mut tls = StreamOwned::new(conn, tcp);

    let mut buf = Vec::with_capacity(RECV_BUF);
    let mut total_recv = 0usize;
    let mut total_sent = 0usize;
    let start = Instant::now();

    loop {
        match recv_frame(&mut tls, &mut buf) {
            Ok(n) => {
                total_recv += n;
                send_frame(&mut tls, &buf[..n])?;
                total_sent += n;
            }

            Err(e) if is_eof(&e) => {
                let secs = start.elapsed().as_secs_f64();
                println!(
                    "[client {id}] closed recv={} sent={} time={:.3}s avg={:.3} MB/s",
                    total_recv,
                    total_sent,
                    secs,
                    mb(total_recv) / secs,
                );
                return Ok(());
            }

            Err(e) => return Err(e),
        }
    }
}

fn run_client(addr: SocketAddr, cert_path: &str, data: &[u8]) -> R<()> {
    let mut tls = connect_tls(addr, cert_path)?;

    send_frame(&mut tls, data)?;

    let mut buf = Vec::new();
    let n = recv_frame(&mut tls, &mut buf)?;

    println!("client received: {}", String::from_utf8_lossy(&buf[..n]));

    Ok(())
}

fn run_many_clients(
    addr: SocketAddr,
    cert: String,
    clients: usize,
    bytes: usize,
    chunk: usize,
) -> R<()> {
    let start = Instant::now();
    let mut threads = Vec::new();

    for id in 0..clients {
        let cert = cert.clone();
        threads.push(thread::spawn(move || {
            run_send_client(id, addr, &cert, bytes, chunk)
        }));
    }

    print_summary("send-many", clients, start, threads)
}

fn run_send_client(
    id: usize,
    addr: SocketAddr,
    cert_path: &str,
    bytes: usize,
    chunk: usize,
) -> R<ClientStats> {
    let mut tls = connect_tls(addr, cert_path)?;

    let mut sent_total = 0usize;
    let mut recv_total = 0usize;

    let data = vec![b'A'; chunk];
    let mut recv_buf = Vec::with_capacity(chunk.max(RECV_BUF));

    let start = Instant::now();
    let mut last_report = Instant::now();
    let mut last_recv = 0usize;

    while sent_total < bytes {
        let left = bytes - sent_total;
        let n = left.min(chunk);

        send_frame(&mut tls, &data[..n])?;
        sent_total += n;

        let got = recv_frame(&mut tls, &mut recv_buf)?;

        if got != n {
            return Err(format!(
                "[client {id}] size mismatch: sent {n}, got {got}"
            ).into());
        }

        recv_total += got;

        let report_elapsed = last_report.elapsed();

        if report_elapsed.as_secs_f64() >= 5.0 {
            let total_elapsed = start.elapsed().as_secs_f64();
            let delta_recv = recv_total - last_recv;

            println!(
                "[client {id}] progress: sent={} recv={} time={:.3}s avg={:.3} MB/s current={:.3} MB/s chunk={}",
                sent_total,
                recv_total,
                total_elapsed,
                mb(recv_total) / total_elapsed,
                mb(delta_recv) / report_elapsed.as_secs_f64(),
                chunk,
            );

            last_report = Instant::now();
            last_recv = recv_total;
        }
    }

    let total_elapsed = start.elapsed().as_secs_f64();

    println!(
        "[client {id}] Final results: sent={} recv={} time={:.3}s avg={:.3} MB/s chunk={}",
        sent_total,
        recv_total,
        total_elapsed,
        mb(recv_total) / total_elapsed,
        chunk,
    );

    Ok(ClientStats {
        id,
        sent: sent_total,
        recv: recv_total,
        secs: total_elapsed,
        conns: 1,
    })
}

fn run_steady(
    addr: SocketAddr,
    cert: String,
    clients: usize,
    seconds: u64,
    chunk: usize,
) -> R<()> {
    let start = Instant::now();
    let deadline = start + Duration::from_secs(seconds);
    let mut threads = Vec::new();

    for id in 0..clients {
        let cert = cert.clone();
        threads.push(thread::spawn(move || {
            run_steady_client(id, addr, &cert, deadline, chunk)
        }));
    }

    print_summary("steady", clients, start, threads)
}

fn run_steady_client(
    id: usize,
    addr: SocketAddr,
    cert_path: &str,
    deadline: Instant,
    chunk: usize,
) -> R<ClientStats> {
    let mut tls = connect_tls(addr, cert_path)?;

    let data = vec![b'S'; chunk];
    let mut recv_buf = Vec::with_capacity(chunk.max(RECV_BUF));

    let start = Instant::now();
    let mut sent = 0usize;
    let mut recv = 0usize;

    while Instant::now() < deadline {
        send_frame(&mut tls, &data)?;
        sent += data.len();

        let got = recv_frame(&mut tls, &mut recv_buf)?;
        recv += got;
    }

    let secs = start.elapsed().as_secs_f64();

    println!(
        "[steady {id}] sent={} recv={} time={:.3}s avg={:.3} MB/s chunk={}",
        sent,
        recv,
        secs,
        mb(recv) / secs,
        chunk
    );

    Ok(ClientStats {
        id,
        sent,
        recv,
        secs,
        conns: 1,
    })
}

fn run_churn(
    addr: SocketAddr,
    cert: String,
    clients: usize,
    seconds: u64,
    bytes_per_conn: usize,
    chunk: usize,
) -> R<()> {
    let start = Instant::now();
    let deadline = start + Duration::from_secs(seconds);
    let mut threads = Vec::new();

    for id in 0..clients {
        let cert = cert.clone();
        threads.push(thread::spawn(move || {
            run_churn_client(id, addr, &cert, deadline, bytes_per_conn, chunk)
        }));
    }

    print_summary("churn", clients, start, threads)
}

fn run_churn_client(
    id: usize,
    addr: SocketAddr,
    cert_path: &str,
    deadline: Instant,
    bytes_per_conn: usize,
    chunk: usize,
) -> R<ClientStats> {
    let start = Instant::now();

    let mut sent = 0usize;
    let mut recv = 0usize;
    let mut conns = 0usize;

    let data = vec![b'C'; chunk];
    let mut recv_buf = Vec::with_capacity(chunk.max(RECV_BUF));

    while Instant::now() < deadline {
        let mut tls = connect_tls(addr, cert_path)?;
        conns += 1;

        let mut conn_sent = 0usize;

        while conn_sent < bytes_per_conn && Instant::now() < deadline {
            let n = (bytes_per_conn - conn_sent).min(chunk);

            send_frame(&mut tls, &data[..n])?;
            sent += n;
            conn_sent += n;

            let got = recv_frame(&mut tls, &mut recv_buf)?;

            if got != n {
                return Err(format!(
                    "[churn {id}] size mismatch: sent {n}, got {got}"
                ).into());
            }

            recv += got;
        }
    }

    let secs = start.elapsed().as_secs_f64();

    println!(
        "[churn {id}] conns={} sent={} recv={} time={:.3}s avg={:.3} MB/s",
        conns,
        sent,
        recv,
        secs,
        mb(recv) / secs,
    );

    Ok(ClientStats {
        id,
        sent,
        recv,
        secs,
        conns,
    })
}

fn print_summary(
    name: &str,
    clients: usize,
    start: Instant,
    threads: Vec<thread::JoinHandle<R<ClientStats>>>,
) -> R<()> {
    let mut total_sent = 0usize;
    let mut total_recv = 0usize;
    let mut total_conns = 0usize;
    let mut slowest_secs = 0.0f64;

    for t in threads {
        let stats = t.join().unwrap()?;

        total_sent += stats.sent;
        total_recv += stats.recv;
        total_conns += stats.conns;

        if stats.secs > slowest_secs {
            slowest_secs = stats.secs;
        }
    }

    let wall_secs = start.elapsed().as_secs_f64();
    let sent_mb = mb(total_sent);
    let recv_mb = mb(total_recv);

    println!();
    println!("=== {name} summary ===");
    println!("Max payload per frame:   {:?}", STCP_FRAME_PAYLOAD_LEN);
    println!("RX bufffer len:          {:?}", STCP_RECEIVE_BUFFER_LEN);
    println!("clients:                 {clients}");
    println!("connections:             {total_conns}");
    println!("total sent:              {} bytes ({:.3} MB)", total_sent, sent_mb);
    println!("total recv:              {} bytes ({:.3} MB)", total_recv, recv_mb);
    println!("wall time:               {:.3} s", wall_secs);
    println!("slowest client time:     {:.3} s", slowest_secs);
    println!("aggregate app speed:     {:.3} MB/s", recv_mb / wall_secs);
    println!("aggregate echo traffic:  {:.3} MB/s", (sent_mb + recv_mb) / wall_secs);
    println!("=======================");

    Ok(())
}

fn send_frame<W: Write>(w: &mut W, data: &[u8]) -> R<()> {
    let len = data.len() as u64;
    w.write_all(&len.to_be_bytes())?;
    w.write_all(data)?;
    w.flush()?;
    Ok(())
}

fn recv_frame<Rd: Read>(r: &mut Rd, buf: &mut Vec<u8>) -> R<usize> {
    let mut hdr = [0u8; 8];
    r.read_exact(&mut hdr)?;

    let len = u64::from_be_bytes(hdr) as usize;

    if buf.len() < len {
        buf.resize(len, 0);
    }

    r.read_exact(&mut buf[..len])?;

    Ok(len)
}

/*
fn connect_tls(
    addr: SocketAddr,
    cert_path: &str,
) -> R<StreamOwned<ClientConnection, TcpStream>> {
    let mut roots = RootCertStore::empty();

    for cert in load_certs(cert_path)? {
        roots.add(cert)?;
    }

    let config = ClientConfig::builder()
        .with_root_certificates(roots)
        .with_no_client_auth();

    let server_name = ServerName::try_from("127.0.0.1")?;
    let conn = ClientConnection::new(Arc::new(config), server_name)?;
    let tcp = TcpStream::connect(addr)?;

    Ok(StreamOwned::new(conn, tcp))
}
*/
fn load_server_config(cert_path: &str, key_path: &str) -> R<ServerConfig> {
    let certs = load_certs(cert_path)?;
    let key = load_private_key(key_path)?;

    let config = ServerConfig::builder()
        .with_no_client_auth()
        .with_single_cert(certs, key)?;

    Ok(config)
}

fn load_certs(path: &str) -> R<Vec<CertificateDer<'static>>> {
    let file = File::open(path)?;
    let mut reader = BufReader::new(file);

    let certs: Vec<_> = rustls_pemfile::certs(&mut reader)
        .collect::<Result<Vec<_>, _>>()?;

    Ok(certs)
}

fn load_private_key(path: &str) -> R<PrivateKeyDer<'static>> {
    let file = File::open(path)?;
    let mut reader = BufReader::new(file);

    let key = rustls_pemfile::private_key(&mut reader)?
        .ok_or("no private key found")?;

    Ok(key)
}

fn is_eof(e: &Box<dyn std::error::Error + Send + Sync>) -> bool {
    if let Some(ioe) = e.downcast_ref::<io::Error>() {
        return ioe.kind() == io::ErrorKind::UnexpectedEof
            || ioe.kind() == io::ErrorKind::ConnectionReset
            || ioe.kind() == io::ErrorKind::BrokenPipe;
    }

    false
}

fn mb(bytes: usize) -> f64 {
    bytes as f64 / 1024.0 / 1024.0
}

fn print_usage() {
    eprintln!("usage:");
    eprintln!("  tls-runner server <addr> <cert.pem> <key.pem>");
    eprintln!("  tls-runner client <addr> <cert.pem> <message>");
    eprintln!("  tls-runner send <addr> <cert.pem> <bytes> [chunk]");
    eprintln!("  tls-runner send-many <addr> <cert.pem> <clients> <bytes> [chunk]");
    eprintln!("  tls-runner steady <addr> <cert.pem> <clients> <seconds> [chunk]");
    eprintln!("  tls-runner churn <addr> <cert.pem> <clients> <seconds> <bytes-per-conn> [chunk]");
}
