#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, shutil
from pathlib import Path
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parent
RPI_RESULTS = ROOT / 'RaspberryPI/benchmark/results'
ZEPHYR_RESULTS = ROOT / 'zephyr/nordic/stcp-mqtt/benchmark/results'
DEFAULT_OUT = ROOT / 'stcp.fi/benchmarks'
TEMPLATE_DIR = ROOT / 'benchmark-report/site/latest'

def latest_dir(base: Path, prefix: str | None = None) -> Path:
    dirs = [p for p in base.iterdir() if p.is_dir() and (prefix is None or p.name.startswith(prefix))]
    if not dirs:
        raise SystemExit(f'No result directories under {base}')
    return max(dirs, key=lambda p: p.stat().st_mtime)

def read_json(path: Path):
    with path.open('r', encoding='utf-8') as f:
        return json.load(f)

def normalize_rpi(path: Path, run_id: str):
    d = read_json(path)
    mode = str(d.get('mode') or d.get('transport') or path.name.split('-')[0]).lower()
    errors = int(d.get('errors') or 0)
    return {
        'schema_version': 2, 'platform': 'raspberry-pi-4', 'board': 'Raspberry Pi 4',
        'carrier': 'ethernet', 'shield': 'built-in-ethernet', 'link_speed': '1 Gb/s', 'duplex': 'full',
        'transport': mode, 'mode': mode, 'direction': d.get('direction', 'echo'),
        'clients': int(d.get('clients') or 1), 'payload_bytes': int(d.get('payload_bytes') or 0),
        'chunk_bytes': int(d.get('payload_bytes') or 0), 'pipeline': int(d.get('pipeline') or 1),
        'elapsed_s': float(d.get('elapsed_s') or 0), 'operations': int(d.get('operations') or 0),
        'errors': errors, 'status': 0 if errors == 0 else -1,
        'tx_mib_s': float(d.get('tx_mib_s') or 0), 'rx_mib_s': float(d.get('rx_mib_s') or 0),
        'combined_mib_s': float(d.get('combined_mib_s') or 0),
        'operations_s': float(d.get('operations_s') or 0),
        'connect_mean_ms': d.get('connect_mean_ms'), 'rtt_p50_ms': d.get('rtt_p50_ms'),
        'rtt_p95_ms': d.get('rtt_p95_ms'), 'rtt_p99_ms': d.get('rtt_p99_ms'),
        'client_cpu_percent': d.get('client_cpu_percent'), 'max_rss_kib': d.get('max_rss_kib'),
        'run_id': run_id, 'source_file': str(path.relative_to(ROOT)),
    }

def normalize_zephyr(path: Path, run_id: str):
    d = dict(read_json(path))
    d.setdefault('schema_version', 2)
    d.setdefault('platform', 'zephyr-nrf9151')
    d.setdefault('board', 'nrf9151dk/nrf9151/ns')
    d.setdefault('carrier', 'ethernet')
    d.setdefault('shield', 'seeed_w5500')
    d.setdefault('link_speed', '100 Mb/s')
    d.setdefault('duplex', 'full')
    d.setdefault('chunk_bytes', d.get('device_payload_bytes', d.get('payload_bytes', 0)))
    d.setdefault('status', 0 if int(d.get('errors') or 0) == 0 else -1)
    d['run_id'] = run_id
    d['source_file'] = str(path.relative_to(ROOT))
    return d

def copy_tree(src: Path, dst: Path):
    if dst.exists():
        shutil.rmtree(dst)
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(src, dst)

