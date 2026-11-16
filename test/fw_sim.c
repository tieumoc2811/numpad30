// Mo phong END-TO-END: nhung chinh firmware/main.c vao khung gia, bam phim vat ly theo thoi gian, giai ma bao cao HID nhu may tinh.
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/wait.h>
#include "pico_stub.h"

i2c_inst_t dummy_i2c, *i2c0 = &dummy_i2c; PIO_t dummy_pio, *pio0 = &dummy_pio; pio_program_t ws2812_program;
uint8_t sim_imu_reg, sim_pwr2; int16_t sim_gyro[3]; int sim_imu_present = 1; uint8_t sim_mb; int sim_mdx, sim_mdy, sim_mrep, sim_wheel, sim_wrep;
uint8_t sim_flash[2 * 1024 * 1024]; int sim_flash_erases; static uint8_t raw_tx[32]; static int raw_txn;
bool tud_hid_n_report(uint8_t itf, uint8_t id, const void *r, uint16_t n) { (void)id; if (itf == 1 && n == 32) { memcpy(raw_tx, r, 32); raw_txn++; } return true; }
typedef struct { uint32_t t; void (*f)(void); } act_t; static act_t acts[64]; static int nacts;
uint32_t sim_buz_log[64]; int sim_buz_n, sim_buz_on; float sim_pwm_div; uint16_t sim_pwm_wrap, sim_pwm_lv;
uint32_t sim_led, fake_ms; uint8_t sim_down[NROW][NCOL]; int sim_rowhi[32]; int sim_i2c_writes, sim_boot, sim_wakeups; uint8_t sim_i2c_cmd[2]; bool sim_suspended;
typedef struct { uint32_t t; uint8_t mods, k[6]; } kbl_t; static kbl_t kbl[4096]; static int nkb;
typedef struct { uint32_t t; uint16_t u; } cnl_t; static cnl_t cnl[1024]; static int ncn;
bool sim_busy, sim_shake; bool tud_hid_n_ready(uint8_t i) { return i == 1 ? true : !sim_busy; }
bool tud_hid_keyboard_report(uint8_t id, uint8_t mod, const uint8_t k[6]) { (void)id; kbl_t *e = &kbl[nkb++]; e->t = fake_ms; e->mods = mod; memcpy(e->k, k, 6); return true; }
bool tud_hid_mouse_report(uint8_t id, uint8_t b, int8_t x, int8_t y, int8_t v, int8_t h) { (void)id; (void)h; if (v) { sim_wheel += v; sim_wrep++; } sim_mb |= b; sim_mdx += x; sim_mdy += y; sim_mrep++; return true; }
bool tud_hid_report(uint8_t id, const void *r, uint16_t n) { (void)n; if (id == 2) { cnl[ncn].t = fake_ms; cnl[ncn++].u = *(const uint16_t *)r; } return true; }

typedef struct { uint32_t t; int r, c, d; } ev_t; static ev_t evs[256]; static int nev; static uint32_t endt; static jmp_buf jb;
static void ev(uint32_t t, int r, int c, int d) { evs[nev++] = (ev_t){t, r, c, d}; }
static void tap(uint32_t t, int r, int c, int len) { ev(t, r, c, 1); ev(t + len, r, c, 0); }
void tud_task(void) {
  fake_ms++; if (sim_shake) sim_gyro[2] = (int16_t)(((fake_ms / 7) & 1) ? 3000 : 0);
  for (int i = 0; i < nacts; i++) if (acts[i].t == fake_ms) acts[i].f();
  for (int i = 0; i < nev; i++) if (evs[i].t == fake_ms) sim_down[evs[i].r][evs[i].c] = evs[i].d;
  if (fake_ms >= endt) longjmp(jb, 1);
}
#define main fw_main
#include "../firmware/main.c"
#undef main

