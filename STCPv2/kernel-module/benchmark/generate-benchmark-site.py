#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import shutil
from collections import Counter, defaultdict
from pathlib import Path
from statistics import mean
from typing import Any

KINDS_BY_CARRIER = {"tcp": ("tcp", "tls", "stcp-tcp"), "udp": ("udp", "stcp-udp")}
LABELS = {"tcp": "TCP", "tls": "TLS", "stcp-tcp": "STCP/TCP", "udp": "UDP", "stcp-udp": "STCP/UDP"}
CASE_KEYS = ("clients", "payload_bytes", "pipeline")
METRICS = (
    "combined_mib_s", "operations_s", "rtt_p50_ms", "rtt_p95_ms", "rtt_p99_ms",
    "client_cpu_percent", "connect_mean_ms", "udp_retransmits", "udp_timeouts",
    "udp_duplicates", "udp_stale", "udp_retransmit_percent",
)


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
        return value if isinstance(value, dict) else {}
    except (OSError, json.JSONDecodeError):
        return {}


def case_kind(path: Path, data: dict[str, Any]) -> str:
    for kind in ("stcp-tcp", "stcp-udp", "tcp", "tls", "udp"):
        if path.stem.startswith(kind + "-"):
            return kind
    mode, transport = str(data.get("mode", "")), str(data.get("transport", ""))
    return f"stcp-{transport}" if mode == "stcp" else mode


def number(data: dict[str, Any], key: str) -> float | None:
    try:
        return float(data[key])
    except (KeyError, TypeError, ValueError):
        return None


def integer(data: dict[str, Any], key: str) -> int | None:
    value = number(data, key)
    return int(value) if value is not None else None


def discover(result_dir: Path, carrier: str) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    source = result_dir / carrier
    if not source.is_dir():
        return rows
    for path in sorted(source.glob("*.json")):
        if path.name.startswith("."):
            continue
        data = load_json(path)
        kind = case_kind(path, data)
        if kind not in KINDS_BY_CARRIER[carrier]:
            continue
        errors = integer(data, "errors") or 0
        operations = integer(data, "operations") or 0
        details = data.get("error_details") or []
        passed = errors == 0 and operations > 0 and not details
        row: dict[str, Any] = {
            "file": path.name, "protocol": kind, "protocol_label": LABELS.get(kind, kind),
            "carrier": carrier, "clients": integer(data, "clients"),
            "payload_bytes": integer(data, "payload_bytes"), "pipeline": integer(data, "pipeline"),
            "elapsed_s": number(data, "elapsed_s"), "operations": operations, "errors": errors,
            "status": "PASS" if passed else "FAIL",
        }
        for metric in METRICS:
            row[metric] = number(data, metric)
        rows.append(row)
    return rows


def most_common_duration(rows: list[dict[str, Any]], pipeline_summary: dict[str, Any]) -> float | None:
    try:
        return float(pipeline_summary.get("configured_case_duration_s"))
    except (TypeError, ValueError):
        rounded = [round(float(r["elapsed_s"])) for r in rows if r.get("elapsed_s") is not None]
        return float(Counter(rounded).most_common(1)[0][0]) if rounded else None


def aggregate(rows: list[dict[str, Any]]) -> dict[str, Any]:
    order = list(LABELS)
    protocols = sorted({r["protocol"] for r in rows}, key=lambda p: order.index(p))
    dimensions = {k: sorted({r[k] for r in rows if r[k] is not None}) for k in CASE_KEYS}
    averages: dict[str, dict[str, float | None]] = {}
    for protocol in protocols:
        selected = [r for r in rows if r["protocol"] == protocol and r["status"] == "PASS"]
        averages[protocol] = {}
        for metric in METRICS:
            values = [r[metric] for r in selected if r.get(metric) is not None]
            averages[protocol][metric] = mean(values) if values else None
    comparison_rows = []
    grouped: dict[tuple[int, int, int], dict[str, dict[str, Any]]] = defaultdict(dict)
    for row in rows:
        if all(row.get(k) is not None for k in CASE_KEYS):
            grouped[tuple(int(row[k]) for k in CASE_KEYS)][row["protocol"]] = row
    for (clients, payload, pipeline), by_protocol in sorted(grouped.items()):
        item: dict[str, Any] = {"clients": clients, "payload_bytes": payload, "pipeline": pipeline}
        for protocol, row in by_protocol.items():
            item[protocol] = {metric: row.get(metric) for metric in METRICS}
        comparison_rows.append(item)
    return {
        "protocols": [{"id": p, "label": LABELS[p]} for p in protocols],
        "dimensions": dimensions, "averages": averages, "comparisons": comparison_rows,
    }


