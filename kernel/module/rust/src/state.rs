use alloc::{
    boxed::Box,
    collections::VecDeque,
    vec::Vec,
};

use core::sync::atomic::{
    AtomicBool,
    AtomicUsize,
    Ordering,
};

use crate::{
    byte_queue::ByteQueue,
    compression::CompressionConfig,
    crypto::{CryptoContext, Role},
    frame::Header,
    spinlock::SpinLock,
};



#[derive(Debug, Clone, Copy, Default)]
pub struct CompressionStats {
    pub tx_attempts: u64,
    pub tx_compressed_frames: u64,
    pub tx_fallback_frames: u64,
    pub tx_input_bytes: u64,
    pub tx_wire_bytes: u64,
    pub tx_errors: u64,
    pub rx_compressed_frames: u64,
    pub rx_wire_bytes: u64,
    pub rx_output_bytes: u64,
    pub rx_errors: u64,
}

#[derive(Debug, Clone, Copy, Default)]
pub struct AutoStats {
    pub off: u64,
    pub levels: [u64; 5],
}

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

// Strong-only ownership for connections and retained UDP frames. Allocation
// stays fallible without depending on Arc layout or unstable allocator APIs.
#[cfg(test)]
static SHARED_VALUE_DROPS: AtomicUsize = AtomicUsize::new(0);

struct SharedStorage<T> {
    references: AtomicUsize,
    value: T,
}

pub struct SharedValue<T> {
    storage: core::ptr::NonNull<SharedStorage<T>>,
}

// Immutable sharing requires a payload safe to share and drop on any thread.
// No mutable payload reference or weak-reference API is exposed.
unsafe impl<T: Send + Sync> Send for SharedValue<T> {}
unsafe impl<T: Send + Sync> Sync for SharedValue<T> {}

impl<T> SharedValue<T> {
    pub fn try_new(value: T) -> Result<Self, crate::error::StcpError> {
        let layout = core::alloc::Layout::new::<SharedStorage<T>>();
        // SharedStorage is nonzero-sized and its matching Box on final
        // release uses exactly this layout and global allocator.
        let raw = unsafe { alloc::alloc::alloc(layout) }.cast::<SharedStorage<T>>();
        let Some(storage) = core::ptr::NonNull::new(raw) else {
            return Err(crate::error::StcpError::NoMem);
        };
        unsafe { storage.as_ptr().write(SharedStorage {
            references: AtomicUsize::new(1), value,
        }); }
        Ok(Self { storage })
    }

    #[cfg(test)]
    pub fn strong_count(this: &Self) -> usize {
        unsafe { this.storage.as_ref() }.references.load(Ordering::Acquire)
    }
}

impl<T> Clone for SharedValue<T> {
    fn clone(&self) -> Self {
        let references = &unsafe { self.storage.as_ref() }.references;
        // Check before incrementing: overflow must never wrap the count and
        // permit premature free. A live &self already keeps the storage live.
        if references.fetch_update(Ordering::Relaxed, Ordering::Relaxed, |n| {
            if n < isize::MAX as usize { Some(n + 1) } else { None }
        }).is_err() {
            panic!("STCP shared reference count overflow");
        }
        Self { storage: self.storage }
    }
}

impl<T> core::ops::Deref for SharedValue<T> {
    type Target = T;
    fn deref(&self) -> &T {
        // Each live handle owns one reference, so value remains initialized
        // and alive for the entire borrow. No mutable reference is exposed.
        &unsafe { self.storage.as_ref() }.value
    }
}

impl<T> Drop for SharedValue<T> {
    fn drop(&mut self) {
        let references = &unsafe { self.storage.as_ref() }.references;
        if references.fetch_sub(1, Ordering::Release) != 1 { return; }
        // Pair with other owners' Release decrements before destroying the
        // synchronized value. Exactly one thread observes the final reference.
        core::sync::atomic::fence(Ordering::Acquire);
        #[cfg(test)]
        SHARED_VALUE_DROPS.fetch_add(1, Ordering::Relaxed);
        unsafe { drop(Box::from_raw(self.storage.as_ptr())); }
    }
}

pub type SharedConnection = SharedValue<Connection>;
// Retain the already encrypted Vec without copying its payload into an Arc.
pub type SharedFrame = SharedValue<Vec<u8>>;

pub struct EndpointConnection {
    pub shared: SharedConnection,
    pub side: Side,
}

