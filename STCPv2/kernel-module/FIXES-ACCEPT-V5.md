# STCP stream accept V5

## Kernel fix

The stream accept path now calls `kernel_accept()` on the underlying TCP listener **before** allocating the Rust child and AF_STCP child socket. This consumes the TCP backlog directly and removes the previous provisional-child/accept ordering race.

Distinctive log markers:

```text
stcp: ACCEPT-V5 enter ...
stcp: ACCEPT-V5 carrier accepted ...
stcp: ACCEPT-V5 complete ...
```

If `ACCEPT-V5 enter` is absent, the Raspberry is not running the module from this package.

## Native test server

`benchmark/stcp_native_server.c` removes CPython and ctypes from the STCP accept test. Build it on Raspberry:

```bash
cd ~/benchmark
bash build-native-server.sh
```

`start-servers.sh` uses it automatically when present.
