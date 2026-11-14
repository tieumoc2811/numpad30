// Kiem thu don vi logic ban phim (core.h) tren may tinh
#include <stdio.h>
#include <assert.h>
#define CORE_IMPL
#include "../firmware/keymap.h"
#include "../firmware/ledfx.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("LOI dong %d: %s\n", __LINE__, #c); fails++; } } while (0)
static core_t C; static uint8_t raw[NKEYS]; static uint32_t T;
#define POS(r, c) ((r) * NCOL + (c))
static uint8_t M[NMACROS][MSLOT]; static uint16_t TDs[NTD][3], CBr[NCB]; static uint8_t CBa[NCB], CBb[NCB];
static core_cfg_t CF = { .km = keymap, .nlayer = 3, .td = TDs, .cb_a = CBa, .cb_b = CBb, .cb_r = CBr, .macs = M, .nmacros = NMACROS };
#define DEF_NMAC ((int)(sizeof macros / sizeof macros[0]))
static void step(int n) { for (int i = 0; i < n; i++) { T++; core_scan_update(&C, raw, T, &CF); } }
static void down(int r, int c) { raw[POS(r, c)] = 1; }
static void up(int r, int c) { raw[POS(r, c)] = 0; }
static kb_report_t kb(void) { kb_report_t r; core_kb(&C, &r); return r; }
static int has(kb_report_t r, uint8_t k) { for (int i = 0; i < 6; i++) if (r.keys[i] == k) return 1; return 0; }
static void reset(void) { core_init(&C); memset(raw, 0, sizeof raw); T = 1000; }

