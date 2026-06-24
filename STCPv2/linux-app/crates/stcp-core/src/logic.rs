use std::ffi::c_void;
use std::io;
use std::mem;
use std::net::{Ipv4Addr, SocketAddr, SocketAddrV4};
use std::os::fd::RawFd;

use crate::crypto::CryptoContext;
use crate::error::StcpError;
use crate::packet::{
    encode_data_packet, encode_public_key_packet, split_data_payload, StcpHeader, StcpPacketType,
    STCP_HEADER_LEN, STCP_MAGIC, STCP_PUBLIC_KEY_LEN,
    STCP_SOCKET_TYPE,
};

use crate::io::{
    raw_socket_recv,
    raw_socket_send,
    raw_socket_peek,
    raw_socket_recv_exact,
    raw_socket_send_exact,
};

use crate::socket::peek_magic;
use crate::types::{
    StcpContext,
    StcpState,
};

pub fn stcp_handshake(ctx: &mut StcpContext) -> Result<(), StcpError> {
    let my_pk = ctx.crypto.public_key_64();
    let pkt = encode_public_key_packet(&my_pk);

    raw_socket_send_exact(ctx.sk, &pkt)
        .map_err(|e| {
            StcpError::Protocol(format!(
                "STCP handshake: public key send error: {:?}",
                e
            ))
        })?;

    let magic = peek_magic(ctx.sk)?;

    if magic != STCP_MAGIC {
        return Err(StcpError::Protocol(format!(
            "STCP handshake: payload not STCP: {:?}",
            magic
        )));
    }

    let mut hdr_buf = [0u8; STCP_HEADER_LEN];

    raw_socket_recv_exact(ctx.sk, &mut hdr_buf)
        .map_err(|e| {
            StcpError::Protocol(format!(
                "STCP handshake: header recv error: {:?}",
                e
            ))
        })?;

    let peer_hdr = StcpHeader::decode(&hdr_buf)?;

    if peer_hdr.packet_type != StcpPacketType::PublicKey {
        return Err(StcpError::Protocol(format!(
            "STCP handshake: expected PublicKey, got {:?}",
            peer_hdr.packet_type
        )));
    }

    let peer_pay_len = peer_hdr.payload_len as usize;

    if peer_pay_len != STCP_PUBLIC_KEY_LEN {
        return Err(StcpError::Protocol(format!(
            "STCP handshake: bad public key len: {}, expected {}",
            peer_pay_len,
            STCP_PUBLIC_KEY_LEN
        )));
    }

    let mut peer_pk = [0u8; STCP_PUBLIC_KEY_LEN];

    raw_socket_recv_exact(ctx.sk, &mut peer_pk)
        .map_err(|e| {
            StcpError::Protocol(format!(
                "STCP handshake: public key recv error: {:?}",
                e
            ))
        })?;

    ctx.crypto.derive_aes_key(&peer_pk)?;

    Ok(())
}