use std::time::Duration;

use tokio::net::*;
use tokio::io::*;

use crate::aes_lib as myAES;
use crate::stcp_elliptic_codec as myEC;
use crate::utils as myUtils;
use crate::dprint;

use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::time::sleep;

use tokio::net::TcpStream;

pub struct StcpUtils;

impl StcpUtils {
    pub fn new() -> Self {
        Self { }
    }

    pub fn print_out(&self, text_name:String, text_src:Vec<u8>) {
        let text = String::from_utf8_lossy(&text_src);
        dprint!("{} ({} bytes): {}", text_name, text_src.len(), text);
    }

    pub async fn read_from_functionality(&self, theStream: &mut TcpStream) -> std::io::Result<Option<Vec<u8>>> {
        let mut buffer = [0; 1024];
    
        match theStream.read(&mut buffer).await {
            Ok(0) => {
                dprint!("Connection closed");
                Ok(None)
            }
            Ok(nBytes) => {
                let received = buffer[..nBytes].to_vec(); // Kopioidaan data uuteen Vec<u8>
                Ok(Some(received))
            }            Err(e) => {
                eprintln!("Read error: {}", e);
                Err(e) // Palautetaan std::io::Error sellaisenaan
            }
        }
    }
        
    pub async fn read_from(&self, theStream: &mut TcpStream) -> (usize, Vec<u8>) {
        match self.read_from_functionality(theStream).await {
            Ok(Some(data)) => (data.len(), data), // Palautetaan luettu data ja sen koko
            Ok(None) => {
                dprint!("Connection closed by peer.");
                (0, Vec::new()) // Tyhjä data ja koko 0
            }
            Err(e) => {
                eprintln!("Error reading from stream: {}", e);
                (0, Vec::new()) // Tyhjä data ja koko 0 virheen tapauksessa
            }
        }
    }

    

    pub async fn do_the_stcp_handshake_server(
        &self, 
        theStream: &mut TcpStream,  // Käytetään viitettä
        mut theEC: myEC::StcpEllipticCodec
    ) -> (Vec<u8>, Vec<u8>) {  // Palautetaan ilman TcpStreamia
        let mut buffer = [0; 1024];
        let mut tries = 15;
        loop {
            tries -= 1;
            if tries < 1 {
                return (vec![],vec![]);
            }

            let theFirstPK = theEC.my_public_key_to_raw_bytes();
            dprint!("Sending my public key. Len: {}", theFirstPK.len());
            theStream.write_all(&theFirstPK).await.unwrap();
            let (responseLen, response) = self.read_from(theStream).await;
            self.print_out("recv in".to_string(), response.clone());
            if responseLen == 65 {
                dprint!("Got message.. checking it..");
                let (thepubkey, thesharedkey) = theEC.check_for_public_key(Some(&response)).unwrap();
                let gotPK = !thepubkey.is_empty();
                let gotSK = !thesharedkey.is_empty();
                dprint!("Got valid: {} // {:?}", thesharedkey.len(), thesharedkey);
            
                if gotPK && gotSK {
                    dprint!("Got valid PublicKey from peer, shared key set: {} // {:?}", thesharedkey.len(), thesharedkey);
                    dprint!("Got PSK: {} // {:?}", thesharedkey.len(), thesharedkey);
                    return (thepubkey, thesharedkey);
                }
            }
        }
    }

    pub async fn do_the_stcp_handshake_client(
        &self, 
        theStream: &mut TcpStream,  // Käytetään viitettä
        mut theEC: myEC::StcpEllipticCodec
    ) -> (Vec<u8>, Vec<u8>) {  // Palautetaan ilman TcpStreamia
        let mut buffer = [0; 1024];

        let mut tries = 15;
        loop {
            tries -= 1;
            if tries < 1 {
                return (vec![],vec![]);
            }

            let theFirstPK = theEC.my_public_key_to_raw_bytes();
            dprint!("Sending my public key. Len: {}", theFirstPK.len());
            theStream.write_all(&theFirstPK).await.unwrap();
            let (responseLen, response) = self.read_from(theStream).await;
            self.print_out("recv in".to_string(), response.clone());
            dprint!("Got message.. checking it..");
            let (thepubkey, thesharedkey) = theEC.check_for_public_key(Some(&response)).unwrap();
            let gotPK = !thepubkey.is_empty();
            let gotSK = !thesharedkey.is_empty();
            dprint!("Got valid? {} // {}", gotPK, gotSK);
            if gotPK && gotSK {
                dprint!("Got valid PublicKey from peer, shared key set.");
                dprint!("Got PSK: {} // {:?}", thesharedkey.len(), thesharedkey);
                return (thepubkey, thesharedkey);
            }
        }
    }
}