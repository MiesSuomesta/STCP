use alloc::{
    boxed::Box,
    collections::VecDeque,
    sync::Arc,
    vec::Vec,
};

use core::{
    ptr,
    sync::atomic::{AtomicU64, Ordering},
};

use crate::{
    crypto::{
        CHACHA_TAG_LEN,
        NONCE_LEN,
        PUBLIC_KEY_WIRE_LEN,
    },
    carrier::{
        incoming_queue,
        wake_accept,
        wake_recv,
    },
    error::StcpError,
    frame::{
        encode_control_frame,
        encode_encrypted_frame,
        encode_frame,
        Header,
        PacketType,
        STCP_FRAME_PAYLOAD_LEN,
        STCP_HEADER_LEN,
    },
    spinlock::SpinLock,
    state::{
        Address,
        Connection,
        EndpointConnection,
        Side,
        SocketState,
        BufferedFrame,
        PendingFrame,
        StcpContext,
    },
};

#[derive(Clone, Copy)]
struct ListenerEntry {
    address: Address,
    ctx: usize,
}

static LISTENERS: SpinLock<Vec<ListenerEntry>> =
    SpinLock::new(Vec::new());

static NEXT_CONNECTION_ID: AtomicU64 = AtomicU64::new(1);

const STCP_SEND_WINDOW: usize = 8;
const STCP_RETRANSMIT_AFTER_TICKS: u32 = 3;
const STCP_MAX_RETRIES: u8 = 5;

fn connection_id(ctx: &StcpContext) -> u64 {
    ctx.inner.lock().connection_id
}

pub fn set_owner(ctx: &StcpContext, owner: usize) {
    let mut inner = ctx.inner.lock();
    inner.owner = owner;

    if let Some(endpoint) = &inner.connection {
        endpoint.shared.set_owner(endpoint.side, owner);
    }
}

fn send_frame(
    ctx: &StcpContext,
    shared: &Arc<Connection>,
    side: Side,
    frame: &[u8],
    flags: i32,
) -> Result<(), StcpError> {
    let (carrier_ptr, connection_id) = {
        let inner = ctx.inner.lock();
        (inner.carrier, inner.connection_id)
    };

    if frame.len() < STCP_HEADER_LEN {
        return Err(StcpError::Protocol);
    }

    let mut wire_frame = frame.to_vec();
    wire_frame[32..40].copy_from_slice(&connection_id.to_be_bytes());

    crate::carrier::transmit(
        shared,
        side,
        carrier_ptr,
        &wire_frame,
        flags,
    )
}

pub fn bind(
    ctx: &StcpContext,
    addr: u32,
    port: u16,
) -> Result<(), StcpError> {
    let mut inner = ctx.inner.lock();

    if inner.state != SocketState::New {
        return Err(StcpError::InvalidState);
    }

    inner.local = Some(Address { addr, port });
    inner.state = SocketState::Bound;

    Ok(())
}

pub fn listen(
    ctx: &StcpContext,
    backlog: i32,
) -> Result<(), StcpError> {
    let address = {
        let mut inner = ctx.inner.lock();

        if inner.state != SocketState::Bound {
            return Err(StcpError::InvalidState);
        }

        let address = inner.local.ok_or(StcpError::InvalidState)?;
        inner.backlog = backlog.max(1) as usize;
        inner.state = SocketState::Listening;
        address
    };

    let ctx_ptr = ptr::from_ref(ctx) as usize;
    let mut listeners = LISTENERS.lock();

    if listeners.iter().any(|entry| entry.address == address) {
        let mut inner = ctx.inner.lock();
        inner.state = SocketState::Bound;
        return Err(StcpError::AddressInUse);
    }

    listeners.push(ListenerEntry {
        address,
        ctx: ctx_ptr,
    });

    Ok(())
}

