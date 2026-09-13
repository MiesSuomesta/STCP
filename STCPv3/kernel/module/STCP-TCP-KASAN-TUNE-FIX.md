# Linux STCP TCP socket tuning KASAN fix

The TCP carrier is tuned once immediately after socket creation. The redundant
second tcp_sock_set_nodelay() after kernel_connect() was removed because KASAN
reported slab-use-after-free in that call during an aborted/racing connection.
Accepted sockets are still tuned after kernel_accept().
