# UDP listener / accept / demux changes

- STCP wire header increased from 32 to 40 bytes.
- Added big-endian 64-bit `connection_id` at bytes 32..39.
- UDP client allocates a non-zero connection ID before its first PublicKey frame.
- UDP listener receiver reads source IPv4/port with `msg_name`.
- Rust listener demultiplexes by listener context, connection ID and peer tuple.
- First valid PublicKey creates a pending child context and wakes accept().
- UDP accepted child carrier shares the listener socket using a reference count.
- Each child carrier stores its own destination address for kernel_sendmsg().
- Unknown connection IDs and mismatched peer tuples are dropped.
- Session release unregisters UDP demux entries before freeing the context.
- Added `make test-udp` and `testing/stcp_udp_test.c` for two simultaneous clients.

Important lifecycle rule: close accepted child sockets before closing the UDP
listener. The listener owns the receive thread and shared kernel UDP socket.
