#![allow(dead_code)]
#![allow(improper_ctypes)]

use core::ffi::{c_void, c_char, c_int};
use crate::types::{
        StcpEcdhPubKey,
        StcpEcdhSecret,
    };

unsafe extern "C" {

    //int stcp_end_of_life_for_sk(void *skvp, int err);
    pub fn stcp_end_of_life_for_sk(skvp: *mut c_void, err: c_int) -> i32;

    // Memory allocation
    pub fn stcp_rust_kernel_alloc(size: usize) -> *mut c_void;
    pub fn stcp_rust_kernel_free(ptr: *mut c_void);

    // TCP raw send/recv
    pub fn stcp_tcp_raw_send(sk: *mut c_void,
                            buf: *const u8,
                            len: usize) -> c_int;

    pub fn stcp_tcp_raw_recv(sk: *mut c_void,
                            buf: *mut u8,
                            len: usize,
                            flags: c_int,
                            recv_len: &mut c_int) -> c_int;

    // Bugi handleri (kontekstille)
    pub fn stcp_bug_null_ctx(sk: *mut c_void) -> !;

    
    //
    // Kernelin crypto puolen funkkarit
    //

    pub fn  stcp_misc_ecdh_public_key_new() -> * mut c_void;
    pub fn stcp_misc_ecdh_private_key_new() -> * mut c_void;
    pub fn  stcp_misc_ecdh_shared_key_new() -> * mut c_void;

    pub fn  stcp_misc_ecdh_public_key_size() -> i32;
    pub fn stcp_misc_ecdh_private_key_size() -> i32;
    pub fn  stcp_misc_ecdh_shared_key_size() -> i32;

    pub fn stcp_misc_ecdh_key_free(key: *mut c_void);

    pub fn stcp_crypto_generate_keypair(
        out_pub: *mut StcpEcdhPubKey,
        out_priv: *mut StcpEcdhSecret,
    ) -> i32;

    pub fn stcp_crypto_compute_shared(
        priv_key: *const StcpEcdhSecret,
        peer_pub: *const StcpEcdhPubKey,
        out_shared: *mut StcpEcdhSecret,
    ) -> i32;

    //
    // Conteksti countterit
    //
    pub fn stcp_exported_rust_ctx_alive_count() -> i32;

}
