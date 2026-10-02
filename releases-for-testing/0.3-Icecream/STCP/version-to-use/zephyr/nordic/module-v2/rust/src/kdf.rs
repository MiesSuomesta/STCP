use crate::error::StcpError;

const BLOCK: usize = 64;
const OUT: usize = 32;

const K: [u32; 64] = [
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2,
];

#[derive(Clone)]
struct Sha256 { state: [u32;8], block: [u8;64], used: usize, bytes: u64 }
impl Sha256 {
    fn new() -> Self { Self { state:[0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19], block:[0;64], used:0, bytes:0 } }
    fn update(&mut self, mut data:&[u8]) {
        self.bytes = self.bytes.wrapping_add(data.len() as u64);
        while !data.is_empty() {
            let n=(64-self.used).min(data.len()); self.block[self.used..self.used+n].copy_from_slice(&data[..n]); self.used+=n; data=&data[n..];
            if self.used==64 { let b=self.block; self.compress(&b); self.used=0; }
        }
    }
    fn finish(mut self)->[u8;32] {
        let bits=self.bytes.wrapping_mul(8); self.block[self.used]=0x80; self.used+=1;
        if self.used>56 { for b in &mut self.block[self.used..] {*b=0;} let x=self.block; self.compress(&x); self.used=0; }
        for b in &mut self.block[self.used..56] {*b=0;} self.block[56..64].copy_from_slice(&bits.to_be_bytes()); let x=self.block; self.compress(&x);
        let mut out=[0u8;32]; for (i,v) in self.state.iter().enumerate(){out[i*4..i*4+4].copy_from_slice(&v.to_be_bytes());} out
    }
    fn compress(&mut self,b:&[u8;64]) { let mut w=[0u32;64]; for i in 0..16 {w[i]=u32::from_be_bytes([b[i*4],b[i*4+1],b[i*4+2],b[i*4+3]]);} for i in 16..64 {let s0=w[i-15].rotate_right(7)^w[i-15].rotate_right(18)^(w[i-15]>>3);let s1=w[i-2].rotate_right(17)^w[i-2].rotate_right(19)^(w[i-2]>>10);w[i]=w[i-16].wrapping_add(s0).wrapping_add(w[i-7]).wrapping_add(s1);} let [mut a,mut b0,mut c,mut d,mut e,mut f,mut g,mut h]=self.state; for i in 0..64 {let s1=e.rotate_right(6)^e.rotate_right(11)^e.rotate_right(25);let ch=(e&f)^(!e&g);let t1=h.wrapping_add(s1).wrapping_add(ch).wrapping_add(K[i]).wrapping_add(w[i]);let s0=a.rotate_right(2)^a.rotate_right(13)^a.rotate_right(22);let maj=(a&b0)^(a&c)^(b0&c);let t2=s0.wrapping_add(maj);h=g;g=f;f=e;e=d.wrapping_add(t1);d=c;c=b0;b0=a;a=t1.wrapping_add(t2);} let v=[a,b0,c,d,e,f,g,h]; for i in 0..8 {self.state[i]=self.state[i].wrapping_add(v[i]);} }
}
fn hash(parts:&[&[u8]])->[u8;32]{let mut s=Sha256::new();for p in parts{s.update(p);}s.finish()}
fn hmac_parts(key:&[u8], parts:&[&[u8]])->[u8;32]{let mut k=[0u8;BLOCK];if key.len()>BLOCK{k[..OUT].copy_from_slice(&hash(&[key]));}else{k[..key.len()].copy_from_slice(key);}let mut ipad=[0x36u8;BLOCK];let mut opad=[0x5cu8;BLOCK];for i in 0..BLOCK{ipad[i]^=k[i];opad[i]^=k[i];}let mut inner=Sha256::new();inner.update(&ipad);for p in parts{inner.update(p);}let ih=inner.finish();let mut outer=Sha256::new();outer.update(&opad);outer.update(&ih);outer.finish()}

pub fn derive_directional_keys(shared:&[u8;32], client_pub:&[u8;32], server_pub:&[u8;32])->Result<([u8;32],[u8;32]),StcpError>{
    let salt=hash(&[b"STCPv2-HKDF-SHA256",client_pub,server_pub]); let prk=hmac_parts(&salt,&[shared]);
    let client=hmac_parts(&prk,&[b"STCPv2 client to server key", &[1]]);
    let server=hmac_parts(&prk,&[b"STCPv2 server to client key", &[1]]);
    if client==server { return Err(StcpError::Crypto); }
    Ok((client,server))
}
