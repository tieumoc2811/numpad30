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
#include "tunes.h"

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
#define W_BZB (W_AMB + W_NAMB)              // 1266: coi bao Morse, 4 word: [bat 0/1, tan so Hz (200-5000), do dai cham ms (20-300), am luong % (1-100)]
#define W_NBZ 4
#define W_NKB (W_BZB + W_NBZ)               // 1270: phim Nokia keu nốt (v17), 12 word: [bat 0/1, nốt MIDI cua 11 phim: 1..9, 0, '.'; ngoai 48..108 -> mac dinh]
#define W_NNK 12
#define W_KNB (W_NKB + W_NNK)               // 1282: v18 nốt kêu theo tung phim / layer: [bat 0/1, do dai ms 20..300] + 560 word, moi word 2 byte (nốt MIDI 24..108, 0 = im); phim n = l*NKEYS+p nam o byte (n&1) cua word W_KNB+2+n/2
#define W_NKN (2 + (NLAYER * NKEYS + 1) / 2)   // 562
#define NW (W_KNB + W_NKN)                  // 1844
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
// 0x7070 AIR (giu de bay) | 0x7071 AIR bat/tat | 0x7072/73/74 chuot trai/phai/giua | 0x7075 hieu chuan gyro | 0x7076/77 cuon len/xuong (giu = lap lai)
// 0x7001 LATCH | 0x7002..5 TG Space/Shift/Ctrl/Alt | 0x7010..16 LED (bo qua) | 0x7020 RENAME | 0x7060..63 Morse
// 0x7490 tat/bat loa | 0x7491/92 am luong loa +/- (v18) | 0x7400|midi NOTE (v13) | 0x7480..89 dieu khien nhac cu | 0x7500|n bai nhac n=0..15 | 0x7600|n bai nhac n=16..255 (v16) | 0x7510/11 tempo -/+ | 0x7512 bai truoc | 0x7513 bai ke | 0x7514 phat/dung | 0x7515 tempo vong (v20)
// 0x7800|kc TK (bam 1 lan giu phim kc, bam lan nua nha; toi da NTK phim cung luc) | 0x7100|n MACRO | 0x7200/0x7280/0x7300|n SEQ tien/lui/dat lai | 0x7F00 BOOT | 0x8000|u media
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
#define KC_WHUP 0x7076
#define KC_WHDN 0x7077
#define MACRO(n) (0x7100 | (n))
#define SEQ_F 0x7200
#define SEQ_B 0x7280
#define SEQ_R 0x7300
#define KC_BOOT 0x7F00
// ---- nhac cu (v13): am don tren coi; NOTE(m) m = so MIDI 0..127 (C4 = 60, C5 = 72) ----
#define NOTE(m) (0x7400 | (m))
#define KC_OCTU 0x7480                      // quang tam +1 (toi da +3)
#define KC_OCTD 0x7481
#define KC_TRU 0x7482                       // chuyen giong +1 nua cung (toi da +11)
#define KC_TRD 0x7483
#define KC_PSTOP 0x7484                     // im lang: dung bai va tat het nốt dang giu
#define KC_PENV 0x7485                      // giu nốt <-> tat dan (pluck)
#define KC_PTIM 0x7486                      // am sac: xung 50% / 25% / 12%
#define KC_PVIB 0x7487                      // rung bat / tat
#define KC_PVOL 0x7488                      // am luong 100% / 50% / 25%
#define KC_DEMO 0x7489                      // phat bai ke tiep (vong qua tat ca NTUNE bai)
#define TUNE(n) ((n) < 16 ? (0x7500 | (n)) : (0x7600 | (n)))   // phat bai n (0..NTUNE-1): n < 16 -> 0x7500|n (giu nguyen tu v13), n >= 16 -> 0x7600|n (v16); bam lai khi dang phat = dung
#define KC_TMPD 0x7510                      // cham lai (5 muc)
#define KC_TMPU 0x7511
#define KC_TPREV 0x7512                     // v20 (layer MUSIC): bai truoc trong danh sach (vong tron) va phat ngay
#define KC_TNEXT 0x7513                     // v20: bai ke tiep va phat ngay
#define KC_TPLAY 0x7514                     // v20: dang phat = dung; dang im = phat lai bai dang chon (chua chon: bai 1)
#define KC_TCYC 0x7515                      // v20: doi tempo vong tron 5 muc (cham -> nhanh -> ve cham)
#define TK(k) (0x7800 | (k))
#define NTK 4                               // so phim toggle giu cung luc
#define CONS(x) (0x8000 | (x))
#define IS_BASIC(k) ((k) >= 0x04 && (k) <= 0xE7)
#define IS_COMBO(k) ((k) >= 0x0100 && (k) <= 0x0FFF && (((k) & 0xFF) == 0 || IS_BASIC((k) & 0xFF)))
#define IS_LT(k) (((k) & 0xE000) == 0x2000)
#define IS_MO(k) (((k) & 0xFF00) == 0x4000)
#define IS_TL(k) (((k) & 0xFF00) == 0x4100)
#define IS_TD(k) (((k) & 0xFFFC) == 0x5000)
#define IS_NKK(k) ((k) >= 0x6000 && (k) <= 0x600A)
#define IS_MORSE(k) ((k) >= KC_MDOT && (k) <= KC_MHINT)
#define IS_MOUSE(k) ((k) >= KC_AIR && (k) <= KC_WHDN)
#define IS_NOTE(k) (((k) & 0xFF80) == 0x7400)
#define KC_BZMUTE 0x7490                    // v18: tat / bat tieng loa (moi am: Morse, Nokia, nhac, bip phim)
#define KC_BZVU 0x7491                      // v18: am luong loa tang 1 muc (100 / 75 / 50 / 25 / 10 %)
#define KC_BZVD 0x7492                      // v18: am luong loa giam 1 muc
#define IS_BZCTL(k) ((k) >= KC_BZMUTE && (k) <= KC_BZVD)
#define IS_PCTL(k) ((k) >= KC_OCTU && (k) <= KC_DEMO)
#define IS_TUNE(k) ((((k) & 0xFFF0) == 0x7500 && ((k) & 15) < NTUNE) || (((k) & 0xFF00) == 0x7600 && ((k) & 0xFF) >= 16 && ((k) & 0xFF) < NTUNE))
#define TUNE_IDX(k) (((k) & 0xFF00) == 0x7600 ? ((k) & 0xFF) : ((k) & 15))   // chi so bai trong TUNE_TAB
#define IS_TEMPO(k) ((k) == KC_TMPD || (k) == KC_TMPU)
#define IS_TSEL(k) ((k) >= KC_TPREV && (k) <= KC_TCYC)   // v20: chon / phat / tempo vong cua danh sach bai
#define IS_PIANO(k) (IS_NOTE(k) || IS_PCTL(k) || IS_BZCTL(k) || IS_TUNE(k) || IS_TEMPO(k) || IS_TSEL(k))
#define IS_MACRO(k) (((k) & 0xFF00) == 0x7100 && ((k) & 0xFF) < NMAC)
#define IS_SEQ(k) ((((k) & 0xFFF8) == SEQ_F) || (((k) & 0xFFF8) == SEQ_B) || (((k) & 0xFFF8) == SEQ_R))
#define IS_TK(k) (((k) & 0xFF00) == 0x7800 && IS_BASIC((k) & 0xFF))
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
  uint8_t tk[NTK];                        // phim dang giu bang TK (0 = o trong)
  int cbi; kc_t cbk; uint32_t cbt0;       // combo cho
  int dfi; kc_t dfk; uint8_t dfw;         // phim bi hoan lai de di sau 1 phim "tap" vua chot (bao cao rieng, dung thu tu)
  int8_t sq_cur[NSQ];
  out_t q[QN]; uint8_t qh, qn, qphase;    // hang doi go phim (macro, Morse, Nokia): moi muc = 1 lan bam + nha
  uint16_t mwait;                         // ms cho truoc khi gui bao cao / buoc macro ke tiep
  bool m_on; uint8_t m_idx, m_rep; uint16_t m_p, m_tp; kc_t mhold[6]; uint8_t mhn;
  int nk_key; uint8_t nk_n; uint32_t nk_t;
  kc_t ch_k; int8_t ch_i; uint32_t ch_t0;   // v19: phim Morse / Nokia dang cho xem co phim thu hai (chord) khong
  uint32_t mo_t; uint8_t mo_auto;          // v19: thoi diem phim Cham / Gach cuoi; vua tu chot -> cho them Space
  uint8_t nk_cap, nk_up; char nk_lc, nk_pc;        // v19: cho viet hoa chu ke tiep / dang viet hoa luot Nokia nay / ky tu Nokia cuoi
  uint8_t ch_n; uint32_t ch_t;             // v19: so chord vua xong (0/1) va thoi diem chord do
  int8_t gh_i; kc_t gh_k; uint8_t gh_mode; int8_t gh_g; int8_t nk_pg;   // v19b: phim Morse / Nokia vua go (gh_i = vi tri, con giu thi la "phim giu"); gh_mode 0 = chua dung, 1 = dang giu de them Space, 2 = dang giu de xoa; gh_g = nhom Nokia duoc bam de xoa; nk_pg = nhom phim Nokia go truoc phim hien tai
  char ms_code[8]; uint8_t ms_len, ms_ok;  // v19b: ma Morse truoc phim cham / gach vua bam (de hoan tac khi phim do chi la phim giu)
  char mo_code[8]; uint8_t mo_len, mo_shown, mo_hint, mo_hopt;
  uint16_t buz_ms;                        // ms con lai cua tieng bip coi (Morse / Nokia); 0 = im
  uint8_t buz_mid;                        // 0 = bip Morse (tan so cai dat); >0 = nốt MIDI cua tieng bip phim Nokia (v17)
  bool rn_on; char rn_buf[NAMELEN + 2]; uint8_t rn_len; uint8_t rn_layer; int rn_nk; uint8_t rn_n; uint32_t rn_t;
  bool boot_req, save_req;
  uint8_t mact[NKEYS];                    // phim chuot dang giu (KC_AIR / KC_MB1..3) cua phim i, 0 = khong
  int8_t wh_dir; uint32_t wh_due;         // con lan chuot: huong dang giu (+1 len, -1 xuong, 0 khong) va thoi diem gui nac ke tiep
  bool air_tog, cal_req;                  // KC_AIRT dang bat; yeu cau hieu chuan gyro (main.c xu ly)
  uint8_t imu_ok, imu_id;                 // main.c dat: cam bien da nhan (0/1) va gia tri WHO_AM_I
  struct { int16_t g[3], b[3], px, py; uint8_t st, sent, rej; } dbg;   // main.c dien ngay truoc proto_cmd: gyro tho, bias, pixel cho, co (bit0 bay, bit1 hieu chuan), dem HID
  // ---- nhac cu (v13), chi o RAM ----
  uint8_t nt_stk[8], nt_n;                // cac nốt (MIDI) dang giu; nốt cuoi = nốt dang keu (am don, uu tien nốt moi)
  uint8_t ntk[NKEYS];                     // nốt dang giu cua phim i (0 = khong)
  int8_t pn_oct, pn_tr;                   // dich quang tam -3..3, chuyen giong -11..11
  uint8_t bz_mute, bz_lvl;               // v18: tat tieng loa 0/1; muc am luong loa 0..4 (100 / 75 / 50 / 25 / 10 %)
  uint8_t pn_env, pn_tim, pn_vib, pn_vol; // 0 giu / 1 tat dan; am sac 0..2; rung 0/1; am luong 0..2 (100 / 50 / 25 %)
  uint8_t tn_cur, tn_note, tn_tempo, pn_demo;   // bai dang phat (n + 1, 0 = khong), nốt dang phat (0 = nghi), muc tempo 0..4, bai demo ke tiep
  uint8_t tn_sel;                         // v20: bai chon / phat gan nhat (n + 1, 0 = chua chon): bai truoc / ke tiep dua vao day
  uint16_t tn_i, tn_left, tn_gap;         // vi tri nốt, ms con lai, ms nghi them sau nốt
  uint16_t nt_age;                        // ms ke tu luc nốt dang keu bat dau (bao hoa o 65535)
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
  c->cbi = -1; c->dfi = -1; c->ch_i = -1; c->gh_i = -1; c->nk_pg = -1; c->nk_key = -1; c->rn_nk = -1; c->mo_hopt = 1;
  for (int i = 0; i < NSQ; i++) c->sq_cur[i] = -1;
  c->pn_vol = 0; c->tn_tempo = 2;
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