pub fn connect(
    ctx: &StcpContext,
    addr: u32,
    port: u16,
) -> Result<(), StcpError> {
    {
        let inner = ctx.inner.lock();

        if inner.state != SocketState::New &&
           inner.state != SocketState::Bound
        {
            return Err(StcpError::InvalidState);
        }
    }

    let target = Address { addr, port };
    let shared = Arc::new(Connection::new());

    /*
     * UDP is connectionless at the carrier layer. The client must not create
     * or enqueue a server child through the in-kernel listener registry.
     *
     * The client sends the first PublicKey datagram with a fresh connection
     * ID. The UDP listener creates the server child only when that datagram is
     * received, because only then are the real peer address and port known.
     */
    if ctx.proto == 254 {
        let mut inner = ctx.inner.lock();

        inner.peer = Some(target);

        if inner.connection_id == 0 {
            inner.connection_id =
                NEXT_CONNECTION_ID.fetch_add(1, Ordering::Relaxed);

            if inner.connection_id == 0 {
                inner.connection_id =
                    NEXT_CONNECTION_ID.fetch_add(1, Ordering::Relaxed);
            }
        }

        inner.state = SocketState::Handshake;
        inner.connection = Some(EndpointConnection {
            shared: shared.clone(),
            side: Side::A,
        });

        inner.tx_nonce = 0;
        inner.expected_rx_nonce = 0;
        inner.tx_sequence = 0;
        inner.expected_rx_sequence = 0;
        inner.highest_acked_sequence = None;
        inner.pending_frames.clear();
        inner.out_of_order_frames.clear();
        inner.last_rx_sequence = None;

        shared.set_owner(Side::A, inner.owner);
        return Ok(());
    }

    /*
     * The existing in-kernel paired transport path is retained for the
     * non-UDP test/backend implementation.
     */
    let listener_ptr = {
        let listeners = LISTENERS.lock();

        listeners
            .iter()
            .find(|entry| entry.address == target)
            .map(|entry| entry.ctx)
            .ok_or(StcpError::ConnectionRefused)?
    };

    let listener = unsafe {
        &*(listener_ptr as *const StcpContext)
    };

    let client_local = {
        let inner = ctx.inner.lock();
        inner.local.unwrap_or(Address { addr: 0, port: 0 })
    };

    let child_ctx = StcpContext::connected_child(
        ctx.proto,
        target,
        client_local,
        shared.clone(),
    )?;

    let child = Box::new(child_ctx);

    {
        let mut inner = ctx.inner.lock();

        inner.peer = Some(target);
        inner.state = SocketState::Handshake;
        inner.connection = Some(EndpointConnection {
            shared: shared.clone(),
            side: Side::A,
        });

        inner.tx_nonce = 0;
        inner.expected_rx_nonce = 0;
        inner.tx_sequence = 0;
        inner.expected_rx_sequence = 0;
        inner.highest_acked_sequence = None;
        inner.pending_frames.clear();
        inner.out_of_order_frames.clear();
        inner.last_rx_sequence = None;
        shared.set_owner(Side::A, inner.owner);
    }

    let listener_owner = {
        let mut inner = listener.inner.lock();

        if inner.state != SocketState::Listening {
            return Err(StcpError::ConnectionRefused);
        }

        if inner.accept_queue.len() >= inner.backlog {
            return Err(StcpError::Again);
        }

        inner.accept_queue.push_back(child);
        inner.owner
    };

    wake_accept(listener_owner);
    Ok(())
}

pub fn start_handshake(ctx: &StcpContext) -> Result<(), StcpError> {
    {
        let inner = ctx.inner.lock();

        if inner.state != SocketState::Handshake {
            return Err(StcpError::InvalidState);
        }

        if inner.carrier == 0 {
            return Err(StcpError::Kernel(-107));
        }
    }

    send_public_key(ctx)
}

pub fn progress_handshake(ctx: &StcpContext) -> Result<(), StcpError> {
    let state = ctx.inner.lock().state;

    if state == SocketState::Handshake {
        process_handshake_frames(ctx)?;
    }

    Ok(())
}

