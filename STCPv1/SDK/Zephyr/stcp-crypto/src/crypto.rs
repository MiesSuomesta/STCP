
use stcpdefines::defines::*;
use stcptypes::types::*;

use aes_gcm::{
    Aes256Gcm,
    aead::{Aead, KeyInit, generic_array::GenericArray},
};

/// Salaa datan käyttäen AES-256-GCM:ää.
/// Palauttaa `Some(encrypted_data)` jos onnistuu, muuten `None`.
pub fn stcp_encrypt(key: &[u8; STCP_AES_KEY_SIZE_IN_BYTES], nonce: &[u8; STCP_AES_NONCE_SIZE_IN_BYTES], plaintext: &[u8]) -> Option<heapless::Vec<u8, 1024>> {

    let cipher = Aes256Gcm::new(GenericArray::from_slice(key));
    let nonce = GenericArray::from_slice(nonce);
    let result = cipher.encrypt(nonce, plaintext).ok()?;

    let mut output = heapless::Vec::new();
    output.extend_from_slice(&result).ok()?;
    Some(output)
}

/// Purkaa AES-256-GCM:llä salatun datan.
/// Palauttaa `Some(plaintext)` jos onnistuu, muuten `None`.
pub fn stcp_decrypt(key: &[u8; STCP_AES_KEY_SIZE_IN_BYTES], nonce: &[u8; STCP_AES_NONCE_SIZE_IN_BYTES], ciphertext: &[u8]) -> Option<heapless::Vec<u8, 1024>> {

    let cipher = Aes256Gcm::new(GenericArray::from_slice(key));
    let nonce = GenericArray::from_slice(nonce);
    let result = cipher.decrypt(nonce, ciphertext).ok()?;

    let mut output = heapless::Vec::new();
    output.extend_from_slice(&result).ok()?;
    Some(output)
}
