# STCP-TCP connect fixes

- Preserve the local in-kernel fast path when a listener is bound to either the exact target address or `0.0.0.0` on the target port.
- If no local listener exists after the TCP carrier has connected, continue through the external/cross-host handshake path instead of returning `ECONNREFUSED`.

This addresses Linux->Linux and Linux->Raspberry failures seen in the 13/16 Robot baseline.
