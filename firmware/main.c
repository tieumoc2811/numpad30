// Numpad30 RP2350-Zero: ma tran 5x7 co diode (hang GP0-4, cot GP5-11), HID ban phim + media, OLED SSD1306 (I2C0 SDA=GP12 SCL=GP13),
// 3 layer (NUM / FN / MACRO), tap-hold, macro go chuoi, den RGB onboard (GP16) theo layer. Logic phim nam trong core.h (kiem thu tren PC).
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/flash.h"
#include "hardware/pwm.h"
#include "pico/flash.h"
#include "ws2812.pio.h"
#include "tusb.h"
#include "font.h"
#define CORE_IMPL
#include "keymap.h"
#define HP_IMPL
#include "hid_proto.h"
#define FS_IMPL
#include "flash_store.h"

static const uint8_t row_pins[NROW] = ROW_PINS, col_pins[NCOL] = COL_PINS;
static const char *const def_names[NLAYER] = NUM_LAYER_NAMES;
static const uint8_t def_colors[NLAYER][3] = {{0, 60, 255}, {255, 110, 0}, {0, 255, 60}, {255, 0, 160}, {255, 230, 0}, {0, 220, 255}, {150, 60, 255}, {255, 255, 255}};
static core_t core;
static cfg_store_t cfg;                          // cau hinh dang chay (RAM), sua duoc qua Raw HID
static core_cfg_t cf;                            // cau hinh dua cho core.h
static const uint8_t ledch_pins[LED_N] = LEDCH_PINS;
static uint8_t led_prev_mode;
static bool greet_on; static uint32_t greet_t0, last_key;
#define DEF_NMAC ((int)(sizeof macros / sizeof macros[0]))
static int led_last = -2;
static uint8_t host_leds;                       // bit0 NumLock, bit1 CapsLock (tu may tinh)
static bool saver;                              // dang o che do tiet kiem (OLED + LED tat)

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

// ---------------- LED RGB onboard (WS2812) ----------------
static void led_init(void) {
  uint off = pio_add_program(pio0, &ws2812_program);
  pio_sm_config c = ws2812_program_get_default_config(off);
  sm_config_set_sideset_pins(&c, LED_PIN); sm_config_set_out_shift(&c, false, true, 24); sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
  sm_config_set_clkdiv(&c, clock_get_hz(clk_sys) / (800000.0f * (ws2812_T1 + ws2812_T2 + ws2812_T3)));
  pio_gpio_init(pio0, LED_PIN); pio_sm_set_consecutive_pindirs(pio0, 0, LED_PIN, 1, true);
  pio_sm_init(pio0, 0, off, &c); pio_sm_set_enabled(pio0, 0, true);
}
static void led_put(uint8_t r, uint8_t g, uint8_t b) {
  r = r * cfg.bright / 255; g = g * cfg.bright / 255; b = b * cfg.bright / 255;
  pio_sm_put_blocking(pio0, 0, ((uint32_t)g << 16 | (uint32_t)r << 8 | b) << 8);
}
static void led_update(void) {
  int layer = core_top_layer(&core);
  int st = saver ? -1 : layer | (host_leds & 2 ? 0x10 : 0);
  if (st == led_last) return;
  led_last = st;
  if (saver) { led_put(0, 0, 0); return; }
  if (host_leds & 2) { led_put(255, 0, 0); return; }   // CapsLock bat -> do
  led_put(cfg.colors[layer][0], cfg.colors[layer][1], cfg.colors[layer][2]);
}

