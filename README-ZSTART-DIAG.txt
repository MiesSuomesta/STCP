STCP Zephyr ZSTART diagnostic overlay
====================================

Extract from the STCP repository root:
  cd ~/STCP
  unzip -o stcp-zephyr-zstart-diag-overlay-20260827.zip

Writes only:
  STCPv2/zephyr/nordic/module-v2/src/stcp_v2_socket.c
  STCPv2/zephyr/nordic/module-v2/src/stcp_v2_rx.c

Markers:
  ZSTART NATIVE_CONNECT ENTER/RETURN
  ZSTART RUST_CONNECT ENTER/RETURN
  ZSTART RX_START CALL ENTER/RETURN
  ZSTART RXSTART ENTER
  ZSTART RXSTART BEFORE_THREAD_CREATE
  ZSTART RXSTART THREAD_CREATED
  ZSTART RX_THREAD RUNNING
  ZSTART RXSTART READY_WAIT ENTER/RETURN

Diagnostic only:
- no STCPv2/kernel files
- no shared Rust files
- no struct/layout changes
- no protocol behavior changes
