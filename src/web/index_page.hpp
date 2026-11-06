#pragma once

// =============================================================================
//  index_page.hpp — HTML главной страницы (SPA)
// =============================================================================

namespace relay::web::index_page
{

  inline constexpr char kHtml[] = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>РЕЛЕ — управление</title>

    <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      font-family: 'Segoe UI', Roboto, system-ui, -apple-system, sans-serif;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      gap: 30px;
      padding: 24px;
      color: #eaeaea;
      background: radial-gradient(circle at 50% 0%, #242a33 0%, #15181e 55%, #0d0f13 100%);
    }

    .status-bar {
      position: absolute; top: 16px; right: 16px;
      display: flex; align-items: center; gap: 12px;
      background: rgba(20, 24, 30, 0.85);
      border: 1px solid #3a414b; border-radius: 999px;
      padding: 8px 16px; font-size: 14px; color: #b8c0ca;
    }
    .status-dot { width: 10px; height: 10px; border-radius: 50%; background: #e74c3c; flex-shrink: 0; }
    .status-dot.on { background: #2ecc71; box-shadow: 0 0 8px rgba(46,204,113,0.7); }
    .status-sep { color: #565d66; }
    .status-ssid { color: #ffffff; font-weight: 600; }

        /* Версия прошивки (слева сверху) */
    .version-badge {
      position: absolute; top: 16px; left: 16px;
      background: rgba(20, 24, 30, 0.85);
      border: 1px solid #3a414b; border-radius: 999px;
      padding: 8px 16px; font-size: 14px; color: #b8c0ca;
      letter-spacing: 1px;
    }

    .icon-btn {
      background: none; border: none; color: #b8c0ca; font-size: 20px;
      cursor: pointer; padding: 4px; line-height: 1; transition: color .2s ease;
    }
    .icon-btn:hover { color: #ffffff; }

    /* ============ БАТАРЕЯ + МЕНЮ ============ */
    .battery-panel {
      position: absolute; top: 64px; right: 16px;
      display: flex; align-items: center; gap: 8px;
      background: rgba(20, 24, 30, 0.85);
      border: 1px solid #3a414b; border-radius: 12px;
      padding: 8px 10px;
    }
    .battery-main {
      display: flex; align-items: center; gap: 10px;
      cursor: pointer; padding: 2px 4px;
    }
    .battery-main svg { display: block; flex-shrink: 0; }
    .battery-text { font-size: 14px; color: #b8c0ca; white-space: nowrap; }
    .battery-panel.updating .battery-text { color: #ff9800; }
    .battery-panel.critical .battery-main svg { animation: batblink 0.5s ease-in-out infinite; }
    @keyframes batblink { 50% { opacity: 0.2; } }

    .menu-btn {
      background: none; border: none; color: #b8c0ca;
      font-size: 20px; cursor: pointer; padding: 2px 6px; line-height: 1;
      border-left: 1px solid #3a414b; transition: color .2s ease;
    }
    .menu-btn:hover { color: #ffffff; }

    h1 {
      font-size: clamp(64px, 14vw, 140px); font-weight: 900; letter-spacing: 14px;
      text-align: center; color: #ffffff; text-shadow: 0 0 35px rgba(255,255,255,0.30);
    }

    .lamps { display: flex; gap: 80px; }
    .lamp { display: flex; flex-direction: column; align-items: center; gap: 16px; }
    .bulb {
      width: 78px; height: 78px; border-radius: 50%;
      background: #2c3138; border: 3px solid #4b515b;
      transition: background .25s ease, border-color .25s ease, box-shadow .25s ease;
    }
    .lamp-label { font-size: 15px; color: #9aa1aa; letter-spacing: 1px; }

    .lamp.activity .bulb.on {
      background: #ff9800; border-color: #ffb74d;
      box-shadow: 0 0 28px 6px rgba(255,152,0,0.65);
    }
    .lamp.activity .bulb.blinking { animation: blink 0.5s ease-in-out infinite; }
    @keyframes blink { 50% { opacity: 0.12; } }

    .lamp.power .bulb.on {
      background: #2ecc71; border-color: #5fe3a0;
      box-shadow: 0 0 28px 6px rgba(46,204,113,0.65);
    }

    .switch { position: relative; display: inline-block; width: 240px; height: 104px; }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider {
      position: absolute; cursor: pointer;
      top: 0; left: 0; right: 0; bottom: 0;
      background: #2c3138; border: 2px solid #4b515b; border-radius: 60px;
      transition: background .3s ease, border-color .3s ease, box-shadow .3s ease;
      display: flex; align-items: center; justify-content: center;
    }
    .slider-text {
      font-weight: 800; font-size: 30px; letter-spacing: 3px;
      color: #8a919b; user-select: none; transition: color .3s ease;
    }
    .switch input:checked + .slider {
      background: #123b26; border-color: #2ecc71;
      box-shadow: 0 0 34px rgba(46,204,113,0.5);
    }
    .switch input:checked + .slider .slider-text { color: #7df0b0; }

    /* Модальные окна */
    .modal-overlay {
      position: fixed; inset: 0; background: rgba(0,0,0,0.65);
      display: none; align-items: center; justify-content: center;
      padding: 20px; z-index: 10;
    }
    .modal-overlay.open { display: flex; }

    .modal {
      background: #1b1f25; border: 1px solid #3a414b; border-radius: 16px;
      width: 100%; max-width: 440px; max-height: 90vh; overflow-y: auto;
    }
    .modal-header {
      display: flex; align-items: center; justify-content: space-between;
      padding: 16px 20px; border-bottom: 1px solid #2c3138;
    }
    .modal-header h2 { font-size: 18px; font-weight: 700; }
    .modal-body { padding: 20px; display: flex; flex-direction: column; gap: 20px; }

    .section h3 {
      font-size: 13px; font-weight: 700; letter-spacing: 1px;
      color: #9aa1aa; text-transform: uppercase; margin-bottom: 10px;
    }

    input[type="text"], input[type="password"], input[type="number"] {
      width: 100%; padding: 12px 14px;
      background: #12151a; border: 1px solid #3a414b; border-radius: 10px;
      color: #eaeaea; font-size: 15px; margin-bottom: 10px; outline: none;
      transition: border-color .2s ease;
    }
    input:focus { border-color: #2ecc71; }

    .btn {
      display: block; width: 100%; padding: 12px; border: none; border-radius: 10px;
      font-size: 15px; font-weight: 700; cursor: pointer;
      text-align: center; text-decoration: none;
      transition: opacity .2s ease;
    }
    .btn:hover { opacity: 0.85; }
    .btn.primary { background: #2ecc71; color: #0b1f12; }
    .btn.danger  { background: #3a2020; color: #ff7b7b; border: 1px solid #6e2f2f; }
    .btn.ghost   { background: #262b33; color: #b8c0ca; }

    .net-list { display: flex; flex-direction: column; gap: 8px; }
    .net-item {
      display: flex; align-items: center; justify-content: space-between;
      background: #12151a; border: 1px solid #2c3138; border-radius: 10px;
      padding: 10px 14px;
    }
    .net-item .ssid { color: #eaeaea; word-break: break-all; }
    .net-item button {
      background: none; border: none; color: #ff7b7b;
      font-size: 13px; cursor: pointer; padding: 4px 8px; flex-shrink: 0;
    }
    .net-item button:hover { text-decoration: underline; }
    .net-empty { color: #565d66; font-size: 14px; text-align: center; padding: 8px; }

    /* Модалка калибровки */
    .cal-modal {
      background: #1b1f25; border: 1px solid #3a414b; border-radius: 16px;
      width: 100%; max-width: 360px; padding: 24px;
      display: flex; flex-direction: column; gap: 16px;
    }
    .cal-modal h2 { font-size: 18px; font-weight: 700; }
    .cal-hint { font-size: 13px; color: #9aa1aa; }
    </style>
</head>

<body>
    <!-- ВЕРСИЯ ПРОШИВКИ -->
    <div class="version-badge" id="firmwareVersion">V—</div>

    <!-- СТАТУС-БАР -->
    <div class="status-bar">
        <span class="status-dot" id="wifiDot"></span>
        <span class="status-ssid" id="wifiSsid">—</span>
        <span class="status-sep">·</span>
        <span id="wifiIp">—</span>
        <button class="icon-btn" onclick="openSettings()" title="Настройки">&#9881;</button>
    </div>

    <!-- БАТАРЕЯ + МЕНЮ (три точки) -->
    <div class="battery-panel" id="batteryPanel">
        <div class="battery-main" id="batteryBox" title="Нажмите для обновления">
            <svg width="46" height="24" viewBox="0 0 46 24">
                <rect x="0" y="5" width="40" height="14" rx="4" fill="none" stroke="#8a919b" stroke-width="2" id="batteryBody"/>
                <rect x="42" y="9" width="3" height="6" rx="1.5" fill="#8a919b" id="batteryCap"/>
                <rect x="3" y="8" width="0" height="8" rx="2" fill="#2ecc71" id="batteryFill"/>
                <rect x="4" y="9" width="6" height="2" rx="1" fill="rgba(255,255,255,0.25)" id="batteryGlint"/>
            </svg>
            <span class="battery-text" id="batteryText">—</span>
        </div>
        <button class="menu-btn" id="batteryMenuBtn" title="Настройки батареи">&#8942;</button>
    </div>

    <h1>РЕЛЕ</h1>

    <div class="lamps">
        <div class="lamp activity">
            <div class="bulb" id="activityLamp"></div>
            <span class="lamp-label">Действие</span>
        </div>
        <div class="lamp power">
            <div class="bulb" id="powerLamp"></div>
            <span class="lamp-label">Питание</span>
        </div>
    </div>

    <label class="switch">
        <input type="checkbox" id="powerSwitch">
        <span class="slider"><span class="slider-text">ВЫКЛ</span></span>
    </label>

    <!-- МОДАЛКА НАСТРОЕК -->
    <div class="modal-overlay" id="modalOverlay">
        <div class="modal">
            <div class="modal-header">
                <h2>Настройки Wi-Fi</h2>
                <button class="icon-btn" onclick="closeSettings()">&#10005;</button>
            </div>
            <div class="modal-body">
                <div class="section">
                    <h3>Текущее подключение</h3>
                    <p id="currentConnection" style="font-size:15px; color:#b8c0ca;">—</p>
                </div>
                <div class="section">
                    <h3>Добавить сеть</h3>
                    <input type="text" id="wifiSsidInput" placeholder="Имя сети (SSID)">
                    <input type="password" id="wifiPassInput" placeholder="Пароль">
                    <button class="btn primary" onclick="addNetwork()">Сохранить сеть</button>
                </div>
                <div class="section">
                    <h3>Сохранённые сети</h3>
                    <div class="net-list" id="networkList"></div>
                </div>
                <div class="section">
                    <h3>Режим точки доступа</h3>
                    <p id="apStatus" style="font-size:14px; color:#b8c0ca; margin-bottom:10px;">—</p>
                    <button class="btn primary" onclick="startAp()">Включить точку доступа</button>
                    <button class="btn danger" onclick="stopAp()" style="margin-top:8px;">Выключить точку доступа</button>
                </div>
                <div class="section">
                    <h3>Система</h3>
                    <a class="btn ghost" href="/update">Обновление прошивки (OTA)</a>
                    <button class="btn danger" onclick="reboot()" style="margin-top:8px;">Перезагрузить устройство</button>
                </div>
            </div>
        </div>
    </div>

    <!-- МОДАЛКА КАЛИБРОВКИ БАТАРЕИ -->
    <div class="modal-overlay" id="calModalOverlay">
        <div class="cal-modal">
            <h2>Калибровка батареи</h2>
            <p class="cal-hint" id="calCurrent">Текущее значение: —</p>
            <input type="number" id="calInput" step="0.0001" min="0.1" max="3.0" placeholder="Например: 0.9205">
            <button class="btn primary" onclick="saveCalibration()">Сохранить в память</button>
            <button class="btn ghost" onclick="closeCalibration()">Отмена</button>
        </div>
    </div>

    <script>
    const activityLamp = document.getElementById('activityLamp');
    const powerLamp    = document.getElementById('powerLamp');
    const powerSwitch  = document.getElementById('powerSwitch');
    const sliderText   = document.querySelector('.slider-text');
    const wifiDot      = document.getElementById('wifiDot');
    const wifiSsid     = document.getElementById('wifiSsid');
    const wifiIp       = document.getElementById('wifiIp');
    const modalOverlay = document.getElementById('modalOverlay');

        const versionBadge = document.getElementById('firmwareVersion');

    async function loadVersion() {
      try {
        const res = await fetch('/api/version');
        const data = await res.json();
        versionBadge.textContent = 'V' + data.version;
      } catch (e) {
        versionBadge.textContent = 'V?';
      }
    }


    const batteryPanel  = document.getElementById('batteryPanel');
    const batteryBox    = document.getElementById('batteryBox');
    const batteryBody   = document.getElementById('batteryBody');
    const batteryFill   = document.getElementById('batteryFill');
    const batteryText   = document.getElementById('batteryText');
    const batteryMenuBtn = document.getElementById('batteryMenuBtn');

    const calModalOverlay = document.getElementById('calModalOverlay');
    const calInput  = document.getElementById('calInput');
    const calCurrent = document.getElementById('calCurrent');

    const BATTERY_STATES = [
      { color: '#2ecc71', label: 'полный' },
      { color: '#ff9800', label: 'половина' },
      { color: '#e74c3c', label: 'низкий' },
      { color: '#e74c3c', label: 'критический' }
    ];
    const BATTERY_FILL_W = [34, 17, 7, 0];

    function setActivity(on) {
      activityLamp.classList.toggle('on', on);
      activityLamp.classList.toggle('blinking', on);
    }

    function updatePower(on) {
      powerSwitch.checked = on;
      sliderText.textContent = on ? 'ВКЛ' : 'ВЫКЛ';
      powerLamp.classList.toggle('on', on);
    }

    function updateBattery(data) {
      const level = data.level;
      const st = BATTERY_STATES[level];

      batteryFill.setAttribute('width', BATTERY_FILL_W[level]);
      batteryFill.setAttribute('fill', st.color);
      batteryBody.setAttribute('stroke', st.color);
      batteryPanel.classList.toggle('critical', level === 3);

      const v = (data.voltage_mv / 1000).toFixed(2);
      batteryText.textContent = v + ' В · ' + data.percent + '%';
      batteryBox.title = 'Заряд: ' + st.label + ' (' + v + ' В). Нажмите для обновления.';
    }

    async function loadBattery() {
      try {
        const res = await fetch('/api/battery');
        const data = await res.json();
        updateBattery(data);
      } catch (e) { batteryText.textContent = '—'; }
    }

    async function refreshBattery() {
      batteryText.textContent = 'обновление...';
      batteryPanel.classList.add('updating');
      try {
        const res = await fetch('/api/battery');
        const data = await res.json();
        updateBattery(data);
      } catch (e) {
        batteryText.textContent = 'ошибка';
      } finally {
        batteryPanel.classList.remove('updating');
      }
    }

    async function openCalibration() {
      try {
        const res = await fetch('/api/battery/config');
        const data = await res.json();
        calInput.value = data.calibration;
        calCurrent.textContent = 'Текущее значение: ' + data.calibration.toFixed(4) +
          ' (по умолчанию: ' + data.default.toFixed(4) + ')';
      } catch (e) {
        calCurrent.textContent = 'Не удалось получить значение';
      }
      calModalOverlay.classList.add('open');
    }

    function closeCalibration() {
      calModalOverlay.classList.remove('open');
    }

    async function saveCalibration() {
      const val = parseFloat(calInput.value);
      if (isNaN(val) || val < 0.1 || val > 3.0) {
        alert('Введите корректное значение от 0.1 до 3.0');
        return;
      }
      try {
        const res = await fetch('/api/battery/config', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({calibration: val})
        });
        const data = await res.json();
        if (data.ok) {
          alert('Калибровка сохранена в память');
          closeCalibration();
          refreshBattery();
        } else {
          alert('Ошибка: ' + (data.error || 'не удалось сохранить'));
        }
      } catch (e) { alert('Нет связи с платой'); }
    }

    async function loadState() {
      try {
        const res = await fetch('/api/state');
        const data = await res.json();
        updatePower(data.power);
      } catch (e) { console.error(e); }
    }

    async function setPower(on) {
      setActivity(true);
      try {
        const res = await fetch('/api/power?on=' + (on ? 1 : 0), { method: 'POST' });
        const data = await res.json();
        if (data.ok) updatePower(data.power);
      } catch (e) { alert('Нет связи с платой'); }
      finally { setActivity(false); }
    }

    async function loadWifiStatus() {
      try {
        const res = await fetch('/api/wifi/status');
        const data = await res.json();
        wifiDot.classList.toggle('on', data.connected);
        wifiSsid.textContent = data.connected ? data.ssid : 'не подключено';
        wifiIp.textContent   = data.connected ? data.ip : '—';
        document.getElementById('currentConnection').textContent =
          data.connected
            ? 'Сеть: ' + data.ssid + ' · IP: ' + data.ip + ' · Сигнал: ' + data.rssi + ' дБм'
            : 'Не подключено';
      } catch (e) { console.error(e); }
    }

    async function loadNetworks() {
      try {
        const res = await fetch('/api/wifi/networks');
        const data = await res.json();
        const list = document.getElementById('networkList');
        list.innerHTML = '';
        if (data.networks.length === 0) {
          list.innerHTML = '<div class="net-empty">Сохранённых сетей нет</div>';
          return;
        }
        data.networks.forEach(function(n) {
          const item = document.createElement('div');
          item.className = 'net-item';
          const ssidSpan = document.createElement('span');
          ssidSpan.className = 'ssid';
          ssidSpan.textContent = n.ssid;
          const btn = document.createElement('button');
          btn.textContent = 'Удалить';
          btn.onclick = function() { removeNetwork(n.index); };
          item.appendChild(ssidSpan);
          item.appendChild(btn);
          list.appendChild(item);
        });
      } catch (e) { console.error(e); }
    }

    async function addNetwork() {
      const ssid = document.getElementById('wifiSsidInput').value.trim();
      const pass = document.getElementById('wifiPassInput').value;
      if (!ssid) { alert('Введите имя сети (SSID)'); return; }
      setActivity(true);
      try {
        const res = await fetch('/api/wifi/add', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({ssid: ssid, pass: pass})
        });
        const data = await res.json();
        if (data.ok) {
          document.getElementById('wifiSsidInput').value = '';
          document.getElementById('wifiPassInput').value = '';
          loadNetworks();
          if (confirm('Сеть сохранена. Перезагрузить устройство, чтобы подключиться к новой сети?')) {
            reboot();
          }
        } else {
          alert('Ошибка: ' + (data.error || 'не удалось сохранить'));
        }
      } catch (e) { alert('Нет связи с платой'); }
      finally { setActivity(false); }
    }

    async function removeNetwork(index) {
      if (!confirm('Удалить эту сеть?')) return;
      setActivity(true);
      try {
        const res = await fetch('/api/wifi/remove', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({index: index})
        });
        const data = await res.json();
        if (data.ok) loadNetworks();
        else alert('Ошибка удаления');
      } catch (e) { alert('Нет связи с платой'); }
      finally { setActivity(false); }
    }

    async function loadApStatus() {
      try {
        const res = await fetch('/api/ap/status');
        const data = await res.json();
        document.getElementById('apStatus').textContent =
          data.active ? 'Точка доступа активна: ' + data.ssid : 'Не активна';
      } catch (e) { console.error(e); }
    }

    async function startAp() {
      const msg = 'Устройство переключится в режим точки доступа.\n\n' +
                  'Подключитесь к сети "RELAY-Setup" и откройте http://192.168.4.1';
      if (!confirm(msg)) return;
      setActivity(true);
      try { await fetch('/api/ap/start', { method: 'POST' }); }
      catch (e) {}
      finally { setActivity(false); alert('Точка доступа запущена.'); }
    }

    async function stopAp() {
      setActivity(true);
      try { await fetch('/api/ap/stop', { method: 'POST' }); }
      catch (e) {}
      finally { setActivity(false); }
    }

    async function reboot() {
      try { await fetch('/api/reboot', { method: 'POST' }); }
      catch (e) {}
      alert('Устройство перезагружается. Подождите несколько секунд.');
    }

    function openSettings() {
      modalOverlay.classList.add('open');
      loadWifiStatus();
      loadNetworks();
      loadApStatus();
    }
    function closeSettings() { modalOverlay.classList.remove('open'); }
    modalOverlay.addEventListener('click', function(e) {
      if (e.target === modalOverlay) closeSettings();
    });
    calModalOverlay.addEventListener('click', function(e) {
      if (e.target === calModalOverlay) closeCalibration();
    });

    powerSwitch.addEventListener('change', function() {
      setPower(this.checked);
    });

    batteryBox.addEventListener('click', refreshBattery);
    batteryMenuBtn.addEventListener('click', function(e) {
      e.stopPropagation();
      openCalibration();
    });

    loadState();
    loadWifiStatus();
    loadBattery();
    loadVersion();

    </script>
</body>
</html>
)rawliteral";

} // namespace relay::web::index_page
