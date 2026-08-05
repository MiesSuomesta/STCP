# STCP-TCP wildcard listener fix

A listener bound to `0.0.0.0:PORT` is now recognized as local when a client connects to the host's concrete IPv4 address on the same port. This keeps Raspberry->Raspberry on the local paired fast path instead of misclassifying it as an external peer.
