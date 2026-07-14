# STCP kernel BSD socket implementation – loopback backend

This package implements the complete BSD socket operation path for testing:

- `socket`
- `bind`
- `listen`
- blocking and non-blocking `accept`
- `connect`
- `sendmsg`
- blocking and non-blocking `recvmsg`
- `poll`
- `shutdown`
- `release`

The Rust backend is an **in-kernel loopback transport**. A client connecting to
an address registered by a listener gets a paired connection. Bytes sent by one
endpoint are queued for the peer endpoint.

This deliberately does not yet transmit IP/UDP packets or perform the STCP
cryptographic handshake. It gives a stable end-to-end BSD socket baseline
before those layers are replaced one at a time.

## Build

```bash
make LLVM=1 V=1 module
```

The Makefile uses:

- nightly Rust
- `-Zbuild-std=core,alloc`
- static relocation model
- kernel code model
- `-Z plt=yes`, required to avoid unsupported `R_X86_64_GOTPCREL` relocations

## Load

```bash
sudo insmod stcp.ko
sudo dmesg | tail -n 30
```

## Test

Build the test program:

```bash
cc -O2 -Wall testing/stcp_test.c -o testing/stcp_test
```

Terminal 1:

```bash
./testing/stcp_test server
```

Terminal 2:

```bash
./testing/stcp_test client
```

## Next replacement order

1. Keep all C `proto_ops` wrappers unchanged.
2. Replace Rust loopback registry/connect with the actual carrier.
3. Replace byte queues with STCP frame queues.
4. Add symmetric key exchange.
5. Encrypt/decrypt data frames.
