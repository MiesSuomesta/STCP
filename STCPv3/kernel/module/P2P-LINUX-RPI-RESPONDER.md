# STCPv2 native P2P: Linux/Raspberry Pi responder support

This overlay adds the missing Noise XX responder half to the canonical shared
Rust core under `kernel/module/rust`.  Because Linux and Raspberry Pi both link
this same core, the implementation is shared instead of forked per platform.

New C ABI exports:

- `stcp_p2p_noise_listener_new()`
- `stcp_p2p_noise_listener_free()`
- `stcp_p2p_noise_listener_message1()` -> consumes XX message 1 and returns message 2
- `stcp_p2p_noise_listener_message3()` -> consumes XX message 3
- `stcp_p2p_noise_listener_complete()`

The existing dialer ABI remains unchanged.

The Noise selftest now runs a complete in-kernel XX exchange between an
initiator and responder and checks that the directional split keys match:
initiator TX == responder RX and initiator RX == responder TX.  Module load
runs the complete P2P core selftest after the existing STCP crypto selftest, so
both Linux and Raspberry Pi reject the module early if their P2P crypto path is
broken.

Expected successful load log includes:

```
stcp: directional crypto selftest passed
stcp: P2P shared core ABI=2 Noise XX initiator/responder selftest passed
```

`stcp_p2p_core_ready()` intentionally remains 0.  Noise XX now has both roles,
but the native P2P transport must not advertise full libp2p readiness before
the exact Yamux and ping stream state machines are complete.
