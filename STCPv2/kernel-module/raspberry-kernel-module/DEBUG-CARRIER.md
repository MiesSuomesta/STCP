# STCP carrier debug build

This build enables verbose carrier diagnostics by default.

Module parameter:

```bash
sudo modprobe stcp carrier_debug=1
# or after loading
echo 1 | sudo tee /sys/module/stcp/parameters/carrier_debug
```

Disable the verbose carrier logs:

```bash
echo 0 | sudo tee /sys/module/stcp/parameters/carrier_debug
```

Useful test sequence on Raspberry Pi:

```bash
sudo dmesg -C
sudo modprobe stcp carrier_debug=1
cd ~/benchmark
bash start-servers.sh
sudo dmesg | grep 'stcp:'
```

Useful test sequence on the x86 client:

```bash
sudo dmesg -C
RPI_HOST=192.168.1.199 DURATION=2 CLIENTS_LIST='1' PAYLOADS='1024' PIPELINES='1' ./run-all.sh
sudo dmesg | grep 'stcp:'
```

The important lines are:

- `bind begin` / `bind complete`: actual carrier address and port
- `listen complete`: TCP state and local port after `kernel_listen()`
- `connect begin`: actual destination passed to `kernel_connect()`
- `kernel_connect result`: exact return code
- `connect failed`: local and remote tuple, socket state and `sk_err`
- `kernel_accept result`: whether the TCP carrier connection reached the listener
