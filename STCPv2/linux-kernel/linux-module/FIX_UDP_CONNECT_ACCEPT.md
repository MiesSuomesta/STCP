# UDP connect/accept fix

The UDP client previously reused the local listener registry used by the
in-kernel paired backend. This enqueued a server child before any UDP datagram
had arrived. The child therefore had no UDP peer address and `accept()` failed
with `-ENOTCONN`. If no local registry entry was found, the client returned
`-ECONNREFUSED` even though its UDP carrier had connected successfully.

The corrected UDP flow is:

1. `connect()` configures only the client context and allocates a non-zero
   connection ID.
2. `start_handshake()` sends the first PublicKey datagram through the connected
   UDP carrier.
3. The listener receive thread decodes the connection ID and source address.
4. A server child is created and queued only for that first PublicKey datagram.
5. `accept()` attaches a child carrier that shares the listener socket and has
   the real peer IP/port.

The non-UDP paired backend keeps its existing listener-registry behavior.
