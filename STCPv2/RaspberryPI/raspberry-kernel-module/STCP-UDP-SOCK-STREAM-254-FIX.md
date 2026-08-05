# Raspberry STCP-UDP socket type fix

The AF_STCP family now accepts the SDK ABI used for STCP-UDP:

- STCP-TCP: `socket(AF_STCP, SOCK_STREAM, 253)`
- STCP-UDP: `socket(AF_STCP, SOCK_STREAM, 254)`

The UDP carrier continues to create an internal `AF_INET/SOCK_DGRAM` socket.
