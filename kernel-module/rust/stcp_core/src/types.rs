#![allow(dead_code)]

use core::sync::atomic::{AtomicU8, Ordering};

/// STCP-protokollan tilavaihe
#[repr(u8)]
#[derive(Clone, Copy, Debug)]
pub enum StcpPhase {
    Init            = 0,
    ClientHelloSent = 1,
    ServerHelloSent = 2,
    Secure          = 3,
}

impl Default for StcpPhase {
    fn default() -> Self {
        StcpPhase::Init
    }
}

/// Yhteyden konteksti (C-ABI yhteensopiva rakenne)
/// Tätä käytetään sekä kernel- että userland-puolella
#[repr(C)]
#[derive(Debug)]
pub struct StcpCtx {
    pub id: u64,
    pub state: u32,
    pub shared_key: [u8; 32],
    // tän ympärille voit myöhemmin lisätä muut kentät (phase tms.)
}

impl StcpCtx {
    pub const fn new() -> Self {
        Self {
            id: 0,
            state: 0,
            shared_key: [0; 32],
        }
    }

    pub fn set_phase(&mut self, _p: StcpPhase) {
        // TODO: oikea phase-logiikka
    }
}

/// Kättelyn tilaa ylläpitävä rakenne.
/// Tätä käytetään STCP:n client/server-handshakeissa.
#[repr(C)]
#[derive(Debug)]
pub struct StcpHandshake {
    phase: AtomicU8,
    pub client_pubkey: [u8; 32],
    pub server_pubkey: [u8; 32],
    pub shared_key:   [u8; 32],
}

impl Default for StcpHandshake {
    fn default() -> Self {
        Self {
            phase: AtomicU8::new(StcpPhase::Init as u8),
            client_pubkey: [0; 32],
            server_pubkey: [0; 32],
            shared_key: [0; 32],
        }
    }
}

impl StcpHandshake {
    /// Luo uusi tyhjä handshake-rakenne (Init-tilassa)
    pub fn new() -> Self {
        Self::default()
    }

    /// Palauttaa nykyisen vaiheen atomisesti
    pub fn phase(&self) -> StcpPhase {
        match self.phase.load(Ordering::Acquire) {
            1 => StcpPhase::ClientHelloSent,
            2 => StcpPhase::ServerHelloSent,
            3 => StcpPhase::Secure,
            _ => StcpPhase::Init,
        }
    }

    /// Asettaa uuden vaiheen atomisesti
    pub fn set_phase(&self, p: StcpPhase) {
        self.phase.store(p as u8, Ordering::Release);
    }

    /// Aloittaa client-puolen kättelyn
    /// (tässä placeholder, päivitä kun logiikka valmis)
    pub fn start_client(&mut self) -> &mut Self {
        self.set_phase(StcpPhase::ClientHelloSent);
        self
    }
}
