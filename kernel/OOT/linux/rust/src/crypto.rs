
use crate::types::{
        ProtoSession,
        StcpEcdhPubKey,
        StcpEcdhSecret,
        STCP_ECDH_PUB_LEN,
    };

use crate::abi;
use alloc::boxed::Box;

pub struct Crypto;
use crate::stcp_dbg;

impl Crypto {
    /// Täyttää sessioon uuden ECDH-priv+pub -avaimen (C-crypto).
    pub fn generate_keypair(sess: &mut ProtoSession) -> i32 {
      stcp_dbg!("Starting keypair generation");   
      sess.private_key.len = 0;
      sess.shared_key.len  = 0;
      stcp_dbg!("Lenghts set 0");   

      stcp_dbg!("Calling C keypair creation");   
      let mut ret: i32 = -22; // EINVAL

      let mut pPubKey = &mut sess.public_key;
      let mut pPrivKey = &mut sess.private_key;

      unsafe {

        ret = abi::stcp_crypto_generate_keypair(
          pPubKey,    // *mut StcpEcdhPubKey
          pPrivKey,   // *mut StcpEcdhSecret
        );

        stcp_dbg!("Called C keypair creation, ret {}", ret);   
        ret
      }
    }

    /// Laskee jaetun salaisuuden sessioon peerin public key -bytestä (X||Y).
    pub fn compute_shared_from_bytes(
        sess: &mut ProtoSession,
        peer_pub_bytes: &[u8; STCP_ECDH_PUB_LEN],
    ) -> i32 {
        stcp_dbg!("Starting to create shared key");   

        let mut peer = StcpEcdhPubKey::new();

        peer.x.copy_from_slice(&peer_pub_bytes[0..32]);
        peer.y.copy_from_slice(&peer_pub_bytes[32..64]);

        stcp_dbg!("Peer key prepared");   

        stcp_dbg!("Calling C shared key calc...");   
        let mut ret: i32 = -22; // EINVAL
        unsafe {
            ret = abi::stcp_crypto_compute_shared(
                &sess.private_key,   // *const StcpEcdhSecret
                &peer,               // *const StcpEcdhPubKey
                &mut sess.shared_key,    // *mut StcpEcdhSecret
            );
        }
        stcp_dbg!("Called C shared key calc, ret {}", ret);   
        ret
    }
}

