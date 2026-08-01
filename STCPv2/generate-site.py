#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, shutil
from pathlib import Path
from datetime import datetime, timezone
from bs4 import BeautifulSoup

ROOT=Path(__file__).resolve().parent
RPI_BASE=ROOT/'RaspberryPI/benchmark/results'
ZEP_BASE=ROOT/'zephyr/nordic/stcp-mqtt/benchmark/results'
OUT_DEFAULT=ROOT/'stcp.fi/benchmarks'
SKIP={'pipeline-summary.json','summary.json','manifest.json'}
LABELS={'tcp':'TCP','udp':'UDP','tls':'TLS','stcp-tcp':'STCP/TCP','stcp-udp':'STCP/UDP'}
def mib(v): return float(v or 0)*8

def readj(p):
    with p.open(encoding='utf-8') as f:return json.load(f)

def json_cases(d:Path):
    out=[]
    for p in d.rglob('*.json'):
        if p.name in SKIP: continue
        try:
            x=readj(p)
            if isinstance(x,dict) and ('payload_bytes' in x or 'operations' in x):out.append(p)
        except Exception: pass
    return out

def run_case_count(run:Path):
    for name in ('pipeline-summary.json','summary.json'):
        q=run/name
        if q.is_file():
            try:
                d=readj(q)
                for k in ('cases_total','total_cases','total'):
                    if isinstance(d.get(k),int) and d[k]>0:return d[k]
            except Exception:pass
    return len(json_cases(run))

def choose_run(base:Path,prefix=None,explicit=None,prefer_largest=False):
    if explicit:
        p=Path(explicit);p=p if p.is_absolute() else ROOT/p
        if not p.is_dir():raise SystemExit(f'Run not found: {p}')
        return p
    ds=[p for p in base.iterdir() if p.is_dir() and (not prefix or p.name.startswith(prefix))]
    if not ds:raise SystemExit(f'No runs under {base}')
    return max(ds,key=lambda p:(run_case_count(p),p.stat().st_mtime)) if prefer_largest else max(ds,key=lambda p:p.stat().st_mtime)

def traffic_type(path:Path,d:dict):
    mode=str(d.get('mode') or '').lower();tr=str(d.get('transport') or '').lower();loc=str(path).lower()
    if mode=='stcp' or tr=='stcp' or 'stcp' in path.name.lower():
        return 'stcp-udp' if tr=='udp' or '/udp/' in loc or 'stcp-udp' in loc else 'stcp-tcp'
    if mode=='tls' or tr=='tls' or 'tls' in path.name.lower():return 'tls'
    if mode=='udp' or tr=='udp' or '/udp/' in loc:return 'udp'
    return 'tcp'

def fnum(v,default=0.0):
    try:return float(v)
    except:return default

def inum(v,default=0):
    try:return int(v)
    except:return default

def normalize(path:Path,d:dict,platform:str):
    errors=inum(d.get('errors')); tt=traffic_type(path,d)
    payload=inum(d.get('payload_bytes')); elapsed=fnum(d.get('elapsed_s'))
    if not elapsed and d.get('elapsed_ms') is not None:elapsed=fnum(d.get('elapsed_ms'))/1000
    c={
      'platform':'raspberry-pi-4' if platform=='rpi' else 'zephyr-nrf9151',
      'platform_label':'Raspberry Pi 4' if platform=='rpi' else 'Zephyr nRF9151',
      'board':'Raspberry Pi 4' if platform=='rpi' else str(d.get('board') or 'nRF9151 DK'),
      'carrier':str(d.get('carrier') or 'ethernet').lower(),
      'transport':str(d.get('transport') or d.get('mode') or 'tcp').lower(),
      'mode':str(d.get('mode') or tt).lower(),'traffic_type':tt,
      'direction':str(d.get('direction') or 'echo').lower(),
      'clients':inum(d.get('clients'),1),'payload_bytes':payload,
      'chunk_bytes':inum(d.get('chunk_bytes') or d.get('device_payload_bytes') or payload),
      'pipeline':inum(d.get('pipeline'),1),'elapsed_s':elapsed,
      'operations':inum(d.get('operations')),'errors':errors,
      'status':inum(d.get('status'),0 if errors==0 else -1),
      'tx_mib_s':fnum(d.get('tx_mib_s')),'rx_mib_s':fnum(d.get('rx_mib_s')),
      'combined_mib_s':fnum(d.get('combined_mib_s')),'operations_s':fnum(d.get('operations_s')),
      'connect_mean_ms':d.get('connect_mean_ms'),'rtt_p50_ms':d.get('rtt_p50_ms'),
      'rtt_p95_ms':d.get('rtt_p95_ms'),'rtt_p99_ms':d.get('rtt_p99_ms'),
      'client_cpu_percent':d.get('client_cpu_percent'),'max_rss_kib':d.get('max_rss_kib'),
      'udp_retransmit_percent':d.get('udp_retransmit_percent'),'udp_timeouts':d.get('udp_timeouts'),
      'source_file':str(path.relative_to(ROOT))
    }
    return c

