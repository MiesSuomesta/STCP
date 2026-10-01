## STCP vs TCP/TLS benchmark results

A new benchmark suite was added to compare STCP, plain TCP and TLS 1.3 using an identical request/echo workload.

### Test configuration

* Loopback interface: `127.0.0.1`
* Clients: 4
* Payload: 262,144 bytes
* Duration: 30 seconds
* Errors: 0 in all completed tests

| Transport | TX throughput | RX throughput | Operations/s |   RTT p50 |   RTT p95 |   RTT p99 | Client CPU |
| --------- | ------------: | ------------: | -----------: | --------: | --------: | --------: | ---------: |
| TCP       |  617.53 MiB/s |  617.53 MiB/s |       2470.1 |  1.317 ms |  2.554 ms |  3.779 ms |     144.1% |
| TLS 1.3   |   51.81 MiB/s |   51.81 MiB/s |        207.2 | 18.446 ms | 23.778 ms | 28.754 ms |     189.5% |
| STCP      |  143.46 MiB/s |  143.46 MiB/s |        573.8 |  6.151 ms | 10.178 ms | 13.458 ms |      95.6% |

### Summary

STCP substantially outperformed TLS 1.3 in this local benchmark.

Compared with TLS, STCP delivered:

* approximately **2.77× higher application throughput**
* approximately **2.77× more operations per second**
* approximately **66.7% lower median RTT**
* approximately **53.2% lower p99 RTT**
* approximately **49.6% lower client-side CPU usage**

Plain TCP remains faster because it does not include the additional transport, reliability and security processing performed by STCP and TLS.

The important comparison is between STCP and TLS: STCP provided significantly higher throughput, lower latency and lower CPU consumption while completing the full 30-second test with four concurrent clients and zero errors.

These results are loopback measurements and should next be validated across physical network interfaces, multiple payload sizes and longer test durations.
