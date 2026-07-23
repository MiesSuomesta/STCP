# Echo server IP whitelist (v7)

Whitelist filtering happens immediately after `accept()` and before TLS handshake or benchmark protocol parsing.

## Allow one exact public IP

```bash
sudo python3 echo_server.py \
  --no-tls --no-stcp \
  --allow 85.76.101.139/32 \
  --verbose
```

The `/32` means exactly one IPv4 address. The shorter form below is equivalent:

```bash
--allow 85.76.101.139
```

## Allow several addresses or networks

Repeat `--allow`:

```bash
sudo python3 echo_server.py \
  --no-tls --no-stcp \
  --allow 85.76.101.139 \
  --allow 192.168.1.0/24
```

## Read whitelist from a file

Copy and edit the supplied example:

```bash
cp whitelist.txt.example whitelist.txt
nano whitelist.txt
```

Then start the server:

```bash
sudo python3 echo_server.py \
  --no-tls --no-stcp \
  --allow-file whitelist.txt \
  --verbose \
  --report-every 5
```

Blank lines and lines beginning with `#` are ignored.

When a whitelist is configured, every address not matching an entry is rejected and logged:

```text
WARNING TCP rejected non-whitelisted client: ('16.58.56.214', 41234)
```

Without `--allow` or `--allow-file`, filtering is disabled for backward compatibility and all clients are accepted.
