
extern crate alloc;
pub use alloc::vec::Vec;

use p256::elliptic_curve::sec1::{ToEncodedPoint, FromEncodedPoint};
use p256::{SecretKey, PublicKey, EncodedPoint};
use p256::ecdh::SharedSecret;
use p256::ecdh::EphemeralSecret;
use crate::random::ZephyrRng as OsRng;

pub struct StcpEllipticCodec {
    shared_secret: Option<[u8; 32]>,
    private_key: EphemeralSecret,
    public_key: EncodedPoint,

    peer_public_key: Option<EncodedPoint>,
}

impl StcpEllipticCodec {

    pub fn check_for_public_key(&mut self, raw_key: Option<&[u8]>) -> Option<(Vec<u8>, Vec<u8>)> {
        if let Some(raw_key) = raw_key {
            let public_key = PublicKey::from_sec1_bytes(raw_key).ok()?;
            let shared_key = self.get_shared_secret_bytes(&public_key);

            let public_key_as_bytes = public_key
                                        .to_encoded_point(true)
                                        .as_bytes()
                                        .to_vec();

            Some((public_key_as_bytes, shared_key))
        } else {
            None
        }
    }

    /// Luo uusi EC-avainpari
    pub fn New() -> Self {
        let mut rng = OsRng;
        let private_key = EphemeralSecret::random(&mut OsRng);
        let public_key = EncodedPoint::from(private_key.public_key());

        // pub => bytes = > Vec
        //let encoded = public_key.to_encoded_point(false).as_bytes().to_vec()

        Self {
            shared_secret: None,
            private_key,
            public_key,
            peer_public_key: None,
        }
    }

    /// Ota raaka julkinen avain ja rakenna siitä EC-julkinen avain
    pub fn raw_public_key_to_proper_public_key(&self, raw_key: &[u8]) -> Option<PublicKey> {
        let encoded = EncodedPoint::from_bytes(raw_key).ok()?;
        PublicKey::from_encoded_point(&encoded).into()
    }

    /// Palauta julkinen avain tavumuodossa (SEC1)
    pub fn my_public_key_to_bytes(&self) -> Vec<u8> {
        self.public_key.as_bytes().to_vec()
    }

    pub fn public_key_to_bytes(&self, pk: &PublicKey) -> Vec<u8> {
        pk.to_encoded_point(false).as_bytes().to_vec()
    }

    pub fn peer_public_key_to_bytes(&self) -> Vec<u8> {
        match &self.peer_public_key {
            Some(pk) => pk.as_bytes().to_vec(),
            None => Vec::new(),
        }
    }

    pub fn my_public_key_to_raw_bytes(&self) -> Vec<u8> {
        self.my_public_key_to_bytes()
    }

    pub fn peer_public_key_to_raw_bytes(&self) -> Vec<u8> {
        self.peer_public_key_to_bytes()
    }

    pub fn compute_shared_secret(&mut self, peer_pub_bytes: &[u8]) -> bool {

        if let Ok(peer_pub) = PublicKey::from_sec1_bytes(peer_pub_bytes) {

            let shared = self.private_key.diffie_hellman(&peer_pub);
            self.peer_public_key = Some(peer_pub.to_encoded_point(false));
            self.shared_secret = Some(
                    shared.raw_secret_bytes()
                        .as_slice()
                        .try_into()
                        .unwrap()
                );
            true
        } else {
            false
        }
    }

    pub fn get_shared_secret_bytes(&self, peer_pub_key: &PublicKey) -> Vec<u8> {

        let shared = self.private_key.diffie_hellman(peer_pub_key);
        let tomatch = shared.raw_secret_bytes().as_slice().to_vec();
        tomatch
    }
}
