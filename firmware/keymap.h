#pragma once
// Keymap Numpad30. Bo cuc dien (hang R0..R4 x cot C0..C6) theo so do ma tran 5x7:
//   R0: M1  M2   M3    Num  /   *  -
//   R1: M4  M5   M6    7    8   9  +
//   R2: M7  M8   M9    4    5   6  (trong)
//   R3: -   Up   -     1    2   3  Enter
//   R4: Left Down Right 0   -   .  -
// Phim "Num": tap = NumLock, giu = vao layer FN (LT).
#include "core.h"
#define __ KC_NO
#define ___ KC_TRNS
#define NUM_LAYER_NAMES { "NUM", "FN", "MACRO" }

static const kc_t keymap[NLAYER][NKEYS] = {
  // ---- Layer 0: NUM (numpad + phim tat) ----
  { C_(KC_C), C_(KC_V), C_(KC_X), LT(1, KC_NUMLOCK), KC_PSLS, KC_PAST, KC_PMNS,
    C_(KC_Z), C_(KC_Y), C_(KC_A), KC_P7, KC_P8, KC_P9, KC_PPLS,
    C_(KC_S), A_(KC_TAB), G_(KC_D), KC_P4, KC_P5, KC_P6, __,
    __, KC_UP, __, KC_P1, KC_P2, KC_P3, KC_PENT,
    KC_LEFT, KC_DOWN, KC_RIGHT, KC_P0, __, KC_PDOT, __ },
  // ---- Layer 1: FN (giu phim Num): F1-F9, media, dieu huong ----
  { KC_F1, KC_F2, KC_F3, ___, CONS(CN_MUTE), CONS(CN_VOLD), CONS(CN_VOLU),
    KC_F4, KC_F5, KC_F6, CONS(CN_PREV), CONS(CN_PLAY), CONS(CN_NEXT), KC_BSPC,
    KC_F7, KC_F8, KC_F9, KC_HOME, KC_PGUP, KC_PGDN, __,
    __, KC_ESC, __, KC_END, KC_INS, KC_DEL, ___,
    KC_TAB, KC_SPACE, KC_ENTER, TG(2), __, KC_BOOT, __ },
  // ---- Layer 2: MACRO (bat/tat bang TG(2) o layer FN, phim 0): M1..M9 go chuoi ky tu ----
  { MACRO(0), MACRO(1), MACRO(2), ___, ___, ___, ___,
    MACRO(3), MACRO(4), MACRO(5), ___, ___, ___, ___,
    MACRO(6), MACRO(7), MACRO(8), ___, ___, ___, ___,
    ___, ___, ___, ___, ___, ___, ___,
    ___, ___, ___, ___, ___, ___, ___ },
};

// Chuoi macro (chi ASCII; Unikey/Telex dang bat se lam hong chu co dau). Sua tuy y.
static const macro_t macros[] = {
  { "Hello!" }, { "Xin chao" }, { "Cam on ban" },
  { "user@example.com" }, { "https://example.com" }, { "git status\n" },
  { "git add -A\n" }, { "git commit -m \"\"" }, { "git push\n" },
};
#define NMACROS ((int)(sizeof macros / sizeof macros[0]))
