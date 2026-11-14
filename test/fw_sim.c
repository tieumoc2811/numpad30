// Mo phong END-TO-END: nhung chinh firmware/main.c vao khung gia, bam phim vat ly theo thoi gian, giai ma bao cao HID nhu may tinh.
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/wait.h>
#include "pico_stub.h"

i2c_inst_t dummy_i2c, *i2c0 = &dummy_i2c; PIO_t dummy_pio, *pio0 = &dummy_pio; pio_program_t ws2812_program;
uint32_t sim_led, fake_ms; uint8_t sim_down[NROW][NCOL]; int sim_rowhi[32]; int sim_i2c_writes, sim_boot, sim_wakeups; uint8_t sim_i2c_cmd[2]; bool sim_suspended;
typedef struct { uint32_t t; uint8_t mods, k[6]; } kbl_t; static kbl_t kbl[4096]; static int nkb;
typedef struct { uint32_t t; uint16_t u; } cnl_t; static cnl_t cnl[1024]; static int ncn;
uint8_t sim_flash[4096]; int sim_flash_writes; uint8_t sim_rep[32][32], rq[32][32]; int sim_raw_n, nrq;
bool tud_hid_n_report(uint8_t itf, uint8_t id, const void *r, uint16_t n) { (void)itf; (void)id; if (sim_raw_n < 32) memcpy(sim_rep[sim_raw_n], r, n); sim_raw_n++; return true; }
bool tud_hid_n_ready(uint8_t i) { (void)i; return true; }
bool tud_hid_keyboard_report(uint8_t id, uint8_t mod, const uint8_t k[6]) { (void)id; kbl_t *e = &kbl[nkb++]; e->t = fake_ms; e->mods = mod; memcpy(e->k, k, 6); return true; }
bool tud_hid_report(uint8_t id, const void *r, uint16_t n) { (void)n; if (id == 2) { cnl[ncn].t = fake_ms; cnl[ncn++].u = *(const uint16_t *)r; } return true; }

typedef struct { uint32_t t; int r, c, d; } ev_t; static ev_t evs[256]; static int nev; static uint32_t endt; static jmp_buf jb;
static void ev(uint32_t t, int r, int c, int d) { evs[nev++] = (ev_t){t, r, c, d}; }
static void tap(uint32_t t, int r, int c, int len) { ev(t, r, c, 1); ev(t + len, r, c, 0); }
void tud_hid_set_report_cb(uint8_t itf, uint8_t id, hid_report_type_t t, uint8_t const *b, uint16_t sz);
static int rq_sent;
void tud_task(void) {
  fake_ms++;
  if (rq_sent < nrq && fake_ms >= 10 + 3 * (uint32_t)rq_sent) tud_hid_set_report_cb(1, 0, 2, rq[rq_sent++], 32);
  for (int i = 0; i < nev; i++) if (evs[i].t == fake_ms) sim_down[evs[i].r][evs[i].c] = evs[i].d;
  if (fake_ms >= endt) longjmp(jb, 1);
}
#define main fw_main
#include "../firmware/main.c"
#undef main

static int fails;
#define CHECK(c) do { if (!(c)) { printf("  LOI dong %d: %s\n", __LINE__, #c); fails++; } } while (0)
static int find_key(uint8_t k, uint8_t mods) { for (int i = 0; i < nkb; i++) if (kbl[i].k[0] == k && kbl[i].mods == mods) return i; return -1; }
static int run(void) { endt += 0; if (setjmp(jb) == 0) fw_main(); return 0; }

