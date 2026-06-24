use std::net::{TcpListener, TcpStream};
use std::sync::{Arc, Mutex};
use std::thread;
use std::io::{Read, Write};
use std::time::Duration;

const MODEM_PORT: u16 = 7777;
const CLIENT_PORT: u16 = 9000;


fn start_stats_poll(modem: Arc<Mutex<TcpStream>>) {

    thread::spawn(move || {

        let mut buf = [0u8; 1024];

        loop {

            thread::sleep(Duration::from_secs(5));

            let mut stream = modem.lock().unwrap();

            if let Err(e) = stream.write_all(b"STATISTICS") {
                println!("stats send failed: {:?}", e);
                continue;
            }

            match stream.read(&mut buf) {
                Ok(n) => {
                    let s = String::from_utf8_lossy(&buf[..n]);
                    println!("STCP stats:\n{}", s);
                }
                Err(e) => {
                    println!("stats recv failed: {:?}", e);
                }
            }
        }
    });
}

fn pipe(mut src: TcpStream, mut dst: TcpStream) {

    let mut buf = [0u8; 4096];

    loop {

        let n = match src.read(&mut buf) {
            Ok(0) => return,
            Ok(n) => n,
            Err(_) => return,
        };

        if dst.write_all(&buf[..n]).is_err() {
            return;
        }
    }
}

fn main() {

    println!("Waiting modem connection...");

    let modem_listener =
        TcpListener::bind(("0.0.0.0", MODEM_PORT)).unwrap();

    let (modem_stream, addr) =
        modem_listener.accept().unwrap();

    println!("Modem connected from {}", addr);

    let modem = Arc::new(Mutex::new(modem_stream));

    let client_listener =
        TcpListener::bind(("0.0.0.0", CLIENT_PORT)).unwrap();

    start_stats_poll(modem.clone());

    println!("Client gateway listening {}", CLIENT_PORT);

    for stream in client_listener.incoming() {

        if let Ok(client) = stream {

            println!("Client connected");

            let modem_lock = modem.clone();

            thread::spawn(move || {

                // lukitaan vain hetkeksi
                let modem_stream = {
                    let guard = modem_lock.lock().unwrap();
                    guard.try_clone().unwrap()
                };

                let client_clone = client.try_clone().unwrap();
                let modem_clone = modem_stream.try_clone().unwrap();

                let t1 = thread::spawn(move || {
                    pipe(client_clone, modem_clone);
                });

                let t2 = thread::spawn(move || {
                    pipe(modem_stream, client);
                });

                let _ = t1.join();
                let _ = t2.join();

                println!("Client session ended");
            });
        }
    }
}