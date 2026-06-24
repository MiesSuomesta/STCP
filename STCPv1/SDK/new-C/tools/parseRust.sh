#!/bin/bash
dir=$(dirname "$1")
fn=$(basename "$1")

(
	cd "$dir"
	rustc -Z parse-crate-root-only "$fn"
)
