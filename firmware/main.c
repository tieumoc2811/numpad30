// Numpad30 RP2350-Zero (v2, tuong thich app Studio cua macropad15): ma tran 5x7 co diode (hang GP0-4, cot GP5-11),
// HID ban phim + media + Raw HID cau hinh (usage page 0xFF60), OLED SSD1306 (I2C0 SDA=GP12 SCL=GP13), den RGB onboard (GP16).
// Cau hinh (phim, layer, tap dance, combo, chuoi, macro, OLED) luu flash, sua bang app qua USB. Logic nam trong core.h / proto.h / ui.h.
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "pico/flash.h"
#include "hardware/flash.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "ws2812.pio.h"
#include "tusb.h"
#include "ui.h"
#include "airmouse.h"

static const uint8_t row_pins[NROW] = ROW_PINS, col_pins[NCOL] = COL_PINS;
static core_t core;
static ui_t ui;
static uint8_t host_leds;                       // bit0 NumLock, bit1 CapsLock (tu may tinh)

static uint32_t ms(void) { return time_us_32() / 1000; }

// ---------------- quet ma tran ----------------
static void matrix_init(void) {
  for (int r = 0; r < NROW; r++) { gpio_init(row_pins[r]); gpio_set_dir(row_pins[r], GPIO_OUT); gpio_put(row_pins[r], 0); }
  for (int c = 0; c < NCOL; c++) { gpio_init(col_pins[c]); gpio_set_dir(col_pins[c], GPIO_IN); gpio_pull_down(col_pins[c]); }
}
static bool scan(uint8_t *raw) {   // tra ve true neu co phim dang bam
  bool any = false;
  for (int r = 0; r < NROW; r++) {
    gpio_put(row_pins[r], 1); busy_wait_us_32(5);
    for (int c = 0; c < NCOL; c++) { uint8_t v = gpio_get(col_pins[c]); raw[r * NCOL + c] = v; any |= v; }
    gpio_put(row_pins[r], 0);
  }
  return any;
}

// ---------------- coi bao (Morse) ----------------
#if BUZ_PIN >= 0
static bool buz_state; static uint32_t buz_hz_set, buz_top;
static void buz_idle(void) { gpio_set_function(BUZ_PIN, GPIO_FUNC_SIO); gpio_set_dir(BUZ_PIN, GPIO_OUT); gpio_put(BUZ_PIN, BUZ_INVERT); }
static void buz_init(void) { gpio_init(BUZ_PIN); buz_idle(); }
static void buz_task(void) {   // goi moi ms: bat / tat coi, doi tan so + do rong xung theo core_buz_hz / core_buz_amp (tieng bip Morse hoac nốt nhac)
  uint32_t hz = core_buz_hz(&core), amp = hz ? core_buz_amp(&core) : 0;
  bool on = hz != 0 && amp != 0;
#if BUZ_ACTIVE
  if (on == buz_state) return;
  buz_state = on; gpio_put(BUZ_PIN, on ^ BUZ_INVERT);
#else
  uint sl = pwm_gpio_to_slice_num(BUZ_PIN);
  if (!on) { if (buz_state) { buz_state = false; pwm_set_enabled(sl, false); buz_idle(); } return; }
  if (!buz_state || hz != buz_hz_set) {   // doi tan so: chia xung de wrap < 65536 (ke ca nốt thap, ~260 Hz)
    uint32_t clk = clock_get_hz(clk_sys);
    float div = (float)clk / ((float)hz * 60000.0f); if (div < 1.0f) div = 1.0f;
    uint32_t top = (uint32_t)((float)clk / (div * (float)hz) + 0.5f); if (top < 2) top = 2; if (top > 65536) top = 65536;
    pwm_set_clkdiv(sl, div); pwm_set_wrap(sl, (uint16_t)(top - 1)); buz_top = top; buz_hz_set = hz;
  }
  uint32_t lv = buz_top / 2 * amp / 256; if (lv < 1) lv = 1;   // am luong + am sac = do rong xung (toi da 50%)
  pwm_set_gpio_level(BUZ_PIN, (uint16_t)lv);
  if (!buz_state) { gpio_set_function(BUZ_PIN, GPIO_FUNC_PWM); pwm_set_enabled(sl, true); buz_state = true; }
#endif
}
#else
static void buz_init(void) {}
static void buz_task(void) {}
#endif