pub struct PendingFrame {
    pub sequence: u64,
    pub bytes: SharedFrame,
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
    /// Authenticated plaintext with the original NONCE_LEN-byte prefix.
    pub payload: Vec<u8>,
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
    pub tx_control_nonce: u64,
    pub control_replay: crate::control::ReplayWindow,
    pub tx_nonce: u64,
    pub expected_rx_nonce: u64,
    pub tx_sequence: u64,
    pub expected_rx_sequence: u64,
    pub highest_acked_sequence: Option<u64>,
    pub last_ack_sent: Option<u64>,
    pub rx_frames_since_ack: u8,
    pub srtt_ms: Option<u32>,
    pub rttvar_ms: u32,
    pub rto_ms: u32,
    pub stats: ReliabilityStats,
    pub pending_frames: VecDeque<PendingFrame>,
    pub out_of_order_frames: Vec<BufferedFrame>,
    pub last_rx_sequence: Option<u64>,
    pub rx_app_data: ByteQueue,
    pub rx_message_ready: bool,
    pub rx_paused: bool,
    pub rx_partial_started_ms: Option<u64>,
    pub rx_terminal_errno: i32,
    pub peer_eof: bool,
    pub carrier: usize,
    pub connection_id: u64,
    pub udp_peer_addr: u32,
    pub udp_peer_port: u16,
    /* Reused by the TCP fast path; C tx_lock serializes socket sends. */
    pub tx_frame_scratch: Vec<u8>,
    pub compression: CompressionConfig,
    pub compression_stats: CompressionStats,
    pub auto_stats: AutoStats,
    pub auto_samples: u32,
    pub auto_slot_bytes: u32,
    pub peer_compression_capable: bool,
}

pub struct StcpContext {
    pub proto: u8,
    pub parser_busy: AtomicBool,
    pub parser_pending: AtomicBool,
    /* UDP demux readers holding a raw context pointer. Release removes the
     * context from UDP_SESSIONS first, then waits for this count to drain. */
    pub udp_demux_refs: AtomicUsize,
    pub inner: SpinLock<ContextInner>,
    // Drop buffers before returning the admission reservation.
    _rx_lease: crate::rx_policy::SessionLease,
}

impl StcpContext {
    pub fn new(proto: u8) -> Result<Self, crate::error::StcpError> {
        let rx_lease = crate::rx_policy::SessionLease::acquire()?;
        let crypto = CryptoContext::new()?;

        Ok(Self {
            _rx_lease: rx_lease,
            proto,
            parser_busy: AtomicBool::new(false),
            parser_pending: AtomicBool::new(false),
            udp_demux_refs: AtomicUsize::new(0),
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
                tx_control_nonce: 0,
                control_replay: crate::control::ReplayWindow::new(),
                tx_nonce: 0,
                expected_rx_nonce: 0,
                tx_sequence: 0,
                expected_rx_sequence: 0,
                highest_acked_sequence: None,
                last_ack_sent: None,
                rx_frames_since_ack: 0,
                srtt_ms: None,
                rttvar_ms: 0,
                rto_ms: 300,
                stats: ReliabilityStats::new(),
                pending_frames: VecDeque::new(),
                out_of_order_frames: Vec::new(),
                last_rx_sequence: None,
                rx_app_data: ByteQueue::new(),
                rx_message_ready: false,
                rx_paused: false,
                rx_partial_started_ms: None,
                rx_terminal_errno: 0,
                peer_eof: false,
                carrier: 0,
                connection_id: 0,
                udp_peer_addr: 0,
                udp_peer_port: 0,
                tx_frame_scratch: Vec::new(),
                compression: CompressionConfig::default(),
                compression_stats: CompressionStats::default(),
                auto_stats: AutoStats::default(),
                auto_samples: 4,
                auto_slot_bytes: 32,
                peer_compression_capable: false,
            }),
        })
    }

    pub fn connected_child(
        proto: u8,
        local: Address,
        peer: Address,
        shared: SharedConnection,
    ) -> Result<Self, crate::error::StcpError> {
        let rx_lease = crate::rx_policy::SessionLease::acquire()?;
        let crypto = CryptoContext::new()?;

        Ok(Self {
            _rx_lease: rx_lease,
            proto,
            parser_busy: AtomicBool::new(false),
            parser_pending: AtomicBool::new(false),
            udp_demux_refs: AtomicUsize::new(0),
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
                tx_control_nonce: 0,
                control_replay: crate::control::ReplayWindow::new(),
                tx_nonce: 0,
                expected_rx_nonce: 0,
                tx_sequence: 0,
                expected_rx_sequence: 0,
                highest_acked_sequence: None,
                last_ack_sent: None,
                rx_frames_since_ack: 0,
                srtt_ms: None,
                rttvar_ms: 0,
                rto_ms: 300,
                stats: ReliabilityStats::new(),
                pending_frames: VecDeque::new(),
                out_of_order_frames: Vec::new(),
                last_rx_sequence: None,
                rx_app_data: ByteQueue::new(),
                rx_message_ready: false,
                rx_paused: false,
                rx_partial_started_ms: None,
                rx_terminal_errno: 0,
                peer_eof: false,
                carrier: 0,
                connection_id: 0,
                udp_peer_addr: 0,
                udp_peer_port: 0,
                tx_frame_scratch: Vec::new(),
                compression: CompressionConfig::default(),
                compression_stats: CompressionStats::default(),
                auto_stats: AutoStats::default(),
                auto_samples: 4,
                auto_slot_bytes: 32,
                peer_compression_capable: false,
            }),
        })
    }
}

