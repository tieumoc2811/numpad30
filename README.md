# Numpad30 RP2350-Zero

Firmware cho ban phim 30 phim (khoi macro 3x3, cum mui ten, numpad 17 phim) dung RP2350-Zero + OLED SSD1306. Ma nguon C tren Pico SDK, khung USB/OLED/LED dua theo du an macropad15rp2350.

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

## Layer (`firmware/keymap.h`)

- **NUM** (mac dinh): numpad day du, 9 phim tat (Ctrl+C/V/X/Z/Y/A/S, Alt+Tab, Win+D), mui ten.
- **FN** (giu phim `Num`; nha ngan = NumLock): F1-F9, tat tieng / am luong / bai truoc-tiep-dung, Home/End/PgUp/PgDn/Ins/Del, Esc, Tab, Space, Enter, Backspace. Phim `0` bat/tat layer MACRO, phim `.` vao bootloader (BOOT).
- **MACRO** (bat/tat bang phim `0` o layer FN): M1-M9 go chuoi ky tu (sua trong `macros[]`, chi ASCII).

Ma phim: `C_(k)`/`S_(k)`/`A_(k)`/`G_(k)` to hop modifier, `MO(n)` giu layer, `TG(n)` bat tat layer, `LT(n,k)` tap/hold, `MACRO(n)`, `CONS(x)` media, `KC_BOOT`.

## Tinh nang

Chong doi 5 ms, 6KRO (tran -> ErrorRollOver), tap-hold co permissive hold, macro go chuoi, phim media, OLED hien layer + phim vua bam + NumLock/CapsLock cua may tinh, LED RGB theo layer (CapsLock = do), tiet kiem OLED/LED sau 60 s, remote wakeup.

## App chinh phim (web, 1 tep)

`numpad30-studio.html` — mo bang trinh duyet, khong can cai dat:

- Sua 30 phim tren 3 layer, doi ten layer, mau LED, thong so `config.h`, xem truoc OLED 128x32 / 128x64.
- Chon phim theo danh muc hoac tim kiem, bat modifier (Ctrl/Shift/Alt/Win), LT/MO/TG, hoac bam phim that de gan.
- Sua 9 macro (tu bo dau khi xuat), hoan tac / lam lai, tu luu nhap trong trinh duyet.
- Xuat `keymap.h`, `config.h`, doan ma mau LED va ho so JSON.
- **Ket noi truc tiep** (Chrome/Edge, WebHID): bam "Ket noi bang phim" -> app doc cau hinh tu ban phim; moi chinh sua (phim, ten layer, mau LED, macro, screensaver, do sang) ap dung ngay vao RAM; "Luu vao ban phim" ghi sector cuoi flash, tu nap lai khi cam USB. Debounce/tap-hold/OLED van doi trong `config.h` roi build lai.

Giao thuc Raw HID 32 byte (usage page 0xFF60, usage 0x61) mo ta trong `firmware/hid_proto.h`; cau hinh luu o `CFG_FLASH_OFFSET` (`config.h`, mac dinh sector cuoi cua flash 2 MB). Flash hong/rong -> dung keymap bien dich san trong `keymap.h`.

## Build

Can Pico SDK 2.1.1 + gcc-arm-none-eabi + cmake:

```sh
export PICO_SDK_PATH=~/pico-sdk
cd firmware && mkdir build && cd build
cmake .. -DPICO_BOARD=pico2 && make -j
```

Giu nut BOOT tren board, cam USB, chep `numpad30.uf2` vao o RPI-RP2350. `.github/workflows/build.yml` build san bang GitHub Actions.

## Kiem thu tren may tinh

`sh test/run_tests.sh` (chi can gcc): test don vi cho `core.h` (chong doi, LT, layer, to hop, macro, 6KRO, nhan) va mo phong end-to-end chinh `main.c` voi Pico SDK/TinyUSB gia. Chua thu tren phan cung that.
