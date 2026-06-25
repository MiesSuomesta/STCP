use std::ffi::c_void;
use std::io;
use std::mem;
use std::net::{Ipv4Addr, SocketAddr, SocketAddrV4};
use std::os::fd::AsRawFd;
use std::os::fd::RawFd;

use libc::IF_OPER_LOWERLAYERDOWN;

use crate::crypto::CryptoContext;
use crate::error::StcpError;

use crate::io::{
    raw_socket_peek,
    raw_socket_recv_exact,
    raw_socket_send_exact,
    raw_socket_send_vectored_exact,
};

use crate::packet::STCP_AES_GCM_TAG_LEN;
use crate::packet::STCP_FRAME_PAYLOAD_LEN;
use crate::packet::STCP_IV_LEN;
use crate::packet::STCP_VERSION;
use crate::packet::STCP_INITIAL_RX_BUFFER_LEN;

use crate::types::{
    StcpState,
    StcpContext,
};

use crate::packet::{
    encode_data_packet, encode_public_key_packet, split_data_payload, StcpHeader, StcpPacketType,
    STCP_MAGIC, 
    STCP_HEADER_LEN, 
    STCP_PUBLIC_KEY_LEN, 
    STCP_MAX_PAYLOAD_LEN,
    STCP_SOCKET_TYPE,
};

impl StcpState {
    pub fn name(&self) -> &'static str {
        match self {
            StcpState::New(_) => "New",
            StcpState::Bound { .. } => "Bound",
            StcpState::Listening { .. } => "Listening",
            StcpState::Connected { .. } => "Connected",
            StcpState::Handshake { .. } => "Handshake",
            StcpState::Ready { .. } => "Ready",
            StcpState::Closing { .. } => "Closing",
            StcpState::Closed { .. } => "Closed",
            StcpState::Error { .. } => "Error",
        }
    }

    pub fn get_ctx_fd(&self) -> RawFd {
        match self {
            StcpState::New          (ctx)      => ctx.sk,
            StcpState::Bound        {ctx, .. } => ctx.sk,
            StcpState::Listening    {ctx, .. } => ctx.sk,
            StcpState::Connected    {ctx, .. } => ctx.sk,
            StcpState::Handshake    {ctx, .. } => ctx.sk,
            StcpState::Ready        {ctx, .. } => ctx.sk,
            StcpState::Closing      {ctx, .. } => ctx.sk,
            StcpState::Closed       {ctx, .. } => ctx.sk,
            StcpState::Error        {ctx, .. } => ctx.sk,
        }
    }

}

impl Drop for StcpContext {
    fn drop(&mut self) {
        if self.sk >= 0 {
            unsafe { libc::close(self.sk) };
            self.sk = -1;
        }
    }
}

pub fn stcp_socket() -> Result<StcpState, StcpError> {
    let fd = unsafe { libc::socket(libc::AF_INET, libc::SOCK_STREAM, STCP_SOCKET_TYPE) };

    if fd < 0 {
        return Err(StcpError::Io(io::Error::last_os_error()));
    }

    Ok(StcpState::New(StcpContext {
        sk: fd,
        crypto: CryptoContext::default(),
        rx_payload_buf: Vec::with_capacity(STCP_INITIAL_RX_BUFFER_LEN),
    }))
}

pub fn stcp_bind(state: StcpState, addr: SocketAddr) -> Result<StcpState, StcpError> {
    match state {
        StcpState::New(ctx) => match addr {
            SocketAddr::V4(addr4) => {
                bind_fd(ctx.sk, addr4)?;

                Ok(StcpState::Bound {
                    ctx,
                    at_where: SocketAddr::V4(addr4),
                })
            }

            SocketAddr::V6(_) => Err(StcpError::Unsupported(
                "IPv6 support not implemented yet".to_string(),
            )),
        },

        other => Err(StcpError::InvalidState(other.name())),
    }
}

pub fn stcp_listen(state: StcpState, backlog: i32) -> Result<StcpState, StcpError> {
    match state {
        StcpState::Bound { ctx, at_where } => {
            let rc = unsafe { libc::listen(ctx.sk, backlog) };

            if rc < 0 {
                return Err(StcpError::Io(io::Error::last_os_error()));
            }

            Ok(StcpState::Listening { ctx, at_where })
        }

        other => Err(StcpError::InvalidState(other.name())),
    }
}

