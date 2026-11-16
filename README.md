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

Mac dinh: 4 layer **NUM** (numpad + Ctrl+C/V/X/Z/Y/A/S, Alt+Tab, Win+D, mui ten), **FN** (giu `Num`; nha ngan = NumLock: F1-F9, media, Home/End/PgUp/PgDn/Ins/Del, `0` bat/tat layer MACRO, `Enter` bat/tat layer MUSIC, `.` = BOOTSEL), **MACRO** (M1-M9), **MUSIC** (piano + 28 bai nhac, xem muc Layer MUSIC). Con lai la layer 4..31 (trong suot).

Tinh nang: 32 layer, MO/TL/LY+/LY-/LT, 4 tap dance (cham/giu/cham 2 lan), 4 combo (30 ms), 8 bo chuoi SEQ x 8 buoc, LATCH, TG Space/Shift/Ctrl/Alt, TK (toggle giu bat ky phim, toi da 4), Nokia multitap + RENAME (doi ten layer tren OLED), Morse (co goi y), 32 macro dang buoc (nhan/giu/nha/cho/go chuoi, lap), OLED: loi chao chu chay + logo dong 16 khung 128x32, screensaver 4 kieu, xoay 0/90/180/270 (chung hoac theo layer), LED RGB onboard (theo layer / cau vong / mot mau / tho / tat, CapsLock = do). Chong doi 5 ms, 6KRO, remote wakeup.

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

- Morse: ma toi da 7 ky hieu; bam them (ky hieu thu 8) se quay vong: xoa ky tu tam va bat dau lai tu ky hieu vua bam.
- Morse: hang tren = ten layer + NUM/CAP; giua = ma dang go (cham = o vuong, gach = thanh dai); duoi = nhanh ke tiep (vd `. R  - W`); ben phai = ky tu dang hien tren may tinh (hoac `?` neu ma chua co ky tu). Khi chua go gi hien `MORSE`.
- Nokia: hien nhom chu cua phim (vd `2DEF`), ky tu dang chon dao mau, thanh duoi giam dan trong 600 ms truoc khi chot. Het gio tro ve man hinh thuong.
- Man doc (90/270) va OLED 64 hang: chi hien nhan ngan (ma Morse + ky tu, hoac ky tu Nokia dang chon).


## Nốt kêu theo phím + loa (v18)

- **Nốt cho phím bất kỳ ở layer bất kỳ**: trong Studio chọn phím, mục "Nốt kêu khi bấm" gán nốt MIDI 24..108 (hoặc "Im"). Phím giữ chức năng cũ, chỉ thêm tiếng bíp nốt (mặc định 70 ms, chỉnh 20..300 ms, có công tắc chung). Nốt tính theo layer đang cho ra mã phím; ô trong suốt (TRNS) lấy nốt của layer bên dưới.
- **Phím điều khiển loa** (nhóm "Nhạc cụ" trong Studio): `Tắt/Bật loa` (mute mọi âm: Morse, Nokia, nhạc, bíp phím), `Loa +` / `Loa -` (5 mức 100, 75, 50, 25, 10 %). Trạng thái chỉ giữ trong RAM, cấp điện lại về 100 % và có tiếng. Đổi mức hoặc bật lại loa sẽ bíp thử 1 nốt.

## Coi bao Morse

- v17: phim Nokia keu 1 nốt ngan (70 ms, `NK_BEEP_MS`) moi lan bam. Cong tac rieng + nốt tung phim (khoi `W_NKB`) chinh trong Studio; mac dinh thang Do truong: 1..9 = C6..D7, 0 = A5, `.` = E7. Chi dung chung chan coi va am luong (`W_BZB`) voi Morse.

- Noi module coi 3 chan: GND -> GND, VCC -> 3V3 (hoac 5V / VBUS neu module ghi 5V), I/O -> **GP14**. Doi chan bang `BUZ_PIN` trong config.h (`-1` = tat han coi).
- O che do Morse, phim **Cham** keu bip ngan (mac dinh 70 ms), **Gach** keu bip dai gap 3, **OK** keu bip ngan (4/7 lan cham) khi chot ky tu. Ma qua dai / khong gui duoc thi im. Bat / tat, tan so, do dai va am luong chinh trong app Studio (luu flash); `BUZ_FREQ` / `BUZ_DOT_MS` trong config.h chi la gia tri mac dinh.
- Mac dinh `BUZ_ACTIVE 0`: chan I/O phat xung PWM (mac dinh 2700 Hz, doi trong app), chay duoc ca coi chu dong lan thu dong. Coi chu dong chi can muc cao thi doi `BUZ_ACTIVE 1`. Neu module kich bang muc thap (I/O keo xuong thi keu, im lang luc khoi dong thi keu lien tuc), doi `BUZ_INVERT 1`.
- Can `hardware_pwm` (da them vao CMakeLists.txt).

## Layer MUSIC (v20): piano + 28 bai nhac + nghe / nho nốt / choi lai

Tu v20 cac layer PIANO / TUNES / TUNES 2 / NOTES duoc **gop thanh 1 layer `MUSIC`** (layer 3, so layer mac dinh la 4: NUM, FN, MACRO, MUSIC). Dung chinh coi Morse tren GP14 lam dan; am don (1 coi = 1 nốt), nốt moi bam uu tien, nha thi quay ve nốt con giu. **Can coi THU DONG** (passive) de doi cao do.