#define POS_(r, c) ((r) * NCOL + (c))
static int fails;
#define CHECK(c) do { if (!(c)) { printf("  LOI dong %d: %s\n", __LINE__, #c); fails++; } } while (0)
static int find_key(uint8_t k, uint8_t mods) { for (int i = 0; i < nkb; i++) if (kbl[i].k[0] == k && kbl[i].mods == mods) return i; return -1; }
static int run(void) { endt += 0; if (setjmp(jb) == 0) fw_main(); return 0; }


// ---- ho tro lenh Raw HID: dat bao cao OUT vao ham goi lai cua firmware, firmware tra loi qua tud_hid_n_report ----
static uint8_t R_in[32];
static void raw_send(void) { tud_hid_set_report_cb(1, 0, 2, R_in, 32); }
static void act_at(uint32_t t, void (*f)(void)) { acts[nacts].t = t; acts[nacts++].f = f; }
static void mk(uint8_t cmd, unsigned off, unsigned n) { memset(R_in, 0, 32); R_in[0] = cmd; R_in[1] = off & 255; R_in[2] = (uint8_t)n; R_in[31] = off >> 8; }
static uint8_t got[16][32]; static int ngot;
static void keep(void) { memcpy(got[ngot++], raw_tx, 32); }
static void a_info(void) { mk(CMD_INFO, 0, 0); raw_send(); }
static void a_wkey(void) { mk(CMD_WRITE, W_KM + POS_(1, 3), 1); R_in[3] = KC_A; raw_send(); }    // P7 -> A tren layer 0
static void a_rkey(void) { mk(CMD_READ, W_KM + POS_(1, 3), 1); raw_send(); }
static void a_wmac(void) { mk(CMD_MWRITE, 2 * MSLOT + 0, 6); memcpy(R_in + 3, "\x01\x00\x00\x00", 4); raw_send(); }
static void a_save(void) { mk(CMD_SAVE, 0, 0); raw_send(); }
static void a_wosb(void) { mk(CMD_WRITE, W_OSB, 1); R_in[3] = 2; raw_send(); }              // timeout 2 s
static void a_wosb5(void) { mk(CMD_WRITE, W_OSB + 5, 1); R_in[3] = 3; raw_send(); }         // kieu 3: tat man hinh
static void a_prev(void) { mk(CMD_PREVIEW, 0, 0); R_in[1] = 0; raw_send(); }
static void a_defaults(void) { mk(CMD_DEFAULTS, 0, 0); raw_send(); }

