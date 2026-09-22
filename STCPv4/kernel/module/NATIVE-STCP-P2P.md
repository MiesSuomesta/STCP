# Native STCP_P2P core – phase 1

This change adds the first native P2P protocol code to the shared Rust core.
It intentionally does **not** claim libp2p compatibility yet.

Implemented now:

- shared `no_std` P2P module
- libp2p multistream-select protocol IDs
- unsigned-varint framing used by multistream-select
- incremental frame decoder
- explicit P2P protocol stage machine
- C ABI version/status hooks
- core selftest

Compatibility boundary:

- STCP remains the byte-stream transport.
- Noise is NOT removed or substituted by STCP crypto.
- `/noise` stays the security protocol negotiated above STCP.
- `/yamux/1.0.0` stays the stream multiplexer.
- `/ipfs/ping/1.0.0` is the first behaviour protocol target.

`stcp_p2p_core_ready()` remains 0 until the exact libp2p Noise XX and Yamux
state machines are implemented and checked against the rust-libp2p golden
reference.  This prevents the Zephyr benchmark from reporting a false PASS.

New ABI:

    stcp_p2p_core_abi_version() -> 1
    stcp_p2p_core_selftest()
    stcp_p2p_core_ready() -> 0 for phase 1
    stcp_p2p_core_stage_name(stage)

Next phase is the exact libp2p Noise XX handshake codec and transcript/identity
binding, followed by Yamux and ping stream negotiation.
