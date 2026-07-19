# V6.5 asynchronous connect / handshake fix

## Symptom

The stress suite reported zero connections, zero traffic and high Python CPU.
V6.3 had made `connect()` wait until the STCP crypto state became `Ready`.
That wait happens before the server application has necessarily completed
`accept()` and attached the accepted carrier/owner, creating a setup stall.

## Fix

* `stcp_connect()` now returns after the carrier connection is established,
  the Rust connection is queued and the public-key handshake is started.
* The first `send()` remains gated by Rust and returns `-EAGAIN` until Ready.
* `stcp_poll()` advertises `EPOLLOUT` only after the handshake is Ready.
* The Python poll helper now treats `POLLERR`, `POLLHUP` and unexpected events
  as errors instead of spinning.

This retains the V6.4 TCP stream resynchronisation and earlier lifecycle fixes.
