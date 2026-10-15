# Robot Framework test setup

These are Robot Framework tests (the Python `robotframework` project), not
Google Test. Four independent suites: `python.robot`, `rust.robot`,
`golang.robot`, `dart.robot`; ten cases per suite, forty total.

## Target setup

Use Linux with a compatible STCP module installed and loaded. Loopback STCP
must work. Tests do not install/reload modules or require sudo. Install each
language's compiler when running its suite: Rust 1.63+, Go 1.20+, Dart 3+;
Python 3.10+ is required for this test setup. Dart requires glibc Linux.
No kernel-module source or commercial SDK crates are needed.

From the package root:

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install -r tests/requirements.txt
bash tests/run.sh --all
# Or one language:
bash tests/run.sh --python
bash tests/run.sh --rust
bash tests/run.sh --golang
bash tests/run.sh --dart
```

The selector is mandatory. `--all` runs all four suites sequentially even
when a previous suite fails, and exits nonzero if any suite fails. Suite setup
builds native binaries once; missing tools/module and failed builds fail the
suite rather than skipping it. Each test starts a fresh language-specific
server and stops only its own process. No broad process killing is used.

Each suite checks its own client against its own server plus an independent
Python socket ABI probe and intentionally faulty STCP server fixtures:

1. ASCII echo, including reported byte count.
2. UTF-8 / Unicode echo.
3. Long message, exercising multiple 4096-byte client exchanges.
4. Server echo of all byte values, 256 KiB total.
5. Ten sequential client processes/connections.
6. Disconnect before sending data, followed by a working echo.
7. Invalid port/IP and empty client message rejected.
8. Client rejects a corrupted echo.
9. Client rejects premature EOF.
10. Client accepts a response sent one byte at a time.

All integration network sockets use AF_STCP=45, SOCK_STREAM, protocol=253.
There is no TCP fallback. Tests cover local interoperability, not network
performance, parallel clients (servers are deliberately serial), or a full
cross-language matrix. Run on each target architecture, e.g. devaus and raspi.
Do not run the same suite concurrently using the same port.

## Results and manual Robot invocation

Results are archived under `test-results/<UTC timestamp>-<pid>/<language>/`:
`report.html`, `log.html`, `output.xml`, `server.log`, and native build logs.
A failed setup can mean no server/build log was created yet; see `log.html`.
Client failure output is included in Robot assertion messages.

Use a separate output directory when invoking Robot manually. Override the
suite's port-range lower bound (19701 Python, 19702 Rust, 19703 Go, 19704 Dart) if needed:

```sh
python -m robot --variable PORT:19801 --outputdir test-results/manual-python tests/robot/python.robot
# Validate all suite syntax without module/toolchains:
python -m robot --dryrun --outputdir test-results/dryrun tests/robot
# Independent tests of the Python echo loop, no STCP module required:
python -m unittest discover -s tests/unit -v
```

Integration test timeout is 45 seconds per case, subprocess client deadline
30 seconds, socket I/O timeout 5 seconds and native build deadline 300 seconds.
Clients themselves remain minimal blocking examples without an application
deadline. These tests do not claim to verify encryption/compression on the wire.

## Accept/readiness fix

Python bypasses CPython's unknown-family accept address handling with libc
`accept(fd, NULL, NULL)`; its returned descriptor is wrapped with explicit
IPv4 metadata for Python I/O. The underlying descriptor remains STCP.
All servers retry accept after EINTR, ECONNABORTED and ECONNRESET.
Readiness checks observe the flushed listening banner instead of opening and
immediately aborting a connection. Test probes set Python's nonblocking timeout
mode only after the raw libc connect, avoiding an unhandled EINPROGRESS.
The same raw accept helper is used for intentionally faulty STCP test fixtures.

## Listener address and port isolation fix

Fault fixtures bind an explicit random port and never call listener
getsockname(): this module returns ENOTCONN for that operation. Each test
chooses a new server port within 20000 ports from PORT (capped at 65535),
retrying address-in-use collisions. This avoids reusing ports still held by
recent connections. Rust uses libc accept with null address arguments, then
wraps the accepted descriptor for stream I/O, without requesting peer metadata.

## Aborted handshakes

The module's accept path can propagate a failed child handshake as EPIPE,
ECONNABORTED, ECONNRESET, ENOTCONN, ETIMEDOUT or ECONNREFUSED. Servers retry
accept for these connection-level errors and EINTR without closing the listener.
Descriptor/resource/programming errors still terminate the server.
Clients retain strict failures: the disconnect test must establish and verify
a new echo without retrying the client. Failed client assertions also include
server exit status and its current log.
