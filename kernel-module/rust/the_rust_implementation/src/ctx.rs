#![allow(dead_code)]

use core::ffi::{c_void, c_int, c_ulong};

use stcp_core::{StcpCtx, StcpHandshake};
use core::mem::{size_of, MaybeUninit};

use crate::error::{ENOPROTOOPT, ENOTTY};

use crate::abi::{
    stcp_rust_blob_get,
    stcp_rust_blob_set,
    stcp_rust_blob_lock,
    stcp_rust_blob_unlock,
    stcp_rust_kernel_alloc,
    stcp_rust_kernel_free,
};

//use crate::handshake::StcpHandshake; // olettaen että sulla on tämä tyyppi

/// Per-socket STCP-tila, jonka osoitin on talletettu kernelin blobbiin.
///
/// Tämä rakenne on täysin Rustin hallussa: kernel ei rakenna sitä itse,
/// vaan antaa meille vain blob-slotin (stcp_rust_blob_* -API).
#[repr(C)]
pub struct StcpContext {
    /// Ydinprotokollan tila (handshake/AES jne), stcp-core crate.
    pub core: StcpCtx,
    handshake: MaybeUninit<StcpHandshake>,
    handshake_ready: bool,
}

impl StcpContext {
    pub const fn new() -> Self {
        Self {
            core: StcpCtx::new(),
            handshake: MaybeUninit::uninit(),
            handshake_ready: false,
        }
    }

    fn ensure_handshake(&mut self) -> &mut StcpHandshake {
        if !self.handshake_ready {
            self.handshake.write(StcpHandshake::new());
            self.handshake_ready = true;
        }
        unsafe { self.handshake.assume_init_mut() }
    }

    fn clear_handshake(&mut self) {
        if self.handshake_ready {
            // Jos haluat oikeasti droppaa:
            // unsafe { self.handshake.assume_init_drop(); }
            self.handshake_ready = false;
        }
    }

    // ===== Hook-API, joita FFI-puoli voi kutsua =====

    pub fn on_close(&mut self) {
        self.clear_handshake();
        // tarvittaessa nollaa core-tila tms.
    }

    pub fn on_bind(&mut self, _uaddr: *const c_void, _addr_len: c_int) {
        // TODO: tallenna bind-infot jos tarvitsee
    }

    pub fn on_listen(&mut self) {
        // TODO: merkitse server-rooli jos tarpeen
    }

    /// Client connect -polku: alustaa handshaken.
    pub fn on_connect_client(
        &mut self,
        _uaddr: *const c_void,
        _addr_len: c_int,
        _flags: c_int,
    ) -> c_int {
        let _client = self.ensure_handshake().start_client();
        // TODO: puskuroi/ lähetä pubkey ekassa sendmsgissä jne.
        0
    }

    pub fn on_accept_server(&mut self, _lsk: *mut c_void) {
        let _ = self.ensure_handshake();
        // TODO: server-puolen init
    }

    pub fn on_getname(
        &mut self,
        _uaddr: *mut c_void,
        _addr_len: *mut c_int,
        _peer: c_int,
    ) {
        // TODO: getsockname/getpeername jos haluat
    }

    pub fn on_sendmsg(
        &mut self,
        _msg: *mut c_void,
        _flags: c_int,
    ) -> c_int {
        // TODO: integroi encrypt_payload + header
        0
    }

    pub fn on_recvmsg(
        &mut self,
        _msg: *mut c_void,
        _len: usize,
        _flags: c_int,
    ) -> c_int {
        // TODO: dekryptaa / feedaa handshakea
        0
    }

    pub fn on_setsockopt(
        &mut self,
        _level: c_int,
        _optname: c_int,
        _optval: *const c_void,
        _optlen: c_int,
    ) -> c_int {
        -ENOPROTOOPT
    }

    pub fn on_getsockopt(
        &mut self,
        _level: c_int,
        _optname: c_int,
        _optval: *mut c_void,
        _optlen: *mut c_int,
    ) -> c_int {
        -ENOPROTOOPT
    }

    pub fn on_poll(
        &mut self,
        _file: *mut c_void,
        _wait: *mut c_void,
    ) -> u32 {
        0
    }

    pub fn on_ioctl(
        &mut self,
        _cmd: c_ulong,
        _arg: c_ulong,
    ) -> c_int {
        -ENOTTY
    }
}

// ===== Helperit: blobbi <-> StcpContext =====

/// Hae tai rakenna uusi StcpContext tälle socketille.
/// Tämä on ainoa paikka, jossa allokoidaan per-socket STCP-tila.
pub unsafe fn get_or_init_ctx(sock_ctx: *mut c_void) -> *mut StcpContext {
    if sock_ctx.is_null() {
        return core::ptr::null_mut();
    }

    stcp_rust_blob_lock(sock_ctx);

    let mut ptr = stcp_rust_blob_get(sock_ctx) as *mut StcpContext;

    if ptr.is_null() {
        let raw = stcp_rust_kernel_alloc(size_of::<StcpContext>()) as *mut StcpContext;
        if raw.is_null() {
            stcp_rust_blob_unlock(sock_ctx);
            return core::ptr::null_mut();
        }
        raw.write(StcpContext::new());
        ptr = raw;
        stcp_rust_blob_set(sock_ctx, ptr as *mut c_void);
    }

    stcp_rust_blob_unlock(sock_ctx);
    ptr
}

/// Vapauta tämän socketin StcpContext.
/// Tätä kutsutaan close-polussa Rustin toimesta.
pub unsafe fn free_ctx_for_socket(sock_ctx: *mut c_void) {
    if sock_ctx.is_null() {
        return;
    }

    stcp_rust_blob_lock(sock_ctx);
    let ptr = stcp_rust_blob_get(sock_ctx) as *mut StcpContext;
    if !ptr.is_null() {
        stcp_rust_blob_set(sock_ctx, core::ptr::null_mut());
        stcp_rust_blob_unlock(sock_ctx);
        stcp_rust_kernel_free(ptr as *mut c_void);
    } else {
        stcp_rust_blob_unlock(sock_ctx);
    }
}

/// Aja `f` lukon alla, jos konteksti löytyy.
pub unsafe fn with_context_lock<F: FnOnce(&mut StcpContext)>(sk: *mut c_void, f: F) {
    if sk.is_null() {
        return;
    }

    stcp_rust_blob_lock(sk);
    let ctx = stcp_rust_blob_get(sk) as *mut StcpContext;
    if !ctx.is_null() {
        f(&mut *ctx);
    }
    stcp_rust_blob_unlock(sk);
}
