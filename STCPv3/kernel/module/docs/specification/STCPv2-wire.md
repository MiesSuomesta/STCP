# STCPv2 wire protocol – compact header, sequence and ACK phase

## Compact header

All multi-byte integer fields are network byte order.

The STCPv2 carrier/session already identifies the protocol, so the previous
4-byte ASCII magic `STCP` is no longer sent in every frame. Packet type,
protocol version and the currently available wire flags are packed into one
descriptor byte.

| Offset | Size | Field |
|---:|---:|---|
| 0 | 1 | Descriptor: bits 0..3 packet type, bits 4..5 version, bits 6..7 flags |
| 1 | 4 | Payload length (`u32`) |
| 5 | 4 | Sequence number (`u32` on wire, `u64` internally) |
| 9 | 4 | Acknowledgment number (`u32` on wire, `u64` internally) |
| 13 | 4 | Connection ID (`u32` on wire, `u64` API/internal representation) |

Header size is **17 bytes** (previously 40 bytes).

The compact wire representation preserves the existing internal `u64`
sequence/ACK types. Encoding rejects sequence, ACK or connection ID values
larger than `u32::MAX`, preventing silent truncation. Connection IDs are
currently allocated from an `AtomicU32`, so the compact connection-ID field
does not reduce the existing allocator's ID space.

The payload-length field remains capable of representing the current
`STCP_MAX_PAYLOAD_LEN` of 64 MiB.

## Descriptor byte

```
bit:   7 6 | 5 4 | 3 2 1 0
       flags  ver   packet type
```

- Packet type: 4 bits, values 0..15 (currently 1..9 used)
- Version: 2 bits (STCPv2 = `2`)
- Flags: 2 bits reserved/available; current frames send zero

## Packet types

- `1 PublicKey`
- `2 HandshakeDone`
- `3 DataChunk`
- `4 DataChunkEnd`
- `5 Ack`
- `6 Ping`
- `7 Pong`
- `8 Close`
- `9 Reset`

## DATA

Each encrypted DATA frame has a monotonically increasing sequence number. The
sequence remains a `u64` in the reliability state while its wire encoding is
`u32`. A session must be renewed before sequence `u32::MAX` would be exceeded.

The AEAD nonce remains encoded separately as an 8-byte payload field in this
revision. Removing that redundancy is intentionally left as a separate crypto
wire-format change.

The complete compact header remains authenticated as associated data.

## ACK

An ACK frame has no payload. Its acknowledgment field contains the highest
successfully authenticated DATA sequence received. ACKs are generated only
after successful AEAD verification.

## Replay and ordering

The receiver accepts only the exact next sequence and nonce. Duplicate,
skipped, or reordered DATA frames move the session to protocol error. Sliding
window reordering support is intentionally deferred.

## PING/PONG

PING and PONG have no payload. PONG copies the PING sequence into its sequence
field. They are control frames and do not consume DATA sequence numbers.

## CLOSE/RESET

CLOSE performs orderly EOF. RESET marks a protocol failure and immediately
invalidates the session.