// ---------------- LED RGB onboard (WS2812) ----------------
static void led_init(void) {
  uint off = pio_add_program(pio0, &ws2812_program);
  pio_sm_config c = ws2812_program_get_default_config(off);
  sm_config_set_sideset_pins(&c, LED_PIN); sm_config_set_out_shift(&c, false, true, 24); sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
  sm_config_set_clkdiv(&c, clock_get_hz(clk_sys) / (800000.0f * (ws2812_T1 + ws2812_T2 + ws2812_T3)));
  pio_gpio_init(pio0, LED_PIN); pio_sm_set_consecutive_pindirs(pio0, 0, LED_PIN, 1, true);
  pio_sm_init(pio0, 0, off, &c); pio_sm_set_enabled(pio0, 0, true);
}
static void led_put(uint8_t r, uint8_t g, uint8_t b, uint8_t br) {
  r = r * br / 255; g = g * br / 255; b = b * br / 255;
  pio_sm_put_blocking(pio0, 0, ((uint32_t)g << 16 | (uint32_t)r << 8 | b) << 8);
}
static void hsv(int h, int s, int v, uint8_t *o) {   // giong hsv() trong app (h 0-255)
  h = ((h % 256) + 256) % 256;
  int r = h / 43, f = (h - r * 43) * 6; if (f > 255) f = 255; int g = 255 - f, c[3];
  switch (r) { case 0: c[0] = 255; c[1] = f; c[2] = 0; break; case 1: c[0] = g; c[1] = 255; c[2] = 0; break; case 2: c[0] = 0; c[1] = 255; c[2] = f; break;
    case 3: c[0] = 0; c[1] = g; c[2] = 255; break; case 4: c[0] = f; c[1] = 0; c[2] = 255; break; default: c[0] = 255; c[1] = 0; c[2] = g; }
  for (int i = 0; i < 3; i++) o[i] = (uint8_t)((c[i] + (255 - c[i]) * (255 - s) / 255) * v / 255);
}
static void layer_color(int l, uint8_t *o) {   // giong layerColor() trong app
  static const uint8_t lc[4][3] = {{0, 80, 255}, {0, 255, 60}, {255, 90, 0}, {200, 0, 255}};
  if (l < 4) { memcpy(o, lc[l], 3); return; }
  int h = (l * 137) % 360, f = (h % 60) * 255 / 60, s = h / 60;
  int v[6][3] = {{255, f, 0}, {255 - f, 255, 0}, {0, 255, f}, {0, 255 - f, 255}, {f, 0, 255}, {255, 0, 255 - f}};
  for (int i = 0; i < 3; i++) o[i] = (uint8_t)v[s][i];
}
// o led: mode 0 theo layer, 1 cau vong, 2 mot mau, 3 tho, 4 tat (W_LDB+6); do sang W_LDB+7; hue/sat W_LDB+3/+4. CapsLock -> do.
static void led_update(uint32_t now) {
  static uint32_t last_val = 0xFFFFFFFFu, tick;
  if (now - tick < 20) return;
  tick = now;
  uint8_t c[3] = {0, 0, 0}; int br = core.cfg.w[W_LDB + 7] & 255;
  int mode = core.cfg.w[W_LDB + 6]; if (mode > 4) mode = 0;
  int hue = (core.cfg.w[W_LDB + 3] % 360) * 256 / 360, sat = core.cfg.w[W_LDB + 4] & 255;
  int spd = core.cfg.w[W_LDB + 1] & 255; uint32_t ph = now * (1 + spd / 16) / 8;
  if (ui.blank) br = 0;
  else if (host_leds & 2) { c[0] = 255; }
  else switch (mode) {
    case 0: layer_color(core_top_layer(&core), c); break;
    case 1: hsv((int)(ph & 255), 255, 255, c); break;
    case 2: hsv(hue, sat, 255, c); break;
    case 3: { int t = ph & 255; t = t < 128 ? t * 2 : (255 - t) * 2; hsv(hue, sat, t, c); } break;
    default: br = 0;
  }
  uint32_t val = ((uint32_t)c[0] << 24) ^ ((uint32_t)c[1] << 16) ^ ((uint32_t)c[2] << 8) ^ (uint32_t)br;
  if (val == last_val) return;
  last_val = val; led_put(c[0], c[1], c[2], br);
}