// ---------------- OLED SSD1306 ----------------
#define OLED_PAGES (OLED_H / 8)
static uint8_t fb[128 * OLED_PAGES];
static bool oled_ok, odirty = true, opower = true;
static int opg = OLED_PAGES;
static void px(int x, int y, bool on) {
  if ((unsigned)x >= 128 || (unsigned)y >= OLED_H) return;
#if OLED_ROTATE
  x = 127 - x; y = OLED_H - 1 - y;
#endif
  if (on) fb[(y >> 3) * 128 + x] |= 1 << (y & 7); else fb[(y >> 3) * 128 + x] &= ~(1 << (y & 7));
}
static void box(int x, int y, int w, int h, bool on) { for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) px(x + i, y + j, on); }
static int tw(const char *t, int s) { int n = strlen(t); return n ? n * 6 * s - s : 0; }
static void txt(int x, int y, int s, const char *t, bool on) {
  for (; *t; t++, x += 6 * s) {
    char ch = *t >= 'a' && *t <= 'z' ? *t - 32 : *t; const char *g = strchr(FC, ch); int i = g ? g - FC : 0;
    for (int c = 0; c < 5; c++) for (int r = 0; r < 7; r++) if (FD[i * 5 + c] >> r & 1) box(x + c * s, y + r * s, s, s, on);
  }
}
static void oled_greet(uint32_t now) {      // chu chay tu phai sang trai khi cam USB
  const int sc = OLED_H == 32 ? 2 : 3; int w = tw(cfg.ext.greet, sc);
  int x = 128 - (int)((now - greet_t0) / 10);
  if (x + w < 0) { greet_on = false; return; }
  txt(x, (OLED_H - 7 * sc) / 2, sc, cfg.ext.greet, true);
}
static void oled_render(void) {
  memset(fb, 0, sizeof fb);
  if (greet_on) { oled_greet(ms()); if (greet_on) { opg = 0; return; } }
  char lab[24]; static char shown[24] = "READY";
  if (core.last_kc) core_label(core.last_kc, shown, sizeof shown);
  strcpy(lab, shown);
  const char *ln = cfg.names[core_top_layer(&core)];
#if OLED_H == 32
  txt(0, 0, 2, ln, true);
  box(102, 0, 21, 9, host_leds & 1); txt(104, 1, 1, "NUM", !(host_leds & 1));
  box(102, 11, 21, 9, host_leds & 2); txt(104, 12, 1, "CAP", !(host_leds & 2));
  int s = tw(lab, 2) <= 128 ? 2 : 1;
  txt(0, s == 2 ? 18 : 22, s, lab, true);
#else
  txt(0, 0, 3, ln, true);
  box(94, 0, 28, 11, host_leds & 1); txt(97, 2, 1, "NUM", !(host_leds & 1));
  box(94, 14, 28, 11, host_leds & 2); txt(97, 16, 1, "CAP", !(host_leds & 2));
  int s = tw(lab, 3) <= 128 ? 3 : 2;
  txt(0, 34, s, lab, true);
#endif
  opg = 0;
}
static bool i2c_w(const uint8_t *b, size_t n) { return i2c_write_timeout_us(i2c0, OLED_ADDR, b, n, false, 4000) == (int)n; }
static void oled_pwr(bool on) { uint8_t c[] = {0x00, on ? 0xAF : 0xAE}; i2c_w(c, 2); opower = on; }
static void oled_try_init(void) {
  const uint8_t ini[] = {0x00, 0xAE, 0xD5, 0x80, 0xA8, OLED_H - 1, 0xD3, 0x00, 0x40, 0x8D, 0x14, 0x20, 0x02, 0xA1, 0xC8,
                         0xDA, OLED_H == 32 ? 0x02 : 0x12, 0x81, 0x7F, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF};
  oled_ok = i2c_w(ini, sizeof ini); opower = true; odirty = true;
}
static void oled_init(void) {
  i2c_init(i2c0, 400000);
  gpio_set_function(OLED_SDA, GPIO_FUNC_I2C); gpio_set_function(OLED_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(OLED_SDA); gpio_pull_up(OLED_SCL);
  oled_try_init();
}
static void oled_task(uint32_t now) {     // goi moi ms: moi lan gui toi da 1 page de khong chan quet phim / USB
  static uint32_t retry, seq, leds_seen = 0xFF, gtick; static int layer_seen = -1, logo_f = -1;
  if (!oled_ok) { if (now - retry >= 2000) { retry = now; oled_try_init(); } return; }
  bool logo = saver && cfg.ext.saver_mode == 1 && cfg.ext.lg_n;
  if (saver && !logo) return;
  if (logo) {                                  // man hinh cho: logo dong
    int f = (int)((now / cfg.ext.lg_ms) % cfg.ext.lg_n);
    if (f != logo_f || odirty) {
      logo_f = f; odirty = false; memset(fb, 0, sizeof fb);
      for (int y = 0; y < 32; y++) for (int x = 0; x < 128; x++) if (cfg.ext.logo[f][(y >> 3) * 128 + x] >> (y & 7) & 1) px(x, y + (OLED_H - 32) / 2, true);
      opg = 0;
    }
  } else {
    logo_f = -1;
    int layer = core_top_layer(&core);
    if (core.press_seq != seq || layer != layer_seen || host_leds != leds_seen) { seq = core.press_seq; layer_seen = layer; leds_seen = host_leds; odirty = true; }
    if (greet_on && now - gtick >= 30) { gtick = now; odirty = true; }
    if (odirty) { odirty = false; oled_render(); }
  }
  if (opg < OLED_PAGES) {
    uint8_t c[] = {0x00, 0xB0 + opg, 0x00, 0x10}, d[129]; d[0] = 0x40; memcpy(d + 1, fb + opg * 128, 128);
    if (i2c_w(c, 4) && i2c_w(d, 129)) opg++; else { oled_ok = false; opg = OLED_PAGES; }
  }
}

// ---------------- cau hinh runtime + flash ----------------
static void cfg_defaults(void) {
  memset(&cfg, 0, sizeof cfg);
  for (int l = 0; l < NLAYER; l++) for (int i = 0; i < NKEYS; i++) cfg.km[l][i] = l < DEF_NLAYER ? keymap[l][i] : KC_TRNS;
  for (int l = 0; l < NLAYER; l++) { if (l < DEF_NLAYER) strncpy(cfg.names[l], def_names[l], HP_NAME_MAX); else { cfg.names[l][0] = 'L'; cfg.names[l][1] = '0' + l; } }
  memcpy(cfg.colors, def_colors, sizeof cfg.colors);
  for (int i = 0; i < DEF_NMAC && i < NMACROS; i++) core_macro_text(cfg.macs[i], macros[i].text);
  cfg.ss_ms = SCREENSAVER_MS; cfg.bright = LED_BRIGHT;
  memset(cfg.ext.cb_a, 0xFF, NCB); memset(cfg.ext.cb_b, 0xFF, NCB);
  cfg.ext.nlayer = DEF_NLAYER; cfg.ext.led_speed = 128; cfg.ext.led_bri = 200; cfg.ext.lg_ms = 150;
  strcpy(cfg.ext.greet, "NUMPAD30");
}
static const uint8_t *flash_cfg(void) { return (const uint8_t *)(XIP_BASE + CFG_FLASH_OFFSET); }
#define CFG_BYTES (CFG_FLASH_SECTORS * FLASH_SECTOR_SIZE)
static uint8_t fbuf[CFG_BYTES]; static size_t flen;
static void flash_job(void *p) {              // 1 sector moi lan de USB khong bi chan lau
  uint32_t o = (uint32_t)(uintptr_t)p * FLASH_SECTOR_SIZE;
  flash_range_erase(CFG_FLASH_OFFSET + o, FLASH_SECTOR_SIZE);
  if (o < flen) { size_t n = flen - o; if (n > FLASH_SECTOR_SIZE) n = FLASH_SECTOR_SIZE; flash_range_program(CFG_FLASH_OFFSET + o, fbuf + o, n); }
}
static void cfg_save(void) {
  memset(fbuf, 0xFF, sizeof fbuf);
  size_t n = fs_pack(&cfg, fbuf, sizeof fbuf);
  if (!n) return;
  flen = (n + FLASH_PAGE_SIZE - 1) & ~(size_t)(FLASH_PAGE_SIZE - 1);
  for (int s = 0; s < CFG_FLASH_SECTORS; s++) flash_safe_execute(flash_job, (void *)(uintptr_t)s, 1000);
}
static void cfg_link(void) {
  cf.km = (const kc_t (*)[NKEYS])cfg.km; cf.nlayer = cfg.ext.nlayer;
  cf.td = (const uint16_t (*)[3])cfg.ext.td; cf.cb_a = cfg.ext.cb_a; cf.cb_b = cfg.ext.cb_b; cf.cb_r = cfg.ext.cb_r;
  cf.macs = (const uint8_t (*)[MSLOT])cfg.macs; cf.nmacros = NMACROS;
}
static void cfg_load(void) {
  cfg_defaults();
  static cfg_store_t tmp; tmp = cfg;            // hong/rong -> giu mac dinh
  if (fs_unpack(&tmp, flash_cfg(), CFG_BYTES)) cfg = tmp;
  cfg_link();
}

// ---------------- den LED ngoai: 6 kenh PWM ----------------
static void ledch_init(void) {
  for (int i = 0; i < LED_N; i++) {
    gpio_set_function(ledch_pins[i], GPIO_FUNC_PWM);
    uint sl = pwm_gpio_to_slice_num(ledch_pins[i]);
    pwm_set_wrap(sl, 255); pwm_set_clkdiv(sl, 4.0f); pwm_set_enabled(sl, true);
    pwm_set_gpio_level(ledch_pins[i], LEDCH_ACTIVE_HIGH ? 0 : 255);
  }
}
static void ledch_task(uint32_t now) {
  static uint32_t nxt;
  if ((int32_t)(now - nxt) < 0) return;
  nxt = now + 8;
  uint8_t layer = (uint8_t)core_top_layer(&core);
  for (int i = 0; i < LED_N; i++) {
    uint32_t v = saver ? 0 : (uint32_t)ledfx(cfg.ext.led_mode, i, now, cfg.ext.led_speed, layer, now - last_key) * cfg.ext.led_bri / 255;
    pwm_set_gpio_level(ledch_pins[i], LEDCH_ACTIVE_HIGH ? v : 255 - v);
  }
}

// ---------------- USB HID ----------------
static uint8_t raw_req[HP_PKT], raw_rep[HP_PKT]; static bool raw_pending, raw_reply;
void tud_hid_set_report_cb(uint8_t itf, uint8_t id, hid_report_type_t t, uint8_t const *b, uint16_t sz) {
  (void)t;
  if (itf == 1) {                                     // Raw HID tu app Studio
    memset(raw_req, 0, sizeof raw_req); memcpy(raw_req, b, sz < HP_PKT ? sz : HP_PKT);
    raw_pending = true; return;
  }
  if (sz < 1) return;
  host_leds = (id == 0 && sz >= 2) ? b[1] : b[0];     // LED output report: bit0 Num, bit1 Caps
}
uint16_t tud_hid_get_report_cb(uint8_t i, uint8_t id, hid_report_type_t t, uint8_t *b, uint16_t n) { (void)i; (void)id; (void)t; (void)b; (void)n; return 0; }

static void raw_task(void) {
  if (raw_pending && !raw_reply) {
    hp_handle(&cfg, raw_req, raw_rep); raw_pending = false; raw_reply = true; led_last = -2; odirty = true;
    core.toggled &= (uint8_t)((1u << cfg.ext.nlayer) - 1);
  }
  if (raw_reply && tud_hid_n_ready(1) && tud_hid_n_report(1, 0, raw_rep, HP_PKT)) {
    raw_reply = false;
    if (cfg.save_req) { cfg.save_req = false; cfg_save(); }   // ghi flash sau khi da tra loi
    if (cfg.defaults_req) { cfg_defaults(); cfg_link(); core.toggled = 0; }   // nap mac dinh vao RAM (chua ghi flash)
  }
}
static void hid_task(void) {
  static kb_report_t sent; static uint16_t cons_sent;
  if (!tud_hid_n_ready(0)) return;
  kb_report_t r;
  if (core_kb(&core, &r)) {
    bool macro = core_macro_active(&core);
    if (memcmp(&r, &sent, sizeof r)) { tud_hid_keyboard_report(1, r.mods, r.keys); sent = r; if (macro) core_macro_ack(&core); return; }
    if (macro) core_macro_ack(&core);
  }
  uint16_t cn = core_cons(&core);
  if (cn != cons_sent) { tud_hid_report(2, &cn, sizeof cn); cons_sent = cn; }
}

int main(void) {
  cfg_load(); matrix_init(); core_init(&core);
  greet_t0 = ms();
  tusb_init(); oled_init(); led_init(); ledch_init();
  greet_on = cfg.ext.greet[0] != 0; greet_t0 = ms();
  uint32_t last = 0, active = ms();
  while (1) {
    tud_task();
    uint32_t now = ms();
    if (now != last) {
      last = now;
      uint8_t raw[NKEYS]; bool any = scan(raw);
      if (any) {
        active = now;
        if (saver) { saver = false; oled_pwr(true); odirty = true; }
        if (tud_suspended()) tud_remote_wakeup();
      }
      cf.nlayer = cfg.ext.nlayer;
      core_scan_update(&core, raw, now, &cf);
      static uint32_t seq_seen; if (core.press_seq != seq_seen) { seq_seen = core.press_seq; last_key = now; }
      if (core.ledreq) {
        int8_t r = core.ledreq; core.ledreq = 0; uint8_t *m = &cfg.ext.led_mode;
        if (r == 2) { if (*m == 1) *m = led_prev_mode; else { led_prev_mode = *m; *m = 1; } }
        else *m = (uint8_t)((*m + (r > 0 ? 1 : LED_NMODES - 1)) % LED_NMODES);
      }
      if (core.boot_req) { oled_pwr(false); reset_usb_boot(0, 0); }
      if (cfg.ss_ms && !saver && now - active >= cfg.ss_ms) { saver = true; if (cfg.ext.saver_mode == 1 && cfg.ext.lg_n) odirty = true; else oled_pwr(false); }
      oled_task(now); led_update(); ledch_task(now);
    }
    hid_task(); raw_task();
  }
}
