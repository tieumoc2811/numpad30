#pragma once
// Chuot bay: gyro 3 truc (MPU-6515 / MPU-6500) -> do dich con tro. Thuan C (khong phu thuoc phan cung) de kiem thu tren PC.
// Gyro doc o +-500 dps (65.5 LSB/dps). Module nam phang, chip huong len: truc Z = quay quanh phuong thang dung (yaw) -> X,
// truc X = gat len/xuong (pitch) -> Y. Mac dinh: quay trai -> con tro sang trai, ngua len -> con tro len. Co co dao truc / doi truc.
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#define AIR_LSB_PER_DPS 65.5f
#define AIR_CAL_MAXFAIL 3      // bi lac qua 3 lan thi hieu chuan bat buoc 1 giay (khong kiem tra lac) de khong bao gio ket o che do hieu chuan
#define AIR_CAL_N 200          // so mau hieu chuan khi khoi dong / bam phim AIRCAL (~1 giay, giu yen)
#define AIR_PPD 0.28f          // pixel tren do khi do nhay = 1 (do nhay 60 ~ 17 px/do)

// AMB: [0]=do nhay X, [1]=do nhay Y (1-255), [2]=vung chet (0.1 do/giay), [3]=lam muot 0-9, [4]=co: bit0 dao X, bit1 dao Y, bit2 Y theo truc gy thay gx, bit3 luon bay
typedef struct {
  float bx, by, bz;                    // do lech khong (LSB)
  int cal_n, cal_k, cal_fail; double sx, sy, sz; // dang hieu chuan (cal_fail = so lan bi lac phai lam lai)
  float ex, ey;                        // toc do da lam muot (do/giay)
  float rx, ry;                        // phan le pixel
} air_t;

static void air_recal(air_t *a) { a->cal_n = AIR_CAL_N; a->cal_k = 0; a->cal_fail = 0; a->sx = a->sy = a->sz = 0; a->ex = a->ey = a->rx = a->ry = 0; }
static void air_init(air_t *a) { memset(a, 0, sizeof *a); air_recal(a); }
static bool air_calibrating(const air_t *a) { return a->cal_n > 0; }

// Goi moi mau gyro. dt_ms: thoi gian tu mau truoc. active: dang "bay". Tra dx, dy (so nguyen pixel, phan le dua vao mau sau).
static void air_update(air_t *a, const uint16_t *amb, int16_t gx, int16_t gy, int16_t gz, uint32_t dt_ms, bool active, int *dx, int *dy) {
  *dx = *dy = 0;
  if (a->cal_n > 0) {                  // hieu chuan: lay trung binh; neu bi lac thi lam lai (toi da AIR_CAL_MAXFAIL lan, sau do lay trung binh bat ke co lac; troi do lech cham se tinh chinh lai khi dung yen)
    if (a->cal_fail < AIR_CAL_MAXFAIL && a->cal_k > 10 && (fabsf(gx - (float)(a->sx / a->cal_k)) > 400 || fabsf(gy - (float)(a->sy / a->cal_k)) > 400 || fabsf(gz - (float)(a->sz / a->cal_k)) > 400)) { a->cal_n = AIR_CAL_N; a->cal_k = 0; a->cal_fail++; a->sx = a->sy = a->sz = 0; return; }
    a->sx += gx; a->sy += gy; a->sz += gz; a->cal_k++;
    if (--a->cal_n == 0) { a->bx = (float)(a->sx / a->cal_k); a->by = (float)(a->sy / a->cal_k); a->bz = (float)(a->sz / a->cal_k); }
    return;
  }
  uint16_t fl = amb[4];
  float ux = (gx - a->bx) / AIR_LSB_PER_DPS, uy = (gy - a->by) / AIR_LSB_PER_DPS, uz = (gz - a->bz) / AIR_LSB_PER_DPS;
  if (!active) {                       // khong bay: theo doi troi do lech cham khi dang dung yen, xoa trang thai
    if (fabsf(ux) < 1.5f && fabsf(uy) < 1.5f && fabsf(uz) < 1.5f) { a->bx += (gx - a->bx) * 0.01f; a->by += (gy - a->by) * 0.01f; a->bz += (gz - a->bz) * 0.01f; }
    a->ex = a->ey = a->rx = a->ry = 0; return;
  }
  float vx = -uz, vy = -((fl & 4) ? uy : ux);   // do/giay: yaw -> X, pitch -> Y
  if (fl & 1) vx = -vx;
  if (fl & 2) vy = -vy;
  float dz = amb[2] / 10.0f;
  vx = fabsf(vx) <= dz ? 0 : (vx > 0 ? vx - dz : vx + dz);
  vy = fabsf(vy) <= dz ? 0 : (vy > 0 ? vy - dz : vy + dz);
  float k = 1.0f / (1 + (amb[3] > 9 ? 9 : amb[3]));
  a->ex += (vx - a->ex) * k; a->ey += (vy - a->ey) * k;
  float dt = (dt_ms > 50 ? 50 : dt_ms) / 1000.0f;
  a->rx += a->ex * dt * (amb[0] > 255 ? 255 : amb[0]) * AIR_PPD;
  a->ry += a->ey * dt * (amb[1] > 255 ? 255 : amb[1]) * AIR_PPD;
  int ix = (int)a->rx, iy = (int)a->ry;
  a->rx -= ix; a->ry -= iy; *dx = ix; *dy = iy;
}
