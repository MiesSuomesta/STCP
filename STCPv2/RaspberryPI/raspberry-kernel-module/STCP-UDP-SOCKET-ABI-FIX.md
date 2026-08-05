# Raspberry STCP-UDP socket ABI fix

The SDK exposes both STCP transports as stream-style AF_STCP sockets:

- `socket(AF_STCP, SOCK_STREAM, 253)` selects the TCP carrier.
- `socket(AF_STCP, SOCK_STREAM, 254)` selects the UDP carrier.

The Raspberry module incorrectly required `SOCK_DGRAM` for protocol 254 and returned `EPROTOTYPE`. `stcp_create()` now requires `SOCK_STREAM` for both protocols, matching the Linux module and SDK. The internal carrier remains `AF_INET/SOCK_DGRAM/IPPROTO_UDP` for protocol 254.
