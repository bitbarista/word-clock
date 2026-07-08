// Single-page mobile-first control UI, served from flash.
#pragma once
#include <pgmspace.h>

static const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<title>TIME INVADERS</title>
<style>
:root{--bg:#0d0f14;--card:#161a22;--line:#262c38;--ink:#e8ecf4;--mut:#8b93a5;
--amber:#ffb300;--green:#4be15f;--red:#ff4d4d;--cyan:#4dd9ff;--r:14px}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
body{margin:0;background:var(--bg);color:var(--ink);
font:15px/1.5 ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}
header{position:sticky;top:0;z-index:5;background:#0d0f14ee;backdrop-filter:blur(8px);
border-bottom:1px solid var(--line);padding:14px 16px;display:flex;align-items:baseline;gap:10px}
header h1{margin:0;font-size:17px;letter-spacing:.22em;color:var(--amber);
text-shadow:0 0 12px #ffb30055}
header .st{margin-left:auto;font-size:12px;color:var(--mut)}
main{max-width:560px;margin:0 auto;padding:14px 14px 60px;display:flex;flex-direction:column;gap:14px}
.card{background:var(--card);border:1px solid var(--line);border-radius:var(--r);padding:14px 16px}
.card h2{margin:0 0 10px;font-size:12px;letter-spacing:.18em;color:var(--mut);text-transform:uppercase}
.row{display:flex;align-items:center;gap:12px;padding:7px 0;min-height:36px}
.row label{flex:1}
.row .hint{font-size:12px;color:var(--mut)}
input[type=range]{flex:1.2;accent-color:var(--amber);min-width:110px}
select,input[type=text],input[type=password],input[type=number]{
background:#0d0f14;color:var(--ink);border:1px solid var(--line);border-radius:8px;
padding:8px 10px;font:inherit;max-width:100%}
select{min-width:130px}
input[type=color]{width:42px;height:30px;border:1px solid var(--line);border-radius:8px;
background:none;padding:2px}
.sw{position:relative;width:46px;height:26px;flex:none}
.sw input{opacity:0;width:0;height:0}
.sw i{position:absolute;inset:0;background:#2a3040;border-radius:26px;transition:.2s;cursor:pointer}
.sw i:before{content:"";position:absolute;width:20px;height:20px;left:3px;top:3px;
background:#8b93a5;border-radius:50%;transition:.2s}
.sw input:checked+i{background:var(--amber)}
.sw input:checked+i:before{transform:translateX(20px);background:#0d0f14}
button{background:#20263252;border:1px solid var(--line);color:var(--ink);border-radius:10px;
padding:9px 13px;font:inherit;font-size:13px;cursor:pointer}
button:active{transform:translateY(1px)}
button.acc{border-color:var(--amber);color:var(--amber)}
.btns{display:flex;flex-wrap:wrap;gap:8px;padding-top:4px}
.chips{display:flex;flex-wrap:wrap;gap:8px}
.chip{border:1px solid var(--line);border-radius:999px;padding:7px 14px;font-size:13px;cursor:pointer}
.chip.on{border-color:var(--amber);color:var(--amber);box-shadow:0 0 10px #ffb30033}
#cv{width:100%;image-rendering:pixelated;border-radius:10px;background:#07080b;display:block}
#phrase{text-align:center;color:var(--mut);font-size:13px;letter-spacing:.08em;padding-top:8px;min-height:20px}
.nets{display:flex;flex-direction:column;gap:6px;padding:6px 0}
.nets button{text-align:left;display:flex;justify-content:space-between}
.ok{color:var(--green)} .warn{color:var(--amber)}
progress{width:100%;accent-color:var(--amber)}
.sub{border-top:1px dashed var(--line);margin-top:8px;padding-top:8px}
.banner{display:none;background:#2a2010;border-bottom:1px solid var(--amber);color:var(--amber);
font-size:13px;text-align:center;padding:8px 14px}
</style></head><body>
<header><h1>TIME INVADERS</h1><span class="st" id="hstat">…</span></header>
<div class="banner" id="usbbanner">&#9888; Programming port connected — LED current limited for safety</div>
<main>

<div class="card">
  <canvas id="cv" width="160" height="160"></canvas>
  <div id="phrase"></div>
</div>

<div class="card"><h2>Display</h2>
  <div class="row"><label>Brightness</label>
    <input type="range" id="brightness" min="10" max="255"></div>
  <div class="row"><label>Night dimming</label>
    <span class="sw"><input type="checkbox" id="nightEnabled"><i></i></span></div>
  <div class="row" data-night><label class="hint">Night brightness</label>
    <input type="range" id="nightBright" min="2" max="120"></div>
  <div class="row" data-night><label class="hint">From / to (hour)</label>
    <input type="number" id="nightFrom" min="0" max="23" style="width:64px">
    <input type="number" id="nightTo" min="0" max="23" style="width:64px"></div>
</div>

<div class="card"><h2>Theme</h2>
  <div class="chips" id="themes"></div>
  <div class="row" style="padding-top:12px"><label>Time colour</label>
    <input type="color" id="timeColor"></div>
  <div class="row"><label>Accent colour</label>
    <input type="color" id="accentColor"></div>
</div>

<div class="card"><h2>Animations</h2>
  <div class="row"><label>Arcade animations</label>
    <span class="sw"><input type="checkbox" id="animsEnabled"><i></i></span></div>
  <div class="row"><label>Time-change style</label>
    <select id="transStyle">
      <option value="0">Random</option><option value="1">Pac-Man eats it</option>
      <option value="2">Matrix rain</option><option value="3">Cannon shoot-up</option>
      <option value="4">Invader zap</option><option value="5">Tetris drop</option>
      <option value="6">Gentle fade</option>
    </select></div>
  <div class="hint">Big animations run on 5-minute changes; single minutes crossfade.</div>
  <div class="row"><label>Attract mode</label>
    <span class="sw"><input type="checkbox" id="attractEnabled"><i></i></span></div>
  <div class="row"><label class="hint">Every <b id="attractMinV"></b> min</label>
    <input type="range" id="attractMin" min="5" max="120" step="5"></div>
  <div class="btns">
    <button onclick="act('attract',1)">&#9608; Invaders</button>
    <button onclick="act('attract',2)">&#9608; Ghosts</button>
    <button onclick="act('attract',3)">&#9608; Pac chase</button>
    <button onclick="act('attract',4)">&#9608; Cannon duel</button>
    <button onclick="act('attract',5)">&#9608; Matrix</button>
    <button onclick="act('attract',6)">&#9608; Insert coin</button>
    <button class="acc" onclick="act('transition',-1)">&#9654; Play transition</button>
  </div>
</div>

<div class="card"><h2>Hidden arcade words</h2>
  <div class="row"><label>Word glints</label>
    <span class="sw"><input type="checkbox" id="wordsEnabled"><i></i></span></div>
  <div class="row"><label class="hint">Every <b id="wordsMinV"></b> min</label>
    <input type="range" id="wordsMin" min="2" max="60"></div>
  <div class="btns"><button onclick="act('words',-1)">&#10024; Glint now</button></div>
</div>

<div class="card"><h2>Ambient</h2>
  <div class="row"><label>Wandering Pac-Man + ghost</label>
    <span class="sw"><input type="checkbox" id="pacAmbient"><i></i></span></div>
</div>

<div class="card"><h2>Time</h2>
  <div class="row"><label>Timezone</label>
    <select id="tzsel">
      <option value="GMT0BST,M3.5.0/1,M10.5.0">London</option>
      <option value="CET-1CEST,M3.5.0,M10.5.0/3">Paris / Berlin</option>
      <option value="WET0WEST,M3.5.0/1,M10.5.0">Lisbon</option>
      <option value="EST5EDT,M3.2.0,M11.1.0">New York</option>
      <option value="CST6CDT,M3.2.0,M11.1.0">Chicago</option>
      <option value="PST8PDT,M3.2.0,M11.1.0">Los Angeles</option>
      <option value="AEST-10AEDT,M10.1.0,M4.1.0/3">Sydney</option>
      <option value="JST-9">Tokyo</option>
      <option value="UTC0">UTC</option>
      <option value="custom">Custom…</option>
    </select></div>
  <div class="row" id="tzcustomrow" style="display:none"><label class="hint">POSIX TZ</label>
    <input type="text" id="tz" style="flex:2"></div>
  <div class="hint">Time syncs from the internet automatically when WiFi is up.</div>
  <div class="btns"><button class="acc" onclick="syncTime()">&#8986; Sync time from this device</button></div>
</div>

<div class="card"><h2>Network</h2>
  <div class="hint" id="netinfo"></div>
  <div class="btns"><button onclick="scan()">&#128246; Scan networks</button></div>
  <div class="nets" id="nets"></div>
  <div class="row"><input type="text" id="ssid" placeholder="SSID" style="flex:1">
    <input type="password" id="pass" placeholder="password" style="flex:1"></div>
  <div class="btns"><button class="acc" onclick="joinWifi()">Join &amp; reboot</button></div>
</div>

<div class="card"><h2>Panel</h2>
  <div class="row"><label>Rotate</label>
    <select id="mapRotate"><option value="0">0&deg;</option><option value="1">90&deg;</option>
    <option value="2">180&deg;</option><option value="3">270&deg;</option></select></div>
  <div class="row"><label>Serpentine rows</label>
    <span class="sw"><input type="checkbox" id="mapSerp"><i></i></span></div>
  <div class="row"><label>Mirror</label>
    <span class="sw"><input type="checkbox" id="mapFlip"><i></i></span></div>
  <div class="btns">
    <button onclick="act('maptest')">Orientation test</button>
    <button onclick="act('identify')">Identify flash</button>
  </div>
  <div class="hint">Test: red = top-left, green = top-right, blue = bottom-left.</div>
  <div class="row sub"><label>Power budget (mA)</label>
    <input type="number" id="powerMa" min="300" max="3000" step="100" style="width:90px"></div>
  <div class="row"><label class="hint">Safe budget while flashing (mA)</label>
    <input type="number" id="usbSafeMa" min="200" max="900" step="50" style="width:90px"></div>
  <div class="hint">Auto-clamps to the safe budget whenever the programming port has
    a computer attached — its onboard diode isn't rated for full panel current
    and a laptop port can't source it anyway.</div>
</div>

<div class="card"><h2>System</h2>
  <div class="row"><label>Hostname</label>
    <input type="text" id="hostname" style="flex:1"></div>
  <div class="hint" id="fwline"></div>
  <div class="row sub"><input type="file" id="fwfile" accept=".bin" style="flex:1">
    <button class="acc" onclick="doOta()">Update</button></div>
  <progress id="otaprog" max="100" value="0" style="display:none"></progress>
  <div class="btns sub"><button onclick="fetch('/api/restart',{method:'POST'})">Restart clock</button></div>
</div>

</main>
<script>
const GRID=["ITKISAHIGHSCOREZ","TWENTYINSERTCOIN","FOURTEENSIXTEENA","SEVENTEENTWELVEB",
"EIGHTEENNINETEEN","THIRTEENQUARTERS","THREELEVENTENZAP","TWONEFIVEHALFPAC",
"MINUTESXPASTOPOW","TWONETHREEIGHTGO","SEVENINEFOURFIVE","SIXTENELEVENGAME",
"TWELVEOCLOCKOVER","SPACEINVADERSPEW","GALAGADONKEYKONG","ASTEROIDSQBERTUP"];
const THEMES=[["Arcade","#ffffff","#ffb300"],["Matrix","#6fff7d","#2e9440"],
["Amber CRT","#ffb300","#ff8c3b"],["Ice","#9fd8ff","#4dd9ff"],["Custom",null,null]];
const $=id=>document.getElementById(id);
let cfg={},debTimer;

function post(url,obj){return fetch(url,{method:'POST',headers:{'Content-Type':'application/json'},
body:JSON.stringify(obj)});}
function saveCfg(patch){Object.assign(cfg,patch);clearTimeout(debTimer);
debTimer=setTimeout(()=>post('/api/config',patch),350);}
function act(what,param){post('/api/action',{do:what,param:param==null?-1:param});}

function bindRange(id,key,lab){const el=$(id);el.addEventListener('input',()=>{
if(lab)$(lab).textContent=el.value;saveCfg({[key]:+el.value});});}
function bindSw(id,key){$(id).addEventListener('change',e=>{saveCfg({[key]:e.target.checked});
if(id==='nightEnabled')nightVis();});}
function bindSel(id,key){$(id).addEventListener('change',e=>saveCfg({[key]:+e.target.value}));}
function bindCol(id,key){$(id).addEventListener('change',e=>{saveCfg({[key]:e.target.value,theme:4});
markTheme(4);});}
function nightVis(){document.querySelectorAll('[data-night]').forEach(r=>
r.style.display=$('nightEnabled').checked?'flex':'none');}

function markTheme(i){document.querySelectorAll('#themes .chip').forEach((c,k)=>
c.classList.toggle('on',k===i));}
THEMES.forEach((t,i)=>{const c=document.createElement('div');c.className='chip';
c.textContent=t[0];c.onclick=()=>{markTheme(i);
if(t[1]){$('timeColor').value=t[1];$('accentColor').value=t[2];
saveCfg({theme:i,timeColor:t[1],accentColor:t[2]});}else saveCfg({theme:i});};
$('themes').appendChild(c);});

async function loadCfg(){cfg=await(await fetch('/api/config')).json();
for(const k of ['brightness','nightBright','nightFrom','nightTo','attractMin','wordsMin','powerMa','usbSafeMa'])
$(k).value=cfg[k];
$('attractMinV').textContent=cfg.attractMin;$('wordsMinV').textContent=cfg.wordsMin;
for(const k of ['nightEnabled','animsEnabled','attractEnabled','wordsEnabled','pacAmbient','mapSerp','mapFlip'])
$(k).checked=cfg[k];
$('transStyle').value=cfg.transStyle;$('mapRotate').value=cfg.mapRotate;
$('timeColor').value=cfg.timeColor;$('accentColor').value=cfg.accentColor;
$('hostname').value=cfg.hostname;$('tz').value=cfg.tz;
const opt=[...$('tzsel').options].find(o=>o.value===cfg.tz);
$('tzsel').value=opt?cfg.tz:'custom';
$('tzcustomrow').style.display=opt?'none':'flex';
markTheme(cfg.theme);nightVis();}

bindRange('brightness','brightness');bindRange('nightBright','nightBright');
bindRange('attractMin','attractMin','attractMinV');bindRange('wordsMin','wordsMin','wordsMinV');
$('nightFrom').addEventListener('change',e=>saveCfg({nightFrom:+e.target.value}));
$('nightTo').addEventListener('change',e=>saveCfg({nightTo:+e.target.value}));
$('powerMa').addEventListener('change',e=>saveCfg({powerMa:+e.target.value}));
$('usbSafeMa').addEventListener('change',e=>saveCfg({usbSafeMa:+e.target.value}));
bindSw('nightEnabled','nightEnabled');bindSw('animsEnabled','animsEnabled');
bindSw('attractEnabled','attractEnabled');bindSw('wordsEnabled','wordsEnabled');
bindSw('pacAmbient','pacAmbient');bindSw('mapSerp','mapSerp');bindSw('mapFlip','mapFlip');
bindSel('transStyle','transStyle');bindSel('mapRotate','mapRotate');
bindCol('timeColor','timeColor');bindCol('accentColor','accentColor');
$('hostname').addEventListener('change',e=>saveCfg({hostname:e.target.value}));
$('tzsel').addEventListener('change',e=>{const v=e.target.value;
$('tzcustomrow').style.display=v==='custom'?'flex':'none';
if(v!=='custom'){$('tz').value=v;saveCfg({tz:v});}});
$('tz').addEventListener('change',e=>saveCfg({tz:e.target.value}));

function syncTime(){post('/api/time',{epoch:Math.floor(Date.now()/1000)});}

async function scan(){$('nets').innerHTML='<span class="hint">scanning…</span>';
for(let i=0;i<10;i++){const r=await(await fetch('/api/scan')).json();
if(!r.scanning){$('nets').innerHTML='';(r.nets||[]).forEach(n=>{
const b=document.createElement('button');
b.innerHTML=`<span>${n.ssid}</span><span class="hint">${n.rssi} dBm</span>`;
b.onclick=()=>$('ssid').value=n.ssid;$('nets').appendChild(b);});return;}
await new Promise(res=>setTimeout(res,1200));}}
function joinWifi(){post('/api/wifi',{ssid:$('ssid').value,pass:$('pass').value});
alert('Joining new network — the clock will reboot.');}

function doOta(){const f=$('fwfile').files[0];if(!f)return alert('Choose a .bin first');
const xhr=new XMLHttpRequest();xhr.open('POST','/update');
$('otaprog').style.display='block';
xhr.upload.onprogress=e=>$('otaprog').value=e.loaded/e.total*100;
xhr.onload=()=>alert(xhr.status==200?'Updated — rebooting.':'Update failed');
const fd=new FormData();fd.append('firmware',f);xhr.send(fd);}

// live preview
const cv=$('cv'),ctx=cv.getContext('2d');
async function preview(){if(document.hidden)return;
try{const hex=await(await fetch('/api/preview')).text();
ctx.fillStyle='#07080b';ctx.fillRect(0,0,160,160);
ctx.font='7px monospace';ctx.textAlign='center';ctx.textBaseline='middle';
for(let i=0;i<256;i++){const r=(i/16)|0,c=i%16;
const col='#'+hex.substr(i*6,6);
if(col!=='#000000'){ctx.fillStyle=col;ctx.fillRect(c*10+1,r*10+1,8,8);ctx.fillStyle='#000';}
else ctx.fillStyle='#252a35';
ctx.fillText(GRID[r][c],c*10+5,r*10+5.5);}}catch(e){}}
async function status(){try{const s=await(await fetch('/api/status')).json();
$('hstat').textContent=`${s.time} · ${s.ip}`;
$('phrase').textContent=s.phrase;
$('netinfo').innerHTML=s.ap?'<span class="warn">Access-point mode — join a WiFi below for auto time.</span>'
:`Connected · ${s.ip} · ${s.rssi} dBm`;
$('fwline').textContent=`Firmware v${s.fw} · ${s.host}.local`;
$('usbbanner').style.display=s.usbLimited?'block':'none';}catch(e){}}
setInterval(preview,1000);setInterval(status,3000);
loadCfg();status();preview();
</script>
</body></html>
)HTML";
