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
    byte_queue::ByteQueue,
    error::StcpError,
    frame::{
        encode_control_frame,
        encode_frame,
        Header,
        PacketType,
        STCP_FRAME_PAYLOAD_LEN,
        STCP_STREAM_FRAME_PAYLOAD_LEN,
        STCP_UDP_FRAME_PAYLOAD_LEN,
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

unsafe extern "C" {
    fn stcp_carrier_destroy(carrier: *mut core::ffi::c_void);
}



struct ParserGuard<'a> { ctx: &'a StcpContext }
impl Drop for ParserGuard<'_> { fn drop(&mut self) { self.ctx.parser_busy.store(false, Ordering::Release); } }
fn try_parser_guard(ctx: &StcpContext) -> Option<ParserGuard<'_>> {
    ctx.parser_busy.compare_exchange(false, true, Ordering::Acquire, Ordering::Relaxed).ok().map(|_| ParserGuard { ctx })
}
struct WireFrame { header: Header, payload: Vec<u8> }
fn extract_next_wire_frame(ctx: &StcpContext, queue: &SpinLock<ByteQueue>) -> Result<Option<WireFrame>, StcpError> {
    let mut retries = 0usize;
    loop {
        retries = retries.saturating_add(1);
        if retries > 1024 {
            crate::carrier::debug_event(208, ctx, retries, queue.lock().len());
            return Err(StcpError::Protocol);
        }
        let header = {
            let mut wire = queue.lock();
            if wire.len() < STCP_HEADER_LEN { return Ok(None); }
            let header = match peek_header(&wire) {
                Ok(header) => header,
                Err(StcpError::Again) => return Ok(None),
                Err(StcpError::Protocol) => { wire.discard(1); continue; }
                Err(error) => return Err(error),
            };
            let frame_len = STCP_HEADER_LEN.checked_add(header.payload_len).ok_or(StcpError::Protocol)?;
            if wire.len() < frame_len { return Ok(None); }
            header
        };
        crate::carrier::debug_event(203, ctx, header.packet_type as usize, header.payload_len);
        let payload = {
            let mut wire = queue.lock();
            let current = peek_header(&wire)?;
            if current.packet_type != header.packet_type || current.payload_len != header.payload_len || current.sequence != header.sequence || current.acknowledgment != header.acknowledgment || current.connection_id != header.connection_id {
                continue;
            }
            remove_header(&mut wire)?;
            if header.payload_len == 0 {
                Vec::new()
            } else {
                wire.take_or_read_vec(header.payload_len)?
            }
        };
        return Ok(Some(WireFrame { header, payload }));
    }
}

/* UDP must permit several large application messages in flight.  The old
 * 64-frame cap was exhausted by two 1 MiB writes (about 36 datagrams), which
 * caused stop-and-wait behaviour as soon as payloads crossed one frame. */
const STCP_SEND_WINDOW: usize = 256;
const STCP_ACK_EVERY_FRAMES: u8 = 8;
const STCP_MAX_RETRANSMIT_PER_TICK: usize = 4;
const STCP_TICK_MS: u32 = 20;
const STCP_INITIAL_RTO_MS: u32 = 60;
const STCP_MIN_RTO_MS: u32 = 20;
const STCP_MAX_RTO_MS: u32 = 3_000;
const STCP_MAX_RETRIES: u8 = 8;

fn ms_to_ticks(ms: u32) -> u32 {
    ms.saturating_add(STCP_TICK_MS - 1) / STCP_TICK_MS
}

fn clamp_rto(ms: u32) -> u32 {
    ms.clamp(STCP_MIN_RTO_MS, STCP_MAX_RTO_MS)
}

fn update_rtt_estimator(inner: &mut crate::state::ContextInner, sample_ms: u32) {
    let sample_ms = sample_ms.max(1);

    match inner.srtt_ms {
        None => {
            inner.srtt_ms = Some(sample_ms);
            inner.rttvar_ms = (sample_ms / 2).max(1);
        }
        Some(srtt) => {
            let error = srtt.abs_diff(sample_ms);
            inner.rttvar_ms = ((3 * inner.rttvar_ms) + error) / 4;
            inner.srtt_ms = Some(((7 * srtt) + sample_ms) / 8);
        }
    }

    let srtt = inner.srtt_ms.unwrap_or(sample_ms);
    inner.rto_ms = clamp_rto(
        srtt.saturating_add(4u32.saturating_mul(inner.rttvar_ms)),
    );
    inner.stats.rtt_samples = inner.stats.rtt_samples.saturating_add(1);
}

