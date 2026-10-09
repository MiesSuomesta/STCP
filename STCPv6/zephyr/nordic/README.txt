Diagnostic-only patch for BEN2 Nordic STCP send EIO.
Overlay paths: application/src/echo_benchmark.c and module-v2/src/stcp_socket_offload.c
No protocol or frame-size changes.
Apply overlay from zephyr-stcp/stcp root, or use standalone patch with patch -p1.
