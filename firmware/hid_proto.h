#pragma once
// Giao thuc cau hinh Numpad30 qua Raw HID (v2): bao cao 32 byte, report ID 0.
// Dung chung firmware + test tren may tinh (dinh nghia HP_IMPL de lay phan xu ly).
// Lenh (byte 0) / tra loi:
//   1  INFO        -> [1, ver, NLAYER, NKEYS, NMACROS, 255, NAME_MAX, flags, NTD, NCB, NLOGO, LED_N, LED_NMODES, sizeof(ext) lo, hi]
//   2  GET_KEYS    <- [2, layer, off, n]           -> [2, layer, off, n, k0 lo, k0 hi, ...]      (n <= 14)
//   3  SET_KEYS    <- [3, layer, off, n, data...]  -> [3, layer, off, n]
//   4  GET_NAME    <- [4, layer]                   -> [4, layer, len, chars...]
//   5  SET_NAME    <- [5, layer, len, chars...]    -> [5, layer, len]
//   6  GET_COLORS  <- [6]                          -> [6, r0,g0,b0, ... (NLAYER mau)]     mau cua den WS2812 theo layer
//   7  SET_COLORS  <- [7, r0,g0,b0, ...]           -> [7]
//   8  GET_MACRO   <- [8, idx, off, n]             -> [8, idx, off, n, bytes...]            (n <= 28, off+n <= 256)
//   9  SET_MACRO   <- [9, idx, off, n, bytes...]   -> [9, idx, off, n]                      o macro = [dai][buoc...]
//   10 SAVE        <- [10]                         -> [10, 1]   (ghi flash sau khi tra loi)
//   11 GET_CFG     <- [11]                         -> [11, ss0..ss3, bright]   (screensaver ms, do sang WS2812)
//   12 SET_CFG     <- [12, ss0..ss3, bright]       -> [12]
//   13 DEFAULTS    <- [13]                         -> [13]      (nap lai mac dinh vao RAM, chua ghi flash)
//   14 GET_EXT     <- [14, off lo, off hi, n]      -> [14, off lo, off hi, n, bytes...]     doc khoi ext_t (n <= 28)
//   15 SET_EXT     <- [15, off lo, off hi, n, bytes...] -> [15, off lo, off hi, n]
//   0xEE loi       -> [0xEE, cmd, ma loi]  (1 = layer/offset, 2 = macro, 3 = ten, 4 = ext, 9 = lenh la)
// Buoc macro: 1 tap kc16 | 2 nhan kc16 | 3 nha kc16 | 4 nghi ms16 | 5 go chuoi (n, ky tu...)
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "core.h"
#include "ledfx.h"

#define HP_PKT 32
#define HP_VER 2
#define HP_NAME_MAX 12
#define HP_MACRO_MAX 255

enum {
  HP_INFO = 1, HP_GET_KEYS = 2, HP_SET_KEYS = 3,
  HP_GET_NAME = 4, HP_SET_NAME = 5,
  HP_GET_COLORS = 6, HP_SET_COLORS = 7,
  HP_GET_MACRO = 8, HP_SET_MACRO = 9,
  HP_SAVE = 10, HP_GET_CFG = 11, HP_SET_CFG = 12,
  HP_DEFAULTS = 13, HP_GET_EXT = 14, HP_SET_EXT = 15,
  HP_ERR = 0xEE
};

// Khoi cau hinh mo rong (toan so nguyen, sap xep de khong can packed). Offset (app Studio dung cung bang nay):
//   0 td[4][3] u16 | 24 cb_r[4] u16 | 32 lg_ms u16 | 34 cb_a[4] | 38 cb_b[4] | 42 nlayer | 43 led_mode | 44 led_speed
//   45 led_bri | 46 saver_mode (0 tat, 1 logo) | 47 lg_n | 48 greet[28] | 76 logo[4][512] (page SSD1306: (y>>3)*128 + x, bit y&7)
typedef struct {
  uint16_t td[NTD][3];
  uint16_t cb_r[NCB];
  uint16_t lg_ms;
  uint8_t cb_a[NCB], cb_b[NCB];
  uint8_t nlayer, led_mode, led_speed, led_bri, saver_mode, lg_n;
  char greet[28];
  uint8_t logo[NLOGO][512];
} ext_t;
_Static_assert(sizeof(ext_t) == 76 + NLOGO * 512, "ext_t layout");
_Static_assert(offsetof(ext_t, cb_a) == 34 && offsetof(ext_t, nlayer) == 42 && offsetof(ext_t, greet) == 48 && offsetof(ext_t, logo) == 76, "ext_t offsets");

// Toan bo cau hinh song cua thiet bi (RAM) + co yeu cau ghi flash / nap mac dinh.
typedef struct {
  kc_t km[NLAYER][NKEYS];
  char names[NLAYER][HP_NAME_MAX + 1];
  uint8_t colors[NLAYER][3];
  uint8_t macs[NMACROS][MSLOT];
  uint32_t ss_ms;
  uint8_t bright;
  ext_t ext;
  bool save_req, defaults_req;
} cfg_store_t;

void hp_handle(cfg_store_t *s, const uint8_t *req, uint8_t *rep);
void ext_sanitize(ext_t *e);

