use openssl::nid::Nid;
use openssl::pkey::{PKey, Private, Public};
use openssl::derive::Deriver;
use openssl::sha::Sha256;
use openssl::ec::*;
use openssl::bn::BigNumContext;
use crate::dprint;

/// 🔐 OpenSSL-pohjainen Elliptic Curve -avainvaihto
pub struct StcpEllipticCodec {
    shared_secret: Vec<u8>,
    private_key: EcKey<Private>,
    public_key: EcKey<Public>,
    peer_public_key: EcKey<Public>,
}

// valid values 16, 24 32 bytes
const STCP_AES_KEY_SIZE_IN_BYTES: usize = 32;

// Must be 16 bytes
const STCP_AES_IV_SIZE_IN_BYTES: usize = 16;

impl StcpEllipticCodec {
    /// 🔹 Luo uuden avainparin
    pub fn new() -> Self {
        let group = EcGroup::from_curve_name(Nid::X9_62_PRIME256V1).expect("Failed to create EC group");
        let private_key = EcKey::generate(&group).expect("Failed to generate EC private key");
        let public_key = EcKey::from_public_key(&group, private_key.public_key()).expect("Failed to create public key");
        let peer_public_key = public_key.clone().to_owned(); // Temp hack...
        Self { private_key, public_key, shared_secret: Vec::new(), peer_public_key }
    }

    pub fn raw_public_key_to_proper_public_key(&self, raw_key: &[u8]) -> Result<EcKey<Public>, openssl::error::ErrorStack> {
        let group = EcGroup::from_curve_name(Nid::X9_62_PRIME256V1)?;  // Valitse oikea käyrä
        let mut ctx = BigNumContext::new()?;
    
        // Luo EcPoint raakadatasta
        let pub_point = EcPoint::from_bytes(&group, raw_key, &mut ctx)?;
    
        // Luo PublicKey EcKey-muodossa
        let pub_key = EcKey::from_public_key(&group, &pub_point)?;
    
        Ok(pub_key)
    }
    

    /// 🔹 Palauta julkinen avain tavumuodossa
    pub fn my_public_key_to_bytes(&self) -> Vec<u8> {
        self.public_key.public_key_to_der().expect("Failed to serialize public key")
    }

    pub fn public_key_to_bytes(&self, param_public_key: EcKey<Public>) -> Vec<u8> {
        param_public_key.public_key_to_der().expect("Failed to serialize public key")
    }

    /// 🔹 Palauta peer'in julkinen avain tavumuodossa
    pub fn peer_public_key_to_bytes(&self) -> Vec<u8> {
        self.peer_public_key.public_key_to_der().expect("Failed to serialize peer's public key")
    }

    pub fn my_public_key_to_raw_bytes(&self) -> Vec<u8> {
        let group = EcGroup::from_curve_name(Nid::X9_62_PRIME256V1).expect("Failed to create EC group");
        let der_form = self.public_key.public_key_to_der().unwrap();
        let key_from_der = EcKey::public_key_from_der(&der_form).unwrap();
        let mut bn_ctx = openssl::bn::BigNumContext::new().unwrap();

        let raw_pub_key = key_from_der.public_key().to_bytes(
            &group,
            openssl::ec::PointConversionForm::UNCOMPRESSED,
            &mut bn_ctx  // Korjattu: annetaan muuttuva viittaus
        ).unwrap();        return raw_pub_key;
    }    

    pub fn peer_public_key_to_raw_bytes(&self) -> Vec<u8> {
        let group = EcGroup::from_curve_name(Nid::X9_62_PRIME256V1).expect("Failed to create EC group");
        let der_form = self.peer_public_key.public_key_to_der().unwrap();
        let key_from_der = EcKey::public_key_from_der(&der_form).unwrap();
        let mut bn_ctx = openssl::bn::BigNumContext::new().unwrap();

        let raw_pub_key = key_from_der.public_key().to_bytes(
            &group,
            openssl::ec::PointConversionForm::UNCOMPRESSED,
            &mut bn_ctx  // Korjattu: annetaan muuttuva viittaus
        ).unwrap();        return raw_pub_key;
    }    
    /// 🔹 Palauta yksityinen avain tavumuodossa
    pub fn private_key_to_bytes(&self) -> Vec<u8> {
        self.private_key.private_key_to_der().expect("Failed to serialize private key")
    }

