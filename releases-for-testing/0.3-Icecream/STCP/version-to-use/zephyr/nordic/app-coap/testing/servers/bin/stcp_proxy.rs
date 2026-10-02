use std::net::{TcpListener, TcpStream};
use std::io::{Read, Write};
use std::thread;
use std::time::Duration;
use rand::Rng;
use std::os::fd::AsRawFd;
use core::ffi::c_void;

use the_stcp_kernel_module::stcp_handshake::rust_session_handshake_lte;
use the_stcp_kernel_module::proto_session::ProtoSession;

unsafe extern "C" {
    // Kernel socket
    pub fn stcp_rust_kernel_socket_create(
        fd: i32
    ) -> *mut c_void;

    pub fn stcp_rust_kernel_socket_destroy(
        sock: *mut c_void
    );
}


fn handle_client(mut stcp_sock: TcpStream) {

    println!("STCP client connected");

    stcp_sock
        .set_read_timeout(Some(Duration::from_secs(10)))
        .ok();

    let mut mosq = match TcpStream::connect("127.0.0.1:1883") {
        Ok(s) => s,
        Err(e) => {
            println!("Mosquitto connect failed: {}", e);
            return;
        }
    };

    mosq.set_read_timeout(Some(Duration::from_secs(10))).ok();

    println!("Connected to Mosquitto");
    let fd = stcp_sock.as_raw_fd();

    let kernel_sock = unsafe { stcp_rust_kernel_socket_create(fd) };

    if kernel_sock.is_null() {
        panic!("Seems we are Out Of Man... Memory");
    }

    let mut session = ProtoSession::new(true, kernel_sock);

    let session_vp =
        (&mut session as *mut ProtoSession) as *mut core::ffi::c_void;
    println!("Running STCP handshake...");

    let rc = unsafe {
        rust_session_handshake_lte(session_vp, kernel_sock)
    };

    println!("Handshake returned {}", rc);

    if rc != 1 {
        println!("Handshake failed");
        return;
    }

    println!("STCP handshake done");

    loop {

        //
        // STCP → MQTT
        //
        let plain = match session.recv_message() {

            Ok(p) => p,

            Err(e) => {
                println!("STCP recv error {:?}", e);
                break;
            }
        };

        if plain.is_empty() {
            continue;
        }

        if mosq.write_all(&plain).is_err() {
            println!("Mosquitto write failed");
            break;
        }

        //
        // MQTT → STCP
        //
        let mut buf = [0u8; 4096];

        let n = match mosq.read(&mut buf) {

            Ok(n) => n,

            Err(_) => {
                continue;
            }
        };

        if n == 0 {
            println!("Mosquitto closed");
            break;
        }

        if let Err(e) = session.send_message(&buf[..n]) {
            println!("STCP send error {:?}", e);
            break;
        }
    }

    //unsafe {
    //    stcp_rust_kernel_socket_destroy(kernel_sock);
    //}

    println!("Client disconnected");
}

fn main() -> std::io::Result<()> {

    let listener = TcpListener::bind("0.0.0.0:7777")?;

    println!("STCP MQTT proxy listening on 7777");

    for conn in listener.incoming() {

        match conn {

            Ok(sock) => {

                thread::spawn(move || {
                    handle_client(sock);
                });

            }

            Err(e) => {
                println!("accept error {}", e);
            }
        }
    }

    Ok(())
}