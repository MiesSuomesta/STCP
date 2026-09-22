STCPv2 silent crypto-stage memory diagnostic
============================================

Based on the uploaded current kernel/module Rust sources and the current
Zephyr silent-X25519/no-wait-tick diagnostic baseline.

No printk/LOG calls are added to the RX -> Rust -> crypto -> X25519 path.
The path only performs relaxed atomic stores to a Rust AtomicUsize.

Stages:
  10  stcp_rust_carrier_receive_from entered
  20  PublicKey payload parsed/copied
  30  derive_session_keys entered
  40  immediately before stcp_kernel_x25519_shared FFI
  50  C stcp_kernel_x25519_shared entered
  60  immediately before stcp_x25519_soft
  70  stcp_x25519_soft returned
  80  C shared-secret success path returning
  90  Rust returned from stcp_kernel_x25519_shared
 100  derive_session_keys completed successfully
 110  queue_to_context / carrier receive returned

The connect waiter prints the current stage ONLY on its normal 60 s timeout:
  CONNDIAG TIMEOUT ... crypto_stage=<N>

Interpretation:
  stage=60 strongly isolates the block inside stcp_x25519_soft().
  stage=40 means Rust->C call did not reach the first C stage store.
  stage=50 means C entered but did not reach the soft-call boundary.
  stage>=70 means X25519 itself returned.

Apply as a project-root overlay.  Build/flash Zephyr and rerun CoAP.
