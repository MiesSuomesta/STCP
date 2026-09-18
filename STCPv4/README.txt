STCPv2 benchmark-v2 phase 1.5: per-transport port split

Complete replacement file:
  zephyr/nordic/application/testing/benchmark-v2/run_benchmark.py

Changes only the host-runner configuration:
  TCP      uses --port      (default 19000)
  STCP-TCP uses --stcp-port (default 19010)

Before each transport the runner configures the matching Zephyr port and
starts only its own benchmark server on that port. Existing unrelated STCP
listeners on 19000 therefore no longer collide with the STCP benchmark.

No firmware, BEN2 wire protocol, workload, stream logic, or host server code
is changed by this overlay.

Example:
  bash run-benchmark.sh --serial /dev/ttyACM0 --host 192.168.1.20 \
    --total 1048576 --chunk 8192 --warmups 1 --runs 5 \
    --port 19000 --stcp-port 19010