fn send_public_key(ctx: &StcpContext) -> Result<(), StcpError> {
    let (shared, side, public_key) = {
        let inner = ctx.inner.lock();

        if inner.state != SocketState::Handshake {
            return Err(StcpError::InvalidState);
        }

        let endpoint = inner
            .connection
            .as_ref()
            .ok_or(StcpError::InvalidState)?;

        (
            endpoint.shared.clone(),
            endpoint.side,
            inner.crypto.public_key(),
        )
    };

    let frame = encode_frame(
        PacketType::PublicKey,
        connection_id(ctx),
        &public_key,
    )?;

    send_frame(ctx, &shared, side, &frame, 0)?;
    Ok(())
}

fn process_handshake_frames(
    ctx: &StcpContext,
) -> Result<(), StcpError> {
    let (shared, side) = connection_for_handshake(ctx)?;
    let queue = incoming_queue(&shared, side);
    let mut wire = queue.lock();
    let mut received_key: Option<[u8; PUBLIC_KEY_WIRE_LEN]> = None;
    let mut received_done = false;

    loop {
        if wire.len() < STCP_HEADER_LEN {
            break;
        }

        let header = peek_header(&wire)?;
        let frame_len = STCP_HEADER_LEN
            .checked_add(header.payload_len)
            .ok_or(StcpError::Protocol)?;

        if wire.len() < frame_len {
            break;
        }

        match header.packet_type {
            PacketType::PublicKey => {
                if header.payload_len != PUBLIC_KEY_WIRE_LEN {
                    return Err(StcpError::Protocol);
                }

                remove_header(&mut wire);

                let mut key = [0u8; PUBLIC_KEY_WIRE_LEN];

                for byte in &mut key {
                    *byte = wire
                        .pop_front()
                        .ok_or(StcpError::Protocol)?;
                }

                received_key = Some(key);
            }
            PacketType::HandshakeDone => {
                if header.payload_len != 0 {
                    return Err(StcpError::Protocol);
                }

                remove_header(&mut wire);
                received_done = true;
            }
            _ => break,
        }
    }

    drop(wire);

    if let Some(key) = received_key {
        {
            let mut inner = ctx.inner.lock();
            let role = inner.role;
            inner.crypto.derive_session_keys(&key, role)?;
        }

        let done = encode_frame(PacketType::HandshakeDone, connection_id(ctx), &[])?;
        send_frame(ctx, &shared, side, &done, 0)?;
    }

    if received_done {
        let mut inner = ctx.inner.lock();
        inner.peer_handshake_done = true;
    }

    let mut inner = ctx.inner.lock();

    if inner.crypto.ready() && inner.peer_handshake_done {
        inner.state = SocketState::Ready;
    }

    Ok(())
}

pub fn accept(
    ctx: &StcpContext,
) -> Result<Box<StcpContext>, StcpError> {
    let mut inner = ctx.inner.lock();

    if inner.state != SocketState::Listening {
        return Err(StcpError::InvalidState);
    }

    inner
        .accept_queue
        .pop_front()
        .ok_or(StcpError::Again)
}

