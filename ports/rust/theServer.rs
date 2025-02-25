
use tokio::net::TcpListener;
use tokio::net::TcpStream;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use std::str;
use stcpCommon::aes_lib as myAES;
use stcpCommon::stcp_elliptic_codec as myEC;
use stcpCommon::utils as myUtils;


#[macro_export]
macro_rules! dprint {
    ($($arg:tt)*) => {
        println!(
            "📍 [{}:{}] {}",
            file!(),
            line!(),
            format!($($arg)*)
        );
    };
}


async fn handle_client_connection(mut stream: TcpStream) {

    let theEC: myEC::StcpEllipticCodec = myEC::StcpEllipticCodec::new();
    let theAES: myAES::StcpAesCodec = myAES::StcpAesCodec::new();
    let theUtilit: myUtils::StcpUtils = myUtils::StcpUtils::new();

    let mut the_shared_key: Vec<u8> = Vec::new();
    let mut the_peer_public_key: Vec<u8> = Vec::new();

    let (the_peer_public_key, the_shared_key) = theUtilit.do_the_stcp_handshake_server(&mut stream, theEC).await;

    if the_peer_public_key.is_empty() || the_shared_key.is_empty() {
        eprintln!("Handshake epäonnistui!");
        return;
    } else {
        dprint!("✅ Connection created, handshake passed ok.");
    }

    let mut buffer = [0; 1024*4];

    let mut loopCount: i32 = 1;
    dprint!("PSK: {} // {:?} //", the_shared_key.len(), the_shared_key);

    while  loopCount < 5 {
        loopCount += 1;
        dprint!("---------------------------------------------------------------------------------------");
        let (bytes_in, the_raw_data) = theUtilit.read_from(&mut stream).await;
        dprint!("Received crypted: {} // {} //{:?} ", bytes_in, the_raw_data.len(), the_raw_data);
        let decrypted_message_in = theAES.handle_aes_incoming_message(&the_raw_data, &the_shared_key);
        let str_in_plain =  String::from_utf8_lossy(&decrypted_message_in);
        dprint!("Received decrypted: {} // {} //{:?} ", str_in_plain.len(), str_in_plain, decrypted_message_in);

        let output_str = format!("[Server response {}] Moro!", loopCount);
        let output = output_str.as_bytes().to_vec();

        let data_out = theAES.handle_aes_outgoing_message(&output, &the_shared_key);
        stream.write_all(&data_out).await;

    }
}

#[tokio::main]
async fn main() -> std::io::Result<()> {
    let listener = TcpListener::bind("127.0.0.1:8080").await?;
    dprint!("TCP server listening on port 8080...");

    loop {
        let (socket, addr) = listener.accept().await?;
        dprint!("New connection from: {}", addr);
        tokio::spawn(handle_client_connection(socket));
    }
}
