async function loadJSON(path){const r=await fetch(path,{cache:"no-store"});if(!r.ok)throw new Error(path);return r.json()}
function metric(name,value,sub=""){return `<div class="card metric"><span>${name}</span><strong>${value}</strong>${sub?`<span>${sub}</span>`:""}</div>`}
async function renderGate(){
  try{
    const q=await loadJSON("assets/data/quality-gate.json");
    if(q.result!=="PASS") throw new Error("gate not PASS");
    const robot=q.robot||{}, a=q.audio_stream_tests||{}, v=q.verified_transfers||{};
    document.querySelector("#gate-badge").textContent="PASS";
    document.querySelector("#gate-badge").className="badge pass";
    document.querySelector("#gate").innerHTML=
      metric("Robot regression",`${robot.PASS||0} / ${robot.TOTAL||0}`,"PASS")+
      metric("Audio Stream Tests",`${a.transfers_passed||0} / ${a.transfers_total||0}`,"PASS")+
      metric("Verified checks / transfers",`${v.passed||0} / ${v.total||0}`,`compression errors ${q.compression_errors||0} · decompression errors ${q.decompression_errors||0}`);
  }catch(e){}
}
function levelCell(x){
  if(!x)return "—";
  const r=Number(x.reduction_percent);
  const saved=Number(x.saved_bytes||0).toLocaleString();
  return `<span class="level-main">${Number.isFinite(r)?r.toFixed(2)+"%":"—"}</span><span class="level-sub">${saved} B saved</span>`;
}
async function renderLevels(){
  try{
    const p=await loadJSON("assets/data/performance.json");
    if(p.status && p.status!=="PASS")return;
    const rows=p.compression_levels||p.levels||[];
    for(const transport of ["stcp-tcp","stcp-udp"]){
      const tr=[...document.querySelectorAll("#levels tbody tr")].find(x=>x.firstElementChild.textContent.toLowerCase()===transport);
      if(!tr)continue;
      for(let level=1;level<=5;level++){
        const x=rows.find(r=>String(r.transport).toLowerCase()===transport && Number(r.level)===level);
        tr.children[level].innerHTML=levelCell(x);
      }
    }
  }catch(e){}
}
renderGate();renderLevels();