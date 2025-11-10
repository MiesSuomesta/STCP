#![allow(dead_code)]

extern crate alloc;
use alloc::vec;
use alloc::vec::Vec;
use core::ffi::c_int;

use stcp_core::crypto::{encrypt_aes256_cbc_random_iv, decrypt_aes256_cbc};
use stcp_core::crypto::CryptoError; 
use crate::error::StcpError;

fn crypto_error_to_error(_e: CryptoError) -> c_int {
    // TODO: tarkempi mapping. Nyt vain yleinen -EIO.
    -5
}

/// IO-kerroksen virhetyyppi: negatiivinen error
pub type IoResult<T> = core::result::Result<T, c_int>;

/// Abstrakti IO-rajapinta, jonka päälle salaus kytketään.
/// Toteutus tehdään Rustissa C:n tarjoamien funkkareiden päälle,
/// mutta C-koodia ei tarvitse muuttaa.
pub trait StcpIo {
    fn send(&mut self, data: &[u8]) -> IoResult<usize>;
    fn recv(&mut self, buf: &mut [u8]) -> IoResult<usize>;

    fn recv_exact(&mut self, buf: &mut [u8]) -> IoResult<()> {
        let mut read = 0;
        while read < buf.len() {
            let n = self.recv(&mut buf[read..])?;
            if n == 0 {
                // EOF / yhteys poikki
                return Err(-1);
            }
            read += n;
        }
        Ok(())
    }
}

fn stcp_error_to_error(_e: StcpError) -> c_int {
    // TODO: map StcpError -> -error tarkasti
    -1
}

/// Lähetä yksi salattu viesti: header (len) + ciphertext.
/// Tästä voi kutsua StcpContext, kun AES on kytketty.
pub fn send_encrypted(
    io: &mut impl StcpIo,
    aes_key: &[u8; 32],
    plaintext: &[u8],
) -> IoResult<()> {
    let enc = encrypt_aes256_cbc_random_iv(aes_key, plaintext)
        .map_err(crypto_error_to_error)?;

    let len = enc.len() as u32;
    let header = len.to_be_bytes();

    io.send(&header)?;
    io.send(&enc)?;
    Ok(())
}

/// Vastaanota yksi salattu viesti ja palauta dekryptattu payload.
pub fn recv_encrypted(
    io: &mut impl StcpIo,
    aes_key: &[u8; 32],
) -> IoResult<Vec<u8>> {
    let mut header = [0u8; 4];
    io.recv_exact(&mut header)?;
    let len = u32::from_be_bytes(header) as usize;

    let mut buf = vec![0u8; len];
    io.recv_exact(&mut buf)?;

    let plain = decrypt_aes256_cbc(aes_key, &buf)
        .map_err(crypto_error_to_error)?;

    Ok(plain)
}
