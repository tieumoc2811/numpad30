// Kiem thu Tap Dance, Combo, Latch, Morse (core.h) tren may tinh: gia lap may tinh nhan bao cao HID va dung lai van ban
#include <stdio.h>
#define CORE_IMPL
#include "../firmware/keymap.h"
static int fails;
#define CHECK(c) do { if (!(c)) { printf("LOI dong %d: %s\n", __LINE__, #c); fails++; } } while (0)
#define POS(r, c) ((r) * NCOL + (c))
static core_t C; static uint8_t raw[NKEYS]; static uint32_t T; static kc_t km[NLAYER][NKEYS]; static core_ext_t X;
static char scr[256]; static int sn; static kb_report_t prev; static uint8_t last_mods; static int cons_seen[8];
static void reset(void) {
  core_init(&C); memset(raw, 0, sizeof raw); T = 1000; memset(km, 0, sizeof km); memset(&X, 0, sizeof X);
  core_set_ext(&C, &X); sn = 0; scr[0] = 0; memset(&prev, 0, sizeof prev); last_mods = 0; morse_hint_on = 0;
}
static char to_ch(uint8_t mods, uint8_t k) {
  for (int ch = 32; ch < 127; ch++) { uint8_t m, kk; if (core_ascii((char)ch, &m, &kk) && kk == k && m == (mods & 0x22 ? 2 : 0)) return (char)ch; }
  return '?';
}
static void pump_one(void) {   // 1 ms: quet + (neu co) gui bao cao nhu hid_task
  T++; core_scan_update(&C, raw, T, (const kc_t (*)[NKEYS])km, macros, NMACROS);
  kb_report_t r;
  if (core_kb(&C, &r)) {
    bool busy = core_macro_active(&C);
    if (memcmp(&r, &prev, sizeof r)) {
      for (int i = 0; i < 6; i++) if (r.keys[i] && r.keys[i] != 1) { int was = 0; for (int j = 0; j < 6; j++) if (prev.keys[j] == r.keys[i]) was = 1;
        if (!was) { if (r.keys[i] == KC_BSPC) { if (sn) sn--; } else scr[sn++] = to_ch(r.mods, r.keys[i]); scr[sn] = 0; } }
      prev = r; last_mods = r.mods;
    }
    if (busy) core_macro_ack(&C);
  }
  uint16_t cn = core_cons(&C); if (cn == CN_MUTE) cons_seen[0]++;
}
static void step(int n) { while (n--) pump_one(); }
static void down(int r, int c) { raw[POS(r, c)] = 1; }
static void up(int r, int c) { raw[POS(r, c)] = 0; }
static void tapk(int r, int c, int hold, int gap) { down(r, c); step(hold); up(r, c); step(gap); }
static int has(uint8_t k) { for (int i = 0; i < 6; i++) if (prev.keys[i] == k) return 1; return 0; }

