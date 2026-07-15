use alloc::{
    boxed::Box,
    collections::VecDeque,
    sync::Arc,
    vec::Vec,
};

use core::sync::atomic::{
    AtomicBool,
    AtomicUsize,
    Ordering,
};

use crate::{
    crypto::{CryptoContext, Role},
    frame::Header,
    spinlock::SpinLock,
};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SocketState {
    New,
    Bound,
    Listening,
    Handshake,
    Ready,
    Closed,
    Error,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Address {
    pub addr: u32,
    pub port: u16,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Side {
    A,
    B,
}

pub struct Connection {
    pub a_to_b: SpinLock<VecDeque<u8>>,
    pub b_to_a: SpinLock<VecDeque<u8>>,
    pub a_closed: AtomicBool,
    pub b_closed: AtomicBool,
    pub owner_a: AtomicUsize,
    pub owner_b: AtomicUsize,
}

impl Connection {
    pub fn new() -> Self {
        Self {
            a_to_b: SpinLock::new(VecDeque::new()),
            b_to_a: SpinLock::new(VecDeque::new()),
            a_closed: AtomicBool::new(false),
            b_closed: AtomicBool::new(false),
            owner_a: AtomicUsize::new(0),
            owner_b: AtomicUsize::new(0),
        }
    }

    pub fn set_owner(&self, side: Side, owner: usize) {
        match side {
            Side::A => self.owner_a.store(owner, Ordering::Release),
            Side::B => self.owner_b.store(owner, Ordering::Release),
        }
    }

    pub fn peer_owner(&self, side: Side) -> usize {
        match side {
            Side::A => self.owner_b.load(Ordering::Acquire),
            Side::B => self.owner_a.load(Ordering::Acquire),
        }
    }

    pub fn close(&self, side: Side) {
        match side {
            Side::A => self.a_closed.store(true, Ordering::Release),
            Side::B => self.b_closed.store(true, Ordering::Release),
        }
    }

    pub fn peer_closed(&self, side: Side) -> bool {
        match side {
            Side::A => self.b_closed.load(Ordering::Acquire),
            Side::B => self.a_closed.load(Ordering::Acquire),
        }
    }
}

pub struct EndpointConnection {
    pub shared: Arc<Connection>,
    pub side: Side,
}

pub struct PendingFrame {
    pub sequence: u64,
    pub bytes: Vec<u8>,
    pub age_ticks: u32,
    pub retries: u8,
}

pub struct BufferedFrame {
    pub header: Header,
    pub nonce: u64,
    pub ciphertext: Vec<u8>,
}

pub struct ContextInner {
    pub state: SocketState,
    pub local: Option<Address>,
    pub peer: Option<Address>,
    pub backlog: usize,
    pub accept_queue: VecDeque<Box<StcpContext>>,
    pub connection: Option<EndpointConnection>,
    pub owner: usize,
    pub crypto: CryptoContext,
    pub role: Role,
    pub peer_handshake_done: bool,
    pub tx_nonce: u64,
    pub expected_rx_nonce: u64,
    pub tx_sequence: u64,
    pub expected_rx_sequence: u64,
    pub highest_acked_sequence: Option<u64>,
    pub pending_frames: VecDeque<PendingFrame>,
    pub out_of_order_frames: Vec<BufferedFrame>,
    pub last_rx_sequence: Option<u64>,
    pub rx_app_data: VecDeque<u8>,
    pub rx_message_ready: bool,
    pub peer_eof: bool,
}

pub struct StcpContext {
    pub proto: u8,
    pub inner: SpinLock<ContextInner>,
}

impl StcpContext {
    pub fn new(proto: u8) -> Result<Self, crate::error::StcpError> {
        let crypto = CryptoContext::new()?;

        Ok(Self {
            proto,
            inner: SpinLock::new(ContextInner {
                state: SocketState::New,
                local: None,
                peer: None,
                backlog: 0,
                accept_queue: VecDeque::new(),
                connection: None,
                owner: 0,
                crypto,
                role: Role::Client,
                peer_handshake_done: false,
                tx_nonce: 0,
                expected_rx_nonce: 0,
                tx_sequence: 0,
                expected_rx_sequence: 0,
                highest_acked_sequence: None,
                pending_frames: VecDeque::new(),
                out_of_order_frames: Vec::new(),
                last_rx_sequence: None,
                rx_app_data: VecDeque::new(),
                rx_message_ready: false,
                peer_eof: false,
            }),
        })
    }

    pub fn connected_child(
        proto: u8,
        local: Address,
        peer: Address,
        shared: Arc<Connection>,
    ) -> Result<Self, crate::error::StcpError> {
        let crypto = CryptoContext::new()?;

        Ok(Self {
            proto,
            inner: SpinLock::new(ContextInner {
                state: SocketState::Handshake,
                local: Some(local),
                peer: Some(peer),
                backlog: 0,
                accept_queue: VecDeque::new(),
                connection: Some(EndpointConnection {
                    shared,
                    side: Side::B,
                }),
                owner: 0,
                crypto,
                role: Role::Server,
                peer_handshake_done: false,
                tx_nonce: 0,
                expected_rx_nonce: 0,
                tx_sequence: 0,
                expected_rx_sequence: 0,
                highest_acked_sequence: None,
                pending_frames: VecDeque::new(),
                out_of_order_frames: Vec::new(),
                last_rx_sequence: None,
                rx_app_data: VecDeque::new(),
                rx_message_ready: false,
                peer_eof: false,
            }),
        })
    }
}
