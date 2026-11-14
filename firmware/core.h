#pragma once
// Logic ban phim thuan C (khong phu thuoc Pico SDK / TinyUSB): chong doi, layer, tap-hold, macro, tao bao cao HID.
// Kiem thu tren may tinh bang test/run_tests.sh.
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "config.h"
#include "morse.h"

#define NKEYS (NROW * NCOL)
#define NLAYER 3
#define NTD 8                                // so tap dance
#define NCOMBO 4                             // so combo
#define QN 64                                // hang doi phim go (Morse co goi y can toi ~26 phim)
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
#define TD(n) (0x4200 | (n))                 // tap dance n (0..NTD-1): tap / giu / bam doi, cau hinh trong core_ext_t
#define KC_LATCH 0x7001                      // chot phim ke tiep (bam lan nua de nha); 0x7002..0x7005 = giu Space/Shift/Ctrl/Alt
#define KC_MORSE_DOT 0x7060                  // Morse: cham
#define KC_MORSE_DASH 0x7061                 // Morse: gach
#define KC_MORSE_OK 0x7062                   // Morse: xong ky tu
#define KC_MORSE_HINT 0x7063                 // Morse: bat/tat goi y
#define KC_BOOT 0x7F00
#define CONS(x) (0x8000 | (x))
#define IS_BASIC(k) ((k) >= 0x04 && (k) <= 0xE7)
#define IS_COMBO(k) ((k) >= 0x0100 && (k) <= 0x0FFF)
#define IS_MO(k) (((k) & 0xFF00) == 0x4000)
#define IS_TG(k) (((k) & 0xFF00) == 0x4100)
#define IS_TD(k) (((k) & 0xFF00) == 0x4200)
#define IS_LATCH(k) ((k) >= KC_LATCH && (k) <= KC_LATCH + 4)
#define IS_MORSE(k) ((k) >= KC_MORSE_DOT && (k) <= KC_MORSE_HINT)
#define IS_MACRO(k) (((k) & 0xF000) == 0x5000)
#define IS_LT(k) (((k) & 0xF000) == 0x6000)
#define IS_CONS(k) ((k) & 0x8000)

