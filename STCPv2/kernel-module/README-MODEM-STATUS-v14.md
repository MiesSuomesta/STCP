# STCP benchmark v14 modem status

New interpreted shell commands:

```text
stcp modem system
stcp modem health
stcp modem signal
stcp modem all
```

`system` reports configured LTE-M/NB-IoT/GNSS modes and the currently attached RAT.
`health` summarizes operator, RAT, band, RSRP, RSRQ, raw XMONITOR SNR, registration,
RRC state, PSM, APN and PDP addresses.

The existing raw AT diagnostic commands remain available.
