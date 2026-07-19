use alloc::{
    boxed::Box,
    collections::VecDeque,
    sync::Arc,
    vec::Vec,
};

use core::ffi::{c_int, c_void};

use crate::{
    byte_queue::ByteQueue,
    error::StcpError,
    frame::{Header, PacketType, STCP_HEADER_LEN},
    spinlock::SpinLock,
    state::{Address, Connection, Side, SocketState, StcpContext},
};

unsafe extern "C" {
    fn stcp_carrier_needs_reliability(carrier: *const c_void) -> bool;
    fn stcp_carrier_create_udp_child(
        listener: *mut c_void,
        child_rust_ctx: *mut c_void,
        peer_addr: u32,
        peer_port: u16,
    ) -> *mut c_void;
    fn stcp_carrier_destroy(carrier: *mut c_void);
    fn stcp_carrier_send(
        carrier: *mut c_void,
        data: *const u8,
        len: usize,
        flags: c_int,
    ) -> isize;
    fn stcp_kernel_wake_accept(owner: *mut c_void);
    fn stcp_kernel_wake_recv(owner: *mut c_void);
    fn stcp_kernel_debug_event(event: u32, ctx: usize, arg0: usize, arg1: usize);
}

#[derive(Clone, Copy)]
struct UdpSessionEntry {
    listener: usize,
    connection_id: u64,
    child: usize,
    peer_addr: u32,
    peer_port: u16,
}

static UDP_SESSIONS: SpinLock<Vec<UdpSessionEntry>> = SpinLock::new(Vec::new());


pub(crate) fn debug_event(event: u32, ctx: &StcpContext, arg0: usize, arg1: usize) {
    unsafe { stcp_kernel_debug_event(event, ctx as *const StcpContext as usize, arg0, arg1) };
}

pub(crate) fn wake_accept(owner: usize) {
    if owner != 0 {
        unsafe { stcp_kernel_wake_accept(owner as *mut c_void) };
    }
}

pub(crate) fn wake_recv(owner: usize) {
    if owner != 0 {
        unsafe { stcp_kernel_wake_recv(owner as *mut c_void) };
    }
}

pub(crate) fn outgoing_queue(
    shared: &Arc<Connection>,
    side: Side,
) -> &SpinLock<ByteQueue> {
    match side {
        Side::A => &shared.a_to_b,
        Side::B => &shared.b_to_a,
    }
}

pub(crate) fn incoming_queue(
    shared: &Arc<Connection>,
    side: Side,
) -> &SpinLock<ByteQueue> {
    match side {
        Side::A => &shared.b_to_a,
        Side::B => &shared.a_to_b,
    }
}

pub(crate) fn reliability_required(carrier: usize) -> bool {
    if carrier == 0 {
        return true;
    }

    unsafe { stcp_carrier_needs_reliability(carrier as *const c_void) }
}

pub(crate) fn set_carrier(ctx: &StcpContext, carrier: usize) {
    let mut inner = ctx.inner.lock();
    inner.carrier = carrier;
}

pub(crate) fn transmit(
    shared: &Arc<Connection>,
    side: Side,
    carrier: usize,
    bytes: &[u8],
    flags: c_int,
) -> Result<(), StcpError> {
    if carrier == 0 {
        outgoing_queue(shared, side)
            .lock()
            .push_slice(bytes)?;
        wake_recv(shared.peer_owner(side));
        return Ok(());
    }

    let ret = unsafe {
        stcp_carrier_send(
            carrier as *mut c_void,
            bytes.as_ptr(),
            bytes.len(),
            flags,
        )
    };

    if ret < 0 {
        return Err(StcpError::Kernel(ret as i32));
    }
    if ret as usize != bytes.len() {
        return Err(StcpError::Kernel(-5));
    }
    Ok(())
}

fn queue_to_context(ctx: &StcpContext, bytes: &[u8]) -> c_int {
    let (shared, side, owner) = {
        let inner = ctx.inner.lock();
        let Some(endpoint) = &inner.connection else {
            return -107;
        };
        (endpoint.shared.clone(), endpoint.side, inner.owner)
    };

    if let Err(error) = incoming_queue(&shared, side)
        .lock()
        .push_slice(bytes)
    {
        return error.errno();
    }

    if let Err(error) = crate::session::progress_handshake(ctx) {
        /* Before accept() attaches the UDP child carrier, ENOTCONN is expected. */
        if error.errno() != -107 {
            return error.errno();
        }
    }

    wake_recv(owner);
    0
}

fn find_udp_child(
    listener: usize,
    connection_id: u64,
) -> Option<UdpSessionEntry> {
    UDP_SESSIONS
        .lock()
        .iter()
        .find(|entry| {
            entry.listener == listener && entry.connection_id == connection_id
        })
        .copied()
}

