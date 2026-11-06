#pragma once

// =============================================================================
//  update_page.hpp — HTML страница веб-обновления прошивки
// -----------------------------------------------------------------------------
//  Позволяет загрузить готовый .bin файл прошивки из браузера (в т.ч. с телефона).
// =============================================================================

namespace relay::web::update_page
{

    inline constexpr char kHtml[] = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>РЕЛЕ — обновление прошивки</title>
<style>
* { box-sizing: border-box; margin: 0; padding: 0; }
body {
  font-family: 'Segoe UI', Roboto, system-ui, sans-serif;
  min-height: 100vh;
  display: flex; align-items: center; justify-content: center;
  padding: 24px; color: #eaeaea;
  background: radial-gradient(circle at 50% 0%, #242a33 0%, #15181e 55%, #0d0f13 100%);
}
.card {
  width: 100%; max-width: 440px;
  background: #1b1f25; border: 1px solid #3a414b; border-radius: 16px;
  padding: 28px; display: flex; flex-direction: column; gap: 20px;
}
h1 { font-size: 22px; font-weight: 700; text-align: center; }
p.hint { font-size: 14px; color: #9aa1aa; text-align: center; }
input[type="password"], input[type="file"] {
  width: 100%; padding: 12px 14px;
  background: #12151a; border: 1px solid #3a414b; border-radius: 10px;
  color: #eaeaea; font-size: 15px; outline: none;
}
input:focus { border-color: #2ecc71; }
.btn {
  width: 100%; padding: 14px; border: none; border-radius: 10px;
  font-size: 16px; font-weight: 700; cursor: pointer;
  background: #2ecc71; color: #0b1f12; transition: opacity .2s ease;
}
.btn:hover { opacity: 0.85; }
.btn:disabled { opacity: 0.4; cursor: not-allowed; }
.status { font-size: 14px; text-align: center; color: #b8c0ca; min-height: 20px; }
.status.ok { color: #2ecc71; }
.status.err { color: #ff7b7b; }
</style>
</head>
<body>
<div class="card">
  <h1>Обновление прошивки РЕЛЕ</h1>
  <p class="hint">Выберите файл прошивки (.bin) и нажмите «Обновить».</p>

  <input type="password" id="password" placeholder="Пароль OTA">

  <input type="file" id="file" accept=".bin">

  <button class="btn" id="btn" onclick="upload()">Обновить</button>
  <div class="status" id="status"></div>
</div>

<script>
async function upload() {
  const fileInput = document.getElementById('file');
  const passInput = document.getElementById('password');
  const btn = document.getElementById('btn');
  const status = document.getElementById('status');

  if (!fileInput.files.length) {
    status.textContent = 'Выберите файл прошивки';
    status.className = 'status err';
    return;
  }

  const form = new FormData();
  form.append('password', passInput.value);
  form.append('update', fileInput.files[0]);

  btn.disabled = true;
  status.textContent = 'Идёт загрузка прошивки...';
  status.className = 'status';

  try {
    const res = await fetch('/update', { method: 'POST', body: form });
    const text = await res.text();
    if (text.trim() === 'OK') {
      status.textContent = 'Успешно! Устройство перезагружается...';
      status.className = 'status ok';
    } else {
      status.textContent = 'Ошибка: ' + text;
      status.className = 'status err';
      btn.disabled = false;
    }
  } catch (e) {
    status.textContent = 'Сеть оборвалась — устройство, возможно, перезагружается';
    status.className = 'status ok';
  }
}
</script>
</body>
</html>
)rawliteral";

} // namespace relay::web::update_page
