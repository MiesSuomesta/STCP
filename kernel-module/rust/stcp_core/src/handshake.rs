#![allow(dead_code)]

use core::convert::TryInto;

use crate::crypto::{derive_shared, StcpKeys, StcpTestRng};
use crate::types::{StcpCtx, StcpPhase};

/// STCP handshake -protokollan viestityypit
pub const STCP_MSG_CLIENT_HELLO: u8 = 0x01;
pub const STCP_MSG_SERVER_HELLO: u8 = 0x02;

/// Deterministiset RNG-seedit smoketestiä varten.
/// HUOM: vain testikäyttöön; korvaa oikealla kernel-RNG:llä myöhemmin.
pub const STCP_CLIENT_RNG_SEED: u64 = 0xC1A0_5EC0_DEAD_BEEF;
pub const STCP_SERVER_RNG_SEED: u64 = 0x5EC0_CAFE_BABE_1234;

/// Julkisen avaimen pituus x25519:ssä
pub const STCP_PUBKEY_LEN: usize = 32;
/// Handshake-viestin koko: [type][pubkey]
pub const STCP_HELLO_LEN: usize = 1 + STCP_PUBKEY_LEN;

/// Abstrakti IO-rajapinta, jonka the_rust_implementation toteuttaa
/// kernel-puolella socket-hookkien avulla.
///
/// Virheet palautetaan negatiivisina errno-arvoina (esim. -EIO, -EINVAL).
pub trait StcpIo {
    /// Lähetä tarkalleen `buf.len()` tavua.
    fn send_all(&mut self, buf: &[u8]) -> Result<(), i32>;

    /// Lue tarkalleen `buf.len()` tavua.
    fn recv_exact(&mut self, buf: &mut [u8]) -> Result<(), i32>;
}

/// Client-puolen handshake:
/// 1. Luo omat X25519-avaimet
/// 2. Lähettää CLIENT_HELLO (0x01 + client_pub[32])
/// 3. Lukee SERVER_HELLO (0x02 + server_pub[32])
/// 4. Laskee ja tallentaa yhteisen avaimen
/// 5. Vaihtaa vaiheeseen `Secure`
pub fn client_handshake<I: StcpIo>(
    ctx: &mut StcpCtx,
    io: &mut I,
) -> Result<(), i32> {
    let mut rng = StcpTestRng::new(STCP_CLIENT_RNG_SEED);
    let keys = StcpKeys::generate_with(&mut rng);

    // --- CLIENT_HELLO ---
    let mut ch = [0u8; STCP_HELLO_LEN];
    ch[0] = STCP_MSG_CLIENT_HELLO;
    ch[1..].copy_from_slice(&keys.public);

    io.send_all(&ch)?;
    ctx.set_phase(StcpPhase::ClientHelloSent);

    // --- SERVER_HELLO ---
    let mut sh = [0u8; STCP_HELLO_LEN];
    io.recv_exact(&mut sh)?;

    if sh[0] != STCP_MSG_SERVER_HELLO {
        return Err(-22); // -EINVAL
    }

    let server_pub_bytes: [u8; STCP_PUBKEY_LEN] = match sh[1..].try_into() {
        Ok(b) => b,
        Err(_) => return Err(-22),
    };

    let shared = derive_shared(&keys.secret, &server_pub_bytes);
    ctx.shared_key.copy_from_slice(&shared);

    ctx.set_phase(StcpPhase::Secure);
    Ok(())
}

/// Server-puolen handshake:
/// 1. Lukee CLIENT_HELLO (0x01 + client_pub[32])
/// 2. Luo omat avaimet
/// 3. Laskee yhteisen avaimen
/// 4. Lähettää SERVER_HELLO (0x02 + server_pub[32])
/// 5. Vaihtaa vaiheeseen `Secure`
pub fn server_handshake<I: StcpIo>(
    ctx: &mut StcpCtx,
    io: &mut I,
) -> Result<(), i32> {
    // --- CLIENT_HELLO ---
    let mut ch = [0u8; STCP_HELLO_LEN];
    io.recv_exact(&mut ch)?;

    if ch[0] != STCP_MSG_CLIENT_HELLO {
        return Err(-22); // -EINVAL
    }

    let client_pub_bytes: [u8; STCP_PUBKEY_LEN] = match ch[1..].try_into() {
        Ok(b) => b,
        Err(_) => return Err(-22),
    };

    let mut rng = StcpTestRng::new(STCP_SERVER_RNG_SEED);
    let keys = StcpKeys::generate_with(&mut rng);

    let shared = derive_shared(&keys.secret, &client_pub_bytes);
    ctx.shared_key.copy_from_slice(&shared);

    // --- SERVER_HELLO ---
    let mut sh = [0u8; STCP_HELLO_LEN];
    sh[0] = STCP_MSG_SERVER_HELLO;
    sh[1..].copy_from_slice(&keys.public);

    io.send_all(&sh)?;

    ctx.set_phase(StcpPhase::Secure);
    Ok(())
}
