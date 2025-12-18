use crate::types::{
    StcpMsgType,
    StcpMessageHeader,
    ProtoSession,
    StcpEcdhPubKey,
    STCP_ECDH_PUB_LEN,
    STCP_ECDH_PUB_XY_LEN,
    STCP_TAG_BYTES,
    kernel_socket,
};
use crate::{stcp_dbg, stcp_dump};
use crate::errorit::*;
//use crate::helpers::{tcp_send_all, tcp_recv_once, tcp_recv_exact, tcp_peek_max};

// TCP helpperi makrot
//use crate::stcp_tcp_recv_once;
use crate::stcp_tcp_send_all;
//use crate::stcp_tcp_recv_exact;
//use crate::stcp_tcp_peek_max;
//use crate::stcp_tcp_recv_until_buffer_full;

use alloc::vec::Vec;
use alloc::vec;
use core::mem::size_of;

pub fn stcp_message_create_header(the_message_type: StcpMsgType, plen: u32) -> StcpMessageHeader {
    return StcpMessageHeader {
        version:  1,
        tag:      STCP_TAG_BYTES,
        msg_type: the_message_type,
        msg_len:  plen,
    };
}

pub fn stcp_message_debug_header(the_header: StcpMessageHeader) {
  stcp_dbg!("   Version         : {}", the_header.version);   
  stcp_dbg!("   Tag             : {}", the_header.tag);   
  stcp_dbg!("   Type            : {}", the_header.msg_type.to_raw());   
  stcp_dbg!("   Payload length  : {}", the_header.msg_len);   
}


pub fn stcp_message_form_a_frame_from_public_key(ftype: StcpMsgType, pk:StcpEcdhPubKey) -> Vec<u8> {
    let mut buffer: Vec<u8> = vec![0u8; STCP_ECDH_PUB_LEN];
    buffer.resize(STCP_ECDH_PUB_LEN, 0);
    buffer[0..STCP_ECDH_PUB_XY_LEN]
        .copy_from_slice(&pk.x[0..STCP_ECDH_PUB_LEN]);
    buffer[STCP_ECDH_PUB_XY_LEN..STCP_ECDH_PUB_LEN]
        .copy_from_slice(&pk.y[0..STCP_ECDH_PUB_LEN]);

    stcp_message_form_a_frame_from(ftype, &buffer)
}

pub fn stcp_message_get_header_size_in_bytes() -> usize {
    size_of::<StcpMessageHeader>()
}

pub fn stcp_message_get_public_key_frame_size_in_bytes() -> usize {
    let header_size_in_bytes = size_of::<StcpMessageHeader>();
    let the_payload_size = header_size_in_bytes + STCP_ECDH_PUB_LEN as usize;
    the_payload_size
}

pub fn stcp_message_form_a_frame_from(ftype: StcpMsgType, data: &[u8]) -> Vec<u8> {

    let header_size_in_bytes = stcp_message_get_header_size_in_bytes();
    let data_lenght = data.len();
    let total_size = header_size_in_bytes + data_lenght;
    
    stcp_dbg!("Header size: {}", header_size_in_bytes);   
    stcp_dbg!("Data size  : {}", data_lenght);   
    stcp_dbg!("Total size : {}", total_size);   

    let mut buffer: Vec<u8> = Vec::with_capacity(total_size);
            buffer.resize(total_size, 0);

    let header = stcp_message_create_header(ftype, data_lenght as u32);
    let header_raw_bytes = header.to_bytes_be();

    // Copy header to buffer
    buffer[0..header_size_in_bytes].copy_from_slice(&header_raw_bytes);

    // Copy data to buffer
    buffer[header_size_in_bytes..].copy_from_slice(data);

    buffer
}


pub fn stcp_message_form_a_header_from_data(data: &[u8]) -> StcpMessageHeader {

    let header_size_in_bytes = stcp_message_get_header_size_in_bytes();
  stcp_dbg!("Header size");   
  stcp_dbg!("Input data size");   

    if data.len() < header_size_in_bytes {
      stcp_dbg!("Invalid length for STCP frame");   
        return stcp_message_create_header(StcpMsgType::Error, 0);
    }

    let header_bytes = &data[0..header_size_in_bytes];
    let hdr = StcpMessageHeader::from_bytes_be(header_bytes);
    stcp_dbg!("Header from bytes:");
    stcp_message_debug_header(hdr);

    hdr
}

pub fn stcp_message_unpack_public_key_from(data: &[u8]) -> (StcpMessageHeader, StcpEcdhPubKey) {

    let mut pk = StcpEcdhPubKey{
        x: [ 0u8; STCP_ECDH_PUB_XY_LEN ],
        y: [ 0u8; STCP_ECDH_PUB_XY_LEN ],
    };

    //let header_size_in_bytes = stcp_message_get_header_size_in_bytes();
    let needed_frame_size = STCP_ECDH_PUB_LEN;

    let (mut the_message_header, the_frame) = stcp_message_unpack_frame_from(data);
    let frame_size_from_header = the_message_header.msg_len;
    let frame_size_from_data = the_frame.len();

    if frame_size_from_data as usize != frame_size_from_header as usize {
      stcp_dbg!("ERROR Frame size header vs actual differs");   
        the_message_header.msg_type = StcpMsgType::Error;
      stcp_dbg!("Marked as error");   
        return (the_message_header, pk);
    }
    stcp_dbg!("Frame size check passed");   

    if needed_frame_size as usize > frame_size_from_header as usize {
      stcp_dbg!("ERROR Need bytes got bytes");   
        the_message_header.msg_type = StcpMsgType::Error;
      stcp_dbg!("Marked as error");   
        return (the_message_header, pk);
    }

    if the_message_header.msg_type != StcpMsgType::Public {
      stcp_dbg!("ERROR Not valid type got");   
        the_message_header.msg_type = StcpMsgType::Error;
      stcp_dbg!("Marked as error");   
        return (the_message_header, pk);
    }
    
  stcp_dbg!("Copying public key");   
    // Checks passed..
    pk.x[0..].copy_from_slice(&the_frame[0..STCP_ECDH_PUB_XY_LEN]);
    pk.y[0..].copy_from_slice(&the_frame[STCP_ECDH_PUB_XY_LEN..]);

  stcp_dbg!("Returning");   
    (the_message_header, pk)
}

