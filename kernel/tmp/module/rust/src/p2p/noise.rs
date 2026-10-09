use alloc::vec::Vec;
use core::ffi::c_int;

use ed25519_compact::{KeyPair, PublicKey, Seed, Signature};

use crate::error::StcpError;
use crate::kdf::{hash, hmac_parts};

pub const NOISE_PROTOCOL_NAME: &[u8] = b"Noise_XX_25519_ChaChaPoly_SHA256";
pub const STATIC_KEY_PREFIX: &[u8] = b"noise-libp2p-static-key:";
pub const NOISE_MAX_FRAME: usize = 65535;
pub const ED25519_KEY_TYPE: u64 = 1;

const TAG_LEN: usize = 16;
const KEY_LEN: usize = 32;

unsafe extern "C" {
    fn stcp_kernel_x25519_keypair(secret: *mut u8, public_key: *mut u8) -> c_int;
    fn stcp_kernel_x25519_shared(shared: *mut u8, secret: *const u8, peer: *const u8) -> c_int;
    fn stcp_kernel_chacha_encrypt(
        key: *const u8,
        nonce: u64,
        aad: *const u8,
        aad_len: usize,
        plain: *const u8,
        plain_len: usize,
        out: *mut u8,
        out_len: usize,
    ) -> c_int;
    fn stcp_kernel_chacha_decrypt(
        key: *const u8,
        nonce: u64,
        aad: *const u8,
        aad_len: usize,
        cipher: *const u8,
        cipher_len: usize,
        out: *mut u8,
        out_len: usize,
    ) -> c_int;
}

#[derive(Clone)]
struct CipherState {
    key: Option<[u8; KEY_LEN]>,
    nonce: u64,
}

impl CipherState {
    const fn new() -> Self { Self { key: None, nonce: 0 } }
    fn initialize_key(&mut self, key: [u8; KEY_LEN]) { self.key = Some(key); self.nonce = 0; }
    fn has_key(&self) -> bool { self.key.is_some() }

    fn encrypt_with_ad(&mut self, ad: &[u8], plain: &[u8]) -> Result<Vec<u8>, StcpError> {
        if !self.has_key() { return Ok(plain.to_vec()); }
        if self.nonce == u64::MAX { return Err(StcpError::Protocol); }
        let key = self.key.as_ref().ok_or(StcpError::InvalidState)?;
        let mut out = Vec::new();
        out.try_reserve_exact(plain.len() + TAG_LEN).map_err(|_| StcpError::NoMem)?;
        out.resize(plain.len() + TAG_LEN, 0);
        let rc = unsafe {
            stcp_kernel_chacha_encrypt(
                key.as_ptr(), self.nonce,
                if ad.is_empty() { core::ptr::null() } else { ad.as_ptr() }, ad.len(),
                if plain.is_empty() { core::ptr::null() } else { plain.as_ptr() }, plain.len(),
                out.as_mut_ptr(), out.len())
        };
        if rc != 0 { return Err(StcpError::Kernel(rc)); }
        self.nonce = self.nonce.wrapping_add(1);
        Ok(out)
    }

    fn decrypt_with_ad(&mut self, ad: &[u8], cipher: &[u8]) -> Result<Vec<u8>, StcpError> {
        if !self.has_key() { return Ok(cipher.to_vec()); }
        if cipher.len() < TAG_LEN || self.nonce == u64::MAX { return Err(StcpError::Protocol); }
        let key = self.key.as_ref().ok_or(StcpError::InvalidState)?;
        let mut out = Vec::new();
        out.try_reserve_exact(cipher.len() - TAG_LEN).map_err(|_| StcpError::NoMem)?;
        out.resize(cipher.len() - TAG_LEN, 0);
        let rc = unsafe {
            stcp_kernel_chacha_decrypt(
                key.as_ptr(), self.nonce,
                if ad.is_empty() { core::ptr::null() } else { ad.as_ptr() }, ad.len(),
                cipher.as_ptr(), cipher.len(), out.as_mut_ptr(), out.len())
        };
        if rc != 0 { return Err(StcpError::Kernel(rc)); }
        self.nonce = self.nonce.wrapping_add(1);
        Ok(out)
    }
}

struct SymmetricState {
    ck: [u8; 32],
    h: [u8; 32],
    cipher: CipherState,
}

impl SymmetricState {
    fn new() -> Self {
        let initial = if NOISE_PROTOCOL_NAME.len() <= 32 {
            let mut p = [0u8; 32]; p[..NOISE_PROTOCOL_NAME.len()].copy_from_slice(NOISE_PROTOCOL_NAME); p
        } else {
            hash(&[NOISE_PROTOCOL_NAME])
        };
        Self { ck: initial, h: initial, cipher: CipherState::new() }
    }

