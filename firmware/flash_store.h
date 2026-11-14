#pragma once
// Dong goi cau hinh de ghi vao 1 sector flash (4 KB). Khong phu thuoc Pico SDK de test tren PC.
// Header 12 byte: "NP30" | ver (u16) | body_len (u16) | crc32 (u32, tren phan body).
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "hid_proto.h"

#define FS_VER 2
#define FS_HDR 12
#define FS_BODY (NLAYER * NKEYS * 2 + NLAYER * (HP_NAME_MAX + 1) + NLAYER * 3 + NMACROS * (HP_MACRO_MAX + 1) + 6)

size_t fs_pack(const cfg_store_t *s, uint8_t *out, size_t cap);
bool fs_unpack(cfg_store_t *s, const uint8_t *in, size_t cap);

#ifdef FS_IMPL
static inline uint32_t fs_crc32(const uint8_t *p, size_t n) {
  uint32_t c = 0xFFFFFFFFu;
  for (size_t i = 0; i < n; i++) {
    c ^= p[i];
    for (int b = 0; b < 8; b++) c = (c >> 1) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(c & 1)));
  }
  return ~c;
}
static inline void fs_put16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static inline void fs_put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }
static inline uint16_t fs_get16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static inline uint32_t fs_get32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }

size_t fs_pack(const cfg_store_t *s, uint8_t *o, size_t cap) {
  if (cap < FS_HDR + FS_BODY) return 0;
  memset(o, 0, FS_HDR + FS_BODY);
  memcpy(o, "NP30", 4);
  fs_put16(o + 4, FS_VER); fs_put16(o + 6, (uint16_t)FS_BODY); fs_put32(o + 8, 0);
  size_t p = FS_HDR;
  for (int l = 0; l < NLAYER; l++) for (int i = 0; i < NKEYS; i++) { fs_put16(o + p, (uint16_t)s->km[l][i]); p += 2; }
  memcpy(o + p, s->names, sizeof s->names); p += sizeof s->names;
  memcpy(o + p, s->colors, sizeof s->colors); p += sizeof s->colors;
  memcpy(o + p, s->macs, sizeof s->macs); p += sizeof s->macs;
  fs_put32(o + p, s->ss_ms); p += 4;
  o[p++] = s->bright;
  o[p++] = s->nl;
  fs_put32(o + 8, fs_crc32(o + FS_HDR, FS_BODY));
  return p;
}
bool fs_unpack(cfg_store_t *s, const uint8_t *in, size_t cap) {
  if (!in || cap < FS_HDR + FS_BODY) return false;
  if (memcmp(in, "NP30", 4) != 0) return false;
  if (fs_get16(in + 4) != FS_VER || fs_get16(in + 6) != FS_BODY) return false;
  if (fs_get32(in + 8) != fs_crc32(in + FS_HDR, FS_BODY)) return false;
  size_t p = FS_HDR;
  for (int l = 0; l < NLAYER; l++) for (int i = 0; i < NKEYS; i++) { s->km[l][i] = (kc_t)fs_get16(in + p); p += 2; }
  memcpy(s->names, in + p, sizeof s->names); p += sizeof s->names;
  memcpy(s->colors, in + p, sizeof s->colors); p += sizeof s->colors;
  memcpy(s->macs, in + p, sizeof s->macs); p += sizeof s->macs;
  s->ss_ms = fs_get32(in + p); p += 4;
  s->bright = in[p++];
  if (in[p] < 1 || in[p] > NLAYER) return false;
  s->nl = in[p];
  for (int i = 0; i < NLAYER; i++) s->names[i][HP_NAME_MAX] = 0;
  for (int i = 0; i < NMACROS; i++) s->macs[i][HP_MACRO_MAX] = 0;
  s->save_req = false;
  return true;
}
#endif
