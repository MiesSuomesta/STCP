# STCP Raspberry benchmark

Raspberry Pi:
```bash
chmod +x *.sh *.py
./start-servers.sh
```

x86 client:
```bash
RPI_HOST=192.168.1.50 ./run-all.sh
```

Quick run:
```bash
RPI_HOST=192.168.1.50 DURATION=10 CLIENTS_LIST='1 4' PAYLOADS='1024 65536 1048576' PIPELINES='1 8' ./run-all.sh
```

Default matrix is 216 runs (~108 min at 30 s each). Outputs go under `results/<timestamp>/`.
