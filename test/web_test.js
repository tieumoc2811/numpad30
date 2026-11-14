// Kiem thu Studio (Playwright + WebHID gia noi toi firmware mo phong build/fw_sim --serve): Tap Dance / Combo / Latch / Morse
const { chromium } = require('playwright');
const { spawn } = require('child_process');
const path = require('path');
const root = path.resolve(__dirname, '..');
let fails = 0; const ok = (c, m) => { if (!c) { console.log('  LOI:', m); fails++; } else console.log('  ok:', m); };
(async () => {
  const sim = spawn(path.join(__dirname, 'build/fw_sim'), ['--serve']);
  const q = []; let buf = ''; const waiters = [];
  sim.stdout.on('data', d => { buf += d; let i; while ((i = buf.indexOf('\n')) >= 0) { const l = buf.slice(0, i); buf = buf.slice(i + 1); const w = waiters.shift(); if (w) w(l); else q.push(l); } });
  const line = () => new Promise(r => q.length ? r(q.shift()) : waiters.push(r));
  const send = async s => { sim.stdin.write(s + '\n'); return line(); };
  const browser = await chromium.launch();
  const page = await browser.newPage({ viewport: { width: 1280, height: 900 }, deviceScaleFactor: 1 });
  const errs = []; page.on('pageerror', e => errs.push(e.message)); page.on('console', m => { if (m.type() === 'error') errs.push(m.text()); });
  page.on('dialog', d => d.accept());
  await page.exposeFunction('__hid', async arr => send(arr.map(b => b.toString(16).padStart(2, '0')).join('')).then(h => h.match(/../g).map(x => parseInt(x, 16))));
  await page.addInitScript(() => {
    const dev = { opened: false, productName: 'Numpad30', _l: [], open: async () => { dev.opened = true; }, close: async () => { dev.opened = false; },
      addEventListener: (t, f) => { dev._l.push(f); }, removeEventListener: (t, f) => { dev._l = dev._l.filter(x => x !== f); },
      sendReport: async (id, data) => { const rsp = await window.__hid([...data]); setTimeout(() => dev._l.slice().forEach(f => f({ data: new DataView(Uint8Array.from(rsp).buffer) })), 0); } };
    Object.defineProperty(navigator, 'hid', { value: { requestDevice: async () => [dev], getDevices: async () => [], addEventListener() {} }, configurable: true });
  });
  await page.goto('file://' + path.join(root, 'numpad30-studio.html'));
  await page.evaluate(() => { try { localStorage.clear(); } catch (e) {} });
  await page.reload();
  const ev = f => page.evaluate(f);

  console.log('1) giao dien Tap Dance / Combo');
  await page.click('#nav button[data-view="smart"]');
  ok(await page.$$eval('#tdTable .slot-row', e => e.length) === 8 && await page.$$eval('#cbTable .slot-row', e => e.length) === 4, '8 hang tap dance + 4 hang combo');
  await page.click('#tdTable [data-slot="td,0,0"]');
  await page.click('#pkCats [data-cat="letters"]'); await page.click('#pkGrid [data-code="4"]');   // TD1 chạm = A
  await page.click('#tdTable [data-slot="td,0,1"]'); await page.click('#pkCats [data-cat="smart"]'); await page.click('#pkGrid [data-code="' + 0x7061 + '"]');   // giữ = Morse gạch
  await page.click('#tdTable [data-slot="td,0,2"]'); await page.click('#pkCats [data-cat="letters"]'); await page.click('#pkGrid [data-code="6"]');   // đôi = C
  await page.click('#cbTable [data-slot="cb,1,0"]'); await page.click('#pkCats [data-cat="letters"]'); await page.click('#pkGrid [data-code="13"]');   // J
  await page.click('#cbTable [data-slot="cb,1,1"]'); await page.click('#pkCats [data-cat="letters"]'); await page.click('#pkGrid [data-code="14"]');   // K
  await page.click('#cbTable [data-slot="cb,1,2"]'); await page.click('#pkCats [data-cat="letters"]'); await page.click('#pkGrid [data-code="29"]');   // Z
  ok(JSON.stringify(await ev(() => state.td[0])) === '[4,28769,6]', 'TD1 = {A, Morse gach, C}: ' + JSON.stringify(await ev(() => state.td[0])));
  ok(JSON.stringify(await ev(() => state.cb[1])) === '[13,14,29]', 'Combo2 = J+K -> Z');
  ok((await page.textContent('#tdTable [data-slot="td,0,1"] span')) === '—', 'o giu hien nhan Morse gach');
  await page.waitForTimeout(2500); await page.screenshot({ path: path.join(root, 'test/build/web_smart.png') });
  // gán TD1 / LATCH / Morse vào phím bằng picker thường
  await ev(() => { ui.layer = 0; setKey(10, TD(0)); setKey(11, KC_LATCH); setKey(12, MORSE_DOT); });
  ok(await ev(() => shortOf(state.keymap[0][10]) + ',' + shortOf(state.keymap[0][11]) + ',' + shortOf(state.keymap[0][12])) === 'TD1,Latch,·', 'nhan phim TD1 / Latch / chấm');
  const code = await ev(() => genKeymap());
  ok(code.includes('def_ext') && code.includes('TD(0)') && code.includes('KC_LATCH') && code.includes('KC_MORSE_DOT') && code.includes('{ KC_A, KC_MORSE_DASH, KC_C }'), 'keymap.h xuat co TD/KC_LATCH/Morse + def_ext');

  console.log('2) ket noi, day xuong, luu flash, mat dien, doc lai');
  await page.click('#hidBtn');
  await page.waitForFunction(() => document.getElementById('hidTitle').textContent.includes('Đã kết nối'), null, { timeout: 15000 });
  ok(true, 'ket noi: ' + await page.textContent('#hidTitle'));
  await ev(() => hidPush(true));
  await ev(() => hidSave());
  ok(!(await page.textContent('#hidTitle')).includes('chưa lưu'), 'da luu flash: ' + await page.textContent('#hidTitle'));
  ok(await send('REBOOT') === 'OK', 'thiet bi mat dien');
  await ev(() => { state.td = state.td.map(() => [0, 0, 0]); state.cb = state.cb.map(() => [0, 0, 0]); state.keymap[0][10] = 0; });
  await ev(() => hidPull());
  ok(JSON.stringify(await ev(() => state.td[0])) === '[4,28769,6]', 'TD1 con nguyen sau khi mat dien');
  ok(JSON.stringify(await ev(() => state.cb[1])) === '[13,14,29]', 'Combo2 con nguyen');
  ok(await ev(() => state.keymap[0][10]) === 0x4200 && await ev(() => state.keymap[0][11]) === 0x7001 && await ev(() => state.keymap[0][12]) === 0x7060, 'phim TD1/LATCH/Morse con nguyen');

  console.log('3) ho so JSON cu (khong co td/cb) van nap duoc');
  const okOld = await ev(() => { const s = sanitize({ names: ['A', 'B', 'C'], keymap: [] }); return s.td.length === 8 && s.cb.length === 4 && s.td[0][0] === 0; });
  ok(okOld, 'sanitize bo sung td/cb mac dinh');
  ok(errs.length === 0, 'khong co loi JS: ' + errs.join(' | '));
  await browser.close(); sim.kill();
  console.log(fails ? 'web: CO LOI' : 'web: tat ca dat'); process.exit(fails ? 1 : 0);
})();
