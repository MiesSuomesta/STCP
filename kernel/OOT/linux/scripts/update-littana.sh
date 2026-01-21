#!/usr/bin/bash

make kmod && (
	scp kmod/stcp_rust.ko nas:~/
	ssh -t nas "bash -i update.sh"
)
