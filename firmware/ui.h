#pragma once
// Man hinh OLED (thuan C, ve vao framebuffer SSD1306 dang page): man hinh lam viec, doi ten layer, LATCH, loi chao, logo dong,
// screensaver, xoay man hinh. Khong phu thuoc phan cung de kiem thu tren PC.
#include "proto.h"
#include "font.h"

#define OLED_PAGES (OLED_H / 8)
#define FB_SIZE (128 * OLED_PAGES)
#define UI_FRAME_MS 40
enum { UI_NORMAL, UI_GREET, UI_SAVER, UI_LOGO };

typedef struct {
  uint8_t fb[FB_SIZE];
  int st; uint32_t t0, idle0, last_draw, seen_touch;
  bool pv;                // che do xem thu (khong tu thoat theo thoi gian)
  bool blank;             // man hinh phai tat (screensaver kieu "Tat man hinh")
  bool dirty;
  int rot;                // huong hieu dung 0..3 (da cong OLED_ROTATE)
  char shown[24];
  uint32_t sig;
} ui_t;

static inline uint16_t ui_osw(const core_t *c, int i) { return c->cfg.w[W_OSB + i]; }
static inline int ui_clamp(int v, int a, int b) { return v < a ? a : v > b ? b : v; }
static const char UI_FONT_OK[] = " !()*+,-./0123456789:=?ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static void ui_text_get(const core_t *c, int at, char *o) {   // doc chuoi trong header du lieu OLED (loc ky tu theo font)
  int n = c->cfg.od[at]; if (n > 63) n = 0; int w = 0;
  for (int i = 0; i < n; i++) { char ch = (char)c->cfg.od[at + 1 + i]; if (ch >= 'a' && ch <= 'z') ch -= 32; o[w++] = strchr(UI_FONT_OK, ch) ? ch : ' '; }
  o[w] = 0;
}
static void ui_px(ui_t *u, int x, int y, bool on) {   // toa do logic -> vat ly theo huong
  int lw = (u->rot & 1) ? OLED_H : 128, lh = (u->rot & 1) ? 128 : OLED_H;
  if (x < 0 || y < 0 || x >= lw || y >= lh) return;
  int px, py;
  switch (u->rot & 3) {
    case 0: px = x; py = y; break;
    case 1: px = y; py = OLED_H - 1 - x; break;
    case 2: px = 127 - x; py = OLED_H - 1 - y; break;
    default: px = 127 - y; py = x; break;
  }
  if (on) u->fb[(py >> 3) * 128 + px] |= 1 << (py & 7); else u->fb[(py >> 3) * 128 + px] &= ~(1 << (py & 7));
}
static void ui_box(ui_t *u, int x, int y, int w, int h, bool on) { for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) ui_px(u, x + i, y + j, on); }
static int ui_tw(const char *t, int s) { int n = (int)strlen(t); return n ? n * 6 * s - s : 0; }
static void ui_txt(ui_t *u, int x, int y, int s, const char *t, bool on) {
  for (; *t; t++, x += 6 * s) {
    char ch = *t >= 'a' && *t <= 'z' ? *t - 32 : *t; const char *g = strchr(FC, ch); int i = g ? (int)(g - FC) : 0;
    for (int c = 0; c < 5; c++) for (int r = 0; r < 7; r++) if (FD[i * 5 + c] >> r & 1) ui_box(u, x + c * s, y + r * s, s, s, on);
  }
}
static void ui_txt_wrap(ui_t *u, int x, int y, int s, const char *t, int cpl, int maxl) {   // xuong dong cho man hinh doc
  char ln[16]; int n = (int)strlen(t);
  for (int l = 0; l < maxl && l * cpl < n; l++) {
    int k = n - l * cpl; if (k > cpl) k = cpl; memcpy(ln, t + l * cpl, k); ln[k] = 0;
    ui_txt(u, x, y + l * 8 * s + l * s, s, ln, true);
  }
}
static void ui_frame(ui_t *u, const core_t *c, int i) {   // logo 128 x OLED_H (dang page cua SSD1306), luon ve ngang
  const uint8_t *f = c->cfg.od + OD_START + i * OD_FSZ;
  int sv = u->rot; u->rot = sv & 2;
  for (int x = 0; x < 128; x++) for (int y = 0; y < OLED_H; y++) ui_px(u, x, y, f[(y >> 3) * 128 + x] >> (y & 7) & 1);
  u->rot = sv;
}
static int ui_text_len_px(const char *t) { return (int)strlen(t) * 12; }
// chu chay phai -> trai; te = thoi gian da troi; tra ve true neu da chay xong 1 luot
static bool ui_scroll(ui_t *u, const char *t, int sp, uint32_t te) {
  int pxs = 20 + 20 * sp, travel = 128 + ui_text_len_px(t);
  uint32_t per = (uint32_t)travel * 1000u / (uint32_t)pxs;
  uint32_t k = te % (per ? per : 1);
  int x = 128 - (int)(k * (uint32_t)pxs / 1000u);
  int sv = u->rot; u->rot = sv & 2;
  ui_txt(u, x, (OLED_H - 14) / 2, 2, t, true);
  u->rot = sv;
  return te >= per;
}
static uint32_t ui_scroll_ms(const char *t, int sp) { return (uint32_t)(128 + ui_text_len_px(t)) * 1000u / (uint32_t)(20 + 20 * sp); }

