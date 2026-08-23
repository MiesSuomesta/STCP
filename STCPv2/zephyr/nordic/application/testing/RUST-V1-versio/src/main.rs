mod stcp_platform_linux;

use the_stcp_kernel_module::stcp_handshake::rust_session_handshake_lte;
use std::net::{TcpListener, TcpStream, Shutdown};
use std::thread;
use std::time::Duration;
use std::io::{Read, Write, /* ErrorKind */};
use nix::poll::{poll, PollTimeout, PollFd, PollFlags};
use std::os::unix::io::AsFd;

use the_stcp_kernel_module::proto_session::ProtoSession;
use the_stcp_kernel_module::slice_helpers::StcpError;
use the_stcp_kernel_module::stcp_dbg;
use std::io::ErrorKind;


use core::ffi::c_void;

const FORWARD_HOST: &str = "127.0.0.1";
const FORWARD_PORT: u16 = 1883;

fn main() {

    let listener = TcpListener::bind("0.0.0.0:7777").unwrap();

    stcp_dbg!("=========================================");
    stcp_dbg!(" STCP Rust Proxy Server");
    stcp_dbg!(" Listening on 0.0.0.0:7777");
    stcp_dbg!(" Forward to {}:{}", FORWARD_HOST, FORWARD_PORT);
    stcp_dbg!("=========================================");
    let poll_flags =
        PollFlags::POLLIN
        | PollFlags::POLLERR
        | PollFlags::POLLHUP;
    loop {

        let mut fds = [
            PollFd::new(listener.as_fd(), poll_flags)
        ];

        poll(&mut fds, PollTimeout::from(1000u16)).unwrap();

        if let Some(revents) = fds[0].revents() {

            if revents.contains(PollFlags::POLLIN) {

                match listener.accept() {

                    Ok((stream, addr)) => {

                        stcp_dbg!("New STCP client {}", addr);

                        thread::spawn(move || {
                           let _ = handle_client(stream);
                        });

                    }

                    Err(e) => {
                        stcp_dbg!("accept error {:?}", e);
                    }
                }
            }
        }
    }    
}

fn handle_client(mut stream: TcpStream) -> Result<(), StcpError> {

    stcp_dbg!("New client {:?}", stream.peer_addr());

    stream.set_nodelay(true).ok();
    stream.set_nonblocking(true).ok();

    let transport = &mut stream as *mut _ as *mut c_void;

    let mut session = ProtoSession::new(true, transport);

    // =============================
    // STCP HANDSHAKE
    // =============================
    let poll_flags =
        PollFlags::POLLIN
        | PollFlags::POLLERR
        | PollFlags::POLLHUP;

    loop {
        let mut fds = [
            PollFd::new(stream.as_fd(), poll_flags)
        ];

        poll(&mut fds, PollTimeout::from(1000u16)).unwrap();

        if !fds[0]
            .revents()
            .unwrap_or(PollFlags::empty())
            .contains(PollFlags::POLLIN)
        {
            continue;
        }

        stcp_dbg!("Server HS state: {}", session.get_status().to_raw());

        let hsret = rust_session_handshake_lte(
            &mut session as *mut _ as *mut c_void,
            transport,
        );

        if hsret == 1 {
            stcp_dbg!("Handshake complete");
            break;
        }

        if hsret == -11 {
            continue;
        }
    }

    stcp_dbg!("AES session established");


    // =============================
    // PROXY LOOP
    // =============================
    let mut do_outer_loop = true;
    let mut do_inner_loop;
    while do_outer_loop {

        // =============================
        // BACKEND CONNECT
        // =============================
        stcp_dbg!("Connecting backend....");
        let mut backend =
            TcpStream::connect((FORWARD_HOST, FORWARD_PORT))
            .map_err(|_| StcpError::Invalid)?;

        backend.set_nodelay(true).ok();
        backend.set_nonblocking(true).ok();
        stcp_dbg!("Backend connected.");

        do_inner_loop = true;
        while do_inner_loop {
            let (stream_ready, backend_ready) = {

                let mut fds = [
                    PollFd::new(stream.as_fd(), PollFlags::POLLIN),
                    PollFd::new(backend.as_fd(), PollFlags::POLLIN),
                ];

                poll(&mut fds, PollTimeout::from(100u16)).unwrap();

                let s = fds[0]
                    .revents()
                    .unwrap_or(PollFlags::empty())
                    .contains(PollFlags::POLLIN);

                let b = fds[1]
                    .revents()
                    .unwrap_or(PollFlags::empty())
                    .contains(PollFlags::POLLIN);

                (s, b)
            };

            if stream_ready {

                match session.recv_message() {

                    Ok(data) => {
                        if !data.is_empty() {
                            backend.write_all(&data).ok();
                        }
                    }

                    Err(StcpError::Again) => {}


                    Err(StcpError::PeerDisconnected) => {
                        stcp_dbg!("Peer disconnected");
                        do_inner_loop = false;
                        continue;
                    }

                    Err(e) => {
                        stcp_dbg!("STCP recv error {:?}", e);
                        do_inner_loop = false;
                        continue;
                    }
                }
            }

            if backend_ready {

                let mut buf = [0u8; 4096];

                match backend.read(&mut buf) {

                    Ok(0) => {
                        stcp_dbg!("Backend closed");
                        break;
                    }

                    Ok(n) => {
                        session.send_message(&buf[..n]).ok();
                    }


                    Err(ref e) if e.kind() == ErrorKind::WouldBlock => {
                        //thread::sleep(Duration::from_millis(500));
                    }
                    Err(ref e) if e.kind() == ErrorKind::TimedOut => {
                        //thread::sleep(Duration::from_millis(50));
                    }

                    Err(ref e) if e.kind() == ErrorKind::ConnectionReset => {
                        do_inner_loop = false;
                        //thread::sleep(Duration::from_millis(500));
                        continue;
                    }

                    Err(ref e) if e.kind() == ErrorKind::BrokenPipe => {
                        do_inner_loop = false;
                        //thread::sleep(Duration::from_millis(500));
                        continue;
                    }

                    Err(e) => {
                        stcp_dbg!("Backend read error {:?}", e);
                        do_inner_loop = false;
                        //thread::sleep(Duration::from_millis(500));
                        continue;
                    }
                }
            }        
        }
        stcp_dbg!("Proxy closed => Reconnecting: {}", do_outer_loop);
        //if (do_outer_loop) {
        //    thread::sleep(Duration::from_millis(500));
        //}
    }

    let _ = stream.shutdown(Shutdown::Both);

    Ok(())
}
