# STCPv2 benchmark report

Generoi uusimmasta Zephyr-tuloshakemistosta:

```bash
bash benchmark/generate-report.sh
```

Tai tietystä tuloshakemistosta:

```bash
python3 benchmark/generate-report.py benchmark/results/zephyr-YYYYMMDD-HHMMSS
```

Valmis raportti kirjoitetaan hakemistoihin:

- `benchmark/site/<result-set>/index.html`
- `benchmark/site/latest/index.html`

Raportissa näkyvät platform, board, carrier/siirtotie, transport, shield, linkkitiedot, fyysinen device chunk, PASS-prosentti sekä parhaat upload-, download- ja full-duplex-tulokset. Suodattimet tukevat platformia, carrieria, transporttia, suuntaa ja mittaria.

Carrier-kohtaiset lisätiedot voidaan antaa `pipeline-summary.json`-tiedostossa esimerkiksi:

```json
{
  "carrier": "ethernet",
  "carrier_info": {
    "link_speed": "100 Mb/s",
    "duplex": "full duplex",
    "chip": "W5500",
    "spi_mhz": 8
  }
}
```

LTE-ajossa vastaava rakenne voi sisältää esimerkiksi operaattorin, RAT:n, bandin, RSRP:n ja RSRQ:n.
