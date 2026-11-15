#pragma once
// Logic ban phim thuan C (khong phu thuoc Pico SDK / TinyUSB): chong doi, layer, tap-hold (LT), tap dance, combo, chuoi (SEQ),
// LATCH, TG Space/Shift/Ctrl/Alt, Nokia multitap, Morse, doi ten layer, macro dang buoc, tao bao cao HID.
// Kiem thu tren may tinh bang test/run_tests.sh. Toan bo ham deu static de nhung duoc vao main.c va file test.
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include "config.h"

#define NKEYS (NROW * NCOL)                 // 35 o ma tran (30 o co phim)
#define NLAYER 32
#define NMAC 32
#define MSLOT 1020                          // byte moi o macro
#define NSQ 8
#define NST 8
// ---- ban do word (uint16) giong app: khop hang so trong numpad30.html ----
#define W_KM 0                              // K(l,p) = l*NKEYS + p
#define W_BT (NLAYER * NKEYS)               // 1120: tap dance 4 x [tap, hold, double]
#define W_CB (W_BT + 12)                    // 1132: combo 4 x [A, B, ket qua]
#define W_NLI (W_CB + 12)                   // 1144: so layer
#define W_LDB (W_NLI + 1)                   // 1145: 9 word LED: [mode, toc do, sang, hue, sat, over, obMode, obSang, huong OLED]
#define W_NLD 9
#define W_LRB (W_LDB + W_NLD)               // 1154: huong OLED tung layer (0-3, >=4 theo huong chung)
#define W_OSB (W_LRB + NLAYER)              // 1186: cai dat man hinh [timeout s, toc do, greet, logo greet, so lan, kieu saver, ms/khung, du phong]
#define W_NOS 8
#define W_SQB (W_OSB + W_NOS)               // 1194: 8 bo x 8 buoc
#define W_AMB (W_SQB + NSQ * NST)           // 1258: chuot bay, 8 word: [do nhay X, do nhay Y, vung chet (0.1 do/s), lam muot 0-9, co (bit0 dao X, bit1 dao Y, bit2 Y theo gy, bit3 luon bay), 3 du phong]
#define W_NAMB 8
#define NW (W_AMB + W_NAMB)                 // 1266
#define LDB_ROT 8
// ---- du lieu OLED (lenh 8/9): [0..143] header, [256 + i*512] khung i ----
#define OD_NF 16
#define OD_FSZ (128 * OLED_H / 8)   // 1 khung logo = 128 x OLED_H pixel dang page SSD1306 (512 B cho 32 hang, 1024 B cho 64 hang)
#define OD_HDR 144
#define OD_START 256
#define OD_SIZE (OD_START + OD_NF * OD_FSZ)
#define NAMELEN 10                          // do dai toi da ten layer tren OLED

typedef uint16_t kc_t;
// ---- ma phim (khong gian ma cua app) ----
// 0 NO | 1 TRNS | 0x04..0xE7 phim HID (0xE0..0xE7 modifier) | (mods<<8)|kc to hop (mods C=1 S=2 A=4 G=8)
// 0x2000|(n<<8)|kc LT | 0x4000|n MO | 0x4100|n TL | 0x4200/0x4201 LY+/LY- | 0x5000|n TD | 0x6000|i Nokia
// 0x7070 AIR (giu de bay) | 0x7071 AIR bat/tat | 0x7072/73/74 chuot trai/phai/giua | 0x7075 hieu chuan gyro
// 0x7001 LATCH | 0x7002..5 TG Space/Shift/Ctrl/Alt | 0x7010..16 LED (bo qua) | 0x7020 RENAME | 0x7060..63 Morse
// 0x7100|n MACRO | 0x7200/0x7280/0x7300|n SEQ tien/lui/dat lai | 0x7F00 BOOT | 0x8000|u media
#define KC_NO 0x0000
#define KC_TRNS 0x0001
#define C_(k) (0x0100 | (k))
#define S_(k) (0x0200 | (k))
#define A_(k) (0x0400 | (k))
#define G_(k) (0x0800 | (k))
#define LT(n, k) (0x2000 | ((n) << 8) | (k))
#define MO(n) (0x4000 | (n))
#define TL(n) (0x4100 | (n))
#define KC_LYUP 0x4200
#define KC_LYDN 0x4201
#define TD(n) (0x5000 | (n))
#define NKK(i) (0x6000 | (i))
#define KC_LATCH 0x7001
#define KC_TGSPC 0x7002
#define KC_TGSFT 0x7003
#define KC_TGCTL 0x7004
#define KC_TGALT 0x7005
#define KC_RENAME 0x7020
#define KC_MDOT 0x7060
#define KC_MDASH 0x7061
#define KC_MOK 0x7062
#define KC_MHINT 0x7063
#define KC_AIR 0x7070
#define KC_AIRT 0x7071
#define KC_MB1 0x7072
#define KC_MB2 0x7073
#define KC_MB3 0x7074
#define KC_AIRCAL 0x7075
#define MACRO(n) (0x7100 | (n))
#define SEQ_F 0x7200
#define SEQ_B 0x7280
#define SEQ_R 0x7300
#define KC_BOOT 0x7F00
#define CONS(x) (0x8000 | (x))
#define IS_BASIC(k) ((k) >= 0x04 && (k) <= 0xE7)
#define IS_COMBO(k) ((k) >= 0x0100 && (k) <= 0x0FFF && (((k) & 0xFF) == 0 || IS_BASIC((k) & 0xFF)))
#define IS_LT(k) (((k) & 0xE000) == 0x2000)
#define IS_MO(k) (((k) & 0xFF00) == 0x4000)
#define IS_TL(k) (((k) & 0xFF00) == 0x4100)
#define IS_TD(k) (((k) & 0xFFFC) == 0x5000)
#define IS_NKK(k) ((k) >= 0x6000 && (k) <= 0x600A)
#define IS_MORSE(k) ((k) >= KC_MDOT && (k) <= KC_MHINT)
#define IS_MOUSE(k) ((k) >= KC_AIR && (k) <= KC_AIRCAL)
#define IS_MACRO(k) (((k) & 0xFF00) == 0x7100 && ((k) & 0xFF) < NMAC)
#define IS_SEQ(k) ((((k) & 0xFFF8) == SEQ_F) || (((k) & 0xFFF8) == SEQ_B) || (((k) & 0xFFF8) == SEQ_R))
#define IS_CONS(k) ((k) & 0x8000)
#define IS_HOLDABLE(k) (IS_BASIC(k) || IS_COMBO(k) || IS_CONS(k))