pub fn stcp_accept(state: &StcpState) -> Result<StcpState, StcpError> {
    let listen_fd = match state {
        StcpState::Listening { ctx, .. } => ctx.sk,
        other => return Err(StcpError::InvalidState(other.name())),
    };

    let mut addr: libc::sockaddr_in = unsafe { mem::zeroed() };
    let mut len = mem::size_of::<libc::sockaddr_in>() as libc::socklen_t;

    let fd = unsafe {
        libc::accept(
            listen_fd,
            &mut addr as *mut _ as *mut libc::sockaddr,
            &mut len,
        )
    };

    if fd < 0 {
        return Err(StcpError::Io(io::Error::last_os_error()));
    }

    let peer = sockaddr_in_to_socket_addr(addr);

    let mut ctx = StcpContext {
        sk: fd,
        crypto: CryptoContext::default(),
        rx_payload_buf: Vec::with_capacity(STCP_INITIAL_RX_BUFFER_LEN),
    };

    stcp_handshake(&mut ctx)?;

    Ok(StcpState::Ready { ctx })
}

pub fn stcp_connect(state: StcpState, addr: SocketAddr) -> Result<StcpState, StcpError> {
    match state {
        StcpState::New(mut ctx) => {
            connect_fd(ctx.sk, addr)?;

            let connected = StcpState::Connected {
                ctx,
                to_where: addr,
            };

            match connected {
                StcpState::Connected { mut ctx, .. } => {
                    
                    stcp_handshake(&mut ctx)?;
                    Ok(StcpState::Ready { ctx })
                }

                _ => unreachable!(),
            }
        }

        other => Err(StcpError::InvalidState(other.name())),
    }
}

/*
 * RAW packet send/recv.
 * Näitä saa käyttää handshakessa/control-paketeissa.
 * Ei vaadi Ready-tilaa.
 */

pub fn stcp_send_raw_frame(
    ctx: &mut StcpContext,
    packet_type: StcpPacketType,
    payload: &[u8],
) -> Result<usize, StcpError> {
    let pkt = encode_raw_packet(packet_type, payload)?;
    let bytes_sent = match raw_socket_send_exact(ctx.sk, &pkt) {
        Ok(len) => len,
        Err(e) => {
            return Err(
                    StcpError::Protocol(
                        format!(
                            "Peek error! ({})",
                            e,
                        )
                    )
                );
        }
    };

    Ok(bytes_sent)
}

pub fn stcp_recv_raw_frame(ctx: &mut StcpContext) -> Result<StcpHeader, StcpError> {
    let mut recv_header = [0u8; STCP_HEADER_LEN];
    raw_socket_recv_exact(ctx.sk, &mut recv_header)?;

    let message_hdr = StcpHeader::decode(&recv_header)?;
    let payload_size = message_hdr.payload_len as usize;

    if payload_size > STCP_MAX_PAYLOAD_LEN {
        return Err(StcpError::Protocol(format!(
            "payload too large: {}",
            payload_size
        )));
    }

    if ctx.rx_payload_buf.capacity() < payload_size {
        let new_cap = payload_size.next_power_of_two();
        ctx.rx_payload_buf.reserve_exact(
            new_cap - ctx.rx_payload_buf.capacity()
        );
    }

    // Tämä oli puuttuva kriittinen rivi:
    ctx.rx_payload_buf.resize(payload_size, 0);

    raw_socket_recv_exact(
        ctx.sk,
        &mut ctx.rx_payload_buf[..payload_size],
    )?;

    Ok(message_hdr)
}

/*
 * SECURE send/recv.
 * Nämä toimivat vain Ready-tilassa.
 */

 pub fn stcp_send(state: &mut StcpState, data: &[u8]) -> Result<usize, StcpError> {
    match state {
        StcpState::Ready { ctx } => {
            let total = data.len();
            let mut pos = 0;

            while pos < data.len() {
                let end = (pos + STCP_FRAME_PAYLOAD_LEN).min(data.len());
                let chunk = &data[pos..end];

                let (iv, ciphertext) = ctx.crypto.encrypt(chunk)?;

                let packet_type = if end == data.len() {
                    StcpPacketType::DataChunkEnd
                } else {
                    StcpPacketType::DataChunk
                };

                let payload_len = STCP_IV_LEN + ciphertext.len();

                let header = StcpHeader::new(
                    packet_type,
                    0,
                    payload_len as u64,
                )
                .encode();

                raw_socket_send_vectored_exact(
                    ctx.sk,
                    &[&header, &iv, &ciphertext],
                )?;

                pos = end;
            }

            Ok(total)
        }

        other => Err(StcpError::InvalidState(other.name())),
    }
}

