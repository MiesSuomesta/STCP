extern crate alloc;
use alloc::format;
use alloc::vec::Vec;
use alloc::boxed::Box;

use core::ptr;

use heapless::String;

use stcpdefines::defines::*;
use stcptypes::types::*;

use iowrapper::stream::StcpStream;
use iowrapper::types::{ServerThreadContext};

use spin::Mutex;

use stcpcrypto::aes_lib::StcpAesCodec;
use stcpcrypto::stcp_elliptic_codec::StcpEllipticCodec;

use iowrapper::handshake::StcpUtils;

use utils;

#[macro_use]
use debug;

pub fn handle_client_connection(mut stream: Mutex<StcpStream>, server_cb: ServerMessageProcessCB) {

    let theEC = StcpEllipticCodec::New();
    let theAES = StcpAesCodec::new();
    let theUtilit = StcpUtils::new();

    let the_shared_key: Vec<u8> = Vec::new();
    let the_peer_public_key: Vec<u8> = Vec::new();

    let mut guard_a = stream.lock();
    let result_hs = theUtilit.do_the_stcp_handshake_server(&mut guard_a, theEC);
    let (the_peer_public_key, the_shared_key) = result_hs;

    if the_peer_public_key.is_empty() || the_shared_key.is_empty() {
        dbg!("Handshake failed.");
        return;
    }

    dbg!("✅ Connection created, handshake passed ok.");

    let mut loopCount: i32 = 1;

    dbg!("PSK: {} // {:?} //", the_shared_key.len(), &the_shared_key);

    loop {
        loopCount += 1;
        dbg!("---------------------------------------------------------------------------------------");

	let mut guard_b = stream.lock();

        let result = theUtilit.read_from(&mut guard_b);
        let (bytes_in, the_raw_data): (usize, Vec<u8>) = result;

        dbg!("Received crypted: {} // {} //{:?} ", bytes_in, the_raw_data.len(), &the_raw_data);
        let decrypted_message_in = theAES.handle_aes_incoming_message(&the_raw_data, &the_shared_key);

	// 🔁 Kutsu C:hen
        dbg!("🔁 Rust => C: calling callback with '{:?}'", &decrypted_message_in);

        let mut output_buf = [0u8; STCP_IPv4_PACKET_MAX_SIZE];
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

        dbg!("🔁 C => Rust: Callback returned: '{:?}'", response);

        let data_out = theAES.handle_aes_outgoing_message(&response, &the_shared_key);

        if let Err(e) = stream.lock().write_all(&data_out) {
            dbg!("Write failed: {:?}", e);
            break;
        }	


    } // Loop end
}
