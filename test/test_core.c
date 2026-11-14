// Kiem thu don vi logic ban phim (core.h) tren may tinh
#include <stdio.h>
#include <assert.h>
#define CORE_IMPL
#include "../firmware/keymap.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("LOI dong %d: %s\n", __LINE__, #c); fails++; } } while (0)
static core_t C; static uint8_t raw[NKEYS]; static uint32_t T;
#define POS(r, c) ((r) * NCOL + (c))
static void step(int n) { for (int i = 0; i < n; i++) { T++; core_scan_update(&C, raw, T, keymap, macros, NMACROS); } }
static void down(int r, int c) { raw[POS(r, c)] = 1; }
static void up(int r, int c) { raw[POS(r, c)] = 0; }
static kb_report_t kb(void) { kb_report_t r; core_kb(&C, &r); return r; }
static int has(kb_report_t r, uint8_t k) { for (int i = 0; i < 6; i++) if (r.keys[i] == k) return 1; return 0; }
static void reset(void) { core_init(&C); memset(raw, 0, sizeof raw); T = 1000; }

int main(void) {
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
  for (int m = 0; m < NMACROS; m++) for (const char *p = macros[m].text; *p; p++) { uint8_t a, b; CHECK(core_ascii(*p, &a, &b)); }
  // 7) tran 6KRO: 7 phim cung luc -> ErrorRollOver
  reset(); for (int c = 3; c <= 5; c++) { down(1, c); down(2, c); } down(3, 3); step(5);
  { kb_report_t r = kb(); CHECK(r.keys[0] == 0x01 && r.keys[5] == 0x01); }
  reset(); for (int c = 3; c <= 5; c++) down(1, c); down(2, 3); down(2, 4); down(2, 5); step(5);
  { kb_report_t r = kb(); CHECK(has(r, KC_P7) && has(r, KC_P4)); }
  // 8) BOOT chi tren layer FN o o R4C5
  reset(); down(4, 5); step(5); CHECK(!C.boot_req); up(4, 5); step(5);
  down(0, 3); step(5 + TAP_TERM_MS); down(4, 5); step(5); CHECK(C.boot_req);
  // 9) nhan
  { char b[24]; core_label(C_(KC_C), b, sizeof b); CHECK(!strcmp(b, "CTL+C")); core_label(A_(KC_TAB), b, sizeof b); CHECK(!strcmp(b, "ALT+TAB"));
    core_label(KC_PENT, b, sizeof b); CHECK(!strcmp(b, "ENTER")); core_label(KC_P7, b, sizeof b); CHECK(!strcmp(b, "7"));
    core_label(CONS(CN_VOLU), b, sizeof b); CHECK(!strcmp(b, "VOL+")); core_label(MACRO(2), b, sizeof b); CHECK(!strcmp(b, "MACRO 3"));
    core_label(LT(1, KC_NUMLOCK), b, sizeof b); CHECK(!strcmp(b, "NUM")); core_label(KC_F10, b, sizeof b); CHECK(!strcmp(b, "F10"));
    core_label(KC_F9, b, sizeof b); CHECK(!strcmp(b, "F9")); }
  // moi nhan chi dung ky tu co trong font
  for (int l = 0; l < NLAYER_DEF; l++) for (int i = 0; i < NKEYS; i++) { char b[24]; core_label(keymap[l][i], b, sizeof b);
    for (char *p = b; *p; p++) if (!strchr(" !()*+,-./0123456789:=?ABCDEFGHIJKLMNOPQRSTUVWXYZ", *p)) { printf("ky tu ngoai font '%c' o layer %d phim %d\n", *p, l, i); fails++; } }
  // 10) toi da 16 layer: MO / TG / LT toi layer 12 va 15, nhan 2 chu so, giam so layer thi layer vuot bi bo qua
  { static kc_t km[NLAYER][NKEYS]; for (int l = 0; l < NLAYER; l++) for (int i = 0; i < NKEYS; i++) km[l][i] = KC_TRNS;
    CHECK(NLAYER == 16);
    for (int i = 0; i < NKEYS; i++) km[0][i] = KC_NO;
    km[0][0] = MO(12); km[0][1] = TG(15); km[0][2] = LT(14, KC_SPACE); km[0][3] = KC_P7; km[12][3] = KC_P8; km[15][3] = KC_P9; km[14][3] = KC_P5;
    core_init(&C); memset(raw, 0, sizeof raw); T = 1000; core_set_layers(&C, 16); CHECK(C.nl == 16);
    raw[0] = 1; for (int i = 0; i < 6; i++) { T++; core_scan_update(&C, raw, T, km, macros, NMACROS); } CHECK(core_top_layer(&C) == 12);
    raw[3] = 1; for (int i = 0; i < 6; i++) { T++; core_scan_update(&C, raw, T, km, macros, NMACROS); } { kb_report_t r = kb(); CHECK(has(r, KC_P8)); }
    raw[3] = 0; raw[0] = 0; for (int i = 0; i < 6; i++) { T++; core_scan_update(&C, raw, T, km, macros, NMACROS); } CHECK(core_top_layer(&C) == 0);
    raw[1] = 1; for (int i = 0; i < 6; i++) { T++; core_scan_update(&C, raw, T, km, macros, NMACROS); } raw[1] = 0; for (int i = 0; i < 6; i++) { T++; core_scan_update(&C, raw, T, km, macros, NMACROS); }
    CHECK(core_top_layer(&C) == 15);
    core_set_layers(&C, 3); CHECK(core_top_layer(&C) == 0 && C.toggled == 0);      // xoa layer: TG(15) khong con tac dung
    core_set_layers(&C, 16); CHECK(core_top_layer(&C) == 0);                         // them lai: khong tu bat
    raw[2] = 1; for (int i = 0; i < 6 + TAP_TERM_MS; i++) { T++; core_scan_update(&C, raw, T, km, macros, NMACROS); } CHECK(core_top_layer(&C) == 14);
    core_set_layers(&C, 0); CHECK(C.nl == 1 && core_top_layer(&C) == 0); core_set_layers(&C, 99); CHECK(C.nl == 16);
    char b[24]; core_label(MO(12), b, sizeof b); CHECK(!strcmp(b, "LAYER 12")); core_label(TG(5), b, sizeof b); CHECK(!strcmp(b, "LAYER 5"));
    core_label(MO(15), b, sizeof b); CHECK(!strcmp(b, "LAYER 15")); core_label(MO(10), b, sizeof b); CHECK(!strcmp(b, "LAYER 10")); }
  printf(fails ? "core: %d LOI\n" : "core: tat ca dat\n", fails);
  return fails != 0;
}
