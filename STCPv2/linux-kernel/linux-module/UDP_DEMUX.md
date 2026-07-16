# UDP demultiplexing implementation

Wire header size is now 40 bytes. Bytes 32..39 contain a big-endian 64-bit
connection ID. TCP sessions use ID zero; UDP clients allocate a non-zero ID.

The listener owns the only UDP receive thread. Rust keeps a demux table with
listener context, connection ID, child context and peer tuple. Accepted child
carriers share the listener socket and send with `msg_name` set to their peer.

This version intentionally queues the child for `accept()` when the first valid
PublicKey frame arrives. The handshake completes asynchronously after C has
attached the child carrier. Blocking `recv()` and `send()` already wait for the
Ready state, so the BSD interface remains synchronous to applications.
