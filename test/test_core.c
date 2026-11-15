// Kiem thu don vi logic (core.h, proto.h, ui.h, keymap.h) tren may tinh
#include <stdio.h>
#include <stdlib.h>
#include "../firmware/ui.h"
#include "../firmware/airmouse.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("LOI dong %d: %s\n", __LINE__, #c); fails++; } } while (0)
static core_t C; static uint8_t raw[NKEYS]; static uint32_t T;
#define POS(r, c) ((r) * NCOL + (c))
#define W(l, r, c) (C.cfg.w[W_KM + (l) * NKEYS + POS(r, c)])
// ---- khung gia HID: moi ms chay 1 lan quet; thu bao cao nhu may tinh (hang doi go phim duoc ack moi ms) ----
typedef struct { uint32_t t; kb_report_t r; } ev_t;
static ev_t evs[8192]; static int nev; static kb_report_t last_rpt; static uint16_t last_cons, cons_log[256]; static int ncons;
static char text[512]; static int ntext;
static void feed(const kb_report_t *r) {   // dung van ban nhu may tinh: phim moi nhan (khong co trong bao cao truoc) -> ky tu
  for (int i = 0; i < 6; i++) { uint8_t k = r->keys[i]; if (!k || k == 1) continue;
    bool was = false; for (int j = 0; j < 6; j++) if (last_rpt.keys[j] == k) was = true;
    if (was) continue;
    if (k == KC_BSPC) { if (ntext) ntext--; continue; }
    for (int ch = 32; ch < 127; ch++) { uint8_t m, kk; if (core_ascii((char)ch, &m, &kk) && kk == k && m == (r->mods & 2)) { text[ntext++] = (char)ch; break; } }
    if (k == KC_ENTER) text[ntext++] = '\n';
  }
}
static void step(int n) {
  for (int i = 0; i < n; i++) {
    T++; core_scan_update(&C, raw, T);
    kb_report_t r; bool got = core_kb(&C, &r);
    uint16_t cn = core_cons(&C);
    if (got && memcmp(&r, &last_rpt, sizeof r)) { feed(&r); last_rpt = r; if (nev < 8192) { evs[nev].t = T; evs[nev++].r = r; } }
    if (cn != last_cons) { last_cons = cn; if (ncons < 256) cons_log[ncons++] = cn; }
    if (got && core_out_busy(&C)) core_out_ack(&C);
  }
}
static void down(int r, int c) { raw[POS(r, c)] = 1; }
static void up(int r, int c) { raw[POS(r, c)] = 0; }
static void press(int r, int c, int hold) { down(r, c); step(hold); up(r, c); step(DEBOUNCE_MS + 1); }
static kb_report_t kb(void) { kb_report_t r; core_kb(&C, &r); return r; }
static int has(kb_report_t r, uint8_t k) { for (int i = 0; i < 6; i++) if (r.keys[i] == k) return 1; return 0; }
static void reset(void) { cfg_defaults(&C.cfg); core_reset(&C); memset(raw, 0, sizeof raw); T = 1000; nev = 0; ntext = 0; text[0] = 0; ncons = 0; last_cons = 0; memset(&last_rpt, 0, sizeof last_rpt); }
static const char *txt(void) { text[ntext] = 0; return text; }
static void drain(void) { for (int g = 0; g < 8000 && (C.qn || C.m_on); g++) step(1); step(20); }
static void setk(int l, int r, int c, kc_t k) { C.cfg.w[W_KM + l * NKEYS + POS(r, c)] = k; }
static int mac_put(int slot, int rep, int gap, const char *name, const uint8_t *st, int n) {   // nhu macroEncode() cua app
  uint8_t *s = C.cfg.mac[slot]; memset(s, 0, MSLOT); s[0] = (uint8_t)rep; s[1] = gap & 255; s[2] = gap >> 8; s[3] = (uint8_t)strlen(name); memcpy(s + 4, name, strlen(name));
  s[16] = n & 255; s[17] = n >> 8; memcpy(s + 18, st, n); return n;
}

