use std::env;
use std::net::SocketAddr;
use std::thread;
use std::time::Instant;

use stcp_core::{
    stcp_accept, stcp_bind, stcp_connect, stcp_listen, stcp_recv, stcp_send, stcp_socket,
};

use stcp_core::packet::{
    STCP_FRAME_PAYLOAD_LEN,
    STCP_RECEIVE_BUFFER_LEN
};

const DEFAULT_CHUNK: usize = 64 * 1024;
const RECV_BUF: usize = STCP_RECEIVE_BUFFER_LEN;

#[derive(Debug)]
struct ClientStats {
    id: usize,
    sent: usize,
    recv: usize,
    secs: f64,
    conns: usize,
}

type R<T> = Result<T, Box<dyn std::error::Error + Send + Sync>>;

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
            run_server(addr)?;
        }

        "client" => {
            let addr: SocketAddr = args.next().ok_or("missing addr")?.parse()?;
            let msg = args.next().ok_or("missing message")?;
            run_client(addr, msg.as_bytes())?;
        }

        "send" => {
            let addr: SocketAddr = args.next().ok_or("missing addr")?.parse()?;
            let bytes: usize = args.next().ok_or("missing byte count")?.parse()?;
            let chunk = args
                .next()
                .map(|v| v.parse())
                .transpose()?
                .unwrap_or(DEFAULT_CHUNK);

            run_send_client(0, addr, bytes, chunk)?;
        }

        "send-many" => {
            let addr: SocketAddr = args.next().ok_or("missing addr")?.parse()?;
            let clients: usize = args.next().ok_or("missing client count")?.parse()?;
            let bytes: usize = args.next().ok_or("missing byte count")?.parse()?;
            let chunk = args
                .next()
                .map(|v| v.parse())
                .transpose()?
                .unwrap_or(DEFAULT_CHUNK);

            run_many_clients(addr, clients, bytes, chunk)?;
        }

        "steady" => {
            let addr: SocketAddr = args.next().ok_or("missing addr")?.parse()?;
            let clients: usize = args.next().ok_or("missing client count")?.parse()?;
            let seconds: u64 = args.next().ok_or("missing seconds")?.parse()?;
            let chunk: usize = args.next().map(|v| v.parse()).transpose()?.unwrap_or(DEFAULT_CHUNK);

            run_steady(addr, clients, seconds, chunk)?;
        }

        "churn" => {
            let addr: SocketAddr = args.next().ok_or("missing addr")?.parse()?;
            let clients: usize = args.next().ok_or("missing client count")?.parse()?;
            let seconds: u64 = args.next().ok_or("missing seconds")?.parse()?;
            let bytes: usize = args.next().ok_or("missing bytes per connection")?.parse()?;
            let chunk: usize = args.next().map(|v| v.parse()).transpose()?.unwrap_or(DEFAULT_CHUNK);

            run_churn(addr, clients, seconds, bytes, chunk)?;
        }

        _ => print_usage(),
    }

    Ok(())
}

// Steady kuormalla ...
fn run_steady(
    addr: SocketAddr,
    clients: usize,
    seconds: u64,
    chunk: usize,
) -> R<()> {
    let start = Instant::now();
    let deadline = start + std::time::Duration::from_secs(seconds);
    let mut threads = Vec::new();

    for id in 0..clients {
        threads.push(thread::spawn(move || {
            run_steady_client(id, addr, deadline, chunk)
        }));
    }

    print_summary("steady", clients, start, threads)
}

fn run_steady_client(
    id: usize,
    addr: SocketAddr,
    deadline: Instant,
    chunk: usize,
) -> R<ClientStats> {
    let sock = stcp_socket()?;
    let mut sock = stcp_connect(sock, addr)?;

    let data = vec![b'S'; chunk];
    let mut recv_buf = vec![0u8; chunk.max(RECV_BUF)];

    let start = Instant::now();
    let mut sent = 0usize;
    let mut recv = 0usize;

    while Instant::now() < deadline {
        stcp_send(&mut sock, &data)?;
        sent += data.len();

        let got = stcp_recv(&mut sock, &mut recv_buf)?;
        recv += got;
    }

    let secs = start.elapsed().as_secs_f64();

    println!(
        "[steady {id}] sent={} recv={} time={:.3}s avg={:.3} MB/s chunk={}",
        sent,
        recv,
        secs,
        (recv as f64 / 1024.0 / 1024.0) / secs,
        chunk
    );

    Ok(ClientStats { id, sent, recv, secs, conns: 1 })
}

