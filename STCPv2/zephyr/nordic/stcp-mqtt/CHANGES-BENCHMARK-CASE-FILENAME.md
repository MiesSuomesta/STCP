# Benchmark runner case filename fix

- Added the missing `case_filename()` helper.
- Result files use deterministic infra-compatible names:
  - `tcp-c1-p4096-q1-upload.json`
  - `tcp-c1-p4096-q1-download.json`
  - `tcp-c1-p4096-q1-full.json`
- Python syntax and filename generation were validated.
- No W5500 driver, firmware transport, SPI, or Ethernet changes.
