# Echo v2 EAGAIN fix

The Zephyr client now treats `EAGAIN`/`EWOULDBLOCK` as a temporary modem-offload condition.
It waits with `zsock_poll()` until the socket is readable/writable or the configured total
operation timeout expires. Error logs identify whether header/payload send/receive failed.
