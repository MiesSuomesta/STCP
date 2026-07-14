use alloc::{
    boxed::Box,
    sync::Arc,
    vec::Vec,
};

use core::{
    ffi::c_void,
    ptr,
};

use crate::{
    error::StcpError,
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
    let (shared, side) = {
        let inner = ctx.inner.lock();

        if inner.state != SocketState::Connected {
            return Err(StcpError::InvalidState);
        }

        let endpoint = inner
            .connection
            .as_ref()
            .ok_or(StcpError::InvalidState)?;

        (endpoint.shared.clone(), endpoint.side)
    };

    if shared.peer_closed(side) {
        return Err(StcpError::Closed);
    }

    match side {
        Side::A => {
            let mut queue = shared.a_to_b.lock();
            queue.extend(data.iter().copied());
        }
        Side::B => {
            let mut queue = shared.b_to_a.lock();
            queue.extend(data.iter().copied());
        }
    }

    wake_recv(shared.peer_owner(side));
    Ok(data.len())
}

pub fn recv(
    ctx: &StcpContext,
    output: &mut [u8],
) -> Result<usize, StcpError> {
    let (shared, side) = {
        let inner = ctx.inner.lock();

        if inner.state != SocketState::Connected {
            return Err(StcpError::InvalidState);
        }

        let endpoint = inner
            .connection
            .as_ref()
            .ok_or(StcpError::InvalidState)?;

        (endpoint.shared.clone(), endpoint.side)
    };

    let count = match side {
        Side::A => {
            let mut queue = shared.b_to_a.lock();
            copy_from_queue(&mut queue, output)
        }
        Side::B => {
            let mut queue = shared.a_to_b.lock();
            copy_from_queue(&mut queue, output)
        }
    };

    if count != 0 {
        return Ok(count);
    }

    if shared.peer_closed(side) {
        return Ok(0);
    }

    Err(StcpError::Again)
}

fn copy_from_queue(
    queue: &mut alloc::collections::VecDeque<u8>,
    output: &mut [u8],
) -> usize {
    let count = output.len().min(queue.len());

    for slot in output.iter_mut().take(count) {
        *slot = queue.pop_front().unwrap_or_default();
    }

    count
}

pub fn has_accept(ctx: &StcpContext) -> bool {
    let inner = ctx.inner.lock();
    !inner.accept_queue.is_empty()
}

pub fn has_data(ctx: &StcpContext) -> bool {
    let (shared, side) = {
        let inner = ctx.inner.lock();

        let Some(endpoint) = &inner.connection else {
            return false;
        };

        (endpoint.shared.clone(), endpoint.side)
    };

    let queue_has_data = match side {
        Side::A => !shared.b_to_a.lock().is_empty(),
        Side::B => !shared.a_to_b.lock().is_empty(),
    };

    queue_has_data || shared.peer_closed(side)
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
