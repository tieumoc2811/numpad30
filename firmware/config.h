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
#define OLED_H 32                            // 32 = 128x32 (0.91"), 64 = 128x64 (0.96")
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
#define NOKIA_MS 600                         // multitap Nokia: bam lai cung phim trong khoang nay -> ky tu ke tiep
#define TAP_HOLD_MS 10                       // phim "tap" tao ra (LT/TD) duoc giu it nhat so ms nay
#define STORE_OFFSET (1024 * 1024)           // vi tri vung cau hinh trong flash (RP2350-Zero 2 MB)
#define STORE_SECTORS 12                     // 12 x 4 KB = 48 KB cho cau hinh + macro + du lieu OLED
#define FW_VER 7                             // bao cho app: >=5 co du lieu OLED, >=7 co Morse + goi y
