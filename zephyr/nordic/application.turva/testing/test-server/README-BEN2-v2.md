# BEN2 server metrics — protocol version 2

This package updates the supplied stcp_server.rs, bench_protocol.rs and retains
stcp_aes_gcm_userspace.rs unchanged. Replace those three files in the existing
server source directory, then use the existing Cargo project/build command.
No new dependency or STCP wire/encryption change is introduced.

This is a breaking BEN2 change: client and server must both use request version 2.
It is a metrics extension to the current client-initiated tests; server-driven
scheduling is not implemented in this package.

## Server

Server collects RX/TX application bytes, receive/send callback counts, RX/TX
windows and whole transaction duration. It computes per-direction and aggregate
MiB/s and persists one JSON object per completed test (or INCOMPLETE on teardown).
Client metrics are stored alongside server metrics. Protocol validation or early
I/O failure yields INCOMPLETE, not a false PASS. Nonzero client errors yield FAIL.

    BEN2_RESULTS=/path/to/results/zephyr-ben2.jsonl ./stcp-server

Default output: benchmark-results/zephyr-ben2.jsonl. Each result also appears on
stdout prefixed BEN2_RESULT. Save failures are reported to stderr. Detailed trace
is off by default; enable with STCP_TRACE=1. Disabled trace skips formatting and
hex previews as well as output. No debug output per data chunk in BEN2.

Receive callbacks are decrypted application deliveries, not recv syscalls.
Send callbacks are submissions to STCP, not proof of delivery onto LTE.
RX window begins at first received data callback, so use client TX window for
sender-side rate and server elapsed for whole-test comparisons. ACK wait is
included in elapsed, not the completed per-direction windows. CPU, IRQ, actual
carrier wire bytes and RTT are not guessed from these metrics.

## Zephyr integration required

The supplied C header is a fixed-size serializer, not the current application.
The Zephyr application source was not supplied, so its send/recv hooks, shell
commands and firmware build have NOT been changed or tested here.

In the existing benchmark implementation:
1. Include zephyr/ben2_metrics.h and encode requests with ben2_request_encode.
2. Collect raw fields into struct ben2_metrics. Count actual successful data bytes
   and all data send/recv calls, partial calls included; exclude BEN2 controls.
   eagain is retryable EAGAIN/EWOULDBLOCK; errors counts terminal errors.
3. Measure TX/RX windows independently with a monotonic clock; elapsed covers
   data start through both directions complete. Connect is separate. Convert
   ticks to microseconds; no floating-point rate calculations on Zephyr.
4. Set carrier to LTE=2 or Ethernet=1 from the actual transport configuration.
   Unavailable values remain zero. Set stack values from existing instrumentation.
5. After upload data, send the 112-byte MTR2 report, then read the 20-byte reply.
   For download/full, send the existing 20-byte ACK followed by MTR2, then read
   the existing 20-byte reply. Report/ACK are sent outside the measurement window.
6. For failed data transfer before the declared byte total, close the connection;
   do not insert a metrics record into the unfinished data stream.

Keep current full-duplex progression: receive and send concurrently/interleaved.
Do not replace it with send-all then receive-all. No report-time floating point
formatting is required on the device.

## Wire contract (all integers unsigned big-endian)

Request remains 20 bytes: BEN2 magic, version=2, mode=1/2/3, chunk, total (five u32).
Payload patterns remain unchanged: upload (offset*31+0x17)&255, download salt=0x53.
ACK and server reply remain five u32: BEN2 magic, mode, status, received, sent,
with received/sent relative to the party emitting that record.

MTR2 report: exactly 112 bytes, header four u32:
magic=0x4d545232, schema=1, record_bytes=112, carrier=0/1/2.
Then twelve u64, in this order:
tx_bytes, rx_bytes, tx_us, rx_us, send_calls, recv_calls, eagain, errors,
stack_used_bytes, stack_free_bytes, connect_us, elapsed_us.

Server validates client bytes against the declared test direction and total;
reported time values are limited to 24h. Unavailable/zero durations produce JSON
null rates, not infinity. Times are reported observations, not externally proven.

## Validation

Run tests/run-tests.sh with cc, Python 3 and rustc installed.
Executed here: C serializer built with -Wall -Wextra -Werror; its exact binary
layout independently decoded and verified with Python. PASS.
Not executed here: Rust tests/full server build (rustc absent), Zephyr build,
STCP encrypted interoperability and LTE runs.

Rust tests cover fragmented upload/MTR2, download, full ACK arriving before the
last pump, nonzero client errors, old-version rejection, malformed/oversize
reports and echo behavior. Test output is also checked as valid persisted JSONL.
