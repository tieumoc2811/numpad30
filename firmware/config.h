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
#define OLED_ROTATE 0                        // 1 = xoay 180 do neu man hinh lap nguoc
#define LED_PIN 16                           // WS2812 onboard cua RP2350-Zero
#define LED_BRIGHT 24                        // do sang den (0-255), giu thap de khong qua dong USB
#define DEBOUNCE_MS 5                        // so lan quet 1 ms giong nhau lien tiep de chot trang thai phim
#define TAP_TERM_MS 200                      // giu >= so ms nay la "hold" (cho phim LT), nha som hon la "tap"
#define MACRO_GAP_MS 12                      // khoang nghi giua 2 bao cao khi go macro
#define SCREENSAVER_MS 60000                 // tat OLED + LED sau so ms khong bam phim (0 = khong bao gio)

// Cau hinh luu o sector cuoi cua flash. RP2350-Zero co 2 MB; doi gia tri neu board cua ban khac.
#define CFG_FLASH_SIZE (2 * 1024 * 1024)
#define CFG_FLASH_OFFSET (CFG_FLASH_SIZE - 4096)
