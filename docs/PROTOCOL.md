# Giao thuc Raw HID Numpad30 (tuong thich app Studio)

Giao dien HID thu 2: usage page `0xFF60`, usage `0x61`, VID `0xC1A0` PID `0x0030`, bao cao 32 byte vao/ra, khong report ID
(`device.sendReport(0, data)` / `inputreport`). Moi yeu cau co dung 1 tra loi.

## Khung
`out[0]` lenh | `out[1]` offset thap | `out[2]` so luong (word hoac byte) | `out[3..30]` du lieu | `out[31]` offset cao.
Tra loi: cung khung (echo `0..2`, `31`), du lieu tu `[3]`. Loi (tham so ngoai ranh, lenh la): `[0] = 0xFF`. Offset = `lo | hi<<8`.

| Lenh | Y nghia |
|---|---|
| 1 | Doc `n<=14` word tu offset (word thu i o `[3+2i]`, little endian) |
| 2 | Ghi `n<=14` word (`[3+2i]`), co hieu luc ngay |
| 3 | Luu toan bo cau hinh vao flash. Tra loi sau khi ghi xong (~0,3 s); `[0]=0xFF` neu ghi that bai |
| 4 | Ve mac dinh trong RAM (phim, macro, du lieu OLED, ten layer); chua luu flash |
| 5 | Thong tin: `[1]`=32 layer, `[2..3]`=NW (1270), `[4]`=FW_VER (14), `[5]`=so hieu ung LED (16), `[6]`=so macro (32), `[7..8]`=MSLOT (1020), `[9]`=chieu cao OLED, `[10]`=OD_NF (16), `[11..12]`=OD_FSZ (512), `[13]`=cam bien chuot bay dang hoat dong (0/1), `[14]`=WHO_AM_I cua cam bien, `[15]`=coi bao (v12): bit0 = co coi (`BUZ_PIN >= 0`), bit1 = coi chu dong (`BUZ_ACTIVE`, tan so / am luong khong co tac dung) |
| 6 / 7 | Doc / ghi `n<=28` byte macro, offset = `o*1020 + vi tri` (0..32639) |
| 8 / 9 | Doc / ghi `n<=28` byte du lieu OLED, offset 0..8447 |
| 10 | Xem thu OLED: `out[1]` = 0 dung, 1 loi chao, 2 screensaver, 4 logo |
| 11 | (v9) Go loi chuot bay, doc gyro truc tiep. Tra loi (s16 little endian): `[3..8]` gyro tho X,Y,Z (LSB, 65.5 LSB/do/s), `[9..14]` bias hieu chuan X,Y,Z, `[15..18]` pixel dang cho gui X,Y, `[19]` bit0 dang bay / bit1 dang hieu chuan, `[20..21]` do nhay X, `[22..23]` do nhay Y, `[24..25]` vung chet (0.1 do/s), `[26]` lam muot, `[27]` co, `[28]` so bao cao chuot da gui, `[29]` so bi tu choi (dem vong 256) |
| 12 | (v10) Thu duong HID chuot, khong dung gyro / phim AIR: `[12, dx, dy]` (int8). Gui thang 1 bao cao chuot; tra loi `[3]` = 1 neu gui duoc, 0 neu giao dien HID chua san sang |
| 13 | (v11) Doc ten layer tren OLED: `[13, layer]` -> `[3..14]` 12 byte, dem 0 (rong = ten mac dinh BASE / MORSE / LAYERn). `[0]=0xFF` neu layer >= 32 |
| 14 | (v11) Dat ten layer: `[14, layer, len, chu...]`, `len <= 10`. Chi giu ky tu OLED ve duoc (A-Z 0-9 khoang trang `! ( ) * + , - . / : = ?`), chu thuong doi thanh hoa. Chua vao flash cho toi khi gui lenh 3 |
| 15 | (v12) Nghe thu coi bao: `[15]`, keu 1 tieng "gach" theo cai dat hien tai (ke ca khi coi dang tat) |

