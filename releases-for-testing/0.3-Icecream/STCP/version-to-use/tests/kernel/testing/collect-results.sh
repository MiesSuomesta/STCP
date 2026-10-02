#!/bin/bash
mkdir -p results
dmesg > results/dmesg.txt
uname -a > results/uname.txt
lsmod > results/lsmod.txt
cp /proc/crypto results/proc_crypto.txt
echo "Collected into results/"