pub fn send(
    ctx: &StcpContext,
    data: &[u8],
) -> Result<usize, StcpError> {
    progress_handshake(ctx)?;

    if !is_ready(ctx) {
        return Err(StcpError::Again);
    }

    process_control_frames(ctx)?;

    let (shared, side) = ready_connection(ctx)?;

    if shared.peer_closed(side) {
        return Err(StcpError::Closed);
    }

    let frame_count = if data.is_empty() {
        0
    } else {
        data.len().div_ceil(STCP_FRAME_PAYLOAD_LEN)
    };

    {
        let inner = ctx.inner.lock();

        if inner.pending_frames.len() + frame_count > STCP_SEND_WINDOW {
            return Err(StcpError::Again);
        }
    }

    let mut position = 0usize;

    while position < data.len() {
        let end = (position + STCP_FRAME_PAYLOAD_LEN).min(data.len());
        let packet_type = if end == data.len() {
            PacketType::DataChunkEnd
        } else {
            PacketType::DataChunk
        };
        let plaintext = &data[position..end];

        let frame = {
            let mut inner = ctx.inner.lock();

            if inner.state != SocketState::Ready {
                return Err(StcpError::InvalidState);
            }

            let sequence = inner.tx_sequence;
            let nonce = inner.tx_nonce;
            let acknowledgment = inner.last_rx_sequence.unwrap_or(0);
            let encrypted_len = plaintext
                .len()
                .checked_add(CHACHA_TAG_LEN)
                .ok_or(StcpError::Protocol)?;
            let payload_len = NONCE_LEN
                .checked_add(encrypted_len)
                .ok_or(StcpError::Protocol)?;
            let header = Header::with_numbers(
                packet_type,
                payload_len,
                sequence,
                acknowledgment,
                inner.connection_id,
            )?.encode();
            let ciphertext = inner.crypto.encrypt(
                nonce,
                &header,
                plaintext,
            )?;

            let frame = encode_encrypted_frame(
                packet_type,
                sequence,
                acknowledgment,
                inner.connection_id,
                nonce,
                &ciphertext,
            )?;

            inner.tx_nonce = inner.tx_nonce
                .checked_add(1)
                .ok_or(StcpError::Crypto)?;
            inner.tx_sequence = inner.tx_sequence
                .checked_add(1)
                .ok_or(StcpError::Protocol)?;

            inner.pending_frames.push_back(PendingFrame {
                sequence,
                bytes: frame.clone(),
                age_ticks: 0,
                retries: 0,
            });

            frame
        };

        send_frame(ctx, &shared, side, &frame, 0)?;
        position = end;
    }

    Ok(data.len())
}

pub fn recv(
    ctx: &StcpContext,
    output: &mut [u8],
) -> Result<usize, StcpError> {
    progress_handshake(ctx)?;

    if output.is_empty() {
        return Ok(0);
    }

    /*
     * accept() may return before the asynchronous carrier handshake has
     * reached Ready.  For a blocking SOCK_STREAM recv this is not an
     * invalid socket state: report Again so the C wrapper can sleep on
     * recv_wq and retry after the carrier wakes it.
     */
    {
        let inner = ctx.inner.lock();

        if inner.state == SocketState::Handshake {
            return Err(StcpError::Again);
        }

        if inner.state != SocketState::Ready &&
           inner.state != SocketState::Closed
        {
            return Err(StcpError::InvalidState);
        }
    }

    fill_application_buffer(ctx)?;

    let mut inner = ctx.inner.lock();

    if inner.rx_message_ready && !inner.rx_app_data.is_empty() {
        let count = output.len().min(inner.rx_app_data.len());

        for slot in output.iter_mut().take(count) {
            *slot = inner.rx_app_data.pop_front().unwrap_or_default();
        }

        if inner.rx_app_data.is_empty() {
            inner.rx_message_ready = false;
        }

        return Ok(count);
    }

    if inner.peer_eof {
        return Ok(0);
    }

    Err(StcpError::Again)
}

