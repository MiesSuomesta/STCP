#!/bin/bash

# Liian vaaralliset
#rm -rf common/musl-cross-make
#rm -rf dependencies/installations

echo "🔁 Cleaning all ....."
cargo clean
rm Cargo.lock
rm -rf target

echo "🔁 Cleaning all: Targets...."
find . -type d -name target -exec rm -rf {} \;

echo "🔁 Cleaning all: .cargo dirs...."
find . -type d -name .cargo -exec rm -rf {} \;

echo "🔁 Cleaning all: .lock files ...."
find . -name Cargo.lock -exec rm {} \;
