use alloc::vec::Vec;
use core::ffi::c_int;
use crate::{error::StcpError, kdf::derive_directional_keys};

pub const X25519_KEY_LEN:usize=32; pub const PUBLIC_KEY_WIRE_LEN:usize=64; pub const CHACHA_KEY_LEN:usize=32; pub const CHACHA_TAG_LEN:usize=16; pub const NONCE_LEN:usize=8;
#[derive(Clone,Copy,PartialEq,Eq)] pub enum Role{Client,Server}
unsafe extern "C" {
 fn stcp_kernel_x25519_keypair(secret:*mut u8,public_key:*mut u8)->c_int;
 fn stcp_kernel_x25519_shared(shared:*mut u8,secret:*const u8,peer:*const u8)->c_int;
 fn stcp_kernel_chacha_encrypt(key:*const u8,nonce:u64,aad:*const u8,aad_len:usize,plain:*const u8,plain_len:usize,out:*mut u8,out_len:usize)->c_int;
 fn stcp_kernel_chacha_decrypt(key:*const u8,nonce:u64,aad:*const u8,aad_len:usize,cipher:*const u8,cipher_len:usize,out:*mut u8,out_len:usize)->c_int;
}
#[derive(Clone)] pub struct CryptoContext{secret_key:[u8;32],public_key:[u8;64],tx_key:Option<[u8;32]>,rx_key:Option<[u8;32]>}
impl CryptoContext{
 pub fn new()->Result<Self,StcpError>{let mut secret=[0;32];let mut pub32=[0;32];let r=unsafe{stcp_kernel_x25519_keypair(secret.as_mut_ptr(),pub32.as_mut_ptr())};if r!=0{return Err(StcpError::Kernel(r));}let mut public=[0;64];public[..32].copy_from_slice(&pub32);Ok(Self{secret_key:secret,public_key:public,tx_key:None,rx_key:None})}
 pub const fn public_key(&self)->[u8;64]{self.public_key}
 pub fn derive_session_keys(&mut self,peer:&[u8;64],role:Role)->Result<(),StcpError>{let mut shared=[0;32];let r=unsafe{stcp_kernel_x25519_shared(shared.as_mut_ptr(),self.secret_key.as_ptr(),peer.as_ptr())};if r!=0{return Err(StcpError::Kernel(r));}let mut local=[0u8;32];local.copy_from_slice(&self.public_key[..32]);let mut remote=[0u8;32];remote.copy_from_slice(&peer[..32]);let (client_pub,server_pub)=match role{Role::Client=>(&local,&remote),Role::Server=>(&remote,&local)};let (c2s,s2c)=derive_directional_keys(&shared,client_pub,server_pub)?;shared.fill(0);match role{Role::Client=>{self.tx_key=Some(c2s);self.rx_key=Some(s2c)},Role::Server=>{self.tx_key=Some(s2c);self.rx_key=Some(c2s)}}Ok(())}
 pub const fn ready(&self)->bool{self.tx_key.is_some()&&self.rx_key.is_some()}
 pub fn encrypt(&self,nonce:u64,aad:&[u8],plain:&[u8])->Result<Vec<u8>,StcpError>{let key=self.tx_key.as_ref().ok_or(StcpError::InvalidState)?;let mut out=Vec::new();out.try_reserve_exact(plain.len()+16).map_err(|_|StcpError::NoMem)?;out.resize(plain.len()+16,0);let r=unsafe{stcp_kernel_chacha_encrypt(key.as_ptr(),nonce,nc(aad),aad.len(),nc(plain),plain.len(),out.as_mut_ptr(),out.len())};if r!=0{out.fill(0);return Err(StcpError::Kernel(r));}Ok(out)}
 pub fn decrypt(&self,nonce:u64,aad:&[u8],cipher:&[u8])->Result<Vec<u8>,StcpError>{if cipher.len()<16{return Err(StcpError::Crypto);}let key=self.rx_key.as_ref().ok_or(StcpError::InvalidState)?;let mut out=Vec::new();out.try_reserve_exact(cipher.len()-16).map_err(|_|StcpError::NoMem)?;out.resize(cipher.len()-16,0);let r=unsafe{stcp_kernel_chacha_decrypt(key.as_ptr(),nonce,nc(aad),aad.len(),cipher.as_ptr(),cipher.len(),nm(&mut out),out.len())};if r!=0{out.fill(0);return Err(StcpError::Kernel(r));}Ok(out)}
}
impl Drop for CryptoContext{fn drop(&mut self){self.secret_key.fill(0);if let Some(k)=&mut self.tx_key{k.fill(0)}if let Some(k)=&mut self.rx_key{k.fill(0)}}}
fn nc(x:&[u8])->*const u8{if x.is_empty(){core::ptr::null()}else{x.as_ptr()}}fn nm(x:&mut[u8])->*mut u8{if x.is_empty(){core::ptr::null_mut()}else{x.as_mut_ptr()}}

pub fn selftest()->Result<(),StcpError>{let mut c=CryptoContext::new()?;let mut s=CryptoContext::new()?;let cp=c.public_key();let sp=s.public_key();c.derive_session_keys(&sp,Role::Client)?;s.derive_session_keys(&cp,Role::Server)?;let aad=b"STCP-selftest";let plain=b"directional session keys";let enc=c.encrypt(0,aad,plain)?;let dec=s.decrypt(0,aad,&enc)?;if dec.as_slice()!=plain{return Err(StcpError::Crypto);}let mut bad=enc.clone();if let Some(x)=bad.last_mut(){*x^=1;}if s.decrypt(0,aad,&bad).is_ok(){return Err(StcpError::Crypto);}if c.decrypt(0,aad,&enc).is_ok(){return Err(StcpError::Crypto);}Ok(())}