// ---------------------------------------------------------------- co go chu (v19): word du phong W_AMB + 7
// bit0 = 1: TAT Morse tu chot khi nghi | bit1 = 1: Nokia so o DAU moi phim (kieu cu "2def"); 0 = so o CUOI ("def2", giong dien thoai). Mac dinh 0 0 = bat tu chot + so o cuoi.
#define W_TYP (W_AMB + 7)
static inline bool morse_auto_on(const cfg_t *g) { return !(g->w[W_TYP] & 1); }
static inline bool nk_digit_first(const cfg_t *g) { return (g->w[W_TYP] & 2) != 0; }

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
static inline bool buz_enabled(const cfg_t *g) { return g->w[W_BZB] != 0; }
static inline uint32_t buz_freq(const cfg_t *g) { uint16_t v = g->w[W_BZB + 1]; return v < 200 || v > 5000 ? BUZ_FREQ : v; }
static inline uint16_t buz_unit(const cfg_t *g) { uint16_t v = g->w[W_BZB + 2]; return v < 20 || v > 300 ? BUZ_DOT_MS : v; }   // "cham" (ms)
static inline uint16_t buz_vol(const cfg_t *g) { uint16_t v = g->w[W_BZB + 3]; return v < 1 || v > 100 ? 100 : v; }
static int morse_press(core_t *c, bool dash) {
  // Da du MORSE_MAX ky hieu: quay vong, xoa ky tu tam va bat dau lai tu ky hieu vua bam
  int base = c->mo_len >= MORSE_MAX ? 0 : c->mo_len;
  char nc[10]; memcpy(nc, c->mo_code, base); nc[base] = dash ? '-' : '.'; nc[base + 1] = 0;
  char ch = morse_find(nc), h[24]; int hl = c->mo_hopt ? morse_hint(nc, h) : (h[0] = 0, 0);
  int del = (c->mo_shown ? 1 : 0) + c->mo_hint;
  if (q_room(c) < del + (ch ? 1 : 0) + 2 * hl) return 0;
  for (int i = 0; i < del; i++) q_bs(c);
  if (ch) q_ascii(c, ch);
  for (int i = 0; i < hl; i++) q_ascii(c, h[i]);
  memcpy(c->mo_code, nc, base + 2); c->mo_len = (uint8_t)(base + 1); c->mo_shown = ch != 0; c->mo_hint = (uint8_t)hl;
  if (buz_enabled(&c->cfg)) { c->buz_mid = 0; c->buz_ms = dash ? 3 * buz_unit(&c->cfg) : buz_unit(&c->cfg); }
  return 1;
}
static void morse_commit(core_t *c) { c->ms_ok = 0; if (c->mo_len && buz_enabled(&c->cfg)) { c->buz_mid = 0; c->buz_ms = (uint16_t)(buz_unit(&c->cfg) * 4 / 7); } for (int i = 0; i < c->mo_hint; i++) q_bs(c); c->mo_len = 0; c->mo_code[0] = 0; c->mo_shown = 0; c->mo_hint = 0; }
static void morse_set_hint(core_t *c, bool on) {
  if (on == !!c->mo_hopt) return;
  if (!on) { if (q_room(c) < c->mo_hint) return; for (int i = 0; i < c->mo_hint; i++) q_bs(c); c->mo_hint = 0; c->mo_hopt = 0; return; }
  char h[24]; int hl = c->mo_len ? morse_hint(c->mo_code, h) : 0;
  if (q_room(c) < 2 * hl) return;
  c->mo_hopt = 1; for (int i = 0; i < hl; i++) q_ascii(c, h[i]); c->mo_hint = (uint8_t)hl;
}