fn reset_reliability(inner: &mut crate::state::ContextInner) {
    inner.srtt_ms = None;
    inner.rttvar_ms = 0;
    inner.rto_ms = STCP_INITIAL_RTO_MS;
    inner.last_ack_sent = None;
    inner.rx_frames_since_ack = 0;
    inner.stats = crate::state::ReliabilityStats::new();
}

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

    /* Frames are encoded with the final connection id. Avoid a full-frame copy. */
    let _ = connection_id;
    crate::carrier::transmit(
        shared,
        side,
        carrier_ptr,
        frame,
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
        reset_reliability(&mut inner);
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
        reset_reliability(&mut inner);
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

fn process_handshake_frames(ctx: &StcpContext) -> Result<(), StcpError> {
    let Some(_guard)=try_parser_guard(ctx) else { crate::carrier::debug_event(209,ctx,1,0); return Ok(()); };
    let (shared,side)=connection_for_handshake(ctx)?; let queue=incoming_queue(&shared,side);
    let mut received_key:Option<[u8;PUBLIC_KEY_WIRE_LEN]>=None; let mut received_done=false;
    loop { let Some(frame)=extract_next_wire_frame(ctx,queue)? else { break; }; match frame.header.packet_type {
        PacketType::PublicKey => { if frame.payload.len()!=PUBLIC_KEY_WIRE_LEN{return Err(StcpError::Protocol);} let mut key=[0u8;PUBLIC_KEY_WIRE_LEN]; key.copy_from_slice(&frame.payload); received_key=Some(key); }
        PacketType::HandshakeDone => { if !frame.payload.is_empty(){return Err(StcpError::Protocol);} received_done=true; }
        _ => { crate::carrier::debug_event(206,ctx,frame.header.packet_type as usize,frame.payload.len()); return Err(StcpError::Protocol); }
    }}
    if let Some(key)=received_key { {let mut inner=ctx.inner.lock(); let role=inner.role; inner.crypto.derive_session_keys(&key,role)?;} let done=encode_frame(PacketType::HandshakeDone,connection_id(ctx),&[])?; send_frame(ctx,&shared,side,&done,0)?; }
    {
        let mut inner = ctx.inner.lock();

        if received_done {
            inner.peer_handshake_done = true;
            crate::carrier::debug_event(251, ctx, 1, 0);
        }

        /*
         * The local crypto context becoming ready only means that we have
         * received and processed the peer public key.  It does NOT mean that
         * the peer has finished installing its keys and accepted our
         * HandshakeDone frame yet.
         *
         * Marking the socket Ready at that earlier point created an
         * intermittent first-DATA race: connect() returned success and the
         * client sent application bytes while the accepted child was still in
         * Handshake.  The first frame could then be consumed as a handshake
         * frame or left unprocessed until both userspace peers timed out.
         */
        if inner.crypto.ready() && inner.peer_handshake_done {
            if inner.state != SocketState::Ready {
                inner.state = SocketState::Ready;
                crate::carrier::debug_event(252, ctx, 1, 0);
            }
        } else {
            crate::carrier::debug_event(
                250,
                ctx,
                inner.crypto.ready() as usize,
                inner.peer_handshake_done as usize,
            );
        }
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

#[inline]
fn frame_payload_len(ctx: &StcpContext) -> usize {
    if ctx.proto == 254 {
        STCP_UDP_FRAME_PAYLOAD_LEN
    } else {
        STCP_STREAM_FRAME_PAYLOAD_LEN
    }
}

pub fn can_send(ctx: &StcpContext, data_len: usize) -> bool {
    if progress_handshake(ctx).is_err() {
        return false;
    }

    /* TCP already provides reliable ordered delivery and has no STCP ACK
     * window to drain. Avoid running the RX parser on every TX readiness
     * query; recv() owns parsing on the stream fast path. */
    let carrier_ptr = ctx.inner.lock().carrier;
    let reliable = crate::carrier::reliability_required(carrier_ptr);
    if reliable && process_control_frames(ctx).is_err() {
        return false;
    }

    let frame_count = if data_len == 0 {
        0
    } else {
        data_len.div_ceil(frame_payload_len(ctx))
    };

    let inner = ctx.inner.lock();
    inner.state == SocketState::Ready &&
        inner.pending_frames.len() + frame_count <= STCP_SEND_WINDOW
}

pub fn send(
    ctx: &StcpContext,
    data: &[u8],
) -> Result<usize, StcpError> {
    progress_handshake(ctx)?;

    if !is_ready(ctx) {
        return Err(StcpError::Again);
    }

    let carrier_ptr = ctx.inner.lock().carrier;
    let reliable = crate::carrier::reliability_required(carrier_ptr);

    if reliable {
        process_control_frames(ctx)?;
    }

    let (shared, side) = ready_connection(ctx)?;

    if shared.peer_closed(side) {
        return Err(StcpError::Closed);
    }

    let frame_count = if data.is_empty() {
        0
    } else {
        data.len().div_ceil(frame_payload_len(ctx))
    };

    if reliable {
        let inner = ctx.inner.lock();
        if inner.pending_frames.len() + frame_count > STCP_SEND_WINDOW {
            return Err(StcpError::Again);
        }
    }

    let payload_limit = frame_payload_len(ctx);
    let mut position = 0usize;

    while position < data.len() {
        let end = (position + payload_limit).min(data.len());
        let packet_type = if end == data.len() {
            PacketType::DataChunkEnd
        } else {
            PacketType::DataChunk
        };
        let plaintext = &data[position..end];

        /*
         * Snapshot immutable crypto/state under the short socket lock. The C
         * tx_lock serializes sends for this socket, so allocation and ChaCha
         * can run without holding the Rust spinlock for several milliseconds.
         */
        let (sequence, nonce, acknowledgment, connection_id, crypto) = {
            let inner = ctx.inner.lock();
            if inner.state != SocketState::Ready {
                return Err(StcpError::InvalidState);
            }
            (
                inner.tx_sequence,
                inner.tx_nonce,
                inner.last_rx_sequence.unwrap_or(0),
                inner.connection_id,
                inner.crypto.clone(),
            )
        };

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
            connection_id,
        )?.encode();
        let frame_len = STCP_HEADER_LEN
            .checked_add(NONCE_LEN)
            .and_then(|value| value.checked_add(encrypted_len))
            .ok_or(StcpError::Protocol)?;
        /*
         * TCP fast path reuses one frame allocation per socket. UDP keeps an
         * Arc because reliability may retain the encrypted bytes for retry.
         */
        let mut frame = if reliable {
            Vec::new()
        } else {
            core::mem::take(&mut ctx.inner.lock().tx_frame_scratch)
        };
        frame.clear();
        if frame.capacity() < frame_len {
            frame.try_reserve_exact(frame_len - frame.capacity())
                .map_err(|_| StcpError::NoMem)?;
        }
        frame.resize(frame_len, 0);
        frame[..STCP_HEADER_LEN].copy_from_slice(&header);
        frame[STCP_HEADER_LEN..STCP_HEADER_LEN + NONCE_LEN]
            .copy_from_slice(&nonce.to_be_bytes());
        let encrypted_written = crypto.encrypt_into(
            nonce,
            &header,
            plaintext,
            &mut frame[STCP_HEADER_LEN + NONCE_LEN..],
        )?;
        frame.truncate(STCP_HEADER_LEN + NONCE_LEN + encrypted_written);

        if reliable {
            let frame: Arc<[u8]> = frame.into();
            {
                let mut inner = ctx.inner.lock();
                if inner.state != SocketState::Ready ||
                   inner.tx_sequence != sequence ||
                   inner.tx_nonce != nonce
                {
                    return Err(StcpError::Again);
                }
                inner.tx_nonce = inner.tx_nonce.checked_add(1).ok_or(StcpError::Crypto)?;
                inner.tx_sequence = inner.tx_sequence.checked_add(1).ok_or(StcpError::Protocol)?;
                let rto_ticks = ms_to_ticks(inner.rto_ms.max(STCP_INITIAL_RTO_MS));
                inner.pending_frames.push_back(PendingFrame {
                    sequence,
                    bytes: Arc::clone(&frame),
                    age_ticks: 0,
                    rto_ticks,
                    retries: 0,
                    retransmitted: false,
                });
                inner.stats.sent_frames = inner.stats.sent_frames.saturating_add(1);
            }
            send_frame(ctx, &shared, side, &frame, 0)?;
        } else {
            {
                let mut inner = ctx.inner.lock();
                if inner.state != SocketState::Ready ||
                   inner.tx_sequence != sequence ||
                   inner.tx_nonce != nonce
                {
                    inner.tx_frame_scratch = frame;
                    return Err(StcpError::Again);
                }
                inner.tx_nonce = inner.tx_nonce.checked_add(1).ok_or(StcpError::Crypto)?;
                inner.tx_sequence = inner.tx_sequence.checked_add(1).ok_or(StcpError::Protocol)?;
            }
            let send_result = send_frame(ctx, &shared, side, &frame, 0);
            ctx.inner.lock().tx_frame_scratch = frame;
            send_result?;
        }
        position = end;
    }

    Ok(data.len())
}

pub fn recv(
    ctx: &StcpContext,
    output: &mut [u8],
) -> Result<usize, StcpError> {
    crate::carrier::debug_event(101, ctx, output.len(), 0);
    progress_handshake(ctx)?;
    crate::carrier::debug_event(102, ctx, 0, 0);

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

    crate::carrier::debug_event(110, ctx, 0, 0);
    fill_application_buffer(ctx)?;
    crate::carrier::debug_event(111, ctx, 0, 0);

    let mut inner = ctx.inner.lock();
    crate::carrier::debug_event(112, ctx, inner.rx_app_data.len(), inner.rx_message_ready as usize);

    if inner.rx_message_ready && !inner.rx_app_data.is_empty() {
        let count = inner.rx_app_data.read_into(output);

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
    { let inner=ctx.inner.lock(); if inner.state!=SocketState::Ready && inner.state!=SocketState::Closed{return Err(StcpError::InvalidState);} if inner.peer_eof{return Ok(());} }
    let Some(_guard)=try_parser_guard(ctx) else { crate::carrier::debug_event(209,ctx,2,0); return Ok(()); };
    let (shared,side)=connection_for_data(ctx)?; let queue=incoming_queue(&shared,side);
    let mut received_frames=Vec::new(); let mut deferred_acks=Vec::new(); let mut deferred_pongs=Vec::new(); let mut peer_eof=false; let mut late_handshake_done=false;
    crate::carrier::debug_event(120,ctx,0,0);
    const MAX_RX_BATCH_FRAMES: usize = 128;
    received_frames.try_reserve(MAX_RX_BATCH_FRAMES.min(8)).map_err(|_| StcpError::NoMem)?;
    let mut extracted = 0usize;
    loop {
      if extracted >= MAX_RX_BATCH_FRAMES { break; }
      let Some(frame)=extract_next_wire_frame(ctx,queue)? else {break;};
      extracted += 1; let header=frame.header; crate::carrier::debug_event(210,ctx,header.packet_type as usize,frame.payload.len()); match header.packet_type {
      PacketType::DataChunk|PacketType::DataChunkEnd => { if frame.payload.len()<NONCE_LEN+CHACHA_TAG_LEN{return protocol_error(ctx);} let mut nb=[0u8;NONCE_LEN]; nb.copy_from_slice(&frame.payload[..NONCE_LEN]); let nonce=u64::from_be_bytes(nb); received_frames.try_reserve(1).map_err(|_|StcpError::NoMem)?; received_frames.push(BufferedFrame{header,nonce,ciphertext:frame.payload}); }
      PacketType::Ack => { if !frame.payload.is_empty(){return protocol_error(ctx);} deferred_acks.try_reserve(1).map_err(|_|StcpError::NoMem)?; deferred_acks.push(header.acknowledgment); }
      PacketType::Ping => { if !frame.payload.is_empty(){return protocol_error(ctx);} deferred_pongs.try_reserve(1).map_err(|_|StcpError::NoMem)?; deferred_pongs.push(header.sequence); }
      PacketType::Pong => { if !frame.payload.is_empty(){return protocol_error(ctx);} }
      PacketType::Reset => return protocol_error(ctx), PacketType::Close => {peer_eof=true;break;}
      PacketType::PublicKey => {if frame.payload.len()!=PUBLIC_KEY_WIRE_LEN{return protocol_error(ctx);}}
      PacketType::HandshakeDone => {if !frame.payload.is_empty(){return protocol_error(ctx);} late_handshake_done=true;}
    }}
    crate::carrier::debug_event(123,ctx,deferred_acks.len(),deferred_pongs.len()); for a in deferred_acks{update_acknowledgment(ctx,a)?;} for seq in deferred_pongs{queue_pong(ctx,seq)?;} crate::carrier::debug_event(124,ctx,received_frames.len(),0);
    let mut became_readable = false;
    for frame in received_frames {
        let expected = current_expected_sequence(ctx);
        if frame.header.sequence < expected {
            {
                let mut inner = ctx.inner.lock();
                inner.stats.duplicate_frames = inner.stats.duplicate_frames.saturating_add(1);
            }
            queue_ack(ctx, frame.header.sequence, true)?;
            continue;
        }
        if frame.header.sequence > expected {
            {
                let mut inner = ctx.inner.lock();
                inner.stats.reordered_frames = inner.stats.reordered_frames.saturating_add(1);
            }
            buffer_out_of_order_frame(ctx, frame)?;
            continue;
        }
        became_readable |= process_in_order_frame(ctx, frame)?;
        while let Some(buffered) = take_next_buffered_frame(ctx) {
            became_readable |= process_in_order_frame(ctx, buffered)?;
        }
    }
    let mut should_wake = became_readable;
    if peer_eof || late_handshake_done {
        let mut inner = ctx.inner.lock();
        if peer_eof && !inner.peer_eof {
            inner.peer_eof = true;
            should_wake = true;
        }
        if late_handshake_done {
            inner.peer_handshake_done = true;
        }
    }
    if should_wake {
        let owner = ctx.inner.lock().owner;
        wake_recv(owner);
    }
    crate::carrier::debug_event(299,ctx,0,0); Ok(())
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
    mut frame: BufferedFrame,
) -> Result<bool, StcpError> {
    let packet_type = frame.header.packet_type;
    let sequence = frame.header.sequence;

    /*
     * Parser serialization guarantees a single RX committer. Snapshot the
     * crypto context and validate sequence/nonce under the lock, then perform
     * allocation and ChaCha decryption outside it.
     */
    let crypto = {
        let inner = ctx.inner.lock();
        if frame.header.sequence != inner.expected_rx_sequence ||
           frame.nonce != inner.expected_rx_nonce
        {
            return Err(StcpError::Protocol);
        }
        inner.crypto.clone()
    };

    let aad = frame.header.encode();
    /* Decrypt directly over the ciphertext portion of the owned wire frame.
     * The nonce prefix is retained as a skipped ByteQueue offset, so RX does
     * not allocate a plaintext Vec and does not copy a multi-megabyte payload. */
    let plaintext_len = crypto.decrypt_in_place(
        frame.nonce,
        &aad,
        &mut frame.ciphertext[NONCE_LEN..],
    )?;
    frame.ciphertext.truncate(NONCE_LEN + plaintext_len);

    let became_readable = {
        let mut inner = ctx.inner.lock();
        if frame.header.sequence != inner.expected_rx_sequence ||
           frame.nonce != inner.expected_rx_nonce
        {
            inner.state = SocketState::Error;
            return Err(StcpError::Protocol);
        }

        let was_readable = inner.rx_message_ready;
        inner.expected_rx_nonce = inner.expected_rx_nonce
            .checked_add(1)
            .ok_or(StcpError::Crypto)?;
        inner.expected_rx_sequence = inner.expected_rx_sequence
            .checked_add(1)
            .ok_or(StcpError::Protocol)?;
        inner.last_rx_sequence = Some(sequence);
        inner.rx_app_data.push_vec_from(frame.ciphertext, NONCE_LEN)?;

        if packet_type == PacketType::DataChunkEnd {
            inner.rx_message_ready = true;
        }
        !was_readable && inner.rx_message_ready
    };

    queue_ack(
        ctx,
        sequence,
        packet_type == PacketType::DataChunkEnd,
    )?;

    Ok(became_readable)
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
        if let Some(frame) = inner.pending_frames.pop_front() {
            /*
             * Karn's algorithm: never derive an RTT sample from a frame
             * that has been retransmitted because the ACK is ambiguous.
             */
            if !frame.retransmitted {
                let sample_ms = frame.age_ticks
                    .max(1)
                    .saturating_mul(STCP_TICK_MS);
                update_rtt_estimator(&mut inner, sample_ms);
            }

            inner.stats.acknowledged_frames =
                inner.stats.acknowledged_frames.saturating_add(1);
        }
    }

    Ok(())
}

fn queue_ack(
    ctx: &StcpContext,
    sequence: u64,
    force: bool,
) -> Result<(), StcpError> {
    let carrier_ptr = ctx.inner.lock().carrier;
    if !crate::carrier::reliability_required(carrier_ptr) {
        return Ok(());
    }

    let should_send = {
        let mut inner = ctx.inner.lock();

        if inner
            .last_ack_sent
            .map(|acknowledged| acknowledged >= sequence)
            .unwrap_or(false)
        {
            return Ok(());
        }

        inner.rx_frames_since_ack =
            inner.rx_frames_since_ack.saturating_add(1);

        if force || inner.rx_frames_since_ack >= STCP_ACK_EVERY_FRAMES {
            inner.rx_frames_since_ack = 0;
            inner.last_ack_sent = Some(sequence);
            true
        } else {
            false
        }
    };

    if !should_send {
        return Ok(());
    }

    /* ACK is cumulative: one control frame releases every pending frame up
     * to `sequence`.  This cuts ACK traffic by roughly 8x for fragmented
     * UDP application messages while DataChunkEnd still flushes promptly. */
    let (shared, side) = connection_for_data(ctx)?;
    let frame = encode_control_frame(
        PacketType::Ack,
        0,
        sequence,
        connection_id(ctx),
        &[],
    )?;
    send_frame(ctx, &shared, side, &frame, 0)?;
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
    Ok(())
}

fn process_control_frames(ctx: &StcpContext) -> Result<(), StcpError> { crate::carrier::debug_event(140,ctx,0,0); let result=fill_application_buffer(ctx); crate::carrier::debug_event(143,ctx,result.is_ok() as usize,0); result }

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
        /* TCP carrier RX already parses and publishes complete frames from
         * queue_to_context(). Avoid a redundant timer-driven parser pass. */
        return Ok(true);
    }

    process_control_frames(ctx)?;

    let (shared, side) = connection_for_data(ctx)?;
    let mut retransmit = Vec::new();

    {
        let mut inner = ctx.inner.lock();
        let mut retransmitted_count = 0u64;
        let mut timed_out = false;

        for pending in &mut inner.pending_frames {
            pending.age_ticks = pending.age_ticks.saturating_add(1);

            if pending.age_ticks < pending.rto_ticks {
                continue;
            }

            if pending.retries >= STCP_MAX_RETRIES {
                timed_out = true;
                break;
            }

            /* Never retransmit the entire window in one timer callback. A
             * delayed cumulative ACK previously made hundreds of frames fire
             * together, congesting loopback UDP and delaying the ACK further. */
            if retransmit.len() >= STCP_MAX_RETRANSMIT_PER_TICK {
                continue;
            }

            pending.age_ticks = 0;
            pending.retries = pending.retries.saturating_add(1);
            pending.retransmitted = true;

            /*
             * Exponential backoff is per frame. Clamp at the global
             * maximum RTO so a dead peer eventually fails predictably.
             */
            pending.rto_ticks = pending.rto_ticks
                .saturating_mul(2)
                .min(ms_to_ticks(STCP_MAX_RTO_MS))
                .max(1);

            retransmitted_count = retransmitted_count.saturating_add(1);
            retransmit.push(pending.bytes.clone());
        }

        inner.stats.retransmitted_frames = inner
            .stats
            .retransmitted_frames
            .saturating_add(retransmitted_count);

        if timed_out {
            inner.stats.timeout_failures =
                inner.stats.timeout_failures.saturating_add(1);
            inner.state = SocketState::Error;
            return Err(StcpError::Closed);
        }
    }

    for frame in retransmit {
        send_frame(ctx, &shared, side, &frame, 0)?;
    }

    Ok(true)
}

