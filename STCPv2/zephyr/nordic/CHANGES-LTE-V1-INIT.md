# LTE initialization update

- Forces LTE-M-only system mode before `lte_lc_connect()`.
- Keeps automatic operator selection.
- Disables PSM and eDRX during benchmark/debug operation.
- Keeps APN `internet` and the existing STCP socket API unchanged.
- Logs `%XSYSTEMMODE`, `CEREG`, `COPS`, `CESQ`, and PDP contexts when LTE connection fails.
- Includes the isolated Python/West build script fix.
