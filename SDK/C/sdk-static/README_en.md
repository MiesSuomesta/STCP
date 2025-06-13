# STCP SDK

The **Secure Tiny Communication Protocol (STCP) SDK** provides a minimal, portable, and secure communication layer built with Rust and C integration. This SDK includes:

- Statically linked libraries
- Clean C header interfaces
- Full CMake integration
- SDK symbol validation
- GPLv3 licensing
- Ready-to-use examples

---

## 🔧 Requirements

- Linux with GCC (tested with GCC 14.2.0)
- CMake ≥ 3.16
- Rust (nightly, MUSL toolchain)
- `musl-cross` toolchain
- `cbindgen` installed via Cargo
- Precompiled OpenSSL with `OPENSSL_NO_GETHOSTBYNAME`

---

## 📦 Build

Clone and build the SDK from scratch:

```bash
git clone https://github.com/MiesSuomesta/STCP.git
cd STCP/ports/dect-nr9161/dect-nr9161-C-STCP-API
bash build-sdk.sh
```

You will get:

- `sdk/include/*.h` — Header files
- `sdk/lib/*.a` — Static libraries
- `sdk/install-sdk.sh` — Installer
- `sdk/pkgconfig/*.pc` — For `pkg-config`
- `sdk.zip` — Compressed distribution

---

## 🧪 SDK Symbol Test

To verify that all C-exposed functions exist in the compiled libraries:

```bash
cmake -B build -DTESTING=1
cmake --build build
```

---

## 🔄 Installing the SDK

By default installs to `/usr/local`. You can customize:

```bash
bash sdk/install-sdk.sh --prefix=/opt/stcp
```

---

## 📚 Examples

Build client and server demo apps:

```bash
cmake -B build
cmake --build build
```

Then run in separate terminals:

```bash
./build/server
./build/client
```

---

## ⚠️ Note on Static Linking

Due to static linking, you may see warnings like:

```
Using 'getaddrinfo' in statically linked applications requires ...
```

These are harmless if you target platforms with correct glibc version, or build for MUSL. This SDK is designed to be fully static for embedded/low-dependency environments.

---

## 📄 License

This project is licensed under **GNU GPLv3**:

```
STCP SDK – Copyright (C) 2025 Lauri Jakku <lja@lja.fi>
```

See `LICENSE.txt` or https://www.gnu.org/licenses/.

---

## 🤝 Contributing

All contributions are welcome! Please send a pull request or reach out if you want to:

- Add new targets (Zephyr, ESP, etc.)
- Improve tests
- Suggest improvements