pub fn stcp_recv(state: &mut StcpState, out: &mut [u8]) -> Result<usize, StcpError> {
    match state {
        StcpState::Ready { ctx } => {
            let mut total = 0usize;

            loop {
                let header = stcp_recv_raw_frame(ctx)?;

                match header.packet_type {
                    StcpPacketType::DataChunk | StcpPacketType::DataChunkEnd => {

                        let (iv, ciphertext) = split_data_payload(&ctx.rx_payload_buf)?;

                        let plaintext = ctx.crypto.decrypt(&iv, ciphertext)?;

                        if total + plaintext.len() > out.len() {
                            return Err(StcpError::Protocol(format!(
                                "caller buffer too small: need {}, have {}",
                                total + plaintext.len(),
                                out.len()
                            )));
                        }

                        out[total..total + plaintext.len()]
                            .copy_from_slice(&plaintext);

                        total += plaintext.len();

                        if header.packet_type == StcpPacketType::DataChunkEnd {
                            return Ok(total);
                        }
                    }

                    StcpPacketType::Close => {
                        let reason = String::from_utf8_lossy(&ctx.rx_payload_buf).to_string();
                        return Err(StcpError::Closed(reason));
                    }

                    other => {
                        return Err(StcpError::Protocol(format!(
                            "unexpected packet in stcp_recv: {other:?}"
                        )));
                    }
                }
            }
        }

        other => Err(StcpError::InvalidState(other.name())),
    }
}

pub fn stcp_close(state: StcpState, reason: impl Into<String>) -> StcpState {
    let reason = reason.into();

    match state {
        StcpState::New(ctx)
        | StcpState::Bound { ctx, .. }
        | StcpState::Listening { ctx, .. }
        | StcpState::Connected { ctx, .. }
        | StcpState::Handshake    {ctx, .. }
        | StcpState::Ready { ctx }
        | StcpState::Closing { ctx, .. }
        | StcpState::Closed { ctx, .. }
        | StcpState::Error { ctx, .. } => StcpState::Closed { ctx, reason },
    }
}

/*
 * Symmetrinen handshake:
 * molemmat puolet lähettävät public keyn ja vastaanottavat peer public keyn.
 */

fn stcp_handshake(mut ctx: &mut StcpContext) -> Result<(), StcpError> {
    let pk = &ctx.crypto.public_key_64();

    stcp_send_raw_frame(&mut ctx, StcpPacketType::PublicKey, pk)?;

    let header = stcp_recv_raw_frame(ctx)?;

    if header.packet_type != StcpPacketType::PublicKey {
        return Err(StcpError::Protocol(format!(
            "expected public key, got {:?}",
            header.packet_type
        )));
    }

    if ctx.rx_payload_buf.len() != STCP_PUBLIC_KEY_LEN {
        return Err(StcpError::Protocol(format!(
            "bad public key len: {}, expected {}",
            ctx.rx_payload_buf.len(),
            STCP_PUBLIC_KEY_LEN
        )));
    }

    ctx.crypto.derive_aes_key(&ctx.rx_payload_buf)?;

    Ok(())
}

fn encode_raw_packet(
    packet_type: StcpPacketType,
    payload: &[u8],
) -> Result<Vec<u8>, StcpError> {
    match packet_type {
        StcpPacketType::PublicKey => {
            let plen = payload.len() as usize;

            if plen != STCP_PUBLIC_KEY_LEN {
                return Err(StcpError::Protocol(format!(
                    "bad public key len: {}, expected {}",
                    payload.len(),
                    STCP_PUBLIC_KEY_LEN
                )));
            }

            let public_key: &[u8; STCP_PUBLIC_KEY_LEN] =
                payload.try_into()
                    .map_err(|_| {
                        StcpError::Protocol(
                            "invalid public key length".to_string()
                        )
                    })?;

            Ok(encode_public_key_packet(public_key))

        }

        StcpPacketType::Data => {
            Err(StcpError::Protocol(
                "raw Data packet is not allowed; use stcp_send()".to_string(),
            ))
        }

        StcpPacketType::DataChunk | StcpPacketType::DataChunkEnd => {
            let mut out = Vec::with_capacity(STCP_HEADER_LEN + payload.len());

            out.extend_from_slice(&STCP_MAGIC);
            out.push(packet_type_to_u8(packet_type));
            out.push(STCP_VERSION);
            out.extend_from_slice(&0u16.to_be_bytes());
            out.extend_from_slice(&(payload.len() as u64).to_be_bytes());
            out.extend_from_slice(payload);

            Ok(out)
        }

        
        StcpPacketType::Close | StcpPacketType::Error => {
            let mut out = Vec::with_capacity(STCP_HEADER_LEN + payload.len());

            out.extend_from_slice(&STCP_MAGIC);
            out.push(packet_type_to_u8(packet_type));
            out.push(STCP_VERSION);
            out.extend_from_slice(&0u16.to_be_bytes());
            out.extend_from_slice(&(payload.len() as u64).to_be_bytes());
            out.extend_from_slice(payload);

            Ok(out)
        }
    }
}

fn packet_type_to_u8(packet_type: StcpPacketType) -> u8 {
    match packet_type {
        StcpPacketType::PublicKey => 1,
        StcpPacketType::Data => 2,
        StcpPacketType::DataChunk => 3,
        StcpPacketType::DataChunkEnd => 4,
        StcpPacketType::Close => 5,
        StcpPacketType::Error => 6,
    }
}

