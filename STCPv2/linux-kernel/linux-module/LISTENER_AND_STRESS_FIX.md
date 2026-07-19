# Listener release and Python stress-suite fixes

## Kernel/Rust listener registry

Listener entries are now removed by `StcpContext` identity on every release,
regardless of the current socket state. This covers:

- normal listener close
- setup failure after listener registration
- signal/timeout teardown
- state already changed to `Closed`

Neither release implementation calls `shutdown()` or sends a Close frame.

## Python suite

The suite now uses separate ports:

- throughput: 7777
- mixed: 7778
- churn: 7779

This keeps the full suite diagnosable even if one preceding test exposes a
listener teardown regression.

The runner also uses one shared two-second shutdown deadline rather than
waiting the full socket timeout once per client thread.