enum {
  KC_A = 0x04, KC_C = 0x06, KC_D = 0x07, KC_J = 0x0D, KC_K = 0x0E, KC_Q = 0x14, KC_S = 0x16, KC_V = 0x19, KC_X = 0x1B, KC_Y = 0x1C, KC_Z = 0x1D,
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

typedef struct { const char *text; } macro_t;   // chuoi ASCII; '\n' = Enter, '\t' = Tab

// cau hinh tap dance / combo (do firmware nap tu flash, core chi doc)
typedef struct {
  kc_t td[NTD][3];                      // [n] = {tap, giu, bam doi}
  kc_t cb[NCOMBO][3];                   // [n] = {phim A, phim B, ket qua}; A = 0: bo trong
} core_ext_t;

typedef struct {
  uint8_t cnt[NKEYS], st[NKEYS];        // chong doi: bo dem, trang thai da chot
  kc_t act[NKEYS];                      // hanh dong dang giu cua phim i (0 = khong)
  uint8_t lts[NKEYS];                   // LT: 0 khong | 1 cho (chua biet tap/hold) | 2 hold | 3 da tap
  uint32_t t0[NKEYS];                   // thoi diem nhan (cho LT)
  uint8_t tapcnt[NKEYS];                // LT tap: so ms con phai giu phim
  int8_t mol[NKEYS];                    // layer ma phim nay dang giu (MO / LT hold), -1 neu khong
  uint8_t toggled;                      // layer bat bang TG
  bool boot_req;
  uint16_t last_kc;                     // phim vua bam (de hien OLED)
  uint32_t press_seq;                   // tang moi lan bam
  // mo rong: tap dance / combo / chot / Morse / hang doi phim go
  const core_ext_t *x; const macro_t *mac; int nmac;
  kc_t q[QN]; uint8_t qh, qt, qph;      // hang doi phim go (moi phan tu: nhan roi nha); qph 0 = chua gui nhan, 1 = cho nha
  uint16_t tcons; uint8_t tcons_ms;     // phim media bam nhanh (tap dance / combo)
  uint8_t tds, tdp, tdn, tdc; uint32_t tdt;   // tap dance: 0 nghi, 1 dang giu (tdc = lan bam), 2 da xu ly giu, 3 cho bam lan 2
  int8_t pend; kc_t pkc; uint32_t pt;   // combo: phim dang cho ghep cap
  bool latch_wait; kc_t lat; uint8_t tg;  // chot: dang cho phim / phim da chot / bit toggle Space,Shift,Ctrl,Alt
  morse_t mrs;
  char note[12];                        // chu hien OLED thay cho nhan phim (Morse, chot...); rong = dung last_kc
  // macro
  const char *mtxt; int mpos; uint8_t mphase; uint8_t mwait; bool mact;
} core_t;


void core_init(core_t *c);
void core_scan_update(core_t *c, const uint8_t *raw, uint32_t now, const kc_t (*km)[NKEYS], const macro_t *macros, int nmacros);
bool core_kb(const core_t *c, kb_report_t *r);        // true = co bao cao can gui luc nay (macro dang cho -> false)
uint16_t core_cons(const core_t *c);
void core_macro_ack(core_t *c);                       // goi sau moi lan gui bao cao khi core_macro_active() la true
void core_set_ext(core_t *c, const core_ext_t *x);    // nap tap dance / combo (NULL = khong co)
int core_top_layer(const core_t *c);
void core_label(kc_t kc, char *o, int n);
bool core_ascii(char ch, uint8_t *mods, uint8_t *key);

static inline bool core_macro_active(const core_t *c) { return c->mact || c->qh != c->qt; }   // dang go macro / hang doi phim: sau moi bao cao gui di phai goi core_macro_ack

#ifdef CORE_IMPL
void core_init(core_t *c) {
  memset(c, 0, sizeof *c);
  for (int i = 0; i < NKEYS; i++) c->mol[i] = -1;
  c->pend = -1;
  morse_hint_on = MORSE_HINT_DEF;
}
void core_set_ext(core_t *c, const core_ext_t *x) { c->x = x; }
int core_top_layer(const core_t *c) {
  uint8_t m = 1 | c->toggled;
  for (int i = 0; i < NKEYS; i++) if (c->mol[i] >= 0) m |= 1u << c->mol[i];
  for (int l = NLAYER - 1; l > 0; l--) if (m >> l & 1) return l;
  return 0;
}
static kc_t resolve(const core_t *c, int i, const kc_t (*km)[NKEYS]) {
  for (int l = core_top_layer(c); l >= 0; l--) { kc_t k = km[l][i]; if (k != KC_TRNS) return k; }
  return KC_NO;
}
static void macro_skip(core_t *c) {   // bo qua ky tu khong go duoc; het chuoi thi dung
  uint8_t md, k;
  while (c->mtxt[c->mpos] && !core_ascii(c->mtxt[c->mpos], &md, &k)) c->mpos++;
  if (!c->mtxt[c->mpos]) c->mact = false;
}
static void macro_begin(core_t *c, const macro_t *m, int n, int idx) {
  if (c->mact || idx < 0 || idx >= n || !m[idx].text) return;
  c->mtxt = m[idx].text; c->mpos = 0; c->mphase = 0; c->mwait = 0; c->mact = true;
  macro_skip(c);
}
// ---- hien thi / hang doi phim go ----
static void show(core_t *c, kc_t k) { c->press_seq++; c->last_kc = k; c->note[0] = 0; }
static void note(core_t *c, const char *t) { strncpy(c->note, t, sizeof c->note - 1); c->note[sizeof c->note - 1] = 0; c->last_kc = 0; c->press_seq++; }
static void qpush_raw(core_t *c, kc_t k) { if ((uint8_t)(c->qh - c->qt) < QN) c->q[c->qh++ & (QN - 1)] = k; }
static int qn(const core_t *c) { return (uint8_t)(c->qh - c->qt); }
// ---- Morse (logic o morse.h; ket noi vao hang doi phim) ----
static core_t *mq_c;
static void mq_bs(void) { qpush_raw(mq_c, KC_BSPC); }
static void mq_put(char ch) { uint8_t md, k; if (core_ascii(ch, &md, &k)) qpush_raw(mq_c, (kc_t)((md << 8) | k)); }
static uint8_t mq_room(void) { return (uint8_t)(QN - qn(mq_c)); }
static const morse_io MQIO = { mq_bs, mq_put, mq_room };
static void mcommit(core_t *c) { mq_c = c; morse_commit_io(&c->mrs, &MQIO); }
static void morse_key(core_t *c, kc_t k) {
  mq_c = c;
  if (k == KC_MORSE_HINT) { uint8_t on = !morse_hint_on; if (morse_set_hint(&c->mrs, on, &MQIO)) note(c, on ? "HINT ON" : "HINT OFF"); else note(c, "FULL"); return; }
  if (k == KC_MORSE_OK) { mcommit(c); note(c, "OK"); return; }
  if (!morse_press(&c->mrs, k == KC_MORSE_DASH, &MQIO)) { note(c, "FULL"); return; }
  char s[12]; int n = 0; char ch = c->mrs.shown ? morse_find(c->mrs.code) : 0;
  if (ch && c->mrs.len <= 5) { s[n++] = ch >= 'a' && ch <= 'z' ? ch - 32 : ch; s[n++] = ' '; }
  for (int i = 0; i < c->mrs.len && n < 8; i++) s[n++] = c->mrs.code[i];
  s[n] = 0; note(c, s);
}
// ---- chot phim ----
static void latch_key(core_t *c, kc_t k) {
  static const char *const nm[4] = {"SPACE", "SHIFT", "CTRL", "ALT"};
  if (k == KC_LATCH) { if (c->lat) { c->lat = 0; note(c, "LATCH OFF"); } else { c->latch_wait = !c->latch_wait; note(c, c->latch_wait ? "LATCH ?" : "LATCH OFF"); } return; }
  int b = k - KC_LATCH - 1; c->tg ^= 1u << b;
  char s[12]; strcpy(s, nm[b]); strcat(s, c->tg >> b & 1 ? " ON" : " OFF"); note(c, s);
}
// ---- ket qua tong hop (tap dance tap/doi, combo): khong gan voi phim vat ly nao ----
static void emit(core_t *c, kc_t k) {
  if (!k || k == KC_TRNS) return;
  if (IS_MORSE(k)) { if (!c->latch_wait) morse_key(c, k); return; }
  if (IS_LATCH(k)) { latch_key(c, k); return; }
  if (IS_MACRO(k)) { show(c, k); if (!c->latch_wait) macro_begin(c, c->mac, c->nmac, k & 0xFFF); return; }
  if (IS_TG(k)) { show(c, k); c->toggled ^= 1u << (k & 0xF); return; }
  if (k == KC_BOOT) { c->boot_req = true; return; }
  if (IS_CONS(k)) { mcommit(c); show(c, k); c->tcons = k & 0x7FFF; c->tcons_ms = 25; return; }
  if (IS_BASIC(k) || IS_COMBO(k)) {
    mcommit(c); show(c, k);
    if (c->latch_wait) { c->lat = k; c->latch_wait = false; return; }
    qpush_raw(c, k);
  }
}
// phim vat ly i vua bam voi hanh dong k (da qua tap dance / combo)
static void fire(core_t *c, int i, kc_t k, uint32_t now) {
  if (!k || k == KC_TRNS) return;
  if (IS_MORSE(k)) { if (!c->latch_wait) morse_key(c, k); return; }
  if (!IS_TD(k)) mcommit(c);
  if (IS_LATCH(k)) { latch_key(c, k); return; }
  show(c, k);
  if (IS_TD(k)) {
    unsigned n = k & 0xFF;
    if (!c->x || n >= NTD) return;
    if (c->tds == 0) { c->tds = 1; c->tdp = i; c->tdn = n; c->tdc = 1; c->tdt = now; }
    else if (c->tds == 3 && c->tdn == n) { c->tds = 1; c->tdc = 2; c->tdp = i; c->tdt = now; }
  }
  else if (IS_MO(k)) { c->act[i] = k; c->mol[i] = k & 0xF; }
  else if (IS_TG(k)) c->toggled ^= 1u << (k & 0xF);
  else if (IS_MACRO(k)) { if (!c->latch_wait) macro_begin(c, c->mac, c->nmac, k & 0xFFF); }
  else if (IS_LT(k)) { c->act[i] = k; c->lts[i] = 1; c->t0[i] = now; }
  else if (k == KC_BOOT) c->boot_req = true;
  else if (c->latch_wait && !IS_CONS(k)) { c->lat = k; c->latch_wait = false; }
  else if (!IS_CONS(k) && qn(c) > 0) qpush_raw(c, k);   // dang xoa/go Morse: xep hang de giu dung thu tu
  else c->act[i] = k;
}
// ---- tap dance ----
static void td_release(core_t *c, int i) {
  if (!c->tds || i != c->tdp) return;
  if (c->tds == 1) { if (c->tdc == 2) { emit(c, c->x->td[c->tdn][2]); c->tds = 0; } else { c->tds = 3; } }
  else if (c->tds == 2) c->tds = 0;
}
static void td_flush(core_t *c) {     // co phim khac bam: chot tap dance ngay theo thu tu bam
  if (c->tds == 3) { emit(c, c->x->td[c->tdn][0]); c->tds = 0; }
  else if (c->tds == 1) { emit(c, c->x->td[c->tdn][c->tdc == 2 ? 2 : 0]); c->tds = 2; }
}
static void td_task(core_t *c, uint32_t now) {
  if (c->tds == 1 && now - c->tdt >= TAP_TERM_MS) {
    kc_t k = c->x->td[c->tdn][1]; int i = c->tdp; c->tds = 2;
    if (IS_MO(k)) { c->act[i] = k; c->mol[i] = k & 0xF; show(c, k); }
    else if (IS_BASIC(k) || IS_COMBO(k) || IS_CONS(k)) { mcommit(c); show(c, k); c->act[i] = k; }
    else emit(c, k);
  } else if (c->tds == 3 && now - c->tdt >= TAP_TERM_MS) { emit(c, c->x->td[c->tdn][0]); c->tds = 0; }
}
static bool in_combo(const core_t *c, kc_t k) {
  if (!c->x || !k || k == KC_TRNS || (k >= 0x4000 && !IS_CONS(k))) return false;
  for (int n = 0; n < NCOMBO; n++) if (c->x->cb[n][0] && (c->x->cb[n][0] == k || c->x->cb[n][1] == k)) return true;
  return false;
}
void core_scan_update(core_t *c, const uint8_t *raw, uint32_t now, const kc_t (*km)[NKEYS], const macro_t *macros, int nmacros) {
  c->mac = macros; c->nmac = nmacros;
  if (c->mwait) c->mwait--;
  if (c->tcons_ms && --c->tcons_ms == 0) c->tcons = 0;
  // 1) chong doi
  bool rel[NKEYS] = {0}, prs[NKEYS] = {0};
  for (int i = 0; i < NKEYS; i++) {
    if (!!raw[i] == c->st[i]) c->cnt[i] = 0;
    else if (++c->cnt[i] >= DEBOUNCE_MS) { c->st[i] = !!raw[i]; c->cnt[i] = 0; (c->st[i] ? prs : rel)[i] = true; }
  }
  // 2) phim LT dang cho: het TAP_TERM -> hold; phim tap giu du thoi gian -> nha
  for (int i = 0; i < NKEYS; i++) {
    if (c->lts[i] == 1 && now - c->t0[i] >= TAP_TERM_MS) { c->lts[i] = 2; c->mol[i] = (c->act[i] >> 8) & 0xF; }
    if (c->lts[i] == 3 && c->tapcnt[i] && --c->tapcnt[i] == 0) { c->lts[i] = 0; c->act[i] = 0; }
  }
  // 3) nha phim
  for (int i = 0; i < NKEYS; i++) if (rel[i]) {
    if (c->pend == i) { kc_t k = c->pkc; c->pend = -1; emit(c, k); }   // combo chua ghep duoc: coi nhu bam nhanh phim don
    if (c->x) td_release(c, i);
    if (c->lts[i] == 1) { c->lts[i] = 3; c->act[i] = c->act[i] & 0xFF; c->tapcnt[i] = 10; }   // tap
    else if (c->lts[i] == 3) { /* dang giu tap, de tapcnt xu ly */ }
    else { c->act[i] = 0; c->mol[i] = -1; c->lts[i] = 0; }
  }
  // 3b) tap dance het gio / combo het gio cho
  if (c->x) td_task(c, now);
  if (c->pend >= 0 && now - c->pt >= COMBO_TERM_MS) { int p = c->pend; c->pend = -1; fire(c, p, c->pkc, now); }
  // 4) nhan phim: phim khac bam luc LT dang cho -> chot hold ngay (permissive hold)
  for (int i = 0; i < NKEYS; i++) if (prs[i]) {
    for (int j = 0; j < NKEYS; j++) if (c->lts[j] == 1) { c->lts[j] = 2; c->mol[j] = (c->act[j] >> 8) & 0xF; }
    kc_t k = resolve(c, i, km);
    if (k == KC_NO) continue;
    if (c->x && c->tds && !(c->tds == 3 && IS_TD(k) && (k & 0xFF) == c->tdn)) td_flush(c);
    if (c->pend >= 0) {
      int hit = -1;
      for (int n = 0; n < NCOMBO; n++) {
        kc_t a = c->x->cb[n][0], b = c->x->cb[n][1];
        if (a && ((a == c->pkc && b == k) || (b == c->pkc && a == k))) hit = n;
      }
      if (hit >= 0) { c->pend = -1; emit(c, c->x->cb[hit][2]); continue; }
      int p = c->pend; c->pend = -1; fire(c, p, c->pkc, now);
    }
    if (in_combo(c, k)) { c->pend = i; c->pkc = k; c->pt = now; }
    else fire(c, i, k, now);
  }
}
static void add_key(kb_report_t *r, int *n, kc_t k) {
  if (IS_COMBO(k)) { uint8_t m = k >> 8; r->mods |= (m & 1 ? 0x01 : 0) | (m & 2 ? 0x02 : 0) | (m & 4 ? 0x04 : 0) | (m & 8 ? 0x08 : 0); k &= 0xFF; }
  if (k >= 0xE0 && k <= 0xE7) { r->mods |= 1u << (k - 0xE0); return; }
  if (!IS_BASIC(k)) return;
  for (int j = 0; j < *n; j++) if (r->keys[j] == k) return;
  if (*n < 6) r->keys[(*n)++] = k; else memset(r->keys, 0x01, 6);   // tran: ErrorRollOver
}
bool core_kb(const core_t *c, kb_report_t *r) {
  memset(r, 0, sizeof *r);
  if (c->mact) {
    if (c->mwait) return false;
    uint8_t md, k;
    if (c->mphase == 0 && core_ascii(c->mtxt[c->mpos], &md, &k)) { r->mods = md; r->keys[0] = k; }
    return true;
  }
  bool qbusy = c->qh != c->qt;
  if (qbusy && c->mwait) return false;
  int n = 0;
  for (int i = 0; i < NKEYS; i++) {
    kc_t k = c->act[i];
    if (!k || IS_CONS(k) || IS_MO(k)) continue;
    if (IS_LT(k)) { if (c->lts[i] != 3) continue; k &= 0xFF; }   // chi xuat khi dang o trang thai "da tap"
    add_key(r, &n, k);
  }
  if (c->lat) add_key(r, &n, c->lat);
  { static const uint8_t tgk[4] = {KC_SPACE, KC_LSFT, KC_LCTL, KC_LALT}; for (int b = 0; b < 4; b++) if (c->tg >> b & 1) add_key(r, &n, tgk[b]); }
  if (qbusy && c->qph == 0) add_key(r, &n, c->q[c->qt & (QN - 1)]);
  return true;
}
uint16_t core_cons(const core_t *c) {
  if (c->tcons_ms) return c->tcons;
  for (int i = 0; i < NKEYS; i++) if (c->act[i] && IS_CONS(c->act[i])) return c->act[i] & 0x7FFF;
  return 0;
}
void core_macro_ack(core_t *c) {
  if (!c->mact) {
    if (c->qh == c->qt) return;
    if (c->qph == 0) c->qph = 1; else { c->qph = 0; c->qt++; }
    c->mwait = QUEUE_GAP_MS;
    return;
  }
  if (c->mphase == 0) c->mphase = 1;
  else { c->mphase = 0; c->mpos++; macro_skip(c); }
  c->mwait = MACRO_GAP_MS;
  if (!c->mact) { c->mwait = 0; }
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
  } else if (IS_MACRO(k)) { ADD("MACRO "); char d[2] = {'1' + (k & 0xF), 0}; ADD(d); }
  else if (IS_TD(k)) { ADD("TAP DANCE "); char d[2] = {'1' + (k & 7), 0}; ADD(d); }
  else if (k == KC_LATCH) ADD("LATCH");
  else if (IS_LATCH(k)) { static const char *const ln[4] = {"LOCK SPACE", "LOCK SHIFT", "LOCK CTRL", "LOCK ALT"}; ADD(ln[k - KC_LATCH - 1]); }
  else if (k == KC_MORSE_DOT) ADD("DOT");
  else if (k == KC_MORSE_DASH) ADD("DASH");
  else if (k == KC_MORSE_OK) ADD("OK");
  else if (k == KC_MORSE_HINT) ADD("HINT");
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