CSS=r'''
:root{--bg:#030b18;--panel:#0b1728;--panel2:#0e1d31;--line:#223955;--text:#edf5ff;--muted:#91a5bf;--blue:#38bdf8;--cyan:#22d3ee;--purple:#a78bfa;--yellow:#fbbf24;--green:#4ade80;--red:#fb7185}*{box-sizing:border-box}body{margin:0;background:linear-gradient(180deg,#061321 0,#020817 100%);color:var(--text);font:13px system-ui,-apple-system,Segoe UI,sans-serif}a{color:inherit}.wrap{width:min(1180px,96vw);margin:auto;padding:18px 12px 52px}.global-header{position:sticky;top:0;z-index:50;background:rgba(3,11,24,.96);border-bottom:1px solid var(--line);backdrop-filter:blur(10px)}.global-inner{width:min(1240px,96vw);margin:auto;display:flex;align-items:center;justify-content:space-between;gap:18px;padding:10px 12px}.site-brand{font-weight:900;font-size:19px;letter-spacing:.04em;text-decoration:none;color:#fff}.site-nav{display:flex;gap:5px;flex-wrap:wrap}.site-nav a{padding:7px 9px;border-radius:7px;text-decoration:none;color:var(--muted)}.site-nav a:hover,.site-nav a.active{color:#fff;background:#10233a}.benchmark-header{display:flex;justify-content:space-between;align-items:center;gap:14px;margin-bottom:10px}.benchmark-nav{display:flex;gap:6px;flex-wrap:wrap}.benchmark-nav a{padding:6px 9px;border:1px solid var(--line);border-radius:7px;text-decoration:none;color:var(--muted)}.benchmark-nav a:hover,.benchmark-nav a.active{color:#fff;border-color:#38bdf8;background:#10233a}.brand{font-weight:900;letter-spacing:.08em;color:#7dd3fc}h1{font-size:30px;margin:8px 0 3px;line-height:1.1}.lead{color:var(--muted);margin:0 0 12px}.badges,.filters{display:flex;gap:7px;flex-wrap:wrap}.badge{padding:5px 8px;border:1px solid var(--line);border-radius:8px;background:#0d1b2e;color:#c5d5e8}.summary{display:grid;grid-template-columns:repeat(6,minmax(0,1fr));gap:8px;margin:12px 0}.metric{background:var(--panel);border:1px solid var(--line);border-radius:9px;padding:10px}.metric small{color:var(--muted);text-transform:uppercase}.metric strong{display:block;font-size:20px;margin-top:3px}.control-panel{background:var(--panel);border:1px solid var(--line);padding:12px;border-radius:9px;margin:12px 0}.advanced{margin-top:9px;padding-top:9px;border-top:1px solid #1b3048}.advanced summary{cursor:pointer;color:var(--muted);font-size:11px}.advanced .filters{margin-top:8px}.filters label{color:var(--muted);font-size:11px}.filters select{display:block;margin-top:3px;background:#071426;color:#fff;border:1px solid var(--line);border-radius:6px;padding:6px 8px;min-width:112px}.section{margin:15px 0}.section-head{display:flex;justify-content:space-between;align-items:end;border-bottom:1px solid var(--line);margin-bottom:8px;padding-bottom:5px}.section-head h2{font-size:15px;margin:0}.section-head p{margin:0;color:var(--muted);font-size:11px}.chart-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px}.chart-grid.core{grid-template-columns:repeat(2,minmax(0,1fr))}.chart{background:linear-gradient(145deg,var(--panel2),var(--panel));border:1px solid var(--line);border-radius:7px;padding:9px;min-width:0}.chart h3{font-size:11px;margin:0 0 2px}.chart .desc{font-size:9px;color:var(--muted);min-height:12px}.bars{height:178px;border-bottom:1px solid #28415e;display:flex;align-items:flex-end;gap:5px;padding:8px 2px 0;overflow:hidden}.bar-item{display:flex;flex-direction:column;justify-content:flex-end;align-items:center;min-width:18px;flex:1;height:100%}.bar{width:min(34px,72%);min-height:1px;border-radius:3px 3px 0 0;background:var(--blue)}.bar:nth-child(2n){background:var(--purple)}.value{font-size:9px;color:#d9e7f5;margin-top:2px;white-space:nowrap}.label{font-size:8px;color:var(--muted);white-space:nowrap;max-width:58px;overflow:hidden;text-overflow:ellipsis}.legend{display:flex;gap:8px 12px;flex-wrap:wrap;align-items:center;color:#c9d7e8;font-size:9px;margin-top:7px;padding-top:6px;border-top:1px solid #1b3048;min-height:20px}.legend span{display:inline-flex;align-items:center;white-space:nowrap}.legend .legend-unit{color:var(--muted)}.legend i{display:inline-block;width:9px;height:9px;border-radius:2px;margin-right:4px;box-shadow:0 0 0 1px rgba(255,255,255,.14)}.empty{height:178px;display:grid;place-items:center;color:var(--muted)}.tablewrap{overflow:auto;border:1px solid var(--line);border-radius:8px;background:var(--panel)}table{width:100%;border-collapse:collapse;min-width:1050px}th,td{padding:7px 8px;border-bottom:1px solid #1b3048;text-align:left;font-variant-numeric:tabular-nums}th{color:#91a7c3;font-size:9px;text-transform:uppercase;position:sticky;top:0;background:#0a1728}.pass{color:var(--green);font-weight:800}.fail{color:var(--red);font-weight:800}.comparison-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:8px}.note{padding:8px 10px;background:#2a2314;border-left:3px solid var(--yellow);color:#e9d9af;margin:10px 0}.landing{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:10px;margin-top:18px}.linkcard{background:var(--panel);border:1px solid var(--line);border-radius:10px;padding:14px;text-decoration:none}.linkcard:hover{border-color:var(--blue)}.linkcard strong{font-size:18px;display:block;margin:5px 0}.linebox{height:180px;background:var(--panel);border:1px solid var(--line);border-radius:8px;padding:8px}.linebox svg{width:100%;height:100%}.payload-picker{display:flex;gap:7px;flex-wrap:wrap;align-items:center}.payload-actions{display:flex;gap:6px;flex-wrap:wrap;margin-bottom:9px}.payload-actions button{background:#10233a;color:#dcecff;border:1px solid var(--line);border-radius:6px;padding:6px 9px;cursor:pointer}.payload-actions button:hover{border-color:var(--blue)}.payload-checks{display:flex;gap:6px;flex-wrap:wrap}.payload-check{display:inline-flex;align-items:center;gap:5px;padding:6px 8px;border:1px solid var(--line);border-radius:7px;background:#071426;color:#dcecff;cursor:pointer}.payload-check input{accent-color:#38bdf8}.compare-bars{height:210px;display:flex;align-items:stretch;gap:8px;padding:8px 2px 0;overflow-x:auto;border-bottom:1px solid #28415e}.compare-group{min-width:92px;flex:1;display:flex;flex-direction:column;justify-content:flex-end;align-items:stretch}.compare-series-row{height:174px;display:flex;align-items:flex-end;justify-content:center;gap:5px}.compare-bar-item{height:100%;min-width:22px;flex:1;display:flex;flex-direction:column;justify-content:flex-end;align-items:center}.compare-bar{width:min(28px,78%);min-height:1px;border-radius:3px 3px 0 0}.compare-bar.missing{height:8px!important;background:transparent!important;border:1px dashed #64748b}.compare-value{font-size:8px;color:#d9e7f5;margin-top:2px;white-space:nowrap}.compare-protocol{font-size:7px;color:var(--muted);white-space:nowrap}.compare-payload{text-align:center;color:#cfe2f7;font-size:9px;font-weight:700;padding:5px 2px 3px;border-top:1px solid #1b3048;margin-top:3px}@media(max-width:850px){.summary{grid-template-columns:repeat(3,1fr)}.landing{grid-template-columns:repeat(2,1fr)}}@media(max-width:600px){.chart-grid,.comparison-grid{grid-template-columns:1fr}.summary{grid-template-columns:repeat(2,1fr)}.benchmark-header,.global-inner{align-items:flex-start;flex-direction:column}.landing{grid-template-columns:1fr}}
'''