static int scenario_main(void) {   // layer, tap-hold, to hop, media, macro mac dinh, BOOT, LED/OLED
  tap(100, 0, 3, 30); tap(300, 1, 3, 40); tap(500, 0, 0, 40);
  ev(700, 0, 3, 1); tap(900, 1, 3, 40); ev(1000, 0, 3, 0);
  ev(1100, 0, 3, 1); tap(1400, 4, 3, 50); ev(1460, 0, 3, 0);   // giu Num + 0 -> bat layer MACRO
  tap(1600, 0, 0, 40);                                          // M1 -> "Hello!"
  ev(2300, 0, 3, 1); tap(2600, 4, 5, 50);                       // giu Num + R4C5 -> BOOT
  endt = 2800; run();
  int a = find_key(KC_NUMLOCK, 0); CHECK(a >= 0 && kbl[a].t >= 100 && kbl[a].t < 200);
  CHECK(find_key(KC_P7, 0) >= 0); CHECK(find_key(KC_C, 0x01) >= 0);
  { int p = -1; for (int i = 0; i < ncn; i++) if (cnl[i].u == CN_PREV) { p = i; break; } CHECK(p >= 0 && cnl[p].t >= 900 && cnl[p].t < 1000); CHECK(p + 1 < ncn && cnl[p + 1].u == 0); }
  { char out[64]; int n = 0;
    for (int i = 0; i < nkb; i++) if (kbl[i].t >= 1600 && kbl[i].t < 2300 && kbl[i].k[0]) {
      for (int ch = 32; ch < 127; ch++) { uint8_t m, k; if (core_ascii((char)ch, &m, &k) && m == kbl[i].mods && k == kbl[i].k[0]) { out[n++] = (char)ch; break; } } }
    out[n] = 0; printf("  macro go ra: '%s'\n", out); CHECK(!strcmp(out, "Hello!")); }
  CHECK(sim_boot >= 1); CHECK(sim_i2c_writes > 10); CHECK(sim_led != 0);
  CHECK(kbl[nkb - 1].k[0] == 0 && kbl[nkb - 1].mods == 0);
  return fails;
}
static uint8_t snap_cmd; static uint32_t snap_led;
static void snap(void) { snap_cmd = sim_i2c_cmd[1]; snap_led = sim_led; }
static void susp(void) { sim_suspended = true; }
static int scenario_saver(void) {   // LED NumLock/Caps tu may tinh, screensaver cau hinh bang app, danh thuc, remote wakeup
  uint8_t l1[] = {0x03}; tud_hid_set_report_cb(0, 1, 2, l1, 1); CHECK(host_leds == 3);
  uint8_t l2[] = {1, 0x02}; tud_hid_set_report_cb(0, 0, 2, l2, 2); CHECK(host_leds == 2);
  endt = 500; run(); CHECK(sim_led == (((uint32_t)0 << 16 | (uint32_t)(255 * 24 / 255) << 8 | 0) << 8));   // CapsLock -> do
  host_leds = 0;
  act_at(600, a_wosb); act_at(610, a_wosb5);   // screensaver sau 2 s, kieu tat man hinh
  act_at(8900, snap); act_at(8950, susp); ev(9000, 1, 3, 1);
  endt = 9100; run();   // loi chao chay ~2.5 s truoc, roi 2 s khong bam phim
  CHECK(snap_cmd == 0xAE && snap_led == 0);
  CHECK(sim_i2c_cmd[1] == 0xAF); CHECK(sim_wakeups > 0);
  return fails;
}
static int scenario_proto(void) {   // Raw HID: info, ghi phim co hieu luc ngay, macro, luu flash, "khoi dong lai" van con
  act_at(20, a_info); act_at(22, keep);
  act_at(30, a_wkey); act_at(32, keep);
  act_at(40, a_rkey); act_at(42, keep);
  act_at(50, a_wmac); act_at(52, keep);
  act_at(60, a_save); act_at(300, keep);     // flash_safe_execute dong bo trong stub
  tap(100, 1, 3, 40);                        // phim P7 da thanh A
  int e0 = sim_flash_erases;
  endt = 500; run();
  CHECK(ngot == 5);
  CHECK(got[0][0] == 5 && got[0][1] == 32 && (got[0][2] | got[0][3] << 8) == NW && got[0][4] == FW_VER && got[0][6] == 32);
  CHECK(got[1][0] == CMD_WRITE); CHECK(got[2][0] == CMD_READ && got[2][3] == KC_A);
  CHECK(find_key(KC_A, 0) >= 0 && find_key(KC_P7, 0) < 0);
  CHECK(got[4][0] == CMD_SAVE); CHECK(sim_flash_erases - e0 == STORE_SECTORS);
  { store_hdr_t h; memcpy(&h, sim_flash + STORE_OFFSET, sizeof h); CHECK(h.magic == STORE_MAGIC); }
  // khoi dong lai (fw_main chay lai tu dau, doc flash): phim A van con, macro moi van con
  nkb = 0; nacts = 0; ngot = 0; nev = 0;
  tap(endt + 100, 1, 3, 40);
  endt += 400; run();
  CHECK(find_key(KC_A, 0) >= 0 && find_key(KC_P7, 0) < 0);
  CHECK(core.cfg.mac[2][0] == 1);
  // ve mac dinh: P7 tro lai
  nkb = 0; nacts = 0; act_at(endt + 20, a_defaults); tap(endt + 100, 1, 3, 40);
  endt += 300; run();
  CHECK(find_key(KC_P7, 0) >= 0 && find_key(KC_A, 0) < 0);
  // flash hong (CRC sai) -> mac dinh
  sim_flash[STORE_OFFSET + 100] ^= 0xFF; nkb = 0; nacts = 0; tap(endt + 100, 1, 3, 40); endt += 300; run();
  CHECK(core.cfg.w[W_KM + POS_(1, 3)] == KC_P7 && find_key(KC_P7, 0) >= 0);
  return fails;
}
static void a_gyro_hold(void) { sim_gyro[2] = (int16_t)(65.5 * 100); }   // 100 do/s quanh Z
static void a_air_cfg(void) { core.cfg.w[W_KM + POS_(1, 3)] = KC_AIR; core.cfg.w[W_KM + POS_(1, 4)] = KC_MB1; core.cfg.w[W_KM + POS_(2, 3)] = KC_WHUP; core.cfg.w[W_KM + POS_(2, 4)] = KC_WHDN; }
static void a_lnw(void) { mk(CMD_LNWRITE, 0, 0); R_in[1] = 3; R_in[2] = 6; memcpy(R_in + 3, "my-Tab", 6); raw_send(); }   // layer 3: "MY-TAB"
static void a_lnr(void) { mk(CMD_LNREAD, 0, 0); R_in[1] = 3; raw_send(); }
static int sn3[3]; static void a_snap(void) { sn3[0] = sim_mrep; sn3[1] = sim_mdx; sn3[2] = sim_mdy; }
static void a_dbg(void) { mk(CMD_AIRDBG, 0, 0); raw_send(); }
static void a_mouse(void) { mk(CMD_MOUSE, 0, 0); R_in[1] = (uint8_t)(int8_t)-20; R_in[2] = 20; raw_send(); }
static int scenario_air(void) {
  nacts = 0; nev = 0; sim_mdx = sim_mdy = sim_mrep = 0; sim_gyro[0] = sim_gyro[1] = sim_gyro[2] = 40;
  act_at(50, a_air_cfg);                            // (main() nap lai cau hinh luc khoi dong nen gan phim sau do)
  act_at(1500, a_gyro_hold);                        // sau khi hieu chuan xong (~1 s), quay Z
  tap(1700, 1, 3, 600);                             // giu AIR 1700..2300
  tap(2500, 1, 4, 100);                             // chuot trai
  act_at(1900, a_dbg); act_at(1902, keep);          // lenh 11 luc dang giu AIR va gyro dang quay
  act_at(2600, a_dbg); act_at(2602, keep);          // lenh 11 luc da nha AIR
  act_at(3099, a_snap); act_at(3100, a_mouse); act_at(3102, keep);   // lenh 12: gui thang (-20, +20), khong qua gyro
  tap(3300, 2, 3, 40); tap(3500, 2, 4, 700);       // cuon len 1 lan; giu cuon xuong 700 ms (nac dau + lap lai)
  act_at(4400, a_lnw); act_at(4402, keep); act_at(4410, a_lnr); act_at(4412, keep);
  endt = 4600; run();
  CHECK(got[2][0] == CMD_MOUSE && got[2][3] == 1 && sim_mrep == sn3[0] + 1 + sim_wrep && sim_mdx == sn3[1] - 20 && sim_mdy == sn3[2] + 20);
  CHECK(sim_wheel < 0 && sim_wrep >= 5 && sim_wrep <= 14);                      // +1 roi -1 * n (nac dau ngay, lap lai sau 350 ms moi 50 ms)
  CHECK(got[3][0] == CMD_LNWRITE && got[4][0] == CMD_LNREAD && memcmp(got[4] + 3, "MY-TAB", 7) == 0 && !strcmp(core.cfg.lname[3], "MY-TAB"));
  { const uint8_t *r = got[0]; int16_t gz = (int16_t)(r[7] | r[8] << 8), bz = (int16_t)(r[13] | r[14] << 8);
    CHECK(r[0] == CMD_AIRDBG && gz == (int16_t)(65.5 * 100) && bz == 40 && (r[19] & 1) && !(r[19] & 2));   // gz = 100 do/s, bias = 40, dang bay, da hieu chuan
    CHECK((r[20] | r[21] << 8) == 60 && (r[22] | r[23] << 8) == 60 && (r[24] | r[25] << 8) == 15 && r[26] == 2 && r[28] > 0 && r[29] == 0);
    CHECK(!(got[1][19] & 1)); }
  CHECK(core.imu_ok == 1 && core.imu_id == 0x74);
  CHECK(sim_mrep > 5); CHECK(sim_mdx < -300 && sim_mdx > -2500);   // gz+ -> sang trai, khi dang giu AIR
  CHECK(sn3[2] == 0 && sim_mb == 1);                              // sim_mb gom moi nut da tung bam
  return fails;
}
static void a_morsemap(void) { core.cfg.w[W_KM + POS_(4, 6)] = KC_MDOT; core.cfg.w[W_KM + POS_(4, 4)] = KC_MDASH; core.cfg.w[W_KM + POS_(3, 2)] = KC_MOK; core.cfg.w[W_KM + POS_(2, 6)] = KC_LATCH; }
static int scenario_buz(void) {   // coi bao: cham ngan, gach dai gap 3, OK tieng ngan; phim khac va OK luc rong thi im
  act_at(20, a_morsemap);
  tap(100, 2, 6, 20);                                  // LATCH: khong phai Morse -> im
  tap(300, 4, 6, 20); tap(700, 4, 4, 20); tap(1100, 3, 2, 20); tap(1500, 3, 2, 20); tap(1700, 1, 3, 30);   // cham, gach, OK, OK luc rong, phim thuong
  endt = 2000; run();
  uint32_t on[8], off[8]; int n = 0, m = 0;
  for (int i = 0; i < sim_buz_n; i++) { if (sim_buz_log[i] & 1) { if (n < 8) on[n++] = sim_buz_log[i] >> 1; } else if (m < 8) off[m++] = sim_buz_log[i] >> 1; }
  printf("  coi: %d lan bat", n); for (int i = 0; i < n && i < m; i++) printf(" [%u ms @ %u]", off[i] - on[i], on[i]); printf("\n");
  CHECK(n == 3 && m == 3);
  if (n == 3 && m == 3) {
    CHECK(on[0] >= 300 && on[0] < 330 + CHORD_MS && off[0] - on[0] >= BUZ_DOT_MS - 3 && off[0] - on[0] <= BUZ_DOT_MS + 3);
    CHECK(on[1] >= 700 && on[1] < 730 + CHORD_MS && off[1] - on[1] >= 3 * BUZ_DOT_MS - 3 && off[1] - on[1] <= 3 * BUZ_DOT_MS + 3);
    CHECK(on[2] >= 1100 && on[2] < 1130 && off[2] - on[2] >= BUZ_DOT_MS * 4 / 7 - 3 && off[2] - on[2] <= BUZ_DOT_MS * 4 / 7 + 3);
  }
  CHECK(!sim_buz_on);
  return fails;
}
static double pcap_hz[8]; static int pcap_on[8], pcap_lv[8], pcap_wrap[8]; static int pcap_n;
static void a_pcap(void) { int i = pcap_n++; pcap_on[i] = sim_buz_on; pcap_hz[i] = sim_buz_on ? 150e6 / sim_pwm_div / ((double)sim_pwm_wrap + 1.0) : 0; pcap_lv[i] = sim_pwm_lv; pcap_wrap[i] = sim_pwm_wrap; }
static int near_hz(double a, double b, double tol) { return (a > b ? a - b : b - a) <= tol; }
static void a_octd(void) { core.cfg.w[W_KM + 3 * NKEYS + POS_(4, 1)] = KC_OCTD; }   // layer MUSIC mac dinh khong con Oct-: gan tam vao R4C1 de thu nốt thap (C3)
static int ioc[4];
static void a_ioc(void) { static int i; ioc[i++ & 3] = sim_i2c_writes; }
static int scenario_piano(void) {   // nhac cu (layer MUSIC v20): Num giu + Enter bat MUSIC, C5 / G5 / C3 cho dung tan so PWM that; BAI SAU phat bai 1 + OLED ve lai; PLAY dung
  ev(100, 0, 3, 1); tap(450, 3, 6, 30); ev(520, 0, 3, 0);   // bat layer MUSIC
  tap(800, 1, 0, 300); tap(1300, 1, 4, 300);                // C5, G5
  act_at(1700, a_octd);
  tap(1800, 4, 1, 30); tap(1900, 4, 1, 30);                 // Oct- x 2
  tap(2200, 1, 0, 300);                                     // C5 - 24 = C3 (131 Hz: can chia xung lon moi vua wrap 16 bit)
  act_at(900, a_pcap); act_at(1150, a_pcap); act_at(1400, a_pcap); act_at(2300, a_pcap); act_at(2600, a_pcap);
  tap(3000, 4, 5, 30);                                      // BAI SAU: chua chon gi -> bai 1 (Ode to Joy), nốt dau E4 + 12 = E5 = 660 Hz
  act_at(2990, a_ioc); act_at(3100, a_pcap); act_at(3400, a_ioc); act_at(4400, a_ioc);   // so lan ghi I2C: OLED phai ve lai khi nốt doi
  tap(4500, 0, 0, 30);                                      // PLAY / STOP = dung
  act_at(4600, a_pcap);
  endt = 4800; run();
  printf("  tan so: %.1f / im / %.1f / %.1f / im Hz; bai 1: %.1f Hz, sau PLAY/STOP: %s; ghi I2C %d -> %d -> %d\n", pcap_hz[0], pcap_hz[2], pcap_hz[3], pcap_hz[5], pcap_on[6] ? "con keu" : "im", ioc[0], ioc[1], ioc[2]);
  CHECK(pcap_n == 7);
  CHECK(pcap_on[0] && near_hz(pcap_hz[0], 524, 1.5));      // C5
  CHECK(!pcap_on[1]);                                       // giua 2 nốt: im
  CHECK(pcap_on[2] && near_hz(pcap_hz[2], 784, 1.5));      // G5
  CHECK(pcap_on[3] && near_hz(pcap_hz[3], 131, 0.6));      // C3
  CHECK(!pcap_on[4]);
  CHECK(pcap_on[5] && near_hz(pcap_hz[5], 660, 1.5));      // bai 1, nốt 1 = E5 (khong bi anh huong boi Oct- vi bai nhac co quang tam rieng)
  CHECK(!pcap_on[6] && !sim_buz_on);                        // PLAY / STOP: im
  CHECK(ioc[1] > ioc[0] && ioc[2] - ioc[1] >= 8);          // OLED duoc cap nhat trong luc phat (nốt doi -> ve lai)
  CHECK(pcap_lv[0] > 0 && pcap_lv[0] <= (pcap_wrap[0] + 1) / 2 + 1 && pcap_lv[0] >= (pcap_wrap[0] + 1) / 2 - 2);   // am luong day, am sac 50%
  CHECK(pcap_wrap[3] > 0 && pcap_wrap[3] < 65536);
  CHECK(find_key(KC_NUMLOCK, 0) < 0);                       // khong lo NumLock / phim HID nao ra ngoai
  return fails;
}

