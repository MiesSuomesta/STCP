STCP Zephyr MSG_DONTWAIT overlay
==================================

Change:
- zephyr/nordic/module-v2/src/stcp_v2_socket.c
- Preserve ZSOCK_MSG_DONTWAIT through recvfrom_socket().
- When the Rust RX core returns -EAGAIN and MSG_DONTWAIT is set,
  return POSIX-style -1 with errno=EAGAIN immediately.
- Blocking read() behavior is unchanged.

Apply from the STCP repository root:
  unzip -o stcp-v2-zephyr-msg-dontwait-overlay.zip

This overlay intentionally changes only stcp_v2_socket.c.