    /// 🔹 Luo julkinen avain tavumuodosta
    pub fn bytes_to_public_key(&self, public_key_bytes: &[u8]) -> EcKey<Public> {
      //  let output_public_key =  EcKey::from_public_key(&group, private_key.public_key()).expect("Failed to create public key");
        let output_public_key =  EcKey::public_key_from_der(public_key_bytes).expect("Failed to create public key");
        output_public_key
    }

    pub fn derive_shared_key_based_aes_key(&self, param_shared_secret: Vec<u8>) -> Vec<u8> {
        let mut okm = vec![0; STCP_AES_KEY_SIZE_IN_BYTES];
        let info = b"stcp aes key";
        let mut prk_hash = Sha256::new();
        prk_hash.update(&param_shared_secret);
        let prk = prk_hash.finish();
    
        let mut t1_hash = Sha256::new();
        t1_hash.update(&[0x01]);
        t1_hash.update(&prk);
        t1_hash.update(info);
        let t1 = t1_hash.finish();
    
        let mut t2_hash = Sha256::new();
        t2_hash.update(&[0x02]);
        t2_hash.update(&prk);
        t2_hash.update(&t1);
        let t2 = t2_hash.finish();
    
        okm[..t1.len()].copy_from_slice(&t1);
        if STCP_AES_KEY_SIZE_IN_BYTES > t1.len() {
            okm[t1.len()..].copy_from_slice(&t2[..STCP_AES_KEY_SIZE_IN_BYTES - t1.len()]);
        }
        dprint!("AES Key: from {:?} // Derived AES Key: {:?} // Length: {}", param_shared_secret , okm, okm.len());
        okm
    }    
    /// 🔹 Laske jaettu salaisuus annetusta julkisesta avaimesta
    pub fn compute_shared_secret(&self, peer_public: EcKey<Public>) -> Vec<u8> {
        let pkey_private = PKey::from_ec_key(self.private_key.to_owned()).expect("Failed to convert to PKey");
        let pkey_peer = PKey::from_ec_key(peer_public.to_owned()).expect("Failed to convert peer key to PKey");

        let mut deriver = Deriver::new(&pkey_private).expect("Failed to create Deriver");
        deriver.set_peer(&pkey_peer).expect("Failed to set peer key");

        deriver.derive_to_vec().expect("Failed to compute shared secret")
    }

    pub fn check_for_public_key(&mut self,
        param_data_in: Option<&[u8]>,
    ) -> Option<(Vec<u8>, Vec<u8>)> {
        if let Some(data) = param_data_in {
            let msg_len = data.len();
            dprint!("Got Public key? {} .. {:x?}", msg_len, data);
    
            if msg_len == 65 {
                dprint!("Content is proper length ...");
                let has_pubkey = data[0] == 4;
                dprint!("Got Public key? {} && byte: {} => {}", msg_len, data[0], has_pubkey);
    
                if has_pubkey {
                    let the_peer_public_key = self.raw_public_key_to_proper_public_key(data).unwrap();
                    if the_peer_public_key.check_key().is_ok() {
                        self.peer_public_key = the_peer_public_key.clone();
                        let peer_public_key_bytes = the_peer_public_key.public_key_to_der().expect("Failed to serialize peer's public key");
                        self.shared_secret = self.compute_shared_secret(the_peer_public_key);
                        dprint!("Handshake return PSK: {} // {:x?} //", self.shared_secret.len(), self.shared_secret);
                        return Some((peer_public_key_bytes, self.shared_secret.clone()));
                    }
                }
                dprint!("Handshake failed");
                return None;
            } else {
                dprint!("Not public key.. length mismatch: {}", msg_len);
                return None;
            }
        }
        None
    }

}
