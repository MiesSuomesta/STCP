# V9 RX readiness / busy-loop fix

## Root cause

`session::has_data()` reported readable whenever the raw carrier byte queue was
non-empty.  TCP can deliver an STCP frame in several fragments.  With only a
partial frame queued, `recv()` returned `-EAGAIN`, but the C wait condition
immediately evaluated true again.  This caused a tight retry loop and the
150-200% CPU load seen in the Python stress test.

## Fix

- `has_data()` now parses all complete frames with `fill_application_buffer()`.
- Readability is reported only when application data is ready or EOF is pending.
- Partial TCP frames remain asleep until the carrier appends more bytes and
  wakes `recv_wq`.
- Python poll setup converts a concurrently closed `fd == -1` into a normal
  `EBADF` shutdown path instead of an uncaught `ValueError`.
