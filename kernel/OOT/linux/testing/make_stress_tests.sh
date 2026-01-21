#!/bin/bash
set -x

PORT=${1:-12345}
HOST=${2:-nas}
MODE=${3:-churn}
report=${4:-5}
proto=${5:-253}
timeout=${6:-30}

myname=$(basename "$0")

msg() {
        echo "[$myname] $*" | sudo tee /dev/kmsg 2>/dev/null
        echo "[$myname @ $(date)] $*"
}


mkTest() {
	TARGS="--clients $1 --duration $2 --msg-size $3   --messages-per-conn $4 --timeout $5"
	JOPT="--summary-json --summary-json-file summary.$HOST.$PORT.$proto.$MODE.$3.json"
	ARGS="--host $HOST --port $PORT --mode $MODE --report-every $report $JOPT $TARGS"

	PYTHONUNBUFFERED=1 stdbuf -oL -eL python3 stcp_stress_test.py $ARGS --proto $proto 2>&1 \
		| while read -r LN; do
   			msg "$LN" 
			done

}

doTestsForMode() {
	MODE="$1"
        # clients duration msg-size msg-per-conn timeout
	mkTest 50 70   256 500 $timeout
	mkTest 20 70  4096 500 $timeout
	mkTest 10 70 16384 200 $timeout
	mkTest  5 70 65536 100 $timeout
}


doTestsForMode "churn"
#doTestsForMode "steady"
