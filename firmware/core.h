#pragma once
// Logic ban phim thuan C (khong phu thuoc Pico SDK / TinyUSB): chong doi, layer, tap-hold, macro, tao bao cao HID.
// Kiem thu tren may tinh bang test/run_tests.sh.
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "config.h"

#define NKEYS (NROW * NCOL)
#define NLAYER 8            // so layer toi da (so layer dang dung nam trong cau hinh, mac dinh 3)
#define DEF_NLAYER 3
#define NMACROS 16          // moi o macro 256 byte: [dai][buoc...] (xem hid_proto.h)
#define MSLOT 256
#define NTD 4               // Tap Dance
#define NCB 4               // Combo
#define NLOGO 8             // so khung logo OLED (tang de khop studio Macropad)
#ifndef COMBO_MS
#define COMBO_MS 30         // hai phim combo phai bam cach nhau toi da bay nhieu ms
#endif
typedef uint16_t kc_t;

// ---- ma phim ----
// 0x0000 NO | 0x0001 TRNS (xuyen xuong layer duoi) | 0x04..0xE7 phim HID thuong (0xE0..0xE7 la modifier)
// (mods<<8)|kc : to hop modifier + phim, mods: C=1 S=2 A=4 G=8
// MO(n)/TG(n): giu / bat tat layer | LT(n,kc): tap = kc, giu = layer n | MACRO(n): go chuoi n | CONS(x): phim da phuong tien
#define KC_NO 0x0000
#define KC_TRNS 0x0001
#define C_(k) (0x0100 | (k))
#define S_(k) (0x0200 | (k))
#define A_(k) (0x0400 | (k))
#define G_(k) (0x0800 | (k))
#define MO(n) (0x4000 | (n))
#define TG(n) (0x4100 | (n))
#define MACRO(n) (0x5000 | (n))
#define LT(n, k) (0x6000 | ((n) << 8) | (k))
#define TD(n) (0x7000 | (n))          // Tap Dance n (cham / giu / cham 2 lan)
#define TK(k) (0x7800 | (k))           // Toggle giu: bam 1 lan = giu phim k (khong can giu tay), bam lan nua = nha
#define KC_LED_TOG 0x7E01            // bat tat den LED ngoai
#define KC_LED_NEXT 0x7E02
#define KC_LED_PREV 0x7E03
#define KC_BOOT 0x7F00
#define CONS(x) (0x8000 | (x))
#define IS_BASIC(k) ((k) >= 0x04 && (k) <= 0xE7)
#define IS_COMBO(k) ((k) >= 0x0100 && (k) <= 0x0FFF)
#define IS_MO(k) (((k) & 0xFF00) == 0x4000)
#define IS_TG(k) (((k) & 0xFF00) == 0x4100)
#define IS_MACRO(k) (((k) & 0xF000) == 0x5000)
#define IS_LT(k) (((k) & 0xF000) == 0x6000)
#define IS_TD(k) (((k) & 0xFF00) == 0x7000)
#define IS_TK(k) (((k) & 0xFF00) == 0x7800)
#define IS_LEDK(k) (((k) & 0xFF00) == 0x7E00)
#define IS_CONS(k) ((k) & 0x8000)

