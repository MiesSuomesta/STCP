# V13 modem status shell commands

Enabled when `CONFIG_STCP_BENCH_SHELL=y`.

Commands:

```text
stcp modem signal
stcp modem network
stcp modem band
stcp modem packet
stcp modem sleep
stcp modem apn
stcp modem all
```

`stcp modem all` runs all supported AT queries. Some commands depend on modem
firmware/operator support; unsupported queries are printed as warnings while the
remaining groups continue.

Recommended workflow around a benchmark:

```text
stcp modem all
stcp bench upload
stcp modem packet
stcp modem signal
```