#ifdef HP_IMPL
void ext_sanitize(ext_t *e) {
  if (e->nlayer < 1 || e->nlayer > NLAYER) e->nlayer = DEF_NLAYER;
  if (e->led_mode >= LED_NMODES) e->led_mode = 0;
  if (e->saver_mode > 1) e->saver_mode = 0;
  if (e->lg_n > NLOGO) e->lg_n = NLOGO;
  if (e->lg_ms < 30) e->lg_ms = 30;
  e->greet[sizeof e->greet - 1] = 0;
}
static void hp_err(uint8_t *rep, uint8_t cmd, uint8_t code) {
  memset(rep, 0, HP_PKT); rep[0] = HP_ERR; rep[1] = cmd; rep[2] = code;
}
void hp_handle(cfg_store_t *s, const uint8_t *q, uint8_t *rep) {
  memset(rep, 0, HP_PKT);
  switch (q[0]) {
  case HP_INFO:
    rep[0] = HP_INFO; rep[1] = HP_VER; rep[2] = NLAYER; rep[3] = NKEYS; rep[4] = NMACROS; rep[5] = HP_MACRO_MAX;
    rep[6] = HP_NAME_MAX; rep[7] = 0x1F; rep[8] = NTD; rep[9] = NCB; rep[10] = NLOGO; rep[11] = LED_N; rep[12] = LED_NMODES;
    rep[13] = (uint8_t)sizeof(ext_t); rep[14] = (uint8_t)(sizeof(ext_t) >> 8);
    break;
  case HP_GET_KEYS: {
    uint8_t l = q[1], off = q[2], n = q[3];
    if (l >= NLAYER || n == 0 || n > 14 || (int)off + n > NKEYS) { hp_err(rep, q[0], 1); break; }
    rep[0] = HP_GET_KEYS; rep[1] = l; rep[2] = off; rep[3] = n;
    for (uint8_t i = 0; i < n; i++) {
      kc_t k = s->km[l][off + i];
      rep[4 + 2 * i] = (uint8_t)k; rep[5 + 2 * i] = (uint8_t)(k >> 8);
    }
    break; }
  case HP_SET_KEYS: {
    uint8_t l = q[1], off = q[2], n = q[3];
    if (l >= NLAYER || n == 0 || n > 14 || (int)off + n > NKEYS) { hp_err(rep, q[0], 1); break; }
    for (uint8_t i = 0; i < n; i++) s->km[l][off + i] = (kc_t)(q[4 + 2 * i] | (q[5 + 2 * i] << 8));
    rep[0] = HP_SET_KEYS; rep[1] = l; rep[2] = off; rep[3] = n;
    break; }
  case HP_GET_NAME: {
    uint8_t l = q[1];
    if (l >= NLAYER) { hp_err(rep, q[0], 1); break; }
    uint8_t n = 0; while (n < HP_NAME_MAX && s->names[l][n]) n++;
    rep[0] = HP_GET_NAME; rep[1] = l; rep[2] = n;
    memcpy(rep + 3, s->names[l], n);
    break; }
  case HP_SET_NAME: {
    uint8_t l = q[1], n = q[2];
    if (l >= NLAYER || n > HP_NAME_MAX) { hp_err(rep, q[0], 3); break; }
    memset(s->names[l], 0, HP_NAME_MAX + 1);
    memcpy(s->names[l], q + 3, n); s->names[l][n] = 0;
    rep[0] = HP_SET_NAME; rep[1] = l; rep[2] = n;
    break; }
  case HP_GET_COLORS:
    rep[0] = HP_GET_COLORS; memcpy(rep + 1, s->colors, NLAYER * 3);
    break;
  case HP_SET_COLORS:
    memcpy(s->colors, q + 1, NLAYER * 3);
    rep[0] = HP_SET_COLORS;
    break;
  case HP_GET_MACRO: {
    uint8_t i = q[1], off = q[2], n = q[3];
    if (i >= NMACROS || n == 0 || n > 28 || (int)off + n > MSLOT) { hp_err(rep, q[0], 2); break; }
    rep[0] = HP_GET_MACRO; rep[1] = i; rep[2] = off; rep[3] = n;
    memcpy(rep + 4, s->macs[i] + off, n);
    break; }
  case HP_SET_MACRO: {
    uint8_t i = q[1], off = q[2], n = q[3];
    if (i >= NMACROS || n > 28 || (int)off + n > MSLOT) { hp_err(rep, q[0], 2); break; }
    memcpy(s->macs[i] + off, q + 4, n);
    rep[0] = HP_SET_MACRO; rep[1] = i; rep[2] = off; rep[3] = n;
    break; }
  case HP_SAVE:
    s->save_req = true; rep[0] = HP_SAVE; rep[1] = 1;
    break;
  case HP_GET_CFG:
    rep[0] = HP_GET_CFG;
    rep[1] = (uint8_t)s->ss_ms; rep[2] = (uint8_t)(s->ss_ms >> 8);
    rep[3] = (uint8_t)(s->ss_ms >> 16); rep[4] = (uint8_t)(s->ss_ms >> 24);
    rep[5] = s->bright;
    break;
  case HP_SET_CFG:
    s->ss_ms = (uint32_t)q[1] | ((uint32_t)q[2] << 8) | ((uint32_t)q[3] << 16) | ((uint32_t)q[4] << 24);
    s->bright = q[5];
    rep[0] = HP_SET_CFG;
    break;
  case HP_DEFAULTS:
    s->defaults_req = true; rep[0] = HP_DEFAULTS;
    break;
  case HP_GET_EXT: case HP_SET_EXT: {
    unsigned off = q[1] | (q[2] << 8), n = q[3];
    if (n > 28 || off + n > sizeof(ext_t)) { hp_err(rep, q[0], 4); break; }
    rep[0] = q[0]; rep[1] = q[1]; rep[2] = q[2]; rep[3] = (uint8_t)n;
    if (q[0] == HP_GET_EXT) memcpy(rep + 4, (const uint8_t *)&s->ext + off, n);
    else { memcpy((uint8_t *)&s->ext + off, q + 4, n); ext_sanitize(&s->ext); }
    break; }
  default:
    hp_err(rep, q[0], 9);
    break;
  }
}
#endif