## Ban do word (uint16)
| Khoi | Offset | Noi dung |
|---|---|---|
| Phim | `l*35 + r*7 + c` (0..1119) | 32 layer x 35 o ma tran 5x7 (30 o co phim; 5 o trong luon 0) |
| Tap dance | 1120 + 3n | `[cham, giu, cham 2 lan]`, n = 0..3 |
| Combo | 1132 + 3n | `[A, B, ket qua]` (ma phim sau khi qua layer), cua so 30 ms, A=0 la tat |
| So layer | 1144 | 1..32 |
| LED | 1145..1153 | `[mode, toc do, sang, hue, sat, over, obMode, obSang, huong OLED chung]` |
| Huong OLED tung layer | 1154..1185 | 0..3, >=4 = theo huong chung |
| Man hinh | 1186..1193 | `[timeout giay (0=tat), toc do chu 1-10, loi chao bat, logo truoc loi chao, so lan chay 1-5, kieu saver 0-3, ms/khung, bai khoi dong]` |
| Chuoi (SEQ) | 1194 + 8n + k | 8 bo x 8 buoc, o 0 = het danh sach |
Khoi chuot bay `AMB` = word 1258..1265: [do nhay X, do nhay Y, vung chet (0.1 do/s), lam muot 0-9, co (bit0 dao X, bit1 dao Y, bit2 Y theo gy, bit3 luon bay), 3 du phong]. Ma phim: 0x7070 AIR (giu de bay), 0x7071 AIR bat/tat, 0x7072/73/74 chuot trai/phai/giua, 0x7075 hieu chuan gyro, 0x7076 / 0x7077 cuon chuot len / xuong (bam 1 nac ngay, giu > 350 ms thi lap moi 50 ms), 0x7076 / 0x7077 cuon chuot len / xuong (bam 1 nac ngay, giu > 350 ms thi lap moi 50 ms).

Tong NW = 1270 (v12: them khoi coi bao W_BZB = 1266, 4 word: `[bat 0/1, tan so Hz 200-5000, do dai cham ms 20-300, am luong % 1-100]`; gach = 3 x cham, OK = 4/7 x cham; gia tri ngoai khoang -> mac dinh). Ban luu flash cu (thieu khoi coi bao, hoac thieu ca chuot bay) van nap duoc, phan thieu lay mac dinh. Ma o trong `r*7+c` khong co phim: (R2,C6) (R3,C0) (R3,C2) (R4,C4) (R4,C6).

LED onboard dung: `obMode` 0 theo layer / 1 cau vong / 2 mot mau (hue,sat) / 3 tho / 4 tat, do sang `obSang`, CapsLock = do. Cac hieu ung/den tung phim (word 0..5) chi luu giu.

## Ma phim
| Ma | Y nghia |
|---|---|
| `0` / `1` | khong gan / trong suot (xuong layer dang bat ke duoi) |
| `0x04..0xE7` | phim HID (`0xE0..0xE7` modifier) |
| `0x0100..0x0FFF` | `(mods<<8)\|kc`, mods: Ctrl 1, Shift 2, Alt 4, Win 8 |
| `0x2000\|(n<<8)\|kc` | LT: cham = kc, giu >= 200 ms = layer n (n<32), permissive hold |
| `0x4000\|n` / `0x4100\|n` | MO(n) giu layer / TL(n) bat tat layer (TL(0) tat het) |
| `0x4200` / `0x4201` | LY+ / LY- chuyen layer ke / truoc |
| `0x5000\|n` | TD(n) tap dance (n<4) |
| `0x6000\|i` | Nokia multitap `1234567890.` (i = 0..10), 600 ms |
| `0x7001` | LATCH; `0x7002..5` TG Space/Shift/Ctrl/Alt |
| `0x7010..16` | lenh LED cua macropad15: nhan nhung bo qua |
| `0x7020` | RENAME (doi ten layer tren OLED, toi da 10 ky tu, Enter luu, tu luu flash) |
| `0x7800\|kc` | TK: bam 1 lan giu phim `kc` (0x04..0xE7, gom modifier), bam lan nua nha. Toi da 4 phim giu cung luc; day thi bo qua. Mat khi rut USB / reset (khong luu flash) |
| `0x7060..63` | Morse cham / gach / xong / goi y |
| `0x7100\|i` | MACRO i (0..31) |
| `0x7200\|n` / `0x7280\|n` / `0x7300\|n` | SEQ tien / lui / dat lai (n = 0..7) |
| `0x7F00` | BOOTSEL |
| `0x8000\|u` | media (consumer usage u) |

