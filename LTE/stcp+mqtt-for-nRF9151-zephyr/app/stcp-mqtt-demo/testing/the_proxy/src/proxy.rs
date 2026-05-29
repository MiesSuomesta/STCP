use std::ffi::c_void;
use std::net::{TcpListener, TcpStream};
use std::os::fd::AsRawFd;
use std::thread;

use crate::bridge::bridge_loop;
use crate::cleanup::cleanup_connection;
use crate::handshake::run_handshake;
use crate::mqtt::connect_to_broker;
use crate::transport::create_transport;

use crate::debug::stcp_uptime_ms;
use crate::stcp_dbg;

use the_stcp_kernel_module::{
    proto_session::ProtoSession,
    slice_helpers::StcpError,
    tcp_io::stcp_tcp_send,
    tcp_io::stcp_tcp_recv,
    stcp_handshake::rust_session_handshake_lte,
    abi::stcp_transport_wait_until_ready,
};

unsafe extern "C" {
    pub fn stcp_rust_transport_get_fd(
        p: *mut c_void,
    ) -> i32;
}

pub fn run_proxy_server(
    bind_addr: &str
) {

    let listener =
        TcpListener::bind(bind_addr)
            .expect("bind failed");

    stcp_dbg!(
        "[PROXY] Listening on {}",
        bind_addr
    );

    loop {

        let (sock, _) =
            listener.accept()
                .expect("accept failed");

        thread::spawn(move || {

            handle_client(sock);

        });
    }
}

fn handle_client(
    stcp_sock: TcpStream
) {

    stcp_dbg!("[PROXY] Incoming client");

    let fd =
        stcp_sock.as_raw_fd();

    let transport =
        create_transport(fd);

    let mut session =
        ProtoSession::new(
            transport
        );

    stcp_dbg!(
        "[PROXY] Handshake starting...."
    );
 
    if let Err(rc) =
        run_handshake(
            &mut session,
            transport
        ) {

        stcp_dbg!(
            "[PROXY] Handshake failed rc={}",
            rc
        );

        cleanup_connection(
            transport
        );

        return;
    }

    stcp_dbg!(
        "[PROXY] Handshake complete"
    );


    let mut mqtt_sock =
        connect_to_broker(
            "127.0.0.1:1883"
        );

    stcp_dbg!(
        "[PROXY] Setting sockets as non bloking...."
    );
    
    stcp_sock.set_nonblocking(true).expect("nonblocking STCP failed");
    mqtt_sock.set_nonblocking(true).expect("nonblocking MQTT failed");

    stcp_dbg!(
        "[PROXY] AES Bridge starting....."
    );

    bridge_loop(
        &mut session,
        transport,
        &mut mqtt_sock
    );
}