enum {
  KC_A = 0x04, KC_C = 0x06, KC_D = 0x07, KC_S = 0x16, KC_V = 0x19, KC_X = 0x1B, KC_Y = 0x1C, KC_Z = 0x1D,
  KC_ENTER = 0x28, KC_ESC = 0x29, KC_BSPC = 0x2A, KC_TAB = 0x2B, KC_SPACE = 0x2C,
  KC_F1 = 0x3A, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12,
  KC_INS = 0x49, KC_HOME = 0x4A, KC_PGUP = 0x4B, KC_DEL = 0x4C, KC_END = 0x4D, KC_PGDN = 0x4E,
  KC_RIGHT = 0x4F, KC_LEFT = 0x50, KC_DOWN = 0x51, KC_UP = 0x52,
  KC_NUMLOCK = 0x53, KC_PSLS = 0x54, KC_PAST = 0x55, KC_PMNS = 0x56, KC_PPLS = 0x57, KC_PENT = 0x58,
  KC_P1 = 0x59, KC_P2, KC_P3, KC_P4, KC_P5, KC_P6, KC_P7, KC_P8, KC_P9, KC_P0 = 0x62, KC_PDOT = 0x63,
  KC_LCTL = 0xE0, KC_LSFT = 0xE1, KC_LALT = 0xE2, KC_LGUI = 0xE3,
  // consumer (usage page 0x0C)
  CN_PLAY = 0xCD, CN_NEXT = 0xB5, CN_PREV = 0xB6, CN_MUTE = 0xE2, CN_VOLU = 0xE9, CN_VOLD = 0xEA
};

typedef struct { uint8_t mods; uint8_t keys[6]; } kb_report_t;

typedef struct {                        // cau hinh dang chay (tro vao RAM cua firmware / mang cua test)
  const kc_t (*km)[NKEYS]; int nlayer;
  const uint16_t (*td)[3];              // NTD x {cham, giu, cham 2 lan}
  const uint8_t *cb_a, *cb_b; const uint16_t *cb_r;   // NCB combo: vi tri phim A, B (0xFF = tat), ket qua
  const uint8_t (*macs)[MSLOT]; int nmacros;
} core_cfg_t;

typedef struct {
  uint8_t cnt[NKEYS], st[NKEYS];        // chong doi: bo dem, trang thai da chot
  kc_t act[NKEYS];                      // hanh dong dang giu cua phim i (0 = khong)
  uint8_t lts[NKEYS];                   // LT: 0 khong | 1 cho (chua biet tap/hold) | 2 hold | 3 da tap
  uint32_t t0[NKEYS];                   // thoi diem nhan (cho LT / TD)
  uint8_t tapcnt[NKEYS];                // LT tap: so ms con phai giu phim
  int8_t mol[NKEYS];                    // layer ma phim nay dang giu (MO / LT hold), -1 neu khong
  uint8_t tds[NKEYS], tdn[NKEYS];       // Tap Dance: 0 khong | 1 cho (da nhan) | 2 cho lan 2 (da nha) | 4 dang giu hanh dong | 5 vua cham
  uint32_t t1[NKEYS];                   // TD: thoi diem nha lan 1
  uint8_t shot[NKEYS];                  // dem nguoc ms cho phim cham nhanh (TD / combo)
  uint8_t cbp[NKEYS]; uint32_t cbt[NKEYS];   // combo: phim dang cho phim thu hai
  uint8_t toggled;                      // layer bat bang TG
  uint8_t lat[6];                       // phim dang duoc giu bang TK (toggle giu)
  bool boot_req;
  int8_t ledreq;                        // phim LED vua bam: 1 = ke, -1 = truoc, 2 = bat tat (main doc roi xoa)
  uint16_t last_kc;                     // phim vua bam (de hien OLED)
  uint32_t press_seq;                   // tang moi lan bam
  // macro: [dai][buoc...]; buoc: 1 tap kc16 | 2 nhan kc16 | 3 nha kc16 | 4 nghi ms16 | 5 go chuoi (n, ky tu...)
  const uint8_t *mac; uint8_t mlen, mtp, mph; uint16_t mpos, mwait, mdelay; bool mact; kb_report_t mh;
} core_t;

typedef struct { const char *text; } macro_t;   // macro mac dinh dang chuoi ASCII; '\n' = Enter, '\t' = Tab

void core_init(core_t *c);
void core_scan_update(core_t *c, const uint8_t *raw, uint32_t now, const core_cfg_t *f);
uint8_t core_macro_text(uint8_t *slot, const char *s);   // chuoi ASCII -> o macro, tra ve dai
bool core_kb(const core_t *c, kb_report_t *r);        // true = co bao cao can gui luc nay (macro dang cho -> false)
uint16_t core_cons(const core_t *c);
void core_macro_ack(core_t *c);                       // goi sau moi lan gui bao cao khi macro dang chay
int core_top_layer(const core_t *c);
void core_label(kc_t kc, char *o, int n);
bool core_ascii(char ch, uint8_t *mods, uint8_t *key);