enum {
  KC_A = 0x04, KC_C = 0x06, KC_D = 0x07, KC_S = 0x16, KC_V = 0x19, KC_X = 0x1B, KC_Y = 0x1C, KC_Z = 0x1D,
  KC_ENTER = 0x28, KC_ESC = 0x29, KC_BSPC = 0x2A, KC_TAB = 0x2B, KC_SPACE = 0x2C,
  KC_F1 = 0x3A, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12,
  KC_INS = 0x49, KC_HOME = 0x4A, KC_PGUP = 0x4B, KC_DEL = 0x4C, KC_END = 0x4D, KC_PGDN = 0x4E,
  KC_RIGHT = 0x4F, KC_LEFT = 0x50, KC_DOWN = 0x51, KC_UP = 0x52,
  KC_NUMLOCK = 0x53, KC_PSLS = 0x54, KC_PAST = 0x55, KC_PMNS = 0x56, KC_PPLS = 0x57, KC_PENT = 0x58,
  KC_P1 = 0x59, KC_P2, KC_P3, KC_P4, KC_P5, KC_P6, KC_P7, KC_P8, KC_P9, KC_P0 = 0x62, KC_PDOT = 0x63,
  KC_LCTL = 0xE0, KC_LSFT = 0xE1, KC_LALT = 0xE2, KC_LGUI = 0xE3,
  CN_PLAY = 0xCD, CN_NEXT = 0xB5, CN_PREV = 0xB6, CN_MUTE = 0xE2, CN_VOLU = 0xE9, CN_VOLD = 0xEA
};

typedef struct { uint8_t mods; uint8_t keys[6]; } kb_report_t;

// ---- cau hinh luu flash (toan bo thu app doc/ghi duoc, tru lname) ----
typedef struct {
  uint16_t w[NW];
  uint8_t mac[NMAC][MSLOT];
  uint8_t od[OD_SIZE];
  char lname[NLAYER][12];                 // ten layer tren OLED (doi bang phim RENAME), "" = ten mac dinh
} cfg_t;

typedef struct { uint8_t mods, key; uint16_t cons; } out_t;
#define QN 96

enum { P_NONE, P_LT, P_LTH, P_TD1, P_TD2, P_TDH, P_TDD };

typedef struct {
  cfg_t cfg;                              // PHAI la thanh vien dau: core_reset giu nguyen cfg
  uint8_t cnt[NKEYS], st[NKEYS];          // chong doi
  kc_t act[NKEYS];                        // ma dang giu cua phim i
  uint8_t hold_min[NKEYS], up_pend[NKEYS];// phim "tap" gia: giu toi thieu, nha vat ly roi thi doi het thoi gian moi nha
  uint8_t swallow[NKEYS];
  uint8_t pend[NKEYS]; kc_t pcode[NKEYS]; uint32_t t0[NKEYS];
  int8_t mol[NKEYS];                      // layer dang giu (MO / LT hold), -1 neu khong
  int8_t cbpair[NKEYS];
  uint32_t toggled;                       // bit layer dang bat bang TL / LY+
  uint8_t tg;                             // bit0 Space bit1 Shift bit2 Ctrl bit3 Alt
  kc_t latch_code; bool latch_wait;
  int cbi; kc_t cbk; uint32_t cbt0;       // combo cho
  int dfi; kc_t dfk; uint8_t dfw;         // phim bi hoan lai de di sau 1 phim "tap" vua chot (bao cao rieng, dung thu tu)
  int8_t sq_cur[NSQ];
  out_t q[QN]; uint8_t qh, qn, qphase;    // hang doi go phim (macro, Morse, Nokia): moi muc = 1 lan bam + nha
  uint16_t mwait;                         // ms cho truoc khi gui bao cao / buoc macro ke tiep
  bool m_on; uint8_t m_idx, m_rep; uint16_t m_p, m_tp; kc_t mhold[6]; uint8_t mhn;
  int nk_key; uint8_t nk_n; uint32_t nk_t;
  char mo_code[8]; uint8_t mo_len, mo_shown, mo_hint, mo_hopt;
  bool rn_on; char rn_buf[NAMELEN + 2]; uint8_t rn_len; uint8_t rn_layer; int rn_nk; uint8_t rn_n; uint32_t rn_t;
  bool boot_req, save_req;
  uint8_t mact[NKEYS];                    // phim chuot dang giu (KC_AIR / KC_MB1..3) cua phim i, 0 = khong
  bool air_tog, cal_req;                  // KC_AIRT dang bat; yeu cau hieu chuan gyro (main.c xu ly)
  uint8_t imu_ok, imu_id;                 // main.c dat: cam bien da nhan (0/1) va gia tri WHO_AM_I
  struct { int16_t g[3], b[3], px, py; uint8_t st, sent, rej; } dbg;   // main.c dien ngay truoc proto_cmd: gyro tho, bias, pixel cho, co (bit0 bay, bit1 hieu chuan), dem HID
  kc_t last_kc; uint32_t press_seq; uint32_t touch;
} core_t;

