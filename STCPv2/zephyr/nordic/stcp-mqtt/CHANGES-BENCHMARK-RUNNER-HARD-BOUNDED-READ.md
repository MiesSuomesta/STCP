# Benchmark runner hard-bounded CDC read

- Replaces pyserial `Serial.read()` with explicit `select.select(..., 0.10)` and `os.read()`.
- The serial receive path now has a guaranteed 100 ms upper bound.
- Readiness probes, Ctrl-C and case timeouts can no longer be blocked by pyserial's internal read loop.
- Keeps `/dev/ttyACM0`, DTR handling and CR shell commands unchanged.
