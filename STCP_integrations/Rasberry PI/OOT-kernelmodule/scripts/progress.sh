#!/usr/bin/bash

MARKER=$( tail -n 1 $1 | sed 's,\]\[.*,,' | awk '{print $2 }' )

cnt=$(grep -A 10000 "$MARKER" $1 |wc -l)
tot=$(cat $1 |wc -l)

echo "$cnt / $tot analysoitu"

