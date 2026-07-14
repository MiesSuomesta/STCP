# STCP framed kernel implementation

This is the next step after the working in-kernel loopback backend.

The BSD socket layer is unchanged. Rust `send` now converts application bytes
into STCP v2 frames, queues the encoded wire bytes, and Rust `recv` parses and
reassembles them.

Implemented frame format:

- magic: `STCP`
- version: `2`
- 16-byte header
- `DataChunk`
- `DataChunkEnd`
- `Close`
- maximum frame payload: 64 KiB

This version still uses the in-kernel loopback carrier. Crypto and a real
network carrier are intentionally the next layers.

## Build

```bash
make LLVM=1 V=1 module
sudo insmod stcp.ko
```

## Basic test

```bash
cc -O2 -Wall testing/stcp_test.c -o testing/stcp_test
./testing/stcp_test server
./testing/stcp_test client
```

## Framing test

```bash
cc -O2 -Wall testing/stcp_large_test.c -o testing/stcp_large_test
```

Terminal 1:

```bash
./testing/stcp_large_test server
```

Terminal 2:

```bash
./testing/stcp_large_test client
```

The test sends 200 KiB, forcing multiple STCP frames, and verifies every byte.
