
use std::net::TcpStream;
use std::thread;
use std::time::{Duration, Instant};
use std::io::{Read, Write};

use rand::Rng;
use std::os::fd::AsRawFd;
use core::ffi::c_void;

use the_stcp_kernel_module::proto_session::ProtoSession;
use the_stcp_kernel_module::stcp_handshake::rust_session_handshake_lte;

const TARGET: &str = "127.0.0.1:9000";

const THREADS: usize = 32;
const MESSAGES_PER_CONN: usize = 1000;

unsafe extern "C" {
    // Kernel socket
    pub fn stcp_rust_kernel_socket_create(
        fd: i32
    ) -> *mut c_void;

    pub fn stcp_rust_kernel_socket_destroy(
        sock: *mut c_void
    );
}

fn run_client(id: usize) {

    loop {

        println!("[{}] connecting", id);

        let mut sock = match TcpStream::connect(TARGET) {
            Ok(s) => s,
            Err(e) => {
                println!("[{}] connect fail {}", id, e);
                thread::sleep(Duration::from_millis(500));
                continue;
            }
        };

        sock.set_nodelay(true).ok();
        let fd = sock.as_raw_fd();

        let kernel_sock = unsafe { stcp_rust_kernel_socket_create(fd) };

        if kernel_sock.is_null() {
            panic!("Seems we are Out Of Man... Memory");
        }

        let mut session = ProtoSession::new(true, kernel_sock);

        let session_vp =
            (&mut session as *mut ProtoSession) as *mut core::ffi::c_void;

        println!("[{}] handshake", id);

        let rc = unsafe {
            rust_session_handshake_lte(session_vp, kernel_sock)
        };

        if rc != 1 {
            println!("[{}] handshake failed", id);
            thread::sleep(Duration::from_millis(200));
            continue;
        }

        println!("[{}] handshake ok", id);

        let mut rng = rand::thread_rng();

        for i in 0..MESSAGES_PER_CONN {

            let size = rng.gen_range(1..512);

            let mut payload = vec![0u8; size];

            rng.fill(&mut payload[..]);

            if let Err(e) = session.send_message(&payload) {
                println!("[{}] send error {:?}", id, e);
                break;
            }

            match session.recv_message() {

                Ok(msg) => {
                    println!(
                        "[{}] recv {} bytes (msg {})",
                        id,
                        msg.len(),
                        i
                    );
                }

                Err(e) => {
                    println!("[{}] recv error {:?}", id, e);
                    break;
                }
            }

            if rng.gen_bool(0.1) {
                thread::sleep(Duration::from_millis(
                    rng.gen_range(1..20)
                ));
            }
        }

        println!("[{}] reconnect", id);
        //unsafe {
        //    stcp_rust_kernel_socket_destroy(kernel_sock);
        //}
        thread::sleep(Duration::from_millis(50));
    }
}

fn main() {

    println!("STCP TORTURE TEST");
    println!("target {}", TARGET);
    println!("threads {}", THREADS);

    for i in 0..THREADS {

        thread::spawn(move || {
            run_client(i);
        });
    }

    loop {
        thread::sleep(Duration::from_secs(60));
    }
}