static inline bool core_macro_active(const core_t *c) { return c->mact; }

#ifdef CORE_IMPL
void core_init(core_t *c) {
  memset(c, 0, sizeof *c);
  for (int i = 0; i < NKEYS; i++) c->mol[i] = -1;
}
int core_top_layer(const core_t *c) {
  uint8_t m = 1 | c->toggled;
  for (int i = 0; i < NKEYS; i++) if (c->mol[i] >= 0) m |= 1u << c->mol[i];
  for (int l = NLAYER - 1; l > 0; l--) if (m >> l & 1) return l;
  return 0;
}
static kc_t resolve(const core_t *c, int i, const core_cfg_t *f) {
  for (int l = core_top_layer(c); l >= 0; l--) { kc_t k = f->km[l][i]; if (k != KC_TRNS) return k; }
  return KC_NO;
}
uint8_t core_macro_text(uint8_t *slot, const char *s) {
  size_t n = strlen(s); if (n > 253) n = 253;
  memset(slot, 0, MSLOT);
  if (!n) return 0;
  slot[0] = (uint8_t)(n + 2); slot[1] = 5; slot[2] = (uint8_t)n; memcpy(slot + 3, s, n);
  return slot[0];
}
static void kc_set(kb_report_t *r, kc_t k, bool on) {   // them / bo 1 phim (hoac modifier, to hop) khoi bao cao
  uint8_t key;
  if (IS_COMBO(k)) { uint8_t m = (k >> 8) & 0xF; if (on) r->mods |= m; else r->mods &= ~m; key = k & 0xFF; }
  else if (IS_BASIC(k)) key = (uint8_t)k; else return;
  if (key >= 0xE0 && key <= 0xE7) { uint8_t b = 1u << (key - 0xE0); if (on) r->mods |= b; else r->mods &= ~b; return; }
  if (!IS_BASIC(key)) return;
  int j; for (j = 0; j < 6; j++) if (r->keys[j] == key) break;
  if (on) { if (j == 6) for (j = 0; j < 6; j++) if (!r->keys[j]) { r->keys[j] = key; break; } }
  else if (j < 6) r->keys[j] = 0;
}
static void m_stop(core_t *c) { c->mact = false; c->mwait = 0; memset(&c->mh, 0, sizeof c->mh); }
static void m_fix(core_t *c) {   // dua ve buoc hop le tiep theo; het / loi -> dung macro
  for (;;) {
    if (c->mpos >= c->mlen) { m_stop(c); return; }
    const uint8_t *s = c->mac + 1 + c->mpos; uint8_t op = s[0];
    if (op == 5) {
      if (c->mpos + 2 > c->mlen || c->mpos + 2 + s[1] > c->mlen) { m_stop(c); return; }
      uint8_t md, k;
      while (c->mtp < s[1] && !core_ascii((char)s[2 + c->mtp], &md, &k)) c->mtp++;
      if (c->mtp >= s[1]) { c->mpos += 2 + s[1]; c->mtp = 0; continue; }
      return;
    }
    if (op < 1 || op > 4 || c->mpos + 3 > c->mlen) { m_stop(c); return; }
    if (op == 4) { c->mdelay = s[1] | (s[2] << 8); if (!c->mdelay) { c->mpos += 3; continue; } }
    return;
  }
}
static void macro_begin(core_t *c, const core_cfg_t *f, int idx) {
  if (c->mact || idx < 0 || idx >= f->nmacros || !f->macs[idx][0]) return;
  c->mac = f->macs[idx]; c->mlen = c->mac[0]; c->mpos = 0; c->mtp = 0; c->mph = 0; c->mwait = 0; c->mdelay = 0;
  memset(&c->mh, 0, sizeof c->mh); c->mact = true; m_fix(c);
}
static void do_press(core_t *c, int i, kc_t k, uint32_t now, const core_cfg_t *f) {
  if (k == KC_NO) return;
  c->press_seq++; c->last_kc = k;
  int n = k & 0xF;
  if (IS_MO(k)) { if (n < f->nlayer) { c->act[i] = k; c->mol[i] = n; } }
  else if (IS_TG(k)) { if (n < f->nlayer) c->toggled ^= 1u << n; }
  else if (IS_MACRO(k)) macro_begin(c, f, k & 0xFFF);
  else if (IS_LT(k)) { if (((k >> 8) & 0xF) < f->nlayer) { c->act[i] = k; c->lts[i] = 1; c->t0[i] = now; } else c->act[i] = k & 0xFF; }
  else if (IS_TD(k)) { int t = k & 0xFF; if (t < NTD) { c->act[i] = k; c->tds[i] = 1; c->tdn[i] = t; c->t0[i] = now; } }
  else if (IS_TK(k)) {
    uint8_t kk = k & 0xFF; int j, fr = -1;
    if (!IS_BASIC(kk)) return;
    for (j = 0; j < 6; j++) { if (c->lat[j] == kk) break; if (!c->lat[j] && fr < 0) fr = j; }
    if (j < 6) c->lat[j] = 0; else if (fr >= 0) c->lat[fr] = kk;
  }
  else if (IS_LEDK(k)) { int v = k & 0xFF; c->ledreq = v == 1 ? 2 : v == 2 ? 1 : -1; }
  else if (k == KC_BOOT) c->boot_req = true;
  else c->act[i] = k;
}
static void td_tap(core_t *c, int i, uint32_t now, const core_cfg_t *f) {   // Tap Dance: cham 1 lan -> hanh dong ngan
  c->act[i] = 0; c->tds[i] = 5; c->shot[i] = 10;
  do_press(c, i, f->td[c->tdn[i]][0], now, f);
}
static void do_release(core_t *c, int i) {
  if (c->tds[i] == 5) return;
  if (c->lts[i] == 1) { c->lts[i] = 3; c->act[i] = c->act[i] & 0xFF; c->tapcnt[i] = 10; }   // tap
  else if (c->lts[i] == 3) { /* dang giu tap, de tapcnt xu ly */ }
  else { c->act[i] = 0; c->mol[i] = -1; c->lts[i] = 0; c->tds[i] = 0; }
}
void core_scan_update(core_t *c, const uint8_t *raw, uint32_t now, const core_cfg_t *f) {
  if (c->mwait) c->mwait--;
  if (c->mact && !c->mwait) {
    const uint8_t *s = c->mac + 1 + c->mpos;
    if (s[0] == 4 && c->mdelay && --c->mdelay == 0) { c->mpos += 3; m_fix(c); }
  }
  // 1) chong doi
  bool rel[NKEYS] = {0}, prs[NKEYS] = {0};
  for (int i = 0; i < NKEYS; i++) {
    if (!!raw[i] == c->st[i]) c->cnt[i] = 0;
    else if (++c->cnt[i] >= DEBOUNCE_MS) { c->st[i] = !!raw[i]; c->cnt[i] = 0; (c->st[i] ? prs : rel)[i] = true; }
  }
  // 2) bo dinh thoi: LT hold, TD hold / het cho lan 2, phim cham nhanh, combo het han
  for (int i = 0; i < NKEYS; i++) {
    if (c->lts[i] == 1 && now - c->t0[i] >= TAP_TERM_MS) { c->lts[i] = 2; c->mol[i] = (c->act[i] >> 8) & 0xF; }
    if (c->lts[i] == 3 && c->tapcnt[i] && --c->tapcnt[i] == 0) { c->lts[i] = 0; c->act[i] = 0; }
    if (c->tds[i] == 1 && now - c->t0[i] >= TAP_TERM_MS) { c->act[i] = 0; c->tds[i] = 4; do_press(c, i, f->td[c->tdn[i]][1], now, f); }
    if (c->tds[i] == 2 && now - c->t1[i] >= TAP_TERM_MS) td_tap(c, i, now, f);
    if (c->shot[i] && --c->shot[i] == 0) { c->act[i] = 0; c->mol[i] = -1; c->tds[i] = 0; }
    if (c->cbp[i] && now - c->cbt[i] >= COMBO_MS) { c->cbp[i] = 0; do_press(c, i, resolve(c, i, f), now, f); }
  }
  // 3) nha phim
  for (int i = 0; i < NKEYS; i++) if (rel[i]) {
    if (c->cbp[i]) {                                  // nha som khi dang cho combo -> thanh 1 lan cham
      c->cbp[i] = 0; kc_t k = resolve(c, i, f); do_press(c, i, k, now, f);
      if (IS_LT(k) || IS_TD(k)) do_release(c, i); else if (c->act[i]) c->shot[i] = 10;
    } else if (c->tds[i] == 1) {
      if (f->td[c->tdn[i]][2]) { c->tds[i] = 2; c->t1[i] = now; } else td_tap(c, i, now, f);
    } else if (c->tds[i] == 2) { /* dang cho lan 2 */ }
    else do_release(c, i);
  }
  // 4) nhan phim: phim khac bam luc LT dang cho -> chot hold ngay (permissive hold)
  for (int i = 0; i < NKEYS; i++) if (prs[i]) {
    for (int j = 0; j < NKEYS; j++) if (c->lts[j] == 1) { c->lts[j] = 2; c->mol[j] = (c->act[j] >> 8) & 0xF; }
    if (c->tds[i] == 5) { c->act[i] = 0; c->tds[i] = 0; c->shot[i] = 0; }
    if (c->tds[i] == 2) { c->act[i] = 0; c->tds[i] = 4; do_press(c, i, f->td[c->tdn[i]][2], now, f); continue; }   // cham 2 lan
    bool hit = false, cand = false;                   // combo theo vi tri phim
    for (int n = 0; n < NCB && !hit; n++) {
      uint8_t a = f->cb_a[n], b = f->cb_b[n];
      if (a >= NKEYS || b >= NKEYS || !f->cb_r[n] || (i != a && i != b)) continue;
      uint8_t p = i == a ? b : a;
      if (c->cbp[p]) { c->cbp[p] = 0; do_press(c, i, f->cb_r[n], now, f); hit = true; } else cand = true;
    }
    if (hit) continue;
    if (cand) { c->cbp[i] = 1; c->cbt[i] = now; continue; }
    do_press(c, i, resolve(c, i, f), now, f);
  }
}
bool core_kb(const core_t *c, kb_report_t *r) {
  memset(r, 0, sizeof *r);
  if (c->mact) {
    if (c->mwait) return false;
    const uint8_t *s = c->mac + 1 + c->mpos; uint8_t op = s[0];
    if (op == 4) return false;
    *r = c->mh;
    if (op == 5) { uint8_t md, k; if (c->mph == 0 && core_ascii((char)s[2 + c->mtp], &md, &k)) { r->mods |= md; kc_set(r, k, true); } }
    else { kc_t k = s[1] | (s[2] << 8); if (op == 2 || (op == 1 && c->mph == 0)) kc_set(r, k, true); else if (op == 3) kc_set(r, k, false); }
    return true;
  }
  int n = 0;
  for (int i = 0; i < NKEYS; i++) {
    kc_t k = c->act[i];
    if (!k || IS_CONS(k) || IS_MO(k)) continue;
    if (IS_LT(k)) { if (c->lts[i] != 3) continue; k &= 0xFF; }   // chi xuat khi dang o trang thai "da tap"
    if (IS_COMBO(k)) { uint8_t m = k >> 8; r->mods |= (m & 1 ? 0x01 : 0) | (m & 2 ? 0x02 : 0) | (m & 4 ? 0x04 : 0) | (m & 8 ? 0x08 : 0); k &= 0xFF; }
    if (k >= 0xE0 && k <= 0xE7) { r->mods |= 1u << (k - 0xE0); continue; }
    if (!IS_BASIC(k)) continue;
    bool dup = false; for (int j = 0; j < n; j++) if (r->keys[j] == k) dup = true;
    if (dup) continue;
    if (n < 6) r->keys[n++] = k; else { memset(r->keys, 0x01, 6); break; }   // tran: ErrorRollOver
  }
  for (int j = 0; j < 6; j++) {                       // phim dang giu bang TK
    uint8_t k = c->lat[j]; if (!k) continue;
    if (k >= 0xE0) { r->mods |= 1u << (k - 0xE0); continue; }
    bool dup = false; for (int m = 0; m < n; m++) if (r->keys[m] == k) dup = true;
    if (!dup && n < 6) r->keys[n++] = k;
  }
  return true;
}
uint16_t core_cons(const core_t *c) {
  for (int i = 0; i < NKEYS; i++) if (c->act[i] && IS_CONS(c->act[i])) return c->act[i] & 0x7FFF;
  return 0;
}
void core_macro_ack(core_t *c) {
  if (!c->mact) return;
  const uint8_t *s = c->mac + 1 + c->mpos; uint8_t op = s[0];
  if (op == 4) return;
  bool next = true;
  if (op == 1 || op == 5) { if (c->mph == 0) { c->mph = 1; next = false; } else c->mph = 0; }
  else kc_set(&c->mh, (kc_t)(s[1] | (s[2] << 8)), op == 2);
  if (next) { if (op == 5) c->mtp++; else c->mpos += 3; m_fix(c); }
  c->mwait = MACRO_GAP_MS;
  if (!c->mact) c->mwait = 0;
}
bool core_ascii(char ch, uint8_t *md, uint8_t *key) {
  static const char sh[] = "!@#$%^&*()";
  *md = 0;
  if (ch >= 'a' && ch <= 'z') { *key = 0x04 + (ch - 'a'); return true; }
  if (ch >= 'A' && ch <= 'Z') { *md = 2; *key = 0x04 + (ch - 'A'); return true; }
  if (ch >= '1' && ch <= '9') { *key = 0x1E + (ch - '1'); return true; }
  if (ch == '0') { *key = 0x27; return true; }
  for (int i = 0; i < 10; i++) if (ch == sh[i]) { *md = 2; *key = i == 9 ? 0x27 : 0x1E + i; return true; }
  switch (ch) {
    case '\n': *key = 0x28; return true; case '\t': *key = 0x2B; return true; case ' ': *key = 0x2C; return true;
    case '-': *key = 0x2D; return true; case '_': *md = 2; *key = 0x2D; return true;
    case '=': *key = 0x2E; return true; case '+': *md = 2; *key = 0x2E; return true;
    case '[': *key = 0x2F; return true; case '{': *md = 2; *key = 0x2F; return true;
    case ']': *key = 0x30; return true; case '}': *md = 2; *key = 0x30; return true;
    case '\\': *key = 0x31; return true; case '|': *md = 2; *key = 0x31; return true;
    case ';': *key = 0x33; return true; case ':': *md = 2; *key = 0x33; return true;
    case '\'': *key = 0x34; return true; case '"': *md = 2; *key = 0x34; return true;
    case '`': *key = 0x35; return true; case '~': *md = 2; *key = 0x35; return true;
    case ',': *key = 0x36; return true; case '<': *md = 2; *key = 0x36; return true;
    case '.': *key = 0x37; return true; case '>': *md = 2; *key = 0x37; return true;
    case '/': *key = 0x38; return true; case '?': *md = 2; *key = 0x38; return true;
  }
  return false;
}
// nhan ngan cho OLED (chi dung ky tu co trong font: A-Z 0-9 khoang trang ! ( ) * + , - . / : = ?)
void core_label(kc_t k, char *o, int n) {
  char t[24]; t[0] = 0;
  #define ADD(s) strncat(t, s, sizeof t - strlen(t) - 1)
  if (IS_CONS(k)) {
    switch (k & 0x7FFF) { case CN_MUTE: ADD("MUTE"); break; case CN_VOLU: ADD("VOL+"); break; case CN_VOLD: ADD("VOL-"); break;
      case CN_PLAY: ADD("PLAY"); break; case CN_NEXT: ADD("NEXT"); break; case CN_PREV: ADD("PREV"); break; default: ADD("MEDIA"); }
  } else if (IS_MACRO(k)) { ADD("MACRO "); int v = (k & 0xFFF) + 1; char d[3] = {0, 0, 0}; if (v >= 10) { d[0] = '1'; d[1] = '0' + (v - 10) % 10; } else d[0] = '0' + v; ADD(d); }
  else if (IS_TK(k)) { ADD("TOG "); char s[12]; core_label(k & 0xFF, s, sizeof s); ADD(s); }
  else if (IS_TD(k)) { ADD("TD "); char d[2] = {'1' + (k & 0x7), 0}; ADD(d); }
  else if (k == KC_LED_TOG) ADD("LED");
  else if (k == KC_LED_NEXT) ADD("LED+");
  else if (k == KC_LED_PREV) ADD("LED-");
  else if (IS_MO(k) || IS_TG(k)) { ADD("LAYER "); char d[2] = {'0' + (k & 0xF), 0}; ADD(d); }
  else if (IS_LT(k)) { core_label(k & 0xFF, o, n); return; }
  else if (k == KC_BOOT) ADD("BOOT");
  else {
    if (IS_COMBO(k)) { uint8_t m = k >> 8; if (m & 1) ADD("CTL+"); if (m & 2) ADD("SFT+"); if (m & 4) ADD("ALT+"); if (m & 8) ADD("WIN+"); k &= 0xFF; }
    char d[2] = {0, 0};
    if (k >= 0x04 && k <= 0x1D) { d[0] = 'A' + (k - 4); ADD(d); }
    else if (k >= 0x1E && k <= 0x26) { d[0] = '1' + (k - 0x1E); ADD(d); }
    else if (k == 0x27) ADD("0");
    else if (k >= 0x3A && k <= 0x45) { ADD("F"); char f[3] = {0}; int v = k - 0x39; if (v >= 10) { f[0] = '1'; f[1] = '0' + v - 10; } else f[0] = '0' + v; ADD(f); }
    else if (k >= KC_P1 && k <= KC_P9) { d[0] = '1' + (k - KC_P1); ADD(d); }
    else switch (k) {
      case KC_P0: ADD("0"); break; case KC_PDOT: ADD("."); break; case KC_PSLS: ADD("/"); break; case KC_PAST: ADD("*"); break;
      case KC_PMNS: ADD("-"); break; case KC_PPLS: ADD("+"); break; case KC_PENT: case KC_ENTER: ADD("ENTER"); break;
      case KC_NUMLOCK: ADD("NUM"); break; case KC_UP: ADD("UP"); break; case KC_DOWN: ADD("DOWN"); break;
      case KC_LEFT: ADD("LEFT"); break; case KC_RIGHT: ADD("RIGHT"); break; case KC_TAB: ADD("TAB"); break;
      case KC_ESC: ADD("ESC"); break; case KC_BSPC: ADD("BKSP"); break; case KC_SPACE: ADD("SPACE"); break;
      case KC_DEL: ADD("DEL"); break; case KC_INS: ADD("INS"); break; case KC_HOME: ADD("HOME"); break; case KC_END: ADD("END"); break;
      case KC_PGUP: ADD("PGUP"); break; case KC_PGDN: ADD("PGDN"); break; case KC_LCTL: ADD("CTRL"); break;
      case KC_LSFT: ADD("SHIFT"); break; case KC_LALT: ADD("ALT"); break; case KC_LGUI: ADD("WIN"); break;
      default: ADD("?"); }
  }
  #undef ADD
  strncpy(o, t, n - 1); o[n - 1] = 0;
}
#endif
