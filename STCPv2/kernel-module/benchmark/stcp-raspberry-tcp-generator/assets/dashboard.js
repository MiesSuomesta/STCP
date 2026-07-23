(() => {
  'use strict';
  const D = window.STCP_BENCHMARK_DATA;
  if (!D) throw new Error('Missing STCP_BENCHMARK_DATA');
  const colors = {tcp:'#69a7ff',stcp:'#5de1c2',tls:'#b99cff'};
  const labels = {tcp:'TCP',stcp:'STCP',tls:'TLS'};
  const $ = s => document.querySelector(s);
  const fmt = (v, digits=2) => v == null || !Number.isFinite(Number(v)) ? 'n/a' : Number(v).toLocaleString(undefined,{maximumFractionDigits:digits});
  const pct = v => v == null ? 'n/a' : `${v>=0?'+':''}${fmt(v,1)}%`;
  const payloadLabel = n => n>=1048576&&n%1048576===0?`${n/1048576} MiB`:n>=1024&&n%1024===0?`${n/1024} KiB`:`${n} B`;
  const esc = s => String(s).replace(/[&<>'"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;',"'":'&#39;','"':'&quot;'}[c]));

  const state = {
    payload: D.dimensions.payload_bytes.includes(1024) ? 1024 : D.dimensions.payload_bytes[0],
    clients: D.dimensions.clients.includes(1) ? 1 : D.dimensions.clients[0],
    pipeline: D.dimensions.pipelines.includes(1) ? 1 : D.dimensions.pipelines[0],
    passOnly: true
  };

  function setupMeta(){
    const m=D.metadata;
    $('#hero-meta').innerHTML=[m.generated_at,m.commit&&`Commit ${m.commit}`,m.kernel&&`Kernel ${m.kernel}`,`${D.summary.total_cases} cases`].filter(Boolean).map(x=>`<span class="meta-pill">${esc(x)}</span>`).join('');
  }
  function setupStatus(){
    $('#status-cards').innerHTML=D.dimensions.protocols.map(p=>{
      const s=D.summary.protocols[p], ok=s.failed===0;
      return `<article class="status-card"><div class="status-top"><span class="protocol" style="color:${colors[p]}">${labels[p]}</span><span class="badge ${ok?'pass':'warn'}">${ok?'PASS':'KNOWN ISSUES'}</span></div><div class="big-number">${fmt(s.pass_percent,1)}%</div><div class="muted">${s.passed}/${s.cases} cases passed · ${s.errors} errors</div></article>`;
    }).join('');
  }
  function setupControls(){
    const fill=(id,vals,labelFn=v=>v)=>{const el=$(id);el.innerHTML=vals.map(v=>`<option value="${v}">${labelFn(v)}</option>`).join('');return el};
    const p=fill('#payload-select',D.dimensions.payload_bytes,payloadLabel);p.value=state.payload;p.onchange=()=>{state.payload=+p.value;renderAll()};
    const c=fill('#clients-select',D.dimensions.clients,v=>`${v} client${v===1?'':'s'}`);c.value=state.clients;c.onchange=()=>{state.clients=+c.value;renderAll()};
    const q=fill('#pipeline-select',D.dimensions.pipelines,v=>`Pipeline ${v}`);q.value=state.pipeline;q.onchange=()=>{state.pipeline=+q.value;renderAll()};
    $('#pass-only').onchange=e=>{state.passOnly=e.target.checked;renderAll()};
  }
  function currentCases(){return D.cases.filter(c=>c.payload_bytes===state.payload&&c.clients===state.clients&&c.pipeline===state.pipeline&&(!state.passOnly||c.errors===0));}
  function caseByProtocol(){return Object.fromEntries(currentCases().map(c=>[c.protocol,c]));}
  function setupSummary(){
    const vs=D.summary.comparisons.stcp_vs_tls, v=vs.median_percent_change;
    $('#executive-copy').textContent=`Across ${vs.matched_pass_cases} directly matched successful cases, STCP is evaluated against TLS with identical payload, client and pipeline settings. Failed cases stay visible in the reliability section and are excluded from the headline medians.`;
    const metrics=[['Throughput vs TLS',v.combined_mib_s,true],['Median RTT vs TLS',v.rtt_p50_ms,false],['Client CPU vs TLS',v.client_cpu_percent,false],['Instructions/op vs TLS',v.server_perf_instructions_per_op,false]];
    $('#comparison-cards').innerHTML=metrics.map(([name,val,higher])=>{const good=val!=null&&(higher?val>0:val<0);return `<div class="comparison"><small>${name}</small><strong style="color:${good?'var(--accent)':val==null?'var(--muted)':'var(--warn)'}">${pct(val)}</strong><small>Median matched-case change</small></div>`}).join('');
  }

  function tooltip(){let t=$('.tooltip');if(!t){t=document.createElement('div');t.className='tooltip';document.body.appendChild(t)}return t}
  function bindTips(root){const t=tooltip();root.querySelectorAll('[data-tip]').forEach(n=>{n.addEventListener('mousemove',e=>{t.innerHTML=n.dataset.tip;t.style.display='block';t.style.left=`${e.clientX+12}px`;t.style.top=`${e.clientY+12}px`});n.addEventListener('mouseleave',()=>t.style.display='none')})}
  function svgWrap(inner,legend=''){return `${legend}<svg viewBox="0 0 720 270" role="img" aria-label="Benchmark chart">${inner}</svg>`}
  function legend(protocols){return `<div class="legend">${protocols.map(p=>`<span class="legend-item"><i class="legend-dot" style="background:${colors[p]}"></i>${labels[p]}</span>`).join('')}</div>`}
  function barChart(id, metric, unit='', digits=2){
    const root=$(id), rows=currentCases().filter(c=>c[metric]!=null), protocols=rows.map(r=>r.protocol);
    if(!rows.length){root.innerHTML='<p class="muted">No data for this selection.</p>';return}
    const max=Math.max(...rows.map(r=>r[metric]),1), left=55, base=230, width=570, bw=Math.min(115,width/(rows.length*1.65)), gap=(width-bw*rows.length)/(rows.length+1);
    let inner='';for(let i=0;i<5;i++){const y=base-i*47.5,v=max*i/4;inner+=`<line class="grid-line" x1="${left}" y1="${y}" x2="${left+width}" y2="${y}"/><text class="tick" x="${left-8}" y="${y+4}" text-anchor="end">${fmt(v,1)}</text>`}
    rows.forEach((r,i)=>{const h=(r[metric]/max)*190,x=left+gap+(bw+gap)*i,y=base-h;const val=`${fmt(r[metric],digits)}${unit}`;inner+=`<rect data-tip="<b>${labels[r.protocol]}</b><br>${val}" x="${x}" y="${y}" width="${bw}" height="${h}" rx="7" fill="${colors[r.protocol]}" opacity=".9"/><text class="bar-label" x="${x+bw/2}" y="${Math.max(14,y-7)}" text-anchor="middle">${val}</text><text class="tick" x="${x+bw/2}" y="252" text-anchor="middle">${labels[r.protocol]}</text>`});
    root.innerHTML=svgWrap(inner,legend(protocols));bindTips(root)
  }
  function groupedBar(id, metrics){
    const root=$(id), rows=currentCases();if(!rows.length){root.innerHTML='<p class="muted">No data.</p>';return}
    const values=rows.flatMap(r=>metrics.map(m=>r[m.key]).filter(v=>v!=null)),max=Math.max(...values,1),left=55,base=230,width=600, groupW=width/rows.length,bw=Math.min(27,(groupW-16)/metrics.length);let inner='';
    for(let i=0;i<5;i++){const y=base-i*47.5,v=max*i/4;inner+=`<line class="grid-line" x1="${left}" y1="${y}" x2="${left+width}" y2="${y}"/><text class="tick" x="${left-8}" y="${y+4}" text-anchor="end">${fmt(v,1)}</text>`}
    rows.forEach((r,ri)=>{metrics.forEach((m,mi)=>{const v=r[m.key];if(v==null)return;const h=v/max*190,x=left+ri*groupW+(groupW-metrics.length*bw)/2+mi*bw,y=base-h;inner+=`<rect data-tip="<b>${labels[r.protocol]} ${m.label}</b><br>${fmt(v,3)} ms" x="${x}" y="${y}" width="${bw-3}" height="${h}" rx="4" fill="${m.color}" opacity=".9"/>`});inner+=`<text class="tick" x="${left+ri*groupW+groupW/2}" y="252" text-anchor="middle">${labels[r.protocol]}</text>`});
    const lg=`<div class="legend">${metrics.map(m=>`<span class="legend-item"><i class="legend-dot" style="background:${m.color}"></i>${m.label}</span>`).join('')}</div>`;root.innerHTML=svgWrap(inner,lg);bindTips(root)
  }
  function lineChart(id, metric, unit='', digits=2){
    const root=$(id), filtered=D.cases.filter(c=>c.clients===state.clients&&c.pipeline===state.pipeline&&(!state.passOnly||c.errors===0)&&c[metric]!=null), payloads=D.dimensions.payload_bytes;
    if(!filtered.length){root.innerHTML='<p class="muted">No data.</p>';return}
    const values=filtered.map(r=>r[metric]),minX=0,maxY=Math.max(...values,1),left=60,top=18,width=620,height=205,base=top+height;let inner='';
    for(let i=0;i<5;i++){const y=base-i*height/4,v=maxY*i/4;inner+=`<line class="grid-line" x1="${left}" y1="${y}" x2="${left+width}" y2="${y}"/><text class="tick" x="${left-8}" y="${y+4}" text-anchor="end">${fmt(v,1)}</text>`}
    const x=i=>payloads.length===1?left+width/2:left+i*width/(payloads.length-1), y=v=>base-v/maxY*height;
    D.dimensions.protocols.forEach(p=>{const map=new Map(filtered.filter(r=>r.protocol===p).map(r=>[r.payload_bytes,r]));const pts=payloads.map((pb,i)=>map.has(pb)?[x(i),y(map.get(pb)[metric]),map.get(pb)]:null).filter(Boolean);if(!pts.length)return;inner+=`<polyline fill="none" stroke="${colors[p]}" stroke-width="3" points="${pts.map(a=>`${a[0]},${a[1]}`).join(' ')}"/>`;pts.forEach(a=>inner+=`<circle data-tip="<b>${labels[p]} · ${payloadLabel(a[2].payload_bytes)}</b><br>${fmt(a[2][metric],digits)}${unit}" cx="${a[0]}" cy="${a[1]}" r="5" fill="${colors[p]}"/>`)});
    payloads.forEach((pb,i)=>inner+=`<text class="tick" x="${x(i)}" y="250" text-anchor="middle">${payloadLabel(pb)}</text>`);root.innerHTML=svgWrap(inner,legend(D.dimensions.protocols));bindTips(root)
  }
  function renderReliability(){
    const failed=D.cases.filter(c=>c.errors>0);const rows=D.cases.filter(c=>c.payload_bytes===state.payload&&c.pipeline===state.pipeline);
    const table=`<div class="matrix-wrap"><table class="matrix"><thead><tr><th>Protocol</th><th>Clients</th><th>Payload</th><th>Pipeline</th><th>Status</th><th>Errors</th></tr></thead><tbody>${rows.map(c=>`<tr><td>${labels[c.protocol]}</td><td>${c.clients}</td><td>${c.payload_label}</td><td>${c.pipeline}</td><td class="${c.errors?'fail':'ok'}">${c.errors?'FAIL':'PASS'}</td><td>${c.errors}</td></tr>`).join('')}</tbody></table></div>`;
    const types=D.summary.failure_types.length?D.summary.failure_types.map(x=>`<div class="issue"><strong>${x.count} occurrences</strong><div class="muted">${esc(x.error)}</div></div>`).join(''):'<div class="issue"><strong>No recorded failures</strong></div>';
    $('#reliability').innerHTML=table+`<div class="issues"><div><b>${failed.length} failed cases</b><div class="muted">${D.summary.total_errors} total errors in the complete matrix</div></div>${types}</div>`;
  }
  function renderAll(){
    barChart('#chart-ops','operations_s',' ops/s',0);barChart('#chart-throughput','combined_mib_s',' MiB/s',2);
    groupedBar('#chart-latency',[{key:'rtt_p50_ms',label:'p50',color:'#5de1c2'},{key:'rtt_p95_ms',label:'p95',color:'#69a7ff'},{key:'rtt_p99_ms',label:'p99',color:'#b99cff'}]);
    barChart('#chart-connect','connect_mean_ms',' ms',2);barChart('#chart-client-cpu','client_cpu_percent','%',1);barChart('#chart-cycles','server_perf_cycles_per_op','',0);barChart('#chart-instructions','server_perf_instructions_per_op','',0);barChart('#chart-context','server_perf_context_switches_per_1k_ops','',0);barChart('#chart-kernel','server_kernel_network_events_per_1k_ops','',0);lineChart('#chart-scaling','combined_mib_s',' MiB/s',2);renderReliability();
  }
  setupMeta();setupStatus();setupControls();setupSummary();renderAll();
})();
