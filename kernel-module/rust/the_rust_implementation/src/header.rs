extern crate alloc;
//use alloc::vec::Vec;
use crate::error::{Result, StcpError};

pub const STCP_MAGIC: u32 = 0xDEADBEEF;
pub const STCP_HEADER_LEN: usize = 12; // 4 (magic) + 8 (len)

/// Rakenna STCP-header: MAGIC (BE) + len (BE u64).
#[inline]
pub fn build_header(len: u64) -> [u8; STCP_HEADER_LEN] {
    let mut h = [0u8; STCP_HEADER_LEN];
    h[..4].copy_from_slice(&STCP_MAGIC.to_be_bytes());
    h[4..12].copy_from_slice(&len.to_be_bytes());
    h
}

/// Parsii STCP-headerin. Tarkistaa magican, palauttaa payload-pituuden.
#[inline]
pub fn parse_header(buf: &[u8]) -> Result<u64> {
    if buf.len() < STCP_HEADER_LEN {
        return Err(StcpError::Proto);
    }

    let mut m = [0u8; 4];
    m.copy_from_slice(&buf[..4]);
    let magic = u32::from_be_bytes(m);
    if magic != STCP_MAGIC {
        return Err(StcpError::Proto);
    }

    let mut l = [0u8; 8];
    l.copy_from_slice(&buf[4..12]);
    let len = u64::from_be_bytes(l);

    Ok(len)
}

/// Valinnainen raja pituudelle DoS:ia vastaan.
#[inline]
pub fn ensure_reasonable_len(len: u64, max: u64) -> Result<()> {
    if len == 0 || len > max {
        return Err(StcpError::MsgSize);
    }
    Ok(())
}
