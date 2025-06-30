extern crate alloc;
use alloc::vec::Vec;
use alloc::boxed::Box;

use aes_gcm::{Aes256Gcm, aead::{Aead, KeyInit, generic_array::GenericArray}};

use stcpdefines::defines::*;
use stcptypes::types::*;
//use crate::macros::*;

pub struct StcpAesCodec {}

impl StcpAesCodec {
    pub fn new() -> Self {
        Self {}
    }

    pub fn the_secure_message_transfer_incoming(&self, msg_incoming_crypted: &Vec<u8>, the_aes_preshared_key: &Vec<u8>) -> Vec<u8> {
        // Luodaan AES-avain (täytetään 32 tavuun)
        if msg_incoming_crypted.len() < STCP_AES_KEY_SIZE_IN_BYTES {
            
            //deprintln!("Lenght check fails! {} bytes in, expected to be at least {} bytes!",
                /*msg_incoming_crypted.len(),
                STCP_AES_KEY_SIZE_IN_BYTES);)*/

            return Vec::new();
        }

        let mut the_aes_key = [0u8; STCP_AES_KEY_SIZE_IN_BYTES];
        let key_bytes = the_aes_preshared_key;
        let len = key_bytes.len().min(STCP_AES_KEY_SIZE_IN_BYTES);
        the_aes_key[..len].copy_from_slice(&key_bytes[..len]);

        let the_incoming_iv = &msg_incoming_crypted[..STCP_AES_IV_SIZE_IN_BYTES];
        let the_encrypted_message = &msg_incoming_crypted[STCP_AES_IV_SIZE_IN_BYTES..];

        self.decrypt(the_encrypted_message, &the_aes_key, the_incoming_iv)
    }

    pub fn the_secure_message_transfer_outgoing(&self, msg_outgoing_plain: &Vec<u8>, the_aes_preshared_key: &Vec<u8>) -> Vec<u8> {
        let mut the_aes_key = [0u8; STCP_AES_KEY_SIZE_IN_BYTES];
        let key_bytes = the_aes_preshared_key;
        let len = key_bytes.len().min(STCP_AES_KEY_SIZE_IN_BYTES);
        the_aes_key[..len].copy_from_slice(&key_bytes[..len]);

        // Luodaan satunnainen IV
        let mut the_outgoing_iv = [0u8; STCP_AES_IV_SIZE_IN_BYTES];
        Self::fill_random_bytes(&mut the_outgoing_iv);

        let crypted_message = self.encrypt(msg_outgoing_plain, &the_aes_key, &the_outgoing_iv);

        // IV + salattu viesti
        let mut final_message = the_outgoing_iv.to_vec();
        final_message.extend_from_slice(&crypted_message);
        final_message
    }

    fn encrypt(&self, plaintext: &[u8], key: &[u8], iv: &[u8]) -> Vec<u8> {

        let key_array = match key.try_into() {
            Ok(k) => GenericArray::from_slice(k),
            Err(_) => return Vec::new(),
        };

        let nonce = match iv.try_into() {
            Ok(n) => GenericArray::from_slice(n),
            Err(_) => return Vec::new(),
        };

        let cipher = Aes256Gcm::new(key_array);
        cipher.encrypt(nonce, plaintext).unwrap_or_else(|_| Vec::new())
    }

    fn decrypt(&self, ciphertext: &[u8], key: &[u8], iv: &[u8]) -> Vec<u8> {

        let key_array = match key.try_into() {
            Ok(k) => GenericArray::from_slice(k),
            Err(_) => return Vec::new(),
        };

        let nonce = match iv.try_into() {
            Ok(n) => GenericArray::from_slice(n),
            Err(_) => return Vec::new(),
        };

        let cipher = Aes256Gcm::new(key_array);
        cipher.decrypt(nonce, ciphertext).unwrap_or_else(|_| Vec::new())
    }

    pub fn handle_aes_incoming_message(
        &self,
        data: &Vec<u8>,
        the_shared_key: &Vec<u8>,
    ) -> Vec<u8> {
        //dprintln!("Received AES: {} // {:?}", data.len(), data);
        let the_incoming: Vec<u8> = self.the_secure_message_transfer_incoming(data, the_shared_key);
        //dprintln!("Incoming decrypted message: {} // {:?}", the_incoming.len(), the_incoming);
        the_incoming
    }

    pub fn handle_aes_outgoing_message(
        &self,
        data: &Vec<u8>,
        the_shared_key: &Vec<u8>,
    ) -> Vec<u8> {
        let resp = self.the_secure_message_transfer_outgoing(
            data,
            the_shared_key,
        );
        //dprintln!("Response crypted: {:?}", resp);
        resp
    }

    pub fn fill_random_bytes(buf: &mut [u8]) {
        for (i, b) in buf.iter_mut().enumerate() {
            *b = (i as u8).wrapping_mul(73) ^ 0xA5;
        }
    }


}