int main(void) {
  // ===== 1) so do ma tran: 30 phim, o trong dung, mac dinh dung ma moi =====
  reset();
  { int n = 0; for (int i = 0; i < NKEYS; i++) if (C.cfg.w[W_KM + i] != KC_NO) n++; CHECK(n == 30); }
  static const int empty[][2] = {{2,6},{3,0},{3,2},{4,4},{4,6}};
  for (unsigned k = 0; k < sizeof empty / sizeof empty[0]; k++) for (int l = 0; l < NLAYER; l++) CHECK(W(l, empty[k][0], empty[k][1]) == KC_NO);
  CHECK(W(0,3,6) == KC_PENT); CHECK(W(0,1,6) == KC_PPLS); CHECK(W(0,4,3) == KC_P0); CHECK(W(0,3,1) == KC_UP);
  CHECK(W(0,0,3) == 0x2153); CHECK(W(1,4,3) == TL(2)); CHECK(W(2,0,0) == MACRO(0)); CHECK(W(1,4,5) == KC_BOOT);
  CHECK(C.cfg.w[W_NLI] == 3); CHECK(W(5,0,0) == KC_TRNS); CHECK(W(31,1,3) == KC_TRNS);
  CHECK(NW == 1266 && W_AMB == 1258 && W_BT == 1120 && W_CB == 1132 && W_NLI == 1144 && W_LDB == 1145 && W_LRB == 1154 && W_OSB == 1186 && W_SQB == 1194);

  // ===== 2) chong doi =====
  reset(); down(1, 3); step(4); CHECK(!has(kb(), KC_P7)); step(1); CHECK(has(kb(), KC_P7));
  up(1, 3); step(4); CHECK(has(kb(), KC_P7)); step(1); CHECK(!has(kb(), KC_P7));
  reset(); for (int i = 0; i < 20; i++) { raw[POS(1,3)] = i & 1; step(1); } CHECK(!has(kb(), KC_P7));

  // ===== 3) to hop modifier =====
  reset(); down(0, 0); step(5); { kb_report_t r = kb(); CHECK(r.mods == 0x01 && has(r, KC_C)); } up(0, 0); step(5); CHECK(kb().mods == 0);
  down(2, 1); step(5); { kb_report_t r = kb(); CHECK(r.mods == 0x04 && has(r, KC_TAB)); } up(2, 1); step(5);
  down(2, 2); step(5); { kb_report_t r = kb(); CHECK(r.mods == 0x08 && has(r, KC_D)); } up(2, 2); step(5);

  // ===== 4) LT: tap / hold / permissive hold =====
  reset(); down(0, 3); step(5); CHECK(!has(kb(), KC_NUMLOCK)); up(0, 3); step(5);
  CHECK(has(kb(), KC_NUMLOCK)); step(12); CHECK(!has(kb(), KC_NUMLOCK)); CHECK(core_top_layer(&C) == 0);
  reset(); down(0, 3); step(5 + TAP_TERM_MS); CHECK(core_top_layer(&C) == 1); CHECK(!has(kb(), KC_NUMLOCK));
  down(1, 3); step(5); CHECK(core_cons(&C) == CN_PREV); up(1, 3); step(5); CHECK(core_cons(&C) == 0);
  up(0, 3); step(5); CHECK(core_top_layer(&C) == 0); CHECK(!has(kb(), KC_NUMLOCK));
  reset(); down(0, 3); step(5); down(1, 3); step(5); CHECK(core_top_layer(&C) == 1); CHECK(core_cons(&C) == CN_PREV); CHECK(!has(kb(), KC_P7));
  up(1, 3); up(0, 3); step(6); CHECK(core_top_layer(&C) == 0);
  reset(); down(1, 3); step(5); down(0, 3); step(5 + TAP_TERM_MS); CHECK(core_top_layer(&C) == 1);
  CHECK(has(kb(), KC_P7)); up(1, 3); step(5); CHECK(!has(kb(), KC_P7)); up(0, 3); step(6);
  // LT khac: LT(3, A) tren R0C0 voi n den 31 ; tap = A
  reset(); setk(0, 0, 0, LT(20, KC_A)); press(0, 0, 30); CHECK(!strcmp(txt(), "") || has(evs[0].r, KC_A)); CHECK(nev >= 1 && has(evs[0].r, KC_A));

  // ===== 5) layer: TL, MO, LY+/-, TRNS roi xuong layer 0 =====
  reset(); down(0, 3); step(5 + TAP_TERM_MS); down(4, 3); step(5); up(4, 3); step(5); up(0, 3); step(6);
  CHECK(core_top_layer(&C) == 2);
  down(0, 3); step(5 + TAP_TERM_MS); down(4, 3); step(5); up(4, 3); step(5); up(0, 3); step(6); CHECK(core_top_layer(&C) == 0);
  reset(); C.cfg.w[W_NLI] = 4; setk(0, 4, 0, MO(3)); setk(3, 0, 0, KC_A); down(4, 0); step(5); CHECK(core_top_layer(&C) == 3);
  down(0, 0); step(5); CHECK(has(kb(), KC_A)); up(0, 0); step(5); up(4, 0); step(5); CHECK(core_top_layer(&C) == 0);
  C.cfg.w[W_NLI] = 3; for (int l = 0; l < 3; l++) { setk(l, 4, 0, KC_LYUP); setk(l, 4, 2, KC_LYDN); }
  press(4, 0, 5); CHECK(core_top_layer(&C) == 1); press(4, 0, 5); CHECK(core_top_layer(&C) == 2);
  press(4, 0, 5); CHECK(core_top_layer(&C) == 0); press(4, 2, 5); CHECK(core_top_layer(&C) == 2);
  // layer vuot so layer (NLI) khong co hieu luc
  reset(); C.cfg.w[W_NLI] = 1; setk(0, 4, 0, TL(2)); press(4, 0, 5); CHECK(core_top_layer(&C) == 0);
  // TL(0) tat moi layer toggle
  reset(); setk(0, 4, 0, TL(2)); setk(0, 4, 2, TL(0)); press(4, 0, 5); CHECK(core_top_layer(&C) == 2); press(4, 2, 5); CHECK(core_top_layer(&C) == 0);

  // ===== 6) tap dance =====
  reset(); setk(0, 1, 3, TD(0));   // TD0: Space / Shift / Enter
  press(1, 3, 20); step(TD_TERM_MS + 5); CHECK(nev >= 1 && has(evs[0].r, KC_SPACE));
  reset(); setk(0, 1, 3, TD(0)); down(1, 3); step(5 + TD_TERM_MS + 5); CHECK((kb().mods & 0x02) != 0); up(1, 3); step(10); CHECK(kb().mods == 0);
  reset(); setk(0, 1, 3, TD(0)); press(1, 3, 20); down(1, 3); step(20); CHECK(has(kb(), KC_ENTER)); up(1, 3); step(15);
  { int space = 0; for (int i = 0; i < nev; i++) if (has(evs[i].r, KC_SPACE)) space++; CHECK(space == 0); }
  // phim khac bam luc tap dance dang giu -> chot "giu" ngay (Shift + A)
  reset(); setk(0, 1, 3, TD(0)); down(1, 3); step(8); down(2, 3); step(8); { kb_report_t r = kb(); CHECK((r.mods & 2) && has(r, 0x5C)); }
  up(2, 3); up(1, 3); step(15);
  // phim khac bam luc tap dance da nha (cho lan 2) -> chot "cham" truoc roi moi den phim khac
  reset(); setk(0, 1, 3, TD(0)); press(1, 3, 20); down(2, 3); step(8); up(2, 3); step(10);
  { int a = -1, b = -1; for (int i = 0; i < nev; i++) { if (a < 0 && has(evs[i].r, KC_SPACE)) a = i; if (b < 0 && has(evs[i].r, 0x5C)) b = i; } CHECK(a >= 0 && b > a); }
  // o "cham 2 lan" rong: go 2 lan phim cham
  reset(); C.cfg.w[W_BT + 2] = 0; setk(0, 1, 3, TD(0)); press(1, 3, 20); down(1, 3); step(20); up(1, 3); step(30);
  { int space = 0; for (int i = 0; i < nev; i++) if (has(evs[i].r, KC_SPACE)) space++; CHECK(space >= 2); }

  // ===== 7) combo (A + B trong 30 ms) =====
  reset(); C.cfg.w[W_CB + 0] = KC_P7; C.cfg.w[W_CB + 1] = KC_P8; C.cfg.w[W_CB + 2] = KC_ESC;
  down(1, 3); step(8); down(1, 4); step(8); CHECK(has(kb(), KC_ESC)); CHECK(!has(kb(), KC_P7)); CHECK(!has(kb(), KC_P8));
  up(1, 3); step(8); CHECK(!has(kb(), KC_ESC)); up(1, 4); step(8); CHECK(kb().keys[0] == 0);
  // qua cua so 30 ms: ra tung phim rieng
  reset(); C.cfg.w[W_CB + 0] = KC_P7; C.cfg.w[W_CB + 1] = KC_P8; C.cfg.w[W_CB + 2] = KC_ESC;
  down(1, 3); step(40); CHECK(has(kb(), KC_P7)); down(1, 4); step(40); CHECK(has(kb(), KC_P8) && !has(kb(), KC_ESC));
  up(1, 3); up(1, 4); step(10);
  // bam nhanh roi nha truoc cua so (phim don) van gui duoc
  reset(); C.cfg.w[W_CB + 0] = KC_P7; C.cfg.w[W_CB + 1] = KC_P8; C.cfg.w[W_CB + 2] = KC_ESC; press(1, 3, 6); step(20);
  { int n7 = 0; for (int i = 0; i < nev; i++) if (has(evs[i].r, KC_P7)) n7++; CHECK(n7 >= 1); }
  // phim A + phim khac (khong phai B): A ra truoc, phim kia ra sau
  reset(); C.cfg.w[W_CB + 0] = KC_P7; C.cfg.w[W_CB + 1] = KC_P8; C.cfg.w[W_CB + 2] = KC_ESC;
  down(1, 3); step(6); down(2, 3); step(10); { kb_report_t r = kb(); CHECK(has(r, KC_P7) && has(r, 0x5C)); } up(1, 3); up(2, 3); step(10);

  // ===== 8) chuoi SEQ =====
  reset(); C.cfg.w[W_SQB + 0] = KC_A; C.cfg.w[W_SQB + 1] = KC_C; C.cfg.w[W_SQB + 2] = C_(KC_V);
  setk(0, 4, 0, SEQ_F + 0); setk(0, 4, 2, SEQ_B + 0); setk(0, 3, 1, SEQ_R + 0);
  press(4, 0, 6); press(4, 0, 6); press(4, 0, 6); press(4, 0, 6);   // A, C, Ctrl+V, quay lai A
  { int seq[8], n = 0; for (int i = 0; i < nev && n < 8; i++) { kb_report_t r = evs[i].r; if (has(r, KC_A) && !r.mods) seq[n++] = 0; else if (has(r, KC_C) && !r.mods) seq[n++] = 1; else if (has(r, KC_V) && r.mods == 1) seq[n++] = 2; }
    CHECK(n == 4 && seq[0] == 0 && seq[1] == 1 && seq[2] == 2 && seq[3] == 0); }
  nev = 0; press(4, 2, 6);   // lui: tu A (0) ve buoc cuoi (Ctrl+V)
  CHECK(nev >= 1 && has(evs[0].r, KC_V) && evs[0].r.mods == 1);
  nev = 0; press(3, 1, 6); press(4, 0, 6); CHECK(nev >= 1 && has(evs[0].r, KC_A));   // dat lai -> tien ra buoc dau

  // ===== 9) LATCH, TG =====
  reset(); setk(0, 4, 0, KC_LATCH); setk(0, 4, 2, KC_TGSFT);
  press(4, 0, 6); CHECK(C.latch_wait); down(1, 3); step(8); up(1, 3); step(8); CHECK(!C.latch_wait); CHECK(has(kb(), KC_P7));
  press(4, 0, 6); CHECK(!has(kb(), KC_P7));
  press(4, 2, 6); CHECK(kb().mods & 2); press(4, 2, 6); CHECK(!(kb().mods & 2));
  reset(); setk(0, 4, 0, KC_TGSPC); press(4, 0, 6); CHECK(has(kb(), KC_SPACE)); press(4, 0, 6); CHECK(!has(kb(), KC_SPACE));

  // ===== 10) Nokia multitap =====
  reset(); setk(0, 1, 3, NKK(1)); setk(0, 1, 4, NKK(9)); setk(0, 1, 5, NKK(10));
  press(1, 3, 6); step(10); press(1, 3, 6); step(10); drain(); CHECK(!strcmp(txt(), "d"));          // 2, roi bam lai nhanh -> d
  reset(); setk(0, 1, 3, NKK(1)); press(1, 3, 6); step(NOKIA_MS + 10); press(1, 3, 6); drain(); CHECK(!strcmp(txt(), "22"));   // cho > 0.6 s -> ky tu moi
  reset(); setk(0, 1, 3, NKK(1)); for (int i = 0; i < 4; i++) { press(1, 3, 6); step(30); } drain(); CHECK(!strcmp(txt(), "f"));
  reset(); setk(0, 1, 3, NKK(1)); for (int i = 0; i < 5; i++) { press(1, 3, 6); step(30); } drain(); CHECK(!strcmp(txt(), "2")); // 2 d e f -> quay lai 2
  reset(); setk(0, 1, 3, NKK(1)); for (int i = 0; i < 3; i++) { press(1, 3, 6); step(30); } drain(); CHECK(!strcmp(txt(), "e"));
  reset(); setk(0, 1, 5, NKK(10)); for (int i = 0; i < 3; i++) { press(1, 5, 6); step(30); } drain(); CHECK(!strcmp(txt(), "?"));
  reset(); setk(0, 1, 4, NKK(9)); press(1, 4, 6); step(30); press(1, 4, 6); drain(); CHECK(!strcmp(txt(), " "));

  // ===== 11) Morse =====
  reset(); setk(0, 1, 3, KC_MDOT); setk(0, 1, 4, KC_MDASH); setk(0, 1, 5, KC_MOK); setk(0, 1, 6, KC_MHINT);
  C.mo_hopt = 0;
  press(1, 3, 6); press(1, 4, 6); drain(); CHECK(!strcmp(txt(), "a"));       // .-  -> a
  press(1, 5, 6); drain(); CHECK(C.mo_len == 0);
  press(1, 4, 6); press(1, 3, 6); press(1, 3, 6); press(1, 3, 6); drain(); CHECK(!strcmp(txt(), "ab"));   // -... -> b (thay the ky tu tam)
  press(1, 5, 6);
  // goi y: bat -> " [.i -a]" hien sau "e"
  reset(); setk(0, 1, 3, KC_MDOT); setk(0, 1, 4, KC_MDASH); setk(0, 1, 5, KC_MOK); setk(0, 1, 6, KC_MHINT);
  CHECK(C.mo_hopt == 1); press(1, 3, 6); drain(); CHECK(!strcmp(txt(), "e [.i -a]"));
  press(1, 3, 6); drain(); CHECK(!strcmp(txt(), "i [.s -u]"));
  press(1, 5, 6); drain(); CHECK(!strcmp(txt(), "i"));        // chot: xoa goi y, giu ky tu
  press(1, 6, 6); drain(); CHECK(C.mo_hopt == 0);
  press(1, 3, 6); drain(); CHECK(!strcmp(txt(), "ie"));
  { char h[24]; CHECK(morse_hint("", h) > 0 && !strcmp(h, " [.e -t]")); CHECK(morse_hint(".", h) > 0 && !strcmp(h, " [.i -a]")); CHECK(morse_find("-.-.") == 'c'); CHECK(morse_find(".-.-") == 0); }
  // ma dai 7 khong nhan them
  reset(); C.mo_hopt = 0; setk(0, 1, 3, KC_MDOT); for (int i = 0; i < 9; i++) press(1, 3, 6); drain(); CHECK(C.mo_len == MORSE_MAX);

  // ===== 12) doi ten layer =====
  reset(); setk(0, 1, 3, KC_RENAME); setk(0, 1, 4, NKK(3)); setk(0, 1, 5, KC_BSPC); setk(0, 1, 6, KC_ENTER);
  press(1, 3, 6); CHECK(C.rn_on);
  press(1, 4, 6); step(20); press(1, 4, 6); step(20); press(1, 4, 6); step(20);    // 4 j k l -> K? (3 lan = k) ... 4,j,k
  CHECK(!strcmp(C.rn_buf, "K")); step(NOKIA_MS + 5); press(1, 4, 6); CHECK(!strcmp(C.rn_buf, "K4"));
  press(1, 5, 6); CHECK(!strcmp(C.rn_buf, "K")); press(1, 6, 6); CHECK(!C.rn_on && C.save_req && !strcmp(C.cfg.lname[0], "K"));
  { char b[16]; core_layer_name(&C, 0, b, sizeof b); CHECK(!strcmp(b, "K")); core_layer_name(&C, 5, b, sizeof b); CHECK(!strcmp(b, "LAYER5")); }
  C.save_req = false; press(1, 3, 6); press(1, 3, 6); CHECK(!C.rn_on && !C.save_req); CHECK(!strcmp(C.cfg.lname[0], "K"));    // RENAME roi RENAME = huy
  press(1, 3, 6); press(1, 6, 6); CHECK(C.cfg.lname[0][0] == 0);                                                      // de trong + Enter = ten mac dinh
  { char b[16]; core_layer_name(&C, 0, b, sizeof b); CHECK(!strcmp(b, "BASE")); }
  // trong luc doi ten phim khong gui ra may tinh
  reset(); setk(0, 1, 3, KC_RENAME); press(1, 3, 6); nev = 0; press(1, 4, 6); press(0, 0, 6); CHECK(nev == 0);
  // ten dai toi da NAMELEN ky tu
  reset(); setk(0, 1, 3, KC_RENAME); setk(0, 1, 4, NKK(0)); press(1, 3, 6); for (int i = 0; i < 14; i++) { press(1, 4, 6); step(NOKIA_MS + 5); } CHECK(C.rn_len == NAMELEN);

  // ===== 13) macro =====
  reset(); down(0, 3); step(5 + TAP_TERM_MS); down(4, 3); step(5); up(4, 3); up(0, 3); step(6); CHECK(core_top_layer(&C) == 2);
  press(0, 0, 6); drain(); CHECK(!strcmp(txt(), "Hello!"));
  ntext = 0; press(1, 2, 6); drain(); CHECK(!strcmp(txt(), "git status\n"));
  ntext = 0; press(2, 2, 6); drain(); CHECK(!strcmp(txt(), "git push\n"));
  // macro co buoc down / tap / up / delay / lap
  reset(); { uint8_t st[] = {2, 0xE0, 0, 1, 0x06, 0, 3, 0xE0, 0, 4, 50, 0, 5, 2, 'h', 'i'}; mac_put(3, 2, 30, "T", st, sizeof st); }
  setk(0, 4, 0, MACRO(3)); press(4, 0, 6); drain();
  { int ctl_c = 0; for (int i = 0; i < nev; i++) if (evs[i].r.mods == 1 && has(evs[i].r, KC_C)) ctl_c++; CHECK(ctl_c == 2); CHECK(!strcmp(txt(), "chichi")); CHECK(!C.m_on); CHECK(kb().mods == 0); }
  // media trong macro
  reset(); { uint8_t st[] = {1, 0xE9, 0x80}; mac_put(0, 1, 0, "V", st, sizeof st); } setk(0, 4, 0, MACRO(0)); press(4, 0, 6); drain(); { int ok = 0; for (int i = 0; i < ncons; i++) if (cons_log[i] == CN_VOLU) ok = 1; CHECK(ok); }
  // macro hong khong lam treo
  reset(); { uint8_t st[] = {9, 1, 2, 5, 200, 'a'}; mac_put(1, 1, 0, "bad", st, sizeof st); } setk(0, 4, 0, MACRO(1)); press(4, 0, 6); for (int i = 0; i < 400; i++) step(1); CHECK(!C.m_on);
  // 32 o macro
  reset(); setk(0, 4, 0, MACRO(31)); press(4, 0, 6); drain(); CHECK(!C.m_on);

  // ===== 14) 6KRO =====
  reset(); for (int c = 3; c <= 5; c++) { down(1, c); down(2, c); } down(3, 3); step(5);
  { kb_report_t r = kb(); CHECK(r.keys[0] == 0x01 && r.keys[5] == 0x01); }
  reset(); for (int c = 3; c <= 5; c++) down(1, c); down(2, 3); down(2, 4); down(2, 5); step(5);
  { kb_report_t r = kb(); CHECK(has(r, KC_P7) && has(r, KC_P4)); }

  // ===== 15) BOOT chi o layer FN =====
  reset(); down(4, 5); step(5); CHECK(!C.boot_req); up(4, 5); step(5);
  down(0, 3); step(5 + TAP_TERM_MS); down(4, 5); step(5); CHECK(C.boot_req);

  // ===== 16) giao thuc =====
  { reset(); uint8_t in[32], out[32];
    memset(in, 0, 32); in[0] = CMD_INFO; proto_cmd(&C, in, out);
    CHECK(out[0] == 5 && out[1] == 32 && (out[2] | out[3] << 8) == 1266 && out[4] == FW_VER && out[5] == 16 && out[6] == 32 && (out[7] | out[8] << 8) == 1020 && out[9] == OLED_H && out[10] == 16 && (out[11] | out[12] << 8) == OD_FSZ);
    CHECK(FW_VER >= 7);
    // doc 14 word tai offset 0 va tai offset >255 (offset cao)
    memset(in, 0, 32); in[0] = CMD_READ; in[1] = 0; in[2] = 14; proto_cmd(&C, in, out);
    for (int i = 0; i < 14; i++) CHECK((out[3 + 2 * i] | out[4 + 2 * i] << 8) == C.cfg.w[i]);
    memset(in, 0, 32); in[0] = CMD_READ; in[1] = (W_SQB + 3) & 255; in[2] = 5; in[31] = (W_SQB + 3) >> 8; C.cfg.w[W_SQB + 5] = 0x1234; proto_cmd(&C, in, out);
    CHECK(out[3 + 2 * 2] == 0x34 && out[4 + 2 * 2] == 0x12);
    // ghi word roi doc lai, hieu luc ngay
    memset(in, 0, 32); in[0] = CMD_WRITE; in[1] = (W_KM + POS(1,3)) & 255; in[2] = 2; in[31] = (W_KM + POS(1,3)) >> 8; in[3] = KC_A; in[5] = 0x2A; in[6] = 0x01;
    CHECK(proto_cmd(&C, in, out) == 0); CHECK(W(0,1,3) == KC_A && W(0,1,4) == 0x012A);
    down(1, 3); step(6); CHECK(has(kb(), KC_A)); up(1, 3); step(6);
    // ghi cuoi bang (hang rao)
    memset(in, 0, 32); in[0] = CMD_WRITE; in[1] = (NW - 1) & 255; in[2] = 1; in[31] = (NW - 1) >> 8; in[3] = 7; proto_cmd(&C, in, out); CHECK(C.cfg.w[NW - 1] == 7 && out[0] == CMD_WRITE);
    memset(in, 0, 32); in[0] = CMD_WRITE; in[1] = (NW - 1) & 255; in[2] = 2; in[31] = (NW - 1) >> 8; proto_cmd(&C, in, out); CHECK(out[0] == 0xFF);   // vuot ranh
    memset(in, 0, 32); in[0] = CMD_READ; in[2] = 15; proto_cmd(&C, in, out); CHECK(out[0] == 0xFF);
    memset(in, 0, 32); in[0] = 77; proto_cmd(&C, in, out); CHECK(out[0] == 0xFF);
    // macro: ghi 28 byte tai offset 3*1020 + 1000 (hi != 0), doc lai
    int mo = 3 * MSLOT + 1000; memset(in, 0, 32); in[0] = CMD_MWRITE; in[1] = mo & 255; in[2] = 20; in[31] = mo >> 8; for (int i = 0; i < 20; i++) in[3 + i] = (uint8_t)(i + 1);
    proto_cmd(&C, in, out); in[0] = CMD_MREAD; memset(in + 3, 0, 28); proto_cmd(&C, in, out); for (int i = 0; i < 20; i++) CHECK(out[3 + i] == i + 1);
    memset(in, 0, 32); in[0] = CMD_MWRITE; in[1] = ((NMAC * MSLOT) - 10) & 255; in[2] = 20; in[31] = ((NMAC * MSLOT) - 10) >> 8; proto_cmd(&C, in, out); CHECK(out[0] == 0xFF);
    // du lieu OLED
    memset(in, 0, 32); in[0] = CMD_ODWRITE; in[1] = (OD_START + 5 * OD_FSZ + 300) & 255; in[31] = (OD_START + 5 * OD_FSZ + 300) >> 8; in[2] = 28; for (int i = 0; i < 28; i++) in[3 + i] = (uint8_t)(0xA0 + i);
    proto_cmd(&C, in, out); in[0] = CMD_ODREAD; memset(in + 3, 0, 28); proto_cmd(&C, in, out); for (int i = 0; i < 28; i++) CHECK(out[3 + i] == 0xA0 + i);
    memset(in, 0, 32); in[0] = CMD_ODREAD; in[1] = (OD_SIZE - 10) & 255; in[2] = 20; in[31] = (OD_SIZE - 10) >> 8; proto_cmd(&C, in, out); CHECK(out[0] == 0xFF);
    // luu / xem thu
    C.dbg.g[0] = -1234; C.dbg.g[2] = 6550; C.dbg.b[1] = -7; C.dbg.px = -3; C.dbg.st = 1; C.dbg.sent = 9;
    memset(in, 0, 32); in[0] = CMD_AIRDBG; CHECK(proto_cmd(&C, in, out) == 0);
    CHECK(out[0] == 11 && (int16_t)(out[3] | out[4] << 8) == -1234 && (int16_t)(out[7] | out[8] << 8) == 6550 && (int16_t)(out[11] | out[12] << 8) == -7 && (int16_t)(out[15] | out[16] << 8) == -3 && out[19] == 1 && out[28] == 9);
    memset(in, 0, 32); in[0] = CMD_MOUSE; in[1] = 236; in[2] = 20; CHECK(proto_cmd(&C, in, out) == PROTO_MOUSE);
    memset(in, 0, 32); in[0] = CMD_LNWRITE; in[1] = 5; in[2] = 9; memcpy(in + 3, "ab_c d~e1", 9); proto_cmd(&C, in, out);
    CHECK(out[0] == CMD_LNWRITE && !strcmp(C.cfg.lname[5], "ABC DE1"));        // chu thuong -> hoa, ky tu ngoai font bi bo
    memset(in, 0, 32); in[0] = CMD_LNREAD; in[1] = 5; proto_cmd(&C, in, out); CHECK(out[0] == CMD_LNREAD && !strcmp((char *)out + 3, "ABC DE1"));
    memset(in, 0, 32); in[0] = CMD_LNREAD; in[1] = NLAYER; proto_cmd(&C, in, out); CHECK(out[0] == 0xFF);
    memset(in, 0, 32); in[0] = CMD_LNWRITE; in[1] = 5; in[2] = NAMELEN + 1; proto_cmd(&C, in, out); CHECK(out[0] == 0xFF);
    { core_t W2; memset(&W2, 0, sizeof W2); cfg_defaults(&W2.cfg); core_reset(&W2);   // cuon chuot: nac dau ngay, giu 350 ms roi lap 50 ms
      W2.mact[0] = KC_WHUP & 0xFF; int n = 0; for (uint32_t t = 1000; t < 1800; t++) n += core_wheel(&W2, t) == 1;
      CHECK(n == 1 + (1800 - 1350 + 49) / 50);
      W2.mact[0] = 0; CHECK(core_wheel(&W2, 1801) == 0); W2.mact[1] = KC_WHDN & 0xFF; CHECK(core_wheel(&W2, 1802) == -1 && core_wheel(&W2, 1803) == 0); }
    memset(in, 0, 32); in[0] = CMD_SAVE; CHECK(proto_cmd(&C, in, out) == PROTO_SAVE);
    memset(in, 0, 32); in[0] = CMD_PREVIEW; in[1] = 4; CHECK(proto_cmd(&C, in, out) == PROTO_PREVIEW && out[1] == 4);
    // ve mac dinh
    memset(in, 0, 32); in[0] = CMD_DEFAULTS; proto_cmd(&C, in, out); CHECK(W(0,1,3) == KC_P7 && C.cfg.w[NW - 1] == 0 && C.cfg.mac[3][1000] == 0 && C.cfg.od[OD_START + 5 * OD_FSZ + 300] == 0);
  }

  // ===== 17) luu flash: dong goi / CRC =====
  { static uint8_t buf[STORE_SIZE]; static cfg_t g; reset();
    C.cfg.w[100] = 0xBEEF; C.cfg.mac[7][500] = 9; strcpy(C.cfg.lname[3], "GAME");
    store_pack(&C.cfg, buf); CHECK(store_unpack(buf, &g)); CHECK(!memcmp(&g, &C.cfg, sizeof g)); CHECK(g.w[100] == 0xBEEF && !strcmp(g.lname[3], "GAME"));
    buf[sizeof(store_hdr_t) + 1000] ^= 1; CHECK(!store_unpack(buf, &g));   // sai 1 bit
    store_pack(&C.cfg, buf); buf[0] ^= 1; CHECK(!store_unpack(buf, &g));    // sai magic
    memset(buf, 0xFF, sizeof buf); CHECK(!store_unpack(buf, &g));            // flash trong
    CHECK(crc32_buf((const uint8_t *)"123456789", 9) == 0xCBF43926u);
  }

  // ===== 17a) coi bao Morse: bip khi bam Cham / Gach / OK =====
  reset(); setk(0, 1, 3, KC_MDOT); setk(0, 1, 4, KC_MDASH); setk(0, 1, 5, KC_MOK); setk(0, 1, 6, KC_LATCH);
  CHECK(!core_buz(&C));
  press(1, 3, 5); { int n = 0; reset(); setk(0, 1, 3, KC_MDOT); down(1, 3); for (int i = 0; i < 20; i++) { step(1); if (core_buz(&C)) n++; } CHECK(n > 0); }
  { int n = 0; reset(); setk(0, 1, 3, KC_MDOT); setk(0, 1, 4, KC_MDASH); down(1, 3); step(8); up(1, 3); step(300);   // cham: bip ~BUZ_DOT_MS roi im
    CHECK(!core_buz(&C)); reset(); setk(0, 1, 4, KC_MDASH); down(1, 4); step(8); for (int i = 0; i < 400; i++) { if (core_buz(&C)) n++; step(1); } CHECK(n >= BUZ_DASH_MS - 12 && n <= BUZ_DASH_MS); }
  reset(); setk(0, 1, 5, KC_MOK); press(1, 5, 5); CHECK(!core_buz(&C));                                           // OK luc rong: im
  reset(); setk(0, 1, 6, KC_LATCH); press(1, 6, 5); CHECK(!core_buz(&C));                                         // phim khac: im

  // ===== 17b) TK: bam 1 lan giu, bam lan nua nha =====
  reset(); setk(0, 4, 0, TK(KC_LSFT)); setk(0, 4, 2, TK(KC_A)); setk(0, 4, 3, TK(KC_Z)); setk(0, 0, 0, KC_P7);
  press(4, 0, 5); CHECK(kb().mods == 2 && !has(kb(), KC_A));                       // giu Shift sau khi nha phim vat ly
  step(200); CHECK(kb().mods == 2);                                                // van giu
  press(4, 2, 5); CHECK(kb().mods == 2 && has(kb(), KC_A));                        // them A
  press(0, 0, 5); { kb_report_t r = kb(); CHECK(r.mods == 2 && has(r, KC_A)); }   // phim thuong gop cung
  press(4, 0, 5); CHECK(kb().mods == 0 && has(kb(), KC_A));                        // nha Shift, A van giu
  press(4, 2, 5); CHECK(kb().mods == 0 && !has(kb(), KC_A));                       // nha A
  for (int j = 0; j < NTK; j++) CHECK(C.tk[j] == 0);
  reset(); for (int j = 0; j < 5; j++) setk(0, 0, j, TK(KC_A + j)); for (int j = 0; j < 5; j++) press(0, j, 5);   // day: toi da NTK phim
  { kb_report_t r = kb(); CHECK(has(r, KC_A) && has(r, KC_A + 3) && !has(r, KC_A + 4)); }
  core_reset(&C); CHECK(!has(kb(), KC_A));                                                                          // reset nha het
  reset(); setk(0, 0, 0, 0x7800); setk(0, 0, 1, 0x7800 | 0x01); press(0, 0, 5); press(0, 1, 5); CHECK(kb().mods == 0 && !has(kb(), 0x01));   // ma sai
  { char b[24]; core_label(&C, TK(KC_LSFT), b, sizeof b); CHECK(!strcmp(b, "TOG SHIFT")); core_label(&C, TK(KC_Z), b, sizeof b); CHECK(!strcmp(b, "TOG Z")); }

  // ===== 18) nhan OLED =====
  reset();
  { char b[24]; core_label(&C, C_(KC_C), b, sizeof b); CHECK(!strcmp(b, "CTL+C")); core_label(&C, A_(KC_TAB), b, sizeof b); CHECK(!strcmp(b, "ALT+TAB"));
    core_label(&C, KC_PENT, b, sizeof b); CHECK(!strcmp(b, "ENTER")); core_label(&C, KC_P7, b, sizeof b); CHECK(!strcmp(b, "7"));
    core_label(&C, CONS(CN_VOLU), b, sizeof b); CHECK(!strcmp(b, "VOL+")); core_label(&C, MACRO(0), b, sizeof b); CHECK(!strcmp(b, "HELLO"));
    core_label(&C, MACRO(20), b, sizeof b); CHECK(!strcmp(b, "MACRO 21")); core_label(&C, LT(1, KC_NUMLOCK), b, sizeof b); CHECK(!strcmp(b, "NUM"));
    core_label(&C, KC_F10, b, sizeof b); CHECK(!strcmp(b, "F10")); core_label(&C, TD(2), b, sizeof b); CHECK(!strcmp(b, "TD 2"));
    core_label(&C, SEQ_F + 1, b, sizeof b); CHECK(!strcmp(b, "SEQ 2+")); core_label(&C, TL(12), b, sizeof b); CHECK(!strcmp(b, "LAYER 12")); }
  { reset(); for (int l = 0; l < 3; l++) for (int i = 0; i < NKEYS; i++) { char b[24]; core_label(&C, C.cfg.w[W_KM + l * NKEYS + i], b, sizeof b);
      for (char *p = b; *p; p++) if (!strchr(UI_FONT_OK, *p)) { printf("ky tu ngoai font '%c' o layer %d phim %d\n", *p, l, i); fails++; } } }
  { char b[24]; static const kc_t all[] = {KC_LATCH, KC_TGSPC, KC_TGSFT, KC_TGCTL, KC_TGALT, KC_RENAME, KC_MDOT, KC_MDASH, KC_MOK, KC_MHINT, KC_LYUP, KC_LYDN, NKK(0), NKK(9), NKK(10), SEQ_R + 3, SEQ_B + 7, MO(31)};
    for (unsigned i = 0; i < sizeof all / sizeof all[0]; i++) { core_label(&C, all[i], b, sizeof b); for (char *p = b; *p; p++) if (!strchr(UI_FONT_OK, *p)) { printf("nhan %04X ky tu ngoai font '%c'\n", all[i], *p); fails++; } } }

  // ===== 19) man hinh (ui.h) =====
  { static ui_t U; reset(); uint32_t t = 1000;
    ui_init(&U, &C, t); CHECK(U.st == UI_GREET);
    int drawn = 0, lit = 0; uint32_t tgreet = 0;
    for (int i = 0; i < 20000 && U.st == UI_GREET; i++) { t += 5; if (ui_task(&U, &C, t, 0)) { drawn++; for (int k = 0; k < FB_SIZE; k++) if (U.fb[k]) { lit++; break; } } tgreet = t - 1000; }
    CHECK(U.st == UI_NORMAL); CHECK(drawn > 20 && lit > 10);
    { char tx[68]; ui_text_get(&C, 16, tx); CHECK(!strcmp(tx, "HELLO NUMPAD30")); uint32_t exp = ui_scroll_ms(tx, 5); CHECK(tgreet + 40 >= exp && tgreet <= exp + 40); }
    // man hinh lam viec co noi dung; layer name "NUM"
    ui_task(&U, &C, t += 50, 0); { int any = 0; for (int k = 0; k < FB_SIZE; k++) any |= U.fb[k]; CHECK(any); }
    // loi chao tat
    C.cfg.w[W_OSB + 2] = 0; ui_init(&U, &C, t); CHECK(U.st == UI_NORMAL); C.cfg.w[W_OSB + 2] = 1;
    // screensaver sau timeout; kieu 3 = tat man hinh; phim bam danh thuc
    C.cfg.w[W_OSB + 0] = 2; C.cfg.w[W_OSB + 5] = 3; ui_init(&U, &C, t); U.st = UI_NORMAL; U.idle0 = t;
    for (int i = 0; i < 1900; i++) ui_task(&U, &C, ++t, 0); CHECK(U.st == UI_NORMAL && !U.blank);
    for (int i = 0; i < 200; i++) ui_task(&U, &C, ++t, 0); CHECK(U.st == UI_SAVER && U.blank);
    C.touch++; ui_task(&U, &C, ++t, 0); CHECK(U.st == UI_NORMAL && !U.blank);
    // kieu 0 chu chay, 1 logo (khung 5 pixel dac biet), 2 xen ke
    C.cfg.od[0] = 1; memset(C.cfg.od + OD_START, 0, OD_FSZ); C.cfg.od[OD_START + 0] = 0x01; C.cfg.od[OD_START + 127 + 3 * 128] = 0x80;   // (0,0) va (127,31)
    ui_cmd(&U, 4, t); ui_task(&U, &C, ++t, 0); CHECK(U.st == UI_LOGO && U.fb[0] == 0x01 && U.fb[127 + 3 * 128] == 0x80);
    C.cfg.w[W_LDB + LDB_ROT] = 2; U.dirty = true; ui_task(&U, &C, ++t, 0); CHECK(U.fb[((OLED_H - 1 - 0) >> 3) * 128 + 127] >> ((OLED_H - 1) & 7) & 1); CHECK(U.fb[((OLED_H - 1 - 31) >> 3) * 128 + 0] >> ((OLED_H - 1 - 31) & 7) & 1);   // xoay 180: 2 diem doi xung cho nhau
    C.cfg.od[OD_START + 1] = 0x01; U.dirty = true; ui_task(&U, &C, ++t, 0); CHECK(U.fb[((OLED_H - 1) >> 3) * 128 + 126] >> ((OLED_H - 1) & 7) & 1);   // (1,0) -> (126,H-1)
    C.cfg.w[W_LDB + LDB_ROT] = 0;
    for (int mode = 0; mode < 3; mode++) { C.cfg.w[W_OSB + 5] = mode; ui_cmd(&U, 2, t); int any = 0; for (int i = 0; i < 400; i++) { ui_task(&U, &C, t += 20, 0); for (int k = 0; k < FB_SIZE; k++) any |= U.fb[k]; } CHECK(any && U.st == UI_SAVER); }
    ui_cmd(&U, 0, t); CHECK(U.st == UI_NORMAL);
    // doi ten / LATCH hien thi, huong doc khong tran bo nho
    C.rn_on = true; strcpy(C.rn_buf, "AB"); C.rn_len = 2; for (int r = 0; r < 4; r++) { C.cfg.w[W_LDB + LDB_ROT] = r; U.dirty = true; ui_task(&U, &C, t += 50, 3); }
    C.rn_on = false; C.latch_wait = true; for (int r = 0; r < 4; r++) { C.cfg.w[W_LDB + LDB_ROT] = r; U.dirty = true; ui_task(&U, &C, t += 50, 1); }
    // huong rieng tung layer > huong chung
    C.latch_wait = false; C.cfg.w[W_LDB + LDB_ROT] = 0; C.cfg.w[W_LRB + 0] = 2; U.dirty = true; ui_task(&U, &C, t += 50, 0); CHECK(U.rot == 2);
    C.cfg.w[W_LRB + 0] = 4; U.dirty = true; ui_task(&U, &C, t += 50, 0); CHECK(U.rot == 0);
  }


  // ===== 19b) man hinh Morse / Nokia =====
  { static ui_t U; reset(); uint32_t t = 5000;
    setk(0, 3, 3, KC_MDOT); setk(0, 3, 4, KC_MDASH); setk(0, 3, 5, KC_MOK); setk(0, 2, 3, NKK(1)); setk(0, 2, 4, NKK(1));
    ui_init(&U, &C, t); U.st = UI_NORMAL; C.cfg.w[W_OSB + 0] = 0; C.cfg.w[W_LDB + LDB_ROT] = 0;
    // Morse: bam cham, gach -> ma ".-" = a
    press(3, 3, 20); press(3, 4, 20); drain();
    CHECK(ui_morse_on(&C) && C.mo_len == 2);
    U.dirty = true; ui_task(&U, &C, t += 50, 0);
    int px_code = 0; for (int x = 0; x < 30; x++) for (int y = 15; y < 20; y++) if (U.fb[(y >> 3) * 128 + x] >> (y & 7) & 1) px_code++;
    #if OLED_H == 64
    CHECK(px_code == 5 * 5 + 14 * 5);
#else
    CHECK(px_code == 4 * 4 + 10 * 4);
#endif                      // 1 cham + 1 gach
    int px_big = 0; for (int x = 104; x < 120; x++) for (int y = 11; y < 32; y++) if (U.fb[(y >> 3) * 128 + x] >> (y & 7) & 1) px_big++;
    CHECK(px_big > 20);                                    // ky tu 'A' lon
    { char o[24]; ui_morse_opts(&C, o); CHECK(!strcmp(o, ". R  - W")); }   // .-. = r, .-- = w
    printf("morse .-:\n"); for (int y = 0; y < OLED_H; y++) { for (int x = 0; x < 128; x++) putchar(U.fb[(y >> 3) * 128 + x] >> (y & 7) & 1 ? '#' : '.'); putchar('\n'); }
    press(3, 5, 20); drain(); U.dirty = true; ui_task(&U, &C, t += 50, 0); CHECK(C.mo_len == 0 && ui_morse_on(&C));   // OK: ma xoa, van o man Morse
    // Nokia: bam 2 lan phim 2 -> chon 'b' (nhom "2def" -> index 1 = 'd')
    T = 20000; press(2, 3, 20); press(2, 3, 20); drain();
    CHECK(C.nk_key == 1 && C.nk_n == 1 && ui_nokia_on(&C, T));
    U.dirty = true; ui_task(&U, &C, T, 0);
    #if OLED_H == 64
    int inv = 0; for (int x = 24; x < 46; x++) for (int y = 18; y < 46; y++)
#else
    int inv = 0; for (int x = 14; x < 27; x++) for (int y = 12; y < 28; y++)
#endif
      if (U.fb[(y >> 3) * 128 + x] >> (y & 7) & 1) inv++;
    CHECK(inv > 100);                                      // o chon dao mau (hop dac)
    printf("nokia 2d:\n"); for (int y = 0; y < OLED_H; y++) { for (int x = 0; x < 128; x++) putchar(U.fb[(y >> 3) * 128 + x] >> (y & 7) & 1 ? '#' : '.'); putchar('\n'); }
    CHECK(!ui_nokia_on(&C, T + NOKIA_MS + 1));            // het gio -> ve man hinh thuong
    // man doc: chi hien nhan ky tu (khong vo)
    C.cfg.w[W_LDB + LDB_ROT] = 1; U.dirty = true; ui_task(&U, &C, T, 0); { int any = 0; for (int k = 0; k < FB_SIZE; k++) any |= U.fb[k]; CHECK(any); }
    C.cfg.w[W_LDB + LDB_ROT] = 0;
  }

  // ===== 20) mac dinh la day du va tu nhat quan =====
  reset();
  for (int m = 0; m < 9; m++) { const uint8_t *s = C.cfg.mac[m]; int len = s[16] | s[17] << 8; CHECK(len > 0 && len + 18 <= MSLOT && s[0] == 1); }
  for (int m = 9; m < NMAC; m++) CHECK(C.cfg.mac[m][16] == 0 && C.cfg.mac[m][17] == 0);
  CHECK(C.cfg.od[0] == 0 && C.cfg.w[W_OSB] == 300 && C.cfg.w[W_OSB + 6] == 120);

  // ===== 21) chuot bay =====
  reset();
  CHECK(C.cfg.w[W_AMB] == 60 && C.cfg.w[W_AMB + 2] == 15 && W(2,1,3) == KC_AIR && W(2,1,4) == KC_MB1 && W(2,1,5) == KC_MB2 && W(2,2,3) == KC_MB3);
  setk(0, 1, 3, KC_AIR); setk(0, 1, 4, KC_MB1); setk(0, 1, 5, KC_MB2); setk(0, 1, 6, KC_AIRT); setk(0, 2, 3, KC_MB3); setk(0, 2, 4, KC_AIRCAL);
  CHECK(!core_air_on(&C) && core_mouse_buttons(&C) == 0);
  down(1, 3); step(DEBOUNCE_MS + 1); CHECK(core_air_on(&C)); CHECK(!has(kb(), KC_P7)); up(1, 3); step(DEBOUNCE_MS + 1); CHECK(!core_air_on(&C));
  down(1, 4); step(DEBOUNCE_MS + 1); CHECK(core_mouse_buttons(&C) == 1); down(1, 5); step(DEBOUNCE_MS + 1); CHECK(core_mouse_buttons(&C) == 3);
  down(2, 3); step(DEBOUNCE_MS + 1); CHECK(core_mouse_buttons(&C) == 7); up(1, 4); up(1, 5); up(2, 3); step(DEBOUNCE_MS + 1); CHECK(core_mouse_buttons(&C) == 0);
  press(1, 6, 20); CHECK(core_air_on(&C)); press(1, 6, 20); CHECK(!core_air_on(&C));
  press(2, 4, 20); CHECK(C.cal_req);
  C.cfg.w[W_AMB + 4] = 8; CHECK(core_air_on(&C)); C.cfg.w[W_AMB + 4] = 0;
  { char b[24]; core_label(&C, KC_AIR, b, sizeof b); CHECK(!strcmp(b, "AIR")); core_label(&C, KC_MB2, b, sizeof b); CHECK(!strcmp(b, "RMB")); }
  { // thuat toan: hieu chuan -> quay yaw duong -> con tro dich; dung yen -> khong troi; vung chet; che do khong bay
    air_t a; air_init(&a); uint16_t am[8] = {60, 60, 15, 0, 0, 0, 0, 0}; int dx, dy, sx = 0, sy = 0;
    for (int i = 0; i < AIR_CAL_N; i++) air_update(&a, am, 30, -20, 10, 5, true, &dx, &dy);
    CHECK(!air_calibrating(&a) && (int)a.bx == 30 && (int)a.by == -20 && (int)a.bz == 10);
    for (int i = 0; i < 200; i++) { air_update(&a, am, 30, -20, 10, 5, true, &dx, &dy); sx += dx; sy += dy; } CHECK(sx == 0 && sy == 0);
    for (int i = 0; i < 200; i++) { air_update(&a, am, 30, -20, (int16_t)(10 + 65.5 * 50), 5, true, &dx, &dy); sx += dx; sy += dy; }   // 50 do/s trong 1 giay
    CHECK(sx < -700 && sx > -900 && sy == 0);                      // quay nguoc kim dong ho (gz+) -> sang trai
    sx = sy = 0; for (int i = 0; i < 200; i++) { air_update(&a, am, (int16_t)(30 + 65.5 * 50), -20, 10, 5, true, &dx, &dy); sx += dx; sy += dy; } CHECK(sy < -400 && sx == 0);
    am[4] = 1; sx = 0; for (int i = 0; i < 200; i++) { air_update(&a, am, 30, -20, (int16_t)(10 + 65.5 * 50), 5, true, &dx, &dy); sx += dx; } CHECK(sx > 400); am[4] = 0;   // dao X
    sx = 0; for (int i = 0; i < 200; i++) { air_update(&a, am, 30, -20, (int16_t)(10 + 65.5 * 50), 5, false, &dx, &dy); sx += dx | dy; } CHECK(sx == 0);   // khong bay
    sx = 0; for (int i = 0; i < 200; i++) { air_update(&a, am, 30, -20, (int16_t)(10 + 65.5 * 1), 5, true, &dx, &dy); sx += dx; } CHECK(sx == 0);        // duoi vung chet
    air_recal(&a); CHECK(air_calibrating(&a));
  }
  { // luu flash: ban cu (khong co khoi chuot bay) van nap duoc, khoi chuot bay = mac dinh
    static uint8_t buf[STORE_SIZE]; static cfg_t g, o; cfg_defaults(&g); g.w[W_KM + 5] = KC_Z; g.mac[2][3] = 9;
    store_pack(&g, buf); CHECK(store_unpack(buf, &o) && o.w[W_KM + 5] == KC_Z);
    size_t old_w = (size_t)(NW - W_NAMB) * 2, rest = sizeof(cfg_t) - (size_t)NW * 2; static uint8_t oldb[STORE_SIZE]; memset(oldb, 0xFF, sizeof oldb);
    store_hdr_t h = {STORE_MAGIC, STORE_VER, (uint32_t)(old_w + rest), 0}; uint8_t *p = oldb + sizeof h;
    memcpy(p, g.w, old_w); memcpy(p + old_w, (uint8_t *)&g + (size_t)NW * 2, rest); h.crc = crc32_buf(p, old_w + rest); memcpy(oldb, &h, sizeof h);
    memset(&o, 0x55, sizeof o); CHECK(store_unpack(oldb, &o) && o.w[W_KM + 5] == KC_Z && o.mac[2][3] == 9 && o.w[W_AMB] == 60 && o.w[W_AMB + 7] == 0 && !strcmp(o.lname[2], "MACRO"));
    p[10] ^= 1; CHECK(!store_unpack(oldb, &o));
  }

  printf(fails ? "core: %d LOI\n" : "core: tat ca dat\n", fails);
  return fails != 0;
}