pub fn reliability_snapshot(
    ctx: &StcpContext,
) -> (u32, u32, u32, crate::state::ReliabilityStats) {
    let inner = ctx.inner.lock();
    (
        inner.srtt_ms.unwrap_or(0),
        inner.rttvar_ms,
        inner.rto_ms,
        inner.stats,
    )
}

fn protocol_error<T>(ctx: &StcpContext) -> Result<T, StcpError> {
    ctx.inner.lock().state = SocketState::Error;
    Err(StcpError::Protocol)
}

fn peek_header(
    wire: &ByteQueue,
) -> Result<Header, StcpError> {
    let mut header_bytes = [0u8; STCP_HEADER_LEN];

    if wire.peek_prefix(&mut header_bytes) != STCP_HEADER_LEN {
        return Err(StcpError::Again);
    }

    Header::decode(&header_bytes)
}

fn remove_header(wire: &mut ByteQueue) -> Result<(), StcpError> {
    /*
     * This must never live inside debug_assert!: debug assertions are
     * compiled out in release builds, which previously left zero-length
     * control frames at the queue head forever.
     */
    if wire.discard(STCP_HEADER_LEN) != STCP_HEADER_LEN {
        return Err(StcpError::Protocol);
    }
    Ok(())
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
    /*
     * Keep poll()/wait_event() side-effect free. Carrier RX publishes complete
     * application data before waking recv_wq, so readiness only needs an
     * acquire-style state check here. Parsing inside a wait condition caused
     * lost-wakeup races and repeated parser work under churn.
     */
    let inner = ctx.inner.lock();
    inner.rx_message_ready || inner.peer_eof
}

