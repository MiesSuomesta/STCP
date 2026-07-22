# Benchmarkit

Tähän hakemistoon kootaan STCPv2:n suorituskykytestit.

## `stcp_vs_tls.py`

Ajaa saman echo-kuorman kolmella kuljetuksella:

- `tcp`: tavallinen TCP
- `tls`: TLS 1.3 TCP:n päällä
- `stcp`: `AF_STCP=45`, `SOCK_STREAM`, protokolla `253`

Tulokset sisältävät TX/RX-läpiviennin, operaatiot sekunnissa, yhteyden muodostusajan, RTT p50/p95/p99:n, virheet ja Python-clientin CPU-kuorman.

## Nopea paikallinen TCP/TLS-vertailu

```bash
cd benchmark
./run-local-comparison.sh
```

Parametrit ympäristömuuttujilla:

```bash
DURATION=60 CLIENTS=16 PAYLOAD=262144 ./run-local-comparison.sh
```

Kaikki kolme, kun STCP-moduuli on ladattu:

```bash
RUN_STCP=1 DURATION=60 CLIENTS=16 PAYLOAD=262144 ./run-local-comparison.sh
```

## Erilliset palvelimet

TCP:

```bash
python3 stcp_vs_tls.py server --mode tcp --bind 0.0.0.0 --port 9000
```

TLS-varmenne ja palvelin:

```bash
./create-test-cert.sh
python3 stcp_vs_tls.py server --mode tls --bind 0.0.0.0 --port 9001 \
  --cert cert.pem --key key.pem
```

STCP:

```bash
sudo python3 stcp_vs_tls.py server --mode stcp --bind 0.0.0.0 --port 9002
```

## Clientit

```bash
python3 stcp_vs_tls.py client --mode tcp --host 127.0.0.1 --port 9000 \
  --clients 4 --payload 262144 --duration 30

python3 stcp_vs_tls.py client --mode tls --host 127.0.0.1 --port 9001 \
  --clients 4 --payload 262144 --duration 30 --insecure

python3 stcp_vs_tls.py client --mode stcp --host 127.0.0.1 --port 9002 \
  --clients 4 --payload 262144 --duration 30
```

`--verify` tarkistaa paluudatan. Maksimaalista läpivientiä mitattaessa tarkistus kannattaa jättää pois.

## Python ja AF_STCP

Pythonin tavallinen `socket.bind((host, port))`, `connect()` ja `accept()` eivät
osaa käsitellä omaa `AF_STCP=45`-osoiteperhettä ja voivat antaa virheen
`OSError: bind(): bad family`.

Benchmark käyttää STCP-moodissa näihin kutsuihin suoraan libc:tä. Socket luodaan
perheellä `AF_STCP`, mutta `bind()`- ja `connect()`-osoite annetaan IPv4
`sockaddr_in`-rakenteena, jossa `sin_family = AF_INET`, kuten STCP-kernelimoduuli
odottaa.
