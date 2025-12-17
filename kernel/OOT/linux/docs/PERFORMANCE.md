# Performance

## Steady-State Performance

Environment:
- Mode: steady
- Clients: 50
- Duration: 70 seconds

Results:
- Operations: 596,106
- Throughput: ~8.8 MB/s
- RPS: ~8,500
- Latency:
  - avg: 5.0 ms
  - p50: 4.2 ms
  - p95: 5.6 ms
  - p99: 9.8 ms
- Errors: 0

Conclusion:
STCP maintains stable encrypted throughput and low latency over sustained
periods without errors or degradation.