// ---------------- OLED SSD1306 (noi dung do ui.h ve) ----------------
static bool oled_ok, opower = true;
static int opg = OLED_PAGES;
static bool i2c_w(const uint8_t *b, size_t n) { return i2c_write_timeout_us(i2c0, OLED_ADDR, b, n, false, 4000) == (int)n; }
static void oled_pwr(bool on) { uint8_t c[] = {0x00, on ? 0xAF : 0xAE}; i2c_w(c, 2); opower = on; }
static void oled_try_init(void) {
  const uint8_t ini[] = {0x00, 0xAE, 0xD5, 0x80, 0xA8, OLED_H - 1, 0xD3, 0x00, 0x40, 0x8D, 0x14, 0x20, 0x02, 0xA1, 0xC8,
                         0xDA, OLED_H == 32 ? 0x02 : 0x12, 0x81, 0x7F, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF};
  oled_ok = i2c_w(ini, sizeof ini); opower = true; ui.dirty = true;
}
static void oled_init(void) {
  i2c_init(i2c0, 400000);
  gpio_set_function(OLED_SDA, GPIO_FUNC_I2C); gpio_set_function(OLED_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(OLED_SDA); gpio_pull_up(OLED_SCL);
  oled_try_init();
}
// ---------------- cam bien gyro MPU-6515 (chuot bay), chung I2C0 voi OLED ----------------
static air_t air; static int air_px, air_py; static uint8_t m_sent;
static int16_t air_g[3]; static uint8_t hid_sent, hid_rej;   // gyro tho moi nhat + dem bao cao chuot (cho lenh go loi 11)
static bool imu_wr(uint8_t reg, uint8_t v) { uint8_t b[2] = {reg, v}; return i2c_write_timeout_us(i2c0, IMU_ADDR, b, 2, false, 4000) == 2; }
static bool imu_rd(uint8_t reg, uint8_t *o, size_t n) {
  return i2c_write_timeout_us(i2c0, IMU_ADDR, &reg, 1, true, 4000) == 1 && i2c_read_timeout_us(i2c0, IMU_ADDR, o, n, false, 4000) == (int)n;
}
static void imu_try_init(void) {
  uint8_t id = 0;
  core.imu_ok = 0;
  if (!imu_wr(0x6B, 0x01)) return;                                  // danh thuc, nguon xung = PLL gyro X
  sleep_ms(10);
  if (!imu_rd(0x75, &id, 1) || id == 0x00 || id == 0xFF) return;    // WHO_AM_I (MPU-6515 ~0x74, MPU-6500 0x70)
  imu_wr(0x1A, 0x03); imu_wr(0x19, 0x00); imu_wr(0x1B, 0x08);       // loc so 41 Hz, 1 kHz, gyro +-500 dps
  imu_wr(0x6C, 0x38);                                               // PWR_MGMT_2: tat gia toc ke (bit5..3). KHONG dung 0x07: do la TAT GYRO
  core.imu_ok = 1; core.imu_id = id; air_recal(&air);
}
static void imu_task(uint32_t now) {      // goi moi ms; doc moi IMU_POLL_MS
  static uint32_t last, retry;
  if (!core.imu_ok) { if (now - retry >= 2000) { retry = now; imu_try_init(); } return; }
  if (core.cal_req) { core.cal_req = false; air_recal(&air); }
  if (now - last < IMU_POLL_MS) return;
  uint32_t dt = now - last; last = now;
  uint8_t b[6];
  if (!imu_rd(0x43, b, 6)) { core.imu_ok = 0; return; }
  int16_t gx = (int16_t)(b[0] << 8 | b[1]), gy = (int16_t)(b[2] << 8 | b[3]), gz = (int16_t)(b[4] << 8 | b[5]);
  air_g[0] = gx; air_g[1] = gy; air_g[2] = gz;
  int dx, dy; air_update(&air, &core.cfg.w[W_AMB], gx, gy, gz, dt, core_air_on(&core), &dx, &dy);
  air_px += dx; air_py += dy;
}
static void oled_task(uint32_t now) {     // goi moi ms: moi lan gui toi da 1 page de khong chan quet phim / USB
  static uint32_t retry;
  if (!oled_ok) { if (now - retry >= 2000) { retry = now; oled_try_init(); } return; }
  if (ui_task(&ui, &core, now, host_leds)) opg = 0;
  if (ui.blank) { if (opower) oled_pwr(false); return; }
  if (!opower) { oled_pwr(true); opg = 0; }
  if (opg < OLED_PAGES) {
    uint8_t c[] = {0x00, 0xB0 + opg, 0x00, 0x10}, d[129]; d[0] = 0x40; memcpy(d + 1, ui.fb + opg * 128, 128);
    if (i2c_w(c, 4) && i2c_w(d, 129)) opg++; else { oled_ok = false; opg = OLED_PAGES; }
  }
}

// ---------------- luu / nap cau hinh (flash) ----------------
static uint8_t store_buf[STORE_SIZE];
typedef struct { uint32_t off; const uint8_t *src; } fl_job_t;
static void fl_do(void *p) { const fl_job_t *j = (const fl_job_t *)p; flash_range_erase(j->off, FLASH_SECTOR_SIZE); flash_range_program(j->off, j->src, FLASH_SECTOR_SIZE); }
static bool store_save(void) {   // moi sector 1 lan flash_safe_execute, giua cac sector van phuc vu USB
  store_pack(&core.cfg, store_buf);
  for (int s = 0; s < STORE_SECTORS; s++) {
    fl_job_t j = {STORE_OFFSET + s * FLASH_SECTOR_SIZE, store_buf + s * FLASH_SECTOR_SIZE};
    if (flash_safe_execute(fl_do, &j, 1000) != PICO_OK) return false;
    tud_task();
  }
  return true;
}
static void store_load(void) {
  if (!store_unpack((const uint8_t *)(XIP_BASE + STORE_OFFSET), &core.cfg)) cfg_defaults(&core.cfg);
  const uint16_t *am = &core.cfg.w[W_AMB];   // app cu / cau hinh hong co the de do nhay = 0 (con tro dung yen) -> dung mac dinh
  if (am[0] == 0 && am[1] == 0) cfg_air_defaults(&core.cfg);
}

// ---------------- USB HID ----------------
static uint8_t rx_buf[32], tx_buf[32]; static volatile bool rx_pend; static bool tx_pend;
void tud_hid_set_report_cb(uint8_t itf, uint8_t id, hid_report_type_t t, uint8_t const *b, uint16_t sz) {
  (void)t;
  if (itf == 1) { if (sz >= 32 && !rx_pend) { memcpy(rx_buf, b + (sz - 32), 32); rx_pend = true; } return; }
  if (sz < 1) return;
  host_leds = (id == 0 && sz >= 2) ? b[1] : b[0];     // LED output report: bit0 Num, bit1 Caps
}
uint16_t tud_hid_get_report_cb(uint8_t i, uint8_t id, hid_report_type_t t, uint8_t *b, uint16_t n) { (void)i; (void)id; (void)t; (void)b; (void)n; return 0; }

static void hid_task(void) {
  static kb_report_t sent; static uint16_t cons_sent;
  if (!tud_hid_n_ready(0)) return;
  kb_report_t r;
  bool got = core_kb(&core, &r);
  uint16_t cn = core_cons(&core);
  if (got && memcmp(&r, &sent, sizeof r)) { tud_hid_keyboard_report(1, r.mods, r.keys); sent = r; return; }
  if (cn != cons_sent) { tud_hid_report(2, &cn, sizeof cn); cons_sent = cn; return; }
  uint8_t mb = core_mouse_buttons(&core);
  int wh = core_wheel(&core, ms());
  if (mb != m_sent || air_px || air_py || wh) {
    int sx = air_px > 127 ? 127 : air_px < -127 ? -127 : air_px, sy = air_py > 127 ? 127 : air_py < -127 ? -127 : air_py;
    if (tud_hid_mouse_report(3, mb, (int8_t)sx, (int8_t)sy, (int8_t)wh, 0)) { hid_sent++; air_px -= sx; air_py -= sy; m_sent = mb; } else hid_rej++;
    return;
  }
  if (got && core_out_busy(&core)) core_out_ack(&core);   // bao cao cua muc hien tai da gui xong
}
static void raw_task(uint32_t now) {
  if (tx_pend) { if (tud_hid_n_ready(1) && tud_hid_n_report(1, 0, tx_buf, 32)) tx_pend = false; return; }
  if (!rx_pend) return;
  uint8_t in[32]; memcpy(in, rx_buf, 32); rx_pend = false;
  for (int k = 0; k < 3; k++) { core.dbg.g[k] = air_g[k]; core.dbg.b[k] = (int16_t)(k == 0 ? air.bx : k == 1 ? air.by : air.bz); }
  core.dbg.px = (int16_t)air_px; core.dbg.py = (int16_t)air_py; core.dbg.sent = hid_sent; core.dbg.rej = hid_rej;
  core.dbg.st = (uint8_t)((core_air_on(&core) ? 1 : 0) | (air_calibrating(&air) ? 2 : 0));
  int f = proto_cmd(&core, in, tx_buf);
  if (f & PROTO_MOUSE) {                       // thu duong HID chuot: gui thang, khong qua gyro
    bool ok = tud_hid_n_ready(0) && tud_hid_mouse_report(3, core_mouse_buttons(&core), (int8_t)in[1], (int8_t)in[2], 0, 0);
    if (ok) hid_sent++; else hid_rej++;
    tx_buf[3] = ok ? 1 : 0;
  }
  if (f & PROTO_PREVIEW) ui_cmd(&ui, in[1], now);
  if ((f & PROTO_SAVE) && !store_save()) tx_buf[0] = 0xFF;
  tx_pend = true;
}

int main(void) {
  store_load();
  matrix_init(); buz_init(); core_reset(&core);
  tusb_init(); oled_init(); led_init(); air_init(&air); imu_try_init();
  uint32_t last = 0, active = ms(); (void)active;
  ui_init(&ui, &core, ms());
  while (1) {
    tud_task();
    uint32_t now = ms();
    if (now != last) {
      last = now;
      uint8_t raw[NKEYS]; bool any = scan(raw);
      if (any && tud_suspended()) tud_remote_wakeup();
      core_scan_update(&core, raw, now);
      if (core.boot_req) { oled_pwr(false); reset_usb_boot(0, 0); }
      if (core.save_req) { core.save_req = false; store_save(); }
      buz_task(); imu_task(now); oled_task(now); led_update(now);
    }
    hid_task(); raw_task(now);
  }
}
