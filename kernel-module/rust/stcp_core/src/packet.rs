use crate::crypto::STCP_IV_LEN;

/// STCP: 8 tavun big endian -header: koko viestin pituus (IV+ciphertext)
pub fn write_header(buf: &mut [u8], len: u64) {
    buf[0] = (len >> 56) as u8;
    buf[1] = (len >> 48) as u8;
    buf[2] = (len >> 40) as u8;
    buf[3] = (len >> 32) as u8;
    buf[4] = (len >> 24) as u8;
    buf[5] = (len >> 16) as u8;
    buf[6] = (len >> 8) as u8;
    buf[7] = len as u8;
}

pub fn read_header(buf: &[u8]) -> u64 {
    ((buf[0] as u64) << 56)
        | ((buf[1] as u64) << 48)
        | ((buf[2] as u64) << 40)
        | ((buf[3] as u64) << 32)
        | ((buf[4] as u64) << 24)
        | ((buf[5] as u64) << 16)
        | ((buf[6] as u64) << 8)
        | (buf[7] as u64)
}

/// Rakenne: [8 hdr][16 IV][cipher...]
pub const STCP_HEADER_LEN: usize = 8;
pub const STCP_META_LEN: usize = STCP_HEADER_LEN + STCP_IV_LEN;
