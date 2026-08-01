#!/bin/bash

python3 scripts/run-infra-benchmarks.py \
  --device /dev/ttyACM0 \
  --serial-number 1052043013 \
  --host 192.168.1.20 \
  --port 19000 \
  --transports tcp \
  --payloads 64,1024,4096,65536,262144,1048576 \
  --directions upload,download,full \
  --total 1048576

