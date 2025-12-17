#!/bin/bash

sudo bash cleanup-xterms

PORT=$1
PROTO=${2:-253}

netcon() {
	TS="$(date -Is)"
	echo "PAXSUDOS-DEBUG $TS : $*" | tee /dev/kmsg
}

bash dev-start.sh
sleep 5

netcon startataan dumpperi....
xterm -fa Monospace -fs 20 -hold -e tcpdump -i lo -nn -vv tcp port $PORT -X 2>&1 | tee tcpdump.log &

netcon startataan serveri nyt....
xterm -fa Monospace -fs 20 -hold -e /usr/bin/python3 /home/pomo/dev/testing/python-use/server.py $PORT $PROTO &
sleep 5

netcon startataan clientti nyt....ja 5sek päästä viimeinen viesti, jos se näkyy niin ei kone ole kaatunut.
sleep 2

netcon startataan clientti....
xterm -fa Monospace -fs 20 -hold -e /usr/bin/python3 /home/pomo/dev/testing/python-use/client.py $PORT $PROTO &

sleep 5

ss -ltnp | grep $1 | tee /dev/kmsg

netcon Jos tämä tulee, nii ei ole kaatunut


