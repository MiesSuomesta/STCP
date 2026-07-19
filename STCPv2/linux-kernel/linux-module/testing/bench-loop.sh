#!/bin/bash
set -e
for i in 1 2 3 4 5; do
  echo "Run $i"
  /usr/bin/time -f "elapsed=%e" ./testing/stcp_large_test client
done