JS=r'''
const DATA=window.PAGE_DATA||{cases:[]};const ALL=DATA.cases||[];const q=s=>document.querySelector(s);const qa=s=>[...document.querySelectorAll(s)];
const fmt=(v,d=2)=>Number(v||0).toFixed(d);const mib=v=>Number(v||0)*8;const uniq=k=>[...new Set(ALL.map(x=>x[k]).filter(v=>v!==null&&v!==undefined))].sort((a,b)=>String(a).localeCompare(String(b),undefined,{numeric:true}));
const labels={tcp:'TCP',udp:'UDP',tls:'TLS','stcp-tcp':'STCP/TCP','stcp-udp':'STCP/UDP'};
function initSelect(id,key,all=true){const e=q(id);if(!e)return;if(all)e.innerHTML='<option value="">All</option>';uniq(key).forEach(v=>{const o=document.createElement('option');o.value=v;o.textContent=key==='traffic_type'?(labels[v]||v):v;e.appendChild(o)});}
function payloadLabel(n){n=Number(n);return n>=1048576?(n/1048576)+' MiB':n>=1024?(n/1024)+' KiB':n+' B'}
function filtered(extra={}){return ALL.filter(x=>{for(const [id,key] of [['#platform','platform'],['#carrier','carrier'],['#traffic','traffic_type'],['#direction','direction'],['#payload','payload_bytes'],['#clients','clients'],['#pipeline','pipeline']]){const e=q(id);if(e&&e.value!==''&&String(x[key])!==e.value)return false}for(const [k,v] of Object.entries(extra))if(v!==''&&v!==undefined&&String(x[k])!==String(v))return false;return true})}
function metricValue(x,key){if(key==='combined_mbit')return mib(x.combined_mib_s);if(key==='tx_mbit')return mib(x.tx_mib_s);if(key==='rx_mbit')return mib(x.rx_mib_s);return Number(x[key]||0)}
function median(values){const a=values.filter(Number.isFinite).sort((a,b)=>a-b);if(!a.length)return 0;const m=Math.floor(a.length/2);return a.length%2?a[m]:(a[m-1]+a[m])/2}
function grouped(rows,group,metric){const m=new Map();rows.filter(x=>Number(x.status||0)===0&&Number(x.errors||0)===0).forEach(x=>{const raw=x[group]??'n/a';const k=String(raw);if(!m.has(k))m.set(k,{raw,label:group==='payload_bytes'?payloadLabel(raw):String(raw),values:[]});m.get(k).values.push(metricValue(x,metric))});return [...m.values()].map(o=>({...o,series:'Median',value:median(o.values),count:o.values.length})).sort((a,b)=>{if(Number.isFinite(Number(a.raw))&&Number.isFinite(Number(b.raw)))return Number(a.raw)-Number(b.raw);return String(a.label).localeCompare(String(b.label),undefined,{numeric:true})})}
const colors=['#38bdf8','#a78bfa','#fbbf24','#4ade80','#22d3ee','#fb7185'];
function drawBars(id,items,unit=''){const el=q(id);if(!el)return;const card=el.closest('.chart');const useful=items.some(x=>Number(x.value)>0);if(card)card.hidden=!useful;if(!items.length||!useful){el.innerHTML='<div class="empty">No matching data</div>';return}const max=Math.max(...items.map(x=>x.value),.000001);el.innerHTML=items.map((x,i)=>`<div class="bar-item" title="${x.series||''} ${x.label}: ${fmt(x.value,3)} ${unit}"><div class="bar" style="height:${Math.max(1,x.value/max*148)}px;background:${colors[i%colors.length]}"></div><div class="value">${fmt(x.value,x.value<10?2:0)}</div><div class="label">${x.label}</div></div>`).join('')}
function drawGroup(id,rows,group,metric,unit){drawBars(id,grouped(rows,group,metric),unit)}
function update(){const rows=filtered();q('#shown')&&(q('#shown').textContent=rows.length);const d=q('#direction')?.value||'';const metric=d==='upload'?'tx_mbit':d==='download'?'rx_mbit':'combined_mbit';drawGroup('#throughputPayload',rows,'payload_bytes',metric,'Mbit/s');drawGroup('#throughputClients',rows,'clients',metric,'Mbit/s');drawGroup('#throughputPipeline',rows,'pipeline',metric,'Mbit/s');drawGroup('#opsPayload',rows,'payload_bytes','operations_s','ops/s');drawGroup('#rtt95Payload',rows,'payload_bytes','rtt_p95_ms','ms');drawGroup('#cpuPayload',rows,'payload_bytes','client_cpu_percent','%');drawGroup('#elapsedPayload',rows,'payload_bytes','elapsed_s','s');drawGroup('#errorsPayload',rows,'payload_bytes','errors','errors');renderTable(rows);renderCompare();}
function renderTable(rows){const b=q('#tbody');if(!b)return;b.innerHTML=rows.map(x=>`<tr><td>${x.platform_label}</td><td>${labels[x.traffic_type]||x.traffic_type}</td><td>${x.direction}</td><td>${payloadLabel(x.payload_bytes)}</td><td>${x.clients}</td><td>${x.pipeline}</td><td>${fmt(mib(x.tx_mib_s),3)}</td><td>${fmt(mib(x.rx_mib_s),3)}</td><td><b>${fmt(mib(x.combined_mib_s),3)}</b></td><td>${fmt(x.operations_s,1)}</td><td>${fmt(x.rtt_p95_ms,2)}</td><td class="${x.errors===0&&x.status===0?'pass':'fail'}">${x.errors===0&&x.status===0?'PASS':'FAIL'}</td></tr>`).join('')}
function selectedComparisonPayloads(){return qa('.comparison-payload-check:checked').map(e=>Number(e.value)).sort((a,b)=>a-b)}
function comparisonMetric(x){const d=q('#direction')?.value||'';return d==='upload'?mib(x.tx_mib_s):d==='download'?mib(x.rx_mib_s):mib(x.combined_mib_s)}
function comparisonBase(payload){return ALL.filter(x=>Number(x.payload_bytes)===Number(payload)).filter(x=>{for(const [id,key] of [['#platform','platform'],['#carrier','carrier'],['#direction','direction'],['#clients','clients'],['#pipeline','pipeline']]){const e=q(id);if(e&&e.value!==''&&String(x[key])!==e.value)return false}return Number(x.status||0)===0&&Number(x.errors||0)===0})}
function protocolValue(payload,t){const r=comparisonBase(payload).filter(x=>x.traffic_type===t);return r.length?median(r.map(comparisonMetric)):null}
function drawComparisonGroups(id,payloads,protocols){const el=q(id);if(!el)return;const groups=payloads.map(payload=>({payload,label:payloadLabel(payload),items:protocols.map((t,i)=>({traffic:t,label:labels[t]+(t==='tls'&&id==='#udpCompare'?' ref.':''),value:protocolValue(payload,t),color:colors[i]}))}));const vals=groups.flatMap(g=>g.items.map(x=>x.value).filter(v=>v!==null&&Number.isFinite(v)));const max=Math.max(...vals,.000001);if(!groups.length){el.innerHTML='<div class="empty">Select at least one payload</div>';return}el.classList.add('compare-bars');el.innerHTML=groups.map(g=>`<div class="compare-group"><div class="compare-series-row">${g.items.map(x=>`<div class="compare-bar-item" title="${g.label} · ${x.label}: ${x.value===null?'N/A':fmt(x.value,3)+' Mbit/s'}"><div class="compare-bar ${x.value===null?'missing':''}" style="height:${x.value===null?8:Math.max(2,x.value/max*148)}px;background:${x.color}"></div><div class="compare-value">${x.value===null?'N/A':fmt(x.value,x.value<10?2:0)}</div><div class="compare-protocol">${x.label}</div></div>`).join('')}</div><div class="compare-payload">${g.label}</div></div>`).join('')}
function setComparisonPayloads(mode){const boxes=qa('.comparison-payload-check');const values=boxes.map(b=>Number(b.value));const pivot=65536;boxes.forEach(b=>{const v=Number(b.value);b.checked=mode==='all'||(mode==='small'&&v<=pivot)||(mode==='large'&&v>pivot)});if(mode==='clear')boxes.forEach(b=>b.checked=false);if(!boxes.some(b=>b.checked)&&boxes.length)boxes[0].checked=true;update()}
function buildComparisonPayloadPicker(){const box=q('#comparisonPayloadChecks');if(!box)return;const values=uniq('payload_bytes').map(Number).sort((a,b)=>a-b);box.innerHTML=values.map(v=>`<label class="payload-check"><input class="comparison-payload-check" type="checkbox" value="${v}" checked> ${payloadLabel(v)}</label>`).join('');qa('.comparison-payload-check').forEach(e=>e.addEventListener('change',()=>{if(!qa('.comparison-payload-check:checked').length)e.checked=true;update()}));qa('[data-payload-action]').forEach(b=>b.addEventListener('click',()=>setComparisonPayloads(b.dataset.payloadAction)))}
function renderCompare(){const payloads=selectedComparisonPayloads();drawComparisonGroups('#tcpCompare',payloads,['tcp','stcp-tcp','tls']);drawComparisonGroups('#udpCompare',payloads,['udp','stcp-udp','tls']);const label=q('#comparisonPayloadLabel');if(label)label.textContent=payloads.length+' selected'}
function boot(){initSelect('#platform','platform');initSelect('#carrier','carrier');initSelect('#traffic','traffic_type',false);initSelect('#direction','direction');initSelect('#payload','payload_bytes');initSelect('#clients','clients');initSelect('#pipeline','pipeline');const tr=q('#traffic');if(tr&&tr.options.length)tr.selectedIndex=0;const dir=q('#direction');if(dir){const full=[...dir.options].find(o=>o.value==='full');if(full)dir.value='full';}buildComparisonPayloadPicker();qa('select').forEach(e=>e.addEventListener('change',update));update()}document.addEventListener('DOMContentLoaded',boot);
'''



