# StcpFreeSDK 0.1.0

Minimal Linux STCP-TCP echo servers and clients in Rust, Python, Go and Dart.
Each implementation echoes raw bytes and serves one client at a time. The
optional argument is the listening port (default 7000). Listen address is
0.0.0.0. Stop with Ctrl+C. Run one implementation at a time on a given port.

Requires an installed and loaded compatible STCP Linux kernel module on both
endpoints. This package contains only application examples: no kernel-module
source, encryption implementation, benchmark tooling or private SDK crates.
It does not configure compression; kernel defaults apply.

## Public socket ABI

- `socket(45 /* AF_STCP */, SOCK_STREAM, 253 /* STCP-TCP */)`
- `bind`: 16-byte IPv4 `sockaddr_in` with `sin_family = AF_INET (2)`.
- Port is in network byte order; address is INADDR_ANY.
- Standard `listen`, `accept`, `recv`/`read`, `send`/`write`, `close`.

The constants and address layout match the supplied SDK Linux backend and
kernel bind implementation. This uses the STCP module's custom family number;
it requires that module, not an unmodified Linux networking stack.
Python and Dart call libc to bind the IPv4 structure to the custom family.
Dart currently targets glibc Linux; all implementations target Linux IPv4.
Ordinary TCP clients cannot communicate with this STCP server. Use an STCP-TCP
client, for example the existing SDK echo client in its STCP-TCP mode.

## Run

From this directory:

```sh
# Python 3, standard library only
python3 python/echo_server.py 7000

# Rust 1.63+ (edition 2021), no Cargo dependencies
cd rust
cargo build --release --offline
./target/release/stcp-free-echo 7000
```

Run Go and Dart from the package root in another shell:

```sh
# Go 1.20+, standard library only
cd golang
go build -o stcp-free-echo ./cmd/echo-server
./stcp-free-echo 7000
```

```sh
# Dart 3+, SDK only; no pub get needed
dart run dart/echo_server.dart 7000
# Optional native build:
dart compile exe dart/echo_server.dart -o stcp-free-echo-dart
./stcp-free-echo-dart 7000
```

## Validation status

Python syntax and echo handling were checked locally, including binary payloads,
EOF and sequential connections using ordinary TCP sockets as an isolated test
of the echo loop. The STCP socket/bind ABI was checked against the supplied
source. No STCP kernel module or Rust, Go and Dart toolchains were available
in the packaging environment: those builds and actual STCP interoperability
must be verified on the target host. No hardware test is claimed.

SDK version 0.1.0 is the version of these examples, independent of STCP module
version. No license to the separately distributed STCP module is included.

## Echo clients

All clients take `IPv4 [port] [message]`. The port defaults to 7000 and the
UTF-8 message to `Hello STCP!`. Empty messages are rejected. Clients compare
the complete echo byte for byte and exit with status 1 on failure. All clients
and servers can be paired across languages using the same compatible module.
Clients alternate sending and receiving chunks of at most 4096 bytes to avoid
buffer deadlocks with long messages; partial reads/writes are handled.
They use blocking sockets with no application deadline; Ctrl+C stops a stalled
connection. Host arguments must be IPv4 literals, not DNS names.

From the package root, with a server running on the target:

```sh
python3 python/echo_client.py 192.168.1.20 7000 "Terve STCP!"

# Build both Rust binaries, then run the client:
(cd rust && cargo build --release --offline)
./rust/target/release/echo-client 192.168.1.20 7000 "Terve STCP!"

# Go binaries have separate command directories:
(cd golang && go build -o echo-client ./cmd/echo-client)
./golang/echo-client 192.168.1.20 7000 "Terve STCP!"

dart run dart/echo_client.dart 192.168.1.20 7000 "Terve STCP!"
# Optional Dart native executable:
dart compile exe dart/echo_client.dart -o stcp-echo-client-dart
```

A successful client prints `PASS: <n> bytes echoed unchanged`.
The Dart client shares the libc bindings in `dart/echo_server.dart`; keep both
Dart sources together (importing the server does not start its main function).
Python client/server loop validation also covers fragmented responses, a
256 KiB binary payload, Unicode, mismatch detection and premature disconnects
using ordinary TCP test sockets. Actual STCP and cross-language tests remain
pending on a host with the required module and language toolchains.

## Included Robot Framework tests

See [tests/README.md](tests/README.md) for the complete setup. Each language
has its own ten-case integration suite, including client/server operation,
binary data, long messages and faulty responses. Quick start:

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install -r tests/requirements.txt
bash tests/run.sh --all
```

Actual STCP is required; the runner never substitutes native TCP. HTML reports,
Robot XML and build/server logs are archived per run and language.
