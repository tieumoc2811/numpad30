#pragma once
// Giao thuc cau hinh Numpad30 qua Raw HID: bao cao 32 byte, report ID 0.
// Dung chung firmware + test tren may tinh (dinh nghia HP_IMPL de lay phan xu ly).
// Lenh (byte 0) / tra loi:
//   1  INFO        -> [1, ver, NLAYER, NKEYS, NMACROS, MACRO_MAX, NAME_MAX, flags]
//   2  GET_KEYS    <- [2, layer, off, n]           -> [2, layer, off, n, k0 lo, k0 hi, ...]
//   3  SET_KEYS    <- [3, layer, off, n, data...]  -> [3, layer, off, n]
//   4  GET_NAME    <- [4, layer]                   -> [4, layer, len, chars...]
//   5  SET_NAME    <- [5, layer, len, chars...]    -> [5, layer, len]
//   6  GET_COLORS  <- [6]                          -> [6, r0,g0,b0, r1,g1,b1, r2,g2,b2]
//   7  SET_COLORS  <- [7, r0,g0,b0, ...]           -> [7]
//   8  GET_MACRO   <- [8, idx, off, n]             -> [8, idx, off, n, chars...]
//   9  SET_MACRO   <- [9, idx, off, n, chars...]   -> [9, idx, off, n]
//   10 SAVE        <- [10]                         -> [10, 1]   (ghi flash, xu ly o main loop)
//   11 GET_CFG     <- [11]                         -> [11, ss0..ss3, bright]
//   12 SET_CFG     <- [12, ss0..ss3, bright]       -> [12]
//   13 GET_TD     <- [13, n]                      -> [13, n, tap lo,hi, giu lo,hi, doi lo,hi]
//   14 SET_TD     <- [14, n, tap lo,hi, giu lo,hi, doi lo,hi] -> [14, n]
//   15 GET_COMBO  <- [15, n]                      -> [15, n, A lo,hi, B lo,hi, ketqua lo,hi]
//   16 SET_COMBO  <- [16, n, A lo,hi, B lo,hi, ketqua lo,hi]    -> [16, n]
//   INFO bo sung: rep[8] = NTD, rep[9] = NCOMBO
//   0xEE loi       -> [0xEE, cmd, ma loi]  (1 = layer/offset, 2 = macro, 3 = ten, 4 = chi so tap dance/combo)
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "core.h"

#define HP_PKT 32
#define HP_VER 2
#define HP_NAME_MAX 12
#define HP_MACRO_MAX 255

#ifndef NMACROS
#define NMACROS 9
#endif

enum {
  HP_INFO = 1, HP_GET_KEYS = 2, HP_SET_KEYS = 3,
  HP_GET_NAME = 4, HP_SET_NAME = 5,
  HP_GET_COLORS = 6, HP_SET_COLORS = 7,
  HP_GET_MACRO = 8, HP_SET_MACRO = 9,
  HP_SAVE = 10, HP_GET_CFG = 11, HP_SET_CFG = 12,
  HP_GET_TD = 13, HP_SET_TD = 14, HP_GET_COMBO = 15, HP_SET_COMBO = 16,
  HP_ERR = 0xEE
};

// Toan bo cau hinh song cua thiet bi (RAM) + co yeu cau ghi flash.
typedef struct {
  kc_t km[NLAYER][NKEYS];
  char names[NLAYER][HP_NAME_MAX + 1];
  uint8_t colors[NLAYER][3];
  char macs[NMACROS][HP_MACRO_MAX + 1];
  uint32_t ss_ms;
  uint8_t bright;
  core_ext_t ext;                 // tap dance + combo
  bool save_req;
} cfg_store_t;

void hp_handle(cfg_store_t *s, const uint8_t *req, uint8_t *rep);

#ifdef HP_IMPL
static void hp_err(uint8_t *rep, uint8_t cmd, uint8_t code) {
  memset(rep, 0, HP_PKT); rep[0] = HP_ERR; rep[1] = cmd; rep[2] = code;
}
void hp_handle(cfg_store_t *s, const uint8_t *q, uint8_t *rep) {
  memset(rep, 0, HP_PKT);
  switch (q[0]) {
  case HP_INFO:
    rep[0] = HP_INFO; rep[1] = HP_VER; rep[2] = NLAYER; rep[3] = NKEYS;
    rep[4] = NMACROS; rep[5] = HP_MACRO_MAX; rep[6] = HP_NAME_MAX; rep[7] = 0x07; rep[8] = NTD; rep[9] = NCOMBO;
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
    if (i >= NMACROS || n == 0 || n > 28 || (int)off >= HP_MACRO_MAX + 1 || (int)off + n > HP_MACRO_MAX + 1) { hp_err(rep, q[0], 2); break; }
    rep[0] = HP_GET_MACRO; rep[1] = i; rep[2] = off; rep[3] = n;
    memcpy(rep + 4, s->macs[i] + off, n);       // co the doc ca byte NUL cuoi chuoi
    break; }
  case HP_SET_MACRO: {
    uint8_t i = q[1], off = q[2], n = q[3];
    if (i >= NMACROS || n > 28 || (int)off + n > HP_MACRO_MAX) { hp_err(rep, q[0], 2); break; }
    memcpy(s->macs[i] + off, q + 4, n);
    s->macs[i][off + n] = 0;                    // ket thuc chuoi ngay sau phan vua ghi
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
  case HP_GET_TD: case HP_GET_COMBO: {
    uint8_t n = q[1], td = q[0] == HP_GET_TD;
    if (n >= (td ? NTD : NCOMBO)) { hp_err(rep, q[0], 4); break; }
    const kc_t *v = td ? s->ext.td[n] : s->ext.cb[n];
    rep[0] = q[0]; rep[1] = n;
    for (int i = 0; i < 3; i++) { rep[2 + 2 * i] = (uint8_t)v[i]; rep[3 + 2 * i] = (uint8_t)(v[i] >> 8); }
    break; }
  case HP_SET_TD: case HP_SET_COMBO: {
    uint8_t n = q[1], td = q[0] == HP_SET_TD;
    if (n >= (td ? NTD : NCOMBO)) { hp_err(rep, q[0], 4); break; }
    kc_t *v = td ? s->ext.td[n] : s->ext.cb[n];
    for (int i = 0; i < 3; i++) v[i] = (kc_t)(q[2 + 2 * i] | (q[3 + 2 * i] << 8));
    rep[0] = q[0]; rep[1] = n;
    break; }
  default:
    hp_err(rep, q[0], 9);
    break;
  }
}
#endif
