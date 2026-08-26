STCPv2 Zephyr LAST_NOMEM_STAGE overlay
======================================

Extract in the STCPv2 repository root:

    cd ~/STCP/STCPv2
    unzip -o stcp-zephyr-last-nomem-stage-overlay.zip

Carries forward the current client-first handshake, RX-memory, direct frame
processing and NOMEM trace changes.

Changed files:
  kernel/module/rust/src/state.rs
  kernel/module/rust/src/session.rs
  kernel/module/rust/src/byte_queue.rs
  kernel/module/rust/src/carrier.rs
  zephyr/nordic/module-v2/src/stcp_v2_rx.c

Behavior:
  - LAST_NOMEM_STAGE is reset at the start of each carrier_receive_from().
  - Specific stages:
      9001 = incoming wire ByteQueue push
      9002 = wire frame payload extraction
      9003 = out-of-order frame queue reserve
      9004 = decrypted plaintext app queue
  - Fallback stages:
      9091 = fill_application_buffer returned NoMem with no specific stage
      9090 = carrier_receive returned NoMem with no specific stage
  - Zephyr prints only on the actual error:
      carrier_receive rc=-12 nomem_stage=XXXX

The FFI getter is exported directly from carrier.rs:
    stcp_rust_last_nomem_stage() -> u32

No sleeps, hexdumps or per-frame diagnostic log flood are added.