fn fill_application_buffer(ctx: &StcpContext) -> Result<(), StcpError> {
    {
        let inner = ctx.inner.lock();

        if inner.state != SocketState::Ready &&
           inner.state != SocketState::Closed
        {
            return Err(StcpError::InvalidState);
        }

        if inner.rx_message_ready || inner.peer_eof {
            return Ok(());
        }
    }

    let (shared, side) = connection_for_data(ctx)?;
    let queue = incoming_queue(&shared, side);
    let mut received_frames = Vec::new();
    let mut peer_eof = false;

    /*
     * First remove all complete frames from the carrier queue.
     * Do not validate DATA ordering while holding the carrier lock.
     */
    {
        let mut wire = queue.lock();

        loop {
            if wire.len() < STCP_HEADER_LEN {
                break;
            }

            let header = peek_header(&wire)?;
            let frame_len = STCP_HEADER_LEN
                .checked_add(header.payload_len)
                .ok_or(StcpError::Protocol)?;

            if wire.len() < frame_len {
                break;
            }

            match header.packet_type {
                PacketType::DataChunk | PacketType::DataChunkEnd => {
                    if header.payload_len < NONCE_LEN + CHACHA_TAG_LEN {
                        return protocol_error(ctx);
                    }

                    remove_header(&mut wire);

                    let mut nonce_bytes = [0u8; NONCE_LEN];

                    for byte in &mut nonce_bytes {
                        *byte = wire
                            .pop_front()
                            .ok_or(StcpError::Protocol)?;
                    }

                    let nonce = u64::from_be_bytes(nonce_bytes);
                    let ciphertext_len = header.payload_len - NONCE_LEN;
                    let mut ciphertext = Vec::new();

                    ciphertext
                        .try_reserve_exact(ciphertext_len)
                        .map_err(|_| StcpError::NoMem)?;

                    for _ in 0..ciphertext_len {
                        ciphertext.push(
                            wire
                                .pop_front()
                                .ok_or(StcpError::Protocol)?,
                        );
                    }

                    received_frames.push(BufferedFrame {
                        header,
                        nonce,
                        ciphertext,
                    });
                }
                PacketType::Ack => {
                    if header.payload_len != 0 {
                        return protocol_error(ctx);
                    }

                    remove_header(&mut wire);
                    update_acknowledgment(
                        ctx,
                        header.acknowledgment,
                    )?;
                }
                PacketType::Ping => {
                    if header.payload_len != 0 {
                        return protocol_error(ctx);
                    }

                    remove_header(&mut wire);
                    drop(wire);
                    queue_pong(ctx, header.sequence)?;
                    return fill_application_buffer(ctx);
                }
                PacketType::Pong => {
                    if header.payload_len != 0 {
                        return protocol_error(ctx);
                    }

                    remove_header(&mut wire);
                }
                PacketType::Reset => {
                    remove_header(&mut wire);

                    for _ in 0..header.payload_len {
                        let _ = wire.pop_front();
                    }

                    drop(wire);
                    return protocol_error(ctx);
                }
                PacketType::Close => {
                    remove_header(&mut wire);

                    for _ in 0..header.payload_len {
                        let _ = wire.pop_front();
                    }

                    peer_eof = true;
                    break;
                }
                PacketType::PublicKey |
                PacketType::HandshakeDone => {
                    return protocol_error(ctx);
                }
            }
        }
    }

    /*
     * Classify DATA frames:
     *
     * - old sequence: duplicate, ACK again;
     * - future sequence: retain in the out-of-order buffer;
     * - expected sequence: decrypt and deliver.
     */
    for frame in received_frames {
        let expected = current_expected_sequence(ctx);

        if frame.header.sequence < expected {
            queue_ack(ctx, frame.header.sequence)?;
            continue;
        }

        if frame.header.sequence > expected {
            buffer_out_of_order_frame(ctx, frame)?;
            continue;
        }

        process_in_order_frame(ctx, frame)?;

        /*
         * The newly accepted frame may close a gap. Drain every now
         * contiguous frame from the out-of-order buffer.
         */
        while let Some(buffered) = take_next_buffered_frame(ctx) {
            process_in_order_frame(ctx, buffered)?;
        }
    }

    if peer_eof {
        let mut inner = ctx.inner.lock();
        inner.peer_eof = true;
    }

    Ok(())
}

fn buffer_out_of_order_frame(
    ctx: &StcpContext,
    frame: BufferedFrame,
) -> Result<(), StcpError> {
    let mut inner = ctx.inner.lock();

    if inner
        .out_of_order_frames
        .iter()
        .any(|buffered| {
            buffered.header.sequence == frame.header.sequence
        })
    {
        return Ok(());
    }

    /*
     * Never retain more frames than the receive side can reasonably
     * accept from the current send window.
     */
    if inner.out_of_order_frames.len() >= STCP_SEND_WINDOW {
        inner.state = SocketState::Error;
        return Err(StcpError::Protocol);
    }

    inner
        .out_of_order_frames
        .try_reserve(1)
        .map_err(|_| StcpError::NoMem)?;

    inner.out_of_order_frames.push(frame);
    Ok(())
}

