# Test fix V10: functional test before stress

`python/run_stress_suite.sh` now runs a deterministic end-to-end echo smoke test first.

Default cases:

- 64 bytes
- 4096 bytes
- 65536 bytes

Each case uses a separate listener and connection and must complete:

1. bind
2. listen
3. connect
4. accept
5. client send
6. server receive and verify
7. server echo
8. client receive and verify
9. clean close and thread join

The script exits non-zero immediately on the first failed stage. Throughput, mixed,
and churn tests are disabled by default and can be enabled with `RUN_STRESS=1`.