// ---------------------------------------------------------------- Nokia multitap, doi ten layer
static const char *const NK_TAB_D[11] = {"1abc", "2def", "3ghi", "4jkl", "5mno", "6pqr", "7stu", "8vwx", "9yz", "0 ", ".,?!"};   // so o dau (kieu cu)
static const char *const NK_TAB_L[11] = {"abc1", "def2", "ghi3", "jkl4", "mno5", "pqr6", "stu7", "vwx8", "yz9", " 0", ".,?!"};     // v19: so o cuoi (mac dinh)
static inline const char *nk_tab(const cfg_t *g, int k) { return nk_digit_first(g) ? NK_TAB_D[k] : NK_TAB_L[k]; }
// v17: moi phim Nokia keu 1 nốt nhac ngan. Cong tac + nốt tung phim nam trong W_NKB (app Studio chinh); am luong dung chung voi coi (W_BZB[3]), khong phu thuoc cong tac Morse.
static const uint8_t NK_DEF[11] = {84, 86, 88, 89, 91, 93, 95, 96, 98, 81, 100};   // mac dinh: thang C truong C6 D6 E6 F6 G6 A6 B6 C7 D7 | 0 = A5 | . = E7
static inline bool nk_beep_on(const cfg_t *g) { return g->w[W_NKB] != 0; }
static inline uint8_t nk_note(const cfg_t *g, int k) { uint16_t v = g->w[W_NKB + 1 + k]; return v < 48 || v > 108 ? NK_DEF[k] : (uint8_t)v; }
// v18: nốt kêu theo tung phim (moi layer, moi phim); phim giu chuc nang cu, chi them tieng bip nốt khi bam. Nốt 0 = im.
static inline bool kn_on(const cfg_t *g) { return g->w[W_KNB] != 0; }
static inline uint16_t kn_ms(const cfg_t *g) { uint16_t v = g->w[W_KNB + 1]; return v < 20 || v > 300 ? NK_BEEP_MS : v; }
static inline uint8_t kn_note(const cfg_t *g, int l, int p) { int n = l * NKEYS + p; uint16_t v = (uint16_t)(g->w[W_KNB + 2 + n / 2] >> ((n & 1) * 8) & 0xFF); return v < 24 || v > 108 ? 0 : (uint8_t)v; }
static void key_beep(core_t *c, int i) {   // goi khi bam phim vat ly i: nốt cua layer dang cho ra ma phim
  if (!kn_on(&c->cfg)) return;
  uint32_t m = core_active(c);
  for (int l = NLAYER - 1; l >= 0; l--) if (m >> l & 1) {
    if (c->cfg.w[W_KM + l * NKEYS + i] == KC_TRNS) continue;
    uint8_t n = kn_note(&c->cfg, l, i); if (n) { c->buz_mid = n; c->buz_ms = kn_ms(&c->cfg); }
    return;
  }
}
static void nokia_beep(core_t *c, int g) { if (nk_beep_on(&c->cfg)) { c->buz_mid = nk_note(&c->cfg, g); c->buz_ms = NK_BEEP_MS; } }
static void nokia_press(core_t *c, int g, uint32_t now) {   // v19: tieng bip phat ngay luc bam (chord_intercept), khong phat o day
  int len = (int)strlen(nk_tab(&c->cfg, g));
  char pc = c->nk_lc;                                   // ky tu truoc luot nay
  if (c->nk_key == g && now - c->nk_t < NOKIA_MS) { q_bs(c); c->nk_n = (c->nk_n + 1) % len; pc = c->nk_pc; }
  else { if (c->nk_key != g) c->nk_pg = (int8_t)c->nk_key; c->nk_key = g; c->nk_n = 0; c->nk_up = c->nk_cap; c->nk_cap = 0; c->nk_pc = pc; }
  char ch = nk_tab(&c->cfg, g)[c->nk_n];
#if NK_AUTOCAP
  if (c->nk_up && ch >= 'a' && ch <= 'z') ch = (char)(ch - 32);
  if (ch == ' ' && (pc == '.' || pc == '!' || pc == '?')) c->nk_cap = 1; else if (ch != ' ') c->nk_cap = 0;   // "0 " -> dau cach sau cau: viet hoa tiep
#endif
  q_ascii(c, ch); c->nk_lc = ch; c->nk_t = now;
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
    int g = k & 0xFF, len = (int)strlen(nk_tab(&c->cfg, g));
    char ch;
    if (c->rn_nk == g && now - c->rn_t < NOKIA_MS && c->rn_len) { c->rn_n = (c->rn_n + 1) % len; ch = nk_tab(&c->cfg, g)[c->rn_n]; c->rn_buf[c->rn_len - 1] = ch >= 'a' && ch <= 'z' ? ch - 32 : ch; }
    else if (c->rn_len < NAMELEN) { c->rn_nk = g; c->rn_n = 0; ch = nk_tab(&c->cfg, g)[0]; c->rn_buf[c->rn_len++] = ch >= 'a' && ch <= 'z' ? ch - 32 : ch; }
    c->rn_buf[c->rn_len] = 0; c->rn_t = now; nokia_beep(c, g); return;
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
static void tk_toggle(core_t *c, uint8_t kc) {   // dang giu -> nha; chua giu -> giu (day thi bo qua)
  int free_i = -1;
  for (int j = 0; j < NTK; j++) { if (c->tk[j] == kc) { c->tk[j] = 0; return; } if (!c->tk[j] && free_i < 0) free_i = j; }
  if (free_i >= 0) c->tk[free_i] = kc;
}
// ---------------------------------------------------------------- nhac cu (v13): am don + bai nhac
#define PN_DECAY_MS 700                     // che do tat dan: thoi gian tu am luong day den im
#define TUNE_GAP 18                         // ms nghi cuoi moi nốt cua bai (de nghe tach nốt)
static const uint16_t NOTE_HZ12[12] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494};   // C4..B4
static const uint16_t TEMPO_MS[5] = {160, 135, 115, 95, 80};                                          // ms moi 1/16 nốt
static uint32_t note_hz(int m) {
  if (m < 24) m = 24;
  if (m > 108) m = 108;
  int sh = m / 12 - 5; uint32_t f = NOTE_HZ12[m % 12];
  return sh >= 0 ? f << sh : f >> -sh;
}
static const uint8_t BZ_LVL[5] = {100, 75, 50, 25, 10};   // v18: muc am luong loa (%)
static void nt_push(core_t *c, uint8_t n) {
  if (c->nt_n >= 8) { memmove(c->nt_stk, c->nt_stk + 1, 7); c->nt_n = 7; }
  c->nt_stk[c->nt_n++] = n; c->nt_age = 0;
}
static void nt_remove(core_t *c, uint8_t n) {
  for (int j = c->nt_n - 1; j >= 0; j--) if (c->nt_stk[j] == n) { memmove(c->nt_stk + j, c->nt_stk + j + 1, (size_t)(c->nt_n - j - 1)); c->nt_n--; return; }
}
static void tune_stop(core_t *c) { c->tn_cur = 0; c->tn_note = 0; c->tn_left = 0; c->tn_gap = 0; }
static void tune_load(core_t *c) {          // nap nốt c->tn_i cua bai dang phat
  const tnote_t *t = &TUNE_TAB[c->tn_cur - 1].p[c->tn_i];
  uint16_t total = (uint16_t)(t->d * TEMPO_MS[c->tn_tempo]);
  if (!t->n) { c->tn_note = 0; c->tn_left = total; c->tn_gap = 0; }
  else { uint16_t g = total > 4 * TUNE_GAP ? TUNE_GAP : total / 4; c->tn_note = (uint8_t)(t->n + 12); c->tn_left = (uint16_t)(total - g); c->tn_gap = g; }   // bai dich len 1 quang tam
  c->nt_age = 0;
}
static void tune_start(core_t *c, int n) { c->tn_cur = (uint8_t)(n + 1); c->tn_sel = (uint8_t)(n + 1); c->tn_i = 0; tune_load(c); }
static void tune_step(core_t *c, int dir) {   // v20: bai truoc / ke tiep (vong tron), phat ngay; chua chon gi: ke = bai 1, truoc = bai cuoi
  int n = c->tn_sel ? ((int)c->tn_sel - 1 + dir + NTUNE) % NTUNE : (dir > 0 ? 0 : NTUNE - 1);
  tune_start(c, n);
}
// v20: nốt thu `off` tinh tu nốt dang phat cua bai dang phat (off am = nốt truoc). Tra -1 neu ngoai bai / khong co bai, 0 = nghi, con lai = nốt MIDI that su phat (da dich len 1 quang tam)
static int core_tune_note(const core_t *c, int off) {
  if (!c->tn_cur) return -1;
  int i = (int)c->tn_i + off; if (i < 0 || i >= (int)TUNE_TAB[c->tn_cur - 1].len) return -1;
  int n = TUNE_TAB[c->tn_cur - 1].p[i].n; return n ? n + 12 : 0;
}
// v20: ten nốt cho OLED, vd 72 -> "C5", 78 -> "F+5" (font OLED khong co dau #, dung + cho nốt thang). 0 = nghi -> "-"
static void note_name(int m, char *o) {
  static const char *const nn[12] = {"C", "C+", "D", "D+", "E", "F", "F+", "G", "G+", "A", "A+", "B"};
  if (m <= 0) { strcpy(o, "-"); return; }
  if (m < 12) m = 12;
  if (m > 131) m = 131;
  int oc = m / 12 - 1; snprintf(o, 6, "%s%d", nn[m % 12], oc);
}
#define W_STARTUP (W_OSB + 7)               // v14: bai khoi dong (word du phong cua khoi man hinh): 0 = tat, 1..NTUNE = bai tuong ung
static void core_boot_tune(core_t *c) { uint16_t v = c->cfg.w[W_STARTUP]; if (v >= 1 && v <= NTUNE) tune_start(c, v - 1); }   // goi 1 lan sau core_reset luc cap dien
static void tune_tick(core_t *c) {          // goi moi ms
  if (!c->tn_cur) return;
  if (c->tn_left > 1) { c->tn_left--; return; }
  if (c->tn_note && c->tn_gap) { c->tn_note = 0; c->tn_left = c->tn_gap; c->tn_gap = 0; return; }   // het nốt -> nghi ngan
  if (++c->tn_i >= TUNE_TAB[c->tn_cur - 1].len) { tune_stop(c); return; }
  tune_load(c);
}
static void piano_press(core_t *c, int i, kc_t k) {
  if (IS_NOTE(k)) {
    int m = (int)(k & 0x7F) + 12 * c->pn_oct + c->pn_tr; if (m < 24) m = 24; if (m > 108) m = 108;
    c->ntk[i] = (uint8_t)m; nt_push(c, (uint8_t)m);
  }
  else if (IS_TUNE(k)) { int n = TUNE_IDX(k); if (c->tn_cur == n + 1) tune_stop(c); else tune_start(c, n); }
  else switch (k) {
    case KC_OCTU: if (c->pn_oct < 3) c->pn_oct++; break;
    case KC_OCTD: if (c->pn_oct > -3) c->pn_oct--; break;
    case KC_TRU: if (c->pn_tr < 11) c->pn_tr++; break;
    case KC_TRD: if (c->pn_tr > -11) c->pn_tr--; break;
    case KC_PSTOP: c->nt_n = 0; memset(c->ntk, 0, sizeof c->ntk); break;   // tune_stop da goi o dau press_code
    case KC_PENV: c->pn_env ^= 1; break;
    case KC_PTIM: c->pn_tim = (uint8_t)((c->pn_tim + 1) % 3); break;
    case KC_PVIB: c->pn_vib ^= 1; break;
    case KC_PVOL: c->pn_vol = (uint8_t)((c->pn_vol + 1) % 3); break;
    case KC_BZMUTE: c->bz_mute ^= 1; break;
    case KC_BZVU: if (c->bz_lvl > 0) c->bz_lvl--; break;
    case KC_BZVD: if (c->bz_lvl < 4) c->bz_lvl++; break;
    case KC_DEMO: { int n = c->pn_demo; c->pn_demo = (uint8_t)((n + 1) % NTUNE); tune_start(c, n); } break;
    case KC_TMPD: if (c->tn_tempo > 0) c->tn_tempo--; break;
    case KC_TMPU: if (c->tn_tempo < 4) c->tn_tempo++; break;
    case KC_TPREV: tune_step(c, -1); break;
    case KC_TNEXT: tune_step(c, 1); break;
    case KC_TPLAY: if (c->tn_cur) tune_stop(c); else tune_start(c, c->tn_sel ? c->tn_sel - 1 : 0); break;
    case KC_TCYC: c->tn_tempo = (uint8_t)((c->tn_tempo + 1) % 5); break;
    default: break;
  }
  if (IS_BZCTL(k) && !c->bz_mute) { c->buz_mid = 84; c->buz_ms = NK_BEEP_MS; }   // bip thu de nghe muc am luong moi
}
// ---------------------------------------------------------------- chord (v19): 2 phim cung luc
// Morse: cham + gach cung luc = chot ky tu (nhu Xong); lam 2 lan lien tiep = Space.
// Nokia: 2 phim so bat ky cung luc = Space; lam 2 lan lien tiep = xoa 1 ky tu (xoa luon dau cach chord dau vua go).
// Phim Morse / Nokia duoc giu lai toi da CHORD_MS de xem co phim thu hai khong; khong co thi xu ly nhu phim don.
static inline bool chord_cls(kc_t k) { return IS_NKK(k) || k == KC_MDOT || k == KC_MDASH; }
static void chord_single(core_t *c, kc_t k, int i, uint32_t now) {
  if (IS_NKK(k)) { nokia_press(c, k & 0xFF, now); c->gh_i = (int8_t)i; c->gh_k = k; c->gh_mode = 0; }
  else {
    char sc[8]; memcpy(sc, c->mo_code, 8); uint8_t sl = c->mo_len;      // v19b: nho ma truoc phim nay, de hoan tac neu phim nay chi la phim giu
    if (morse_press(c, k == KC_MDASH)) {
      memcpy(c->ms_code, sc, 8); c->ms_len = sl; c->ms_ok = 1; c->mo_t = now; c->mo_auto = 0;
      c->gh_i = (int8_t)i; c->gh_k = k; c->gh_mode = 0;
    }
  }
}
static void chord_flush(core_t *c, uint32_t now) {
  if (!c->ch_k) return;
  kc_t k = c->ch_k; int i = c->ch_i; c->ch_k = 0; c->ch_i = -1;
  chord_single(c, k, i, now);
}
static void chord_fire(core_t *c, bool nokia, uint32_t now) {
  bool dbl = c->ch_n && now - c->ch_t < CHORD2_MS;
  c->ch_n = dbl ? 0 : 1; c->ch_t = now; c->gh_i = -1;
  if (nokia) {
    c->nk_key = -1; c->nk_pg = -1;               // het multitap: ky tu dang go duoc chot
    if (dbl) { if (q_room(c) >= 2) { q_bs(c); q_bs(c); } c->nk_lc = 0; c->nk_cap = 0; }   // xoa dau cach cua chord dau + 1 ky tu truoc do
    else { q_ascii(c, ' '); if (c->nk_lc == '.' || c->nk_lc == '!' || c->nk_lc == '?') c->nk_cap = NK_AUTOCAP; c->nk_lc = ' '; }
    if (nk_beep_on(&c->cfg)) { c->buz_mid = dbl ? NK_CHORD_DEL_NOTE : NK_CHORD_SP_NOTE; c->buz_ms = NK_BEEP_MS; }   // bip rieng: cao = Space, thap = xoa
  } else {
    morse_commit(c); c->mo_auto = 0;             // chot ky tu (giong phim Xong)
    if (dbl) { q_ascii(c, ' '); if (buz_enabled(&c->cfg)) { c->buz_mid = 0; c->buz_ms = buz_unit(&c->cfg); } }   // Space: 1 bip ngan
  }
}
// ---------------------------------------------------------------- v19b: giu 1 phim + bam phim kia (co thu tu)
// Morse: giu Cham + bam Gach = chot ky tu + Space | giu Gach + bam Cham = xoa (ky tu dang go do, neu khong co thi xoa ky tu truoc).
//        Phim giu chi la phim bo tro: ky hieu no vua them vao ma bi hoan tac. Giu nguyen phim giu, bam tiep nhieu lan = lap lai.
// Nokia: giu phim vua go + bam phim moi = Space (chu vua go o lai)
//        giu phim moi + bam phim vua go (truoc phim moi) = xoa 1 ky tu (chu cua phim giu bi huy, xoa them chu truoc do). Giu nguyen = lap lai.
static void morse_undo(core_t *c) {            // bo ky hieu cham / gach vua them: tra ma + chu hien tren may tinh ve nhu truoc
  if (!c->ms_ok) return;
  c->ms_ok = 0;
  char ch = c->ms_len ? morse_find(c->ms_code) : 0, h[24]; int hl = (c->ms_len && c->mo_hopt) ? morse_hint(c->ms_code, h) : (h[0] = 0, 0);
  int del = (c->mo_shown ? 1 : 0) + c->mo_hint;
  if (q_room(c) < del + (ch ? 1 : 0) + 2 * hl) return;
  for (int i = 0; i < del; i++) q_bs(c);
  if (ch) q_ascii(c, ch);
  for (int i = 0; i < hl; i++) q_ascii(c, h[i]);
  memcpy(c->mo_code, c->ms_code, 8); c->mo_len = c->ms_len; c->mo_shown = ch != 0; c->mo_hint = (uint8_t)hl;
}
static bool hold_gesture(core_t *c, int i, kc_t k, uint32_t now) {
  (void)now;
  if (c->gh_i >= 0 && !c->st[c->gh_i]) c->gh_i = -1;          // phim giu da nha
  if (c->gh_i < 0 || c->gh_i == i) return false;
  kc_t h = c->gh_k; bool nk = IS_NKK(k);
  if (nk != IS_NKK(h)) return false;                          // Morse lan Nokia: khong phai cu chi
  int mode, g = k & 0xFF;
  if (nk) {
    if (g == (h & 0xFF)) return false;                        // 2 phim cung nhom
    if (c->gh_mode == 2) { if (g != c->gh_g) { c->gh_i = -1; return false; } mode = 2; }
    else if (c->gh_mode == 1) mode = 1;
    else mode = (c->nk_pg >= 0 && g == c->nk_pg) ? 2 : 1;     // bam lai phim truoc phim giu = xoa, con lai = Space
  } else {
    if (k == h) return false;
    mode = h == KC_MDOT ? 1 : 2;                              // giu Cham = Space, giu Gach = xoa
  }
  bool first = c->gh_mode == 0;
  if (nk) {
    if (mode == 2) { if (q_room(c) < 2) return true; q_bs(c); if (first) q_bs(c); }   // lan dau: huy chu cua phim giu, xoa them chu truoc
    else { q_ascii(c, ' '); if (c->nk_lc == '.' || c->nk_lc == '!' || c->nk_lc == '?') c->nk_cap = NK_AUTOCAP; }
    if (mode == 2) { c->nk_lc = 0; c->nk_cap = 0; } else c->nk_lc = ' ';
    c->nk_key = -1; c->nk_pg = -1; c->gh_g = (int8_t)g;
    if (nk_beep_on(&c->cfg)) { c->buz_mid = mode == 2 ? NK_CHORD_DEL_NOTE : NK_CHORD_SP_NOTE; c->buz_ms = NK_BEEP_MS; }
  } else {
    if (first) morse_undo(c);
    if (mode == 1) { morse_commit(c); q_ascii(c, ' '); }
    else if (c->mo_len) {                                     // huy ky tu dang go
      int del = (c->mo_shown ? 1 : 0) + c->mo_hint; if (q_room(c) >= del) for (int j = 0; j < del; j++) q_bs(c);
      c->mo_len = 0; c->mo_code[0] = 0; c->mo_shown = 0; c->mo_hint = 0; c->ms_ok = 0;
    } else q_bs(c);                                           // khong co ky tu dang go: xoa ky tu truoc
    c->mo_auto = 0;
    if (buz_enabled(&c->cfg)) { c->buz_mid = 0; c->buz_ms = (uint16_t)(buz_unit(&c->cfg) * (mode == 1 ? 1 : 2)); }
  }
  c->gh_mode = (uint8_t)mode; c->ch_n = 0;
  return true;
}
// true = phim da duoc xu ly o day (dang cho hoac da thanh chord / cu chi), press_code khong lam gi them
static bool chord_intercept(core_t *c, int i, kc_t k, uint32_t now) {
  if (!chord_cls(k)) { chord_flush(c, now); c->mo_auto = 0; c->gh_i = -1; return false; }
  if (IS_NKK(k)) nokia_beep(c, k & 0xFF);          // bip nốt ngay khi bam, khong cho het CHORD_MS
  if (c->ch_k) {
    bool both_nk = IS_NKK(c->ch_k) && IS_NKK(k), both_mo = !IS_NKK(c->ch_k) && !IS_NKK(k) && c->ch_k != k;
    if (c->ch_i != i && c->st[c->ch_i] && now - c->ch_t0 <= CHORD_MS && (both_nk || both_mo)) {   // phim dau con dang giu = that su bam cung luc
      bool nokia = both_nk; c->ch_k = 0; c->ch_i = -1; chord_fire(c, nokia, now); return true;
    }
    chord_flush(c, now);
  }
  if (hold_gesture(c, i, k, now)) return true;      // v19b: phim da go truoc do con giu + phim nay = Space / xoa
  c->ch_k = k; c->ch_i = (int8_t)i; c->ch_t0 = now; return true;
}
// v19: Morse tu chot. Nghi >= 8 don vi cham (toi thieu 400 ms) sau net cuoi -> chot ky tu; nghi them (tong 2,5 lan) -> them Space.
static void morse_tick(core_t *c, uint32_t now) {
  if (!morse_auto_on(&c->cfg)) { c->mo_auto = 0; return; }
  uint32_t lt = 8u * buz_unit(&c->cfg); if (lt < 400) lt = 400;
  bool held = c->gh_i >= 0 && c->st[c->gh_i] && (c->gh_k == KC_MDOT || c->gh_k == KC_MDASH);   // v19b: dang giu phim Cham / Gach: chua tu chot (co the sap la cu chi giu + bam)
  if (c->mo_len && !held && now - c->mo_t >= lt) { morse_commit(c); c->mo_auto = 1; }
  else if (c->mo_auto && !c->mo_len && now - c->mo_t >= lt * 5 / 2) { c->mo_auto = 0; q_ascii(c, ' '); }
}
static void press_code(core_t *c, int i, kc_t k, uint32_t now) {
  if (!k || k == KC_TRNS) return;
  if (c->tn_cur && !IS_TUNE(k) && !IS_TEMPO(k) && !IS_TSEL(k)) tune_stop(c);   // bam phim khac khi dang phat bai -> dung bai
  if (IS_SEQ(k)) { seq_step(c, &k); if (!k) return; }
  if (c->rn_on) { rename_press(c, k, now); return; }
  if (c->latch_wait && IS_HOLDABLE(k) && !IS_CONS(k)) { c->latch_code = k; c->latch_wait = false; c->swallow[i] = 1; return; }
  if (c->latch_wait && IS_CONS(k)) { c->latch_wait = false; c->swallow[i] = 1; return; }
  if (chord_intercept(c, i, k, now)) return;
  if (!IS_NKK(k)) { c->nk_key = -1; c->nk_pg = -1; c->nk_lc = 0; if (k == KC_ENTER || k == KC_PENT) c->nk_cap = NK_AUTOCAP; }
  if (IS_HOLDABLE(k)) c->act[i] = k;
  else if (IS_MO(k)) { if ((k & 0xFF) < NLAYER) c->mol[i] = (int8_t)(k & 0xFF); }
  else if (IS_TL(k)) { int n = k & 0xFF; if (n == 0) c->toggled = 0; else if (n < NLAYER) c->toggled ^= 1u << n; }
  else if (k == KC_LYUP) layer_cycle(c, 1);
  else if (k == KC_LYDN) layer_cycle(c, -1);
  else if (k == KC_LATCH) { if (c->latch_code) c->latch_code = 0; else c->latch_wait = true; }
  else if (k >= KC_TGSPC && k <= KC_TGALT) c->tg ^= 1u << (k - KC_TGSPC);
  else if (IS_TK(k)) tk_toggle(c, (uint8_t)k);
  else if (k == KC_RENAME) { c->rn_on = true; c->rn_len = 0; c->rn_buf[0] = 0; c->rn_layer = (uint8_t)core_top_layer(c); c->rn_nk = -1; c->latch_wait = false; }
  else if (IS_MORSE(k)) {
    if (k == KC_MOK) { morse_commit(c); c->mo_auto = 0; } else morse_set_hint(c, !c->mo_hopt);   // cham / gach / nokia da xu ly o chord_intercept
  }
  else if (IS_MOUSE(k)) { if (k == KC_AIRT) c->air_tog = !c->air_tog; else if (k == KC_AIRCAL) c->cal_req = true; else c->mact[i] = (uint8_t)(k & 0xFF); }
  else if (IS_MACRO(k)) macro_begin(c, k & 0xFF);
  else if (IS_PIANO(k)) piano_press(c, i, k);
  else if (k == KC_BOOT) c->boot_req = true;
}
static void do_release(core_t *c, int i) {
  if (c->ntk[i]) { nt_remove(c, c->ntk[i]); c->ntk[i] = 0; }
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
  key_beep(c, i);
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
  if (c->buz_ms && !--c->buz_ms) c->buz_mid = 0;
  if (c->nt_age < 0xFFFF) c->nt_age++;
  tune_tick(c);
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
  if (c->ch_k && now - c->ch_t0 >= CHORD_MS) chord_flush(c, now);
  morse_tick(c, now);
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
  for (int j = 0; j < NTK; j++) if (c->tk[j]) rpt_add(r, &n, c->tk[j]);
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
// Coi: tan so (Hz, 0 = im) va bien do (0..256, 256 = xung 50%) hien tai. Nốt nhac (phim hoac bai) uu tien hon tieng bip Morse.
static inline uint8_t core_note(const core_t *c) { return c->tn_cur ? c->tn_note : (c->nt_n ? c->nt_stk[c->nt_n - 1] : 0); }
static uint32_t core_buz_hz(const core_t *c) {
  uint8_t n = core_note(c);
  if (!n) return c->buz_ms ? (c->buz_mid ? note_hz(c->buz_mid) : buz_freq(&c->cfg)) : 0;
  uint32_t hz = note_hz(n);
  if (c->pn_vib && c->nt_age > 150) {       // rung ~6 Hz, +-1.5%, sau 150 ms
    int ph = (int)((uint32_t)(c->nt_age - 150) * 1536u / 1000u) & 255, tri = (ph < 128 ? ph : 255 - ph) * 2 - 128;
    hz = (uint32_t)((int32_t)hz + (int32_t)hz * tri / 8500);
  }
  return hz;
}
static uint32_t core_buz_amp_raw(const core_t *c) {
  uint32_t vol = buz_vol(&c->cfg);
  if (!core_note(c)) return vol * 256 / 100;
  static const uint8_t volm[3] = {100, 50, 25}, tim[3] = {100, 50, 25};
  uint32_t a = vol * volm[c->pn_vol % 3] / 100 * 256 / 100 * tim[c->pn_tim % 3] / 100;   // 100% vol + xung 50% = 256
  if (c->pn_env) {
    if (c->nt_age >= PN_DECAY_MS) return 0;
    uint32_t r = PN_DECAY_MS - c->nt_age; a = a * r * r / ((uint32_t)PN_DECAY_MS * PN_DECAY_MS);
  }
  return a;
}
static uint32_t core_buz_amp(const core_t *c) { return c->bz_mute ? 0 : core_buz_amp_raw(c) * BZ_LVL[c->bz_lvl % 5] / 100; }   // v18: tat tieng / muc am luong loa ap dung cho moi am
static inline bool core_buz(const core_t *c) { return core_buz_hz(c) != 0 && core_buz_amp(c) != 0; }   // true = coi dang phai keu
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
  else if (IS_TK(k)) { char s[16]; core_label(c, k & 0xFF, s, sizeof s); ADD("TOG "); ADD(s); }
  else if (k == KC_LATCH) ADD("LATCH");
  else if (k == KC_TGSPC) ADD("TG SPACE"); else if (k == KC_TGSFT) ADD("TG SHIFT"); else if (k == KC_TGCTL) ADD("TG CTRL"); else if (k == KC_TGALT) ADD("TG ALT");
  else if (k == KC_RENAME) ADD("RENAME");
  else if (k == KC_AIR) ADD("AIR"); else if (k == KC_AIRT) ADD("AIR TG"); else if (k == KC_AIRCAL) ADD("AIR CAL"); else if (k == KC_WHUP) ADD("SCROLL UP"); else if (k == KC_WHDN) ADD("SCROLL DN");
  else if (k == KC_MB1) ADD("LMB"); else if (k == KC_MB2) ADD("RMB"); else if (k == KC_MB3) ADD("MMB");
  else if (k == KC_MDOT) ADD("DOT"); else if (k == KC_MDASH) ADD("DASH"); else if (k == KC_MOK) ADD("OK"); else if (k == KC_MHINT) ADD("HINT");
  else if (IS_NOTE(k)) {
    static const char *nn[12] = {"C", "C", "D", "D", "E", "F", "F", "G", "G", "A", "A", "B"}; static const char sh[12] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
    int m = (int)(k & 0x7F) + 12 * c->pn_oct + c->pn_tr; if (m < 24) m = 24; if (m > 108) m = 108;
    ADD(nn[m % 12]); if (sh[m % 12]) ADD("+"); d[0] = (char)('0' + m / 12 - 1); ADD(d);   // dau + = thang (font OLED khong co #)
  }
  else if (IS_TSEL(k)) {
    char b[16];
    switch (k) { case KC_TPREV: strcpy(b, "PREV SONG"); break; case KC_TNEXT: strcpy(b, "NEXT SONG"); break; case KC_TPLAY: strcpy(b, "PLAY STOP"); break; default: snprintf(b, sizeof b, "TEMPO %d", c->tn_tempo + 1); }
    ADD(b);
  }
  else if (IS_TUNE(k)) ADD(TUNE_TAB[TUNE_IDX(k)].name);
  else if (IS_PCTL(k) || IS_TEMPO(k)) {
    char b[16];
    switch (k) {
      case KC_OCTU: case KC_OCTD: snprintf(b, sizeof b, "OCT %+d", c->pn_oct); break;
      case KC_TRU: case KC_TRD: snprintf(b, sizeof b, "TRANS %+d", c->pn_tr); break;
      case KC_PSTOP: strcpy(b, "STOP"); break;
      case KC_PENV: strcpy(b, c->pn_env ? "PLUCK" : "SUSTAIN"); break;
      case KC_PTIM: snprintf(b, sizeof b, "TIMBRE %d", c->pn_tim + 1); break;
      case KC_PVIB: strcpy(b, c->pn_vib ? "VIB ON" : "VIB OFF"); break;
      case KC_PVOL: snprintf(b, sizeof b, "VOL %d", c->pn_vol == 0 ? 100 : c->pn_vol == 1 ? 50 : 25); break;
      case KC_DEMO: strcpy(b, "DEMO"); break;
      default: snprintf(b, sizeof b, "TEMPO %d", c->tn_tempo + 1); break;
    }
    ADD(b);
  }
  else if (IS_BZCTL(k)) { char b[16]; if (k == KC_BZMUTE) strcpy(b, c->bz_mute ? "MUTE ON" : "MUTE OFF"); else snprintf(b, sizeof b, "BZ VOL %d", BZ_LVL[c->bz_lvl % 5]); ADD(b); }
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
// Con lan: tra +1 (cuon len), -1 (cuon xuong) hoac 0 cho moi nac can gui. Nac dau gui ngay khi bam, giu qua WHEEL_DELAY_MS thi lap moi WHEEL_REPEAT_MS.
#ifndef WHEEL_DELAY_MS
#define WHEEL_DELAY_MS 350
#define WHEEL_REPEAT_MS 50
#endif
static int core_wheel(core_t *c, uint32_t now) {
  int d = 0;
  for (int i = 0; i < NKEYS; i++) { if (c->mact[i] == (KC_WHUP & 0xFF)) d = 1; else if (c->mact[i] == (KC_WHDN & 0xFF)) d = -1; }
  if (d == 0) { c->wh_dir = 0; return 0; }
  if (d != c->wh_dir) { c->wh_dir = (int8_t)d; c->wh_due = now + WHEEL_DELAY_MS; return d; }
  if ((int32_t)(now - c->wh_due) >= 0) { c->wh_due = now + WHEEL_REPEAT_MS; return d; }
  return 0;
}
static bool core_air_on(const core_t *c) {
  if (c->air_tog || (c->cfg.w[W_AMB + 4] & 8)) return true;
  for (int i = 0; i < NKEYS; i++) if (c->mact[i] == (KC_AIR & 0xFF)) return true;
  return false;
}
