//! Wire-v3 authenticated controls. DATA and controls use separate keys/nonces.
use alloc::vec::Vec;
use crate::{crypto::CryptoContext, error::StcpError, frame::{Header, PacketType, STCP_HEADER_LEN}};

pub const CONTROL_PAYLOAD_LEN: usize = 8 + 16;

pub fn is_control(kind: PacketType) -> bool {
    matches!(kind, PacketType::Ack | PacketType::Ping | PacketType::Pong |
                   PacketType::Close | PacketType::Reset)
}

pub fn encode(crypto: &CryptoContext, kind: PacketType, sequence: u64,
              acknowledgment: u64, connection_id: u64, nonce: u64)
              -> Result<Vec<u8>, StcpError> {
    if !is_control(kind) { return Err(StcpError::Protocol); }
    let header = Header::with_numbers(kind, CONTROL_PAYLOAD_LEN, sequence,
                                      acknowledgment, connection_id)?.encode();
    let tag = crypto.control_tag(nonce, &header)?;
    let mut frame = Vec::new();
    frame.try_reserve_exact(STCP_HEADER_LEN + CONTROL_PAYLOAD_LEN)
         .map_err(|_| StcpError::NoMem)?;
    frame.extend_from_slice(&header);
    frame.extend_from_slice(&nonce.to_be_bytes());
    frame.extend_from_slice(&tag);
    Ok(frame)
}

/// Verify first; the caller commits the replay window only on Some(nonce).
pub fn verify(crypto: &CryptoContext, header: &Header, payload: &[u8],
              connection_id: u64) -> Option<u64> {
    if !is_control(header.packet_type) || header.flags != 0 ||
       header.connection_id != connection_id ||
       header.payload_len != CONTROL_PAYLOAD_LEN || payload.len() != CONTROL_PAYLOAD_LEN {
        return None;
    }
    let nonce = u64::from_be_bytes(payload[..8].try_into().ok()?);
    if !crypto.verify_control_tag(nonce, &header.encode(), &payload[8..]) {
        return None;
    }
    Some(nonce)
}

pub struct ReplayWindow {
    highest: Option<u64>,
    seen: u64,
}

impl ReplayWindow {
    pub const fn new() -> Self { Self { highest: None, seen: 0 } }

    /// Call only after authentication. Bit 0 tracks highest, bit 63 highest-63.
    pub fn accept(&mut self, nonce: u64) -> bool {
        match self.highest {
            None => { self.highest = Some(nonce); self.seen = 1; true }
            Some(highest) if nonce > highest => {
                let distance = nonce - highest;
                self.seen = if distance >= 64 { 1 } else { (self.seen << distance) | 1 };
                self.highest = Some(nonce);
                true
            }
            Some(highest) => {
                let distance = highest - nonce;
                if distance >= 64 { return false; }
                let bit = 1u64 << distance;
                if self.seen & bit != 0 { return false; }
                self.seen |= bit;
                true
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn replay_reorder_and_edges() {
        let mut w = ReplayWindow::new();
        assert!(w.accept(0)); assert!(!w.accept(0));
        assert!(w.accept(2)); assert!(w.accept(1)); assert!(!w.accept(1));
        assert!(w.accept(65)); assert!(w.accept(3)); assert!(!w.accept(1));
        assert!(w.accept(u64::MAX)); assert!(!w.accept(u64::MAX));
        assert!(w.accept(u64::MAX - 63)); assert!(!w.accept(u64::MAX - 64));
    }
}
