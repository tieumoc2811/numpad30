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
| 5 | Thong tin: `[1]`=32 layer, `[2..3]`=NW (1258), `[4]`=FW_VER (7), `[5]`=so hieu ung LED (16), `[6]`=so macro (32), `[7..8]`=MSLOT (1020), `[9]`=chieu cao OLED, `[10]`=OD_NF (16), `[11..12]`=OD_FSZ (512), `[13]`=do dai ten layer toi da (10; 0 = firmware cu khong ho tro lenh 11/12) |
| 6 / 7 | Doc / ghi `n<=28` byte macro, offset = `o*1020 + vi tri` (0..32639) |
| 8 / 9 | Doc / ghi `n<=28` byte du lieu OLED, offset 0..8447 |
| 11 | Doc ten layer: `out[1]` = layer (0..31); tra loi `[3..14]` = 12 byte ASCII, het chuoi la byte 0, rong = ten mac dinh |
| 12 | Doi ten layer: `out[1]` = layer, `out[2]` = so ky tu (0..10, 0 = ve ten mac dinh), `[3..]` = ky tu. Chu thuong tu doi thanh HOA; ky tu ngoai `A-Z 0-9 khoang trang ! ( ) * + , - . / : = ?` hoac qua dai -> `[0]=0xFF`, ten cu giu nguyen. Co hieu luc ngay tren OLED; goi lenh 3 de luu flash |
| 10 | Xem thu OLED: `out[1]` = 0 dung, 1 loi chao, 2 screensaver, 4 logo |

## Ban do word (uint16)
| Khoi | Offset | Noi dung |
|---|---|---|
| Phim | `l*35 + r*7 + c` (0..1119) | 32 layer x 35 o ma tran 5x7 (30 o co phim; 5 o trong luon 0) |
| Tap dance | 1120 + 3n | `[cham, giu, cham 2 lan]`, n = 0..3 |
| Combo | 1132 + 3n | `[A, B, ket qua]` (ma phim sau khi qua layer), cua so 30 ms, A=0 la tat |
| So layer | 1144 | 1..32 |
| LED | 1145..1153 | `[mode, toc do, sang, hue, sat, over, obMode, obSang, huong OLED chung]` |
| Huong OLED tung layer | 1154..1185 | 0..3, >=4 = theo huong chung |
| Man hinh | 1186..1193 | `[timeout giay (0=tat), toc do chu 1-10, loi chao bat, logo truoc loi chao, so lan chay 1-5, kieu saver 0-3, ms/khung, du phong]` |
| Chuoi (SEQ) | 1194 + 8n + k | 8 bo x 8 buoc, o 0 = het danh sach |
Tong NW = 1258. Ma o trong `r*7+c` khong co phim: (R2,C6) (R3,C0) (R3,C2) (R4,C4) (R4,C6).

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
