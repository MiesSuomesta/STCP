pub struct ContextInner {
    pub state: SocketState,
    pub local: Option<Address>,
    pub peer: Option<Address>,
    pub backlog: usize,
    pub accept_queue: VecDeque<Box<StcpContext>>,
    pub connection: Option<EndpointConnection>,

    pub owner: usize,
    pub carrier: usize,

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
