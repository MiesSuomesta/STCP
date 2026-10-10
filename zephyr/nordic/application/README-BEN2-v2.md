# Zephyr BEN2 v2 overlay — 2026-10-10

Based on application-10102026_110823.zip. Changes are limited to src/:
- echo_benchmark.c / echo_benchmark.h
- bench_shell.c
- new ben2_metrics.h

## Install

Extract into the application directory containing CMakeLists.txt, Kconfig and src/.
Take a backup of the existing src/echo_benchmark.c, src/echo_benchmark.h and
src/bench_shell.c first. The supplied original upload remains a rollback copy.

    cd /srv/stcp-project/STCP/version-to-use/zephyr/nordic/application
    unzip -o /path/to/BEN2-v2-zephyr-overlay.zip
    ./scripts/build-lte.sh
    ./scripts/flash-lte.sh

The path above matches the supplied build-script layout; use your actual active
application path if different. Ethernet: use scripts/build-ethernet.sh and
scripts/flash-ethernet.sh instead. Build scripts/configuration are not changed.

Use the matching BEN2-v2-server-overlay.zip server. Both peers need request
version 2; old BEN2 v1 servers are incompatible. No change to STCP itself.

    stcp config host 80.186.162.127
    stcp config port 19000
    stcp config transport stcp
    stcp bench all

Server output: BEN2_RESULT JSON on stdout and BEN2_RESULTS JSONL file
(default benchmark-results/zephyr-ben2.jsonl).

## What changed

Zephyr measures raw data bytes, separate TX/RX microsecond windows, socket-call
counts, EAGAIN, errors, connect time, data elapsed time and optional thread stack
usage. Carrier is selected from CONFIG_STCP_TRANSPORT_ETHERNET: Ethernet=1,
otherwise LTE=2. No rate calculations or transfer-time progress output remain
in the benchmark. Local shell JSON schema 3 contains raw counts/status only.
The report command remains available but no longer enables data-path logging.

Upload sends MTR2 immediately after data. Download/full sends its existing ACK,
then MTR2. All modes then receive the server reply. Controls are excluded from
I/O counters and data duration. No extra report is inserted into an incomplete
data stream: a failed transfer closes, producing INCOMPLETE on the server.

Full retains concurrent send/receive. TX/RX counters are separate and collected
after the sender thread has joined, avoiding concurrent updates to shared
counters and allowing safe reuse of the sender thread/stack in later tests.

Existing 1024-byte STCP send slices, connection behavior, timeouts, modem/PDN
setup and payload patterns are retained. The shell still triggers tests; this
package adds measurement reporting, not server-driven test scheduling.

MTR2: 112 bytes, four big-endian u32 values (magic 0x4d545232, schema 1, size 112,
carrier), then 12 big-endian u64 values: tx_bytes, rx_bytes, tx_us, rx_us,
send_calls, recv_calls, eagain, errors, stack_used_bytes, stack_free_bytes,
connect_us, elapsed_us. Zero means unavailable for stack or timing fields.

## Validation

PASS: modified benchmark C compiled with -Wall -Wextra -Werror against a host
API shim. Shell C syntax checked for LTE and Ethernet reporting branches.
PASS: 18 host TCP checks: upload/download/full x 4096/65537/1048576 bytes x two
carrier branches. Independent Python server validates request version, complete
payload patterns, ACK ordering, exact MTR2 layout, byte counts and I/O metrics.

    ./validation/run-host-tests.sh

This is host validation, not an nRF9151 build or radio test. Actual Zephyr/NCS
build, thread-stack instrumentation branch, modem PDN binding and encrypted STCP
interoperability must still be verified on your device. Rust server compilation
was unavailable in the creation environment.
