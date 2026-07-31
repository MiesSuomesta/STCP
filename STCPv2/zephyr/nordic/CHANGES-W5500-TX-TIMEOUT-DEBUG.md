# W5500 TX timeout diagnostics

Ethernet build now patches the NCS 3.3.0 W5500 driver before compilation.

Changes in `w5500_tx()`:

- checks and reports the `S0_CR_SEND` command result;
- increases SENDOK wait from 10 ms to 500 ms;
- on timeout logs `IR`, `SIR`, `S0_IR`, `S0_SR` and the physical INT GPIO level;
- reports SPI read return codes for every diagnostic register;
- stores the untouched driver as `eth_w5500.c.stcp-original`;
- patch operation is idempotent.

The Ethernet build also enables `CONFIG_ETHERNET_LOG_LEVEL_DBG=y`.

Run:

```bash
bash scripts/build-ethernet.sh
bash scripts/flash-ethernet.sh
```

A failed transmit should now produce a line similar to:

```text
TX timeout: len=... IR=... SIR=... S0_IR=... S0_SR=... INT=...
```

## NCS 3.3.0 compile fix

The debug patch no longer references `W5500_SIR`, which is not defined by the
NCS 3.3.0 W5500 driver headers. Timeout diagnostics now read only `IR`,
`S0_IR`, `S0_SR`, and the interrupt GPIO level. The build helper also repairs
the earlier broken patch automatically before compiling.