## O macro (1020 byte)
`[lap, nghi_lo, nghi_hi, dai_ten, ten(12), dai_buoc_lo, dai_buoc_hi, buoc...]`. Buoc: `1 c_lo c_hi` nhan, `2` giu, `3` nha, `4 ms_lo ms_hi` cho, `5 n chu...` go chuoi ASCII (n<=64).

## Du lieu OLED
`[0]` so khung (<=16); `[16]` dai + chu loi chao (<=63); `[80]` dai + chu screensaver; khung i tai `256 + 512*i`, 128x32 dang page SSD1306 (`(y>>3)*128+x`, bit `y&7`). Chu hop le: `A-Z 0-9 khoang trang ! ( ) * + , - . / : = ?`.

## Luu flash
Vung `0x100000` (1 MB), 12 sector x 4 KB: `magic "NP30"`, ver, len, CRC32, roi toan bo `cfg_t` (word, macro, du lieu OLED, ten layer). Sai CRC/magic -> dung mac dinh.

## Ma phim nhac cu (v13)
Khong doi khoi cau hinh (NW van 1270); chi them ma phim, ghi vao o phim nhu moi ma khac:

| Ma | Y nghia |
|---|---|
| `0x7400 \| m` | Nốt, `m` = so MIDI 0..127 (C4 = 60, C5 = 72, thuc te kep 24..108 sau khi dich quang tam / chuyen giong). Am don, khong gui phim HID |
| `0x7480` / `0x7481` | Oct+ / Oct- (+-3) |
| `0x7482` / `0x7483` | Trans+ / Trans- (+-11 nua cung) |
| `0x7484` | STOP: dung bai, tat het nốt dang giu |
| `0x7485` | Giu <-> tat dan (pluck) |
| `0x7486` | Am sac (xung 50 / 25 / 12 %) |
| `0x7487` | Rung bat / tat |
| `0x7488` | Am luong 100 / 50 / 25 % |
| `0x7489` | DEMO: phat bai ke tiep (vong 9 bai) |
| `0x7500 \| n` | Phat bai n (0..8, xem `firmware/tunes.h`); bam lai khi dang phat = dung |
| `0x7510` / `0x7511` | Tempo cham / nhanh (5 muc, 160..80 ms moi 1/16 nốt) |

Mac dinh co them layer 3 `PIANO` va layer 4 `TUNES` (so layer mac dinh 3 -> 5); layer FN R3C6 = `TL(3)`. Layer PIANO dung `LT(1, 0)` o R0C3 (giu = FN, bam nhanh khong lam gi).

## Bai khoi dong (v14)
Word `OSB + 7` = 1193 (truoc day la o du phong): `0` tat (mac dinh, cau hinh cu van dung), `1..9` = phat bai tuong ung (thu tu nhu ma `0x7500|n`, n = gia tri - 1) tren coi ngay khi cap dien. Gia tri ngoai khoang -> im. Bam phim bat ky thi dung bai. Can coi THU DONG tren GP14. Doi bang lenh ghi (2) nhu moi word man hinh, luu flash bang lenh 3; co hieu luc o lan cap dien ke tiep.
