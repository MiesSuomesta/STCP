#!/bin/bash

for dev in /dev/ttyACM0 /dev/ttyACM1; do
    stty -F "$dev" 115200 cs8 -cstopb -parenb \
        -ixon -ixoff -crtscts raw -echo
done

cat /dev/ttyACM0 > /tmp/acm0.log &
P0=$!

cat /dev/ttyACM1 > /tmp/acm1.log &
P1=$!

echo "Paina nyt RESET ja odota 5 sekuntia"
sleep 5

kill "$P0" "$P1" 2>/dev/null || true

echo "=== ACM0 ==="
cat /tmp/acm0.log
echo "=== ACM1 ==="
cat /tmp/acm1.log
