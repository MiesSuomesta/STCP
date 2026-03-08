use aes_gcm::{
    aead::{Aead, KeyInit},
    Aes256Gcm, Nonce
};

use alloc::vec::Vec;



const IV_LEN: usize = 12;

unsafe extern "C" {
    pub fn stcp_random_get(buf: *mut u8, len: usize);
}

#[repr(C)]
pub struct StcpAesCodec {
    cipher: Aes256Gcm,
}

impl StcpAesCodec {

    pub fn new(key: &[u8]) -> Self {
        Self {
            cipher: Aes256Gcm::new_from_slice(key).unwrap(),
        }
    }

    pub fn generate_iv(&self) -> [u8; 12] {

        let mut iv = [0u8; 12];
        unsafe {
            stcp_random_get(iv.as_mut_ptr(), iv.len());
        }
        iv
    }


    pub fn encrypt(
        &self,
        plaintext: &[u8],
    ) -> Vec<u8> {

        let iv: [u8; 12] = self.generate_iv();
        let nonce = Nonce::from_slice(&iv);

        let ciphertext = self.cipher
            .encrypt(nonce, plaintext)
            .expect("encrypt failed");

        let mut output = Vec::with_capacity(12 + ciphertext.len());

        output.extend_from_slice(&iv);
        output.extend_from_slice(&ciphertext);

        output
    } 

    pub fn decrypt(&self, payload: &[u8]) -> Option<Vec<u8>> {
        
        if payload.len() < IV_LEN {
            return None;
        }

        let iv = &payload[..IV_LEN];
        let ciphertext = &payload[IV_LEN..];

        let nonce = Nonce::from_slice(iv);

        self.cipher.decrypt(nonce, ciphertext).ok()
    }
}
