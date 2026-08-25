STCPv2 Zephyr first-boot RX diagnostic overlay
================================================

Basis
-----
Built directly from stcp-25082026_084116.zip.

Purpose
-------
This overlay DOES NOT change connect/RX ordering or add sleeps/barriers.
It only instruments the existing first-boot path so we can see exactly
whether the native TCP response reaches zsock_recv(), whether the RX thread
exits, and what the shared Rust core returns for the received handshake frame.

Changed files
-------------
module-v2/src/stcp_v2_rx.c
module-v2/src/stcp_v2_socket.c

New log prefixes
----------------
RXDIAG   RX thread lifecycle, native recv result, errno, first RX hexdumps,
         and carrier_receive return codes.
CONNDIAG native TCP connect, Rust connect, RX start, handshake start,
         connect wait state and timeout.

Test
----
1. Apply after rolling back the previous rx_ready overlay.
2. Clean build + flash.
3. Capture the FIRST modem/board boot from power-on.
4. If it fails, reboot once and capture the SECOND boot too.
5. Send both logs back.

Important
---------
This is diagnostic instrumentation only. It intentionally leaves the
original behavior unchanged so the first-vs-second boot difference remains
observable.
