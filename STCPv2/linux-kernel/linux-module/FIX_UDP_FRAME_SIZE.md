# UDP frame size fix

`STCP_FRAME_PAYLOAD_LEN` was reduced from 64 KiB to 60 KiB in both
`rust/src/frame.rs` and `rust/src/packet.rs`.

The UDP maximum payload is 65507 bytes. An encrypted STCP frame also contains
its protocol header, nonce/counter fields, and the Poly1305 authentication tag.
A full 64 KiB plaintext chunk therefore cannot fit in one UDP datagram.

60 KiB leaves several kilobytes of headroom for all STCP framing and crypto
overhead.
