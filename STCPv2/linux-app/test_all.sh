#!/bin/bash

pncnote -a "STCP Testing" "STCP test" "Doing simple benchmark"
sudo bash tests/benchmark.sh

pncnote -a "STCP Testing" "STCP test" "Doing perf churn run"
sudo bash tests/benchmark-perf-churn-small.sh

pncnote -a "STCP Testing" "STCP test" "Doing flamegraph run"
sudo bash tests/benchmark-flamegraph.sh

pncnote -a "STCP Testing" "STCP test" "Doing turbostat run"
sudo bash tests/benchmark-turbostat.sh

pncnote -a "STCP Testing" "STCP test" "All done."
