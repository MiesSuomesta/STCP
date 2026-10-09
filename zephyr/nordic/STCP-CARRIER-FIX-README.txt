STCP Zephyr carrier short-write fix + diagnostics
=================================================
Source: stcp-09102026_121138.zip, module-v2/ subtree.

INSTALL (from /srv/stcp-project/zephyr-stcp/stcp):
  unzip -o /path/to/stcp-carrier-short-write-fix-overlay.zip

Changes:
- Rust carrier::transmit retries short writes for TCP stream carriers.
- UDP datagrams remain atomic; no retries for short datagrams.
- Zephyr C carrier logs short/failed sends and snapshots errno before logging.
- No EAGAIN retry: errors remain visible rather than potentially spinning.

IMPORTANT: Overlay includes full files from supplied archive. Check git diff before building.
  git diff -- module-v2/rust/src/carrier.rs module-v2/src/stcp_rust_platform.c

Test: rebuild/flash, run stcp bench upload, grep STCP_CARRIER_TX and STCP_SOCKET_SEND_FAIL.
Not compiled or device-tested here.
