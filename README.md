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

- Morse: hang tren = ten layer + NUM/CAP; giua = ma dang go (cham = o vuong, gach = thanh dai); duoi = nhanh ke tiep (vd `. R  - W`); ben phai = ky tu dang hien tren may tinh (hoac `?` neu ma chua co ky tu). Khi chua go gi hien `MORSE`.
- Nokia: hien nhom chu cua phim (vd `2DEF`), ky tu dang chon dao mau, thanh duoi giam dan trong 600 ms truoc khi chot. Het gio tro ve man hinh thuong.
- Man doc (90/270) va OLED 64 hang: chi hien nhan ngan (ma Morse + ky tu, hoac ky tu Nokia dang chon).


## Coi bao Morse

- Noi module coi 3 chan: GND -> GND, VCC -> 3V3 (hoac 5V / VBUS neu module ghi 5V), I/O -> **GP14**. Doi chan bang `BUZ_PIN` trong config.h (`-1` = tat han coi).
- O che do Morse, phim **Cham** keu bip ngan (mac dinh 70 ms), **Gach** keu bip dai gap 3, **OK** keu bip ngan (4/7 lan cham) khi chot ky tu. Ma qua dai / khong gui duoc thi im. Bat / tat, tan so, do dai va am luong chinh trong app Studio (luu flash); `BUZ_FREQ` / `BUZ_DOT_MS` trong config.h chi la gia tri mac dinh.
- Mac dinh `BUZ_ACTIVE 0`: chan I/O phat xung PWM (mac dinh 2700 Hz, doi trong app), chay duoc ca coi chu dong lan thu dong. Coi chu dong chi can muc cao thi doi `BUZ_ACTIVE 1`. Neu module kich bang muc thap (I/O keo xuong thi keu, im lang luc khoi dong thi keu lien tuc), doi `BUZ_INVERT 1`.
- Can `hardware_pwm` (da them vao CMakeLists.txt).

## Nhac cu (v13): PIANO + TUNES

Dung chinh coi Morse tren GP14 (xem muc tren) lam dan: **moi phim = 1 nốt** (layer PIANO) hoac **moi phim = 1 bai** (layer TUNES). Am don (1 coi = 1 nốt), nốt moi bam uu tien, nha thi quay ve nốt con giu.

- **Can coi THU DONG** (passive) de doi cao do. Coi chu dong chi co 1 tong co dinh, layer dan se khong ra dung nốt.
- Bat: giu `Num` + bam `Enter` (R3C6, o layer FN). Tat: lai giu `Num` + `Enter`. Giu `Num` o PIANO / TUNES van vao FN (bam nhanh khong lam gi, khong bat NumLock).
- Cau hinh luu flash cu van la 3 layer: layer PIANO / TUNES / TUNES 2 chi xuat hien sau lenh "ve mac dinh" (app Studio) roi luu, hoac tu gan phim bang ma o `docs/PROTOCOL.md`. Layer mac dinh hien la 6 (v16, truoc la 5): sau khi nap firmware moi, can "ve mac dinh" + luu de co trang TUNES 2 va phim mui ten R4C2 o trang 1.

```
        C0     C1     C2     C3     C4     C5     C6
R0:    STOP    C#5    D#5   [Num]   F#5    G#5    A#5     phim den
R1:     C5     D5     E5     F5     G5     A5     B5      phim trang
R2:     C6     D6     E6     F6     G6     A6     --      quang tam tren
R3:     --    Oct+    --    Giu/    Rung   Am     (FN)
                            Tat dan       luong
R4:   Trans-  Oct-  Trans+  Am sac  --    TUNES   --
```