pub fn peek_magic(fd: RawFd) -> Result<[u8; 4], StcpError> {
    let mut magic = [0u8; 4];
    raw_socket_peek(fd, &mut magic);
    Ok(magic)
}

fn socket_recv_packet(fd: RawFd) -> Result<(StcpHeader, Vec<u8>), StcpError> {
    let magic = peek_magic(fd)?;

    if magic != STCP_MAGIC {
        return Err(StcpError::Protocol(format!(
            "not an STCP packet: {magic:?}"
        )));
    }

    let mut header_buf = [0u8; STCP_HEADER_LEN];
    recv_exact(fd, &mut header_buf)?;

    let header = StcpHeader::decode(&header_buf)?;

    let mut payload = vec![0u8; header.payload_len as usize];
    recv_exact(fd, &mut payload)?;

    Ok((header, payload))
}

fn send_exact(fd: RawFd, mut data: &[u8]) -> Result<(), StcpError> {
    while !data.is_empty() {
        let rc = unsafe {
            libc::send(
                fd,
                data.as_ptr() as *const c_void,
                data.len(),
                0,
            )
        };

        if rc < 0 {
            let e = io::Error::last_os_error();

            if e.kind() == io::ErrorKind::Interrupted {
                continue;
            }

            return Err(StcpError::Io(e));
        }

        if rc == 0 {
            return Err(StcpError::Closed("send returned 0".to_string()));
        }

        data = &data[rc as usize..];
    }

    Ok(())
}

fn recv_exact(fd: RawFd, out: &mut [u8]) -> Result<(), StcpError> {
    recv_exact_flags(fd, out, 0)
}

fn recv_exact_flags(
    fd: RawFd,
    mut out: &mut [u8],
    flags: i32,
) -> Result<(), StcpError> {
    while !out.is_empty() {
        let rc = unsafe {
            libc::recv(
                fd,
                out.as_mut_ptr() as *mut c_void,
                out.len(),
                flags,
            )
        };

        if rc < 0 {
            let e = io::Error::last_os_error();

            if e.kind() == io::ErrorKind::Interrupted {
                continue;
            }

            return Err(StcpError::Io(e));
        }

        if rc == 0 {
            return Err(StcpError::Closed("recv returned 0".to_string()));
        }

        let n = rc as usize;
        let tmp = out;
        out = &mut tmp[n..];
    }

    Ok(())
}

fn bind_fd(fd: RawFd, addr: SocketAddrV4) -> Result<(), StcpError> {
    let ip = addr.ip().octets();
    let port = addr.port();

    let sockaddr = libc::sockaddr_in {
        sin_family: libc::AF_INET as libc::sa_family_t,
        sin_port: port.to_be(),
        sin_addr: libc::in_addr {
            s_addr: u32::from_ne_bytes(ip),
        },
        sin_zero: [0; 8],
    };

    let ret = unsafe {
        libc::bind(
            fd,
            &sockaddr as *const libc::sockaddr_in as *const libc::sockaddr,
            mem::size_of::<libc::sockaddr_in>() as libc::socklen_t,
        )
    };

    if ret < 0 {
        return Err(StcpError::Io(io::Error::last_os_error()));
    }

    Ok(())
}

fn connect_fd(fd: RawFd, addr: SocketAddr) -> Result<(), StcpError> {
    let (raw, len) = socket_addr_to_sockaddr_in(addr)?;

    let rc = unsafe {
        libc::connect(
            fd,
            &raw as *const _ as *const libc::sockaddr,
            len,
        )
    };

    if rc < 0 {
        return Err(StcpError::Io(io::Error::last_os_error()));
    }

    Ok(())
}

fn socket_addr_to_sockaddr_in(
    addr: SocketAddr,
) -> Result<(libc::sockaddr_in, libc::socklen_t), StcpError> {
    match addr {
        SocketAddr::V4(v4) => {
            let raw = libc::sockaddr_in {
                sin_family: libc::AF_INET as libc::sa_family_t,
                sin_port: v4.port().to_be(),
                sin_addr: libc::in_addr {
                    s_addr: u32::from_ne_bytes(v4.ip().octets()),
                },
                sin_zero: [0; 8],
            };

            Ok((
                raw,
                mem::size_of::<libc::sockaddr_in>() as libc::socklen_t,
            ))
        }

        SocketAddr::V6(_) => Err(StcpError::Unsupported(
            "IPv6 support not implemented yet".to_string(),
        )),
    }
}

fn sockaddr_in_to_socket_addr(addr: libc::sockaddr_in) -> SocketAddr {
    let ip = Ipv4Addr::from(addr.sin_addr.s_addr.to_ne_bytes());
    let port = u16::from_be(addr.sin_port);

    SocketAddr::V4(SocketAddrV4::new(ip, port))
}