fn take_next_buffered_frame(
    ctx: &StcpContext,
) -> Option<BufferedFrame> {
    let mut inner = ctx.inner.lock();
    let expected = inner.expected_rx_sequence;

    let position = inner
        .out_of_order_frames
        .iter()
        .position(|frame| {
            frame.header.sequence == expected
        })?;

    Some(inner.out_of_order_frames.swap_remove(position))
}

fn process_in_order_frame(
    ctx: &StcpContext,
    frame: BufferedFrame,
) -> Result<(), StcpError> {
    let packet_type = frame.header.packet_type;
    let sequence = frame.header.sequence;

    let plaintext = {
        let mut inner = ctx.inner.lock();

        if frame.header.sequence != inner.expected_rx_sequence {
            inner.state = SocketState::Error;
            return Err(StcpError::Protocol);
        }

        /*
         * TX nonce and DATA sequence advance together. A buffered frame
         * is decrypted only once it becomes the next expected frame.
         */
        if frame.nonce != inner.expected_rx_nonce {
            inner.state = SocketState::Error;
            return Err(StcpError::Protocol);
        }

        let aad = frame.header.encode();

        match inner.crypto.decrypt(
            frame.nonce,
            &aad,
            &frame.ciphertext,
        ) {
            Ok(plaintext) => {
                inner.expected_rx_nonce = inner
                    .expected_rx_nonce
                    .checked_add(1)
                    .ok_or(StcpError::Crypto)?;

                inner.expected_rx_sequence = inner
                    .expected_rx_sequence
                    .checked_add(1)
                    .ok_or(StcpError::Protocol)?;

                inner.last_rx_sequence = Some(sequence);
                plaintext
            }
            Err(error) => {
                inner.state = SocketState::Error;
                return Err(error);
            }
        }
    };

    {
        let mut inner = ctx.inner.lock();
        inner.rx_app_data.extend(plaintext);

        if packet_type == PacketType::DataChunkEnd {
            inner.rx_message_ready = true;
        }
    }

    queue_ack(ctx, sequence)?;
    Ok(())
}

fn current_expected_sequence(ctx: &StcpContext) -> u64 {
    ctx.inner.lock().expected_rx_sequence
}

fn update_acknowledgment(
    ctx: &StcpContext,
    acknowledgment: u64,
) -> Result<(), StcpError> {
    let mut inner = ctx.inner.lock();

    if acknowledgment >= inner.tx_sequence && inner.tx_sequence != 0 {
        inner.state = SocketState::Error;
        return Err(StcpError::Protocol);
    }

    if inner.highest_acked_sequence
        .map(|previous| acknowledgment > previous)
        .unwrap_or(true)
    {
        inner.highest_acked_sequence = Some(acknowledgment);
    }

    while inner.pending_frames
        .front()
        .map(|frame| frame.sequence <= acknowledgment)
        .unwrap_or(false)
    {
        inner.pending_frames.pop_front();
    }

    Ok(())
}

fn queue_ack(
    ctx: &StcpContext,
    sequence: u64,
) -> Result<(), StcpError> {
    let (shared, side) = connection_for_data(ctx)?;
    let frame = encode_control_frame(
        PacketType::Ack,
        0,
        sequence,
        connection_id(ctx),
        &[],
    )?;
    send_frame(ctx, &shared, side, &frame, 0)?;
    wake_recv(shared.peer_owner(side));
    Ok(())
}

fn queue_pong(
    ctx: &StcpContext,
    ping_sequence: u64,
) -> Result<(), StcpError> {
    let (shared, side) = connection_for_data(ctx)?;
    let frame = encode_control_frame(
        PacketType::Pong,
        ping_sequence,
        0,
        connection_id(ctx),
        &[],
    )?;
    send_frame(ctx, &shared, side, &frame, 0)?;
    wake_recv(shared.peer_owner(side));
    Ok(())
}