static int scenario_main(void) {   // layer, tap-hold, to hop, media, macro, BOOT, LED/OLED
  tap(100, 0, 3, 30);             // A: tap Num -> NumLock
  tap(300, 1, 3, 40);             // B: KP7
  tap(500, 0, 0, 40);             // C: Ctrl+C
  ev(700, 0, 3, 1); tap(900, 1, 3, 40); ev(1000, 0, 3, 0);   // D: giu Num, bam 7 -> Prev
  ev(1100, 0, 3, 1); tap(1400, 4, 3, 50); ev(1460, 0, 3, 0); // E: giu Num + 0 -> bat layer MACRO
  tap(1600, 0, 0, 40);            // M1 -> "Hello!"
  ev(2300, 0, 3, 1); tap(2600, 4, 5, 50);                    // F: giu Num + R4C5 -> BOOT
  endt = 2800; run();
  int a = find_key(KC_NUMLOCK, 0); CHECK(a >= 0 && kbl[a].t >= 100 && kbl[a].t < 200);
  CHECK(find_key(KC_P7, 0) >= 0);
  CHECK(find_key(KC_C, 0x01) >= 0);
  { int p = -1; for (int i = 0; i < ncn; i++) if (cnl[i].u == CN_PREV) { p = i; break; } CHECK(p >= 0 && cnl[p].t >= 900 && cnl[p].t < 1000); CHECK(p + 1 < ncn && cnl[p + 1].u == 0); }
  { char out[64]; int n = 0;   // go chuoi qua bao cao HID (chi lay cac bao cao trong khoang macro)
    for (int i = 0; i < nkb; i++) if (kbl[i].t >= 1600 && kbl[i].t < 2300 && kbl[i].k[0]) {
      for (int ch = 32; ch < 127; ch++) { uint8_t m, k; if (core_ascii((char)ch, &m, &k) && m == kbl[i].mods && k == kbl[i].k[0]) { out[n++] = (char)ch; break; } } }
    out[n] = 0; printf("  macro go ra: '%s'\n", out); CHECK(!strcmp(out, "Hello!")); }
  CHECK(sim_boot >= 1);   // firmware that khong quay lai sau reset_usb_boot; stub thi lap lai moi vong
  CHECK(sim_i2c_writes > 10);             // OLED da duoc khoi tao va ve
  CHECK(sim_led != 0);                    // den LED sang theo layer
  // khong con phim ket sau khi moi thu nha
  CHECK(kbl[nkb - 1].k[0] == 0 && kbl[nkb - 1].mods == 0);
  return fails;
}
static int scenario_saver(void) {   // LED NumLock/Caps tu may tinh, man hinh cho, danh thuc, remote wakeup
  uint8_t l1[] = {0x03}; tud_hid_set_report_cb(0, 1, 2, l1, 1); CHECK(host_leds == 3);
  uint8_t l2[] = {1, 0x02}; tud_hid_set_report_cb(0, 0, 2, l2, 2); CHECK(host_leds == 2);
  endt = 500; run(); CHECK(sim_led == (((uint32_t)0 << 16 | (uint32_t)(255 * LED_BRIGHT / 255) << 8 | 0) << 8));   // CapsLock -> do
  host_leds = 0;
  { uint8_t q[6] = {HP_SET_CFG, 0x10, 0x27, 0, 0, LED_BRIGHT}; if (nrq < 32) { memset(rq[nrq], 0, 32); memcpy(rq[nrq], q, 6); nrq++; } }   // man hinh cho sau 10 s
  endt += 11000; run();
  CHECK(sim_i2c_cmd[1] == 0xAE); CHECK(sim_led == 0);
  sim_suspended = true; sim_down[1][3] = 1; endt += 20; run();
  CHECK(sim_i2c_cmd[1] == 0xAF); CHECK(sim_wakeups > 0);
  return fails;
}
// Raw HID: xep hang cac goi (moi goi cach 3 ms) de tud_task() tiem vao firmware trong MOT lan chay, ghi lai tung tra loi
static void rawq(const uint8_t *q, int n) { if (nrq < 32) { memset(rq[nrq], 0, 32); memcpy(rq[nrq], q, n); nrq++; } }
static int scenario_raw(void) {     // Raw HID: doc/sua phim, ten, mau, macro, luu flash, nap lai khi khoi dong
  memset(sim_flash, 0xFF, sizeof sim_flash);
  rawq((uint8_t[]){HP_INFO}, 1);                                              // 0
  rawq((uint8_t[]){HP_GET_KEYS, 0, 0, 2}, 4);                                 // 1
  rawq((uint8_t[]){HP_SET_KEYS, 0, 5, 1, (uint8_t)KC_Y, (uint8_t)(KC_Y >> 8)}, 6);   // 2
  rawq((uint8_t[]){HP_SET_NAME, 1, 3, 'A', 'B', 'C'}, 6);                     // 3
  rawq((uint8_t[]){HP_SET_COLORS, 0, 3, 1, 2, 3, 4, 5, 6, 7, 8, 9}, 12);        // 4 mau layer 0..2
  rawq((uint8_t[]){HP_SET_MACRO, 0, 0, 2, 'h', 'i'}, 6);                      // 5
  rawq((uint8_t[]){HP_GET_KEYS, NLAYER, 0, 1}, 4);                            // 6 loi layer (vuot toi da)
  rawq((uint8_t[]){HP_SET_CFG, 0x10, 0x27, 0, 0, 77}, 6);                     // 7 ss = 10000 ms, sang 77
  rawq((uint8_t[]){HP_SET_NLAYERS, 5}, 2);                                    // 8 con 5 layer
  rawq((uint8_t[]){HP_SET_NLAYERS, NLAYER + 1}, 2);                           // 9 loi
  rawq((uint8_t[]){HP_SET_NLAYERS, 0}, 2);                                    // 10 loi
  rawq((uint8_t[]){HP_SET_KEYS, 12, 0, 1, (uint8_t)KC_Z, (uint8_t)(KC_Z >> 8)}, 6);   // 11 layer 12 (du lieu luon co san)
  rawq((uint8_t[]){HP_GET_COLORS, 0, 3}, 3);                                  // 12
  rawq((uint8_t[]){HP_SAVE}, 1);                                              // 13
  endt = 100; run();
  CHECK(sim_raw_n == 14);
  CHECK(sim_rep[0][0] == HP_INFO && sim_rep[0][1] == HP_VER && sim_rep[0][2] == NLAYER_DEF && sim_rep[0][3] == NKEYS && sim_rep[0][4] == NMACROS && sim_rep[0][8] == NLAYER);
  CHECK(sim_rep[1][0] == HP_GET_KEYS && (sim_rep[1][4] | sim_rep[1][5] << 8) == C_(KC_C));
  CHECK(sim_rep[2][0] == HP_SET_KEYS && sim_rep[6][0] == HP_ERR && sim_rep[13][0] == HP_SAVE);
  CHECK(sim_rep[8][0] == HP_SET_NLAYERS && sim_rep[8][1] == 5 && core.nl == 5 && cfg.nl == 5);
  CHECK(sim_rep[9][0] == HP_ERR && sim_rep[10][0] == HP_ERR && sim_rep[11][0] == HP_SET_KEYS);
  CHECK(sim_rep[12][0] == HP_GET_COLORS && sim_rep[12][2] == 3 && sim_rep[12][3] == 1 && sim_rep[12][11] == 9);
  CHECK(sim_flash_writes == 1 && !memcmp(sim_flash, "NP30", 4));
  cfg_store_t before = cfg; memset(&cfg, 0, sizeof cfg); cfg_load();            // gia lap khoi dong lai tu flash
  CHECK(cfg.nl == 5 && cfg.km[12][0] == KC_Z && cfg.km[0][5] == KC_Y && !strcmp(cfg.names[1], "ABC") && !strcmp(cfg.macs[0], "hi") && cfg.colors[2][2] == 9);
  CHECK(cfg.ss_ms == 10000 && cfg.bright == 77 && !memcmp(cfg.km, before.km, sizeof cfg.km) && macs[0].text == cfg.macs[0]);
  sim_flash[100] ^= 0xFF; cfg_load(); CHECK(cfg.km[0][5] != KC_Y && cfg.bright == LED_BRIGHT);   // CRC hong -> ve mac dinh
  return fails;
}
int main(int argc, char **argv) {
  (void)argv; int rc = 0;
  for (int s = 0; s < 3; s++) {   // moi kich ban chay trong tien trinh rieng (trang thai tinh cua main.c khoi tron lan)
    fflush(stdout); pid_t p = fork();
    if (p == 0) { printf("kich ban %d (%s)\n", s, s == 2 ? "Raw HID + flash" : s ? "man hinh cho / LED may tinh" : "layer, tap-hold, macro, BOOT"); fflush(stdout); exit(s == 2 ? scenario_raw() : s ? scenario_saver() : scenario_main()); }
    int st; waitpid(p, &st, 0); if (!WIFEXITED(st) || WEXITSTATUS(st)) rc = 1;
  }
  if (argc > 5) return 2;
  printf(rc ? "e2e: CO LOI\n" : "e2e: tat ca dat\n"); return rc;
}
