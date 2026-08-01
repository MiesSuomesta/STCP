# STCP connect diagnostics and ping shell restoration

This package adds high-detail diagnostics without changing the STCP wire protocol.

## New diagnostics

The application logs:

- DNS resolution entry/result
- socket family/type/protocol and returned fd
- nonblocking setup
- `zsock_connect()` entry/result/errno
- poll completion and `SO_ERROR`
- final connect failure code

The STCP socket-offload layer logs:

- connect callback entry
- context, STCP fd, carrier fd, state and target address
- native carrier `connect()` entry/result/errno
- asynchronous wait result
- exact early-exit letter and reason
- successful carrier connection

## Ping shell

Restores:

```text
stcp ping <ip|name> [count] [timeout_ms]
```

## Suggested first test

```text
stcp ping 192.168.1.20
stcp config transport stcp
stcp config host 192.168.1.20
stcp config port 19002
stcp config chunk 4096
stcp config total 65536
stcp bench upload
```