    fn mix_hash(&mut self, data: &[u8]) { self.h = hash(&[&self.h, data]); }

    fn hkdf2(ck: &[u8;32], ikm: &[u8]) -> ([u8;32], [u8;32]) {
        let temp_key = hmac_parts(ck, &[ikm]);
        let out1 = hmac_parts(&temp_key, &[&[1u8]]);
        let out2 = hmac_parts(&temp_key, &[&out1, &[2u8]]);
        (out1, out2)
    }

    fn mix_key(&mut self, ikm: &[u8]) {
        let (ck, temp_k) = Self::hkdf2(&self.ck, ikm);
        self.ck = ck;
        self.cipher.initialize_key(temp_k);
    }

    fn encrypt_and_hash(&mut self, plain: &[u8]) -> Result<Vec<u8>, StcpError> {
        let out = self.cipher.encrypt_with_ad(&self.h, plain)?;
        self.mix_hash(&out);
        Ok(out)
    }

    fn decrypt_and_hash(&mut self, cipher: &[u8]) -> Result<Vec<u8>, StcpError> {
        let plain = self.cipher.decrypt_with_ad(&self.h, cipher)?;
        self.mix_hash(cipher);
        Ok(plain)
    }

    fn split(&self) -> ([u8;32], [u8;32]) { Self::hkdf2(&self.ck, &[]) }
}

fn x25519_keypair() -> Result<([u8;32],[u8;32]), StcpError> {
    let mut secret = [0u8;32];
    let mut public = [0u8;32];
    let rc = unsafe { stcp_kernel_x25519_keypair(secret.as_mut_ptr(), public.as_mut_ptr()) };
    if rc != 0 { return Err(StcpError::Kernel(rc)); }
    Ok((secret, public))
}

fn x25519(secret: &[u8;32], public: &[u8;32]) -> Result<[u8;32], StcpError> {
    let mut shared = [0u8;32];
    let rc = unsafe { stcp_kernel_x25519_shared(shared.as_mut_ptr(), secret.as_ptr(), public.as_ptr()) };
    if rc != 0 { return Err(StcpError::Kernel(rc)); }
    Ok(shared)
}

fn put_varint(mut v: usize, out: &mut Vec<u8>) -> Result<(), StcpError> {
    loop {
        let mut b=(v & 0x7f) as u8; v >>= 7; if v != 0 { b |= 0x80; }
        out.try_reserve(1).map_err(|_|StcpError::NoMem)?; out.push(b); if v==0 { return Ok(()); }
    }
}

fn get_varint(input: &[u8], off: &mut usize) -> Result<usize, StcpError> {
    let mut v=0usize; let mut shift=0usize;
    loop {
        if *off >= input.len() || shift >= usize::BITS as usize { return Err(StcpError::Protocol); }
        let b=input[*off]; *off += 1; v |= ((b&0x7f) as usize) << shift;
        if b&0x80==0 { return Ok(v); } shift += 7;
    }
}

fn put_key(field: usize, wire: usize, out: &mut Vec<u8>) -> Result<(), StcpError> { put_varint((field<<3)|wire, out) }
fn put_bytes(field: usize, bytes: &[u8], out: &mut Vec<u8>) -> Result<(), StcpError> {
    put_key(field,2,out)?; put_varint(bytes.len(),out)?; out.try_reserve(bytes.len()).map_err(|_|StcpError::NoMem)?; out.extend_from_slice(bytes); Ok(())
}

pub fn encode_ed25519_public_key(pk: &[u8;32]) -> Result<Vec<u8>,StcpError> {
    let mut out=Vec::new(); put_key(1,0,&mut out)?; put_varint(ED25519_KEY_TYPE as usize,&mut out)?; put_bytes(2,pk,&mut out)?; Ok(out)
}

pub fn encode_handshake_payload(identity_key: &[u8], identity_sig: &[u8]) -> Result<Vec<u8>,StcpError> {
    let mut out=Vec::new(); put_bytes(1,identity_key,&mut out)?; put_bytes(2,identity_sig,&mut out)?; Ok(out)
}

pub fn decode_handshake_payload(input: &[u8]) -> Result<(Vec<u8>,Vec<u8>),StcpError> {
    let mut off=0usize; let mut key=None; let mut sig=None;
    while off<input.len() {
        let tag=get_varint(input,&mut off)?; let field=tag>>3; let wire=tag&7;
        if wire != 2 { return Err(StcpError::Protocol); }
        let len=get_varint(input,&mut off)?; let end=off.checked_add(len).ok_or(StcpError::Protocol)?; if end>input.len(){return Err(StcpError::Protocol);}
        match field { 1=>key=Some(input[off..end].to_vec()), 2=>sig=Some(input[off..end].to_vec()), _=>{} }
        off=end;
    }
    Ok((key.ok_or(StcpError::Protocol)?, sig.ok_or(StcpError::Protocol)?))
}

