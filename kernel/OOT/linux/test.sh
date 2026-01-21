#!/bin/bash

MYDIR=$(dirname "$0")
ROOTDIR=$(cd "$MYDIR" && pwd)

sudo bash $ROOTDIR/cleanup-xterms
sudo bash $ROOTDIR/scripts/rlmod

PORT=$1
PROTO=${2:-253}
LOOPS=${3:-1}

netcon() {
	TS="$(date -Is)"
	echo "PAXSUDOS-DEBUG $TS : $*" | tee /dev/kmsg
}

bash $ROOTDIR/scripts/enable-netconsole.sh
sleep 5

netcon startataan dumpperi....
xterm -fa Monospace -fs 20 -hold -e tcpdump -i lo -nn -vvv port $PORT -X 2>&1 | tee tcpdump.log &

netcon startataan serveri nyt....
xterm -fa Monospace -fs 20 -hold -e /usr/bin/python3 $ROOTDIR/testing/python-use/server.py --host 127.0.0.1 --port $PORT --proto 253 --mode app --echo &

netcon startataan clientti nyt....ja 5sek päästä viimeinen viesti, jos se näkyy niin ei kone ole kaatunut.
sleep 5

netcon startataan clientti....
xterm -fa Monospace -fs 20 -hold -e /usr/bin/python3 $ROOTDIR/testing/python-use/client.py --host 127.0.0.1 --port $PORT --proto 253 --loops $LOOPS --mode app --expect-echo &

sleep 5

ss -ltnp | grep $1 | tee /dev/kmsg

netcon Jos tämä tulee, nii ei ole kaatunut
sleep 15
netcon Jos tämä tulee, nii ei ole kaatunut 20sek ajoaikaa jo



