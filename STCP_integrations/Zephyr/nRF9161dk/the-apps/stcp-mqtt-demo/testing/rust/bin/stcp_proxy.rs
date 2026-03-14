use std::net::{TcpListener, TcpStream};
use std::io::{Read, Write};
use the_stcp_kernel_module::stcp_handshake::rust_session_handshake_lte;
use the_stcp_kernel_module::proto_session::ProtoSession;
use the_stcp_kernel_module::slice_helpers::StcpError;
use std::os::unix::io::AsRawFd;

fn handle_client(mut stcp_sock: TcpStream) -> std::io::Result<()> {

    println!("STCP client connected");

    let mut mosq = TcpStream::connect("127.0.0.1:1883")?;
    println!("Connected to Mosquitto");

let fd = Box::new(stcp_sock.as_raw_fd());

let transport_vp =
    Box::into_raw(fd) as *mut core::ffi::c_void;

    let mut session = ProtoSession::new(true, transport_vp);

    let session_vp =
        (&mut session as *mut ProtoSession) as *mut core::ffi::c_void;

println!("transport_vp = {:p}", transport_vp);
println!("session ptr   = {:p}", &session);
println!("session.transport = {:p}", session.transport);

    println!("Running STCP handshake...");
    let session_vp = (&mut session as *mut ProtoSession) as *mut core::ffi::c_void;
    let transport_vp = (&mut stcp_sock as *mut TcpStream) as *mut core::ffi::c_void;
    let rc = unsafe { rust_session_handshake_lte(session_vp, transport_vp) };
    println!("Handshake returned: {:?}", rc);

    if (rc == 1) {
        println!("STCP handshake done");
    } else {
        println!("STCP handshake failed");
        return Ok(()); // Ei oikeesti ok, mut joo
    }


    loop {

        //
        // STCP → MQTT
        //
        let plain = match session.recv_message() {
            Ok(p) => {
                println!("STCP message {} bytes", p.len());
                println!("first byte {:02X}", p[0]);
                p
            }
            Err(e) => {
                println!("STCP recv error {:?}", e);
                break;
            }
        };

        println!("STCP → MQTT {} bytes", plain.len());

        mosq.write_all(&plain)?;

        //
        // MQTT → STCP
        //
        let mut buf = [0u8; 4096];

        let n = mosq.read(&mut buf)?;

        if n == 0 {
            println!("Mosquitto closed");
            break;
        }

        println!("MQTT → STCP {} bytes", n);

        session.send_message(&buf[..n]).unwrap();
    }

    Ok(())
}

fn main() -> std::io::Result<()> {

    let listener = TcpListener::bind("0.0.0.0:7777")?;

    println!("STCP MQTT proxy listening on 7777");

    for conn in listener.incoming() {

        match conn {
            Ok(sock) => {
                handle_client(sock)?;
            }
            Err(e) => {
                println!("accept error {}", e);
            }
        }
    }

    Ok(())
}