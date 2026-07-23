# STCP benchmark shell v5

Build-time selection is controlled by Kconfig:

```ini
CONFIG_STCP_BENCH_SHELL=y
CONFIG_STCP_BENCH_AUTORUN=n
```

With the shell enabled, use:

```text
stcp config show
stcp config host lja.fi
stcp config port 19000
stcp config transport tcp
stcp config chunk 4096
stcp config total 1048576
stcp config timeout 60000

stcp bench upload
stcp bench download
stcp bench full
stcp bench all
```

To remove the shell completely from the build and run the tests automatically:

```ini
CONFIG_STCP_BENCH_SHELL=n
CONFIG_STCP_BENCH_AUTORUN=y
```

TLS is accepted as a runtime transport name but currently returns `-ENOTSUP` until the TLS socket implementation is added. TCP works immediately and STCP uses the current `AF_STCP` socket path.
