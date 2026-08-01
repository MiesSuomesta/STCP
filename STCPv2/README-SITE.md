# STCPv2 website and benchmark reports

## Generate

```bash
bash generate-site.sh
```

Generated website root:

```text
stcp.fi/
├── index.html
├── technology/
├── benchmarks/
│   ├── index.html
│   ├── raspberry-pi/tcp/index.html
│   ├── raspberry-pi/udp/index.html
│   ├── zephyr/index.html
│   ├── compare/index.html
│   ├── releases/index.html
│   └── raw/index.html
├── downloads/
├── documentation/
└── contact/
```

The same global menu is used by the homepage and every generated benchmark page.

## Publish benchmark section

```bash
STCP_PUBLISH_HOST=fuji \
STCP_PUBLISH_USER=www-data \
bash publish-stcp-fi.sh
```

The benchmark publisher updates `/var/www/stcp.fi/benchmarks` by default.

## Current selected result sets

- Raspberry Pi: automatically selects the result directory with the largest valid recursive case set.
- Zephyr: selects the newest `zephyr-*` result set.

## Multi-payload protocol comparison

Protocol-family comparison charts support selecting several payloads at once.
Controls include Select all, Clear, Small payloads, and Large payloads. Each
selected payload is rendered as one group with TCP/STCP-TCP/TLS or
UDP/STCP-UDP/TLS-reference bars. Missing protocol results are shown as N/A.
