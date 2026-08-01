# Julkaisu stcp.fi-sivustolle

Julkaisija generoi uusimman raportin, validoi että kaikki tapaukset ovat PASS, luo SHA-256-manifestin ja julkaisee sivun atomisesti. Keskeneräinen upload ei koskaan korvaa toimivaa `latest/`-sivua.

## 1. Asetukset

```bash
cp benchmark/.stcp-publish.env.example benchmark/.stcp-publish.env
nano benchmark/.stcp-publish.env
```

Esimerkki:

```bash
STCP_PUBLISH_HOST=fuji
STCP_PUBLISH_USER=pomo
STCP_PUBLISH_PORT=22
STCP_PUBLISH_REMOTE_DIR=/var/www/stcp.fi/benchmarks/zephyr
STCP_PUBLISH_KEEP_RELEASES=10
```

Etäkäyttäjällä pitää olla kirjoitusoikeus kohdehakemistoon. Apache/Nginx-palvelimen tulee palvella polku esimerkiksi osoitteessa:

```text
https://stcp.fi/benchmarks/zephyr/
```

## 2. Dry-run paikallisesti

```bash
bash benchmark/publish-stcp-fi.sh \
  --local-target /tmp/stcp.fi-zephyr \
  --dry-run
```

Todellinen paikallinen testijulkaisu:

```bash
bash benchmark/publish-stcp-fi.sh \
  --local-target /tmp/stcp.fi-zephyr

firefox /tmp/stcp.fi-zephyr/latest/index.html
```

## 3. Julkaisu palvelimelle

```bash
bash benchmark/publish-stcp-fi.sh
```

Tietyn tulosjoukon julkaisu:

```bash
bash benchmark/publish-stcp-fi.sh \
  --result-dir benchmark/results/zephyr-YYYYMMDD-HHMMSS
```

Suoraan komentoriviargumenteilla:

```bash
bash benchmark/publish-stcp-fi.sh \
  --host fuji \
  --user pomo \
  --remote-dir /var/www/stcp.fi/benchmarks/zephyr
```

## Etärakenne

```text
benchmarks/zephyr/
├── index.html              # ohjaa latest/-sivulle
├── latest/                 # nykyinen julkaisu
└── releases/
    └── zephyr-YYYYMMDD-HHMMSS/
```

Jokaisessa julkaisussa on `publish-manifest.json`, joka sisältää tiedostojen SHA-256-tarkisteet.