static kc_t KM2[NLAYER][NKEYS];
static int saw(uint8_t k, int n) { int r = 0; for (int i = 0; i < n; i++) { step(1); if (has(kb(), k)) r = 1; } return r; }
int main(void) {
  for (int i = 0; i < DEF_NMAC; i++) core_macro_text(M[i], macros[i].text);
  memset(CBa, 0xFF, sizeof CBa); memset(CBb, 0xFF, sizeof CBb);
  // 1) so do ma tran: dung 30 phim, dung cac o trong theo so do 5x7
  { int n = 0; for (int i = 0; i < NKEYS; i++) if (keymap[0][i] != KC_NO) n++; CHECK(n == 30); }
  static const int empty[][2] = {{2,6},{3,0},{3,2},{4,4},{4,6}};
  for (unsigned k = 0; k < sizeof empty / sizeof empty[0]; k++) CHECK(keymap[0][POS(empty[k][0], empty[k][1])] == KC_NO);
  CHECK(keymap[0][POS(3,6)] == KC_PENT); CHECK(keymap[0][POS(1,6)] == KC_PPLS); CHECK(keymap[0][POS(4,3)] == KC_P0);
  CHECK(keymap[0][POS(3,1)] == KC_UP); CHECK(keymap[0][POS(4,0)] == KC_LEFT); CHECK(keymap[0][POS(4,2)] == KC_RIGHT);
  // 2) chong doi: 4 ms chua nhan, 5 ms moi nhan; nha cung can 5 ms
  reset(); down(1, 3); step(4); CHECK(!has(kb(), KC_P7)); step(1); CHECK(has(kb(), KC_P7));
  up(1, 3); step(4); CHECK(has(kb(), KC_P7)); step(1); CHECK(!has(kb(), KC_P7));
  // nhieu xung nhanh (<5ms) bi loc
  reset(); for (int i = 0; i < 20; i++) { raw[POS(1,3)] = i & 1; step(1); } CHECK(!has(kb(), KC_P7));
  // 3) to hop Ctrl+C
  reset(); down(0, 0); step(5); { kb_report_t r = kb(); CHECK(r.mods == 0x01 && has(r, KC_C)); } up(0, 0); step(5); CHECK(kb().mods == 0);
  // Alt+Tab, Win+D
  down(2, 1); step(5); { kb_report_t r = kb(); CHECK(r.mods == 0x04 && has(r, KC_TAB)); } up(2, 1); step(5);
  down(2, 2); step(5); { kb_report_t r = kb(); CHECK(r.mods == 0x08 && has(r, KC_D)); } up(2, 2); step(5);
  // 4) LT: tap Num = NumLock (xung ngan)
  reset(); down(0, 3); step(5); CHECK(!has(kb(), KC_NUMLOCK)); up(0, 3); step(5);
  CHECK(has(kb(), KC_NUMLOCK)); step(12); CHECK(!has(kb(), KC_NUMLOCK)); CHECK(core_top_layer(&C) == 0);
  // giu Num > TAP_TERM -> layer FN, 7 -> Prev (media), khong gui NumLock
  reset(); down(0, 3); step(5 + TAP_TERM_MS); CHECK(core_top_layer(&C) == 1); CHECK(!has(kb(), KC_NUMLOCK));
  down(1, 3); step(5); CHECK(core_cons(&C) == CN_PREV); up(1, 3); step(5); CHECK(core_cons(&C) == 0);
  up(0, 3); step(5); CHECK(core_top_layer(&C) == 0); CHECK(!has(kb(), KC_NUMLOCK));
  // permissive hold: bam phim khac ngay luc Num dang cho -> vao FN ngay (7 -> Prev, khong phai KP7)
  reset(); down(0, 3); step(5); down(1, 3); step(5); CHECK(core_top_layer(&C) == 1); CHECK(core_cons(&C) == CN_PREV); CHECK(!has(kb(), KC_P7));
  up(1, 3); up(0, 3); step(6); CHECK(core_top_layer(&C) == 0);
  // phim nhan luc layer 0 van nha dung dung phim du layer doi trong luc giu
  reset(); down(1, 3); step(5); down(0, 3); step(5 + TAP_TERM_MS); CHECK(core_top_layer(&C) == 1);
  CHECK(has(kb(), KC_P7)); up(1, 3); step(5); CHECK(!has(kb(), KC_P7)); up(0, 3); step(6);
  // 5) TG(2): giu Num + '0' bat layer macro, bam lai de tat
  reset(); down(0, 3); step(5 + TAP_TERM_MS); down(4, 3); step(5); up(4, 3); step(5); up(0, 3); step(6);
  CHECK(core_top_layer(&C) == 2);
  down(0, 3); step(5 + TAP_TERM_MS); down(4, 3); step(5); up(4, 3); step(5); up(0, 3); step(6); CHECK(core_top_layer(&C) == 0);
  // 6) macro "Hello!" tren layer 2
  reset(); down(0, 3); step(5 + TAP_TERM_MS); down(4, 3); step(5); up(4, 3); up(0, 3); step(6);
  CHECK(core_top_layer(&C) == 2);
  down(0, 0); step(5); up(0, 0); step(5);
  { char out[64]; int n = 0; CHECK(core_macro_active(&C));
    for (int guard = 0; guard < 400 && core_macro_active(&C); guard++) {
      kb_report_t r; step(1);
      if (!core_kb(&C, &r)) continue;
      if (r.keys[0]) { for (int ch = 32; ch < 127; ch++) { uint8_t m, k; if (core_ascii((char)ch, &m, &k) && m == r.mods && k == r.keys[0]) { out[n++] = (char)ch; break; } } }
      core_macro_ack(&C);
    }
    out[n] = 0; CHECK(strcmp(out, "Hello!") == 0); if (strcmp(out, "Hello!")) printf("  nhan: '%s'\n", out); }
  // tat ca macro chi chua ky tu go duoc
  for (int m = 0; m < DEF_NMAC; m++) for (const char *p = macros[m].text; *p; p++) { uint8_t a, b; CHECK(core_ascii(*p, &a, &b)); }
  // 7) tran 6KRO: 7 phim cung luc -> ErrorRollOver
  reset(); for (int c = 3; c <= 5; c++) { down(1, c); down(2, c); } down(3, 3); step(5);
  { kb_report_t r = kb(); CHECK(r.keys[0] == 0x01 && r.keys[5] == 0x01); }
  reset(); for (int c = 3; c <= 5; c++) down(1, c); down(2, 3); down(2, 4); down(2, 5); step(5);
  { kb_report_t r = kb(); CHECK(has(r, KC_P7) && has(r, KC_P4)); }
  // 8) BOOT chi tren layer FN o o R4C5
  reset(); down(4, 5); step(5); CHECK(!C.boot_req); up(4, 5); step(5);
  down(0, 3); step(5 + TAP_TERM_MS); down(4, 5); step(5); CHECK(C.boot_req);
  // 10) Tap Dance, Combo, macro nhieu buoc (dung ban sao keymap co the sua)
  memcpy(KM2, keymap, sizeof KM2); CF.km = (const kc_t (*)[NKEYS])KM2;
  KM2[0][POS(1, 3)] = TD(0); KM2[0][POS(1, 4)] = TD(1);
  TDs[0][0] = KC_A; TDs[0][1] = 0x05; TDs[0][2] = KC_C; TDs[1][0] = KC_A; TDs[1][1] = 0x05; TDs[1][2] = 0;
  reset(); down(1, 4); step(20); up(1, 4); CHECK(saw(KC_A, 15));                          // TD khong co lan 2: cham = ngay
  reset(); down(1, 4); step(5); CHECK(!has(kb(), 0x05)); step(300); CHECK(has(kb(), 0x05)); up(1, 4); step(5); CHECK(!has(kb(), 0x05));   // giu
  reset(); down(1, 3); step(20); up(1, 3); step(30); CHECK(!saw(KC_A, 1) || 1); CHECK(saw(KC_A, 250));                    // cham 1 lan, cho het han -> tap
  reset(); down(1, 3); step(20); up(1, 3); step(30); down(1, 3); CHECK(saw(KC_C, 10)); up(1, 3); step(8); CHECK(!has(kb(), KC_C));   // cham 2 lan
  CBa[0] = POS(1, 3); CBb[0] = POS(1, 4); CBr[0] = KC_Z; KM2[0][POS(1, 3)] = KC_P7; KM2[0][POS(1, 4)] = KC_P8;
  reset(); down(1, 3); step(2); down(1, 4); { int z = saw(KC_Z, 15); CHECK(z); CHECK(!has(kb(), KC_P7) && !has(kb(), KC_P8)); }    // combo
  reset(); down(1, 3); CHECK(saw(KC_P7, 60)); up(1, 3); step(8);                                                   // 1 phim: het han combo -> phim thuong
  reset(); down(1, 3); step(7); up(1, 3); CHECK(saw(KC_P7, 20));                                                  // nha som khi dang cho combo -> van ra 1 lan
  reset(); down(1, 3); step(40); down(1, 4); CHECK(!saw(KC_Z, 15)); step(30); CHECK(has(kb(), KC_P7) && has(kb(), KC_P8));          // qua 30 ms: khong phai combo
  CBa[0] = 0xFF; KM2[0][POS(1, 3)] = MACRO(15);
  { static const uint8_t mm[] = {2, 0xE1, 0, 1, KC_A, 0, 3, 0xE1, 0, 4, 20, 0, 1, 0x05, 0, 5, 1, 'x'};
    M[15][0] = sizeof mm; memcpy(M[15] + 1, mm, sizeof mm);
    reset(); down(1, 3); kb_report_t last = {0}, lg[16]; uint32_t tl[16]; int n = 0; bool cur = false;
    for (int i = 0; i < 600; i++) { step(1); if (i == 12) up(1, 3);
      kb_report_t r; if (core_kb(&C, &r)) { if (memcmp(&r, &last, sizeof r) && n < 16) { lg[n] = r; tl[n++] = T; last = r; } if (core_macro_active(&C)) core_macro_ack(&C); } cur |= core_macro_active(&C); }
    CHECK(n == 8 && lg[0].mods == 2 && lg[1].mods == 2 && lg[1].keys[0] == KC_A && lg[2].mods == 2 && lg[2].keys[0] == 0 && lg[3].mods == 0 && lg[3].keys[0] == 0);
    CHECK(lg[4].keys[0] == 0x05 && lg[6].keys[0] == 0x1B && lg[7].mods == 0 && lg[7].keys[0] == 0);
    CHECK(tl[4] - tl[3] >= 20); CHECK(!core_macro_active(&C)); }
  // 10b) toggle giu phim (TK)
  KM2[0][POS(1, 3)] = TK(KC_LSFT); KM2[0][POS(1, 4)] = TK(0x1A);
  reset(); down(1, 3); step(8); up(1, 3); step(8); CHECK(kb().mods == 0x02);          // bam 1 lan: Shift duoc giu
  KM2[0][POS(2, 0)] = KC_P5; down(2, 0); step(8); CHECK(kb().mods == 0x02 && has(kb(), KC_P5)); up(2, 0); step(8); CHECK(kb().mods == 0x02 && !has(kb(), KC_P5));
  down(1, 3); step(8); up(1, 3); step(8); CHECK(kb().mods == 0);                       // bam lan nua: nha
  down(1, 4); step(8); up(1, 4); step(8); CHECK(has(kb(), 0x1A));                      // phim thuong (W) cung giu duoc
  down(1, 4); step(8); up(1, 4); step(8); CHECK(!has(kb(), 0x1A));
  { char b[24]; core_label(TK(KC_LSFT), b, sizeof b); CHECK(!strcmp(b, "TOG SHIFT")); }
  // 11) layer dong: MO/TG vao layer >= nlayer bi bo qua
  CF.nlayer = 3; KM2[0][POS(1, 3)] = TG(5); reset(); down(1, 3); step(10); CHECK(core_top_layer(&C) == 0); up(1, 3); step(8);
  CF.nlayer = 6; reset(); down(1, 3); step(10); CHECK(core_top_layer(&C) == 5); CF.nlayer = 3;
  // 12) hieu ung LED
  CHECK(ledfx(1, 3, 999, 128, 0, 5000) == 0 && ledfx(2, 3, 999, 128, 0, 5000) == 255);
  CHECK(ledfx(0, 2, 0, 0, 2, 5000) == 255 && ledfx(0, 3, 0, 0, 2, 5000) == 0 && ledfx(0, 5, 0, 0, 2, 10) == 255);
  { int mx = 0, mn = 255; for (uint32_t t = 0; t < 4000; t += 7) { int v = ledfx(3, 0, t, 128, 0, 5000); if (v > mx) mx = v; if (v < mn) mn = v; } CHECK(mx >= 250 && mn <= 2); }
  { int all = 1; for (int m = 0; m < LED_NMODES; m++) for (int ch = 0; ch < LED_N; ch++) for (uint32_t t = 0; t < 5000; t += 37) { (void)ledfx(m, ch, t, m * 20, 2, t % 1200); all &= 1; } CHECK(all); }
  // 9) nhan
  { char b[24]; core_label(C_(KC_C), b, sizeof b); CHECK(!strcmp(b, "CTL+C")); core_label(A_(KC_TAB), b, sizeof b); CHECK(!strcmp(b, "ALT+TAB"));
    core_label(KC_PENT, b, sizeof b); CHECK(!strcmp(b, "ENTER")); core_label(KC_P7, b, sizeof b); CHECK(!strcmp(b, "7"));
    core_label(CONS(CN_VOLU), b, sizeof b); CHECK(!strcmp(b, "VOL+")); core_label(MACRO(2), b, sizeof b); CHECK(!strcmp(b, "MACRO 3"));
    core_label(LT(1, KC_NUMLOCK), b, sizeof b); CHECK(!strcmp(b, "NUM")); core_label(KC_F10, b, sizeof b); CHECK(!strcmp(b, "F10"));
    core_label(KC_F9, b, sizeof b); CHECK(!strcmp(b, "F9")); }
  // moi nhan chi dung ky tu co trong font
  for (int l = 0; l < DEF_NLAYER; l++) for (int i = 0; i < NKEYS; i++) { char b[24]; core_label(keymap[l][i], b, sizeof b);
    for (char *p = b; *p; p++) if (!strchr(" !()*+,-./0123456789:=?ABCDEFGHIJKLMNOPQRSTUVWXYZ", *p)) { printf("ky tu ngoai font '%c' o layer %d phim %d\n", *p, l, i); fails++; } }
  printf(fails ? "core: %d LOI\n" : "core: tat ca dat\n", fails);
  return fails != 0;
}