// Curn kuormalla 
fn run_churn(
    addr: SocketAddr,
    clients: usize,
    seconds: u64,
    bytes_per_conn: usize,
    chunk: usize,
) -> R<()> {
    let start = Instant::now();
    let deadline = start + std::time::Duration::from_secs(seconds);
    let mut threads = Vec::new();

    for id in 0..clients {
        threads.push(thread::spawn(move || {
            run_churn_client(id, addr, deadline, bytes_per_conn, chunk)
        }));
    }

    print_summary("churn", clients, start, threads)
}

fn run_churn_client(
    id: usize,
    addr: SocketAddr,
    deadline: Instant,
    bytes_per_conn: usize,
    chunk: usize,
) -> R<ClientStats> {
    let start = Instant::now();

    let mut sent = 0usize;
    let mut recv = 0usize;
    let mut conns = 0usize;

    let data = vec![b'C'; chunk];
    let mut recv_buf = vec![0u8; chunk.max(RECV_BUF)];

    while Instant::now() < deadline {
        let sock = stcp_socket()?;
        let mut sock = stcp_connect(sock, addr)?;
        conns += 1;

        let mut conn_sent = 0usize;

        while conn_sent < bytes_per_conn && Instant::now() < deadline {
            let n = (bytes_per_conn - conn_sent).min(chunk);

            stcp_send(&mut sock, &data[..n])?;
            sent += n;
            conn_sent += n;

            let got = stcp_recv(&mut sock, &mut recv_buf)?;

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
        (recv as f64 / 1024.0 / 1024.0) / secs,
    );

    Ok(ClientStats { id, sent, recv, secs, conns })
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
    let sent_mb = total_sent as f64 / 1024.0 / 1024.0;
    let recv_mb = total_recv as f64 / 1024.0 / 1024.0;

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

fn run_server(addr: SocketAddr) -> R<()> {
    let sock = stcp_socket()?;
    let sock = stcp_bind(sock, addr)?;
    let sock = stcp_listen(sock, 128)?;

   // println!("STCP server listening on {addr}");

    let mut id: usize = 0;

    loop {
        let client = stcp_accept(&sock)?;
        id += 1;
        let client_id = id;

       // println!("[client {client_id}] accepted");

        thread::spawn(move || {
            handle_client(client_id, client)
        });
    }
}

fn handle_client(
    id: usize,
    mut client: stcp_core::StcpState,
) -> R<()> {
    let mut buf = vec![0u8; RECV_BUF];
    let mut total: usize = 0;

    loop {
        match stcp_recv(&mut client, &mut buf) {
            Ok(n) => {
                total += n;
                stcp_send(&mut client, &buf[..n])?;
            }

            Err(e) => {
               // println!("[client {id}] closed, total {total} bytes");
                return Err(Box::new(e));
            }
        }
    }
}

fn run_client(addr: SocketAddr, data: &[u8]) -> R<()> {
    let sock = stcp_socket()?;
    let mut sock = stcp_connect(sock, addr)?;

    stcp_send(&mut sock, data)?;

    let mut buf = vec![0u8; RECV_BUF];
    let n = stcp_recv(&mut sock, &mut buf)?;

   // println!("client received: {}", String::from_utf8_lossy(&buf[..n]));
    Ok(())
}

fn run_many_clients(
    addr: SocketAddr,
    clients: usize,
    bytes: usize,
    chunk: usize,
) -> R<()> {
    let start = Instant::now();
    let mut threads = Vec::new();

    for id in 0..clients {
        threads.push(thread::spawn(move || {
            run_send_client(id, addr, bytes, chunk)
        }));
    }

    let mut total_sent = 0usize;
    let mut total_recv = 0usize;
    let mut slowest_secs = 0.0f64;

    for t in threads {
        let stats = t.join().unwrap()?;

        total_sent += stats.sent;
        total_recv += stats.recv;

        if stats.secs > slowest_secs {
            slowest_secs = stats.secs;
        }
    }

    let wall_secs = start.elapsed().as_secs_f64();

    let total_sent_mb = total_sent as f64 / 1024.0 / 1024.0;
    let total_recv_mb = total_recv as f64 / 1024.0 / 1024.0;

    let app_mb = total_recv_mb;
    let echo_socket_mb = total_sent_mb + total_recv_mb;

    println!();
    println!("=== send-many summary ===");
    println!("Max payload per frame:   {:?}", STCP_FRAME_PAYLOAD_LEN);
    println!("RX bufffer len:          {:?}", STCP_RECEIVE_BUFFER_LEN);
    println!("clients:                 {clients}");
    println!("bytes/client:            {bytes}");
    println!("chunk:                   {chunk}");
    println!("total sent:              {} bytes ({:.3} MB)", total_sent, total_sent_mb);
    println!("total recv:              {} bytes ({:.3} MB)", total_recv, total_recv_mb);
    println!("wall time:               {:.3} s", wall_secs);
    println!("slowest client time:     {:.3} s", slowest_secs);
    println!("aggregate app speed:     {:.3} MB/s", app_mb / wall_secs);
    println!("aggregate echo traffic:  {:.3} MB/s", echo_socket_mb / wall_secs);
    println!("=========================");

    Ok(())
}

fn run_send_client(
    id: usize,
    addr: SocketAddr,
    bytes: usize,
    chunk: usize,
) -> R<ClientStats> {
    
    let sock = stcp_socket()?;
    let mut sock = stcp_connect(sock, addr)?;

    let mut sent_total = 0usize;
    let mut recv_total = 0usize;

    let data = vec![b'A'; chunk];
    let mut recv_buf = vec![0u8; chunk.max(RECV_BUF)];

    let start = Instant::now();
    let mut last_report = Instant::now();

    let mut last_sent = 0usize;
    let mut last_recv = 0usize;

    while sent_total < bytes {
        let left = bytes - sent_total;
        let n = left.min(chunk);

        stcp_send(&mut sock, &data[..n])?;
        sent_total += n;

        let got = stcp_recv(&mut sock, &mut recv_buf)?;

        if got != n {
            return Err(format!(
                "[client {id}] size mismatch: sent {n}, got {got}"
            ).into());
        }

        recv_total += got;

        let report_elapsed = last_report.elapsed();

        if report_elapsed.as_secs_f64() >= 5.0 {
            let total_elapsed = start.elapsed().as_secs_f64();

            let delta_sent = sent_total - last_sent;
            let delta_recv = recv_total - last_recv;

            let avg_mb = recv_total as f64 / 1024.0 / 1024.0;
            let inst_mb = delta_recv as f64 / 1024.0 / 1024.0;

            println!(
                "[client {id}] progress: sent={} recv={} time={:.3}s avg={:.3} MB/s current={:.3} MB/s chunk={}",
                sent_total,
                recv_total,
                total_elapsed,
                avg_mb / total_elapsed,
                inst_mb / report_elapsed.as_secs_f64(),
                chunk,
            );

            last_report = Instant::now();
            last_sent = sent_total;
            last_recv = recv_total;
        }
    }

    let total_elapsed = start.elapsed().as_secs_f64();
    let total_mb = recv_total as f64 / 1024.0 / 1024.0;

    println!(
        "[client {id}] Final results: sent={} recv={} time={:.3}s avg={:.3} MB/s chunk={}",
        sent_total,
        recv_total,
        total_elapsed,
        total_mb / total_elapsed,
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

fn print_usage() {
    eprintln!("usage:");
    eprintln!("  stcp-runner server <addr>");
    eprintln!("  stcp-runner client <addr> <message>");
    eprintln!("  stcp-runner send <addr> <bytes> [chunk]");
    eprintln!("  stcp-runner send-many <addr> <clients> <bytes> [chunk]");
    eprintln!("  stcp-runner steady <addr> <clients> <seconds> [chunk]");
    eprintln!("  stcp-runner churn <addr> <clients> <seconds> <bytes-per-conn> [chunk]");
}
