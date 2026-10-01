# STCP Python stress tests

`stcp_stress.py` uses direct libc syscalls through `ctypes`, because Python's standard `socket` module cannot encode sockaddr data for custom AF_STCP family 45. It starts an STCP echo server and configurable client workers in
one process. It reports the situation every five seconds by default.

## Throughput test

Persistent connections, suitable for measuring maximum throughput:

```bash
./stcp_stress.py \
  --mode throughput \
  --clients 16 \
  --payload 262144 \
  --duration 180 \
  --report-every 5
```

## Connection churn

Creates and closes a connection for every request:

```bash
./stcp_stress.py \
  --mode churn \
  --clients 32 \
  --payload 4096 \
  --duration 60
```

## Mixed workload

Keeps each connection for 32 exchanges and then reconnects:

```bash
./stcp_stress.py \
  --mode mixed \
  --clients 16 \
  --payload 65536 \
  --reconnect-every 32 \
  --duration 120
```

## Report fields

Every interval shows:

- instantaneous TX and RX throughput
- completed operations per second
- successful and failed connections
- send, receive, verification and server errors
- p50, p95 and p99 request/echo latency
- Python process CPU usage and RSS
- cumulative received data

The final result is also written to `stcp-stress-result.json`.

## Suggested test sequence

Run correctness first:

```bash
make LLVM=1 test-basic
make LLVM=1 test-large
```

Then short stress tests:

```bash
./stcp_stress.py --mode throughput --clients 4 --duration 20
./stcp_stress.py --mode mixed --clients 8 --duration 30
./stcp_stress.py --mode churn --clients 16 --duration 30
```

Then the longer throughput run:

```bash
./stcp_stress.py \
  --mode throughput \
  --clients 16 \
  --payload 1048576 \
  --duration 180
```

For a 250 MB/s target, benchmark on a non-KASAN release kernel. KASAN is still
recommended for correctness and short stress runs.


## AF_STCP address-family fix

The socket syscall uses `AF_STCP=45`, but the address passed to `bind()` and `connect()` is an IPv4 `sockaddr_in` whose `sin_family` is `AF_INET`. This matches `stcp_bind()` and `stcp_connect()` in the kernel module.

## Osoiteperheen tärkeä ero

Socket luodaan perheellä `AF_STCP=45`, mutta `bind()`- ja `connect()`-kutsuille
annettava rakenne on IPv4 `sockaddr_in`, jossa `sin_family = AF_INET`.
Tämä vastaa kernelimoduulin `stcp_bind()`- ja `stcp_connect()`-wrappereita.

## Yhteydenmuodostuksen korjaus

Client-socket pidetään blocking-tilassa `connect()`-kutsun ajan, koska STCP:n
nykyinen connect-polku tekee handshaken synkronisesti. `O_NONBLOCK` asetetaan
vasta onnistuneen connectin jälkeen. Listener vaihdetaan nonblocking-tilaan
vasta `listen()`-kutsun jälkeen ja acceptoidut socketit heti acceptin jälkeen.