static void ui_init(ui_t *u, const core_t *c, uint32_t now) {
  memset(u, 0, sizeof *u);
  strcpy(u->shown, "READY");
  u->st = ui_osw(c, 2) ? UI_GREET : UI_NORMAL; u->t0 = now; u->idle0 = now; u->dirty = true;
}
static void ui_cmd(ui_t *u, int n, uint32_t now) {   // lenh xem thu tu app
  u->t0 = now; u->dirty = true; u->pv = true; u->blank = false;
  if (n == 1) u->st = UI_GREET; else if (n == 2) u->st = UI_SAVER; else if (n == 4) u->st = UI_LOGO;
  else { u->st = UI_NORMAL; u->pv = false; u->idle0 = now; }
}
// ---- man hinh Morse / Nokia ----
static bool ui_morse_on(const core_t *c) { return c->mo_len > 0 || (c->last_kc >= KC_MDOT && c->last_kc <= KC_MHINT); }
static bool ui_nokia_on(const core_t *c, uint32_t now) { return c->nk_key >= 0 && c->nk_key < 11 && now - c->nk_t < NOKIA_MS; }
static char ui_up(char ch) { return ch >= 'a' && ch <= 'z' ? ch - 32 : ch; }
static void ui_morse_opts(const core_t *c, char *o) {   // ". E  - T": nhanh ke tiep (chi in nhanh co ky tu)
  o[0] = 0; if (c->mo_len >= MORSE_MAX) return;
  for (int d = 0; d < 2; d++) {
    char nc[10]; memcpy(nc, c->mo_code, c->mo_len); nc[c->mo_len] = d ? '-' : '.'; nc[c->mo_len + 1] = 0;
    char ch = morse_find(nc);
    if (ch) { size_t l = strlen(o); if (l) { o[l++] = ' '; o[l++] = ' '; } o[l++] = d ? '-' : '.'; o[l++] = ' '; o[l++] = ui_up(ch); o[l] = 0; }
  }
}
static void ui_mode_top(ui_t *u, const char *ln, uint8_t host_leds) {   // hang tren: ten layer + NUM / CAP (ngang, 32 hang)
  ui_txt(u, 0, 0, 1, ln, true);
  ui_box(u, 80, 0, 21, 9, host_leds & 1); ui_txt(u, 82, 1, 1, "NUM", !(host_leds & 1));
  ui_box(u, 104, 0, 21, 9, host_leds & 2); ui_txt(u, 106, 1, 1, "CAP", !(host_leds & 2));
}
static void ui_morse_view(ui_t *u, const core_t *c, const char *ln, uint8_t host_leds) {
  ui_mode_top(u, ln, host_leds);
  int x = 0;   // ma dang go: cham = o vuong nho, gach = thanh dai
  char o[24]; ui_morse_opts(c, o);
  char ch = c->mo_len ? morse_find(c->mo_code) : 0;   // ky tu dang hien tren may tinh
  char b[2] = {ch ? ui_up(ch) : '?', 0};
#if OLED_H == 64
  for (int i = 0; i < c->mo_len; i++) {
    if (c->mo_code[i] == '-') { ui_box(u, x, 15, 14, 5, true); x += 18; } else { ui_box(u, x, 15, 5, 5, true); x += 10; }
  }
  if (!c->mo_len) ui_txt(u, 0, 14, 2, "MORSE", true);
  ui_txt(u, 0, 30, 2, o[0] ? o : "-", true);
  if (c->mo_len) ui_txt(u, 104, 28, 4, b, true);
#else
  for (int i = 0; i < c->mo_len; i++) {
    if (c->mo_code[i] == '-') { ui_box(u, x, 15, 10, 4, true); x += 14; } else { ui_box(u, x, 15, 4, 4, true); x += 8; }
  }
  if (!c->mo_len) ui_txt(u, 0, 13, 1, "MORSE", true);
  ui_txt(u, 0, 24, 1, o[0] ? o : "-", true);
  if (c->mo_len) ui_txt(u, 104, 11, 3, b, true);
#endif
}
static void ui_nokia_view(ui_t *u, const core_t *c, const char *ln, uint8_t host_leds, uint32_t now) {
  ui_mode_top(u, ln, host_leds);
  const char *g = nk_tab(&c->cfg, c->nk_key); int n = (int)strlen(g);
  for (int i = 0; i < n; i++) {   // nhom ky tu cua phim, ky tu dang chon dao mau
    bool sel = i == c->nk_n; char b[2] = {ui_up(g[i]) == ' ' ? '_' : ui_up(g[i]), 0};
#if OLED_H == 64
    ui_box(u, i * 24, 18, 22, 28, sel); ui_txt(u, i * 24 + 3, 21, 3, b, !sel);
#else
    ui_box(u, i * 14, 12, 13, 16, sel); ui_txt(u, i * 14 + 1, 13, 2, b, !sel);
#endif
  }
  uint32_t el = now - c->nk_t; int w = el >= NOKIA_MS ? 0 : (int)(128u * (NOKIA_MS - el) / NOKIA_MS);   // thanh con lai truoc khi chot ky tu
  ui_box(u, 0, OLED_H - 2, w, 2, true);
}
static void ui_normal(ui_t *u, const core_t *c, uint8_t host_leds, uint32_t now) {
  char ln[16], lab[24];
  if (c->rn_on) { strcpy(ln, "NAME"); strcpy(lab, c->rn_buf); strcat(lab, "_"); }
  else { core_layer_name(c, core_top_layer(c), ln, sizeof ln); strcpy(lab, u->shown); if (c->latch_wait) strcpy(lab, "LATCH ?"); }
  bool mo = !c->rn_on && !c->latch_wait && ui_morse_on(c), nk = !c->rn_on && !c->latch_wait && ui_nokia_on(c, now);
  if (mo || nk) {
    if (!(u->rot & 1)) { if (nk) ui_nokia_view(u, c, ln, host_leds, now); else ui_morse_view(u, c, ln, host_leds); return; }
    if (nk) { lab[0] = ui_up(nk_tab(&c->cfg, c->nk_key)[c->nk_n]); lab[1] = 0; if (lab[0] == ' ') lab[0] = '_'; }   // man doc / 64 hang: chi hien ky tu
    else { size_t k = 0; for (int i = 0; i < c->mo_len && k < 8; i++) lab[k++] = c->mo_code[i]; if (c->mo_len) { lab[k++] = ' '; lab[k++] = c->mo_shown ? ui_up(morse_find(c->mo_code)) : '?'; } lab[k] = 0; if (!k) strcpy(lab, "MORSE"); }
  }
  if (u->rot & 1) {   // doc: chu 1x, xuong dong
    int cpl = OLED_H == 32 ? 5 : 10;
    ui_txt_wrap(u, 0, 0, 1, ln, cpl, 3);
    ui_box(u, 0, 36, cpl * 6 - 1, 9, host_leds & 1); ui_txt(u, 1, 37, 1, "NUM", !(host_leds & 1));
    ui_box(u, 0, 48, cpl * 6 - 1, 9, host_leds & 2); ui_txt(u, 1, 49, 1, "CAP", !(host_leds & 2));
    ui_txt_wrap(u, 0, 64, 1, lab, cpl, 6);
    return;
  }
#if OLED_H == 32
  ui_txt(u, 0, 0, 2, ln, true);
  ui_box(u, 102, 0, 21, 9, host_leds & 1); ui_txt(u, 104, 1, 1, "NUM", !(host_leds & 1));
  ui_box(u, 102, 11, 21, 9, host_leds & 2); ui_txt(u, 104, 12, 1, "CAP", !(host_leds & 2));
  int s = ui_tw(lab, 2) <= 128 ? 2 : 1;
  ui_txt(u, 0, s == 2 ? 18 : 22, s, lab, true);
#else
  ui_txt(u, 0, 0, 3, ln, true);
  ui_box(u, 94, 0, 28, 11, host_leds & 1); ui_txt(u, 97, 2, 1, "NUM", !(host_leds & 1));
  ui_box(u, 94, 14, 28, 11, host_leds & 2); ui_txt(u, 97, 16, 1, "CAP", !(host_leds & 2));
  int s = ui_tw(lab, 3) <= 128 ? 3 : 2;
  ui_txt(u, 0, 34, s, lab, true);
#endif
}
// Goi moi ms. Tra ve true neu fb vua duoc ve lai (caller gui xuong man hinh). u->blank = man hinh phai tat.
static bool ui_task(ui_t *u, const core_t *c, uint32_t now, uint8_t host_leds) {
  int layer = core_top_layer(c);
  // huong theo layer
  int r = c->cfg.w[W_LRB + layer]; if (r > 3) r = c->cfg.w[W_LDB + LDB_ROT] & 3;
  int rot = (r + OLED_ROTATE * 2) & 3;
  if (c->touch != u->seen_touch) {
    u->seen_touch = c->touch; u->idle0 = now;
    if (u->st == UI_SAVER || u->st == UI_LOGO) { u->st = UI_NORMAL; u->pv = false; u->blank = false; u->dirty = true; }
  }
  if (c->last_kc) core_label(c, c->last_kc, u->shown, sizeof u->shown);
  // dieu kien vao screensaver
  uint16_t to = ui_osw(c, 0);
  if (u->st == UI_NORMAL && to && !c->rn_on && !c->latch_wait && now - u->idle0 >= (uint32_t)to * 1000u) { u->st = UI_SAVER; u->t0 = now; u->pv = false; u->dirty = true; }
  uint32_t sig = (uint32_t)u->st * 0x9E3779B1u ^ c->press_seq * 31u ^ (uint32_t)layer << 8 ^ host_leds << 16 ^ (uint32_t)c->rn_on << 20
               ^ (uint32_t)c->latch_wait << 21 ^ (uint32_t)rot << 22 ^ (uint32_t)c->rn_len << 24 ^ (c->rn_len ? (uint32_t)c->rn_buf[c->rn_len - 1] << 4 : 0)
               ^ (uint32_t)ui_morse_on(c) << 28 ^ (uint32_t)c->mo_len << 29 ^ (ui_nokia_on(c, now) ? 0x40000000u ^ (uint32_t)((now - c->nk_t) / UI_FRAME_MS) * 2654435761u : 0);
  bool anim = u->st != UI_NORMAL;
  if (sig == u->sig && !u->dirty && !(anim && now - u->last_draw >= UI_FRAME_MS)) return false;
  u->sig = sig; u->dirty = false; u->last_draw = now; u->rot = rot;
  memset(u->fb, 0, sizeof u->fb);
  int nf = c->cfg.od[0] <= OD_NF ? c->cfg.od[0] : 0, sp = ui_clamp(ui_osw(c, 1), 1, 10), frms = ui_clamp(ui_osw(c, 6), 20, 1000);
  uint32_t te = now - u->t0; char t[68];
  u->blank = false;
  switch (u->st) {
    case UI_GREET: {
      uint32_t logo = (ui_osw(c, 3) && nf) ? (uint32_t)nf * frms : 0;
      if (te < logo) { ui_frame(u, c, (int)(te / frms)); break; }
      ui_text_get(c, 16, t);
      uint32_t tt = te - logo, total = ui_scroll_ms(t, sp) * ui_clamp(ui_osw(c, 4), 1, 5);
      if (!t[0] || tt >= total) { u->st = UI_NORMAL; u->pv = false; u->idle0 = now; u->dirty = true; ui_normal(u, c, host_leds, now); break; }
      ui_scroll(u, t, sp, tt);
      break;
    }
    case UI_LOGO:
      if (!nf) { ui_txt(u, 0, 8, 2, "NO LOGO", true); break; }
      ui_frame(u, c, (int)((te / frms) % nf));
      break;
    case UI_SAVER: {
      int mode = ui_clamp(ui_osw(c, 5), 0, 3);
      ui_text_get(c, 80, t);
      if (mode == 3) { u->blank = true; break; }
      if (mode == 1 && nf) { ui_frame(u, c, (int)((te / frms) % nf)); break; }
      if (mode == 2 && nf && t[0]) {   // xen ke: 1 luot chu roi 1 vong logo
        uint32_t tm = ui_scroll_ms(t, sp), lg = (uint32_t)nf * frms, k = te % (tm + lg);
        if (k < tm) ui_scroll(u, t, sp, k); else ui_frame(u, c, (int)((k - tm) / frms));
        break;
      }
      ui_scroll(u, t, sp, te);
      break;
    }
    default: ui_normal(u, c, host_leds, now); break;
  }
  return true;
}
