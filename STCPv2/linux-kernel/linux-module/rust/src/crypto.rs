use alloc::vec::Vec;
use crate::error::StcpError;
use crate::packet::{STCP_IV_LEN, STCP_PUBLIC_KEY_LEN};

/* API mirrors the application CryptoContext, but backend is intentionally
 * separated. Wire this to Linux Crypto API (X25519 + ChaCha20-Poly1305/AEAD). */
pub struct CryptoContext { ready: bool, public_key: [u8; STCP_PUBLIC_KEY_LEN] }
impl CryptoContext {
 pub const fn new()->Self{Self{ready:false,public_key:[0;STCP_PUBLIC_KEY_LEN]}}
 pub fn public_key_64(&self)->[u8;STCP_PUBLIC_KEY_LEN]{self.public_key}
 pub fn derive_key(&mut self, peer:&[u8])->Result<(),StcpError>{if peer.len()!=STCP_PUBLIC_KEY_LEN{return Err(StcpError::Crypto)};self.ready=true;Ok(())}
 pub fn encrypt(&mut self, plain:&[u8])->Result<([u8;STCP_IV_LEN],Vec<u8>),StcpError>{if !self.ready{return Err(StcpError::Crypto)};Ok(([0;STCP_IV_LEN],plain.to_vec()))}
 pub fn decrypt(&self,_iv:&[u8],cipher:&[u8])->Result<Vec<u8>,StcpError>{if !self.ready{return Err(StcpError::Crypto)};Ok(cipher.to_vec())}
}
