STCP_P2P application benchmark overlay
======================================

Purpose
-------
Adds a native STCP_P2P benchmark command tree to the Zephyr application
without inventing a non-compatible P2P wire protocol.

Current state
-------------
The shell/UI and benchmark worker are present now.  The backend contract is
implemented as weak symbols returning -ENOSYS until the native stcp-p2p-core
Zephyr adapter is linked.

The existing rust-libp2p + Noise + Yamux over STCP userspace program remains
the golden wire/reference implementation.

Commands
--------
  stcp p2p show
  stcp p2p host <ip|name>
  stcp p2p port <port>
  stcp p2p payload <bytes>
  stcp p2p total <bytes>
  stcp p2p timeout <ms>
  stcp p2p ping [count]
  stcp p2p bench upload
  stcp p2p bench download
  stcp p2p bench full
  stcp p2p bench all

Expected before native backend is linked
----------------------------------------
  stcp p2p show
    P2P backend : not linked

Benchmark commands then fail explicitly with -ENOSYS.  This is intentional:
the application must never claim compatibility before the native protocol
implementation exists.

Backend ABI
-----------
A future Zephyr adapter supplies strong definitions for:
  stcp_p2p_backend_available
  stcp_p2p_backend_ping
  stcp_p2p_backend_upload
  stcp_p2p_backend_download
  stcp_p2p_backend_full

These replace the weak stubs in src/p2p_benchmark.c.