pub fn stcp_message_unpack_frame_from(data: &[u8]) -> (StcpMessageHeader, Vec<u8>) {

    let header_size_in_bytes = stcp_message_get_header_size_in_bytes();
    let data_lenght = data.len();

    if data_lenght < header_size_in_bytes {
      stcp_dbg!("Not a frame");   
        let tmp = stcp_message_create_header(StcpMsgType::Error, 0);
        return ( tmp , Vec::with_capacity(0));
    }
    
    let header_bytes = &data[0..header_size_in_bytes];
    let header = StcpMessageHeader::from_bytes_be(header_bytes);

    let frame_size = header.msg_len;

    if data_lenght != (header_size_in_bytes as usize + frame_size as usize) {
      stcp_dbg!("Reported data versus actual differs");   
        let tmp = stcp_message_create_header(StcpMsgType::Error, 0);
        return ( tmp , Vec::with_capacity(0));
    }

    // get the type
    //let frame_type = header.msg_type;

    let mut buffer: Vec<u8> = Vec::with_capacity(frame_size as usize);
    buffer.resize(frame_size as usize, 0);

    buffer[0..frame_size as usize].copy_from_slice(&data[header_size_in_bytes as usize..]);

    (header, buffer)
}

pub fn stcp_message_decrypt_buffer(input: &mut [u8], output: &mut [u8]) -> isize {
  stcp_dbg!("Got input bytes Output bytes max");   
  stcp_dump!("Input data to decrypt", input);
  let mut ln = input.len();

  if ln > output.len() {
      stcp_dbg!("Warning Input buffer is bigger than Output buffer");   
      ln = output.len();
  }
  
  stcp_dbg!("Got len to copy");   
  output[..ln].copy_from_slice(&input[..ln]);
  stcp_dump!("Decrypted plain data", output);

  ln as isize
}

pub fn stcp_message_frame_from_raw(frame_payload_in: &[u8]) -> 
                                                    (StcpMessageHeader, Vec<u8>, isize) {

    let header_size_in_bytes = stcp_message_get_header_size_in_bytes();

    let frame_payload_in_len = frame_payload_in.len();
    
    if frame_payload_in_len <= header_size_in_bytes as usize {
      return (stcp_message_create_header(StcpMsgType::Unknown, 0), Vec::with_capacity(0), -EAGAIN as isize);
    }

    let hdr: StcpMessageHeader = stcp_message_form_a_header_from_data(&frame_payload_in);
  
    stcp_dbg!("From raw: Got header");   
    stcp_message_debug_header(hdr);
 
    if hdr.tag != STCP_TAG_BYTES {
        stcp_dbg!("RECV FRAME: Receive error No valid TAG found");   
        return (stcp_message_create_header(StcpMsgType::Unknown, 0), Vec::with_capacity(0), -EAGAIN as isize);
    }

    let total_size_from_header:usize = hdr.msg_len as usize;

    if frame_payload_in_len < total_size_from_header {
      return (stcp_message_create_header(StcpMsgType::Unknown, 0), Vec::with_capacity(0), -EAGAIN as isize);
    }
    stcp_dump!("Got raw frame", &frame_payload_in);
 
    let the_payload_size = total_size_from_header;
    let mut the_payload_data: Vec<u8> = Vec::with_capacity(the_payload_size as usize);
            the_payload_data.resize(the_payload_size as usize, 0);

    let end_source_index = header_size_in_bytes as usize + the_payload_size as usize;
    stcp_dbg!("Copying {} bytes from frame_payload_in as payload", frame_payload_in_len);   
    the_payload_data[0..].copy_from_slice(&frame_payload_in[header_size_in_bytes..end_source_index]);

    (hdr, the_payload_data, total_size_from_header as isize)
}

pub fn stcp_message_send_frame(sess: *mut ProtoSession, transport: *mut kernel_socket, ftype: StcpMsgType, payload_to_send: &mut [u8]) -> i32 {

    let s = unsafe { &mut *sess };
    let sock = transport as *mut kernel_socket;
    
    if s.is_server {
      stcp_dbg!("Server Sending frame");   
    } else {
      stcp_dbg!("Client Sending frame");   
    }

    let hdr: StcpMessageHeader = stcp_message_create_header(ftype, payload_to_send.len() as u32);
    let header_bytes = hdr.to_bytes_be();
    let header_lenght_in_bytes = header_bytes.len();

    stcp_dbg!("SendFrame Got header");   
    stcp_message_debug_header(hdr);

    let the_frame_data_size = payload_to_send.len() + header_lenght_in_bytes;
    let mut the_frame_data: Vec<u8> = Vec::with_capacity(the_frame_data_size as usize);
            the_frame_data.resize(the_frame_data_size, 0);

    the_frame_data[0..header_lenght_in_bytes].copy_from_slice(&header_bytes[0..header_lenght_in_bytes]);
    the_frame_data[header_lenght_in_bytes..the_frame_data_size].copy_from_slice(&payload_to_send[0..]);

    stcp_dbg!("SendFrame Sending..");   
    let sent = stcp_tcp_send_all!(sock, &the_frame_data);
    stcp_dbg!("SendFrame Sent..");   

    sent as i32
}


