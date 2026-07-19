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
    byte_queue::ByteQueue,
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
    pub a_to_b: SpinLock<ByteQueue>,
    pub b_to_a: SpinLock<ByteQueue>,
    pub a_closed: AtomicBool,
    pub b_closed: AtomicBool,
    pub owner_a: AtomicUsize,
    pub owner_b: AtomicUsize,
}

impl Connection {
    pub fn new() -> Self {
        Self {
            a_to_b: SpinLock::new(ByteQueue::new()),
            b_to_a: SpinLock::new(ByteQueue::new()),
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
    pub bytes: Arc<[u8]>,
    pub age_ticks: u32,
    pub rto_ticks: u32,
    pub retries: u8,
    pub retransmitted: bool,
}

#[derive(Debug, Clone, Copy)]
pub struct ReliabilityStats {
    pub sent_frames: u64,
    pub acknowledged_frames: u64,
    pub retransmitted_frames: u64,
    pub duplicate_frames: u64,
    pub reordered_frames: u64,
    pub timeout_failures: u64,
    pub rtt_samples: u64,
}

impl ReliabilityStats {
    pub const fn new() -> Self {
        Self {
            sent_frames: 0,
            acknowledged_frames: 0,
            retransmitted_frames: 0,
            duplicate_frames: 0,
            reordered_frames: 0,
            timeout_failures: 0,
            rtt_samples: 0,
        }
    }
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
    pub srtt_ms: Option<u32>,
    pub rttvar_ms: u32,
    pub rto_ms: u32,
    pub stats: ReliabilityStats,
    pub pending_frames: VecDeque<PendingFrame>,
    pub out_of_order_frames: Vec<BufferedFrame>,
    pub last_rx_sequence: Option<u64>,
    pub rx_app_data: ByteQueue,
    pub rx_message_ready: bool,
    pub peer_eof: bool,
    pub carrier: usize,
    pub connection_id: u64,
    pub udp_peer_addr: u32,
    pub udp_peer_port: u16,
}

pub struct StcpContext {
    pub proto: u8,
    pub parser_busy: AtomicBool,
    pub inner: SpinLock<ContextInner>,
}

impl StcpContext {
    pub fn new(proto: u8) -> Result<Self, crate::error::StcpError> {
        let crypto = CryptoContext::new()?;

        Ok(Self {
            proto,
            parser_busy: AtomicBool::new(false),
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
                srtt_ms: None,
                rttvar_ms: 0,
                rto_ms: 300,
                stats: ReliabilityStats::new(),
                pending_frames: VecDeque::new(),
                out_of_order_frames: Vec::new(),
                last_rx_sequence: None,
                rx_app_data: ByteQueue::new(),
                rx_message_ready: false,
                peer_eof: false,
                carrier: 0,
                connection_id: 0,
                udp_peer_addr: 0,
                udp_peer_port: 0,
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
            parser_busy: AtomicBool::new(false),
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
                srtt_ms: None,
                rttvar_ms: 0,
                rto_ms: 300,
                stats: ReliabilityStats::new(),
                pending_frames: VecDeque::new(),
                out_of_order_frames: Vec::new(),
                last_rx_sequence: None,
                rx_app_data: ByteQueue::new(),
                rx_message_ready: false,
                peer_eof: false,
                carrier: 0,
                connection_id: 0,
                udp_peer_addr: 0,
                udp_peer_port: 0,
            }),
        })
    }
}
