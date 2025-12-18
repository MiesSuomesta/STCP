use crate::types::{
    StcpMsgType,
    StcpMessageHeader,
    ProtoSession,
    StcpEcdhPubKey,
    STCP_ECDH_PUB_LEN,
    STCP_ECDH_PUB_XY_LEN,
    STCP_TAG_BYTES,
    STCP_RECV_BLOCK,
    STCP_TCP_RECV_NO_BLOCK,
    kernel_socket,
};
use crate::{stcp_dbg, stcp_dump};
use crate::errorit::*;
use crate::helpers::{tcp_send_all, tcp_recv_once, tcp_recv_exact, tcp_peek_max};

// TCP helpperi makrot
use crate::stcp_tcp_recv_once;
use crate::stcp_tcp_send_all;
use crate::stcp_tcp_recv_exact;
use crate::stcp_tcp_peek_max;
use crate::stcp_tcp_recv_until_buffer_full;

use alloc::vec::Vec;
use alloc::vec;
use core::mem::size_of;

pub fn stcp_message_create_header(msgType: StcpMsgType, plen: u32) -> StcpMessageHeader {
    return StcpMessageHeader {
        version:  1,
        tag:      STCP_TAG_BYTES,
        msg_type: msgType,
        msg_len:  plen,
    };
}

pub fn stcp_message_debug_header(theHdr: StcpMessageHeader) {
  stcp_dbg!("   Version         : {}", theHdr.version);   
  stcp_dbg!("   Tag             : {}", theHdr.tag);   
  stcp_dbg!("   Type            : {}", theHdr.msg_type.to_raw());   
  stcp_dbg!("   Payload length  : {}", theHdr.msg_len);   
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
    let hrdSize = size_of::<StcpMessageHeader>();
    let payloadSize = hrdSize + STCP_ECDH_PUB_LEN as usize;
    payloadSize
}

pub fn stcp_message_form_a_frame_from(ftype: StcpMsgType, data: &[u8]) -> Vec<u8> {

    let headerSize = stcp_message_get_header_size_in_bytes();
    let dataLen = data.len();
    let totalSize = headerSize + dataLen;
    
    stcp_dbg!("Header size: {}", headerSize);   
    stcp_dbg!("Data size  : {}", dataLen);   
    stcp_dbg!("Total size : {}", totalSize);   

    let mut buffer: Vec<u8> = Vec::with_capacity(totalSize);
            buffer.resize(totalSize, 0);

    let header = stcp_message_create_header(ftype, dataLen as u32);
    let headerRaw = header.to_bytes_be();

    // Copy header to buffer
    buffer[0..headerSize].copy_from_slice(&headerRaw);

    // Copy data to buffer
    buffer[headerSize..].copy_from_slice(data);

    buffer
}


pub fn stcp_message_form_a_header_from_data(data: &[u8]) -> StcpMessageHeader {

    let headerSize = stcp_message_get_header_size_in_bytes();
  stcp_dbg!("Header size");   
  stcp_dbg!("Input data size");   

    if data.len() < headerSize {
      stcp_dbg!("Invalid length for STCP frame");   
        return stcp_message_create_header(StcpMsgType::Error, 0);
    }

    let mut header_bytes = &data[0..headerSize];
    let mut hdr = StcpMessageHeader::from_bytes_be(header_bytes);
    stcp_dbg!("Header from bytes:");
    stcp_message_debug_header(hdr);

    hdr
}

pub fn stcp_message_unpack_public_key_from(data: &[u8]) -> (StcpMessageHeader, StcpEcdhPubKey) {

    let mut pk = StcpEcdhPubKey{
        x: [ 0u8; STCP_ECDH_PUB_XY_LEN ],
        y: [ 0u8; STCP_ECDH_PUB_XY_LEN ],
    };

    let headerSize = stcp_message_get_header_size_in_bytes();
    let neededFrameSize = STCP_ECDH_PUB_LEN;

    let (mut msgHdr, theFrame) = stcp_message_unpack_frame_from(data);
    let frameSizeFromHeader = msgHdr.msg_len;
    let frameSizeFromData = theFrame.len();

    if frameSizeFromData as usize != frameSizeFromHeader as usize {
      stcp_dbg!("ERROR Frame size header vs actual differs");   
        msgHdr.msg_type = StcpMsgType::Error;
      stcp_dbg!("Marked as error");   
        return (msgHdr, pk);
    }
    stcp_dbg!("Frame size check passed");   

    if neededFrameSize as usize > frameSizeFromHeader as usize {
      stcp_dbg!("ERROR Need bytes got bytes");   
        msgHdr.msg_type = StcpMsgType::Error;
      stcp_dbg!("Marked as error");   
        return (msgHdr, pk);
    }

    if msgHdr.msg_type != StcpMsgType::Public {
      stcp_dbg!("ERROR Not valid type got");   
        msgHdr.msg_type = StcpMsgType::Error;
      stcp_dbg!("Marked as error");   
        return (msgHdr, pk);
    }
    
  stcp_dbg!("Copying public key");   
    // Checks passed..
    pk.x[0..].copy_from_slice(&theFrame[0..STCP_ECDH_PUB_XY_LEN]);
    pk.y[0..].copy_from_slice(&theFrame[STCP_ECDH_PUB_XY_LEN..]);

  stcp_dbg!("Returning");   
    (msgHdr, pk)
}

