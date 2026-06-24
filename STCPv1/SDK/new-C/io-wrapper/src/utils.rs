#![cfg_attr(not(feature = "std"), no_std)]

extern crate alloc;
use alloc::vec::Vec;
use alloc::boxed::Box;

use core::cmp::min;
use core::str;
use core::fmt::Write;
use heapless::String;
use core::net::{IpAddr, SocketAddr};
use stcpdefines::defines::*;
use stcptypes::types::*;

use stcpcrypto::stcp_elliptic_codec::StcpEllipticCodec;
use stcpcrypto::stcp_elliptic_codec as myEC;

//
use crate::listener::StcpListener;
use crate::stream::StcpStream;

pub struct StcpUtils;

impl StcpUtils {
    pub fn new() -> Self {
        Self { }
    }

    pub fn read_from_functionality(&self, theStream: &mut StcpStream) -> StcpResult<Option<Vec<u8>>> {
        let mut buffer = [0; STCP_IPv4_PACKET_MAX_SIZE];
        match theStream.read(&mut buffer) {
            Ok(0) => {
                //dprint!("Connection closed");
                Ok(None)
            }
            Ok(nBytes) => {
                let received = buffer[..nBytes].to_vec(); // Kopioidaan data uuteen Vec<u8>
                Ok(Some(received))
            }
            Err(_) => {
                Err(StcpError::ReadFailed) // Palautetaan std::io::Error sellaisenaan
            }
        }
    }

    pub fn read_from(&self, theStream: &mut StcpStream) -> (usize, Vec<u8>) {
        match self.read_from_functionality(theStream) {
            Ok(Some(data)) => (data.len(), data as Vec<u8>), // Palautetaan luettu data ja sen koko
            Ok(None) => {
                //dprint!("Connection closed by peer.");
                (0, Vec::new()) // Tyhjä data ja koko 0
            }
            Err(_) => {
                //deprintln!("Error reading from stream");
                (0, Vec::new()) // Tyhjä data ja koko 0 virheen tapauksessa
            }
        }
    }

    pub fn do_the_stcp_handshake_server(
        &self, 
        theStream: &mut StcpStream,  // Käytetään viitettä
        mut theEC: myEC::StcpEllipticCodec
    ) -> (Vec<u8>, Vec<u8>) {  // Palautetaan ilman TcpStreamia
        let mut buffer = [0; STCP_IPv4_PACKET_MAX_SIZE];
        let mut tries = 15;
        loop {
            tries -= 1;
            if tries < 1 {
                return (Vec::new(),Vec::new());
            }

            let theFirstPK = theEC.my_public_key_to_raw_bytes();
            //dprint!("Sending my public key. Len: {}", theFirstPK.len());
            match theStream.write_all(&theFirstPK) {
                Ok(v) => v,
                Err(e) => {
                    //dprintln!("Failed to write publickey: {:?}", e);
                    return (Vec::new(),Vec::new());
                }
            };

            let (responseLen, response) = self.read_from(theStream);
            if responseLen <= 0 {
                //dprintln!("Failed to read publickey..");
                return (Vec::new(),Vec::new());
            }

            let dataUtf = str::from_utf8(&response).unwrap();
            let mut stringi: String<STCP_IPv4_PACKET_MAX_SIZE> = String::new();
            stringi.push_str(dataUtf);

            //dprint!("recv {} bytes, data: '{:?}'", response.len(), stringi.as_str());

            if responseLen == 65 {
                //dprint!("Got message.. checking it..");
                let (thepubkey, thesharedkey) = theEC.check_for_public_key(Some(&response)).unwrap();
                let gotPK = !thepubkey.is_empty();
                let gotSK = !thesharedkey.is_empty();
                //dprint!("Got valid: {} // {:?}", thesharedkey.len(), thesharedkey);
            
                if gotPK && gotSK {
                   //dprint!("✅ Got valid PublicKey from peer, shared key set: {} // {:?}", thesharedkey.len(), thesharedkey);
                    return (thepubkey, thesharedkey);
                }
            }
        }
    }


    pub fn do_the_stcp_handshake_client(
        &self, 
        theStream: &mut StcpStream,  // Käytetään viitettä
        mut theEC: myEC::StcpEllipticCodec
    ) -> (Vec<u8>, Vec<u8>) {  // Palautetaan ilman TcpStreamia
        let mut buffer = [0; STCP_IPv4_PACKET_MAX_SIZE];

        let mut tries = 15;
        loop {
            tries -= 1;
            if tries < 1 {
                return (Vec::new(),Vec::new());
            }

            let theFirstPK = theEC.my_public_key_to_raw_bytes();
            //dprint!("Sending my public key. Len: {}", theFirstPK.len());

            match theStream.write_all(&theFirstPK) {
                Ok(v) => v,
                Err(e) => {
                    //dprintln!("Failed to write publickey: {:?}", e);
                    return (Vec::new(),Vec::new());
                }
            };

            let (responseLen, response) = self.read_from(theStream);
            if responseLen <= 0 {
                //dprintln!("Failed to read publickey..");
                return (Vec::new(),Vec::new());
            }

            //dprint!("recv len: {:?}: {:?}", responseLen, response);
            //dprint!("Got message.. checking it..");
            let (thepubkey, thesharedkey) = theEC.check_for_public_key(Some(&response)).unwrap();
            let gotPK = !thepubkey.is_empty();
            let gotSK = !thesharedkey.is_empty();
            //dprint!("Got valid? {} // {}", gotPK, gotSK);
            if gotPK && gotSK {
               // //dprint!("✅ Got valid PublicKey from peer, shared key set.");
            // PSK pois debugeista 
            // //dprint!("Got PSK: {} // {:?}", thesharedkey.len(), thesharedkey);
                return (thepubkey, thesharedkey);
            }
        }
    }
}

pub fn make_socket_addr(ip_str: &str, port: u16) -> Option<SocketAddr> {
    let ip: IpAddr = ip_str.parse().ok()?;
    Some(SocketAddr::new(ip, port))
}

pub fn fill_random_bytes(buf: &mut [u8]) {
    for (i, b) in buf.iter_mut().enumerate() {
        *b = (i as u8).wrapping_mul(73) ^ 0xA5;
    }
}