fn process_control_frames(ctx: &StcpContext) -> Result<(), StcpError> {
    let (shared, side) = connection_for_data(ctx)?;
    let queue = incoming_queue(&shared, side);
    let mut wire = queue.lock();

    loop {
        if wire.len() < STCP_HEADER_LEN {
            break;
        }

        let header = peek_header(&wire)?;
        let frame_len = STCP_HEADER_LEN
            .checked_add(header.payload_len)
            .ok_or(StcpError::Protocol)?;

        if wire.len() < frame_len {
            break;
        }

        match header.packet_type {
            PacketType::Ack => {
                if header.payload_len != 0 {
                    return protocol_error(ctx);
                }
                remove_header(&mut wire);
                update_acknowledgment(ctx, header.acknowledgment)?;
            }
            PacketType::Ping => {
                if header.payload_len != 0 {
                    return protocol_error(ctx);
                }
                remove_header(&mut wire);
                drop(wire);
                queue_pong(ctx, header.sequence)?;
                return Ok(());
            }
            PacketType::Pong => {
                if header.payload_len != 0 {
                    return protocol_error(ctx);
                }
                remove_header(&mut wire);
            }
            _ => break,
        }
    }

    Ok(())
}


pub fn tick(ctx: &StcpContext) -> Result<bool, StcpError> {
    {
        let inner = ctx.inner.lock();

        if inner.state == SocketState::Closed ||
           inner.state == SocketState::Error
        {
            return Ok(false);
        }

        if inner.state != SocketState::Ready {
            return Ok(true);
        }
    }

    let carrier_ptr = {
        let inner = ctx.inner.lock();
        inner.carrier
    };

    if !crate::carrier::reliability_required(carrier_ptr) {
        process_control_frames(ctx)?;
        return Ok(true);
    }

    process_control_frames(ctx)?;

    let (shared, side) = connection_for_data(ctx)?;
    let mut retransmit = Vec::new();

    {
        let mut inner = ctx.inner.lock();

        for pending in &mut inner.pending_frames {
            pending.age_ticks = pending.age_ticks.saturating_add(1);

            if pending.age_ticks < STCP_RETRANSMIT_AFTER_TICKS {
                continue;
            }

            if pending.retries >= STCP_MAX_RETRIES {
                inner.state = SocketState::Error;
                return Err(StcpError::Closed);
            }

            pending.age_ticks = 0;
            pending.retries = pending.retries.saturating_add(1);
            retransmit.push(pending.bytes.clone());
        }
    }

    for frame in retransmit {
        send_frame(ctx, &shared, side, &frame, 0)?;
    }

    Ok(true)
}

fn protocol_error<T>(ctx: &StcpContext) -> Result<T, StcpError> {
    ctx.inner.lock().state = SocketState::Error;
    Err(StcpError::Protocol)
}

fn peek_header(
    wire: &VecDeque<u8>,
) -> Result<Header, StcpError> {
    let mut header_bytes = [0u8; STCP_HEADER_LEN];

    for (index, byte) in wire.iter().take(STCP_HEADER_LEN).enumerate() {
        header_bytes[index] = *byte;
    }

    Header::decode(&header_bytes)
}

fn remove_header(wire: &mut VecDeque<u8>) {
    for _ in 0..STCP_HEADER_LEN {
        let _ = wire.pop_front();
    }
}

fn connection_for_handshake(
    ctx: &StcpContext,
) -> Result<(Arc<Connection>, Side), StcpError> {
    let inner = ctx.inner.lock();

    if inner.state != SocketState::Handshake &&
       inner.state != SocketState::Ready
    {
        return Err(StcpError::InvalidState);
    }

    let endpoint = inner
        .connection
        .as_ref()
        .ok_or(StcpError::InvalidState)?;

    Ok((endpoint.shared.clone(), endpoint.side))
}

