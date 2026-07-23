# v21 benchmark summary

`stcp bench all` prints one final comparison block containing:

- transport, server, chunk size and transfer size
- operator, RAT, LTE band, RSRP, RSRQ, raw SNR and active APN
- upload throughput and elapsed time
- download throughput and elapsed time
- full-duplex TX, RX and aggregate throughput
- status/error code for every phase

The summary is printed even if one phase fails. Tests that were not run are marked `FAILED(-ECANCELED)`.
