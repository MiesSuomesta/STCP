# SEND & RECV FLOW

## Overview

STCP integrates with the Linux networking stack by intercepting socket sendmsg() and recvmsg() operations. Encryption and decryption occur transparently inside the kernel module.

## Send Path (sendmsg)

```
Userspace send()
    ↓
stcp_sendmsg()
    ↓
Rust: encrypt + frame construction
    ↓
TCP sendmsg()
    ↓
Network
```

### Send Path Details

1. Userspace application calls send()
2. STCP checks connection state
3. If AES\_READY:
   - Payload is encrypted using AES-GCM
   - STCP frame header is prepended
4. Frame is passed to the TCP stack
5. send() returns the plaintext byte count

If AES\_READY is not set, data may be transmitted unencrypted as part of the handshake protocol.

## Receive Path (recvmsg)

```
Network
    ↓
TCP recvmsg()
    ↓
Rust: buffer + parse frame
    ↓
Rust: decrypt payload (AES mode)
    ↓
stcp_recvmsg()
    ↓
Userspace recv()
```

### Receive Path Details

1. TCP delivers raw bytes to STCP
2. Rust buffers incoming data
3. Complete frames are parsed
4. If AES_READY:
   - Payload is decrypted
5. Plaintext is copied to userspace buffer
6. recv() returns plaintext byte count

## Blocking and Non-blocking Semantics

- Blocking sockets wait until a full frame is available
- Non-blocking sockets return `-EAGAIN` if a complete frame is not yet available
- No busy loops are used

## Send/Receive Invariants

- Userspace never sees encrypted data
- Wire data is always encrypted after handshake
- send()/recv() semantics match standard TCP behavior
- Errors are propagated using standard errno values