// ---------------------------------------------------------------- tien ich
static inline int core_nl(const core_t *c) { int n = c->cfg.w[W_NLI]; return n < 1 ? 1 : n > NLAYER ? NLAYER : n; }
static uint32_t core_active(const core_t *c) {
  uint32_t m = 1 | c->toggled;
  for (int i = 0; i < NKEYS; i++) if (c->mol[i] >= 0) m |= 1u << c->mol[i];
  int n = core_nl(c);
  if (n < 32) m &= (1u << n) - 1;
  return m | 1;
}
static int core_top_layer(const core_t *c) {
  uint32_t m = core_active(c);
  for (int l = NLAYER - 1; l > 0; l--) if (m >> l & 1) return l;
  return 0;
}
static void core_reset(core_t *c) {       // dat lai trang thai chay, giu cau hinh (va trang thai cam bien do main.c dat)
  uint8_t iok = c->imu_ok, iid = c->imu_id;
  memset((uint8_t *)c + sizeof(cfg_t), 0, sizeof(core_t) - sizeof(cfg_t));
  c->imu_ok = iok; c->imu_id = iid;
  for (int i = 0; i < NKEYS; i++) { c->mol[i] = -1; c->cbpair[i] = -1; }
  c->cbi = -1; c->dfi = -1; c->nk_key = -1; c->rn_nk = -1; c->mo_hopt = 1;
  for (int i = 0; i < NSQ; i++) c->sq_cur[i] = -1;
}
static bool core_ascii(char ch, uint8_t *md, uint8_t *key) {
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

// ---------------------------------------------------------------- hang doi go phim
static int q_room(const core_t *c) { return QN - c->qn; }
static bool q_push(core_t *c, uint8_t mods, uint8_t key, uint16_t cons) {
  if (c->qn >= QN) return false;
  out_t *o = &c->q[(c->qh + c->qn) % QN]; o->mods = mods; o->key = key; o->cons = cons; c->qn++;
  return true;
}
static bool q_ascii(core_t *c, char ch) { uint8_t m, k; return core_ascii(ch, &m, &k) && q_push(c, m, k, 0); }
static bool q_bs(core_t *c) { return q_push(c, 0, KC_BSPC, 0); }
static bool q_code(core_t *c, kc_t k) {   // go 1 lan bam cua ma phim bat ky (phim thuong, to hop, modifier, media)
  if (IS_CONS(k)) return q_push(c, 0, 0, k & 0x7FFF);
  if (IS_COMBO(k)) return q_push(c, (k >> 8) & 0xF, k & 0xFF, 0);
  if (k >= 0xE0 && k <= 0xE7) return q_push(c, 1u << (k - 0xE0), 0, 0);
  if (IS_BASIC(k)) return q_push(c, 0, k, 0);
  return false;
}

// ---------------------------------------------------------------- Morse (khop morse engine trong app)
#define MORSE_MAX 7
static const char *const MORSE_TAB[][2] = {
  {"a", ".-"}, {"b", "-..."}, {"c", "-.-."}, {"d", "-.."}, {"e", "."}, {"f", "..-."}, {"g", "--."}, {"h", "...."}, {"i", ".."},
  {"j", ".---"}, {"k", "-.-"}, {"l", ".-.."}, {"m", "--"}, {"n", "-."}, {"o", "---"}, {"p", ".--."}, {"q", "--.-"}, {"r", ".-."},
  {"s", "..."}, {"t", "-"}, {"u", "..-"}, {"v", "...-"}, {"w", ".--"}, {"x", "-..-"}, {"y", "-.--"}, {"z", "--.."}, {"0", "-----"},
  {"1", ".----"}, {"2", "..---"}, {"3", "...--"}, {"4", "....-"}, {"5", "....."}, {"6", "-...."}, {"7", "--..."}, {"8", "---.."}, {"9", "----."},
  {".", ".-.-.-"}, {",", "--..--"}, {"?", "..--.."}, {"'", ".----."}, {"!", "-.-.--"}, {"/", "-..-."}, {"(", "-.--."}, {")", "-.--.-"}, {"&", ".-..."},
  {":", "---..."}, {";", "-.-.-."}, {"=", "-...-"}, {"+", ".-.-."}, {"-", "-....-"}, {"_", "..--.-"}, {"\"", ".-..-."}, {"$", "...-..-"}, {"@", ".--.-."}
};
#define NMORSE ((int)(sizeof MORSE_TAB / sizeof MORSE_TAB[0]))
static char morse_find(const char *code) { for (int i = 0; i < NMORSE; i++) if (!strcmp(MORSE_TAB[i][1], code)) return MORSE_TAB[i][0][0]; return 0; }
static int morse_hint(const char *code, char *o) {   // " [.i -a]"; tra do dai
  o[0] = 0; if (strlen(code) >= MORSE_MAX) return 0;
  char b[24]; b[0] = 0; int nb = 0;
  for (int d = 0; d < 2; d++) {
    char nc[10]; size_t l = strlen(code); memcpy(nc, code, l); nc[l] = d ? '-' : '.'; nc[l + 1] = 0;
    char ch = morse_find(nc);
    if (ch) { if (nb) strcat(b, " "); char t[3] = {d ? '-' : '.', ch, 0}; strcat(b, t); nb++; }
  }
  if (!nb) return 0;
  strcpy(o, " ["); strcat(o, b); strcat(o, "]"); return (int)strlen(o);
}
static int morse_press(core_t *c, bool dash) {
  if (c->mo_len >= MORSE_MAX) return 0;
  char nc[10]; memcpy(nc, c->mo_code, c->mo_len); nc[c->mo_len] = dash ? '-' : '.'; nc[c->mo_len + 1] = 0;
  char ch = morse_find(nc), h[24]; int hl = c->mo_hopt ? morse_hint(nc, h) : (h[0] = 0, 0);
  int del = (c->mo_shown ? 1 : 0) + c->mo_hint;
  if (q_room(c) < del + (ch ? 1 : 0) + 2 * hl) return 0;
  for (int i = 0; i < del; i++) q_bs(c);
  if (ch) q_ascii(c, ch);
  for (int i = 0; i < hl; i++) q_ascii(c, h[i]);
  memcpy(c->mo_code, nc, c->mo_len + 2); c->mo_len++; c->mo_shown = ch != 0; c->mo_hint = (uint8_t)hl;
  return 1;
}
static void morse_commit(core_t *c) { for (int i = 0; i < c->mo_hint; i++) q_bs(c); c->mo_len = 0; c->mo_code[0] = 0; c->mo_shown = 0; c->mo_hint = 0; }
static void morse_set_hint(core_t *c, bool on) {
  if (on == !!c->mo_hopt) return;
  if (!on) { if (q_room(c) < c->mo_hint) return; for (int i = 0; i < c->mo_hint; i++) q_bs(c); c->mo_hint = 0; c->mo_hopt = 0; return; }
  char h[24]; int hl = c->mo_len ? morse_hint(c->mo_code, h) : 0;
  if (q_room(c) < 2 * hl) return;
  c->mo_hopt = 1; for (int i = 0; i < hl; i++) q_ascii(c, h[i]); c->mo_hint = (uint8_t)hl;
}

// ---------------------------------------------------------------- Nokia multitap, doi ten layer
static const char *const NK_TAB[11] = {"1abc", "2def", "3ghi", "4jkl", "5mno", "6pqr", "7stu", "8vwx", "9yz", "0 ", ".,?!"};
static void nokia_press(core_t *c, int g, uint32_t now) {
  int len = (int)strlen(NK_TAB[g]);
  if (c->nk_key == g && now - c->nk_t < NOKIA_MS) { q_bs(c); c->nk_n = (c->nk_n + 1) % len; }
  else { c->nk_key = g; c->nk_n = 0; }
  q_ascii(c, NK_TAB[g][c->nk_n]); c->nk_t = now;
}
static void rename_end(core_t *c, bool save) {
  if (save) {
    memset(c->cfg.lname[c->rn_layer], 0, 12); memcpy(c->cfg.lname[c->rn_layer], c->rn_buf, c->rn_len);
    c->save_req = true;
  }
  c->rn_on = false;
}
static void rename_press(core_t *c, kc_t k, uint32_t now) {
  if (IS_NKK(k)) {
    int g = k & 0xFF, len = (int)strlen(NK_TAB[g]);
    char ch;
    if (c->rn_nk == g && now - c->rn_t < NOKIA_MS && c->rn_len) { c->rn_n = (c->rn_n + 1) % len; ch = NK_TAB[g][c->rn_n]; c->rn_buf[c->rn_len - 1] = ch >= 'a' && ch <= 'z' ? ch - 32 : ch; }
    else if (c->rn_len < NAMELEN) { c->rn_nk = g; c->rn_n = 0; ch = NK_TAB[g][0]; c->rn_buf[c->rn_len++] = ch >= 'a' && ch <= 'z' ? ch - 32 : ch; }
    c->rn_buf[c->rn_len] = 0; c->rn_t = now; return;
  }
  c->rn_nk = -1;
  if (k == KC_BSPC) { if (c->rn_len) c->rn_buf[--c->rn_len] = 0; }
  else if (k == KC_ENTER || k == KC_PENT) rename_end(c, true);
  else if (k == KC_RENAME || k == KC_ESC || k == KC_TAB) rename_end(c, false);
}

// ---------------------------------------------------------------- macro (dinh dang o: [lap, nghi(2), dai ten, ten(12), dai buoc(2), buoc...])
static void macro_begin(core_t *c, int idx) {
  if (c->m_on || idx < 0 || idx >= NMAC) return;
  c->m_on = true; c->m_idx = (uint8_t)idx; c->m_rep = 0; c->m_p = 0; c->m_tp = 0; c->mhn = 0;
}
static void macro_stop(core_t *c) { c->m_on = false; c->mhn = 0; }
static void macro_tick(core_t *c) {
  if (!c->m_on || c->qn || c->mwait) return;
  const uint8_t *s = c->cfg.mac[c->m_idx];
  uint16_t len = (uint16_t)(s[16] | (s[17] << 8)); if (len > MSLOT - 18) len = MSLOT - 18;
  const uint8_t *st = s + 18;
  if (c->m_p >= len) {
    uint8_t rep = s[0] ? s[0] : 1; uint16_t gap = (uint16_t)(s[1] | (s[2] << 8));
    if (++c->m_rep < rep) { c->m_p = 0; c->m_tp = 0; c->mwait = gap ? gap : MACRO_GAP_MS; }
    else macro_stop(c);
    return;
  }
  uint8_t op = st[c->m_p];
  if (op >= 1 && op <= 4) {
    if (c->m_p + 3 > len) { macro_stop(c); return; }
    kc_t k = (kc_t)(st[c->m_p + 1] | (st[c->m_p + 2] << 8)); c->m_p += 3;
    if (op == 1) q_code(c, k);
    else if (op == 2) { if (c->mhn < 6 && !IS_CONS(k) && (IS_BASIC(k) || IS_COMBO(k))) c->mhold[c->mhn++] = k; c->mwait = MACRO_GAP_MS; }
    else if (op == 3) { for (int i = 0; i < c->mhn; i++) if (c->mhold[i] == k) { c->mhold[i] = c->mhold[--c->mhn]; break; } c->mwait = MACRO_GAP_MS; }
    else c->mwait = k;
  } else if (op == 5) {
    if (c->m_p + 2 > len) { macro_stop(c); return; }
    uint8_t n = st[c->m_p + 1];
    if (c->m_p + 2 + n > len) { macro_stop(c); return; }
    if (c->m_tp >= n) { c->m_p += 2 + n; c->m_tp = 0; return; }
    char ch = (char)st[c->m_p + 2 + c->m_tp++]; q_ascii(c, ch);
  } else macro_stop(c);
}

// ---------------------------------------------------------------- xu ly phim
static kc_t resolve(const core_t *c, int i) {
  uint32_t m = core_active(c);
  for (int l = NLAYER - 1; l >= 0; l--) if (m >> l & 1) { kc_t k = c->cfg.w[W_KM + l * NKEYS + i]; if (k != KC_TRNS) return k; }
  return KC_NO;
}
static void layer_cycle(core_t *c, int d) {
  int cur = 0, n = core_nl(c);
  for (int l = NLAYER - 1; l > 0; l--) if (c->toggled >> l & 1) { cur = l; break; }
  int t = ((cur + d) % n + n) % n; c->toggled = t ? 1u << t : 0;
}
static void seq_step(core_t *c, kc_t *kp) {   // SEQ -> ma buoc ke tiep (0 = khong lam gi)
  kc_t k = *kp; int g = k & 0xFFF8, n = k & 7, len = 0;
  const uint16_t *st = &c->cfg.w[W_SQB + n * NST];
  while (len < NST && st[len]) len++;
  if (g == SEQ_R) { c->sq_cur[n] = -1; *kp = 0; return; }
  if (!len) { *kp = 0; return; }
  int cur = c->sq_cur[n], i;
  if (g == SEQ_F) i = (cur < 0 || cur >= len) ? 0 : (cur + 1) % len;
  else i = (cur < 0 || cur >= len) ? len - 1 : (cur + len - 1) % len;
  c->sq_cur[n] = (int8_t)i;
  kc_t s = st[i];
  *kp = (s == KC_TRNS || IS_TD(s) || IS_SEQ(s)) ? 0 : s;
}
static void press_code(core_t *c, int i, kc_t k, uint32_t now) {
  if (!k || k == KC_TRNS) return;
  if (IS_SEQ(k)) { seq_step(c, &k); if (!k) return; }
  if (c->rn_on) { rename_press(c, k, now); return; }
  if (c->latch_wait && IS_HOLDABLE(k) && !IS_CONS(k)) { c->latch_code = k; c->latch_wait = false; c->swallow[i] = 1; return; }
  if (c->latch_wait && IS_CONS(k)) { c->latch_wait = false; c->swallow[i] = 1; return; }
  if (!IS_NKK(k)) c->nk_key = -1;
  if (IS_HOLDABLE(k)) c->act[i] = k;
  else if (IS_MO(k)) { if ((k & 0xFF) < NLAYER) c->mol[i] = (int8_t)(k & 0xFF); }
  else if (IS_TL(k)) { int n = k & 0xFF; if (n == 0) c->toggled = 0; else if (n < NLAYER) c->toggled ^= 1u << n; }
  else if (k == KC_LYUP) layer_cycle(c, 1);
  else if (k == KC_LYDN) layer_cycle(c, -1);
  else if (k == KC_LATCH) { if (c->latch_code) c->latch_code = 0; else c->latch_wait = true; }
  else if (k >= KC_TGSPC && k <= KC_TGALT) c->tg ^= 1u << (k - KC_TGSPC);
  else if (k == KC_RENAME) { c->rn_on = true; c->rn_len = 0; c->rn_buf[0] = 0; c->rn_layer = (uint8_t)core_top_layer(c); c->rn_nk = -1; c->latch_wait = false; }
  else if (IS_NKK(k)) nokia_press(c, k & 0xFF, now);
  else if (IS_MORSE(k)) {
    if (k == KC_MDOT) morse_press(c, false); else if (k == KC_MDASH) morse_press(c, true);
    else if (k == KC_MOK) morse_commit(c); else morse_set_hint(c, !c->mo_hopt);
  }
  else if (IS_MOUSE(k)) { if (k == KC_AIRT) c->air_tog = !c->air_tog; else if (k == KC_AIRCAL) c->cal_req = true; else c->mact[i] = (uint8_t)(k & 0xFF); }
  else if (IS_MACRO(k)) macro_begin(c, k & 0xFF);
  else if (k == KC_BOOT) c->boot_req = true;
}
static void do_release(core_t *c, int i) {
  c->act[i] = 0; c->mact[i] = 0; c->mol[i] = -1; c->pend[i] = P_NONE; c->hold_min[i] = 0; c->up_pend[i] = 0;
  int j = c->cbpair[i]; if (j >= 0) { c->cbpair[i] = -1; c->cbpair[j] = -1; c->act[j] = 0; c->swallow[j] = 1; }
}
static void fire_tap(core_t *c, int i, kc_t k, uint32_t now) {   // phim "tap" tao ra: giu >= TAP_HOLD_MS; i da nha vat ly thi nha sau do
  press_code(c, i, k, now);
  c->hold_min[i] = TAP_HOLD_MS;
}
static void td_fire_tap(core_t *c, int i, uint32_t now) {
  int n = c->pcode[i] & 3; c->pend[i] = P_NONE;
  fire_tap(c, i, c->cfg.w[W_BT + n * 3], now); c->up_pend[i] = 1;
}
static bool flush_others(core_t *c, int except, uint32_t now) {   // phim khac bam luc LT / tap dance dang cho -> chot ngay; true neu vua chot 1 "tap"
  bool tap = false;
  for (int j = 0; j < NKEYS; j++) if (j != except) {
    if (c->pend[j] == P_LT) { c->pend[j] = P_LTH; c->mol[j] = (c->pcode[j] >> 8) & 0x1F; }
    else if (c->pend[j] == P_TD1) {
      int n = c->pcode[j] & 3; kc_t h = c->cfg.w[W_BT + n * 3 + 1]; if (!h) h = c->cfg.w[W_BT + n * 3];
      c->pend[j] = P_TDH; press_code(c, j, h, now);
    }
    else if (c->pend[j] == P_TD2) { td_fire_tap(c, j, now); tap = true; }
  }
  return tap;
}
static void dispatch_press(core_t *c, int i, kc_t k, uint32_t now) {
  flush_others(c, i, now);
  c->press_seq++; c->last_kc = k;
  if (IS_LT(k)) { c->pend[i] = P_LT; c->pcode[i] = k; c->t0[i] = now; }
  else if (IS_TD(k)) {
    int n = k & 3;
    if (c->pend[i] == P_TD2 && (c->pcode[i] & 3) == n) {   // lan bam thu hai
      kc_t d = c->cfg.w[W_BT + n * 3 + 2];
      c->pend[i] = P_TDD;
      if (d) { fire_tap(c, i, d, now); }
      else { kc_t t = c->cfg.w[W_BT + n * 3]; if (IS_HOLDABLE(t)) { q_code(c, t); q_code(c, t); } else fire_tap(c, i, t, now); }   // khong co "cham 2x": go 2 lan
    } else { c->pend[i] = P_TD1; c->pcode[i] = k; c->t0[i] = now; }
  }
  else press_code(c, i, k, now);
}
static bool is_combo_member(const core_t *c, kc_t k) {
  for (int n = 0; n < 4; n++) { kc_t a = c->cfg.w[W_CB + n * 3], b = c->cfg.w[W_CB + n * 3 + 1]; if (a && (k == a || k == b)) return true; }
  return false;
}
static void combo_flush(core_t *c, uint32_t now) {
  if (c->cbi < 0) return;
  int i = c->cbi; c->cbi = -1;
  dispatch_press(c, i, c->cbk, now);
}
static void defer_flush(core_t *c, uint32_t now) {
  if (c->dfi < 0) return;
  int i = c->dfi; c->dfi = -1; dispatch_press(c, i, c->dfk, now);
}
static void on_press(core_t *c, int i, uint32_t now) {
  c->touch++;
  defer_flush(c, now);
  bool ft = flush_others(c, i, now);
  if (c->pend[i] == P_TD2) { dispatch_press(c, i, (kc_t)(0x5000 | (c->pcode[i] & 3)), now); return; }   // lan bam thu hai cua tap dance: bo qua lop va combo
  kc_t k = resolve(c, i);
  if (k == KC_NO) return;
  if (ft) { c->dfi = i; c->dfk = k; c->dfw = TAP_HOLD_MS + 1; return; }   // di sau phim tap vua chot, o bao cao rieng
  if (c->cbi >= 0) {
    int a = c->cbi;
    for (int n = 0; n < 4; n++) {
      kc_t ca = c->cfg.w[W_CB + n * 3], cb = c->cfg.w[W_CB + n * 3 + 1], r = c->cfg.w[W_CB + n * 3 + 2];
      if (ca && ((c->cbk == ca && k == cb) || (c->cbk == cb && k == ca))) {
        c->cbi = -1;
        c->press_seq++; c->last_kc = r;
        c->cbpair[a] = (int8_t)i; c->cbpair[i] = (int8_t)a; c->swallow[i] = 1;
        if (r) press_code(c, a, r, now);
        return;
      }
    }
    combo_flush(c, now);
  }
  if (is_combo_member(c, k)) { c->cbi = i; c->cbk = k; c->cbt0 = now; return; }
  dispatch_press(c, i, k, now);
}
static void on_release(core_t *c, int i, uint32_t now) {
  if (c->dfi == i) { defer_flush(c, now); if (c->pend[i] == P_NONE) { c->hold_min[i] = TAP_HOLD_MS; c->up_pend[i] = 1; return; } }
  if (c->cbi == i) { combo_flush(c, now); if (c->pend[i] == P_NONE) { c->hold_min[i] = TAP_HOLD_MS; c->up_pend[i] = 1; return; } }
  if (c->swallow[i]) { c->swallow[i] = 0; if (c->cbpair[i] >= 0) do_release(c, i); return; }
  switch (c->pend[i]) {
    case P_LT: c->pend[i] = P_NONE; fire_tap(c, i, c->pcode[i] & 0xFF, now); c->up_pend[i] = 1; return;
    case P_TD1: c->pend[i] = P_TD2; c->t0[i] = now; return;
    case P_TD2: return;
    default: break;
  }
  if (c->hold_min[i]) c->up_pend[i] = 1; else do_release(c, i);
}
static void core_scan_update(core_t *c, const uint8_t *raw, uint32_t now) {
  if (c->mwait) c->mwait--;
  bool rel[NKEYS] = {0}, prs[NKEYS] = {0};
  for (int i = 0; i < NKEYS; i++) {
    if (!!raw[i] == c->st[i]) c->cnt[i] = 0;
    else if (++c->cnt[i] >= DEBOUNCE_MS) { c->st[i] = !!raw[i]; c->cnt[i] = 0; (c->st[i] ? prs : rel)[i] = true; }
  }
  for (int i = 0; i < NKEYS; i++) {   // thoi gian: LT / tap dance / giu toi thieu
    if (c->pend[i] == P_LT && now - c->t0[i] >= TAP_TERM_MS) { c->pend[i] = P_LTH; c->mol[i] = (c->pcode[i] >> 8) & 0x1F; }
    else if (c->pend[i] == P_TD1 && now - c->t0[i] >= TD_TERM_MS) {
      int n = c->pcode[i] & 3; kc_t h = c->cfg.w[W_BT + n * 3 + 1]; if (!h) h = c->cfg.w[W_BT + n * 3];
      c->pend[i] = P_TDH; press_code(c, i, h, now);
    }
    else if (c->pend[i] == P_TD2 && now - c->t0[i] >= TD_TERM_MS) td_fire_tap(c, i, now);
    if (c->hold_min[i] && --c->hold_min[i] == 0 && c->up_pend[i]) do_release(c, i);
  }
  if (c->dfi >= 0 && --c->dfw == 0) defer_flush(c, now);
  if (c->cbi >= 0 && now - c->cbt0 >= COMBO_MS) combo_flush(c, now);
  for (int i = 0; i < NKEYS; i++) if (rel[i]) on_release(c, i, now);
  for (int i = 0; i < NKEYS; i++) if (prs[i]) on_press(c, i, now);
  macro_tick(c);
}

// ---------------------------------------------------------------- bao cao HID
static void rpt_add(kb_report_t *r, int *n, kc_t k) {
  if (IS_COMBO(k)) { r->mods |= (k >> 8) & 0xF; k &= 0xFF; }
  if (k >= 0xE0 && k <= 0xE7) { r->mods |= 1u << (k - 0xE0); return; }
  if (!IS_BASIC(k)) return;
  for (int j = 0; j < *n && j < 6; j++) if (r->keys[j] == k) return;
  if (*n < 6) r->keys[(*n)++] = (uint8_t)k; else { memset(r->keys, 0x01, 6); *n = 7; }
}
static bool core_kb(const core_t *c, kb_report_t *r) {   // false = dang cho giua 2 buoc go phim
  memset(r, 0, sizeof *r);
  if (c->qn && c->mwait) return false;
  int n = 0;
  for (int i = 0; i < NKEYS; i++) { kc_t k = c->act[i]; if (k && !IS_CONS(k) && n < 7) rpt_add(r, &n, k); }
  static const kc_t tgk[4] = {KC_SPACE, KC_LSFT, KC_LCTL, KC_LALT};
  for (int b = 0; b < 4; b++) if (c->tg >> b & 1) rpt_add(r, &n, tgk[b]);
  if (c->latch_code && !IS_CONS(c->latch_code)) rpt_add(r, &n, c->latch_code);
  for (int i = 0; i < c->mhn; i++) rpt_add(r, &n, c->mhold[i]);
  if (c->qn && c->qphase == 0) { const out_t *o = &c->q[c->qh]; r->mods |= o->mods; if (o->key) rpt_add(r, &n, o->key); }
  if (n == 7) memset(r->keys, 0x01, 6);
  return true;
}
static uint16_t core_cons(const core_t *c) {
  if (c->qn && c->qphase == 0 && c->q[c->qh].cons) return c->q[c->qh].cons;
  for (int i = 0; i < NKEYS; i++) if (c->act[i] && IS_CONS(c->act[i])) return c->act[i] & 0x7FFF;
  if (c->latch_code && IS_CONS(c->latch_code)) return c->latch_code & 0x7FFF;
  return 0;
}
static inline bool core_out_busy(const core_t *c) { return c->qn != 0; }
static void core_out_ack(core_t *c) {   // goi sau moi bao cao da gui khi hang doi co muc
  if (!c->qn) return;
  if (c->qphase == 0) c->qphase = 1; else { c->qphase = 0; c->qh = (c->qh + 1) % QN; c->qn--; }
  c->mwait = MACRO_GAP_MS;
}

// ---------------------------------------------------------------- nhan ngan cho OLED (chi dung ky tu co trong font)
static void core_label(const core_t *c, kc_t k, char *o, int n) {
  char t[24]; t[0] = 0;
  #define ADD(s) strncat(t, s, sizeof t - strlen(t) - 1)
  char d[3] = {0, 0, 0};
  if (IS_CONS(k)) {
    switch (k & 0x7FFF) { case CN_MUTE: ADD("MUTE"); break; case CN_VOLU: ADD("VOL+"); break; case CN_VOLD: ADD("VOL-"); break;
      case CN_PLAY: ADD("PLAY"); break; case CN_NEXT: ADD("NEXT"); break; case CN_PREV: ADD("PREV"); break; default: ADD("MEDIA"); }
  } else if (IS_MACRO(k)) {
    const uint8_t *s = c->cfg.mac[k & 0xFF]; int nl = s[3] <= 12 ? s[3] : 0; int w = 0;
    for (int i = 0; i < nl && w < 12; i++) { char ch = (char)s[4 + i]; if (ch >= 'a' && ch <= 'z') ch -= 32; if (strchr("!()*+,-./0123456789:=?ABCDEFGHIJKLMNOPQRSTUVWXYZ ", ch)) { d[0] = ch; ADD(d); w++; } }
    if (!w) { char b[12]; snprintf(b, sizeof b, "MACRO %d", (k & 0xFF) + 1); ADD(b); }
  }
  else if (IS_SEQ(k)) { ADD("SEQ "); d[0] = '1' + (k & 7); ADD(d); ADD((k & 0xFFF8) == SEQ_F ? "+" : (k & 0xFFF8) == SEQ_B ? "-" : " RST"); }
  else if (IS_MO(k) || IS_TL(k)) { ADD("LAYER "); int v = k & 0xFF; if (v >= 10) { d[0] = '0' + v / 10; ADD(d); } d[0] = '0' + v % 10; ADD(d); }
  else if (k == KC_LYUP) ADD("LAYER+");
  else if (k == KC_LYDN) ADD("LAYER-");
  else if (IS_LT(k)) { core_label(c, k & 0xFF, o, n); return; }
  else if (IS_TD(k)) { ADD("TD "); d[0] = '0' + (k & 3); ADD(d); }
  else if (IS_NKK(k)) { d[0] = (k & 0xFF) == 10 ? '.' : (k & 0xFF) == 9 ? '0' : '1' + (k & 0xFF); ADD("NOKIA "); ADD(d); }
  else if (k == KC_LATCH) ADD("LATCH");
  else if (k == KC_TGSPC) ADD("TG SPACE"); else if (k == KC_TGSFT) ADD("TG SHIFT"); else if (k == KC_TGCTL) ADD("TG CTRL"); else if (k == KC_TGALT) ADD("TG ALT");
  else if (k == KC_RENAME) ADD("RENAME");
  else if (k == KC_AIR) ADD("AIR"); else if (k == KC_AIRT) ADD("AIR TG"); else if (k == KC_AIRCAL) ADD("AIR CAL");
  else if (k == KC_MB1) ADD("LMB"); else if (k == KC_MB2) ADD("RMB"); else if (k == KC_MB3) ADD("MMB");
  else if (k == KC_MDOT) ADD("DOT"); else if (k == KC_MDASH) ADD("DASH"); else if (k == KC_MOK) ADD("OK"); else if (k == KC_MHINT) ADD("HINT");
  else if (k == KC_BOOT) ADD("BOOT");
  else if (k == 1) ADD("-");
  else {
    if (IS_COMBO(k)) { uint8_t m = k >> 8; if (m & 1) ADD("CTL+"); if (m & 2) ADD("SFT+"); if (m & 4) ADD("ALT+"); if (m & 8) ADD("WIN+"); k &= 0xFF; }
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
      case 0x39: ADD("CAPS"); break; case 0x2D: ADD("-"); break; case 0x2E: ADD("="); break; case 0x36: ADD(","); break;
      case 0x37: ADD("."); break; case 0x38: ADD("/"); break;
      default: ADD("?"); }
  }
  #undef ADD
  strncpy(o, t, n - 1); o[n - 1] = 0;
}
static bool core_has_morse(const core_t *c, int l) {
  for (int p = 0; p < NKEYS; p++) { kc_t k = c->cfg.w[W_KM + l * NKEYS + p]; if (IS_MORSE(k)) return true; }
  return false;
}
static void core_layer_name(const core_t *c, int l, char *o, int n) {   // ten tren OLED: ten da luu, khong co thi mac dinh
  if (l < 0 || l >= NLAYER) l = 0;
  if (c->cfg.lname[l][0]) { strncpy(o, c->cfg.lname[l], n - 1); o[n - 1] = 0; return; }
  if (l == 0) strncpy(o, "BASE", n - 1); else if (core_has_morse(c, l)) strncpy(o, "MORSE", n - 1);
  else { char b[10]; snprintf(b, sizeof b, "LAYER%d", l); strncpy(o, b, n - 1); }
  o[n - 1] = 0;
}

// ---- chuot: nut dang giu (bit0 trai, bit1 phai, bit2 giua) va co "dang bay" ----
static uint8_t core_mouse_buttons(const core_t *c) {
  uint8_t b = 0;
  for (int i = 0; i < NKEYS; i++) { uint8_t m = c->mact[i]; if (m >= (KC_MB1 & 0xFF) && m <= (KC_MB3 & 0xFF)) b |= 1u << (m - (KC_MB1 & 0xFF)); }
  return b;
}
static bool core_air_on(const core_t *c) {
  if (c->air_tog || (c->cfg.w[W_AMB + 4] & 8)) return true;
  for (int i = 0; i < NKEYS; i++) if (c->mact[i] == (KC_AIR & 0xFF)) return true;
  return false;
}
