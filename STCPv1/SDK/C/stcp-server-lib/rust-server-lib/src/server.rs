use std::result::Result;
use tokio::net::TcpStream;
use tokio::io::AsyncWriteExt;

use stcpcommon::aes_lib as myAES;
use stcpcommon::stcp_elliptic_codec as myEC;
use stcpcommon::utils as myUtils;
use stcpcommon::defines_etc as myDefs;
use stcpcommon::defines_etc::{StcpServer, ServerMessageProcessCB};

use stcpcommon::dprint;

pub fn stcp_internal_server_bind(ip: &str, port: u16, cb: ServerMessageProcessCB) -> Result<StcpServer, Box<dyn std::error::Error>> {
    let addr = format!("{}:{}", ip, port);
    let listener = tokio::runtime::Runtime::new()?.block_on(tokio::net::TcpListener::bind(&addr))?;
    Ok(StcpServer { listener, port, callback: cb })
}

pub fn stcp_internal_server_listen(server: &mut StcpServer) -> Result<(), Box<dyn std::error::Error>> {
    let rt = tokio::runtime::Runtime::new()?;
    rt.block_on(async move {
        loop {
            let (stream, addr) = server.listener.accept().await?;
            println!("New connection from: {}", addr);
            let cb = server.callback;
            tokio::spawn(async move {
                handle_client_connection(stream, cb).await;
            });
        }
    })
}

pub fn stcp_internal_server_stop(server_ptr: *mut StcpServer) {
    if server_ptr.is_null() {
        return;
    }
    unsafe {
        drop(Box::from_raw(server_ptr));
    }
}

pub async fn handle_client_connection(mut stream: TcpStream, server_cb: ServerMessageProcessCB) {

    let theEC: myEC::StcpEllipticCodec = myEC::StcpEllipticCodec::new();
    let theAES: myAES::StcpAesCodec = myAES::StcpAesCodec::new();
    let theUtilit: myUtils::StcpUtils = myUtils::StcpUtils::new();

    let the_shared_key: Vec<u8> = Vec::new();
    let the_peer_public_key: Vec<u8> = Vec::new();

    let (the_peer_public_key, the_shared_key) = theUtilit.do_the_stcp_handshake_server(&mut stream, theEC).await;

    if the_peer_public_key.is_empty() || the_shared_key.is_empty() {
        eprintln!("Handshake failed.");
        return;
    }

    dprint!("✅ Connection created, handshake passed ok.");

    let buffer = [0u8; myDefs::STCP_IPv4_PACKET_MAX_SIZE];

    let mut loopCount: i32 = 1;

    dprint!("PSK: {} // {:?} //", the_shared_key.len(), the_shared_key);

    loop {
        loopCount += 1;
        dprint!("---------------------------------------------------------------------------------------");

        let (bytes_in, the_raw_data): (usize, Vec<u8>) = theUtilit.read_from(&mut stream).await;

        dprint!("Received crypted: {} // {} //{:?} ", bytes_in, the_raw_data.len(), the_raw_data);
        let decrypted_message_in = theAES.handle_aes_incoming_message(&the_raw_data, &the_shared_key);


	// 🔁 Kutsu C:hen
        dprint!("🔁 Rust => C: calling callback with '{:?}'", decrypted_message_in);



        let mut output_buf = [0u8; myDefs::STCP_IPv4_PACKET_MAX_SIZE];
        let mut actual_len: usize = 0;

        // 🔁 Käytä server.callback:ia
        let response = unsafe {
            (server_cb)(
                decrypted_message_in.as_ptr(),
                decrypted_message_in.len(),
                output_buf.as_mut_ptr(),
                output_buf.len(),
                &mut actual_len as *mut usize,
            );
            output_buf[..actual_len].to_vec()
        };

        let str_in_plain =  String::from_utf8_lossy(&response);

        dprint!("🔁 C => Rust: Callback returned: '{}'", str_in_plain);

        let data_out = theAES.handle_aes_outgoing_message(&response, &the_shared_key);
        let _ = stream.write_all(&data_out).await;

    }
}
