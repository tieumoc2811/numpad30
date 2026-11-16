#pragma once
// ===== Cau hinh phan cung: Numpad30 (RP2350-Zero), ma tran 5x7 co diode ROW2COL =====
// Hang = output (keo cao khi quet), cot = input pull-down. Diode: anode ve phia cong tac, vach (cuc am) ve phia cot.
#define NROW 5
#define NCOL 7
#define ROW_PINS {0, 1, 2, 3, 4}            // R0..R4 -> GP0..GP4
#define COL_PINS {5, 6, 7, 8, 9, 10, 11}    // C0..C6 -> GP5..GP11
#define OLED_SDA 12                          // I2C0 SDA
#define OLED_SCL 13                          // I2C0 SCL
#define OLED_ADDR 0x3C
#ifndef OLED_H
#define OLED_H 32                            // 32 = 128x32 (0.91"), 64 = 128x64 (0.96"); doi bang -DOLED_H=64 hoac sua dong nay
#endif
#ifndef OLED_ROTATE
#define OLED_ROTATE 1                        // 1 = OLED lap nguoc: tu xoay 180 do o moi huong (app "Ngang 0 do" = huong da bu nay). Test PC dung -DOLED_ROTATE=0
#endif
#define LED_PIN 16                           // WS2812 onboard cua RP2350-Zero
#define LED_BRIGHT 24                        // do sang den (0-255), giu thap de khong qua dong USB
#define DEBOUNCE_MS 5                        // so lan quet 1 ms giong nhau lien tiep de chot trang thai phim
#define TAP_TERM_MS 200                      // giu >= so ms nay la "hold" (cho phim LT), nha som hon la "tap"
#define MACRO_GAP_MS 12                      // khoang nghi giua 2 bao cao khi go macro
// (screensaver nay duoc cau hinh bang app: o OSB[0] tinh bang giay, 0 = tat)
// ===== Them o v2 (tuong thich app Studio) =====
#define COMBO_MS 30                          // 2 phim combo bam trong khoang nay -> ket qua combo
#define TD_TERM_MS 200                       // tap dance: giu >= hold, nha roi cho lan bam thu 2 trong khoang nay
#define NK_BEEP_MS 70                        // v17: do dai tieng nốt khi bam phim Nokia (ms)
#define NOKIA_MS 600                         // multitap Nokia: bam lai cung phim trong khoang nay -> ky tu ke tiep
#define CHORD_MS 40                          // v19: 2 phim Morse (cham+gach) hoac 2 phim Nokia bam trong khoang nay = chord
#define CHORD2_MS 450                        // v19: chord thu hai trong khoang nay (tinh tu chord truoc) = chord doi
#define NK_AUTOCAP 1                         // v19: Nokia tu viet hoa chu dau sau ". " "! " "? " va sau Enter (0 = tat, phai build lai)
#define NK_CHORD_SP_NOTE 91                  // v19: nốt MIDI bip khi chord Nokia = Space (G6)
#define NK_CHORD_DEL_NOTE 72                 // v19: nốt MIDI bip khi chord doi Nokia = xoa (C5)
#define TAP_HOLD_MS 10                       // phim "tap" tao ra (LT/TD) duoc giu it nhat so ms nay
#define STORE_OFFSET (1024 * 1024)           // vi tri vung cau hinh trong flash (RP2350-Zero 2 MB)
#define STORE_SECTORS 14                     // 14 x 4 KB = 56 KB cho cau hinh + macro + du lieu OLED
#define FW_VER 20                             // bao cho app: >=20 layer MUSIC gop (phim bai truoc / ke / phat / tempo vong 0x7512..15 + OLED hien ten bai va nốt khi phat), >=5 co du lieu OLED, >=7 co Morse + goi y, >=8 co chuot bay (info[13]=cam bien OK, info[14]=WHO_AM_I), >=9 lenh 11 doc gyro truc tiep, >=10 lenh 12 thu HID chuot, >=11 lenh 13/14 doc/ghi ten layer + phim cuon chuot, >=12 coi bao Morse (khoi W_BZB, lenh 15 nghe thu, info[15]), >=13 nhac cu (ma phim 0x74xx / 0x75xx, layer PIANO + TUNES; khong doi khoi cau hinh), >=14 bai khoi dong (word OSB+7: 0 tat, 1..9 = bai), >=15 sua chuot bay: khong cong don chuyen dong khi USB ngu + hieu chuan khong ket khi bi lac, >=16 20 bai nhac (them 11 bai thieu nhi / dan ca; ma bai n>=16 = 0x7600|n; layer TUNES 2; bai khoi dong 1..20), >=17 phim Nokia keu nốt nhac (moi phim 1 nốt, dung coi Morse), >=18 nốt kêu theo tung phim o moi layer (khoi W_KNB, 562 word) + phim tat/bat loa va am luong loa (0x7490..92)
// ===== Coi bao (buzzer) cho che do Morse =====
// Module coi 3 chan: GND -> GND, VCC -> 3V3 (hoac 5V/VBUS neu module can 5V), I/O -> BUZ_PIN. Phat tieng khi bam Cham / Gach / OK.
#ifndef BUZ_PIN
#define BUZ_PIN 14                           // GP14 (con trong). Dat -1 de tat han coi
#endif
#ifndef BUZ_ACTIVE
#define BUZ_ACTIVE 0                         // 0 = xung PWM BUZ_FREQ Hz (chay ca coi chu dong lan thu dong); 1 = muc DC (chi coi chu dong)
#endif
#ifndef BUZ_INVERT
#define BUZ_INVERT 0                         // 1 = module kich bang muc THAP (chan I/O keo xuong de keu)
#endif
// Gia tri MAC DINH cua cai dat coi (app Studio doi duoc va luu flash, xem W_BZB):
#define BUZ_FREQ 2700                        // Hz (tan so cong huong thuong gap cua coi 12 mm); chi dung khi BUZ_ACTIVE = 0
#define BUZ_DOT_MS 70                        // do dai tieng "cham"; "gach" = 3 lan, "OK" = 4/7 lan (dung ti le Morse)
#define IMU_ADDR 0x68                        // MPU-6515 / MPU-6500 (chan AD0 noi GND; noi 3V3 thi 0x69). Dung chung I2C0 voi OLED (GP12/GP13)
#define IMU_POLL_MS 5                        // doc gyro 200 lan/giay