static int sS[4][3]; static int sI;
static void a_s(void) { int i = sI++; sS[i][0] = sim_mrep; sS[i][1] = sim_mdx; sS[i][2] = sim_mdy; }
static void a_susp_on(void) { sim_busy = true; sim_suspended = true; }
static void a_susp_off(void) { sim_busy = false; sim_suspended = false; }
static void a_airt(void) { core.cfg.w[W_KM + POS_(1, 3)] = KC_AIRT; }
static int scenario_susp(void) {   // v15: may ngu 3 s khi AIR dang bat -> luc day con tro khong duoc nhay
  nacts = 0; nev = 0; sim_mdx = sim_mdy = sim_mrep = 0; sim_gyro[0] = sim_gyro[1] = sim_gyro[2] = 40;
  act_at(50, a_airt); act_at(1500, a_gyro_hold); tap(1700, 1, 3, 30);
  act_at(1900, a_s); act_at(2000, a_s);
  act_at(2001, a_susp_on); act_at(5000, a_s);
  act_at(5001, a_susp_off); act_at(5100, a_s);
  endt = 5200; run();
  int norm = sS[1][1] - sS[0][1], woke = sS[3][1] - sS[2][1];
  printf("  binh thuong 100 ms: dx=%d | trong luc ngu: %d bao cao | 100 ms sau khi day: dx=%d\n", norm, sS[2][0] - sS[1][0], woke);
  CHECK(norm < -100 && norm > -300);                  // chuot bay dang chay dung
  CHECK(sS[2][0] - sS[1][0] <= 1);                    // ngu: khong bao cao nao (toi da 1 cai dang bay)
  CHECK(woke < 0 && woke > -400);                     // day: chi nhu toc do binh thuong, khong xa ~5000 px
  return fails;
}
static void a_shake_on(void) { sim_shake = true; }
static void a_shake_off(void) { sim_shake = false; sim_gyro[2] = 40; }
static int scenario_cal(void) {   // v15: cam tay lac luc hieu chuan -> van xong, chuot bay chay duoc
  nacts = 0; nev = 0; sim_mdx = sim_mdy = sim_mrep = 0; sim_gyro[0] = sim_gyro[1] = sim_gyro[2] = 40;
  act_at(50, a_airt); act_at(100, a_shake_on); act_at(3000, a_shake_off); tap(3500, 1, 3, 30);
  endt = 5000; run();
  printf("  sau 5 s: dang hieu chuan = %d, bao cao chuot = %d\n", (int)air_calibrating(&air), sim_mrep);
  CHECK(!air_calibrating(&air) && sim_mrep > 5);
  return fails;
}
int main(int argc, char **argv) {
  (void)argv; int rc = 0;
  for (int s = 0; s < 8; s++) {   // moi kich ban chay trong tien trinh rieng (trang thai tinh cua main.c khoi tron lan)
    fflush(stdout); pid_t p = fork();
    if (p == 0) { printf("kich ban %d (%s)\n", s, s == 0 ? "layer, tap-hold, macro, BOOT" : s == 1 ? "man hinh cho / LED may tinh" : s == 2 ? "Raw HID + luu flash" : s == 3 ? "chuot bay (gyro gia)" : s == 4 ? "coi bao Morse" : s == 5 ? "nhac cu (layer MUSIC)" : s == 6 ? "chuot bay: may ngu / thuc day" : "chuot bay: hieu chuan bi lac"); fflush(stdout); exit(s == 6 ? scenario_susp() : s == 7 ? scenario_cal() : s == 0 ? scenario_main() : s == 1 ? scenario_saver() : s == 2 ? scenario_proto() : s == 3 ? scenario_air() : s == 4 ? scenario_buz() : scenario_piano()); }
    int st; waitpid(p, &st, 0); if (!WIFEXITED(st) || WEXITSTATUS(st)) rc = 1;
  }
  if (argc > 5) return 2;
  printf(rc ? "e2e: CO LOI\n" : "e2e: tat ca dat\n"); return rc;
}
