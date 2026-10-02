# STCP cross-host TCP accept / reset fix — 2026-08-22

Postmortem `stcp-postmortem-20260822-174140.zip` isolated the cross-host failure to the TCP accept/connect handshake.

## Root cause fixed

`stcp_carrier_get_endpoints()` treated every non-zero return from `kernel_getsockname()` / `kernel_getpeername()` as an error. On the tested kernels these helpers return a **positive sockaddr length** on success (typically 16 for IPv4).

That produced this failure sequence on the Raspberry server:

1. underlying TCP `kernel_accept()` succeeded;
2. `stcp_carrier_get_endpoints()` returned +16;
3. STCP destroyed the just-accepted carrier and returned +16 from its protocol `accept` callback;
4. the generic socket accept path treats only negative values as errors, so userspace received a new fd anyway;
5. that fd had no STCP `newsock->sk`, and the first recv returned `EINVAL`;
6. the Linux client observed `ECONNRESET` immediately after sending its PublicKey frame.

The fix accepts non-negative getname results and normalizes the endpoint helper to 0 on success.

## Additional hardening

* TCP carriers now record their first terminal RX/TX error.
* terminal RX wakes the owning socket so connect/accept waits can fail immediately instead of sleeping until the 5 s handshake timeout.
* connect and external-accept waits include the carrier terminal-error condition.
* release skips sending the protocol CLOSE frame after an already-terminal TCP reset/close, avoiding a second send into a dead transport.
* TX06/RX05 and A01..A17 checkpoints cover send-return and TCP accept setup.

## Expected next-run signature

For Linux -> Raspberry TCP, a healthy server side should now show roughly:

A02 kernel-accept-enter
A03 kernel-accept-exit ret=0
A04 rust-accept ret=-EAGAIN (cross-host case)
A05 endpoints ret=0 ...
A06 external-child-create ret=0
A08 child-sock-alloc-exit ... newsock_sk=<non-null>
A09 carrier-attached
A11 rx-start-exit ret=0
A13 connection-id-wait-exit ... cid=<non-zero>
A15 handshake-start-exit ret=0
A17 handshake-wait-exit ... connected=1 terminal=0
stcp: accept complete ... external=1

The client should no longer see RX04 ret=-104 immediately after its PublicKey frame.
