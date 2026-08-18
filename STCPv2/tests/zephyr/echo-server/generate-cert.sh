#!/usr/bin/env bash
set -euo pipefail
openssl req -x509 -newkey rsa:2048 -sha256 -nodes \
  -keyout key.pem -out cert.pem -days 3650 \
  -subj '/CN=stcp-echo' \
  -addext 'subjectAltName=DNS:lja.fi,IP:127.0.0.1'
chmod 600 key.pem
printf 'Created %s/cert.pem and %s/key.pem\n' "$PWD" "$PWD"
