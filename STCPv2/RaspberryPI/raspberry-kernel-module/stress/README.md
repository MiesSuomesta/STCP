# STCP stress tests

This package contains separate TCP-carrier and UDP-carrier stress runners.

## Files

- `testing/stcp_stress.c`
- `testing/run-stress-tcp.sh`
- `testing/run-stress-udp.sh`
- `Makefile.stress.inc`

## Modes

### Churn

Repeated lifecycle test:

```text
socket → connect → send → recv/verify → shutdown → close
```

Every worker creates a fresh connection for every iteration.

### Steady

Each worker keeps one persistent connection and repeatedly sends and verifies
payloads for the requested duration. Failed connections are re-established.

## Build manually

```bash
cc -O2 -g -Wall -Wextra -Werror -pthread \
  testing/stcp_stress.c \
  -o testing/stcp-stress
```

## TCP carrier

```bash
bash testing/run-stress-tcp.sh
```

Defaults:

- protocol 253
- port 7777
- 16 clients
- 1000 churn iterations/client
- 60-second steady test
- 4096-byte payload

## UDP carrier

```bash
bash testing/run-stress-udp.sh
```

Defaults:

- protocol 254
- port 7778
- 16 clients
- 1000 churn iterations/client
- 60-second steady test
- 4096-byte payload

## Environment overrides

```bash
STCP_STRESS_CLIENTS=64 \
STCP_STRESS_ITERATIONS=10000 \
STCP_STRESS_DURATION=3600 \
STCP_STRESS_PAYLOAD=65536 \
bash testing/run-stress-udp.sh
```

Carrier protocol IDs can be overridden:

```bash
STCP_TCP_PROTO=253
STCP_UDP_PROTO=254
```

## Makefile integration

Append `Makefile.stress.inc` to the module Makefile, or include it:

```make
include Makefile.stress.inc
```

Then run:

```bash
make LLVM=1 stress-tcp
make LLVM=1 stress-udp
make LLVM=1 stress-all
```

## Recommended progression

First:

```bash
STCP_STRESS_CLIENTS=8 \
STCP_STRESS_ITERATIONS=1000 \
STCP_STRESS_DURATION=60 \
make stress-all
```

Then KASAN overnight:

```bash
STCP_STRESS_CLIENTS=64 \
STCP_STRESS_ITERATIONS=100000 \
STCP_STRESS_DURATION=28800 \
STCP_STRESS_PAYLOAD=16384 \
make stress-all
```
