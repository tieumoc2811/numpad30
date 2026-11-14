# Numpad30 RP2350-Zero

Firmware cho ban phim 30 phim (khoi macro 3x3, cum mui ten, numpad 17 phim) dung RP2350-Zero + OLED SSD1306. Ma nguon C tren Pico SDK. Tu v2 firmware tuong thich app Studio cua macropad15: cau hinh qua Raw HID, luu flash (xem `docs/PROTOCOL.md`).

## Phan cung (khop so do ma tran 5x7 co diode)

| Chuc nang | Chan |
|---|---|
| Hang R0..R4 (output) | GP0..GP4 |
| Cot C0..C6 (input, pull-down) | GP5..GP11 |
| OLED SDA / SCL (I2C0) | GP12 / GP13, dia chi 0x3C |
| LED RGB onboard | GP16 |

Diode 1N4148, huong ROW2COL: anode ve phia cong tac, vach (cuc am) ve phia cot. Doi chan/OLED trong `firmware/config.h`.

Bo cuc dien:

```
R0: M1   M2   M3    Num  /   *  -
R1: M4   M5   M6    7    8   9  +
R2: M7   M8   M9    4    5   6  .
R3: .    Up   .     1    2   3  Enter
R4: Left Down Right 0    .   .  .      (. = o trong; 0 o C3, dau cham o C5)
```

## Cau hinh (v2)
Keymap, macro, OLED khong con nam trong ma nguon: `firmware/keymap.h` chi la **mac dinh** (lan dau, hoac lenh "ve mac dinh"). Sau do app Studio sua tat ca qua USB (giao dien Raw HID usage page 0xFF60) va luu vao flash o 1 MB.

Mac dinh: 3 layer **NUM** (numpad + Ctrl+C/V/X/Z/Y/A/S, Alt+Tab, Win+D, mui ten), **FN** (giu `Num`; nha ngan = NumLock: F1-F9, media, Home/End/PgUp/PgDn/Ins/Del, `0` bat/tat layer MACRO, `.` = BOOTSEL), **MACRO** (M1-M9). Con lai la layer 3..31 (trong suot).

Tinh nang: 32 layer, MO/TL/LY+/LY-/LT, 4 tap dance (cham/giu/cham 2 lan), 4 combo (30 ms), 8 bo chuoi SEQ x 8 buoc, LATCH, TG Space/Shift/Ctrl/Alt, Nokia multitap + RENAME (doi ten layer tren OLED), Morse (co goi y), 32 macro dang buoc (nhan/giu/nha/cho/go chuoi, lap), OLED: loi chao chu chay + logo dong 16 khung 128x32, screensaver 4 kieu, xoay 0/90/180/270 (chung hoac theo layer), LED RGB onboard (theo layer / cau vong / mot mau / tho / tat, CapsLock = do). Chong doi 5 ms, 6KRO, remote wakeup.

Ma phim, ban do word, giao thuc: `docs/PROTOCOL.md`. Cau truc ma: `core.h` (engine phim), `proto.h` (giao thuc + dong goi flash), `ui.h` (OLED), `keymap.h` (mac dinh), `main.c` (phan cung).

## Build

Can Pico SDK 2.1.1 + gcc-arm-none-eabi + cmake:

```sh
export PICO_SDK_PATH=~/pico-sdk
cd firmware && mkdir build && cd build
cmake .. -DPICO_BOARD=pico2 && make -j
```

Giu nut BOOT tren board, cam USB, chep `numpad30.uf2` vao o RPI-RP2350. Can SDK 2.1.1 co `pico_flash`. `.github/workflows/build.yml` build san bang GitHub Actions.

## Kiem thu tren may tinh

`sh test/run_tests.sh` (chi can gcc): test don vi (chong doi, LT, layer, tap dance, combo, SEQ, LATCH, Nokia, Morse, RENAME, macro, 6KRO, giao thuc, CRC flash, OLED) va mo phong end-to-end chinh `main.c` voi Pico SDK/TinyUSB gia (gom Raw HID + luu/nap flash). **Chua thu tren phan cung that** (chua bien dich ARM, chua thu USB/flash/OLED that).

## Man hinh Morse / Nokia (OLED 128x32 ngang)

- Morse: hang tren = ten layer + NUM/CAP; giua = ma dang go (cham = o vuong, gach = thanh dai); duoi = nhanh ke tiep (vd `. R  - W`); ben phai = ky tu dang hien tren may tinh (hoac `?` neu ma chua co ky tu). Khi chua go gi hien `MORSE`.
- Nokia: hien nhom chu cua phim (vd `2DEF`), ky tu dang chon dao mau, thanh duoi giam dan trong 600 ms truoc khi chot. Het gio tro ve man hinh thuong.
- Man doc (90/270) va OLED 64 hang: chi hien nhan ngan (ma Morse + ky tu, hoac ky tu Nokia dang chon).
