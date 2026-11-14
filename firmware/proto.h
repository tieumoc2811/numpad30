#pragma once
// Giao thuc Raw HID (32 byte / bao cao, khong report ID) + dong goi cau hinh de luu flash. Thuan C, kiem thu tren PC.
// Khung: out[0]=lenh, out[1]=offset thap, out[2]=so (word / byte), out[3..30]=du lieu, out[31]=offset cao. Tra loi: cung khung, lenh 5 khac.
#include "keymap.h"

#define CMD_READ 1        // doc word: [1, lo, n<=14] hi -> r[3 + 2i] = word (little endian)
#define CMD_WRITE 2       // ghi word: [2, lo, n<=14, w0lo, w0hi, ...] hi
#define CMD_SAVE 3        // luu flash (tra loi sau khi ghi xong)
#define CMD_DEFAULTS 4    // ve mac dinh (RAM, chua luu flash)
#define CMD_INFO 5        // thong tin
#define CMD_MREAD 6       // doc byte macro: offset = o*MSLOT + vi tri, n<=28
#define CMD_MWRITE 7      // ghi byte macro
#define CMD_ODREAD 8      // doc byte du lieu OLED (n<=28)
#define CMD_ODWRITE 9     // ghi byte du lieu OLED
#define CMD_PREVIEW 10    // [10, n]: 0 dung, 1 loi chao, 2 screensaver, 4 logo
#define CMD_LNREAD 11     // doc ten layer: [11, layer] -> r[3..14] = 12 byte (0 = ten mac dinh)
#define CMD_LNWRITE 12    // ghi ten layer: [12, layer, n<=NAMELEN, ky tu...]; n = 0 -> ten mac dinh; co hieu luc ngay, luu bang CMD_SAVE
#define PROTO_SAVE 1      // proto_cmd tra ve bit nay: caller phai luu flash roi moi gui tra loi
#define PROTO_PREVIEW 2   // caller phai goi ui_cmd(out[1])

static bool name_char_ok(char ch) { return (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || (ch && strchr(" !()*+,-./:=?", ch)); }

static int proto_cmd(core_t *c, const uint8_t *in, uint8_t *out) {
  memset(out, 0, 32);
  out[0] = in[0]; out[1] = in[1]; out[2] = in[2]; out[31] = in[31];
  unsigned off = (unsigned)(in[1] | (in[31] << 8)), n = in[2];
  int flags = 0;
  switch (in[0]) {
    case CMD_READ:
      if (n > 14 || off + n > NW) { out[0] = 0xFF; break; }
      for (unsigned i = 0; i < n; i++) { uint16_t v = c->cfg.w[off + i]; out[3 + 2 * i] = v & 255; out[4 + 2 * i] = v >> 8; }
      break;
    case CMD_WRITE:
      if (n > 14 || off + n > NW) { out[0] = 0xFF; break; }
      for (unsigned i = 0; i < n; i++) c->cfg.w[off + i] = (uint16_t)(in[3 + 2 * i] | (in[4 + 2 * i] << 8));
      break;
    case CMD_SAVE: flags |= PROTO_SAVE; break;
    case CMD_DEFAULTS: { cfg_t *g = &c->cfg; cfg_defaults(g); core_reset(c); } break;
    case CMD_INFO:
      out[1] = NLAYER; out[2] = NW & 255; out[3] = NW >> 8; out[4] = FW_VER; out[5] = 16; out[6] = NMAC;
      out[7] = MSLOT & 255; out[8] = MSLOT >> 8; out[9] = OLED_H; out[10] = OD_NF; out[11] = OD_FSZ & 255; out[12] = OD_FSZ >> 8; out[13] = NAMELEN;
      break;
    case CMD_MREAD: case CMD_MWRITE:
      if (n > 28 || off + n > NMAC * MSLOT) { out[0] = 0xFF; break; }
      if (in[0] == CMD_MREAD) memcpy(out + 3, (const uint8_t *)c->cfg.mac + off, n); else memcpy((uint8_t *)c->cfg.mac + off, in + 3, n);
      break;
    case CMD_ODREAD: case CMD_ODWRITE:
      if (n > 28 || off + n > OD_SIZE) { out[0] = 0xFF; break; }
      if (in[0] == CMD_ODREAD) memcpy(out + 3, c->cfg.od + off, n); else memcpy(c->cfg.od + off, in + 3, n);
      break;
    case CMD_LNREAD:
      if (in[1] >= NLAYER) { out[0] = 0xFF; break; }
      memcpy(out + 3, c->cfg.lname[in[1]], 12);
      break;
    case CMD_LNWRITE: {
      char nm[12] = {0}; bool ok = in[1] < NLAYER && n <= NAMELEN;
      for (unsigned i = 0; ok && i < n; i++) {
        char ch = (char)in[3 + i]; if (ch >= 'a' && ch <= 'z') ch -= 32;
        if (!name_char_ok(ch)) ok = false; else nm[i] = ch;
      }
      if (!ok) { out[0] = 0xFF; break; }
      memcpy(c->cfg.lname[in[1]], nm, 12);
    } break;
    case CMD_PREVIEW: flags |= PROTO_PREVIEW; break;
    default: out[0] = 0xFF; break;
  }
  return flags;
}

// ---------------- luu / nap flash ----------------
#define STORE_MAGIC 0x4E503330u   // "NP30"
#define STORE_VER 1u
#define STORE_SIZE (STORE_SECTORS * 4096)
typedef struct { uint32_t magic, ver, len, crc; } store_hdr_t;
_Static_assert(sizeof(store_hdr_t) + sizeof(cfg_t) <= STORE_SIZE, "cau hinh vuot vung flash");
_Static_assert(NW == 1258 && NKEYS == 35, "ban do word khop app");

static uint32_t crc32_buf(const uint8_t *p, size_t n) {
  uint32_t c = 0xFFFFFFFFu;
  for (size_t i = 0; i < n; i++) { c ^= p[i]; for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1))); }
  return ~c;
}
static void store_pack(const cfg_t *g, uint8_t *buf) {    // buf: STORE_SIZE byte, phan thua = 0xFF
  memset(buf, 0xFF, STORE_SIZE);
  store_hdr_t h = {STORE_MAGIC, STORE_VER, sizeof(cfg_t), crc32_buf((const uint8_t *)g, sizeof(cfg_t))};
  memcpy(buf, &h, sizeof h); memcpy(buf + sizeof h, g, sizeof(cfg_t));
}
static bool store_unpack(const uint8_t *buf, cfg_t *g) {   // dung neu hop le (magic, phien ban, kich thuoc, CRC)
  store_hdr_t h; memcpy(&h, buf, sizeof h);
  if (h.magic != STORE_MAGIC || h.ver != STORE_VER || h.len != sizeof(cfg_t)) return false;
  if (crc32_buf(buf + sizeof h, sizeof(cfg_t)) != h.crc) return false;
  memcpy(g, buf + sizeof h, sizeof(cfg_t));
  return true;
}
