
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpStream;
use std::str;

use stcpCommon::{aes_lib as myAES, dprint};
use stcpCommon::stcp_elliptic_codec as myEC;
use stcpCommon::utils as myUtils;

#[tokio::main]
async fn main() {

    let theEC: myEC::StcpEllipticCodec = myEC::StcpEllipticCodec::new();
    let theAES: myAES::StcpAesCodec = myAES::StcpAesCodec::new();
    let theUtilit: myUtils::StcpUtils = myUtils::StcpUtils::new();

    // Yhdistä palvelimeen
    let mut stream = TcpStream::connect("127.0.0.1:8080").await.unwrap();
    println!("Connected to server!");

    let mut the_shared_key: Vec<u8> = Vec::new();
    let mut the_peer_public_key: Vec<u8> = Vec::new();
    
    let (the_peer_public_key, the_shared_key) = theUtilit.do_the_stcp_handshake_server(&mut stream, theEC).await;

    if the_peer_public_key.is_empty() || the_shared_key.is_empty() {
        eprintln!("Handshake epäonnistui!");
        return;
    } else {
        dprint!("✅ Connection created, handshake passed ok.");
    }

    let mut loopCount: i32 = 1;
    dprint!("PSK: {} // {:?} //", the_shared_key.len(), the_shared_key);

    while  loopCount < 5 {
        loopCount += 1;
        dprint!("---------------------------------------------------------------------------------------");

        let output_str = format!("[Client response {}] Tervehdystä??", loopCount);
        let output = output_str.as_bytes().to_vec();

        let data_out = theAES.handle_aes_outgoing_message(&output.to_vec(), &the_shared_key);
        stream.write_all(&data_out).await;

        let (bytes_in, the_raw_data) = theUtilit.read_from(&mut stream).await;
        let str_in_crypt =  String::from_utf8_lossy(&the_raw_data);
        dprint!("Received raw: {} // {} //{:?} ", bytes_in, str_in_crypt, the_raw_data);
        let decrypted_message_in = theAES.handle_aes_incoming_message(&the_raw_data, &the_shared_key);
        let str_in_plain =  String::from_utf8_lossy(&decrypted_message_in);
        dprint!("Received decrypted: {} // {} //{:?} ", str_in_plain.len(), str_in_plain, decrypted_message_in);
        
    }
    /*

        match stream.read(&mut buffer).await {
            Ok(0) => {
                println!("Connection closed");
                return;
            }
            Ok(nBytes) => {
                let response = theAES.handle_aes_incoming_message(&buffer, nBytes, &the_shared_key);
                // Process message
                let out_str = format!("[Client {} msg]: MORO", nLen);
                let resp_out = out_str.as_bytes().to_vec();
                dprint!("[Client] responding: // {} //", out_str);
                let output = theAES.handle_aes_outgoing_message(&resp_out, &the_shared_key);
                dprint!("Client sending: {} // {:?}", output.len(), output);
                stream.write_all(&output).await;
            },
            Err(e) => {
                eprintln!("Read error: {}", e);
                return;
            }
        };
    */

}