- Bat: giu `Num` + bam `Enter` (R3C6, o layer FN). Tat: lai giu `Num` + `Enter`. Giu `Num` o MUSIC van vao FN (bam nhanh khong lam gi, khong bat NumLock).
- **Nghe va ghi nho**: bam `BAI SAU` / `BAI TRUOC` (2 phim) de chuyen qua lai trong danh sach 28 bai (vong tron, bam la phat ngay). Khi bai dang phat, **OLED hien ten bai, nốt dang phat (chu lon, vd `E5`), 4 nốt ke tiep, nốt vua qua, so bai `03/28`, tempo `T3` va thanh tien do**. Ten nốt giong het nhan phim: `C5` = phim C5, `F+5` = Fa thang quang tam 5 (font OLED khong co dau #).
- **Choi lai**: ca 24 nốt can thiet deu co phim rieng o quang tam goc, nen **moi nốt cua ca 28 bai** deu bam duoc truc tiep (test tu dong kiem tra dieu nay). Bam nốt bat ky khi dang phat = dung bai de tu choi; `PLAY/STOP` phat lai tu dau bai dang chon, hoac dung.
- Bai dang phat khong lam man hinh vao screensaver. Het bai tu dung, OLED ve man thuong.

```
        C0       C1     C2     C3      C4      C5     C6
R0:   PLAY/STOP  C#5    D#5   [Num]    F#5     G#5    A#5     phim den quang 5
R1:     C5       D5     E5     F5      G5      A5     B5      phim trang quang 5
R2:     C6       D6     E6     F6      G6      A6     --      phim trang quang 6
R3:     --       C#6    --     D#6    TEMPO    F#6    (FN)    phim den quang 6; R3C4 = tempo vong
R4:     G4       A4     B4    BAI TRUOC  --   BAI SAU  --      G4 A4 B4 = nốt thap cua mot so bai
```

- `PLAY/STOP` (R0C0): dang phat = dung; dang im = phat lai bai dang chon (chua chon: bai 1).
- `BAI TRUOC` (R4C3, phim `0`) / `BAI SAU` (R4C5, phim `.`): chon bai truoc / ke, vong tron 1 -> 28 -> 1, phat ngay. Chua chon gi: BAI SAU = bai 1, BAI TRUOC = bai 28.
- `TEMPO` (R3C4, phim `2`): doi tempo vong tron 5 muc (160 / 135 / 115 / 95 / 80 ms moi 1/16 nốt; mac dinh muc 3) -> cham de nghe ro nốt, roi tang dan. Doi tempo co hieu luc tu nốt ke tiep, khong dung bai. Tempo luu trong RAM (mat khi rut dien).
- Nốt thap nhat cua cac bai la G4 (392 Hz), cao nhat G6 (1568 Hz). Coi 12 mm yeu duoi ~400 Hz; coi thuong chi nghe ro ~400 Hz - 5 kHz. Cac bai duoc dich len 1 quang tam so voi ban viet trong `tunes.h` (nho vay, nốt hien tren OLED = dung nốt phim dang mang ten do).
- Cac ma phim cu van con va gan duoc trong Studio: Oct+/- (+-3 quang tam), Trans+/- (+-11 nua cung), Giu / Tat dan, Rung, Am luong, Am sac, STOP, DEMO, tempo -/+ va phim `TUNE(n)` (moi bai 1 phim). Layer mac dinh khong con du phim cho cac phim nay; muon dung, gan vao layer 4 tro len (layer trong suot) hoac doi phim trong layer MUSIC.
- Sua / them bai trong `firmware/tunes.h` (`{midi, do dai 1/16}`); them bai thu 21 tro len: them mang nốt + 1 dong `TUNE_OF(...)` vao `TUNE_TAB`, doi `NTUNE`. Tat ca la ban nhac dan gian / co truyen (het ban quyen). Nhac thieu nhi Viet Nam hien dai (vd. Chu ech con, Chau len ba...) con ban quyen tac gia nen khong dua vao san.
- 28 bai (v21 them bai 21..28): 1 Ode to Joy, 2 Twinkle, 3 Jingle Bells, 4 Fur Elise, 5 Happy Birthday, 6 Frere Jacques, 7 Greensleeves, 8 Mary Had a Little Lamb, 9 Canon in D, 10 Old MacDonald, 11 Row Your Boat, 12 London Bridge, 13 Cross Buns, 14 Yankee Doodle, 15 Lightly Row, 16 Au Clair de la Lune, 17 Surprise Symphony, 18 Silent Night, 19 Deck the Halls, 20 We Wish You a Merry Christmas, 21 Turkish March (Mozart), 22 Korobeiniki (dan ca Nga), 23 Beethoven 5, 24 Minuet in G, 25 Sakura, 26 Oh Susanna, 27 Itsy Bitsy Spider, 28 La Cucaracha.
- **Bai moi v21** viet tu tri nho o dang rut gon (Turkish March chi co chu de chinh, Beethoven 5 la dong co mo dau...), nen co the lech vai nốt so voi ban goc: nghe sai thi sua trong `tunes.h`. Nốt chi dung G4..G6 (khong dung G#4 / A#4 / G#6) de 24 phim layer MUSIC choi lai duoc het; test tu dong kiem tra dieu nay cho moi bai moi them. **Chua dua vao san** cac bai con ban quyen (vd. nhac phim *Cuop bien vung Caribe*, *Baby Shark*): tu nhap nốt vao `tunes.h` neu muon dung rieng.