int main(void) {
  // ---- Tap Dance: tap / bam doi / giu ----
  reset(); km[0][POS(1,3)] = TD(0); km[0][POS(1,4)] = KC_X; X.td[0][0] = KC_A; X.td[0][1] = KC_LSFT; X.td[0][2] = KC_C;
  tapk(1, 3, 40, 300); CHECK(!strcmp(scr, "a"));
  tapk(1, 3, 40, 30); tapk(1, 3, 40, 300); CHECK(!strcmp(scr, "ac"));
  down(1, 3); step(TAP_TERM_MS + 30); CHECK(last_mods & 0x02); up(1, 3); step(30); CHECK(last_mods == 0 && !strcmp(scr, "ac"));
  // phim khac bam khi tap dance dang cho: tap dance chot truoc, dung thu tu
  tapk(1, 3, 40, 20); tapk(1, 4, 40, 300); CHECK(!strcmp(scr, "acax"));
  // bam doi roi bam phim khac: "c" roi "x"; tap dance 1 (chua cau hinh, rong) khong lam gi
  tapk(1, 3, 30, 20); down(1, 3); step(30); up(1, 3); tapk(1, 4, 40, 300); CHECK(!strcmp(scr, "acaxcx"));
  // ---- Tap Dance giu = MO(1): doi layer ----
  reset(); km[0][POS(1,3)] = TD(1); km[1][POS(1,4)] = KC_Z; km[0][POS(1,4)] = KC_X; X.td[1][0] = KC_A; X.td[1][1] = MO(1);
  down(1, 3); step(TAP_TERM_MS + 20); CHECK(core_top_layer(&C) == 1); tapk(1, 4, 40, 20); CHECK(!strcmp(scr, "z"));
  up(1, 3); step(20); CHECK(core_top_layer(&C) == 0);
  // ---- Tap Dance tap ra Media: phat bao cao consumer ----
  reset(); km[0][POS(1,3)] = TD(0); X.td[0][0] = CONS(CN_MUTE); memset(cons_seen, 0, sizeof cons_seen); tapk(1, 3, 40, 300); CHECK(cons_seen[0] > 5);
  // ---- Combo ----
  reset(); km[0][POS(1,3)] = KC_J; km[0][POS(1,4)] = KC_K; X.cb[0][0] = KC_J; X.cb[0][1] = KC_K; X.cb[0][2] = KC_Z;
  down(1, 3); step(10); down(1, 4); step(40); up(1, 3); up(1, 4); step(40); CHECK(!strcmp(scr, "z"));
  reset(); km[0][POS(1,3)] = 0x0D; km[0][POS(1,4)] = 0x0E; X.cb[0][0] = 0x0D; X.cb[0][1] = 0x0E; X.cb[0][2] = KC_Z;   // J,K
  tapk(1, 3, 15, 50); CHECK(!strcmp(scr, "j"));                       // tha som: phim don
  down(1, 3); step(15); CHECK(!has(0x0D)); step(30); CHECK(has(0x0D)); up(1, 3); step(30); CHECK(!strcmp(scr, "jj"));   // giu qua COMBO_TERM: nhu phim giu
  down(1, 3); step(10); down(1, 4); step(5); up(1, 4); up(1, 3); step(60); CHECK(!strcmp(scr, "jjz"));
  // phim khac khong thuoc combo bam khi dang cho: phim cho ra truoc
  km[0][POS(2,3)] = KC_X; down(1, 3); step(8); tapk(2, 3, 30, 60); up(1, 3); step(30); CHECK(!strcmp(scr, "jjzjx") || !strcmp(scr, "jjzxj") ); CHECK(!strcmp(scr, "jjzjx"));
  // ---- Latch ----
  reset(); km[0][POS(1,3)] = KC_LATCH; km[0][POS(1,4)] = KC_Q; km[0][POS(1,5)] = 0x7003;
  tapk(1, 3, 40, 20); CHECK(C.latch_wait); tapk(1, 4, 40, 20); CHECK(!C.latch_wait && has(0x14) && !strcmp(scr, "q"));
  step(100); CHECK(has(0x14));                                      // van duoc chot
  tapk(1, 3, 40, 20); CHECK(!has(0x14));                           // bam LATCH lan nua: nha
  tapk(1, 5, 40, 20); CHECK(last_mods & 0x02); tapk(1, 4, 40, 20); CHECK(!strcmp(scr, "qQ")); tapk(1, 5, 40, 20); CHECK(last_mods == 0);
  // ---- Morse ----
  reset(); km[0][POS(1,3)] = KC_MORSE_DOT; km[0][POS(1,4)] = KC_MORSE_DASH; km[0][POS(1,5)] = KC_MORSE_OK; km[0][POS(2,3)] = KC_X;
  tapk(1, 3, 20, 30); CHECK(!strcmp(scr, "e")); tapk(1, 4, 20, 30); CHECK(!strcmp(scr, "a"));   // "." -> e, ".-" -> a (xoa e, go a)
  tapk(1, 3, 20, 30); tapk(1, 3, 20, 30); CHECK(!strcmp(scr, "l"));                         // ".-.." = l
  tapk(1, 5, 20, 30); tapk(1, 3, 20, 30); CHECK(!strcmp(scr, "le"));                         // xong -> ky tu moi
  tapk(2, 3, 20, 50); CHECK(!strcmp(scr, "lex"));                                           // phim khac chot ky tu
  tapk(1, 3, 20, 30); tapk(1, 3, 20, 30); CHECK(!strcmp(scr, "lexi"));                       // ky tu moi: ".." = i
  CHECK(!strcmp(C.note, "I ..")); 
  // Morse goi y
  reset(); morse_hint_on = 1; km[0][POS(1,3)] = KC_MORSE_DOT; km[0][POS(1,4)] = KC_MORSE_DASH; km[0][POS(1,5)] = KC_MORSE_OK; km[0][POS(2,3)] = KC_X;
  tapk(1, 3, 20, 80); CHECK(!strcmp(scr, "e [.i -a]"));
  tapk(1, 4, 20, 150); CHECK(!strcmp(scr, "a [.r -w]"));
  tapk(1, 5, 20, 80); CHECK(!strcmp(scr, "a")); tapk(2, 3, 20, 80); CHECK(!strcmp(scr, "ax"));
  // Morse: ma khong co ky tu ("..--") chi xoa
  reset(); km[0][POS(1,3)] = KC_MORSE_DOT; km[0][POS(1,4)] = KC_MORSE_DASH;
  tapk(1, 3, 20, 30); tapk(1, 3, 20, 30); tapk(1, 4, 20, 30); tapk(1, 4, 20, 30); CHECK(!strcmp(scr, "")); tapk(1, 3, 20, 30); tapk(1, 3, 20, 30); CHECK(!strcmp(scr, "?"));
  // Morse tu tap dance (tap = cham, giu = gach)
  reset(); km[0][POS(1,3)] = TD(0); X.td[0][0] = KC_MORSE_DOT; X.td[0][1] = KC_MORSE_DASH;
  tapk(1, 3, 30, 300); CHECK(!strcmp(scr, "e")); down(1, 3); step(TAP_TERM_MS + 30); up(1, 3); step(60); CHECK(!strcmp(scr, "a"));
  // ---- nhan nhanh khi Morse dang xoa: thu tu dung ----
  reset(); morse_hint_on = 1; km[0][POS(1,3)] = KC_MORSE_DOT; km[0][POS(2,3)] = KC_X;
  tapk(1, 3, 20, 0); tapk(2, 3, 20, 200); CHECK(!strcmp(scr, "ex"));
  printf(fails ? "ext: CO LOI\n" : "ext: tat ca dat\n"); return fails;
}