def format_html(document: str) -> str:
    """Return stable, human-editable HTML with consistent indentation."""
    return BeautifulSoup(document, "html.parser").prettify(formatter="html5") + "\n"

def format_css(source: str) -> str:
    """Lightweight CSS formatting without changing selector semantics."""
    out = []
    indent = 0
    token = []
    in_string = None
    escape = False
    for ch in source.strip():
        if in_string:
            token.append(ch)
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == in_string:
                in_string = None
            continue
        if ch in ("\"", "'"):
            in_string = ch
            token.append(ch)
        elif ch == "{":
            head = "".join(token).strip()
            if head:
                out.append("    " * indent + head + " {")
            token = []
            indent += 1
        elif ch == ";":
            value = "".join(token).strip()
            if value:
                out.append("    " * indent + value + ";")
            token = []
        elif ch == "}":
            value = "".join(token).strip()
            if value:
                out.append("    " * indent + value)
            token = []
            indent = max(0, indent - 1)
            out.append("    " * indent + "}")
            out.append("")
        else:
            token.append(ch)
    tail = "".join(token).strip()
    if tail:
        out.append("    " * indent + tail)
    return "\n".join(out).rstrip() + "\n"

def nav(active, base='../'):
    items = [
        ('site-home', 'Home', '/'),
        ('benchmarks', 'Benchmarks', '/benchmarks/'),
        ('github', 'GitHub', 'https://github.com/Paxsudos-IT/STCP'),
    ]
    links = '\n'.join(
        f'    <a class="{"active" if key == active else ""}" href="{url}">{label}</a>'
        for key, label, url in items
    )
    return f'''
<nav class="site-nav">
{links}
</nav>
'''

