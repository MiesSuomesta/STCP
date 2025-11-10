#![allow(dead_code)]
#![allow(deprecated)]
extern crate alloc;
use alloc::vec::Vec;

use aes_gcm::{
    aead::{AeadInPlace, Error as AeadError, KeyInit},
    Aes256Gcm,
    Key,
    Nonce,
};

use rand_core::{RngCore, Error as RandError};
use x25519_dalek::{x25519, X25519_BASEPOINT_BYTES};

use crate::types::StcpCtx;

/// AES-GCM parametrit
pub const STCP_KEY_LEN: usize = 32;
pub const STCP_IV_LEN: usize = 12;   // Aes256Gcm = 96-bit nonce
pub const STCP_TAG_LEN: usize = 16;  // GCM tag

/// Yksinkertainen, ei-krypto RNG smoketestiä varten.
/// VAIHDA myöhemmin kernelin oikeaan RNG:hen.
pub struct StcpTestRng {
    state: u64,
}

#[derive(Debug)]
pub enum CryptoError {
    InvalidKey,
    InvalidInput,
    EncryptFailed,
    DecryptFailed,
}

impl From<AeadError> for CryptoError {
    fn from(_: AeadError) -> Self {
        CryptoError::EncryptFailed
    }
}

/// Yhteensopivuus-funktio the_rust_implementationille:
/// Palauttaa: [iv || ciphertext]
/// Yhteensopivuus-funktio the_rust_implementationille:
/// Palauttaa: [iv || ciphertext]
///
/// HUOM: käyttää StcpTestRng:ää placeholderina. Vaihda myöhemmin kernelin
/// oikeaan RNG:hen / C-puolen randomiin.
pub fn encrypt_aes256_cbc_random_iv(
    key: &[u8; 32],
    plaintext: &[u8],
) -> Result<Vec<u8>, CryptoError> {
    let cipher = make_aes(key);

    // Placeholder-RNG: vaihda myöhemmin oikeaan kerneli RNG:hen
    let mut rng = StcpTestRng::new(0x1234_5678_9abc_def0);
    let mut iv = [0u8; STCP_IV_LEN];
    rng.fill_bytes(&mut iv);

    let nonce = Nonce::from_slice(&iv);

    // ciphertext buffer
    let mut buf = Vec::with_capacity(plaintext.len());
    buf.extend_from_slice(plaintext);

    // encrypt in-place, tag erikseen
    let tag = cipher
        .encrypt_in_place_detached(nonce, &[], &mut buf)
        .map_err(|_| CryptoError::EncryptFailed)?;

    // [iv][cipher][tag]
    let mut out = Vec::with_capacity(STCP_IV_LEN + buf.len() + STCP_TAG_LEN);
    out.extend_from_slice(&iv);
    out.extend_from_slice(&buf);
    out.extend_from_slice(tag.as_slice());

    Ok(out)
}

/// Purkaa [iv || ciphertext]
pub fn decrypt_aes256_cbc(
    key: &[u8; 32],
    data: &[u8],
) -> Result<Vec<u8>, CryptoError> {
    if data.len() < STCP_IV_LEN + STCP_TAG_LEN {
        return Err(CryptoError::InvalidInput);
    }

    let (iv, rest) = data.split_at(STCP_IV_LEN);
    let (ct, tag_bytes) = rest.split_at(rest.len() - STCP_TAG_LEN);

    let mut buf = Vec::with_capacity(ct.len());
    buf.extend_from_slice(ct);

    let cipher = make_aes(key);
    let nonce = Nonce::from_slice(iv);

    use aes_gcm::Tag;
    let mut tag_arr = [0u8; STCP_TAG_LEN];
    tag_arr.copy_from_slice(tag_bytes);
    let tag = Tag::from(tag_arr);

    cipher
        .decrypt_in_place_detached(nonce, &[], &mut buf, &tag)
        .map_err(|_| CryptoError::DecryptFailed)?;

    Ok(buf)
}


impl StcpTestRng {
    pub const fn new(seed: u64) -> Self {
        Self { state: seed }
    }
}

impl RngCore for StcpTestRng {
    fn next_u32(&mut self) -> u32 {
        (self.next_u64() >> 32) as u32
    }

    fn next_u64(&mut self) -> u64 {
        // yksinkertainen xorshift
        let mut x = self.state;
        x ^= x << 7;
        x ^= x >> 9;
        x ^= x << 8;
        self.state = x;
        x
    }

