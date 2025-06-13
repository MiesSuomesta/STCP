# STCP SDK

**STCP SDK** on staattisesti linkitettävä C-kielinen ohjelmointirajapinta [STCP](https://github.com/MiesSuomesta/STCP) -protokollan käyttöön sulautetussa ympäristössä. Projektissa Rust hoitaa kryptografian ja protokollan ytimen, ja C-rajapinta tarjoaa yksinkertaisen ja tehokkaan tavan käyttää sitä ulkopuolisista ohjelmista.

---

## 🔧 Sisältö

Tämä SDK-paketti sisältää:

```
sdk/
├── include/                # C-headerit
│   ├── stcp_client.h
│   └── stcp_server.h
├── lib/                    # Staattisesti linkitettävät kirjastot
│   ├── libstcpclient.a
│   └── libstcpserver.a
├── examples/               # Client/server-demot C-kielellä
├── install-sdk.sh          # Asennusskripti (oletus: /usr/local)
├── CMakeLists.txt          # Build-tuki (myös testaukselle)
└── README.md               # Tämä tiedosto
```

---

## 🛠️ Asennus

Kloonaa ja rakenna SDK:

```bash
git clone https://github.com/MiesSuomesta/STCP.git
cd STCP/SDK/C
bash tools/setup.sh
bash build-sdk.sh
```

Tuloksena saat:

- `sdk/include/*.h` — Headerit
- `sdk/lib/*.a` — Staattiset kirjastot
- `sdk/install-sdk.sh` — Installi scripti
- `sdk/pkgconfig/*.pc` — Komennolle `pkg-config` tarkoitettu konfiguraatio
- `sdk.zip` — SDK pakattuna valmiiksi siirtoa varten

---


---

## ✅ Symbolitestaus (valinnainen)

Varmistaaksemme, että kirjastot sisältävät kaikki headerissa mainitut symbolit, voit käyttää CMake-testivaihetta:

```bash
cmake -B build -DTESTING=1
cmake --build build
```

---

## 📚 Esimerkit

SDK sisältää yksinkertaiset C-esimerkit:

```bash
cd sdk/examples
make
./client &
./server
```

---

## 💬 Yhteensopivuus

- Linux (x86_64, MUSL)
- Staattisesti linkitettävä, ei tarvitse Rustia ajonaikaisesti
- Riippuvuudet: `libc`, `OpenSSL (libcrypto)` (staattisesti linkitetty)

---

## 🧪 Build it yourself

Projektin voi rakentaa alusta:

```bash
git clone https://github.com/MiesSuomesta/STCP.git
cd ports/dect-nr9161/dect-nr9161-C-STCP-API
./build-sdk.sh
```

---

## 📜 Lisenssi

STCP SDK – Copyright (C) 2025 [Lauri Jakku](mailto:lja@lja.fi)

This program is free software: you can redistribute it and/or modify it  
under the terms of the [GNU General Public License v3.0 or later (GPL-3.0+)](https://www.gnu.org/licenses/gpl-3.0).

---

## 🤝 Osallistuminen

Pull requestit, bugiraportit ja parannusehdotukset ovat lämpimästi tervetulleita!  
Voit aloittaa forkkamalla tämän repopohjan tai luomalla uuden isännöintiprojektin SDK:n pohjalta.

---
