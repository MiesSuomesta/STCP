#!/bin/bash

PORT=$1
python3 stcp_stress_test.py --port $PORT --mode steady --clients 50 --duration 70 --msg-size 256   --messages-per-conn 500 --timeout 10 --report-every 15
python3 stcp_stress_test.py --port $PORT --mode steady --clients 20 --duration 70 --msg-size 4096  --messages-per-conn 500 --timeout 10 --report-every 15
python3 stcp_stress_test.py --port $PORT --mode steady --clients 10 --duration 70 --msg-size 16384 --messages-per-conn 200 --timeout 15 --report-every 15
python3 stcp_stress_test.py --port $PORT --mode steady --clients 5  --duration 70 --msg-size 65536 --messages-per-conn 100 --timeout 20 --report-every 15

