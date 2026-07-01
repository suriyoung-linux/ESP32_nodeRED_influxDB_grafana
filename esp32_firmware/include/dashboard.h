#ifndef DASHBOARD_H
#define DASHBOARD_H

// Dashboard HTML/CSS/JS stored in flash (PROGMEM)
// Self-contained UI: no CDN dependency, realtime via WebSocket.

static const char DASHBOARD_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="th">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 Dashboard</title>
<style>
  :root {
    color-scheme: light;
    --bg: #f5f7fb;
    --panel: #ffffff;
    --panel-soft: #f8fafc;
    --text: #172033;
    --muted: #667085;
    --line: #e4e7ec;
    --line-strong: #cfd6e3;
    --green: #13a36f;
    --red: #dc3e42;
    --blue: #2563eb;
    --cyan: #0891b2;
    --amber: #d97706;
    --rose: #e11d48;
    --shadow: 0 10px 30px rgba(15, 23, 42, .07);
  }

  * { box-sizing: border-box; }

  body {
    margin: 0;
    min-height: 100vh;
    background:
      radial-gradient(circle at 18% 0%, rgba(37, 99, 235, .08), transparent 28rem),
      linear-gradient(180deg, #fbfcff 0%, var(--bg) 42%, #eef2f7 100%);
    color: var(--text);
    font-family: Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
    letter-spacing: 0;
  }

  button, input { font: inherit; }
  button { cursor: pointer; }

  .shell {
    width: min(1180px, calc(100% - 28px));
    margin: 0 auto;
    padding: 22px 0 28px;
  }

  .topbar {
    display: grid;
    grid-template-columns: 1fr auto;
    gap: 16px;
    align-items: center;
    margin-bottom: 18px;
  }

  .brand {
    display: flex;
    gap: 12px;
    align-items: center;
    min-width: 0;
  }

  .brand-mark {
    width: 44px;
    height: 44px;
    border-radius: 8px;
    display: grid;
    place-items: center;
    background: #111827;
    color: #fff;
    font-weight: 800;
    letter-spacing: 0;
    box-shadow: var(--shadow);
    flex: 0 0 auto;
  }

  h1 {
    margin: 0;
    font-size: clamp(1.35rem, 3vw, 2rem);
    line-height: 1.12;
    letter-spacing: 0;
  }

  .subtitle {
    margin-top: 5px;
    color: var(--muted);
    font: 600 .88rem ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
    overflow-wrap: anywhere;
  }

  .live-pill {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    justify-self: end;
    min-height: 40px;
    padding: 8px 12px;
    border: 1px solid var(--line);
    border-radius: 999px;
    background: rgba(255, 255, 255, .86);
    box-shadow: 0 4px 18px rgba(15, 23, 42, .06);
    white-space: nowrap;
  }

  .dot {
    width: 9px;
    height: 9px;
    border-radius: 50%;
    background: var(--red);
    animation: pulse 1.8s infinite;
  }

  .dot.ok { background: var(--green); }
  @keyframes pulse { 50% { opacity: .35; } }

  .summary {
    display: grid;
    grid-template-columns: repeat(4, minmax(0, 1fr));
    gap: 10px;
    margin-bottom: 14px;
  }

  .summary-item {
    min-height: 78px;
    padding: 12px 14px;
    border: 1px solid var(--line);
    border-radius: 8px;
    background: rgba(255, 255, 255, .92);
    box-shadow: 0 5px 20px rgba(15, 23, 42, .045);
  }

  .summary-label,
  .label {
    color: var(--muted);
    font-size: .74rem;
    font-weight: 800;
    text-transform: uppercase;
    letter-spacing: .08em;
  }

  .summary-value {
    margin-top: 8px;
    font-size: 1.45rem;
    line-height: 1;
    font-weight: 800;
    letter-spacing: 0;
  }

  .summary-note {
    margin-top: 7px;
    color: var(--muted);
    font-size: .8rem;
    overflow-wrap: anywhere;
  }

  .grid {
    display: grid;
    grid-template-columns: 1.05fr .95fr .95fr;
    gap: 14px;
    align-items: start;
  }

  .panel {
    border: 1px solid var(--line);
    border-radius: 8px;
    background: var(--panel);
    box-shadow: var(--shadow);
    overflow: hidden;
  }

  .span-2 { grid-column: span 2; }
  .span-3 { grid-column: 1 / -1; }

  .panel-head {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 10px;
    padding: 14px 16px 0;
  }

  .panel-title {
    margin: 0;
    font-size: .95rem;
    font-weight: 800;
    letter-spacing: 0;
  }

  .panel-meta {
    color: var(--muted);
    font-size: .78rem;
    white-space: nowrap;
  }

  .panel-body { padding: 14px 16px 16px; }

  .relay-list {
    display: grid;
    gap: 10px;
  }

  .relay-row {
    display: grid;
    grid-template-columns: auto 1fr auto;
    gap: 12px;
    align-items: center;
    min-height: 72px;
    padding: 12px;
    border: 1px solid var(--line);
    border-radius: 8px;
    background: var(--panel-soft);
  }

  .relay-row.on {
    background: #f0fbf6;
    border-color: #a8e8cf;
  }

  .relay-icon {
    width: 38px;
    height: 38px;
    border-radius: 8px;
    display: grid;
    place-items: center;
    background: #e8edf5;
    color: #475467;
    font-weight: 900;
  }

  .relay-row.on .relay-icon {
    background: #d9f8e9;
    color: var(--green);
  }

  .relay-name {
    font-weight: 800;
  }

  .relay-state {
    margin-top: 2px;
    color: var(--muted);
    font-size: .82rem;
  }

  .relay-row.on .relay-state { color: #087a53; }

  .action-btn {
    min-width: 72px;
    min-height: 38px;
    border: 1px solid var(--line-strong);
    border-radius: 8px;
    background: #fff;
    color: #344054;
    font-weight: 800;
    transition: transform .15s ease, border-color .15s ease, background .15s ease;
  }

  .action-btn:hover {
    transform: translateY(-1px);
    border-color: #98a2b3;
  }

  .relay-row.on .action-btn {
    border-color: #0f8f62;
    background: var(--green);
    color: #fff;
  }

  .metric-xl {
    display: flex;
    align-items: baseline;
    justify-content: center;
    gap: 8px;
    min-height: 118px;
  }

  .metric-number {
    font-size: clamp(3.1rem, 9vw, 5.4rem);
    line-height: .9;
    font-weight: 850;
    letter-spacing: 0;
  }

  .unit {
    color: var(--muted);
    font-weight: 800;
  }

  .metric-grid {
    display: grid;
    grid-template-columns: repeat(2, minmax(0, 1fr));
    gap: 10px;
  }

  .metric-box {
    min-height: 100px;
    padding: 12px;
    border: 1px solid var(--line);
    border-radius: 8px;
    background: var(--panel-soft);
  }

  .metric-value {
    margin-top: 8px;
    font-size: 2rem;
    line-height: 1;
    font-weight: 850;
  }

  .full { grid-column: 1 / -1; }

  .gauge {
    width: 100%;
    height: 7px;
    margin-top: 12px;
    overflow: hidden;
    border-radius: 999px;
    background: #e6ebf2;
  }

  .bar {
    width: 0%;
    height: 100%;
    border-radius: 999px;
    background: var(--blue);
    transition: width .45s ease, background .2s ease;
  }

  .fineprint {
    margin-top: 10px;
    color: var(--muted);
    font-size: .78rem;
  }

  .badge {
    display: inline-flex;
    align-items: center;
    min-height: 22px;
    padding: 3px 8px;
    border-radius: 999px;
    background: #eef2f7;
    color: #475467;
    font-size: .72rem;
    font-weight: 850;
    letter-spacing: .04em;
    text-transform: uppercase;
  }

  .badge.ok { background: #dcfaeb; color: #087a53; }
  .badge.warn { background: #fff2cc; color: #9a5b00; }
  .badge.bad { background: #ffe4e8; color: #b4232a; }

  .info-list {
    display: grid;
    gap: 2px;
  }

  .info-row {
    display: grid;
    grid-template-columns: minmax(94px, auto) 1fr;
    gap: 12px;
    align-items: center;
    min-height: 34px;
    border-bottom: 1px solid #eef1f5;
  }

  .info-row:last-child { border-bottom: 0; }

  .info-label {
    color: var(--muted);
    font-size: .8rem;
    font-weight: 750;
  }

  .info-value {
    min-width: 0;
    color: #243044;
    font: 750 .86rem ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
    text-align: right;
    overflow-wrap: anywhere;
  }

  .topics {
    display: grid;
    grid-template-columns: repeat(2, minmax(0, 1fr));
    gap: 14px;
  }

  .topic-group-title {
    margin: 0 0 8px;
    color: #344054;
    font-size: .8rem;
    font-weight: 850;
    text-transform: uppercase;
    letter-spacing: .08em;
  }

  .topic-row {
    display: grid;
    grid-template-columns: 42px 1fr;
    gap: 8px;
    align-items: start;
    padding: 8px 0;
    border-bottom: 1px solid #eef1f5;
  }

  .topic-row:last-child { border-bottom: 0; }

  .dir {
    display: inline-flex;
    justify-content: center;
    padding: 3px 6px;
    border-radius: 6px;
    font-size: .66rem;
    font-weight: 900;
  }

  .pub { background: #e0eaff; color: #174ea6; }
  .sub { background: #def7ea; color: #087a53; }

  code {
    display: block;
    max-width: 100%;
    color: #1f2a44;
    font: 750 .78rem ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
    overflow-wrap: anywhere;
  }

  .topic-desc {
    margin-top: 3px;
    color: var(--muted);
    font-size: .75rem;
  }

  .example {
    margin-top: 10px;
    padding: 10px;
    border: 1px solid var(--line);
    border-radius: 8px;
    background: #f8fafc;
    color: #475467;
    font: 700 .76rem ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
    overflow-wrap: anywhere;
  }

  .footer {
    margin: 18px 0 0;
    color: var(--muted);
    text-align: center;
    font-size: .78rem;
  }

  .c-rose { color: var(--rose); }
  .c-cyan { color: var(--cyan); }
  .c-blue { color: var(--blue); }
  .c-amber { color: var(--amber); }
  .c-green { color: var(--green); }
  .c-red { color: var(--red); }
  .aqi-1 { color: #0f9f6e; }
  .aqi-2 { color: #65a30d; }
  .aqi-3 { color: #ca8a04; }
  .aqi-4 { color: #ea580c; }
  .aqi-5 { color: #dc2626; }

  @media (max-width: 980px) {
    .summary { grid-template-columns: repeat(2, minmax(0, 1fr)); }
    .grid { grid-template-columns: repeat(2, minmax(0, 1fr)); }
    .span-2, .span-3 { grid-column: 1 / -1; }
  }

  @media (max-width: 640px) {
    .shell {
      width: min(100% - 20px, 1180px);
      padding-top: 14px;
    }

    .topbar {
      grid-template-columns: 1fr;
    }

    .live-pill {
      justify-self: stretch;
      justify-content: center;
      border-radius: 8px;
    }

    .summary,
    .grid,
    .topics,
    .metric-grid {
      grid-template-columns: 1fr;
    }

    .metric-box.full { grid-column: auto; }
    .brand-mark { width: 40px; height: 40px; }
    .panel-head, .panel-body { padding-left: 12px; padding-right: 12px; }
  }
</style>
</head>
<body>
<main class="shell">
  <header class="topbar">
    <div class="brand">
      <div class="brand-mark">32</div>
      <div>
        <h1>ESP32 Control Dashboard</h1>
        <div class="subtitle" id="ip-label">กำลังเชื่อมต่อ...</div>
      </div>
    </div>
    <div class="live-pill">
      <span class="dot" id="live-dot"></span>
      <strong id="live-label">Connecting</strong>
      <span class="panel-meta" id="last-update"></span>
    </div>
  </header>

  <section class="summary" aria-label="System summary">
    <div class="summary-item">
      <div class="summary-label">DS18B20</div>
      <div class="summary-value c-rose"><span id="summary-ds18">--</span><span class="unit"> °C</span></div>
      <div class="summary-note">GPIO 14</div>
    </div>
    <div class="summary-item">
      <div class="summary-label">XY-MD03</div>
      <div class="summary-value c-cyan"><span id="summary-xymd">--</span><span class="unit"> %RH</span></div>
      <div class="summary-note" id="summary-xymd-temp">Temp -- °C</div>
    </div>
    <div class="summary-item">
      <div class="summary-label">Relay</div>
      <div class="summary-value c-green" id="summary-relay">--/3</div>
      <div class="summary-note">Active outputs</div>
    </div>
    <div class="summary-item">
      <div class="summary-label">Network</div>
      <div class="summary-value c-blue" id="summary-rssi">--</div>
      <div class="summary-note" id="summary-mqtt">MQTT --</div>
    </div>
  </section>

  <section class="grid">
    <article class="panel">
      <div class="panel-head">
        <h2 class="panel-title">Relay Control</h2>
        <span class="panel-meta">GPIO 17 / 16 / 4</span>
      </div>
      <div class="panel-body">
        <div class="relay-list" id="relay-container"></div>
      </div>
    </article>

    <article class="panel">
      <div class="panel-head">
        <h2 class="panel-title">DS18B20 Temperature</h2>
        <span id="ds18-sim-badge"></span>
      </div>
      <div class="panel-body">
        <div class="metric-xl">
          <span class="metric-number c-rose" id="ds18-temp">--</span>
          <span class="unit">°C</span>
        </div>
        <div class="label">Measured range</div>
        <div class="gauge"><div class="bar" id="ds18-bar"></div></div>
        <div class="fineprint">Mapped display range: -10°C to 50°C · Sensor range: -55°C to 125°C</div>
      </div>
    </article>

    <article class="panel">
      <div class="panel-head">
        <h2 class="panel-title">XY-MD03 Sensor</h2>
        <span id="xymd-sim-badge"></span>
      </div>
      <div class="panel-body">
        <div class="metric-grid">
          <div class="metric-box">
            <div class="label">Temperature</div>
            <div class="metric-value c-cyan"><span id="xymd-temp">--</span><span class="unit"> °C</span></div>
            <div class="gauge"><div class="bar" id="xymd-temp-bar"></div></div>
          </div>
          <div class="metric-box">
            <div class="label">Humidity</div>
            <div class="metric-value c-blue" id="xymd-hum">--</div>
            <div class="gauge"><div class="bar" id="xymd-hum-bar"></div></div>
          </div>
          <div class="fineprint full">Modbus RTU · Serial0 · Slave ID 2 · 9600 8N1</div>
        </div>
      </div>
    </article>

    <article class="panel">
      <div class="panel-head">
        <h2 class="panel-title">Weather</h2>
        <span class="panel-meta" id="w-city">--</span>
      </div>
      <div class="panel-body">
        <div class="metric-grid">
          <div class="metric-box">
            <div class="label">Temperature</div>
            <div class="metric-value c-amber"><span id="w-temp">--</span><span class="unit"> °C</span></div>
          </div>
          <div class="metric-box">
            <div class="label">Humidity</div>
            <div class="metric-value c-blue" id="w-hum">--</div>
          </div>
          <div class="metric-box full">
            <div class="label">Rain chance <span style="float:right;color:var(--blue)" id="w-rain-pct">--%</span></div>
            <div class="gauge"><div class="bar" id="w-rain-bar"></div></div>
          </div>
          <div class="metric-box">
            <div class="label">Air Quality</div>
            <div class="metric-value" id="w-aqi-label">--</div>
            <div class="fineprint">AQI <span id="w-aqi-num">-</span></div>
          </div>
          <div class="metric-box">
            <div class="label">PM2.5</div>
            <div class="metric-value c-amber"><span id="w-pm25">--</span></div>
            <div class="fineprint">µg/m³</div>
          </div>
        </div>
      </div>
    </article>

    <article class="panel span-2">
      <div class="panel-head">
        <h2 class="panel-title">WiFi &amp; System</h2>
        <span class="panel-meta">Realtime device info</span>
      </div>
      <div class="panel-body">
        <div class="info-list">
          <div class="info-row"><span class="info-label">SSID</span><span class="info-value" id="wifi-ssid">--</span></div>
          <div class="info-row"><span class="info-label">IP Address</span><span class="info-value c-green" id="wifi-ip">--</span></div>
          <div class="info-row"><span class="info-label">RSSI</span><span class="info-value" id="wifi-rssi-text">--</span></div>
          <div class="info-row"><span class="info-label">MAC Address</span><span class="info-value" id="wifi-mac">--</span></div>
          <div class="info-row"><span class="info-label">Free Heap</span><span class="info-value" id="sys-heap">-- KB</span></div>
          <div class="info-row"><span class="info-label">Uptime</span><span class="info-value" id="sys-uptime">--</span></div>
        </div>
        <div class="fineprint">Signal strength <span id="wifi-rssi-val">-- dBm</span></div>
        <div class="gauge"><div class="bar" id="wifi-signal-bar"></div></div>
      </div>
    </article>

    <article class="panel span-3">
      <div class="panel-head">
        <h2 class="panel-title">MQTT Broker</h2>
        <div>
          <span id="mqtt-status-badge"></span>
          <span class="panel-meta" id="mqtt-host-label">--</span>
        </div>
      </div>
      <div class="panel-body">
        <div class="topics">
          <div>
            <p class="topic-group-title">Publish Topics</p>
            <div class="topic-row"><span class="dir pub">PUB</span><div><code id="t-telemetry">--</code><div class="topic-desc">ข้อมูลทั้งหมด (ทุก 5s)</div></div></div>
            <div class="topic-row"><span class="dir pub">PUB</span><div><code id="t-status">--</code><div class="topic-desc">online / offline (LWT)</div></div></div>
            <div class="topic-row"><span class="dir pub">PUB</span><div><code id="t-r1-state">--</code><div class="topic-desc">Relay 1 state</div></div></div>
            <div class="topic-row"><span class="dir pub">PUB</span><div><code id="t-r2-state">--</code><div class="topic-desc">Relay 2 state</div></div></div>
            <div class="topic-row"><span class="dir pub">PUB</span><div><code id="t-r3-state">--</code><div class="topic-desc">Relay 3 state</div></div></div>
          </div>
          <div>
            <p class="topic-group-title">Subscribe Topics</p>
            <div class="topic-row"><span class="dir sub">SUB</span><div><code id="t-r1-set">--</code><div class="topic-desc">ON / OFF / TOGGLE</div></div></div>
            <div class="topic-row"><span class="dir sub">SUB</span><div><code id="t-r2-set">--</code><div class="topic-desc">ON / OFF / TOGGLE</div></div></div>
            <div class="topic-row"><span class="dir sub">SUB</span><div><code id="t-r3-set">--</code><div class="topic-desc">ON / OFF / TOGGLE</div></div></div>
            <div class="example">mosquitto_pub -h broker.hivemq.com -t <span id="t-r1-set-ex">--</span> -m ON</div>
          </div>
        </div>
      </div>
    </article>
  </section>

  <p class="footer">ESP32 Local Dashboard · Real-time via WebSocket</p>
</main>

<script>
const wsUrl = `ws://${location.hostname}/ws`;
let ws, reconnectTimer;

function connect() {
  ws = new WebSocket(wsUrl);
  ws.onopen = () => { setLive(true); clearTimeout(reconnectTimer); };
  ws.onclose = () => { setLive(false); reconnectTimer = setTimeout(connect, 3000); };
  ws.onerror = () => ws.close();
  ws.onmessage = (e) => { try { render(JSON.parse(e.data)); } catch (_) {} };
}

function setLive(ok) {
  const dot = document.getElementById('live-dot');
  dot.className = ok ? 'dot ok' : 'dot';
  document.getElementById('live-label').textContent = ok ? 'Live' : 'Disconnected';
}

function badge(text, type) {
  return `<span class="badge ${type || ''}">${text}</span>`;
}

function setText(id, value) {
  const el = document.getElementById(id);
  if (el) el.textContent = value;
}

function setBar(id, pct, color) {
  const el = document.getElementById(id);
  if (!el) return;
  el.style.width = Math.max(0, Math.min(100, pct)) + '%';
  if (color) el.style.background = color;
}

function render(d) {
  setText('last-update', new Date().toLocaleTimeString('th-TH'));

  if (d.relay) {
    const rc = document.getElementById('relay-container');
    rc.innerHTML = '';
    let onCount = 0;

    d.relay.forEach((on, i) => {
      if (on) onCount++;
      const n = i + 1;
      const row = document.createElement('div');
      row.className = `relay-row ${on ? 'on' : ''}`;
      row.innerHTML = `
        <div class="relay-icon">${on ? 'ON' : 'OFF'}</div>
        <div>
          <div class="relay-name">Relay ${n}</div>
          <div class="relay-state">${on ? 'เปิดอยู่' : 'ปิดอยู่'}</div>
        </div>
        <button class="action-btn" onclick="toggleRelay(${n})">${on ? 'ปิด' : 'เปิด'}</button>`;
      rc.appendChild(row);
    });

    setText('summary-relay', `${onCount}/3`);
  }

  if (d.ds18 !== undefined) {
    const t = parseFloat(d.ds18.temp);
    setText('ds18-temp', t.toFixed(2));
    setText('summary-ds18', t.toFixed(1));
    setBar('ds18-bar', (t + 10) / 60 * 100, '#e11d48');
    document.getElementById('ds18-sim-badge').innerHTML =
      d.ds18.sim ? badge('SIM', 'warn') : badge('LIVE', 'ok');
  }

  if (d.xymd !== undefined) {
    const xt = parseFloat(d.xymd.temp);
    const xh = parseFloat(d.xymd.hum);
    setText('xymd-temp', xt.toFixed(1));
    setText('xymd-hum', xh.toFixed(1) + '%');
    setText('summary-xymd', xh.toFixed(1));
    setText('summary-xymd-temp', `Temp ${xt.toFixed(1)} °C`);
    setBar('xymd-temp-bar', (xt / 50) * 100, '#0891b2');
    setBar('xymd-hum-bar', xh, '#2563eb');
    document.getElementById('xymd-sim-badge').innerHTML =
      d.xymd.sim ? badge('SIM', 'warn') : badge('LIVE', 'ok');
  }

  if (d.weather && d.weather.valid) {
    const w = d.weather;
    setText('w-temp', Number(w.temp).toFixed(1));
    setText('w-hum', w.hum + '%');
    setText('w-rain-pct', w.rain + '%');
    setBar('w-rain-bar', w.rain, '#2563eb');
    setText('w-pm25', Number(w.pm25).toFixed(1));
    setText('w-aqi-num', w.aqi);
    setText('w-city', w.city || '--');

    const aqiEl = document.getElementById('w-aqi-label');
    aqiEl.textContent = w.aqiLabel;
    aqiEl.className = `metric-value aqi-${w.aqi}`;
  }

  if (d.wifi) {
    const wf = d.wifi;
    setText('wifi-ssid', wf.ssid || '--');
    setText('wifi-ip', wf.ip || '--');
    setText('wifi-mac', wf.mac || '--');
    setText('ip-label', 'http://' + (wf.ip || '...'));

    const rssi = wf.rssi || -100;
    const pct = Math.max(0, Math.min(100, (rssi + 100) * 2));
    const color = pct > 60 ? '#13a36f' : pct > 30 ? '#d97706' : '#dc3e42';

    setText('wifi-rssi-val', rssi + ' dBm');
    setText('wifi-rssi-text', rssi + ' dBm');
    setText('summary-rssi', rssi + ' dBm');
    document.getElementById('wifi-rssi-text').style.color = color;
    document.getElementById('summary-rssi').style.color = color;
    setBar('wifi-signal-bar', pct, color);
  }

  if (d.mqtt) {
    const m = d.mqtt;
    setText('mqtt-host-label', m.host + ':' + m.port);
    setText('summary-mqtt', m.connected ? 'MQTT connected' : 'MQTT disconnected');
    document.getElementById('mqtt-status-badge').innerHTML =
      m.connected ? badge('Connected', 'ok') : badge('Disconnected', 'bad');

    setText('t-telemetry', m.t_telemetry);
    setText('t-status', m.t_status);
    setText('t-r1-state', m.t_r1_state);
    setText('t-r2-state', m.t_r2_state);
    setText('t-r3-state', m.t_r3_state);
    setText('t-r1-set', m.t_r1_set);
    setText('t-r2-set', m.t_r2_set);
    setText('t-r3-set', m.t_r3_set);
    setText('t-r1-set-ex', m.t_r1_set);
  }

  if (d.sys) {
    setText('sys-heap', (d.sys.heap / 1024).toFixed(1) + ' KB');
    setText('sys-uptime', fmtUptime(d.sys.uptime));
  }
}

function toggleRelay(n) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({ cmd: 'relay', n }));
  }
}

function fmtUptime(sec) {
  const d = Math.floor(sec / 86400);
  const h = Math.floor((sec % 86400) / 3600);
  const m = Math.floor((sec % 3600) / 60);
  const s = sec % 60;
  return d > 0 ? `${d}d ${h}h ${m}m` : `${h}h ${m}m ${s}s`;
}

connect();
</script>
</body>
</html>
)rawhtml";

#endif // DASHBOARD_H
