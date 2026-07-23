#!/usr/bin/env python3
"""Generate a Raspberry Pi benchmark landing page for TCP and UDP carriers."""
from __future__ import annotations
import argparse, datetime as dt, hashlib, json
from pathlib import Path
from typing import Any


def args_parse():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('output', type=Path)
    p.add_argument('--tcp-page', type=Path)
    p.add_argument('--udp-page', type=Path)
    p.add_argument('--platform', default='Raspberry Pi')
    return p.parse_args()


def read_summary(page: Path|None) -> dict[str, Any]|None:
    if not page: return None
    f=page/'summary.json'
    if not f.is_file(): return None
    return json.loads(f.read_text(encoding='utf-8'))


def card(name:str, slug:str, summary:dict[str,Any]|None)->str:
    if summary is None:
        return f'''<article class="carrier-card unavailable"><div class="eyebrow">{name}</div><h2>Data not generated</h2><p>Run the matching benchmark matrix and regenerate this site.</p></article>'''
    total=summary.get('total_cases',0); passed=summary.get('passed_cases',0); failed=summary.get('failed_cases',0)
    stcp=summary.get('protocols',{}).get('stcp',{})
    rate=stcp.get('pass_percent',0)
    return f'''<article class="carrier-card"><div class="eyebrow">{name}</div><h2>{passed}/{total} cases passed</h2><p>STCP reliability: <strong>{rate:.1f}%</strong> · failed cases: {failed}</p><a class="button" href="./{slug}/">Open {name} dashboard →</a></article>'''


def main():
    a=args_parse(); out=a.output.resolve(); out.mkdir(parents=True,exist_ok=True)
    tcp=read_summary(a.tcp_page); udp=read_summary(a.udp_page)
    generated=dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat()
    html=f'''<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>{a.platform} benchmarks · STCP</title><style>
:root{{--bg:#07111f;--card:#0d1b2d;--line:#23364d;--text:#edf6ff;--muted:#9fb2c8;--accent:#60a5fa}}*{{box-sizing:border-box}}body{{margin:0;background:linear-gradient(145deg,#07111f,#0a1728);color:var(--text);font:16px/1.55 system-ui,sans-serif}}main{{max-width:1120px;margin:auto;padding:72px 24px}}a{{color:inherit}}nav{{display:flex;justify-content:space-between;align-items:center;margin-bottom:64px}}nav a{{text-decoration:none}}h1{{font-size:clamp(2.4rem,7vw,5.6rem);line-height:.95;margin:.2em 0}}.lead{{max-width:760px;color:var(--muted);font-size:1.15rem}}.grid{{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:24px;margin-top:48px}}.carrier-card{{background:rgba(13,27,45,.88);border:1px solid var(--line);border-radius:24px;padding:30px;box-shadow:0 20px 60px rgba(0,0,0,.2)}}.unavailable{{opacity:.65}}.eyebrow{{text-transform:uppercase;letter-spacing:.14em;color:var(--accent);font-weight:700;font-size:.82rem}}h2{{font-size:1.7rem;margin:.45em 0}}p{{color:var(--muted)}}.button{{display:inline-block;margin-top:16px;background:var(--accent);color:#06101d;text-decoration:none;font-weight:800;padding:12px 18px;border-radius:12px}}footer{{margin-top:64px;color:var(--muted);font-size:.9rem}}</style></head><body><main><nav><a href="/">STCP</a><a href="/platforms/raspberry-pi/">Raspberry Pi</a></nav><div class="eyebrow">Reproducible benchmark suite</div><h1>{a.platform}<br>carrier benchmarks</h1><p class="lead">Interactive TCP and UDP carrier dashboards with throughput, latency, CPU, memory, IRQ, perf counters, reliability status and downloadable raw results.</p><section class="grid">{card('TCP carrier','tcp',tcp)}{card('UDP carrier','udp',udp)}</section><footer>Generated {generated}. Comparisons use matched benchmark dimensions; failed STCP cases remain visible.</footer></main></body></html>'''
    (out/'index.html').write_text(html,encoding='utf-8')
    combined={'schema_version':1,'generated_at':generated,'platform':a.platform,'tcp':tcp,'udp':udp}
    (out/'benchmark-index.json').write_text(json.dumps(combined,indent=2)+'\n',encoding='utf-8')
    files=[]
    for f in sorted(out.rglob('*')):
        if f.is_file() and f.name!='manifest.json':
            files.append({'path':str(f.relative_to(out)),'bytes':f.stat().st_size,'sha256':hashlib.sha256(f.read_bytes()).hexdigest()})
    (out/'manifest.json').write_text(json.dumps({'generated_at':generated,'files':files},indent=2)+'\n',encoding='utf-8')
    print(f'[ OK ] Landing page: {out / "index.html"}')
if __name__=='__main__': raise SystemExit(main())
