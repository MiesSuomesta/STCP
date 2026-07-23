# Performance benchmark v11

This release removes the Nagle-disabling socket option completely from both
the Zephyr modem client and the Python echo server.

Changes from v10:

- removed the unavailable Zephyr TCP header
- removed the related Kconfig option
- removed the related modem-side socket option
- removed the related server-side socket option
- retained nonblocking burst I/O, progress reporting and socket I/O statistics
- retained best-effort SO_SNDBUF/SO_RCVBUF tuning

Use a clean build directory after updating the source tree.
