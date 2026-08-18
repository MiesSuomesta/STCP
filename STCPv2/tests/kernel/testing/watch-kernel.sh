#!/bin/bash
watch -n1 '
echo "==== dmesg ====";
dmesg | tail -20;
echo;
echo "==== STCP ====";
ps -eo pid,pcpu,stat,cmd | grep stcp | grep -v grep
'
