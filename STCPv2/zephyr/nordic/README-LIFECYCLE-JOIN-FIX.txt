STCPv2 Zephyr lifecycle teardown fix - 2026-08-26

Problem:
  stcp_v2_rx_stop() waited on sock->event for RX-thread termination.
  sock->event is also used by handshake/data signalling, so a stale token can
  make teardown continue while rx_thread is still alive. rust_ctx/carrier can
  then be released underneath the RX thread, corrupting the next connection.

Fix:
  - set rx_stop
  - shutdown native socket to wake blocking recv()
  - wait for rx_thread with k_thread_join(..., 1s)
  - abort + join as deterministic fallback
  - clear stale event semaphore after the thread is gone

Scope:
  module-v2/src/stcp_v2_rx.c only.
  No shared-core/wire-protocol changes.
