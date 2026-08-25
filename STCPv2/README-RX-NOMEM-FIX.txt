STCPv2 Zephyr RX -ENOMEM overlay — corrected repository paths
================================================================

Extract this ZIP directly in the STCPv2 repository root:

    cd ~/STCP/STCPv2
    unzip -o stcp-zephyr-rx-nomem-fix-overlay-correct-paths.zip

Files:
    kernel/module/rust/src/state.rs
    kernel/module/rust/src/session.rs
    kernel/module/rust/src/byte_queue.rs
    zephyr/nordic/module-v2/src/stcp_v2_rx.c

The OLD-FOR-REFERENCE-ONLY tree is intentionally untouched.

This is the same RX -ENOMEM/client-first fix as the previous overlay; only
the repository paths have been corrected.