def landing_html(data: dict) -> str:
    cases = data['cases']
    total = len(cases)
    passed = sum(1 for c in cases if int(c.get('errors') or 0) == 0 and int(c.get('status') or 0) == 0)
    carriers = ', '.join(sorted({c.get('carrier', 'unknown') for c in cases}))
    generated = data['generated_utc']
    return f'''<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>STCPv2 Benchmarks</title>
<style>body{{margin:0;background:#07111f;color:#eaf2ff;font:16px system-ui}}main{{max-width:1120px;margin:auto;padding:42px 20px}}h1{{font-size:44px;margin:0 0 8px}}p{{color:#9fb0c8;line-height:1.55}}.meta{{display:flex;gap:10px;flex-wrap:wrap;margin:20px 0}}.pill{{background:#14243a;border:1px solid #2a405f;border-radius:999px;padding:7px 12px}}.cards{{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:18px;margin:30px 0}}a.card{{display:block;text-decoration:none;color:inherit;background:#0d1a2b;border:1px solid #263750;border-radius:16px;padding:22px;transition:.15s}}a.card:hover{{border-color:#55d6ff;transform:translateY(-2px)}}.label{{color:#55d6ff;font-weight:750}}footer{{margin-top:34px;color:#71839d}}</style></head><body><main>
<h1>STCPv2 Benchmarks</h1><p>Unified Raspberry Pi and Zephyr results generated with one normalized schema and one report engine.</p>
<div class="meta"><span class="pill">{passed}/{total} PASS</span><span class="pill">2 platforms</span><span class="pill">Carriers: {carriers}</span></div>
<div class="cards">
<a class="card" href="latest/index.html"><div class="label">Combined comparison</div><h2>All platforms</h2><p>Compare Raspberry Pi and Zephyr in the same interactive report.</p></a>
<a class="card" href="latest/raspberry-pi.html"><div class="label">Platform results</div><h2>Raspberry Pi</h2><p>Open Raspberry Pi TCP, TLS and STCP benchmark results.</p></a>
<a class="card" href="latest/zephyr.html"><div class="label">Platform results</div><h2>Zephyr nRF9151</h2><p>Open nRF9151 DK results with Ethernet or LTE carrier metadata.</p></a>
</div><footer>Generated {generated}. The existing stcp.fi home page is not modified.</footer></main></body></html>'''

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--rpi-run')
    ap.add_argument('--zephyr-run')
    ap.add_argument('--output', default=str(DEFAULT_OUT))
    args = ap.parse_args()
    rpi = Path(args.rpi_run) if args.rpi_run else latest_dir(RPI_RESULTS)
    zep = Path(args.zephyr_run) if args.zephyr_run else latest_dir(ZEPHYR_RESULTS, 'zephyr-')
    out = Path(args.output)
    latest = out / 'latest'
    latest.mkdir(parents=True, exist_ok=True)
    cases = [normalize_rpi(p, rpi.name) for p in sorted(rpi.glob('*.json'))]
    cases.extend(normalize_zephyr(p, zep.name) for p in sorted(zep.rglob('*.json')) if p.name != 'pipeline-summary.json')
    data = {
        'schema_version': 3,
        'generated_utc': datetime.now(timezone.utc).isoformat(),
        'runs': [
            {'platform': 'raspberry-pi-4', 'run_id': rpi.name, 'path': str(rpi.relative_to(ROOT)), 'cases': sum(c['platform'] == 'raspberry-pi-4' for c in cases)},
            {'platform': 'zephyr-nrf9151', 'run_id': zep.name, 'path': str(zep.relative_to(ROOT)), 'cases': sum(c['platform'] == 'zephyr-nrf9151' for c in cases)},
        ],
        'cases': cases,
    }
    for name in ('index.html', 'raspberry-pi.html', 'zephyr.html'):
        shutil.copy2(TEMPLATE_DIR / name, latest / name)
    (latest / 'data.js').write_text('window.BENCHMARK_DATA=' + json.dumps(data, separators=(',', ':')) + ';\n', encoding='utf-8')
    copy_tree(rpi, latest / 'raw/raspberry-pi-4' / rpi.name)
    copy_tree(zep, latest / 'raw/zephyr-nrf9151' / zep.name)
    out.mkdir(parents=True, exist_ok=True)
    (out / 'index.html').write_text(landing_html(data), encoding='utf-8')
    passed = sum(1 for c in cases if int(c.get('errors') or 0) == 0 and int(c.get('status') or 0) == 0)
    print(f'[OK] Generated {out}')
    print(f'[INFO] Cases={len(cases)} passed={passed} failed={len(cases)-passed}')

if __name__ == '__main__':
    main()