fn decode_ed25519_public_key(input:&[u8])->Result<[u8;32],StcpError>{
    let mut off=0usize; let mut ty=None; let mut data=None;
    while off<input.len(){let tag=get_varint(input,&mut off)?;let field=tag>>3;let wire=tag&7;match (field,wire){
        (1,0)=>ty=Some(get_varint(input,&mut off)? as u64),
        (2,2)=>{let n=get_varint(input,&mut off)?;let end=off.checked_add(n).ok_or(StcpError::Protocol)?;if end>input.len(){return Err(StcpError::Protocol);}data=Some(input[off..end].to_vec());off=end;},
        (_,0)=>{let _=get_varint(input,&mut off)?;},
        (_,2)=>{let n=get_varint(input,&mut off)?;off=off.checked_add(n).ok_or(StcpError::Protocol)?;if off>input.len(){return Err(StcpError::Protocol);}},
        _=>return Err(StcpError::Protocol)}}
    if ty!=Some(ED25519_KEY_TYPE){return Err(StcpError::Protocol);}let d=data.ok_or(StcpError::Protocol)?;if d.len()!=32{return Err(StcpError::Protocol);}let mut pk=[0u8;32];pk.copy_from_slice(&d);Ok(pk)
}


pub fn make_ed25519_identity(seed_bytes: [u8;32], static_public: &[u8;32]) -> Result<(Vec<u8>,Vec<u8>),StcpError> {
    if seed_bytes.iter().all(|b| *b == 0) { return Err(StcpError::Crypto); }
    let kp = KeyPair::from_seed(Seed::new(seed_bytes));
    let identity_key = encode_ed25519_public_key(kp.pk.as_ref().try_into().map_err(|_|StcpError::Crypto)?)?;
    let mut signed=Vec::new(); signed.try_reserve_exact(STATIC_KEY_PREFIX.len()+32).map_err(|_|StcpError::NoMem)?;
    signed.extend_from_slice(STATIC_KEY_PREFIX); signed.extend_from_slice(static_public);
    let sig = kp.sk.sign(&signed, None);
    Ok((identity_key, sig.as_ref().to_vec()))
}

pub fn verify_identity_payload(payload:&[u8], remote_static:&[u8;32])->Result<Vec<u8>,StcpError>{
    let (identity_key,sig_bytes)=decode_handshake_payload(payload)?;
    let pk_bytes=decode_ed25519_public_key(&identity_key)?;
    if sig_bytes.len()!=64{return Err(StcpError::Protocol);}
    let pk=PublicKey::from_slice(&pk_bytes).map_err(|_|StcpError::Crypto)?;
    let sig=Signature::from_slice(&sig_bytes).map_err(|_|StcpError::Crypto)?;
    let mut signed=Vec::new(); signed.try_reserve_exact(STATIC_KEY_PREFIX.len()+32).map_err(|_|StcpError::NoMem)?;
    signed.extend_from_slice(STATIC_KEY_PREFIX); signed.extend_from_slice(remote_static);
    pk.verify(&signed,&sig).map_err(|_|StcpError::Crypto)?;
    Ok(identity_key)
}

#[derive(Debug,Clone,Copy,PartialEq,Eq)]
pub enum NoiseStep { WaitMessage2, Complete }

pub struct NoiseInitiator {
    symmetric: SymmetricState,
    e_secret:[u8;32],
    e_public:[u8;32],
    s_secret:[u8;32],
    s_public:[u8;32],
    identity_key:Vec<u8>,
    identity_sig:Vec<u8>,
    step:NoiseStep,
    pub remote_identity_key:Option<Vec<u8>>,
    tx_key:Option<[u8;32]>,
    rx_key:Option<[u8;32]>,
}

impl NoiseInitiator {
    pub fn from_identity_seed(seed:[u8;32])->Result<Self,StcpError>{
        let (s_secret,s_public)=x25519_keypair()?;
        let (identity_key,identity_sig)=make_ed25519_identity(seed,&s_public)?;
        Self::new(s_secret,s_public,identity_key,identity_sig)
    }

    pub fn new(s_secret:[u8;32],s_public:[u8;32],identity_key:Vec<u8>,identity_sig:Vec<u8>)->Result<Self,StcpError>{
        let (e_secret,e_public)=x25519_keypair()?;
        Ok(Self{symmetric:SymmetricState::new(),e_secret,e_public,s_secret,s_public,identity_key,identity_sig,step:NoiseStep::WaitMessage2,remote_identity_key:None,tx_key:None,rx_key:None})
    }

