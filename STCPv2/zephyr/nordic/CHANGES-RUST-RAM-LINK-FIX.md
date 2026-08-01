# Rust STCP Zephyr RAM and link fix

## Fixed

- Removed the unavailable `net_sprint_ipv4_addr()` dependency. IPv4 logging
  now formats the address locally from `sin_addr.s_addr`.
- Limited the Ethernet benchmark build to `CONFIG_STCP_MAX_SOCKETS=2`.
- Reduced each Rust carrier RX stack from 4096 to 3072 bytes.
- Switched the Rust static library release profile to size optimization, fat
  LTO, stripped symbols, and no debug info.

## Why RAM overflowed

`struct stcp_ctx` embeds `K_KERNEL_STACK_MEMBER(rust_rx_stack, ...)`. With the
default eight sockets and a 4096-byte stack, the socket pool alone reserved
about 32 KiB for RX stacks. The client benchmark needs one active socket and
one spare, so two entries save over 24 KiB of static RAM, enough to cover the
reported 20080-byte overflow.

## Build

```bash
rm -rf stcp-mqtt/build-ethernet
cd stcp-mqtt
bash scripts/build-ethernet.sh
```

After a successful link, inspect RAM usage in the Zephyr memory report before
increasing `CONFIG_STCP_MAX_SOCKETS`.
