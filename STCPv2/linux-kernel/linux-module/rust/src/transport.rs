use alloc::{
    boxed::Box,
    collections::VecDeque,
    sync::Arc,
    vec::Vec,
};

use core::{
    ffi::c_void,
    ptr,
};

use crate::{
    error::StcpError,
    packet::{
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

unsafe extern "C" {
    fn stcp_kernel_wake_accept(owner: *mut c_void);
    fn stcp_kernel_wake_recv(owner: *mut c_void);
}

#[derive(Clone, Copy)]
struct ListenerEntry {
    address: Address,
    ctx: usize,
}

static LISTENERS: SpinLock<Vec<ListenerEntry>> =
    SpinLock::new(Vec::new());

fn wake_accept(owner: usize) {
    if owner != 0 {
        unsafe {
            stcp_kernel_wake_accept(owner as *mut c_void);
        }
    }
}

fn wake_recv(owner: usize) {
    if owner != 0 {
        unsafe {
            stcp_kernel_wake_recv(owner as *mut c_void);
        }
    }
}

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

    if listeners
        .iter()
        .any(|entry| entry.address == address)
    {
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

    let child = Box::new(StcpContext::connected_child(
        ctx.proto,
        target,
        client_local,
        shared.clone(),
    ));

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

    {
        let mut inner = ctx.inner.lock();

        inner.peer = Some(target);
        inner.state = SocketState::Connected;
        inner.connection = Some(EndpointConnection {
            shared: shared.clone(),
            side: Side::A,
        });

        shared.set_owner(Side::A, inner.owner);
    }

    wake_accept(listener_owner);
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
    let (shared, side) = connection_for(ctx)?;

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

        let frame = encode_frame(packet_type, &data[position..end])?;
        encoded.extend_from_slice(&frame);
        position = end;
    }

    let queue = outgoing_queue(&shared, side);
    queue.lock().extend(encoded);

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

        if inner.rx_message_ready || inner.peer_eof {
            return Ok(());
        }
    }

    let (shared, side) = connection_for(ctx)?;
    let queue = incoming_queue(&shared, side);
    let mut wire = queue.lock();
    let mut assembled = VecDeque::new();
    let mut message_ready = false;
    let mut peer_eof = false;

    loop {
        if wire.len() < STCP_HEADER_LEN {
            break;
        }

        let mut header_bytes = [0u8; STCP_HEADER_LEN];

        for (index, byte) in wire.iter().take(STCP_HEADER_LEN).enumerate() {
            header_bytes[index] = *byte;
        }

        let header = Header::decode(&header_bytes)?;
        let frame_len = STCP_HEADER_LEN + header.payload_len;

        if wire.len() < frame_len {
            break;
        }

        for _ in 0..STCP_HEADER_LEN {
            let _ = wire.pop_front();
        }

        match header.packet_type {
            PacketType::DataChunk | PacketType::DataChunkEnd => {
                for _ in 0..header.payload_len {
                    assembled.push_back(
                        wire.pop_front().ok_or(StcpError::Protocol)?,
                    );
                }

                if header.packet_type == PacketType::DataChunkEnd {
                    message_ready = true;
                    break;
                }
            }
            PacketType::Close => {
                for _ in 0..header.payload_len {
                    let _ = wire.pop_front();
                }

                peer_eof = true;
                break;
            }
        }
    }

    drop(wire);

    if message_ready || peer_eof {
        let mut inner = ctx.inner.lock();

        inner.rx_app_data.extend(assembled);
        inner.rx_message_ready = message_ready;
        inner.peer_eof = peer_eof;
    }

    Ok(())
}

fn connection_for(
    ctx: &StcpContext,
) -> Result<(Arc<Connection>, Side), StcpError> {
    let inner = ctx.inner.lock();

    if inner.state != SocketState::Connected &&
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

fn outgoing_queue(
    shared: &Arc<Connection>,
    side: Side,
) -> &SpinLock<VecDeque<u8>> {
    match side {
        Side::A => &shared.a_to_b,
        Side::B => &shared.b_to_a,
    }
}

fn incoming_queue(
    shared: &Arc<Connection>,
    side: Side,
) -> &SpinLock<VecDeque<u8>> {
    match side {
        Side::A => &shared.b_to_a,
        Side::B => &shared.a_to_b,
    }
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

    let Ok((shared, side)) = connection_for(ctx) else {
        return false;
    };

    !incoming_queue(&shared, side).lock().is_empty() ||
        shared.peer_closed(side)
}

pub fn is_connected(ctx: &StcpContext) -> bool {
    ctx.inner.lock().state == SocketState::Connected
}

pub fn shutdown(
    ctx: &StcpContext,
    _how: i32,
) {
    let connection = {
        let mut inner = ctx.inner.lock();
        inner.state = SocketState::Closed;

        inner.connection
            .as_ref()
            .map(|endpoint| {
                (endpoint.shared.clone(), endpoint.side)
            })
    };

    if let Some((shared, side)) = connection {
        if let Ok(close_frame) = encode_frame(PacketType::Close, &[]) {
            outgoing_queue(&shared, side)
                .lock()
                .extend(close_frame);
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
