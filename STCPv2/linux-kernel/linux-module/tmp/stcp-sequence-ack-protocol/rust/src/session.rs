use alloc::{
    boxed::Box,
    collections::VecDeque,
    sync::Arc,
    vec::Vec,
};

use core::{
    ptr,
};

use crate::{
    crypto::{
        CHACHA_TAG_LEN,
        NONCE_LEN,
        PUBLIC_KEY_WIRE_LEN,
    },
    carrier::{
        incoming_queue,
        outgoing_queue,
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

pub fn set_owner(ctx: &StcpContext, owner: usize) {
    let mut inner = ctx.inner.lock();
    inner.owner = owner;

    if let Some(endpoint) = &inner.connection {
        endpoint.shared.set_owner(endpoint.side, owner);
    }
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

    let shared = Arc::new(Connection::new());

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
        inner.last_rx_sequence = None;
        shared.set_owner(Side::A, inner.owner);
    }

    perform_symmetric_handshake(ctx, child.as_ref())?;

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

fn perform_symmetric_handshake(
    client: &StcpContext,
    server: &StcpContext,
) -> Result<(), StcpError> {
    send_public_key(client)?;
    send_public_key(server)?;

    process_handshake_frames(client)?;
    process_handshake_frames(server)?;

    process_handshake_frames(client)?;
    process_handshake_frames(server)?;

    if !is_ready(client) || !is_ready(server) {
        return Err(StcpError::Protocol);
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
        &public_key,
    )?;

    outgoing_queue(&shared, side).lock().extend(frame);
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

        let done = encode_frame(PacketType::HandshakeDone, &[])?;
        outgoing_queue(&shared, side).lock().extend(done);
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
    process_control_frames(ctx)?;

    let (shared, side) = ready_connection(ctx)?;

    if shared.peer_closed(side) {
        return Err(StcpError::Closed);
    }

    let mut encoded = Vec::new();
    let mut position = 0usize;

    while position < data.len() {
        let end = (position + STCP_FRAME_PAYLOAD_LEN).min(data.len());
        let packet_type = if end == data.len() {
            PacketType::DataChunkEnd
        } else {
            PacketType::DataChunk
        };
        let plaintext = &data[position..end];

        let (sequence, acknowledgment, nonce, ciphertext) = {
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
            )?.encode();
            let ciphertext = inner.crypto.encrypt(
                nonce,
                &header,
                plaintext,
            )?;

            inner.tx_nonce = inner.tx_nonce
                .checked_add(1)
                .ok_or(StcpError::Crypto)?;
            inner.tx_sequence = inner.tx_sequence
                .checked_add(1)
                .ok_or(StcpError::Protocol)?;

            (sequence, acknowledgment, nonce, ciphertext)
        };

        let frame = encode_encrypted_frame(
            packet_type,
            sequence,
            acknowledgment,
            nonce,
            &ciphertext,
        )?;

        encoded.extend_from_slice(&frame);
        position = end;
    }

    outgoing_queue(&shared, side).lock().extend(encoded);
    wake_recv(shared.peer_owner(side));

    Ok(data.len())
}

pub fn recv(
    ctx: &StcpContext,
    output: &mut [u8],
) -> Result<usize, StcpError> {
    if output.is_empty() {
        return Ok(0);
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
    let mut wire = queue.lock();
    let mut encrypted_frames: Vec<(Header, u64, Vec<u8>)> = Vec::new();
    let mut peer_eof = false;

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
                if header.sequence != current_expected_sequence(ctx) {
                    return protocol_error(ctx);
                }

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
                        wire.pop_front().ok_or(StcpError::Protocol)?,
                    );
                }

                let is_end = header.packet_type == PacketType::DataChunkEnd;
                encrypted_frames.push((header, nonce, ciphertext));

                if is_end {
                    break;
                }
            }
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
                queue_pong(ctx, header.sequence)?;
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
            PacketType::PublicKey | PacketType::HandshakeDone => {
                return protocol_error(ctx);
            }
        }
    }

    drop(wire);

    let mut assembled = VecDeque::new();
    let mut message_ready = false;

    for (header, nonce, ciphertext) in encrypted_frames {
        let plaintext = {
            let mut inner = ctx.inner.lock();

            if nonce != inner.expected_rx_nonce ||
               header.sequence != inner.expected_rx_sequence
            {
                inner.state = SocketState::Error;
                return Err(StcpError::Protocol);
            }

            let aad = header.encode();

            match inner.crypto.decrypt(
                nonce,
                &aad,
                &ciphertext,
            ) {
                Ok(plaintext) => {
                    inner.expected_rx_nonce = inner.expected_rx_nonce
                        .checked_add(1)
                        .ok_or(StcpError::Crypto)?;
                    inner.expected_rx_sequence = inner.expected_rx_sequence
                        .checked_add(1)
                        .ok_or(StcpError::Protocol)?;
                    inner.last_rx_sequence = Some(header.sequence);
                    plaintext
                },
                Err(error) => {
                    inner.state = SocketState::Error;
                    return Err(error);
                }
            }
        };

        assembled.extend(plaintext);
        queue_ack(ctx, header.sequence)?;

        if header.packet_type == PacketType::DataChunkEnd {
            message_ready = true;
        }
    }

    if message_ready || peer_eof {
        let mut inner = ctx.inner.lock();
        inner.rx_app_data.extend(assembled);
        inner.rx_message_ready = message_ready;
        inner.peer_eof = peer_eof;
    }

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
        &[],
    )?;
    outgoing_queue(&shared, side).lock().extend(frame);
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
        &[],
    )?;
    outgoing_queue(&shared, side).lock().extend(frame);
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
            &[],
        ) {
            outgoing_queue(&shared, side).lock().extend(close_frame);
        }

        shared.close(side);
        wake_recv(shared.peer_owner(side));
    }
}

pub fn release(ctx: &StcpContext) {
    let local = {
        let inner = ctx.inner.lock();

        if inner.state == SocketState::Listening {
            inner.local
        } else {
            None
        }
    };

    if let Some(address) = local {
        let ctx_ptr = ptr::from_ref(ctx) as usize;
        let mut listeners = LISTENERS.lock();

        listeners.retain(|entry| {
            entry.ctx != ctx_ptr || entry.address != address
        });
    }

    shutdown(ctx, 2);
}
