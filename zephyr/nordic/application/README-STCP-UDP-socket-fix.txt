Extract into zephyr/nordic/application, overwriting src/echo_benchmark.c. Rebuild and flash using your normal Ethernet/LTE build scripts (clean build recommended).

Root cause: benchmark called AF_STCP + SOCK_STREAM + protocol 254. Uploaded Zephyr module accepts only protocol 0 or IPPROTO_STCP (253). It selects UDP using SOCK_DGRAM and internally passes 254 to Rust.
Fix: STCP-TCP uses SOCK_STREAM + 253; STCP-UDP uses SOCK_DGRAM + 253. Linux server ABI remains unchanged.

All twelve matrix combinations passed host fixture after correction. Injected failure still reports failure and remaining tests complete. Host shim now enforces Zephyr socket contract. Target hardware validation remains pending. This fixes socket creation only; plain UDP download/full failures are a separate issue.

With server --transport all and base port 19000:
stcp config port 19000
stcp bench matrix

Single STCP-UDP test:
stcp config transport stcp-udp
stcp config port 19001
stcp bench upload
