#pragma once
// 6 kenh den LED don sac (moi kenh 1 BJT), hieu ung thuan so nguyen: app Studio dung cung cong thuc de xem truoc.
// ledfx(mode, ch, t, speed, layer, kt): do sang 0..255 cua kenh ch (0..5)
//   t = thoi gian (ms), speed 0..255, layer = layer dang dung, kt = so ms tu lan bam phim gan nhat.
#include <stdint.h>
#define LED_N 6
#define LED_NMODES 12
static inline uint32_t fx_period(uint8_t speed) { return 3200u - (uint32_t)speed * 10u; }                       // 650..3200 ms
static inline uint8_t fx_tri(uint32_t t, uint32_t p) { uint32_t ph = (t % p) * 512u / p; return (uint8_t)(ph < 256 ? ph : 511 - ph); }
static inline uint8_t fx_gam(uint8_t v) { return (uint8_t)((uint32_t)v * v / 255u); }
static inline uint8_t fx_hash(uint32_t x) { return (uint8_t)((x * 2654435761u) >> 24); }
static inline uint8_t ledfx(uint8_t mode, uint8_t ch, uint32_t t, uint8_t speed, uint8_t layer, uint32_t kt) {
  uint32_t P = fx_period(speed);
  switch (mode) {
    case 0: return kt < 150 ? 255 : (ch <= layer ? 255 : 0);                                // Layer: so den sang = layer + 1, nhay khi bam
    case 1: return 0;                                                                         // Tat
    case 2: return 255;                                                                       // Sang tinh
    case 3: return fx_gam(fx_tri(t, P));                                                      // Tho
    case 4: { int d = (int)ch * 255 - (int)fx_tri(t, P) * 5; if (d < 0) d = -d; return d >= 255 ? 0 : (uint8_t)(255 - d); }   // Quet qua lai
    case 5: return fx_gam(fx_tri(t + (uint32_t)ch * P / 6u, P));                              // Song
    case 6: return (t % P) < P / 2 ? 255 : 0;                                                 // Nhap nhay
    case 7: { uint8_t h = fx_hash((uint32_t)ch * 7919u + t / 180u); return h > 160 ? 255 : (h > 110 ? 70 : 0); }   // Long lanh
    case 8: return kt >= 600 ? 18 : (uint8_t)(255 - kt * 237u / 600u);                        // Phan ung: sang khi bam, mo dan
    case 9: return (((t % P) < P / 2) ^ (ch & 1)) ? 255 : 0;                                  // Xen ke
    case 10: return ch < (t % P) * 8u / P ? 255 : 0;                                          // Day dan
    case 11: { if (kt > 900) return 0; int d = (int)kt - (int)ch * 90; if (d < 0 || d >= 270) return 0; return (uint8_t)(255 - d * 255 / 270); }   // Gon song tu kenh 0
  }
  return 0;
}