fn ready_connection(
    ctx: &StcpContext,
) -> Result<(Arc<Connection>, Side), StcpError> {
    let inner = ctx.inner.lock();

    if inner.state != SocketState::Ready {
        return Err(StcpError::InvalidState);
    }

    let endpoint = inner
        .connection
        .as_ref()
        .ok_or(StcpError::InvalidState)?;

    Ok((endpoint.shared.clone(), endpoint.side))
}

fn connection_for_data(
    ctx: &StcpContext,
) -> Result<(Arc<Connection>, Side), StcpError> {
    let inner = ctx.inner.lock();

    if inner.state != SocketState::Ready &&
       inner.state != SocketState::Closed
    {
        return Err(StcpError::InvalidState);
    }

    let endpoint = inner
        .connection
        .as_ref()
        .ok_or(StcpError::InvalidState)?;

    Ok((endpoint.shared.clone(), endpoint.side))
}

pub fn has_accept(ctx: &StcpContext) -> bool {
    let inner = ctx.inner.lock();
    !inner.accept_queue.is_empty()
}

pub fn has_data(ctx: &StcpContext) -> bool {
    let _ = progress_handshake(ctx);

    {
        let inner = ctx.inner.lock();

        if inner.rx_message_ready || inner.peer_eof {
            return true;
        }
    }

    let Ok((shared, side)) = connection_for_data(ctx) else {
        return false;
    };

    !incoming_queue(&shared, side).lock().is_empty() ||
        shared.peer_closed(side)
}

pub fn is_connected(ctx: &StcpContext) -> bool {
    let _ = progress_handshake(ctx);
    is_ready(ctx)
}

fn is_ready(ctx: &StcpContext) -> bool {
    ctx.inner.lock().state == SocketState::Ready
}

pub fn shutdown(
    ctx: &StcpContext,
    _how: i32,
) {
    let connection = {
        let mut inner = ctx.inner.lock();

        if inner.state == SocketState::Closed {
            return;
        }

        inner.state = SocketState::Closed;

        inner.connection
            .as_ref()
            .map(|endpoint| {
                (endpoint.shared.clone(), endpoint.side)
            })
    };

    if let Some((shared, side)) = connection {
        let acknowledgment = ctx.inner.lock().last_rx_sequence.unwrap_or(0);
        if let Ok(close_frame) = encode_control_frame(
            PacketType::Close,
            0,
            acknowledgment,
            connection_id(ctx),
            &[],
        ) {
            let _ = send_frame(ctx, &shared, side, &close_frame, 0);
        }

        shared.close(side);
        wake_recv(shared.peer_owner(side));
    }
}

pub fn release(ctx: &StcpContext) {
    crate::carrier::unregister_context(ctx);
    /*
     * Final release is a local teardown only.  It must never call
     * shutdown() or send a Close frame: the C carrier may already be
     * detached/stopped and the peer may already have disappeared.
     */
    let (local, connection) = {
        let mut inner = ctx.inner.lock();

        let local = if inner.state == SocketState::Listening {
            inner.local
        } else {
            None
        };

        let connection = inner.connection.take().map(|endpoint| {
            (endpoint.shared, endpoint.side)
        });

        inner.state = SocketState::Closed;
        inner.owner = 0;
        inner.carrier = 0;
        inner.accept_queue.clear();
        inner.pending_frames.clear();
        inner.out_of_order_frames.clear();
        inner.rx_app_data.clear();
        inner.rx_message_ready = false;
        inner.peer_eof = true;

        (local, connection)
    };

    if let Some(address) = local {
        let ctx_ptr = ptr::from_ref(ctx) as usize;
        let mut listeners = LISTENERS.lock();

        listeners.retain(|entry| {
            entry.ctx != ctx_ptr || entry.address != address
        });
    }

    if let Some((shared, side)) = connection {
        shared.set_owner(side, 0);
        shared.close(side);
        wake_recv(shared.peer_owner(side));
    }
}
