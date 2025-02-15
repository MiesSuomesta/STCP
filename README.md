# STCP
Secure TCP connection protocol with AES security layer.

# STCP packet format

Packet format over standard TCP packet-payload: [ 16/24/32 bytes of AES IV key ] + [ the AES-encrypted payload ]

# STCP Packet handling

Incoming: fetch the IV-vector of 16/24/32 bytes from incoming packet -> use it and predefined AES key to decrypt package, prior to handing the TCP-packet to message handler.

Outgoing: Generate random IV-vector of 16/24/32 bytes and apply to outgoint TCP packet -> use it and predefined AES key to encrypt package, prior to handing the TCP-packet to sending the message.


