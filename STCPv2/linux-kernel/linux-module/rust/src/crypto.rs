use alloc::vec::Vec;

use core::ffi::{c_int, c_void};

use crate::error::StcpError;

pub const X25519_KEY_LEN: usize = 32;
pub const PUBLIC_KEY_WIRE_LEN: usize = 64;
pub const CHACHA_KEY_LEN: usize = 32;
pub const CHACHA_TAG_LEN: usize = 16;
pub const NONCE_LEN: usize = 8;

unsafe extern "C" {
    fn stcp_kernel_x25519_keypair(
        secret: *mut u8,
        public_key: *mut u8,
    ) -> c_int;

    fn stcp_kernel_x25519_shared(
        shared_key: *mut u8,
        secret: *const u8,
        peer_public: *const u8,
    ) -> c_int;

    fn stcp_kernel_chacha_encrypt(
        key: *const u8,
        nonce: u64,
        aad: *const u8,
        aad_len: usize,
        plaintext: *const u8,
        plaintext_len: usize,
        ciphertext: *mut u8,
        ciphertext_len: usize,
    ) -> c_int;

    fn stcp_kernel_chacha_decrypt(
        key: *const u8,
        nonce: u64,
        aad: *const u8,
        aad_len: usize,
        ciphertext: *const u8,
        ciphertext_len: usize,
        plaintext: *mut u8,
        plaintext_len: usize,
    ) -> c_int;
}

#[derive(Clone)]
pub struct CryptoContext {
    secret_key: [u8; X25519_KEY_LEN],
    public_key: [u8; PUBLIC_KEY_WIRE_LEN],
    shared_key: Option<[u8; CHACHA_KEY_LEN]>,
}

impl CryptoContext {
    pub fn new() -> Result<Self, StcpError> {
        let mut secret_key = [0u8; X25519_KEY_LEN];
        let mut public_key_32 = [0u8; X25519_KEY_LEN];

        let ret = unsafe {
            stcp_kernel_x25519_keypair(
                secret_key.as_mut_ptr(),
                public_key_32.as_mut_ptr(),
            )
        };

        if ret != 0 {
            return Err(StcpError::from_kernel_errno(ret));
        }

        let mut public_key = [0u8; PUBLIC_KEY_WIRE_LEN];
        public_key[..X25519_KEY_LEN].copy_from_slice(&public_key_32);

        Ok(Self {
            secret_key,
            public_key,
            shared_key: None,
        })
    }

    pub const fn public_key(&self) -> [u8; PUBLIC_KEY_WIRE_LEN] {
        self.public_key
    }

    pub fn derive_shared_key(
        &mut self,
        peer_public_wire: &[u8; PUBLIC_KEY_WIRE_LEN],
    ) -> Result<(), StcpError> {
        let mut shared_key = [0u8; CHACHA_KEY_LEN];

        let ret = unsafe {
            stcp_kernel_x25519_shared(
                shared_key.as_mut_ptr(),
                self.secret_key.as_ptr(),
                peer_public_wire.as_ptr(),
            )
        };

        if ret != 0 {
            return Err(StcpError::from_kernel_errno(ret));
        }

        self.shared_key = Some(shared_key);
        Ok(())
    }

    pub const fn ready(&self) -> bool {
        self.shared_key.is_some()
    }

    pub fn encrypt(
        &self,
        nonce: u64,
        aad: &[u8],
        plaintext: &[u8],
    ) -> Result<Vec<u8>, StcpError> {
        let key = self.shared_key.as_ref().ok_or(StcpError::InvalidState)?;
        let mut output = Vec::new();

        output
            .try_reserve_exact(plaintext.len() + CHACHA_TAG_LEN)
            .map_err(|_| StcpError::NoMem)?;

        output.resize(plaintext.len() + CHACHA_TAG_LEN, 0);

        let ret = unsafe {
            stcp_kernel_chacha_encrypt(
                key.as_ptr(),
                nonce,
                nullable_const(aad),
                aad.len(),
                nullable_const(plaintext),
                plaintext.len(),
                output.as_mut_ptr(),
                output.len(),
            )
        };

        if ret != 0 {
            return Err(StcpError::from_kernel_errno(ret));
        }

        Ok(output)
    }

    pub fn decrypt(
        &self,
        nonce: u64,
        aad: &[u8],
        ciphertext: &[u8],
    ) -> Result<Vec<u8>, StcpError> {
        if ciphertext.len() < CHACHA_TAG_LEN {
            return Err(StcpError::Crypto);
        }

        let key = self.shared_key.as_ref().ok_or(StcpError::InvalidState)?;
        let plaintext_len = ciphertext.len() - CHACHA_TAG_LEN;
        let mut output = Vec::new();

        output
            .try_reserve_exact(plaintext_len)
            .map_err(|_| StcpError::NoMem)?;

        output.resize(plaintext_len, 0);

        let output_len = output.len();
        let output_ptr = nullable_mut(&mut output);

        let ret = unsafe {
            stcp_kernel_chacha_decrypt(
                key.as_ptr(),
                nonce,
                nullable_const(aad),
                aad.len(),
                ciphertext.as_ptr(),
                ciphertext.len(),
                output_ptr,
                output_len,
            )
        };

        if ret != 0 {
            return Err(StcpError::from_kernel_errno(ret));
        }

        Ok(output)
    }
}

impl Drop for CryptoContext {
    fn drop(&mut self) {
        self.secret_key.fill(0);

        if let Some(key) = &mut self.shared_key {
            key.fill(0);
        }
    }
}

fn nullable_const(data: &[u8]) -> *const u8 {
    if data.is_empty() {
        core::ptr::null()
    } else {
        data.as_ptr()
    }
}

fn nullable_mut(data: &mut [u8]) -> *mut u8 {
    if data.is_empty() {
        core::ptr::null_mut()
    } else {
        data.as_mut_ptr()
    }
}