def benchmark_nav(active, base='../'):
    items = [
        ('home', 'Overview', base + 'index.html'),
        ('rpi-tcp', 'Raspberry Pi TCP', base + 'raspberry-pi/tcp/index.html'),
        ('rpi-udp', 'Raspberry Pi UDP', base + 'raspberry-pi/udp/index.html'),
        ('zephyr', 'Zephyr', base + 'zephyr/index.html'),
        ('compare', 'Compare', base + 'compare/index.html'),
    ]
    links = '\n'.join(
        f'    <a class="{"active" if key == active else ""}" href="{url}">{label}</a>'
        for key, label, url in items
    )
    return f'''
<nav class="benchmark-nav">
{links}
</nav>
'''

def shell(title, lead, active, content, cases, base='../'):
    data = json.dumps(
        {
            'generated_utc': datetime.now(timezone.utc).isoformat(),
            'cases': cases,
        },
        indent=2,
    )
    document = f'''
<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>{title}</title>
    <link rel="stylesheet" href="{base}assets/report.css">
</head>
<body>
    <!-- Shared STCPv2 site navigation -->
    <header class="global-header">
        <div class="global-inner">
            <a class="site-brand" href="/">STCPv2</a>
            {nav('benchmarks', base)}
        </div>
    </header>

    <!-- Benchmark report content -->
    <main class="wrap">
        <div class="benchmark-header">
            <div class="brand">STCPV2 BENCHMARKS</div>
            {benchmark_nav(active, base)}
        </div>

        <h1>{title}</h1>
        <p class="lead">{lead}</p>

        {content}
    </main>

    <!-- Page-specific benchmark data. Safe to edit by hand. -->
    <script>
        window.PAGE_DATA = {data};
    </script>
    <script src="{base}assets/report.js" defer></script>
</body>
</html>
'''
    return format_html(document)

