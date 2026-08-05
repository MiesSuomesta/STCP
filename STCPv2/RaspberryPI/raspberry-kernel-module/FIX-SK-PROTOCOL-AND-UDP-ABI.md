# Raspberry STCP protocol and UDP accept fix

Two related fixes are included:

1. Store the selected STCP protocol in `sk->sk_protocol`. Without this,
   STCP-UDP `accept()` enters the TCP branch and calls the UDP carrier accept
   helper with a NULL Rust child context, resulting in `EINVAL`.
2. Keep the public SDK ABI stream-shaped for both STCP-TCP/253 and STCP-UDP/254.
   The UDP carrier underneath remains an AF_INET datagram socket.
