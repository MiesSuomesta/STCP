use std::net::{TcpListener, TcpStream};
use std::io::{Read, Write};
use std::thread;
use std::time::{Instant, Duration};
use std::sync::{Arc, Mutex};

struct Stats {
    rx_bytes: u64,
    tx_bytes: u64,
    messages: u64,
    errors: u64,
}

fn handle_client(mut sock: TcpStream, stats: Arc<Mutex<Stats>>) {

    println!("MODEM CONNECTED: {}", sock.peer_addr().unwrap());

    let mut buf = [0u8; 4096];

    loop {

        let n = match sock.read(&mut buf) {
            Ok(0) => {
                println!("MODEM DISCONNECTED");
                break;
            }
            Ok(n) => n,
            Err(e) => {
                println!("recv error: {:?}", e);
                stats.lock().unwrap().errors += 1;
                break;
            }
        };

        {
            let mut s = stats.lock().unwrap();
            s.rx_bytes += n as u64;
            s.messages += 1;
        }

        if let Err(e) = sock.write_all(&buf[..n]) {
            println!("send error {:?}", e);
            stats.lock().unwrap().errors += 1;
            break;
        }

        stats.lock().unwrap().tx_bytes += n as u64;
    }
}

fn main() {

    let listener = TcpListener::bind("0.0.0.0:7777")
        .expect("bind failed");

    println!("STCP torture tester listening on :7777");

    let stats = Arc::new(Mutex::new(Stats {
        rx_bytes: 0,
        tx_bytes: 0,
        messages: 0,
        errors: 0,
    }));

    let stats_monitor = stats.clone();

    thread::spawn(move || {

        let start = Instant::now();

        let mut prev_rx = 0;
        let mut prev_tx = 0;

        loop {

            thread::sleep(Duration::from_secs(5));

            let s = stats_monitor.lock().unwrap();

            let rx_rate = (s.rx_bytes - prev_rx) / 5;
            let tx_rate = (s.tx_bytes - prev_tx) / 5;

            prev_rx = s.rx_bytes;
            prev_tx = s.tx_bytes;

            println!(
                "uptime={}s  rx={} B/s  tx={} B/s  messages={} errors={}",
                start.elapsed().as_secs(),
                rx_rate,
                tx_rate,
                s.messages,
                s.errors
            );
        }
    });

    for stream in listener.incoming() {

        match stream {

            Ok(sock) => {
                let stats_clone = stats.clone();

                thread::spawn(|| {
                    handle_client(sock, stats_clone);
                });
            }

            Err(e) => {
                println!("accept error {:?}", e);
            }
        }
    }
}