fn create_udp_child(
    listener: &StcpContext,
    listener_ptr: usize,
    connection_id: u64,
    peer_addr: u32,
    peer_port: u16,
) -> Result<usize, StcpError> {
    let (local, backlog, owner, listener_carrier) = {
        let inner = listener.inner.lock();
        if inner.state != SocketState::Listening {
            return Err(StcpError::InvalidState);
        }
        (
            inner.local.ok_or(StcpError::InvalidState)?,
            inner.backlog,
            inner.owner,
            inner.carrier,
        )
    };

    {
        let inner = listener.inner.lock();
        if inner.accept_queue.len() >= backlog {
            return Err(StcpError::Again);
        }
    }

    let shared = Arc::new(Connection::new());
    let peer = Address {
        addr: peer_addr,
        port: peer_port,
    };
    let child = Box::new(StcpContext::connected_child(
        listener.proto,
        local,
        peer,
        shared,
    )?);

    {
        let mut inner = child.inner.lock();
        inner.connection_id = connection_id;
        inner.udp_peer_addr = peer_addr;
        inner.udp_peer_port = peer_port;
    }

    let child_ptr = Box::into_raw(child) as usize;

    if listener_carrier == 0 {
        unsafe { drop(Box::from_raw(child_ptr as *mut StcpContext)) };
        return Err(StcpError::Kernel(-107));
    }

    let child_carrier = unsafe {
        stcp_carrier_create_udp_child(
            listener_carrier as *mut c_void,
            child_ptr as *mut c_void,
            peer_addr,
            peer_port,
        )
    };

    if child_carrier.is_null() {
        unsafe { drop(Box::from_raw(child_ptr as *mut StcpContext)) };
        return Err(StcpError::NoMem);
    }

    {
        let child_ref = unsafe { &*(child_ptr as *const StcpContext) };
        child_ref.inner.lock().carrier = child_carrier as usize;
    }

    UDP_SESSIONS.lock().push(UdpSessionEntry {
        listener: listener_ptr,
        connection_id,
        child: child_ptr,
        peer_addr,
        peer_port,
    });

    {
        let mut inner = listener.inner.lock();
        inner.accept_queue.push_back(unsafe { Box::from_raw(child_ptr as *mut StcpContext) });
    }

    wake_accept(owner);
    Ok(child_ptr)
}

pub(crate) fn unregister_context(ctx: &StcpContext) {
    let ptr = ctx as *const StcpContext as usize;
    UDP_SESSIONS.lock().retain(|entry| {
        entry.child != ptr && entry.listener != ptr
    });
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_carrier_receive(
    raw_ctx: *mut c_void,
    data: *const u8,
    len: usize,
) -> c_int {
    stcp_rust_carrier_receive_from(raw_ctx, data, len, 0, 0)
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_carrier_receive_from(
    raw_ctx: *mut c_void,
    data: *const u8,
    len: usize,
    peer_addr: u32,
    peer_port: u16,
) -> c_int {
    if raw_ctx.is_null() || (data.is_null() && len != 0) {
        return -22;
    }

    let ctx = unsafe { &*(raw_ctx as *const StcpContext) };
    let bytes = if len == 0 {
        &[]
    } else {
        unsafe { core::slice::from_raw_parts(data, len) }
    };

    let state = ctx.inner.lock().state;
    if state != SocketState::Listening || ctx.proto != 254 {
        return queue_to_context(ctx, bytes);
    }

    if bytes.len() < STCP_HEADER_LEN {
        return -71;
    }

    let header = match Header::decode(bytes) {
        Ok(header) => header,
        Err(error) => return error.errno(),
    };

    if header.connection_id == 0 {
        return -71;
    }

    let listener_ptr = ctx as *const StcpContext as usize;
    let entry = match find_udp_child(listener_ptr, header.connection_id) {
        Some(entry) => entry,
        None => {
            if header.packet_type != PacketType::PublicKey {
                /* Unknown connection IDs are silently discarded. */
                return 0;
            }
            match create_udp_child(
                ctx,
                listener_ptr,
                header.connection_id,
                peer_addr,
                peer_port,
            ) {
                Ok(child) => UdpSessionEntry {
                    listener: listener_ptr,
                    connection_id: header.connection_id,
                    child,
                    peer_addr,
                    peer_port,
                },
                Err(error) => return error.errno(),
            }
        }
    };

    if entry.peer_addr != peer_addr || entry.peer_port != peer_port {
        return 0;
    }

    let child = unsafe { &*(entry.child as *const StcpContext) };
    queue_to_context(child, bytes)
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_get_udp_peer(
    raw_ctx: *mut c_void,
    out_addr: *mut u32,
    out_port: *mut u16,
) -> c_int {
    if raw_ctx.is_null() || out_addr.is_null() || out_port.is_null() {
        return -22;
    }
    let ctx = unsafe { &*(raw_ctx as *const StcpContext) };
    let inner = ctx.inner.lock();
    if inner.udp_peer_port == 0 {
        return -107;
    }
    unsafe {
        *out_addr = inner.udp_peer_addr;
        *out_port = inner.udp_peer_port;
    }
    0
}