/*
 * Called by the carrier immediately after appending bytes. It advances the
 * handshake and parses all currently complete frames before the C side wakes
 * recv_wq. This guarantees that a wake corresponds to data/EOF visible to
 * recv(), eliminating the producer-before-publication race.
 */
pub(crate) fn progress_receive(ctx: &StcpContext) -> Result<bool, StcpError> {
    progress_handshake(ctx)?;

    let state = ctx.inner.lock().state;
    if state == SocketState::Ready || state == SocketState::Closed {
        fill_application_buffer(ctx)?;
    }

    let inner = ctx.inner.lock();
    Ok(inner.rx_message_ready || inner.peer_eof)
}

pub fn is_connected(ctx: &StcpContext) -> bool {
    let _ = progress_handshake(ctx);
    is_ready(ctx)
}

pub(crate) fn is_ready_snapshot(ctx: &StcpContext) -> bool {
    ctx.inner.lock().state == SocketState::Ready
}

fn is_ready(ctx: &StcpContext) -> bool {
    is_ready_snapshot(ctx)
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

fn unregister_listener(ctx: &StcpContext) {
    let ctx_ptr = ptr::from_ref(ctx) as usize;
    let mut listeners = LISTENERS.lock();

    /*
     * Remove by context identity rather than by socket state/address.
     * Release may run after a partial setup, an error transition, or after
     * the state was already changed to Closed.
     */
    listeners.retain(|entry| entry.ctx != ctx_ptr);
}

pub fn release(ctx: &StcpContext) {
    crate::carrier::unregister_context(ctx);
    unregister_listener(ctx);

    /*
     * Final release is local teardown only. It must not call shutdown(),
     * send a Close frame, or touch a carrier that may already be detached.
     */
    let (connection, queued_children) = {
        let mut inner = ctx.inner.lock();

        let connection = inner.connection.take().map(|endpoint| {
            (endpoint.shared, endpoint.side)
        });

        inner.state = SocketState::Closed;
        inner.owner = 0;
        inner.carrier = 0;

        let queued_children = inner.accept_queue.drain(..).collect::<Vec<_>>();

        inner.pending_frames.clear();
        inner.out_of_order_frames.clear();
        inner.rx_app_data.clear();
        inner.rx_message_ready = false;
        inner.peer_eof = true;

        (connection, queued_children)
    };

    for child in queued_children {
        let carrier = {
            let mut child_inner = child.inner.lock();
            let carrier = child_inner.carrier;
            child_inner.carrier = 0;
            child_inner.owner = 0;
            child_inner.state = SocketState::Closed;
            carrier
        };

        if carrier != 0 {
            unsafe {
                stcp_carrier_destroy(
                    carrier as *mut core::ffi::c_void,
                )
            };
        }

        drop(child);
    }

    if let Some((shared, side)) = connection {
        shared.set_owner(side, 0);
        shared.close(side);
        wake_recv(shared.peer_owner(side));
    }
}
