# STCPv2 BSD socket kernel skeleton

This tree converts the uploaded working application design into a Linux kernel
BSD socket architecture:

`socket(AF_STCP, SOCK_STREAM, 253)` → `net_proto_family.create` → `proto_ops`
→ C/Rust FFI → Rust state/packet/crypto core.

## Implemented skeleton

- AF_STCP/PF_STCP family registration
- `socket`, `release`, `bind`, `listen`, `accept`, `connect`
- `sendmsg`, `recvmsg`, `shutdown`
- `struct stcp_sock` with Rust context and accept wait queue
- Rust `no_std + alloc` state model matching New→Bound→Listening/Handshake→Ready
- 16-byte STCP v2 header and packet types matching the application
- FFI ABI and errno mapping
- Application source snapshot in `app-reference/`

## Intentionally unfinished platform pieces

The uploaded app uses libc TCP sockets and userspace crates. A kernel module
cannot call libc or `std`. Therefore these must be implemented before real data
works:

1. `rust/src/transport.rs`: carrier TX/RX and frame queues. Current send/recv
   return `-EAGAIN`.
2. `rust/src/crypto.rs`: Linux Crypto API backend for X25519 and
   ChaCha20-Poly1305. Current encrypt/decrypt is a clearly marked placeholder
   and must not be used as security.
3. Network ingress: packet reception must locate the matching context, perform
   symmetric handshake, and queue accepted children; then wake `accept_wq`.
4. Rust static library aggregation into one relocatable `rust_core.o` must be
   adapted to the exact kernel/Rust toolchain.
5. `proto_ops` signatures can vary between kernel versions. This skeleton uses
   the newer `struct proto_accept_arg` form seen in the STCPv2 work.

## Build direction

```sh
make rust
make module
sudo insmod stcp.ko
cc tests/test_stcp.c -o tests/test_stcp
./tests/test_stcp
```

Use the same LLVM/Rust versions as the target kernel. Build first with KASAN,
lockdep and netconsole enabled.
