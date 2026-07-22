#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
openssl req -x509 -newkey rsa:2048 -nodes \
  -keyout key.pem -out cert.pem -days 3650 \
  -subj '/CN=localhost'
chmod 600 key.pem
printf 'Created %s and %s\n' "$PWD/cert.pem" "$PWD/key.pem"
