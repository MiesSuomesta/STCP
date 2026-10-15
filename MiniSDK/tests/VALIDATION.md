# Packaging validation

- Robot Framework 7.5: all four suites, 40 cases, passed dry-run keyword/syntax validation.
- Python unittest: 9/9 passed (ASCII, Unicode, binary 256 KiB, fragmented echo, mismatch, early EOF, raw libc accept and retry after ECONNRESET/ECONNABORTED/EINTR).
- Python test library syntax compilation and bash runner syntax checks passed.
- Missing STCP module preflight produced the expected explicit failure.

Dry-run does not execute STCP operations or language builds. Native Rust/Go/Dart
builds and all 40 actual integration cases remain unverified until run on the
target with a compatible STCP module and toolchains.

The fault-fixture regression test exercises all three modes (corruption, early
EOF and fragmented success) using isolated TCP test sockets and forbids
listener getsockname. Robot integration continues to require real STCP.
Target results before this fix: all four languages passed ASCII, Unicode,
long message, 256 KiB binary and repeated-connection cases. Full target rerun
is still needed after the listener-address and port-isolation changes.

Latest supplied target run: Python, Go and Dart 10/10 each; Rust 9/10,
with the immediate-disconnect case reporting connection refused from its
follow-up client. The next update broadens accept recovery to connection-level
child handshake errors. This fix is not yet verified on the target; 9 local
unit tests and 40-case Robot dry-run still pass.
