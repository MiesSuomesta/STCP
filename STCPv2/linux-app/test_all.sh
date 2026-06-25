#!/bin/bash

#bash cleanup
#cargo build -r

#./target/release/stcp-runner server 127.0.0.1:7777 >& /dev/null &
#PID_STCP=$!

#./target/release/tls-runner server 127.0.0.1:7778 crates/tls-runner/cert/cert.pem crates/tls-runner/cert/key.pem § >& /dev/null &
#PID_TLS=$!

#sleep 3

pncnote -a "STCP Testing" "STCP test" "Doing simple benchmark"
sudo bash tests/benchmark.sh

pncnote -a "STCP Testing" "STCP test" "Doing perf churn run"
sudo bash tests/benchmark-perf-churn-small.sh

pncnote -a "STCP Testing" "STCP test" "Doing flamegraph run"
sudo bash tests/benchmark-flamegraph.sh

pncnote -a "STCP Testing" "STCP test" "Doing turbostat run"
sudo bash tests/benchmark-turbostat.sh

pncnote -a "STCP Testing" "STCP test" "All done."

kill $PID_STCP $PID_TLS
