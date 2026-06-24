#!/bin/bash 
set -euo pipefail
cln=${1:-nope}
BLOG=/tmp/clean-log.$$.txt

doOnTrap() {
	echo "💥 BANG! Somethign wrong at $1"
	cp $BLOG clean-log.txt
	exit 1
}

trap "doOnTrap ${LINENRO:-0}" ERR

doCleanCargo() {
	(
		cd $1
		echo "🔄 Cleaning $* (at $PWD) ...."
		cargo clean
		rm -rf target Cargo.lock
	) 2>&1 | tee -a $BLOG

}

(
	doCleanCargo . "The server"
	doCleanCargo rust-server-lib "RUST server lib"
	doCleanCargo rust-c-wrapper "RUST C wrapper lib"
	echo "🔄 Cleaning main build (at $PWD) ...."
	rm -rf build
)  2>&1 | tee -a $BLOG

rm -f $BLOG
