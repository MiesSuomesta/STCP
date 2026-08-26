ZRXSEQ clean Zephyr-only diagnostic overlay.

Built from the stcp_v2_rx.c supplied after resetting to the GitHub foundation.
Old RXSTAT/RXPROBE/RXDIAG instrumentation is removed.
Only the first 12 successful SOCK_STREAM recv() calls and their
stcp_rust_carrier_receive() return values are logged.

No kernel/module changes.
No shared Rust changes.
No struct/layout changes.
No protocol/lifecycle behavior changes.
