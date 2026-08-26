STCPv2 Zephyr RX diagnostics overlay - 2026-08-26

Apply from zephyr/nordic root (the directory containing module-v2/):

  unzip -o stcp-zephyr-rxdiag-overlay-20260826.zip

Changes:
- restores RX startup ready barrier (rx_ready semaphore)
- logs RX thread startup and exit
- logs first 8 recv/recvfrom attempts, then every 1000th empty retry
- logs every successful received frame and first 8 bytes
- logs shared-core carrier_receive enter/return and connected state
- preserves errno immediately after recv
- no Rust core or protocol changes

Useful markers:
  RXDIAG THREAD READY
  RXDIAG START READY
  RXDIAG recv ENTER
  RXDIAG recv RETURN
  RXDIAG DATA
  RXDIAG CORE ENTER
  RXDIAG CORE RETURN
  RXDIAG EOF
  RXDIAG THREAD EXIT