pub fn stcp_message_unpack_frame_from(data: &[u8]) -> (StcpMessageHeader, Vec<u8>) {

    let headerSize = stcp_message_get_header_size_in_bytes();
    let dataLen = data.len();

    if dataLen < headerSize {
      stcp_dbg!("Not a frame");   
        let tmp = stcp_message_create_header(StcpMsgType::Error, 0);
        return ( tmp , Vec::with_capacity(0));
    }
    
    let header_bytes = &data[0..headerSize];
    let header = StcpMessageHeader::from_bytes_be(header_bytes);

    let frameSize = header.msg_len;

    if (dataLen != (headerSize as usize + frameSize as usize)) {
      stcp_dbg!("Reported data versus actual differs");   
        let tmp = stcp_message_create_header(StcpMsgType::Error, 0);
        return ( tmp , Vec::with_capacity(0));
    }

    // get the type
    let frame_type = header.msg_type;

    let mut buffer: Vec<u8> = Vec::with_capacity(frameSize as usize);
    buffer.resize(frameSize as usize, 0);

    buffer[0..frameSize as usize].copy_from_slice(&data[headerSize as usize..]);

    (header, buffer)
}

pub fn stcp_message_decrypt_buffer(input: &mut [u8], output: &mut [u8]) -> isize {
  stcp_dbg!("Got input bytes Output bytes max");   
  stcp_dump!("Input data to decrypt", input);
  let mut ln = input.len();

  if (ln > output.len()) {
      stcp_dbg!("Warning Input buffer is bigger than Output buffer");   
      ln = output.len();
  }
  
  stcp_dbg!("Got len to copy");   
  output[..ln].copy_from_slice(&input[..ln]);
  stcp_dump!("Decrypted plain data", output);

  ln as isize
}

pub fn stcp_message_frame_from_raw(framePayloadIn: &[u8]) -> 
                                                    (StcpMessageHeader, Vec<u8>, isize) {

    let hdrSize = stcp_message_get_header_size_in_bytes();

    let framePayloadInLen = framePayloadIn.len();
    
    if framePayloadInLen <= hdrSize as usize {
      return (stcp_message_create_header(StcpMsgType::Unknown, 0), Vec::with_capacity(0), -EAGAIN as isize);
    }

    let hdr: StcpMessageHeader = stcp_message_form_a_header_from_data(&framePayloadIn);
  
    stcp_dbg!("From raw: Got header");   
    stcp_message_debug_header(hdr);
 
    if hdr.tag != STCP_TAG_BYTES {
        stcp_dbg!("RECV FRAME: Receive error No valid TAG found");   
        return (stcp_message_create_header(StcpMsgType::Unknown, 0), Vec::with_capacity(0), -EAGAIN as isize);
    }

    let totalSizeFromHeader:usize = hdr.msg_len as usize;

    if framePayloadInLen < totalSizeFromHeader {
      return (stcp_message_create_header(StcpMsgType::Unknown, 0), Vec::with_capacity(0), -EAGAIN as isize);
    }
    stcp_dump!("Got raw frame", &framePayloadIn);
 
    let payloadSize = totalSizeFromHeader;
    let mut payloadData: Vec<u8> = Vec::with_capacity(payloadSize as usize);
            payloadData.resize(payloadSize as usize, 0);

    let endSourceIndex = hdrSize as usize + payloadSize as usize;
    stcp_dbg!("Copying {} bytes from framePayloadIn as payload", framePayloadInLen);   
    payloadData[0..].copy_from_slice(&framePayloadIn[hdrSize..endSourceIndex]);

    (hdr, payloadData, totalSizeFromHeader as isize)
}

pub fn stcp_message_send_frame(sess: *mut ProtoSession, transport: *mut kernel_socket, ftype: StcpMsgType, payloadToSend: &mut [u8]) -> i32 {

    let s = unsafe { &mut *sess };
    let sock = transport as *mut kernel_socket;
    
    if s.is_server {
      stcp_dbg!("Server Sending frame");   
    } else {
      stcp_dbg!("Client Sending frame");   
    }

    let hdr: StcpMessageHeader = stcp_message_create_header(ftype, payloadToSend.len() as u32);
    let hdrBytes = hdr.to_bytes_be();
    let hdrBytesLen = hdrBytes.len();

    stcp_dbg!("SendFrame Got header");   
    stcp_message_debug_header(hdr);

    let theFrameDataSize = payloadToSend.len() + hdrBytesLen;
    let mut theFrameData: Vec<u8> = Vec::with_capacity(theFrameDataSize as usize);
            theFrameData.resize(theFrameDataSize, 0);

    theFrameData[0..hdrBytesLen].copy_from_slice(&hdrBytes[0..hdrBytesLen]);
    theFrameData[hdrBytesLen..theFrameDataSize].copy_from_slice(&payloadToSend[0..]);

    stcp_dbg!("SendFrame Sending..");   
    let sent = stcp_tcp_send_all!(sock, &theFrameData);
    stcp_dbg!("SendFrame Sent..");   

    sent as i32
}