def controls(include_platform=False,force_traffic=None):
    platform='<label>Platform<select id="platform"></select></label>' if include_platform else ''
    traffic='<label>Traffic<select id="traffic"></select></label>'
    return f'''<div class="control-panel"><div class="filters">{platform}{traffic}<label>Direction<select id="direction"></select></label><label>Payload<select id="payload"></select></label></div><details class="advanced"><summary>Advanced filters</summary><div class="filters"><label>Carrier<select id="carrier"></select></label><label>Clients<select id="clients"></select></label><label>Pipeline<select id="pipeline"></select></label></div></details></div>'''

def chart(id,title,desc):
    if id == "tcpCompare":
        legend = '<span><i style="background:#38bdf8"></i>TCP</span><span><i style="background:#a78bfa"></i>STCP/TCP</span><span><i style="background:#fbbf24"></i>TLS</span>'
    elif id == "udpCompare":
        legend = '<span><i style="background:#22d3ee"></i>UDP</span><span><i style="background:#4ade80"></i>STCP/UDP</span><span><i style="background:#fbbf24"></i>TLS reference</span>'
    else:
        legend = '<span><i style="background:#38bdf8"></i>Median for each x-axis group</span><span class="legend-unit">One bar per category; detailed variants remain in the table</span>'
    return f'<div class="chart"><h3>{title}</h3><div class="desc">{desc}</div><div class="bars" id="{id}"></div><div class="legend" aria-label="Chart legend">{legend}</div></div>' 
def section(title,desc,charts,cls=''):
    return f'<section class="section"><div class="section-head"><h2>{title}</h2><p>{desc}</p></div><div class="chart-grid {cls}">{"".join(charts)}</div></section>'
def results_table():return '''<section class="section"><div class="section-head"><h2>Detailed results</h2><p>All cases matching the current filters (<span id="shown">0</span>)</p></div><div class="tablewrap"><table><thead><tr><th>Platform</th><th>Traffic</th><th>Direction</th><th>Payload</th><th>Clients</th><th>Pipeline</th><th>TX Mbit/s</th><th>RX Mbit/s</th><th>Total Mbit/s</th><th>Ops/s</th><th>RTT p95</th><th>Status</th></tr></thead><tbody id="tbody"></tbody></table></div></section>'''

def long_report(cases,title,lead,active,run,include_platform=False,base='../../'):
    total=len(cases);passed=sum(c['errors']==0 and c['status']==0 for c in cases);best=max([mib(c['combined_mib_s']) for c in cases] or [0]);payloads=len(set(c['payload_bytes'] for c in cases));
    traffic=', '.join(sorted({LABELS.get(c['traffic_type'],c['traffic_type']) for c in cases}))
    content=f'''<div class="badges"><span class="badge">Run: {run}</span><span class="badge">Traffic: {traffic}</span><span class="badge">Cases: {total}</span></div><div class="summary"><div class="metric"><small>Cases</small><strong>{total}</strong></div><div class="metric"><small>Passed</small><strong>{passed}</strong></div><div class="metric"><small>Failed</small><strong>{total-passed}</strong></div><div class="metric"><small>Best total</small><strong>{best:.2f}</strong><small>Mbit/s</small></div><div class="metric"><small>Payloads</small><strong>{payloads}</strong></div><div class="metric"><small>Pass rate</small><strong>{(100*passed/total if total else 0):.1f}%</strong></div></div>{controls(include_platform)}'''
    content+=section('Core performance','One value per category — use the filters to inspect one traffic type and direction at a time',[chart('throughputPayload','Throughput by payload','Total for full, TX for upload, RX for download'),chart('throughputClients','Throughput by client count','Median throughput for each client count'),chart('throughputPipeline','Throughput by pipeline depth','Median throughput for each pipeline depth'),chart('opsPayload','Operations/s by payload','Median application operation rate')],'core')
    content+=section('Timing and quality','Only the most useful supporting metrics',[chart('rtt95Payload','RTT p95 by payload','Tail latency'),chart('elapsedPayload','Elapsed time by payload','Median case duration'),chart('cpuPayload','Client CPU by payload','Median client-side CPU load'),chart('errorsPayload','Errors by payload','Failed operations')])
    content+=f'''<section class="section"><div class="section-head"><h2>Protocol family comparison</h2><p>Select one or more payloads for side-by-side protocol comparison</p></div><div class="control-panel"><div class="payload-actions"><button type="button" data-payload-action="all">Select all</button><button type="button" data-payload-action="clear">Clear</button><button type="button" data-payload-action="small">Small payloads</button><button type="button" data-payload-action="large">Large payloads</button><span class="badge">Selected: <strong id="comparisonPayloadLabel">0 selected</strong></span></div><div id="comparisonPayloadChecks" class="payload-checks" aria-label="Comparison payloads"></div></div><div class="comparison-grid">{chart('tcpCompare','TCP vs STCP/TCP vs TLS','Grouped by selected payloads')}{chart('udpCompare','UDP vs STCP/UDP vs TLS reference','Grouped by selected payloads; missing reference values appear as N/A')}</div></section>'''
    content+=results_table()
    return shell(title,lead,active,content,cases,base)

