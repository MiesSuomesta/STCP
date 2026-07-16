# Korjaukset

Paketissa on korjattu lokissa näkyneet rajapintavirheet:

1. `rust/src/ffi.rs`: poistettu käyttämätön `set_carrier`-importti.
2. `rust/src/state.rs`: poistettu turha, rekursioriskin sisältävä `StcpContext::set_carrier`; carrier tallennetaan `carrier.rs`:n helperillä.
3. `src/stcp_proto.c`: poistettu `INIT_DELAYED_WORK(..., stcp_retransmit_workfn)`, koska callback ei kuulu tämän tiedoston näkyvyyteen. Work käynnistetään nykyisten `stcp_start_retransmit_work()`-polkujen kautta.
4. `include/stcp_rust_ffi.h`: päivitetty vastaamaan nykyistä Rust ABI:a: `stcp_rust_create(u8, void **) -> int` ja lisätty `stcp_rust_set_carrier`.
5. `patches/stcp_socket.h.patch`: lisää puuttuvan `carrier`-kentän `struct stcp_sock`:iin.

Kopioi paketin tiedostot projektin juureen säilyttäen hakemistot. Jos käytät olemassa olevaa `include/stcp_socket.h`:ta, sovella patch:

    patch -p1 < patches/stcp_socket.h.patch

Tämän jälkeen aja:

    make clean
    make all test

## Follow-up fixes
- Added `int stcp_rust_crypto_selftest(void);` to `include/stcp_rust_ffi.h`.
- Marked the currently file-local `stcp_kernel_should_drop_data()` helper as `static` to satisfy `-Wmissing-prototypes`.

## V3

`apply-fixes.sh` lisää `struct stcp_carrier *carrier;` -kentän oikean projektin
`include/stcp_socket.h`-tiedostoon. Aja projektin juuressa:

```bash
bash apply-fixes.sh .
```
