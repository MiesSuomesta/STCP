
use crate::types::{
        StcpEcdhPubKey,
        StcpEcdhSecret,
        STCP_ECDH_PUB_LEN,
    };

use crate::abi;
use core::panic::Location;
use crate::proto_session::ProtoSession;
use crate::proto_session;



pub struct Crypto;
use crate::stcp_dbg;
use crate::stcp_dump;

impl Crypto {
    /// Täyttää sessioon uuden ECDH-priv+pub -avaimen (C-crypto).
    pub fn generate_keypair(sess: &mut ProtoSession) -> i32 {
      stcp_dbg!("Starting keypair generation");   
      stcp_dbg!("Sess ptr inside generate_keypair: {:?}", sess as *mut _);
      stcp_dbg!("Calling C keypair creation");   
      let mut ret: i32 = 0;

      let the_public_key = &mut sess.public_key;
      let the_private_key = &mut sess.private_key;
      stcp_dbg!("sizeof pubkey {}", core::mem::size_of::<StcpEcdhPubKey>());
      stcp_dbg!("sizeof secret {}", core::mem::size_of::<StcpEcdhSecret>());

      unsafe {

        ret = abi::stcp_crypto_generate_keypair(
          the_public_key,    // *mut StcpEcdhPubKey
          the_private_key,   // *mut StcpEcdhSecret
        );

        stcp_dbg!("Session for server: {:?}", sess.is_server);
        stcp_dump!("Public  key X", &sess.public_key.x);
        stcp_dump!("Public  key Y", &sess.public_key.y);
        stcp_dump!("Private key  ", &sess.private_key.data);

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

        if peer_pub_bytes.iter().all(|&b| b == 0) {
            stcp_dbg!("❌ Peer public key is all zeros – aborting ECDH");
            return -1;
        }

        let mut peer = StcpEcdhPubKey::new();

        peer.x.copy_from_slice(&peer_pub_bytes[0..32]);
        peer.y.copy_from_slice(&peer_pub_bytes[32..64]);

        stcp_dump!("Peer pub X", &peer.x);
        stcp_dump!("Peer pub Y", &peer.y);

        stcp_dbg!("Peer key prepared");   

        stcp_dbg!("Calling C shared key calc...");   
        unsafe {
          let ret: i32 = abi::stcp_crypto_compute_shared(
              &sess.private_key,   // *const StcpEcdhSecret
              &peer,               // *const StcpEcdhPubKey
              &mut sess.shared_key,    // *mut StcpEcdhSecret
          );
          stcp_dbg!("Called C shared key calc, ret {}", ret);   
          stcp_dump!("Peer shared key", &sess.shared_key.to_bytes_be());
          ret
        }
    }
}