- **Oct+ / Oct-**: +-3 quang tam. **Trans-/+**: chuyen giong +-11 nua cung. **Giu / Tat dan**: nốt keu deu khi giu, hoac tat dan ~0,7 s nhu gay dan. **Rung**: +-1,5 % ~6 Hz. **Am luong**: 100 / 50 / 25 % (nhan voi am luong chung trong app). **Am sac**: xung 50 / 25 / 12 %. Cac cai dat nay nam trong RAM, mat khi rut dien.
- Mac dinh bat dau tu C5 (524 Hz) vi coi 12 mm yeu duoi ~400 Hz. Nốt thap nhat 32 Hz / cao nhat ~4,2 kHz (gioi han 24..108 MIDI); coi thuong chi nghe ro ~400 Hz - 5 kHz.
- **TUNES** (R4C5 o PIANO): 20 bai chia 2 trang, phim `-` / `+` cham / nhanh (5 muc), R3C5 dung. Bam lai bai dang phat hoac bat ky phim nao khac = dung. Sua / them bai trong `firmware/tunes.h` (`{midi, do dai 1/16}`).
  - **Trang 1 (TUNES)**: M1..M9 = Ode to Joy, Twinkle, Jingle Bells, Fur Elise, Happy Birthday, Frere Jacques, Greensleeves, Mary Had a Little Lamb, Canon in D. Phim mui ten phai `R4C2` sang trang 2, `R4C5` ve PIANO.
  - **Trang 2 (TUNES 2, v16)**: M1..M9 = Old MacDonald, Row Your Boat, London Bridge, Cross Buns (Hot Cross Buns), Yankee Doodle, Lightly Row, Au Clair de la Lune, Surprise Symphony (Haydn), Silent Night; `R1C3` = Deck the Halls, `R1C4` = We Wish You a Merry Christmas. Phim mui ten trai `R4C0` ve trang 1.
  - Tat ca la ban nhac dan gian / co truyen (het ban quyen). Nhac thieu nhi Viet Nam hien dai (vd. Chu ech con, Chau len ba...) con ban quyen tac gia nen khong dua vao san; muon them thi tu nhap nốt vao `tunes.h`.
  - Moi bai dung 1 phim; muon them bai thu 21 tro len: them mang nốt + 1 dong `TUNE_OF(...)` vao `TUNE_TAB`, doi `NTUNE`, roi gan phim `TUNE(n)` (xem `firmware/keymap.h`).
- OLED hien ten nốt (`C+5` = Do thang quang tam 5, font khong co dau #) hoac ten bai.
- Sua o v13: tan so PWM duoi ~570 Hz truoc day tran thanh ghi wrap 16 bit (bo chia xung co dinh /4); gio chia xung tu dong, nốt 32 Hz van dung.

- **Bai khoi dong (v14, v16: 1..20)**: chon 1 trong 20 bai de coi phat khi cap dien (app Studio, tab Coi bao -> "Bai khoi dong", nho "Luu vao thiet bi"). Mac dinh tat. Bam phim bat ky de dung. Word `OSB+7`, xem `docs/PROTOCOL.md`.

## Chuot bay (MPU-6515)

- Noi MPU-6515 chung I2C0 voi OLED: SDA -> GP12, SCL -> GP13, VCC -> 3V3, GND -> GND, AD0 -> GND (dia chi 0x68; AD0 -> 3V3 thi doi `IMU_ADDR` thanh 0x69 trong config.h). Gyro doc o +-500 dps, 200 lan/giay.
- HID them chuot (report ID 3). Phim: AIR (giu de bay), AIR bat/tat, chuot trai/phai/giua, hieu chuan, cuon len/xuong (giu de cuon lien tuc). Mac dinh nam o layer MACRO: R1C3 AIR, R1C4 trai, R1C5 phai, R2C3 giua, R2C4 AIR bat/tat, R2C5 hieu chuan, R3C3 cuon len, R3C4 cuon xuong (cau hinh cu da luu: tu gan 2 phim nay trong app).
- Khi cam USB hoac bam hieu chuan, de ban phim nam yen ~1 giay. Do nhay, vung chet, lam muot, dao truc chinh trong app (tab Den LED -> Chuot bay).
- Cau hinh cu da luu trong flash van dung duoc (khoi chuot bay lay mac dinh); phim chuot phai tu gan trong app neu khong ve mac dinh.
- Neu cam bien khong tra loi (day sai), ban phim van chay binh thuong, firmware thu lai moi 2 giay.
- Do phan giai OLED mac dinh 128x32; doi `OLED_H` thanh 64 trong config.h neu dung man 0.96".
- Kiem tra cam bien trong app Studio (tab Chuot bay): "Kiem tra cam bien" doc gyro tho qua lenh 11 (firmware v9+), "Thu chuot" gui thang bao cao chuot qua lenh 12 (v10+). Gyro tho dung yen ~0 (lech bias nho) va phai nhay so khi nghieng. Neu 3 so dung o 0 la gyro bi tat (PWR_MGMT_2 0x6C phai la 0x38, khong phai 0x07).
- Ten layer (v11): app doc/ghi ten layer tren thiet bi qua lenh 13/14 (toi da 10 ky tu: A-Z 0-9 va `! ( ) * + , - . / : = ?`). Ten doi bang phim RENAME tren ban phim duoc app doc lai khi ket noi hoac bam "Dong bo ten".
