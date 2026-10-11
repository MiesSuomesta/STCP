# Zephyr BEN2 benchmark matrix

Linux server (Rust 1.85+):

```sh
cd ben2-server
cargo build --release
./target/release/ben2-server --transport all --bind 192.168.1.20 --port 19000 --report-ms 5000
```

Zephyr shell, after rebuilding and flashing the application:

```text
stcp config host 192.168.1.20
stcp config port 19000
stcp config total 1048576
stcp bench matrix
```

For LTE use the public server IP. Matrix runs TCP, STCP-TCP, UDP and STCP-UDP; each runs UL, DL and FULL. Failed tests do not prevent subsequent tests. Configuration port is the base port. Forward TCP 19000, TCP 19001, UDP 19002, UDP 19003 to the server. The matrix summary uses TCP 19001. Changing the base port shifts all four ports.

Linux prints each protocol's PASS/FAIL and UL/DL/FULL bit/sec. FULL is combined TX+RX. Zephyr sends raw counters; Linux calculates rates. Matrix elapsed time includes benchmark protocol control exchanges. Detailed BEN2_RESULT includes transfer timing. Final summary is appended to benchmark-results/zephyr-matrix.jsonl (override BEN2_MATRIX_RESULTS). Individual BEN2 results and periodic progress remain available. --report-ms 0 disables periodic progress.

Install overlay by extracting into zephyr/nordic/application. Includes complete Cargo.toml, Cargo.lock and Rust sources under testing/test-server. No Linux tester is needed: Zephyr initiates all connections. STCP kernel module must already be installed and support protocols 253/254. UDP tests detect losses; they do not add application retransmissions.

Validation: host C build with -Wall -Wextra -Werror; matrix exercised against independent Python fixture across all twelve combinations, including injected UL failure and successful remaining tests. Existing upload/download/full host checks passed. STCP is simulated by native sockets in host fixtures; hardware and real kernel interoperability must be tested on target. Rust toolchain was unavailable in the creation environment, so server compilation is not verified here.

```sh
sh validation/run-matrix-host.sh
```