    pub fn write_message1(&mut self)->Result<Vec<u8>,StcpError>{
        let mut msg=Vec::new();
        msg.try_reserve_exact(32).map_err(|_|StcpError::NoMem)?;
        msg.extend_from_slice(&self.e_public);
        self.symmetric.mix_hash(&self.e_public);
        /* XX message 1 still processes the (empty) handshake payload.
         * With no cipher key yet this emits zero bytes but MUST MixHash(""). */
        let empty=self.symmetric.encrypt_and_hash(&[])?;
        debug_assert!(empty.is_empty());
        Ok(msg)
    }

    pub fn read_message2_write_message3(&mut self,msg2:&[u8])->Result<Vec<u8>,StcpError>{
        if self.step!=NoiseStep::WaitMessage2 || msg2.len()<32+48+16{return Err(StcpError::Protocol);}
        let mut re=[0u8;32];re.copy_from_slice(&msg2[..32]);self.symmetric.mix_hash(&re);
        let ee=x25519(&self.e_secret,&re)?;self.symmetric.mix_key(&ee);
        let rs_plain=self.symmetric.decrypt_and_hash(&msg2[32..80])?;if rs_plain.len()!=32{return Err(StcpError::Protocol);}let mut rs=[0u8;32];rs.copy_from_slice(&rs_plain);
        let es=x25519(&self.e_secret,&rs)?;self.symmetric.mix_key(&es);
        let remote_payload=self.symmetric.decrypt_and_hash(&msg2[80..])?;
        self.remote_identity_key=Some(verify_identity_payload(&remote_payload,&rs)?);

        let mut msg3=Vec::new();let enc_s=self.symmetric.encrypt_and_hash(&self.s_public)?;msg3.try_reserve(enc_s.len()+128).map_err(|_|StcpError::NoMem)?;msg3.extend_from_slice(&enc_s);
        let se=x25519(&self.s_secret,&re)?;self.symmetric.mix_key(&se);
        let payload=encode_handshake_payload(&self.identity_key,&self.identity_sig)?;
        let enc_payload=self.symmetric.encrypt_and_hash(&payload)?;msg3.extend_from_slice(&enc_payload);
        let (k1,k2)=self.symmetric.split();self.tx_key=Some(k1);self.rx_key=Some(k2);self.step=NoiseStep::Complete;Ok(msg3)
    }

    pub fn complete(&self)->bool{self.step==NoiseStep::Complete}
}

pub fn encode_noise_frame(msg:&[u8],out:&mut Vec<u8>)->Result<(),StcpError>{
    if msg.len()>NOISE_MAX_FRAME{return Err(StcpError::Protocol);}out.try_reserve(msg.len()+2).map_err(|_|StcpError::NoMem)?;out.extend_from_slice(&(msg.len() as u16).to_be_bytes());out.extend_from_slice(msg);Ok(())
}

pub fn decode_noise_frame(input:&[u8])->Result<Option<(usize,usize)>,StcpError>{
    if input.len()<2{return Ok(None);}let n=u16::from_be_bytes([input[0],input[1]]) as usize;if input.len()<n+2{return Ok(None);}Ok(Some((2,n)))
}

pub fn selftest()->Result<(),StcpError>{
    if hash(&[b"abc"]) != [0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad]{return Err(StcpError::Crypto);}
    let pk=[0x11u8;32];let enc=encode_ed25519_public_key(&pk)?;if enc.len()!=36||enc[0]!=0x08||enc[1]!=0x01||enc[2]!=0x12||enc[3]!=0x20{return Err(StcpError::Protocol);}
    let payload=encode_handshake_payload(&enc,&[0x22u8;64])?;let (k,s)=decode_handshake_payload(&payload)?;if k!=enc||s!=[0x22u8;64]{return Err(StcpError::Protocol);}
    let static_pub=[0x33u8;32];let (ik,isig)=make_ed25519_identity([0x44u8;32],&static_pub)?;let ip=encode_handshake_payload(&ik,&isig)?;let verified=verify_identity_payload(&ip,&static_pub)?;if verified!=ik{return Err(StcpError::Crypto);}
    let mut f=Vec::new();encode_noise_frame(&[1,2,3],&mut f)?;if f!=[0,3,1,2,3]{return Err(StcpError::Protocol);}match decode_noise_frame(&f)?{Some((2,3))=>{},_=>return Err(StcpError::Protocol)}
    let st=SymmetricState::new();
    let mut expected = [0u8; 32];
    expected.copy_from_slice(NOISE_PROTOCOL_NAME);
    if st.ck != expected || st.h != expected { return Err(StcpError::Protocol); }
    Ok(())
}
