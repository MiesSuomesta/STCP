# STCP IPv4 / DNS / carrier connect debug update

Changes in this package:

- Ethernet benchmark default host is `192.168.1.20` instead of `lja.fi`.
- Numeric IPv4 hosts use a direct `sockaddr_in` fast path and do not depend on DNS.
- Hostname resolution logs DNS start, result metadata and a clear failure hint.
- The STCP offload builds a clean AF_INET carrier address before native TCP connect.
- Connect logs include incoming address length, `sizeof(sockaddr_in)`, carrier family,
  carrier address length and final target.
- Existing STCP blocking-connect feature flag, ping command and connect diagnostics remain.

Expected test:

```
stcp config host 192.168.1.20
stcp config port 19002
stcp config transport stcp
stcp config chunk 4096
stcp config total 65536
stcp bench upload
```

Expected key logs:

```
APP RESOLVE IPV4 FAST PATH ...
CONNECT ENTER ...
CARRIER TARGET ... family=2 ...
CALLING CARRIER CONNECT ...
CARRIER CONNECT RETURN ...
```
