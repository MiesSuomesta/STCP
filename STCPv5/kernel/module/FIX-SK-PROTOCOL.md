# Preserve STCP protocol in `struct sock`

`stcp_accept()` reads `sock->sk->sk_protocol` to distinguish STCP-TCP (253)
from STCP-UDP (254), and the external TCP child path forwards the same value to
Rust. The create path previously never initialized that field, leaving it as 0.

The fix assigns:

```c
sk->sk_protocol = protocol;
```

immediately after `sock_init_data()`.
