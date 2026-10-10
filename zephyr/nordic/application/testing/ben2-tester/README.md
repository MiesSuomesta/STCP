# BEN2 Rust combination tester

Standalone Linux Rust 1.85+ Cargo project. Standard library only; no external crates
or STCP core sources. Native STCP tests use the loaded stcp.ko socket API.

```sh
cargo build --release --locked
./target/release/ben2-tester --host 192.168.1.20
```

Each round runs all 12 combinations: TCP/UDP/STCP-TCP/STCP-UDP × UL/DL/FULL.
Each direction opens a fresh connection, validates the deterministic payload,
sends BEN2 v2 MTR2 raw metrics and validates the final server reply. FULL runs TX
and RX concurrently. One failure does not suppress later modes or transports.

Output shape (illustrative numbers, not measured results):

```text
* STCP-TCP     [PASS]
  -> UL   438000 bit/sec
  -> DL   410000 bit/sec
  -> FULL 800000 bit/sec
```

Transport PASS requires all three modes to pass. Failed modes print
`n/a bit/sec [FAIL: reason]`. Exit codes: 0 all PASS, 1 test failure, 2 invalid CLI.
UL/DL are respective payload rates. **FULL is aggregate UL+DL**, not each direction's
individual speed. Timing excludes socket connect and control exchange; a valid
server reply is required before a result is marked PASS.

## Start four server processes on the target

Use the BEN2 v2 Rust server supplied earlier, in four terminals:

```sh
./target/release/ben2-server --transport stcp-tcp --bind 192.168.1.20 --port 19000
./target/release/ben2-server --transport stcp-udp --bind 192.168.1.20 --port 19001
./target/release/ben2-server --transport tcp --bind 192.168.1.20 --port 19002
./target/release/ben2-server --transport udp --bind 192.168.1.20 --port 19003
```

Port defaults match these commands. Configure them individually:

```sh
./target/release/ben2-tester --host 192.168.1.20 \
  --tcp-port 19002 --udp-port 19003 --stcp-tcp-port 19000 --stcp-udp-port 19001 \
  --total 1048576 --chunk 4096 --timeout-ms 60000 --rounds 3
```

To test just the current STCP-TCP server:

```sh
./target/release/ben2-tester --host 192.168.1.20 --transport stcp-tcp --stcp-tcp-port 19000
```

Same Linux binary runs on x86/aarch64 when built for that host; run on host or RPi
with another machine's IP to check those endpoints. This tester is not Zephyr
firmware and cannot label a Linux↔Linux path as LTE/Zephyr. It sends carrier=unknown
in MTR2; stack counters=0 (not measured). Testing Zephyr's transport/carrier matrix
still requires its app and hardware.

Native STCP modes require matching modules on both client and server. Plain UDP
adds no pacing/retransmission: loss/reorder/duplicates fail the run. Both UDP choices
use the same 16-byte BUD2 envelope and 1024-byte payload limit as the earlier overlay.
All transports use writes up to 1024 bytes for a comparable send-call profile.
`--chunk` bounds these slices further if smaller than 1024. Send/receive call
counters track actual socket syscalls; rates count application payload only.
The server keeps results in its normal JSONL file. A single endpoint is tested
per invocation; run on each desired host/RPi combination to build a machine matrix.

## Validation

```sh
cargo test --locked
```

Included tests exercise fragmented/coalesced envelopes, wrong sequence/run,
BEN2 metric byte layout and native TCP UL/DL/FULL against an independent in-process
reference. The creation environment has no Rust compiler, so Rust compilation and
these tests have NOT been run here. Hardware STCP interop remains to be verified.
