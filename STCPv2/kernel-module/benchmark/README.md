# STCP Raspberry benchmark

Raspberry Pi:
```bash
chmod +x *.sh *.py
./start-servers.sh
```

x86 client:
```bash
RPI_HOST=192.168.1.50 ./run-all.sh
```

Quick run:
```bash
RPI_HOST=192.168.1.50 DURATION=10 CLIENTS_LIST='1 4' PAYLOADS='1024 65536 1048576' PIPELINES='1 8' ./run-all.sh
```

Default matrix is 216 runs (~108 min at 30 s each). Outputs go under `results/<timestamp>/`.


## Carrier selection

Raspberry: `STCP_TRANSPORT=tcp bash start-servers.sh` or `STCP_TRANSPORT=udp bash start-servers.sh`.

Client: `STCP_TRANSPORT=tcp RPI_HOST=192.168.1.199 ./run-all.sh` or UDP respectively.

## Full TCP + UDP matrix

```bash
RPI_HOST=192.168.1.199 RPI_SSH=pi@192.168.1.199 ./run-all-full.sh
```

100-client, 32 KiB test for both carriers:

```bash
RPI_HOST=192.168.1.199 RPI_SSH=pi@192.168.1.199 DURATION=30 CLIENTS_LIST="100" PAYLOADS="32768" PIPELINES="1" ./run-all-full.sh
```


## Raspberry server IRQ and softirq metrics

IRQ collection is enabled by default. Each test captures Raspberry counters before and after the run over SSH:

- `/proc/interrupts`
- `/proc/softirqs`
- `/proc/stat`

Required variables:

```bash
RPI_HOST=192.168.1.199
RPI_SSH=pi@192.168.1.199
RPI_BENCHMARK_DIR=/home/pi/benchmark
```

Disable collection:

```bash
IRQ_METRICS=0 ./run-all.sh
```

Important output fields:

- `server_network_irq_per_1k_ops`
- `server_net_rx_softirq_per_1k_ops`
- `server_net_tx_softirq_per_1k_ops`
- `server_kernel_network_events_per_1k_ops`
- `server_network_irq_per_mib`
- `server_cpu_busy_percent`

These are kernel-activity and processing-efficiency proxies, not direct electrical power measurements.


## perf efficiency dashboard metrics

Enabled by default with `PERF_METRICS=1`.

The Raspberry Pi server captures system-wide `perf stat` counters around each test:

- task-clock
- context-switches
- cpu-migrations
- page-faults
- cycles
- instructions
- branches
- branch-misses
- cache-references
- cache-misses

Normalized result fields include cycles/op, instructions/op, context switches/1k ops,
cycles/MiB, instructions/MiB, IPC and cache/branch miss percentages.

Default privilege prefix:

```bash
REMOTE_PERF_PREFIX="sudo -n"
```

If perf is available without sudo:

```bash
REMOTE_PERF_PREFIX="" ./run-all-full.sh
```

Unsupported counters become null and do not fail the benchmark.
These are processing-efficiency proxies, not direct electrical power measurements.


## Publish latest benchmark to stcp.fi

After a completed full run:

```bash
PUBLISH_TARGET='www-data@fuji:~/html/public/stcp.fi/' \
./publish-latest-web.sh
```

Run and publish automatically:

```bash
AUTO_PUBLISH_WEB=1 \
PUBLISH_TARGET='www-data@fuji:~/html/public/stcp.fi/' \
./run-all-full.sh
```
