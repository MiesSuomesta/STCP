use std::str;
use openssl::rand::rand_bytes;
use openssl::symm::{Cipher, Crypter, Mode};
use rand_core::le;
use crate::defines_etc::*;
use crate::dprint;

const STCP_AES_KEY_SIZE_IN_BYTES: usize = 32;
const STCP_AES_IV_SIZE_IN_BYTES: usize = 16;

pub struct StcpAesCodec {}

impl StcpAesCodec {
    pub fn new() -> Self {
        Self {}
    }

    pub fn the_secure_message_transfer_incoming(&self, msg_incoming_crypted: &Vec<u8>, the_aes_preshared_key: &Vec<u8>) -> Vec<u8> {
        // Luodaan AES-avain (täytetään 32 tavuun)
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
        rand_bytes(&mut the_outgoing_iv).expect("Failed to generate IV");

        let crypted_message = self.encrypt(msg_outgoing_plain, &the_aes_key, &the_outgoing_iv);

        // IV + salattu viesti
        let mut final_message = the_outgoing_iv.to_vec();
        final_message.extend_from_slice(&crypted_message);
        final_message
    }

    fn encrypt(&self, plaintext: &[u8], key: &[u8], iv: &[u8]) -> Vec<u8> {
        let cipher = Cipher::aes_256_cbc();
        let mut encrypter = Crypter::new(cipher, Mode::Encrypt, key, Some(iv)).unwrap();
        encrypter.pad(false);

        let padded_plaintext = self.pkcs7_pad(plaintext, cipher.block_size());

        let mut encrypted = vec![0; padded_plaintext.len() + cipher.block_size()];
        let count = encrypter.update(&padded_plaintext, &mut encrypted).unwrap();
        let rest = encrypter.finalize(&mut encrypted[count..]).unwrap();
        encrypted.truncate(count + rest);
        encrypted
    }

    fn decrypt(&self, ciphertext: &[u8], key: &[u8], iv: &[u8]) -> Vec<u8> {
        let cipher = Cipher::aes_256_cbc();
        let mut decrypter = Crypter::new(cipher, Mode::Decrypt, key, Some(iv)).unwrap();
        decrypter.pad(false);

        let mut decrypted = vec![0; ciphertext.len() + cipher.block_size()];
        let count = decrypter.update(ciphertext, &mut decrypted).unwrap();
        let rest = decrypter.finalize(&mut decrypted[count..]).unwrap();
        decrypted.truncate(count + rest);

        self.pkcs7_unpad(&decrypted)
    }

    fn pkcs7_pad(&self, data: &[u8], block_size: usize) -> Vec<u8> {
        let pad_len = block_size - (data.len() % block_size);
        let mut padded = data.to_vec();
        padded.extend(vec![pad_len as u8; pad_len]);
        padded
    }

    fn pkcs7_unpad(&self, data: &[u8]) -> Vec<u8> {
        if data.is_empty() {
            return vec![];
        }
        let pad_len = *data.last().unwrap() as usize;
        if pad_len == 0 || pad_len > data.len() {
            return vec![];
        }
        data[..data.len() - pad_len].to_vec()
    }    pub fn derive_shared_key_based_aes_key(&self, common_secret: &[u8]) -> Vec<u8> {
        let mut aes_key = [0; STCP_AES_KEY_SIZE_IN_BYTES];
        let mut common_secret_bytes = common_secret.to_vec();
        common_secret_bytes.resize(STCP_AES_KEY_SIZE_IN_BYTES, 0);
        aes_key.copy_from_slice(&common_secret_bytes);
        aes_key.to_vec()
    }

    pub fn handle_aes_incoming_message(
        &self,
        data: &Vec<u8>,
        the_shared_key: &Vec<u8>,
    ) -> Vec<u8> {
// PSK pois debugeista 
//         dprint!("PSK: {} // {:?} //", the_shared_key.len(), the_shared_key);
        println!("Received AES: {} // {:?}", data.len(), data);
        let the_incoming: Vec<u8> = self.the_secure_message_transfer_incoming(data, the_shared_key);
// PSK pois debugeista 
//         dprint!("PSK: {} // {:?} //", the_shared_key.len(), the_shared_key);
        println!("Incoming decrypted message: {} // {:?}", the_incoming.len(), the_incoming);
        the_incoming
    }

    pub fn handle_aes_outgoing_message(
        &self,
        data: &Vec<u8>,
        the_shared_key: &Vec<u8>,
    ) -> Vec<u8> {
// PSK pois debugeista 
//         dprint!("PSK: {} // {:?} //", the_shared_key.len(), the_shared_key);
        let resp = self.the_secure_message_transfer_outgoing(
            data,
            the_shared_key,
        );
// PSK pois debugeista 
//         dprint!("PSK: {} // {:?} //", the_shared_key.len(), the_shared_key);
        println!("Response crypted: {:?}", resp);
        resp
    }


}
