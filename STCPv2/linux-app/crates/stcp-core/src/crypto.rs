use aes_gcm::{
    Aes256Gcm, Nonce, aead::{
        Aead,
        KeyInit,
        OsRng,
        rand_core::RngCore,
    },
};

use x25519_dalek::{PublicKey, StaticSecret};

use crate::error::StcpError;
use crate::packet::{
    STCP_IV_LEN,
    STCP_PUBLIC_KEY_LEN,
    STCP_AES_GCM_NONCE_LEN,
};

pub const AES_KEY_LEN: usize = 32;

#[derive(Clone)]
pub struct CryptoContext {
    own_secret_key: StaticSecret,
    own_public_key_32: PublicKey,
    derived_aes_key: [u8; AES_KEY_LEN],
    aes_ready: bool,
    cipher: Option<Aes256Gcm>,
}

impl std::fmt::Debug for CryptoContext {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("CryptoContext")
            .field("own_public_key_64", &self.public_key_64())
            .field("aes_ready", &self.aes_ready)
            .field("derived_aes_key", &"<hidden>")
            .finish()
    }
}

impl Default for CryptoContext {
    fn default() -> Self {
        let own_secret_key = StaticSecret::random_from_rng(OsRng);
        let own_public_key_32 = PublicKey::from(&own_secret_key);

        Self {
            own_secret_key,
            own_public_key_32,
            derived_aes_key: [0u8; AES_KEY_LEN],
            aes_ready: false,
            cipher: None,
        }
    }
}

impl CryptoContext {
    pub fn public_key_64(&self) -> [u8; STCP_PUBLIC_KEY_LEN] {
        let mut out = [0u8; STCP_PUBLIC_KEY_LEN];
        out[..32].copy_from_slice(&self.own_public_key_32.to_bytes());
        out
    }

    pub fn derive_aes_key(&mut self, peer_pk_64: &[u8]) -> Result<(), StcpError> {
        if peer_pk_64.len() != STCP_PUBLIC_KEY_LEN {
            return Err(StcpError::Crypto(format!(
                "invalid peer public key len: {}",
                peer_pk_64.len()
            )));
        }

        let peer32: [u8; 32] = peer_pk_64[..32]
            .try_into()
            .map_err(|_| StcpError::Crypto(
                "peer public key conversion failed".to_string()
            ))?;

        let peer_public = PublicKey::from(peer32);
        let shared = self.own_secret_key.diffie_hellman(&peer_public);

        self.derived_aes_key = shared.to_bytes();

        self.cipher = Some(
            Aes256Gcm::new_from_slice(&self.derived_aes_key)
                .map_err(|e| StcpError::Crypto(format!(
                    "AES init failed: {e:?}"
                )))?
        );

        self.aes_ready = true;

        Ok(())
    }

    pub fn aes_ready(&self) -> bool {
        self.aes_ready
    }

    fn cipher(&self) -> Result<&Aes256Gcm, StcpError> {
        if !self.aes_ready {
            return Err(StcpError::Crypto(
                "AES key is not ready".to_string(),
            ));
        }

        self.cipher
            .as_ref()
            .ok_or_else(|| StcpError::Crypto(
                "AES cipher is not ready".to_string(),
            ))
    }

    pub fn encrypt(
        &mut self,
        plaintext: &[u8],
    ) -> Result<([u8; STCP_IV_LEN], Vec<u8>), StcpError> {
        let cipher = self.cipher()?;

        let mut iv = [0u8; STCP_IV_LEN];
        OsRng.fill_bytes(&mut iv);

        let nonce = Nonce::from_slice(&iv[..STCP_AES_GCM_NONCE_LEN]);

        let ciphertext = cipher
            .encrypt(nonce, plaintext)
            .map_err(|e| StcpError::Crypto(format!(
                "AES-GCM encrypt failed: {e:?}"
            )))?;

        Ok((iv, ciphertext))
    }

    pub fn decrypt(
        &self,
        iv: &[u8],
        ciphertext: &[u8],
    ) -> Result<Vec<u8>, StcpError> {
        if iv.len() != STCP_IV_LEN {
            return Err(StcpError::Crypto(format!(
                "invalid IV len: {}",
                iv.len()
            )));
        }

        let cipher = self.cipher()?;

        let nonce = Nonce::from_slice(&iv[..STCP_AES_GCM_NONCE_LEN]);

        cipher
            .decrypt(nonce, ciphertext)
            .map_err(|e| StcpError::Crypto(format!(
                "AES-GCM decrypt failed: {e:?}"
            )))
    }
}