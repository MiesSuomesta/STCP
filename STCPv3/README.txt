STCP benchmark notify overlay

Replaces only:
  tests/benchmark/raspberrypi/benchmark_client.py

Includes previous deadline fixes plus the actual condition wakeup fix:
  state_cv.notify_all()
after adding a new outstanding request.

Install from STCP repository root:
  unzip -o stcp-benchmark-notify-overlay.zip -d .
  python3 -m py_compile tests/benchmark/raspberrypi/benchmark_client.py
