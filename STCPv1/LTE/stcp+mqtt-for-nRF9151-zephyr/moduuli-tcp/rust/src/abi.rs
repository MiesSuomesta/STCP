#![allow(dead_code)]
#![allow(improper_ctypes)]

use core::ffi::{c_char, c_int, c_void};
use crate::types::{
        StcpEcdhPubKey,
        StcpEcdhSecret,
        ModemState,
    };


#[repr(C)]
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct stcp_ctx {
    _private: [u8; 0],
}

unsafe extern "C" {

    /* ========================================================= */
    /* TRANSPORT / MODEM STATE                                   */
    /* ========================================================= */

    pub fn stcp_transport_get_modem_states(
        plte: *mut c_int,
        ppdn: *mut c_int,
        pip: *mut c_int,
        pradio: *mut c_int,
        connection_ok: *mut c_int,
    ) -> c_int;

    pub fn stcp_pdn_is_active() -> c_int;

    pub fn stcp_radio_is_active() -> c_int;

    pub fn stcp_transport_modem_has_ip() -> c_int;

    pub fn stcp_transport_modem_lte_network_is_up() -> c_int;

    pub fn stcp_transport_pdn_is_active() -> c_int;

    /* ========================================================= */
    /* RESET / STATUS                                            */
    /* ========================================================= */

    pub fn stcp_is_reset_requested() -> c_int;

    pub fn stcp_set_reset_requested();

    /* ========================================================= */
    /* WAIT FUNCTIONS                                            */
    /* ========================================================= */

    pub fn stcp_transport_wait_for_network_up(
        seconds: c_int
    ) -> c_int;

    pub fn stcp_transport_wait_for_radio_connected(
        seconds: c_int
    ) -> c_int;

    pub fn stcp_transport_wait_for_data_path(
        seconds: c_int
    ) -> c_int;

    pub fn stcp_pdn_wait_until_active_or_secs_passed(
        seconds: c_int
    ) -> c_int;

    pub fn stcp_library_wait_until_lte_ready(
        timeout: c_int
    ) -> c_int;

    pub fn stcp_update_cell_event_wait_until_seen_or_secs_passed(
        seconds: c_int
    ) -> c_int;

    pub fn stcp_transport_wait_until_ready(
        seconds: c_int
    ) -> c_int;

    /* ========================================================= */
    /* TRANSPORT CONTROL                                         */
    /* ========================================================= */

    pub fn stcp_transport_init() -> c_int;

    pub fn stcp_transport_connect() -> c_int;

    pub fn stcp_transport_wakeup_radio();

    /* ========================================================= */
    /* LTE / MODEM DEBUG                                         */
    /* ========================================================= */

    pub fn stcp_lte_issue_at_command(
        cmd: *mut c_char
    ) -> c_int;

    pub fn dump_sim_status();

    pub fn dump_modem_full_status();

    /* ========================================================= */
    /* POLL / SOCKET                                             */
    /* ========================================================= */

    pub fn stcp_poll_fd_changes(
        fd: c_int,
        timeout: c_int,
        events: c_int,
    ) -> c_int;

    /* ========================================================= */
    /* CONNECTION                                                */
    /* ========================================================= */

    pub fn stcp_rust_api_transport_get_fd(
        pks_void: *mut c_void
    ) -> c_int;

    pub fn stcp_transport_context_connect(
        ctx: *mut stcp_ctx
    ) -> c_int;

    pub fn stcp_is_context_open_for_fd_io(
        ctx: *mut stcp_ctx
    ) -> i32;

    /* ========================================================= */
    /* IO                                                        */
    /* ========================================================= */
}

unsafe extern "C" {

    //int stcp_end_of_life_for_sk(void *skvp, int err);
    pub fn stcp_end_of_life_for_sk(skvp: *mut c_void, err: c_int) -> i32;

    // Memory allocation
    pub fn stcp_rust_kernel_alloc(size: usize) -> *mut c_void;
    pub fn stcp_rust_kernel_free(ptr: *mut c_void);

    // Bugi handleri (kontekstille)
    pub fn stcp_bug_null_ctx(sk: *mut c_void) -> !;

    pub fn stcp_random_get(buf: *mut u8, len: usize);
    //
    // Kernelin crypto puolen funkkarit
    //
    pub fn stcp_sleep_ms(ms: u32);

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

    pub fn stcp_is_debug_enabled() -> i32;
    
    pub fn stcp_rust_log(level: i32, buf: *const u8, len: usize);

    //
    // Conteksti countterit
    //
    pub fn stcp_exported_rust_ctx_alive_count() -> i32;


    // LTE API
}
 
