use std::net::{TcpListener, TcpStream};
use std::io::{Read, Write};
use std::thread;
use std::time::{Instant, Duration};
use std::sync::{Arc, Mutex};

struct Stats {
    rx_bytes: u64,
    tx_bytes: u64,
    rx_messages: u64,
    tx_messages: u64,
    rx_errors: u64,
    tx_errors: u64,
}

fn parse_frames(buf: &[u8], stats: &Arc<Mutex<Stats>>) -> i32 {
    let mut offset = 0;
    let mut msgs = 0;

    while offset + 16 <= buf.len() {
        let len = u32::from_be_bytes([
            buf[offset + 12],
            buf[offset + 13],
            buf[offset + 14],
            buf[offset + 15],
        ]) as usize;

        let frame_size = 16 + len;

        if offset + frame_size > buf.len() {
            break;
        }

        println!("FRAME len={}", len);

        {
            let mut s = stats.lock().unwrap();
            s.rx_bytes += len as u64; // 👈 payload, ei TCP chunk
        }

        offset += frame_size;
        msgs += 1;
    }
    
    msgs
}

fn parse_stats_frame(buf: &[u8], stats: Arc<Mutex<Stats>>) -> i32 {
    let mut offset = 0;
    let mut msgs = 0;

    while offset + 16 <= buf.len() {
        let len = u32::from_be_bytes([
            buf[offset + 12],
            buf[offset + 13],
            buf[offset + 14],
            buf[offset + 15],
        ]) as usize;

        let frame_size = 16 + len;

        if offset + frame_size > buf.len() {
            break; // incomplete
        }

        println!("FRAME len={}", len);

        offset += frame_size;
        stats.lock().unwrap().rx_bytes += len as u64;
        msgs += 1;
    }
    
    msgs
}

fn handle_client(mut sock: TcpStream, stats: Arc<Mutex<Stats>>) {

    // Ei blokkaavaksi tehdä ...

    println!("MODEM CONNECTED: {}", sock.peer_addr().unwrap());
 
    let mut buf = [0u8; 4096];
    let mut check = 100;
    let statsStr = "STATISTICS";

    loop {
        check -= 1;
        
        sock.set_nonblocking(false).unwrap();
        std::thread::sleep(Duration::from_millis(1));

        if (check < 1) {
            sock.set_nonblocking(false).unwrap();
            let statsStrBytes = statsStr.as_bytes();
            let _ = match sock.write(statsStrBytes) {
                Ok(_) => {
                    println!("sent stats");
                },
                Err(e) => {
                    println!("send error: {:?}", e);
                    break;
                }
            };

            let n = match sock.read(&mut buf) {
                Ok(0) => {
                    println!("MODEM DISCONNECTED");
                    break;
                },
                Ok(n) => {
                    println!("recv stats: {:?}", &buf[..n]);

                },
                Err(e) => {
                    println!("recv error: {:?}", e);
                    break;
                }
            };

            sock.set_nonblocking(true).unwrap();
        }

        let n = match sock.read(&mut buf) {
            Ok(0) => {
                println!("MODEM DISCONNECTED");
                break;
            },
            Ok(n) => {
                let msgs = parse_frames(&buf[..n], &stats.clone());
                stats.lock().unwrap().rx_messages += msgs as u64;
                n
            },
            Err(ref e) if e.kind() == std::io::ErrorKind::WouldBlock => {
                0 // metching 
            },
            Err(e) => {
                println!("recv error: {:?}", e);
                stats.lock().unwrap().rx_errors += 1;
                break;
            }
        };
        
        match sock.write(&buf) {
            Ok(n) => {
                stats.lock().unwrap().tx_bytes += n as u64;
                stats.lock().unwrap().tx_messages += 1;
            },
            Err(ref e) if e.kind() == std::io::ErrorKind::WouldBlock => {
            },
            Err(e) => {
                stats.lock().unwrap().tx_errors += 1;
                println!("send error: {:?}", e);
                break;
            }
        }
    }
}

fn main() {

    let listener = TcpListener::bind("0.0.0.0:7777")
        .expect("bind failed");

    println!("STCP torture tester listening on :7777");

    let stats = Arc::new(Mutex::new(Stats {
        rx_bytes: 0,
        tx_bytes: 0,
        rx_messages: 0,
        tx_messages: 0,
        rx_errors: 0,
        tx_errors: 0
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
                "uptime={}s  rx={} ({}) B/s  tx={} ({}) B/s  messages(RX/TX)={}/{} errors(RX/TX)={}/{}",
                start.elapsed().as_secs(),
                rx_rate,
                prev_rx,
                tx_rate,
                prev_tx,
                s.rx_messages,
                s.tx_messages,
                s.rx_errors,
                s.tx_errors
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
