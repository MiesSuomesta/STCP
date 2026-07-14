use alloc::{
    boxed::Box,
    collections::VecDeque,
    sync::Arc,
};

use core::sync::atomic::{
    AtomicBool,
    AtomicUsize,
    Ordering,
};

use crate::spinlock::SpinLock;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SocketState {
    New,
    Bound,
    Listening,
    Connected,
    Closed,
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

pub struct ContextInner {
    pub state: SocketState,
    pub local: Option<Address>,
    pub peer: Option<Address>,
    pub backlog: usize,
    pub accept_queue: VecDeque<Box<StcpContext>>,
    pub connection: Option<EndpointConnection>,
    pub owner: usize,
}

pub struct StcpContext {
    pub proto: u8,
    pub inner: SpinLock<ContextInner>,
}

impl StcpContext {
    pub fn new(proto: u8) -> Self {
        Self {
            proto,
            inner: SpinLock::new(ContextInner {
                state: SocketState::New,
                local: None,
                peer: None,
                backlog: 0,
                accept_queue: VecDeque::new(),
                connection: None,
                owner: 0,
            }),
        }
    }

    pub fn connected_child(
        proto: u8,
        local: Address,
        peer: Address,
        shared: Arc<Connection>,
    ) -> Self {
        Self {
            proto,
            inner: SpinLock::new(ContextInner {
                state: SocketState::Connected,
                local: Some(local),
                peer: Some(peer),
                backlog: 0,
                accept_queue: VecDeque::new(),
                connection: Some(EndpointConnection {
                    shared,
                    side: Side::B,
                }),
                owner: 0,
            }),
        }
    }
}