    fn fill_bytes(&mut self, dest: &mut [u8]) {
        let _ = self.try_fill_bytes(dest);
    }

    fn try_fill_bytes(&mut self, dest: &mut [u8]) -> Result<(), RandError> {
        for chunk in dest.chunks_mut(8) {
            let v = self.next_u64();
            let bytes = v.to_le_bytes();
            let n = chunk.len();
            chunk.copy_from_slice(&bytes[..n]);
        }
        Ok(())
    }
}

/// x25519-avainpari: paljaat 32-tavuiset arvot
pub struct StcpKeys {
    pub secret: [u8; 32],
    pub public: [u8; 32],
}

fn clamp_scalar(s: &mut [u8; 32]) {
    // RFC7748 key clamp
    s[0] &= 248;
    s[31] &= 127;
    s[31] |= 64;
}

impl StcpKeys {
    pub fn generate_with<R: RngCore>(rng: &mut R) -> Self {
        let mut sk = [0u8; 32];
        rng.fill_bytes(&mut sk);
        clamp_scalar(&mut sk);

        let pk = x25519(sk, X25519_BASEPOINT_BYTES);

        Self { secret: sk, public: pk }
    }
}

/// Laske jaettu salaisuus puhtaalla x25519:llä.
pub fn derive_shared(secret: &[u8; 32], peer_pub: &[u8; 32]) -> [u8; 32] {
    x25519(*secret, *peer_pub)
}

/// Luo AES-GCM konteksti annetulla 256-bit avaimella.
fn make_aes(key: &[u8; STCP_KEY_LEN]) -> Aes256Gcm {
    let k = Key::<Aes256Gcm>::from_slice(key);
    Aes256Gcm::new(k)
}

/// Salaa `plaintext` → kirjoittaa `out_buf`iin: [cipher || tag],
/// käyttää `out_iv` nonceen. Ei allokoi heapista.
pub fn encrypt_payload(
    _ctx: &mut StcpCtx,
    key: &[u8; STCP_KEY_LEN],
    plaintext: &[u8],
    out_iv: &mut [u8; STCP_IV_LEN],
    out_buf: &mut [u8],
    rng: &mut impl RngCore,
) -> Result<usize, i32> {
    if plaintext.len() + STCP_TAG_LEN > out_buf.len() {
        return Err(-12); // -ENOMEM
    }

    rng.try_fill_bytes(out_iv).map_err(|_| -5)?;

    // kopioi plaintext ulos bufferiin
    out_buf[..plaintext.len()].copy_from_slice(plaintext);

    let cipher = make_aes(key);
    let nonce = Nonce::from_slice(out_iv);

    // in-place + detached tag
    let tag = cipher
        .encrypt_in_place_detached(nonce, &[], &mut out_buf[..plaintext.len()])
        .map_err(|_| -5)?;

    let tag_bytes = tag.as_slice();
    out_buf[plaintext.len()..plaintext.len() + STCP_TAG_LEN]
        .copy_from_slice(tag_bytes);

    Ok(plaintext.len() + STCP_TAG_LEN)
}

/// Purkaa [cipher || tag] muodossa olevan viestin.
pub fn decrypt_payload(
    key: &[u8; STCP_KEY_LEN],
    iv: &[u8; STCP_IV_LEN],
    ciphertext_and_tag: &[u8],
    out_plain: &mut [u8],
) -> Result<usize, i32> {
    if ciphertext_and_tag.len() < STCP_TAG_LEN {
        return Err(-22); // -EINVAL
    }

    let ct_len = ciphertext_and_tag.len() - STCP_TAG_LEN;
    if ct_len > out_plain.len() {
        return Err(-12); // -ENOMEM
    }

    let (ct, tag_bytes) = ciphertext_and_tag.split_at(ct_len);

    // kopioi cipher out_plainiin, puretaan siihen päälle
    out_plain[..ct_len].copy_from_slice(ct);

    let cipher = make_aes(key);
    let nonce = Nonce::from_slice(iv);

    use aes_gcm::Tag;
    let mut tag_array = [0u8; STCP_TAG_LEN];
    tag_array.copy_from_slice(tag_bytes);
    let tag = Tag::from(tag_array);

    cipher
        .decrypt_in_place_detached(nonce, &[], &mut out_plain[..ct_len], &tag)
        .map_err(|_| -5)?;

    Ok(ct_len)
}
