# STCPv2 V6.4 — TCP stream parser resynchronisation

Observed failure:

    stcp: Rust carrier receive failed: -71

`-71` is `-EPROTO`. The handshake parser decoded only the queue prefix and,
on an invalid/misaligned header, returned the error without consuming any
bytes. The same bad prefix was therefore parsed again after every subsequent
TCP receive, permanently stalling the connection.

## Fix

`process_handshake_frames()` now behaves as a byte-stream parser:

- incomplete headers and frames remain queued;
- a malformed prefix discards one byte and searches again;
- the parser continues until a valid STCP v2 header is aligned;
- one corrupt/misaligned prefix can no longer poison the connection forever.

All V6.3 connect-handshake, V6.2 kthread lifetime, V6.1 wakeup and ByteQueue
optimisations remain included.
