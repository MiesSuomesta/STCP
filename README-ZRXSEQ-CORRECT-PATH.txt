STCP Zephyr ZRXSEQ diagnostic overlay

Extract from the STCP repository root, e.g. ~/STCP:
  unzip -o stcp-zephyr-zrxseq-correct-path-overlay-20260827.zip

Writes exactly:
  STCPv2/zephyr/nordic/module-v2/src/stcp_v2_rx.c

Diagnostics:
  ZRXSEQ THREAD START ...
  ZRXSEQ RECV seq=N ... n=...
  ZRXSEQ CORE seq=N ... rc=... connected=...

Only the first 12 successful SOCK_STREAM recv() calls are logged.

No STCPv2/kernel files.
No shared Rust files.
No struct/layout changes.
No protocol behavior changes.
