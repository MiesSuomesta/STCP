# STCPv6 Nordic ↔ Linux BEN2 test server patch

Source: Nordic package `stcp-09102026_113325.zip`, `application/testing/test-server/bin/stcp_server.rs`.

## Installation

```bash
cd /srv/stcp-project/STCP/STCPv6/zephyr/nordic/application/testing/test-server
cp -a bin/stcp_server.rs bin/stcp_server.rs.before-ben2
unzip -o /path/to/stcpv6-ben2-duplex-fix.zip 'bin/*'
cargo test --release --bin stcp-server
cargo build --release --bin stcp-server
```

Stop old stcp-server and restart `target/release/stcp-server` (TCP port 19000).

Nordic shell: `stcp bench upload`, then `stcp bench download`, then `stcp bench full`.

## Changes

* Detect BEN2 on decrypted application stream; preserve echo fallback for non-BEN2 clients.
* Receive and verify upload data (`salt=0x17`) without echoing it.
* Generate download data (`salt=0x53`) and verify BEN2 ACK; emit 20-byte BE BEN2 reply.
* FULL duplex: send at most one 1024-byte application chunk per connection-loop iteration, allowing the incoming stream to be drained between writes. This is bounded interleaving, not separate sender/receiver threads.
* Reassemble STCP wire frames across arbitrary TCP reads before feeding the Rust core.
* No changes to handshake, AES-GCM or KDF.

## Limitations

No complete STCP cargo workspace or physical Nordic board available in this environment; compilation and hardware integration are **not verified**. The BEN2 protocol module includes unit tests for upload, download and interleaved FULL. Existing server uses verbose per-frame tracing, which will distort throughput measurements. For benchmarking performance, disable verbose traces after correctness tests pass.
