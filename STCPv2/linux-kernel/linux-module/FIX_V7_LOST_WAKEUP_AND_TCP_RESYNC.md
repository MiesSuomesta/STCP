# V7: lost wakeup and TCP stream resynchronisation fix

- Removed waitqueue_active() as a correctness guard before accept/recv wakeups.
- Wake all waiters directly, preventing the wait_event lost-wakeup race.
- Added byte-wise STCP header resynchronisation to ready-state data and control parsing.
- Corrected the throughput suite label to the actual 25-second duration.

Validation: Python stress scripts pass py_compile. Full kernel build was unavailable in this container because it lacks the matching kernel build tree and Cargo.
