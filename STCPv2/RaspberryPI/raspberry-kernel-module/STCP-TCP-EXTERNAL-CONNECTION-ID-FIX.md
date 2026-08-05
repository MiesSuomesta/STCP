# Raspberry STCP-TCP external connection-id fix

The external TCP server child now adopts the Linux client's non-zero STCP
connection_id from the first PublicKey frame before starting the server side
handshake. The RX wakeup includes the adoption event and accept waits for the
id, preventing PublicKey frames with mismatched connection ids and the
Linux-to-Raspberry handshake timeout.
