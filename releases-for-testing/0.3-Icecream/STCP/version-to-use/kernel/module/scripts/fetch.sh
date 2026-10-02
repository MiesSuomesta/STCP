#!/bin/bash
DN=$(dirname $0)
scp lja@serveri:~/konsoli.log /tmp/konsoli.log
bash $DN/decode-oops.sh /tmp/konsoli.log
