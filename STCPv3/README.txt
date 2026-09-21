STCP benchmark TLS blocking-I/O overlay

Replaces only:
  tests/benchmark/raspberrypi/benchmark_client.py

Includes all previous benchmark-client fixes:
  - hard duration/drain deadlines
  - sender state_cv.notify_all()
  - STCP hostname resolution (e.g. --host raspi)
  - TCP/TLS socket timeout is used only for connect; benchmark I/O is blocking

Install from STCP repository root:
  unzip -o stcp-benchmark-tls-blocking-overlay.zip -d .
  python3 -m py_compile tests/benchmark/raspberrypi/benchmark_client.py
