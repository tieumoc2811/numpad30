#pragma once
// Cau hinh MAC DINH (lan dau chua co flash, hoac app gui lenh 4 "ve mac dinh"). Sau do app sua tat ca bang giao thuc HID.
// Bo cuc dien (hang R0..R4 x cot C0..C6) theo so do ma tran 5x7:
//   R0: M1  M2   M3    Num  /   *  -
//   R1: M4  M5   M6    7    8   9  +
//   R2: M7  M8   M9    4    5   6  (trong)
//   R3: -   Up   -     1    2   3  Enter
//   R4: Left Down Right 0   -   .  -
// Phim "Num": tap = NumLock, giu = vao layer FN (LT). Cac o trong luon = 0 o moi layer.
#include "core.h"
#define __ KC_NO
#define ___ KC_TRNS
#define NUM_DEFAULT_LAYERS 3

static const kc_t def_layers[NUM_DEFAULT_LAYERS][NKEYS] = {
  // ---- Layer 0: NUM (numpad + phim tat) ----
  { C_(KC_C), C_(KC_V), C_(KC_X), LT(1, KC_NUMLOCK), KC_PSLS, KC_PAST, KC_PMNS,
    C_(KC_Z), C_(KC_Y), C_(KC_A), KC_P7, KC_P8, KC_P9, KC_PPLS,
    C_(KC_S), A_(KC_TAB), G_(KC_D), KC_P4, KC_P5, KC_P6, __,
    __, KC_UP, __, KC_P1, KC_P2, KC_P3, KC_PENT,
    KC_LEFT, KC_DOWN, KC_RIGHT, KC_P0, __, KC_PDOT, __ },
  // ---- Layer 1: FN (giu phim Num): F1-F9, media, dieu huong; phim 0 bat/tat layer MACRO; phim . = BOOTSEL ----
  { KC_F1, KC_F2, KC_F3, ___, CONS(CN_MUTE), CONS(CN_VOLD), CONS(CN_VOLU),
    KC_F4, KC_F5, KC_F6, CONS(CN_PREV), CONS(CN_PLAY), CONS(CN_NEXT), KC_BSPC,
    KC_F7, KC_F8, KC_F9, KC_HOME, KC_PGUP, KC_PGDN, __,
    __, KC_ESC, __, KC_END, KC_INS, KC_DEL, ___,
    KC_TAB, KC_SPACE, KC_ENTER, TL(2), __, KC_BOOT, __ },
  // ---- Layer 2: MACRO (bat/tat bang TL(2) o layer FN, phim 0): M1..M9 ----
  { MACRO(0), MACRO(1), MACRO(2), ___, ___, ___, ___,
    MACRO(3), MACRO(4), MACRO(5), ___, ___, ___, ___,
    MACRO(6), MACRO(7), MACRO(8), ___, ___, ___, __,
    __, ___, __, ___, ___, ___, ___,
    ___, ___, ___, ___, __, ___, __ },
};
#undef __
#undef ___

// o trong cua ma tran 5x7 (khong co cong tac): R2C6, R3C0, R3C2, R4C4, R4C6
static bool slot_valid(int i) { return i != 20 && i != 21 && i != 23 && i != 32 && i != 34; }

// Macro mac dinh: ten (toi da 12) + chuoi ASCII; '\n' o cuoi = them buoc nhan Enter (o macro cua app khong luu duoc ky tu xuong dong)
typedef struct { const char *name, *text; bool enter; } defmac_t;
static const defmac_t def_macros[] = {
  {"Hello", "Hello!", false}, {"Xin chao", "Xin chao", false}, {"Cam on ban", "Cam on ban", false},
  {"Email", "user@example.com", false}, {"URL", "https://example.com", false}, {"git status", "git status", true},
  {"git add", "git add -A", true}, {"git commit", "git commit -m \"\"", false}, {"git push", "git push", true},
};
static void macro_slot_set(uint8_t *s, const char *name, const char *text, bool enter) {
  memset(s, 0, MSLOT);
  s[0] = 1;
  size_t nl = strlen(name); if (nl > 12) nl = 12;
  s[3] = (uint8_t)nl; memcpy(s + 4, name, nl);
  uint8_t *p = s + 18; size_t tl = strlen(text); if (tl > 64) tl = 64;
  *p++ = 5; *p++ = (uint8_t)tl; memcpy(p, text, tl); p += tl;
  if (enter) { *p++ = 1; *p++ = KC_ENTER; *p++ = 0; }
  uint16_t len = (uint16_t)(p - (s + 18)); s[16] = len & 255; s[17] = len >> 8;
}

static void cfg_defaults(cfg_t *g) {
  memset(g, 0, sizeof *g);
  for (int l = 0; l < NLAYER; l++) for (int i = 0; i < NKEYS; i++)
    g->w[W_KM + l * NKEYS + i] = !slot_valid(i) ? KC_NO : l < NUM_DEFAULT_LAYERS ? def_layers[l][i] : KC_TRNS;
  g->w[W_BT + 0] = KC_SPACE; g->w[W_BT + 1] = KC_LSFT; g->w[W_BT + 2] = KC_ENTER;   // TD0: cham = Space, giu = Shift, cham 2 lan = Enter
  g->w[W_NLI] = NUM_DEFAULT_LAYERS;
  static const uint16_t ld[W_NLD] = {0, 128, 200, 210, 255, 1, 0, 24, 2};   // [mode, toc do, sang, hue, sat, over, obMode, obSang, huong OLED]
  for (int i = 0; i < W_NLD; i++) g->w[W_LDB + i] = ld[i];
  for (int l = 0; l < NLAYER; l++) g->w[W_LRB + l] = 4;
  static const uint16_t os[W_NOS] = {300, 5, 1, 0, 1, 0, 120, 0};
  for (int i = 0; i < W_NOS; i++) g->w[W_OSB + i] = os[i];
  for (int m = 0; m < (int)(sizeof def_macros / sizeof def_macros[0]); m++) macro_slot_set(g->mac[m], def_macros[m].name, def_macros[m].text, def_macros[m].enter);
  const char *gr = "HELLO NUMPAD30", *sv = "NUMPAD30";
  g->od[16] = (uint8_t)strlen(gr); memcpy(g->od + 17, gr, strlen(gr));
  g->od[80] = (uint8_t)strlen(sv); memcpy(g->od + 81, sv, strlen(sv));
  strcpy(g->lname[0], "NUM"); strcpy(g->lname[1], "FN"); strcpy(g->lname[2], "MACRO");
}