// Keep allocation failure inside the protocol's Result boundary. Ownership
// of value stays local until allocation succeeds, so error drops its leases.
pub(crate) fn try_box_context(value: StcpContext) -> Result<Box<StcpContext>, crate::error::StcpError> {
    let layout = core::alloc::Layout::new::<StcpContext>();
    // StcpContext is non-zero-sized. alloc() respects its layout/alignment;
    // Box takes over exactly this allocation after the initialized write.
    let raw = unsafe { alloc::alloc::alloc(layout) }.cast::<StcpContext>();
    if raw.is_null() {
        return Err(crate::error::StcpError::NoMem);
    }
    unsafe {
        raw.write(value);
        Ok(Box::from_raw(raw))
    }
}

#[cfg(test)]
mod shared_connection_tests {
    use super::*;
    #[test]
    fn shared_frame_keeps_original_buffer_until_last_reference() {
        let bytes=alloc::vec![17u8;4096];let pointer=bytes.as_ptr();
        let frame=SharedFrame::try_new(bytes).unwrap();
        assert_eq!(frame.as_ptr(),pointer);
        let retained=frame.clone();drop(frame);
        assert_eq!(retained.as_ptr(),pointer);assert_eq!(retained.len(),4096);
        assert!(retained.iter().all(|b|*b==17));
        assert_eq!(SharedFrame::strong_count(&retained),1);
    }
    #[test]
    fn generic_shared_storage_honors_payload_alignment_and_drop_once() {
        static DROPS: AtomicUsize=AtomicUsize::new(0);
        #[repr(align(128))]
        struct Aligned;
        impl Drop for Aligned {fn drop(&mut self){DROPS.fetch_add(1,Ordering::Relaxed);}}
        let before=DROPS.load(Ordering::Relaxed);
        let value=SharedValue::try_new(Aligned).unwrap();
        assert_eq!((&*value as *const Aligned as usize)%128,0);
        let copy=value.clone();drop(value);assert_eq!(DROPS.load(Ordering::Relaxed),before);
        std::thread::spawn(move ||drop(copy)).join().unwrap();
        assert_eq!(DROPS.load(Ordering::Relaxed),before+1);
    }

    #[test]
    fn shared_value_lives_until_final_reference_and_drops_once() {
        let before=SHARED_VALUE_DROPS.load(Ordering::Acquire);
        let a=SharedConnection::try_new(Connection::new()).unwrap();
        a.set_owner(Side::A,42);a.a_to_b.lock().push_slice(b"alive").unwrap();
        let b=a.clone();assert_eq!(SharedConnection::strong_count(&a),2);drop(a);
        assert_eq!(SHARED_VALUE_DROPS.load(Ordering::Acquire),before);
        assert_eq!(b.peer_owner(Side::B),42);let mut buf=[0u8;5];
        assert_eq!(b.a_to_b.lock().peek_prefix(&mut buf),5);assert_eq!(&buf,b"alive");
        drop(b);assert_eq!(SHARED_VALUE_DROPS.load(Ordering::Acquire),before+1);
    }
    #[test]
    fn concurrent_clone_drop_and_last_release_on_another_thread() {
        let before=SHARED_VALUE_DROPS.load(Ordering::Acquire);
        let shared=SharedConnection::try_new(Connection::new()).unwrap();
        std::thread::scope(|scope| {
            for _ in 0..8 {
                let handle=shared.clone();
                scope.spawn(move || {for _ in 0..2000 {
                    let copy=handle.clone();copy.owner_a.fetch_add(1,Ordering::Relaxed);drop(copy);
                }});
            }
        });
        assert_eq!(shared.owner_a.load(Ordering::Acquire),16000);
        assert_eq!(SharedConnection::strong_count(&shared),1);
        assert_eq!(SHARED_VALUE_DROPS.load(Ordering::Acquire),before);
        std::thread::spawn(move ||drop(shared)).join().unwrap();
        assert_eq!(SHARED_VALUE_DROPS.load(Ordering::Acquire),before+1);
    }
    #[test]
    fn reference_overflow_is_rejected_without_increment_or_wrap() {
        let shared=SharedConnection::try_new(Connection::new()).unwrap();
        let refs=&unsafe {shared.storage.as_ref()}.references;
        refs.store(isize::MAX as usize,Ordering::Relaxed);
        let result=std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| shared.clone()));
        let count=refs.load(Ordering::Acquire);refs.store(1,Ordering::Relaxed);
        assert!(result.is_err());assert_eq!(count,isize::MAX as usize);
    }
}