def write_csv(path: Path, rows: list[dict[str, Any]]) -> None:
    fields = ["protocol", "clients", "payload_bytes", "pipeline", "elapsed_s", "status", "operations", "errors", *METRICS, "file"]
    with path.open("w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields, extrasaction="ignore")
        writer.writeheader(); writer.writerows(rows)


def html_page(carrier: str, case_duration: float | None) -> str:
    title = f"Raspberry Pi {carrier.upper()} benchmark"
    duration_text = "unknown" if case_duration is None else f"{case_duration:g} seconds per case"
    return f"""<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>{title} · STCP</title><link rel="stylesheet" href="assets/dashboard.css"></head><body>
<header class="hero"><div class="wrap"><p class="eyebrow">STCP · Raspberry Pi 4 · {carrier.upper()}</p><h1>{title}</h1>
<p class="lead">Protocol comparison across client counts, payload sizes and pipeline depths.</p>
<div class="hero-badges"><span id="case-count">Loading cases…</span><span>Case runtime: <strong id="case-runtime">{duration_text}</strong></span><span id="run-status">Loading status…</span></div></div></header>
<main class="wrap"><section class="summary-grid" id="summary-grid"></section>
<section class="panel controls sticky-controls"><div><label for="clients">Fixed clients</label><select id="clients"></select></div><div><label for="payload_bytes">Fixed payload</label><select id="payload_bytes"></select></div><div><label for="pipeline">Fixed pipeline</label><select id="pipeline"></select></div><div><label for="protocol-filter">Protocols</label><select id="protocol-filter"><option value="all">All protocols</option></select></div></section>
<section class="panel"><div class="section-heading"><div><p class="eyebrow">Dynamic comparison</p><h2>Automatically updating bar charts</h2></div><p>Every chart follows the fixed selections above; the named dimension is varied.</p></div><div id="chart-groups"></div></section>
<section class="panel"><div class="section-heading"><div><p class="eyebrow">Parameter map</p><h2>Throughput heatmap</h2></div><p id="heatmap-note"></p></div><div id="heatmap-meta" class="chart-meta"></div><div id="heatmap-legend" class="chart-legend"><span class="legend-item"><i class="legend-swatch" style="background:var(--c1)"></i>Higher colour intensity = higher combined throughput</span></div><div id="heatmap" class="heatmap-wrap"></div><div class="chart-caption"><span>X: Payload size</span><span>Y: Client count</span><span>Cells: MiB/s</span></div></section>
<section class="panel"><div class="section-heading"><div><p class="eyebrow">Scalability</p><h2>Client scaling efficiency</h2></div><p>Throughput relative to one client for the selected payload and pipeline.</p></div><div id="scaling-meta" class="chart-meta"></div><div id="scaling-legend"></div><div id="scaling-chart" class="chart"></div><div class="chart-caption"><span>X: Client count</span><span>Y: Relative throughput (%)</span></div></section>
<section class="panel"><div class="section-heading"><div><p class="eyebrow">Best results</p><h2>Best observed result by protocol</h2></div></div><div id="best-results" class="best-grid"></div></section>
<section class="panel"><div class="section-heading"><div><p class="eyebrow">Exact values</p><h2>Selected tuple comparison</h2></div></div><div class="table-wrap"><table id="comparison-table"><thead></thead><tbody></tbody></table></div></section>
<section class="panel run-summary"><div class="section-heading"><div><p class="eyebrow">Pipeline metadata</p><h2>Benchmark run summary</h2></div></div><div id="run-summary-grid" class="summary-grid compact"></div></section>
<nav class="raw-links"><a href="dashboard-data.json">Dashboard JSON</a><a href="summary.json">Summary JSON</a><a href="cases.csv">Cases CSV</a><a href="report.md">Markdown report</a><a href="manifest.json">SHA-256 manifest</a><a href="raw/">Raw JSON files</a></nav></main>
<script>window.STCP_CARRIER={json.dumps(carrier)};</script><script src="assets/dashboard.js"></script></body></html>
"""

CSS = r'''
:root{color-scheme:dark;--panel:#0f172a;--panel2:#111827;--line:#263449;--text:#e5eefb;--muted:#94a3b8;--accent:#38bdf8;--good:#4ade80;--bad:#fb7185;--c1:#38bdf8;--c2:#a78bfa;--c3:#f59e0b;--c4:#4ade80;--c5:#fb7185}*{box-sizing:border-box}body{margin:0;background:linear-gradient(180deg,#07111f 0,#020617 420px);color:var(--text);font:15px/1.5 system-ui,-apple-system,sans-serif}.wrap{width:min(1380px,calc(100% - 2rem));margin:auto}.hero{padding:3rem 0 2rem;border-bottom:1px solid var(--line)}h1{font-size:clamp(2rem,5vw,4rem);line-height:1;margin:.3rem 0 1rem}h2{font-size:1.2rem;margin:.2rem 0}h3{font-size:1rem;margin:0}.lead,.section-heading p,.chart-card p{color:var(--muted)}.eyebrow{margin:0;text-transform:uppercase;letter-spacing:.12em;font-size:.72rem;color:var(--accent);font-weight:700}.hero-badges,.raw-links{display:flex;gap:.65rem;flex-wrap:wrap}.hero-badges span,.raw-links a{border:1px solid var(--line);background:rgba(15,23,42,.8);border-radius:999px;padding:.45rem .75rem;color:var(--text);text-decoration:none}.hero-badges strong{color:var(--accent)}main{padding:1.5rem 0 4rem}.panel{background:rgba(15,23,42,.92);border:1px solid var(--line);border-radius:16px;padding:1rem;margin-bottom:1rem}.summary-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(155px,1fr));gap:.75rem;margin-bottom:1rem}.summary-grid.compact{margin:0}.stat,.best-card{background:var(--panel2);border:1px solid var(--line);border-radius:12px;padding:.8rem}.stat small,.best-card small{color:var(--muted);display:block;text-transform:uppercase;letter-spacing:.06em;font-size:.68rem}.stat strong,.best-card strong{display:block;margin-top:.25rem;font-size:1.15rem}.controls{display:grid;grid-template-columns:repeat(auto-fit,minmax(170px,1fr));gap:.75rem}.sticky-controls{position:sticky;top:.5rem;z-index:10;box-shadow:0 10px 35px rgba(0,0,0,.28)}.controls label{display:block;color:var(--muted);font-size:.75rem;margin-bottom:.25rem}.controls select{width:100%;background:#07111f;color:var(--text);border:1px solid var(--line);border-radius:9px;padding:.65rem}.section-heading{display:flex;justify-content:space-between;align-items:end;gap:1rem;margin-bottom:.9rem}.chart-family{margin-top:1.5rem}.chart-family-title{border-bottom:1px solid var(--line);padding-bottom:.45rem;margin-bottom:.75rem}.chart-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:1rem}.chart-card{background:#091321;border:1px solid var(--line);border-radius:13px;padding:.8rem;min-width:0}.chart-card p{font-size:.78rem;margin:.2rem 0 0}.chart-meta{display:flex;gap:.35rem .7rem;flex-wrap:wrap;margin:.45rem 0 .2rem;color:var(--muted);font-size:.72rem}.chart-meta span{white-space:nowrap}.chart-legend{display:flex;flex-wrap:wrap;gap:.45rem .85rem;margin:.55rem 0 .15rem;padding:.45rem .55rem;border:1px solid var(--line);border-radius:9px;background:rgba(15,23,42,.55)}.legend-item{display:inline-flex;align-items:center;gap:.38rem;color:var(--text);font-size:.75rem;font-weight:650}.legend-swatch{width:.78rem;height:.78rem;border-radius:3px;display:inline-block;flex:0 0 auto}.chart-caption{display:flex;justify-content:space-between;gap:.6rem;flex-wrap:wrap;margin-top:.35rem;color:var(--muted);font-size:.7rem}.bar-chart{width:100%;min-height:300px;margin-top:.35rem}.chart{width:100%;min-height:360px}.bar-chart svg,.chart svg{display:block;width:100%;height:auto}.table-wrap,.heatmap-wrap{overflow:auto}table{width:100%;border-collapse:collapse;white-space:nowrap}th,td{text-align:right;padding:.6rem;border-bottom:1px solid var(--line)}th:first-child,td:first-child{text-align:left}th{color:var(--muted);font-size:.72rem;text-transform:uppercase}.heatmap-table th,.heatmap-table td{text-align:center;min-width:88px}.heat-cell{border-radius:7px;font-weight:700;color:#fff;text-shadow:0 1px 1px rgba(0,0,0,.65)}.best-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(190px,1fr));gap:.75rem}.best-card .protocol{color:var(--accent);font-weight:700;margin-bottom:.4rem}.best-card dl{margin:.4rem 0 0}.best-card dt{color:var(--muted);font-size:.72rem}.best-card dd{margin:0 0 .45rem}.raw-links{margin-top:1rem}.good{color:var(--good)}.bad{color:var(--bad)}.empty{padding:2rem;text-align:center;color:var(--muted)}@media(max-width:850px){.chart-grid{grid-template-columns:1fr}.section-heading{align-items:start;flex-direction:column}.sticky-controls{position:static}.bar-chart{min-height:260px}}
'''

JS = r'''
const METRICS={combined_mib_s:{label:'Combined throughput',unit:'MiB/s',digits:2,family:'Throughput'},operations_s:{label:'Operations',unit:'ops/s',digits:1,family:'Operations'},rtt_p50_ms:{label:'RTT p50',unit:'ms',digits:2,family:'Latency'},rtt_p95_ms:{label:'RTT p95',unit:'ms',digits:2,family:'Latency'},rtt_p99_ms:{label:'RTT p99',unit:'ms',digits:2,family:'Latency'},client_cpu_percent:{label:'Client CPU',unit:'%',digits:1,family:'CPU'},connect_mean_ms:{label:'Connect mean',unit:'ms',digits:2,family:'Connection'},errors:{label:'Errors',unit:'',digits:0,family:'Reliability'},udp_retransmits:{label:'UDP retransmits',unit:'',digits:0,family:'Reliability'},udp_timeouts:{label:'UDP timeouts',unit:'',digits:0,family:'Reliability'},udp_duplicates:{label:'UDP duplicates',unit:'',digits:0,family:'Reliability'},udp_stale:{label:'UDP stale packets',unit:'',digits:0,family:'Reliability'},udp_retransmit_percent:{label:'UDP retransmit rate',unit:'%',digits:2,family:'Reliability'}};
const DIMENSIONS={payload_bytes:'Payload size',clients:'Client count',pipeline:'Pipeline depth'},COLORS=['var(--c1)','var(--c2)','var(--c3)','var(--c4)','var(--c5)'];let DATA,SUMMARY;const $=id=>document.getElementById(id);const payload=v=>v>=1048576?`${v/1048576} MiB`:v>=1024?`${v/1024} KiB`:`${v} B`;const dimLabel=(k,v)=>k==='payload_bytes'?payload(v):String(v);const fmt=(v,m)=>v==null||!Number.isFinite(Number(v))?'–':`${Number(v).toFixed(METRICS[m]?.digits??2)}${METRICS[m]?.unit?` ${METRICS[m].unit}`:''}`;const protocols=()=>$('protocol-filter').value==='all'?DATA.protocols:DATA.protocols.filter(p=>p.id===$('protocol-filter').value);const fixed=k=>Number($(k).value);
function rowsForDimension(xdim){return DATA.cases.filter(r=>r.status==='PASS'&&protocols().some(p=>p.id===r.protocol)&&Object.keys(DIMENSIONS).every(k=>k===xdim||Number(r[k])===fixed(k)))}
function legendHTML(series){return `<div class="chart-legend" aria-label="Chart legend">${series.map((s,i)=>`<span class="legend-item"><i class="legend-swatch" style="background:${COLORS[i%COLORS.length]}"></i>${s.label}</span>`).join('')}</div>`}
function chartMetaHTML(xdim){const parts=Object.keys(DIMENSIONS).filter(k=>k!==xdim).map(k=>`${DIMENSIONS[k]}: <strong>${dimLabel(k,fixed(k))}</strong>`);parts.push(`Case runtime: <strong>${SUMMARY.configured_case_duration_s==null?'–':`${SUMMARY.configured_case_duration_s} s`}</strong>`);parts.push(`Run: <strong>${SUMMARY.run_id||'–'}</strong>`);return `<div class="chart-meta">${parts.map(x=>`<span>${x}</span>`).join('')}</div>`}
function groupedBarChart(target,series,labels,metric,height=310){const width=760,pad={l:68,r:18,t:20,b:66},all=series.flatMap(s=>s.values.filter(Number.isFinite));if(!all.length){target.innerHTML='<div class="empty">No data for this selection</div>';return}const max=Math.max(...all,1)*1.12,iw=width-pad.l-pad.r,ih=height-pad.t-pad.b,groupW=iw/Math.max(labels.length,1),barGap=3,barW=Math.max(3,Math.min(34,(groupW-10)/Math.max(series.length,1)-barGap)),y=v=>pad.t+ih-v/max*ih;let out=`<svg viewBox="0 0 ${width} ${height}" role="img">`;for(let i=0;i<=5;i++){const yy=pad.t+ih*i/5,val=max*(1-i/5);out+=`<line x1="${pad.l}" y1="${yy}" x2="${width-pad.r}" y2="${yy}" stroke="var(--line)"/><text x="${pad.l-8}" y="${yy+4}" text-anchor="end" fill="var(--muted)" font-size="11">${val.toFixed(max<10?1:0)}</text>`}labels.forEach((l,li)=>{const gx=pad.l+li*groupW+groupW/2;out+=`<text x="${gx}" y="${height-28}" text-anchor="middle" fill="var(--muted)" font-size="11">${l}</text>`;series.forEach((s,si)=>{const v=s.values[li];if(!Number.isFinite(v))return;const total=series.length*(barW+barGap)-barGap,x=gx-total/2+si*(barW+barGap),yy=y(v);out+=`<rect x="${x}" y="${yy}" width="${barW}" height="${Math.max(pad.t+ih-yy,1)}" rx="3" fill="${COLORS[si%COLORS.length]}"><title>${s.label} · ${l}: ${fmt(v,metric)}</title></rect>`})});out+=`<text x="17" y="${height/2}" transform="rotate(-90 17 ${height/2})" text-anchor="middle" fill="var(--muted)" font-size="11">${METRICS[metric].unit||METRICS[metric].label}</text></svg>`;target.innerHTML=out}
function lineChart(target,series,labels){const width=1100,height=360,pad={l:72,r:24,t:25,b:65},vals=series.flatMap(s=>s.values.filter(Number.isFinite));if(!vals.length){target.innerHTML='<div class="empty">No data for this selection</div>';return}const max=Math.max(...vals,100)*1.12,iw=width-pad.l-pad.r,ih=height-pad.t-pad.b,x=i=>pad.l+(labels.length===1?iw/2:i*iw/(labels.length-1)),y=v=>pad.t+ih-v/max*ih;let out=`<svg viewBox="0 0 ${width} ${height}">`;for(let i=0;i<=5;i++){const yy=pad.t+ih*i/5,val=max*(1-i/5);out+=`<line x1="${pad.l}" y1="${yy}" x2="${width-pad.r}" y2="${yy}" stroke="var(--line)"/><text x="${pad.l-10}" y="${yy+4}" text-anchor="end" fill="var(--muted)" font-size="12">${val.toFixed(0)}%</text>`}labels.forEach((l,i)=>out+=`<text x="${x(i)}" y="${height-28}" text-anchor="middle" fill="var(--muted)" font-size="12">${l}</text>`);series.forEach((s,si)=>{const pts=s.values.map((v,i)=>Number.isFinite(v)?`${x(i)},${y(v)}`:null).filter(Boolean);if(pts.length)out+=`<polyline points="${pts.join(' ')}" fill="none" stroke="${COLORS[si%COLORS.length]}" stroke-width="3"/>`;s.values.forEach((v,i)=>{if(Number.isFinite(v))out+=`<circle cx="${x(i)}" cy="${y(v)}" r="4" fill="${COLORS[si%COLORS.length]}"><title>${s.label}: ${v.toFixed(1)}%</title></circle>`})});out+='</svg>';target.innerHTML=out}
function chartSeries(xdim,metric){const rows=rowsForDimension(xdim),xs=[...new Set(rows.map(r=>Number(r[xdim])))].sort((a,b)=>a-b);return {xs,series:protocols().map(p=>({label:p.label,values:xs.map(x=>{const r=rows.find(q=>q.protocol===p.id&&Number(q[xdim])===x);return r&&r[metric]!=null?Number(r[metric]):NaN})}))}}
function makeCharts(){const families=['Throughput','Operations','Latency','Connection','CPU','Reliability'],container=$('chart-groups');container.innerHTML='';families.forEach(family=>{const metrics=Object.entries(METRICS).filter(([,m])=>m.family===family).map(([k])=>k).filter(k=>DATA.cases.some(r=>r[k]!=null));if(!metrics.length)return;const section=document.createElement('section');section.className='chart-family';section.innerHTML=`<h2 class="chart-family-title">${family}</h2><div class="chart-grid"></div>`;const grid=section.querySelector('.chart-grid');metrics.forEach(metric=>Object.keys(DIMENSIONS).forEach(xdim=>{const {xs,series}=chartSeries(xdim,metric);const card=document.createElement('article');card.className='chart-card';card.innerHTML=`<h3>${METRICS[metric].label} by ${DIMENSIONS[xdim]}</h3>${chartMetaHTML(xdim)}${legendHTML(series)}<div class="bar-chart"></div><div class="chart-caption"><span>X: ${DIMENSIONS[xdim]}</span><span>Y: ${METRICS[metric].unit||METRICS[metric].label}</span></div>`;grid.appendChild(card);groupedBarChart(card.querySelector('.bar-chart'),series,xs.map(v=>dimLabel(xdim,v)),metric)}));container.appendChild(section)})}
function controls(){Object.keys(DIMENSIONS).forEach(key=>{const el=$(key);DATA.dimensions[key].forEach(v=>el.add(new Option(dimLabel(key,v),v)));el.value=DATA.dimensions[key][0]});$('payload_bytes').value=DATA.dimensions.payload_bytes[Math.floor(DATA.dimensions.payload_bytes.length/2)];$('pipeline').value=DATA.dimensions.pipeline.at(-1);DATA.protocols.forEach(p=>$('protocol-filter').add(new Option(p.label,p.id)));['clients','payload_bytes','pipeline','protocol-filter'].forEach(id=>$(id).addEventListener('change',renderAll))}
function renderHeatmap(){const protocol=protocols()[0],pipeline=fixed('pipeline'),clients=DATA.dimensions.clients,payloads=DATA.dimensions.payload_bytes,rows=DATA.cases.filter(r=>r.status==='PASS'&&r.protocol===protocol.id&&Number(r.pipeline)===pipeline),vals=rows.map(r=>Number(r.combined_mib_s)).filter(Number.isFinite),max=Math.max(...vals,1);let html='<table class="heatmap-table"><thead><tr><th>Clients / Payload</th>'+payloads.map(p=>`<th>${payload(p)}</th>`).join('')+'</tr></thead><tbody>';clients.forEach(c=>{html+=`<tr><th>${c}</th>`;payloads.forEach(p=>{const r=rows.find(x=>Number(x.clients)===c&&Number(x.payload_bytes)===p),v=r?.combined_mib_s,alpha=v==null?.08:.18+.72*Number(v)/max;html+=`<td class="heat-cell" style="background:rgba(56,189,248,${alpha})" title="${protocol.label} · clients ${c} · payload ${payload(p)} · pipeline ${pipeline}: ${v==null?'no data':Number(v).toFixed(2)+' MiB/s'}">${v==null?'–':Number(v).toFixed(1)}</td>`});html+='</tr>'});$('heatmap').innerHTML=html+'</tbody></table>';$('heatmap-note').textContent=`${protocol.label} · pipeline ${pipeline} · MiB/s`;$('heatmap-meta').innerHTML=`<span>Protocol: <strong>${protocol.label}</strong></span><span>Pipeline depth: <strong>${pipeline}</strong></span><span>Case runtime: <strong>${SUMMARY.configured_case_duration_s==null?'–':SUMMARY.configured_case_duration_s+' s'}</strong></span><span>Run: <strong>${SUMMARY.run_id||'–'}</strong></span>`}
function renderScaling(){const pv=fixed('payload_bytes'),q=fixed('pipeline'),clients=DATA.dimensions.clients,series=protocols().map(p=>{const rows=DATA.cases.filter(r=>r.status==='PASS'&&r.protocol===p.id&&Number(r.payload_bytes)===pv&&Number(r.pipeline)===q),base=Number(rows.find(r=>Number(r.clients)===clients[0])?.combined_mib_s);return {label:p.label,values:clients.map(c=>{const v=Number(rows.find(r=>Number(r.clients)===c)?.combined_mib_s);return Number.isFinite(v)&&base>0?v/base*100:NaN})}});$('scaling-meta').innerHTML=`<span>Payload size: <strong>${payload(pv)}</strong></span><span>Pipeline depth: <strong>${q}</strong></span><span>Case runtime: <strong>${SUMMARY.configured_case_duration_s==null?'–':SUMMARY.configured_case_duration_s+' s'}</strong></span><span>Run: <strong>${SUMMARY.run_id||'–'}</strong></span>`;$('scaling-legend').innerHTML=legendHTML(series);lineChart($('scaling-chart'),series,clients.map(String))}
function renderBest(){const tuple=r=>r?`c${r.clients} · ${payload(r.payload_bytes)} · q${r.pipeline}`:'–';$('best-results').innerHTML=protocols().map(p=>{const rows=DATA.cases.filter(r=>r.status==='PASS'&&r.protocol===p.id),max=m=>rows.filter(r=>r[m]!=null).sort((a,b)=>Number(b[m])-Number(a[m]))[0],min=m=>rows.filter(r=>r[m]!=null).sort((a,b)=>Number(a[m])-Number(b[m]))[0],tp=max('combined_mib_s'),ops=max('operations_s'),lat=min('rtt_p95_ms');return `<article class="best-card"><div class="protocol">${p.label}</div><dl><dt>Best throughput</dt><dd><strong>${fmt(tp?.combined_mib_s,'combined_mib_s')}</strong><small>${tuple(tp)}</small></dd><dt>Best operations</dt><dd><strong>${fmt(ops?.operations_s,'operations_s')}</strong><small>${tuple(ops)}</small></dd><dt>Lowest RTT p95</dt><dd><strong>${fmt(lat?.rtt_p95_ms,'rtt_p95_ms')}</strong><small>${tuple(lat)}</small></dd></dl></article>`}).join('')}
function renderTable(){const rows=DATA.cases.filter(r=>Number(r.clients)===fixed('clients')&&Number(r.payload_bytes)===fixed('payload_bytes')&&Number(r.pipeline)===fixed('pipeline')&&protocols().some(p=>p.id===r.protocol)),metrics=['combined_mib_s','operations_s','rtt_p50_ms','rtt_p95_ms','rtt_p99_ms','client_cpu_percent','connect_mean_ms','errors'],t=$('comparison-table');t.tHead.innerHTML=`<tr><th>Protocol</th>${metrics.map(m=>`<th>${METRICS[m].label}</th>`).join('')}</tr>`;t.tBodies[0].innerHTML=rows.map(r=>`<tr><td>${r.protocol_label}</td>${metrics.map(m=>`<td>${fmt(r[m],m)}</td>`).join('')}</tr>`).join('')||'<tr><td colspan="9">No matching tuple</td></tr>'}
function summary(){const pass=DATA.cases.filter(r=>r.status==='PASS').length,fail=DATA.cases.length-pass;$('summary-grid').innerHTML=[['Protocols',DATA.protocols.map(p=>p.label).join(' / ')],['Cases',DATA.cases.length],['Passed',pass],['Failed',fail],['Payloads',DATA.dimensions.payload_bytes.length],['Case runtime',SUMMARY.configured_case_duration_s==null?'–':`${SUMMARY.configured_case_duration_s} s`]].map(([a,b])=>`<div class="stat"><small>${a}</small><strong>${b}</strong></div>`).join('');$('case-count').textContent=`${DATA.cases.length} benchmark cases`;$('run-status').innerHTML=`<strong class="${fail?'bad':'good'}">${fail?'FAILURES '+fail:'ALL CASES PASSED'}</strong>`;if(SUMMARY.configured_case_duration_s!=null)$('case-runtime').textContent=`${SUMMARY.configured_case_duration_s} seconds per case`;const fields=[['Run',SUMMARY.run_id],['Started',SUMMARY.started_at?new Date(SUMMARY.started_at).toLocaleString():'–'],['Finished',SUMMARY.finished_at?new Date(SUMMARY.finished_at).toLocaleString():'–'],['Total duration',SUMMARY.total_wall_duration_human||'–'],['Configured per case',SUMMARY.configured_case_duration_s==null?'–':`${SUMMARY.configured_case_duration_s} s`],['Average measured case',SUMMARY.average_case_wall_duration_s==null?'–':`${Number(SUMMARY.average_case_wall_duration_s).toFixed(2)} s`],['Progress',`${SUMMARY.case_count_completed}/${SUMMARY.case_count_expected}`],['Result',`${String(SUMMARY.status||'unknown').toUpperCase()} · PASS ${SUMMARY.passed||0} · FAIL ${SUMMARY.failed||0}`]];$('run-summary-grid').innerHTML=fields.map(([a,b])=>`<div class="stat"><small>${a}</small><strong>${b}</strong></div>`).join('')}
function renderAll(){makeCharts();renderHeatmap();renderScaling();renderBest();renderTable()}
Promise.all([fetch('dashboard-data.json',{cache:'no-store'}).then(r=>{if(!r.ok)throw Error(r.status);return r.json()}),fetch('pipeline-summary.json',{cache:'no-store'}).then(r=>{if(!r.ok)throw Error(r.status);return r.json()})]).then(([d,s])=>{DATA=d;SUMMARY=s;controls();summary();renderAll()}).catch(e=>{console.error(e);$('run-status').textContent='Dashboard data unavailable'})
'''


def build_one(result_dir: Path, output_dir: Path, carrier: str) -> None:
    rows = discover(result_dir, carrier)
    if not rows:
        raise SystemExit(f"No {carrier} benchmark JSON files found in {result_dir}")
    output_dir.mkdir(parents=True, exist_ok=True); (output_dir / "assets").mkdir(exist_ok=True)
    raw = output_dir / "raw"; raw.mkdir(exist_ok=True)
    for row in rows:
        shutil.copy2(result_dir / carrier / row["file"], raw / row["file"])
    pipeline = load_json(result_dir / "pipeline-summary.json")
    duration = most_common_duration(rows, pipeline)
    if duration is not None: pipeline["configured_case_duration_s"] = duration
    pipeline.update({"carrier_view": carrier, "case_count_completed": len(rows), "case_count_expected": len(rows),
                     "passed": sum(r["status"] == "PASS" for r in rows), "failed": sum(r["status"] == "FAIL" for r in rows)})
    pipeline["status"] = "complete" if pipeline["failed"] == 0 else "failed"
    agg = aggregate(rows)
    dashboard = {"schema_version": 3, "carrier": carrier, "case_duration_s": duration, "cases": rows, **agg}
    summary = {"carrier": carrier, "total_cases": len(rows), "passed_cases": pipeline["passed"], "failed_cases": pipeline["failed"],
               "total_errors": sum(r["errors"] for r in rows), "protocols": agg["protocols"], "dimensions": agg["dimensions"],
               "averages": agg["averages"], "configured_case_duration_s": duration}
    (output_dir / "dashboard-data.json").write_text(json.dumps(dashboard, indent=2) + "\n")
    (output_dir / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    (output_dir / "pipeline-summary.json").write_text(json.dumps(pipeline, indent=2) + "\n")
    write_csv(output_dir / "cases.csv", rows)
    (output_dir / "assets" / "dashboard.css").write_text(CSS.strip() + "\n")
    (output_dir / "assets" / "dashboard.js").write_text(JS.strip() + "\n")
    (output_dir / "index.html").write_text(html_page(carrier, duration))
    report = [f"# Raspberry Pi {carrier.upper()} benchmark", "", f"- Cases: {len(rows)}", f"- Passed: {pipeline['passed']}", f"- Failed: {pipeline['failed']}", f"- Configured duration per case: {duration:g} s" if duration is not None else "- Configured duration per case: unknown", "", "## Protocol averages", ""]
    for p in agg["protocols"]:
        av = agg["averages"][p["id"]]
        report.append(f"- **{p['label']}**: {av['combined_mib_s']:.2f} MiB/s combined, RTT p95 {av['rtt_p95_ms']:.2f} ms" if av["combined_mib_s"] is not None and av["rtt_p95_ms"] is not None else f"- **{p['label']}**: insufficient data")
    (output_dir / "report.md").write_text("\n".join(report) + "\n")
    manifest = {}
    for path in sorted(output_dir.rglob("*")):
        if path.is_file() and path.name != "manifest.json": manifest[str(path.relative_to(output_dir))] = hashlib.sha256(path.read_bytes()).hexdigest()
    (output_dir / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")


def main() -> int:
    ap = argparse.ArgumentParser(); ap.add_argument("--mode", choices=("tcp", "udp", "both"), required=True); ap.add_argument("--result-dir", required=True); ap.add_argument("--output-dir", required=True); args = ap.parse_args()
    result_dir, output_dir = Path(args.result_dir).resolve(), Path(args.output_dir).resolve()
    if args.mode == "both":
        build_one(result_dir, output_dir / "tcp", "tcp"); build_one(result_dir, output_dir / "udp", "udp")
        (output_dir / "index.html").write_text('<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>STCP Raspberry Pi benchmarks</title><style>body{background:#020617;color:#e2e8f0;font-family:system-ui;margin:0;padding:4rem}a{display:inline-block;color:#7dd3fc;border:1px solid #334155;border-radius:12px;padding:1rem 1.5rem;margin:.5rem;text-decoration:none}</style></head><body><h1>STCP Raspberry Pi benchmarks</h1><a href="tcp/">TCP / TLS / STCP-TCP</a><a href="udp/">UDP / STCP-UDP</a></body></html>\n')
    else: build_one(result_dir, output_dir, args.mode)
    print(output_dir); return 0

if __name__ == "__main__": raise SystemExit(main())