def landing(rc,zc):
    cards=f'''<div class="summary"><div class="metric"><small>Raspberry Pi cases</small><strong>{len(rc)}</strong></div><div class="metric"><small>Zephyr cases</small><strong>{len(zc)}</strong></div><div class="metric"><small>Total cases</small><strong>{len(rc)+len(zc)}</strong></div><div class="metric"><small>Platforms</small><strong>2</strong></div><div class="metric"><small>RPI transports</small><strong>{len(set(c['traffic_type'] for c in rc))}</strong></div><div class="metric"><small>Zephyr carriers</small><strong>{len(set(c['carrier'] for c in zc))}</strong></div></div><div class="landing"><a class="linkcard" href="raspberry-pi/tcp/index.html"><small>Raspberry Pi</small><strong>TCP benchmark</strong><span>TCP, STCP/TCP and TLS views</span></a><a class="linkcard" href="raspberry-pi/udp/index.html"><small>Raspberry Pi</small><strong>UDP benchmark</strong><span>UDP and STCP/UDP views</span></a><a class="linkcard" href="zephyr/index.html"><small>Embedded</small><strong>Zephyr benchmark</strong><span>The same full multi-chart report model</span></a><a class="linkcard" href="compare/index.html"><small>Cross-platform</small><strong>Platform comparison</strong><span>Raspberry Pi versus Zephyr</span></a></div>'''
    return shell('STCPv2 benchmark results','Choose the complete Raspberry Pi TCP/UDP views, the Zephyr report, or the cross-platform comparison.','home',cards,[],base='')

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--rpi-run');ap.add_argument('--zephyr-run');ap.add_argument('--output',default=str(OUT_DEFAULT));a=ap.parse_args()
    rpi=choose_run(RPI_BASE,explicit=a.rpi_run,prefer_largest=True);zep=choose_run(ZEP_BASE,'zephyr-',a.zephyr_run)
    rc=[]
    for p in json_cases(rpi):
        c=normalize(p,readj(p),'rpi');c['run_id']=rpi.name;rc.append(c)
    zc=[]
    for p in json_cases(zep):
        c=normalize(p,readj(p),'zep');c['run_id']=zep.name;zc.append(c)
    out = Path(a.output)
    shutil.rmtree(out, ignore_errors=True)
    assets = out / 'assets'
    assets.mkdir(parents=True, exist_ok=True)
    (assets / 'report.css').write_text(format_css(CSS), encoding='utf-8')
    (assets / 'report.js').write_text(JS.strip() + '\n', encoding='utf-8')
    for d in ('raspberry-pi/tcp','raspberry-pi/udp','zephyr','compare','releases','raw/raspberry-pi','raw/zephyr'): (out/d).mkdir(parents=True,exist_ok=True)
    rpi_tcp=[c for c in rc if c['traffic_type'] in ('tcp','stcp-tcp','tls')]
    rpi_udp=[c for c in rc if c['traffic_type'] in ('udp','stcp-udp')]
    (out/'index.html').write_text(landing(rc,zc),encoding='utf-8')
    (out/'raspberry-pi/index.html').parent.mkdir(exist_ok=True)
    rpi_cards=f'''<div class="landing"><a class="linkcard" href="tcp/index.html"><small>Raspberry Pi</small><strong>TCP benchmark</strong><span>{len(rpi_tcp)} cases: TCP, STCP/TCP and TLS</span></a><a class="linkcard" href="udp/index.html"><small>Raspberry Pi</small><strong>UDP benchmark</strong><span>{len(rpi_udp)} cases: UDP and STCP/UDP</span></a></div>'''
    (out/'raspberry-pi/index.html').write_text(shell('Raspberry Pi benchmark results','Open the complete TCP or UDP benchmark view.','rpi',rpi_cards,rc,base='../'),encoding='utf-8')
    (out/'raspberry-pi/tcp/index.html').write_text(long_report(rpi_tcp,'Raspberry Pi TCP benchmark','Complete TCP, STCP/TCP and TLS benchmark dashboard in the original long-form multi-chart layout.','rpi-tcp',rpi.name,base='../../'),encoding='utf-8')
    (out/'raspberry-pi/udp/index.html').write_text(long_report(rpi_udp,'Raspberry Pi UDP benchmark','Complete UDP and STCP/UDP benchmark dashboard in the original long-form multi-chart layout.','rpi-udp',rpi.name,base='../../'),encoding='utf-8')
    (out/'zephyr/index.html').write_text(long_report(zc,'Zephyr benchmark','nRF9151 results rendered with the same complete multi-chart layout as Raspberry Pi.','zephyr',zep.name,base='../'),encoding='utf-8')
    (out/'compare/index.html').write_text(long_report(rc+zc,'Raspberry Pi vs Zephyr','Cross-platform comparison using the same chart sections and filter model.','compare',f'{rpi.name} + {zep.name}',include_platform=True,base='../'),encoding='utf-8')
    rpi_raw = out/'raw/raspberry-pi'/rpi.name
    zep_raw = out/'raw/zephyr'/zep.name
    shutil.copytree(rpi, rpi_raw)
    shutil.copytree(zep, zep_raw)
    def write_directory_indexes(tree_root, title):
        directories = [tree_root] + [d for d in tree_root.rglob('*') if d.is_dir()]
        for directory in directories:
            entries = []
            if directory != tree_root:
                entries.append('<li><a href="../">../</a></li>')
            for child in sorted(directory.iterdir(), key=lambda x: (not x.is_dir(), x.name.lower())):
                if child.name == 'index.html':
                    continue
                href = child.name + ('/' if child.is_dir() else '')
                entries.append(f'<li><a href="{href}">{href}</a></li>')
            relative = directory.relative_to(tree_root)
            label = title if str(relative) == '.' else f'{title} / {relative}'
            page = '<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>' + label + '</title><style>' + CSS + 'ul{line-height:1.8}code{color:#7dd3fc}</style></head><body><header class="global-header"><div class="global-inner"><a class="site-brand" href="/">STCPv2</a>' + nav('benchmarks','/') + '</div></header><div class="wrap"><h1>' + label + '</h1><p class="lead">Raw benchmark files used by the generated report.</p><ul>' + ''.join(entries) + '</ul></div></body></html>'
            (directory/'index.html').write_text(format_html(page), encoding='utf-8')
    write_directory_indexes(rpi_raw, f'Raspberry Pi raw data — {rpi.name}')
    write_directory_indexes(zep_raw, f'Zephyr raw data — {zep.name}')
    history='<div class="landing"><a class="linkcard" href="../raspberry-pi/tcp/index.html"><small>Raspberry Pi</small><strong>'+rpi.name+'</strong><span>'+str(len(rc))+' benchmark cases</span></a><a class="linkcard" href="../zephyr/index.html"><small>Zephyr</small><strong>'+zep.name+'</strong><span>'+str(len(zc))+' benchmark cases</span></a></div>'
    (out/'releases/index.html').write_text(shell('Benchmark release history','Selected benchmark result sets used by the generated reports.','history',history,[],base='../'),encoding='utf-8')
    rawcards='<div class="landing"><a class="linkcard" href="raspberry-pi/'+rpi.name+'/"><small>Raw JSON</small><strong>Raspberry Pi</strong><span>'+rpi.name+'</span></a><a class="linkcard" href="zephyr/'+zep.name+'/"><small>Raw JSON</small><strong>Zephyr</strong><span>'+zep.name+'</span></a></div>'
    (out/'raw/index.html').write_text(shell('Raw benchmark data','Browse the exact JSON inputs used to build the reports.','raw',rawcards,[],base='../'),encoding='utf-8')
    manifest={'generated_utc':datetime.now(timezone.utc).isoformat(),'raspberry_pi_run':rpi.name,'raspberry_pi_cases':len(rc),'raspberry_pi_tcp_cases':len(rpi_tcp),'raspberry_pi_udp_cases':len(rpi_udp),'zephyr_run':zep.name,'zephyr_cases':len(zc),'pages':['index.html','raspberry-pi/index.html','raspberry-pi/tcp/index.html','raspberry-pi/udp/index.html','zephyr/index.html','compare/index.html','releases/index.html','raw/index.html']}
    (out/'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')

    # Final pass: keep every generated HTML file consistently formatted.
    for html_file in out.rglob('*.html'):
        html_file.write_text(format_html(html_file.read_text(encoding='utf-8')), encoding='utf-8')
    print(f'[OK] Generated {out}');print(f'[INFO] Raspberry Pi: {len(rc)} cases ({len(rpi_tcp)} TCP-family, {len(rpi_udp)} UDP-family)');print(f'[INFO] Zephyr: {len(zc)} cases')
if __name__=='__main__':main()
