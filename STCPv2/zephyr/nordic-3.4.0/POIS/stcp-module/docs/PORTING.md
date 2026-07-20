# Linux -> Zephyr C-side port mapping

This package ports the Linux module's C-facing socket and carrier layer only.
No Rust source has been copied or modified.

## Preserved ABI

- `AF_STCP = 45`
- `STCP_PROTO_TCP = 253`
- `STCP_PROTO_UDP = 254`
- `socket(AF_STCP, SOCK_STREAM, protocol)`
- `bind()` and `connect()` use `struct sockaddr_in`, matching the Linux module.

## Mapping

| Linux module | Zephyr port |
|---|---|
| `net_proto_family.create` | `NET_SOCKET_OFFLOAD_REGISTER` create callback |
| `proto_ops` | `socket_op_vtable` |
| `struct stcp_sock` | `struct stcp_socket_ctx` static pool |
| kernel TCP/UDP sockets | hidden Zephyr `zsock_socket(AF_INET, ...)` carrier |
| `kernel_sendmsg/recvmsg` | `zsock_send/zsock_recv` |
| `kernel_bind/connect/listen/accept` | matching `zsock_*` calls |
| Linux wait queues/workqueues | intentionally omitted until protocol engine is connected |
| Rust FFI | intentionally omitted; no Rust changes |

## Current behavior

This is a functional C carrier port, not yet the encrypted/reliable STCP protocol.
AF_STCP sockets currently pass application bytes directly over their selected
TCP or UDP carrier. This verifies the complete BSD socket offload and carrier
path on Zephyr before inserting the existing Rust protocol engine.

The next integration point is `src/stcp_core.c`: replace its direct carrier
send/recv calls with the unchanged Rust FFI calls and add Zephyr wakeups/work.
