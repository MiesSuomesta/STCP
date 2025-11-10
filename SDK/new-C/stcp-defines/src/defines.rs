
// Protokollan maksimi paketin koko (IPv4)
pub const STCP_IPv4_PACKET_PAYLOAD_MAX_SIZE: usize = (1024 * 64) - 1;
pub const STCP_IPv4_PACKET_HEADERS_MAX_SIZE: usize = 60; // 60 bytes
pub const STCP_IPv4_PACKET_MAX_SIZE: usize = STCP_IPv4_PACKET_PAYLOAD_MAX_SIZE - STCP_IPv4_PACKET_HEADERS_MAX_SIZE;


// AES IV/KEY pituudet
pub const STCP_AES_KEY_SIZE_IN_BYTES: usize = 32;
pub const STCP_AES_IV_SIZE_IN_BYTES: usize = 16;
pub const STCP_AES_NONCE_SIZE_IN_BYTES: usize = 12;

pub fn force_link_globals() {

    globals::zephyr_allocator::touchme_alloc();

//    globals::panic::touchme_panic();

}
