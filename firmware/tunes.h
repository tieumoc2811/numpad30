#pragma once
// Bai nhac cho che do nhac cu (v13). Moi nốt {midi, do dai}: do dai tinh theo 1/16 nốt (4 = nốt den), midi 0 = nghi.
// Cac bai viet o quang tam 4-5; khi phat dich len 1 quang tam (coi nho phat ro hon tu ~500 Hz). Toan bo la ban nhac dan gian / co truyen (khong con ban quyen).
#include <stdint.h>
#define NTUNE 63                           // v16: 9 bai goc (0..8) + 11 bai thieu nhi / dan ca (9..19); v21: +8 bai co dien / dan ca (20..27); v22: +35 bai (28..62). Ma phim: 0..15 = 0x7500|n, 16.. = 0x7600|n
typedef struct { uint8_t n, d; } tnote_t;
typedef struct { const char *name; const tnote_t *p; uint16_t len; } tune_t;
enum { N_R = 0, N_G3 = 55, N_A3 = 57, N_B3 = 59, N_C4 = 60, N_Cs4, N_D4, N_Ds4, N_E4, N_F4, N_Fs4, N_G4, N_Gs4, N_A4, N_As4, N_B4,
       N_C5, N_Cs5, N_D5, N_Ds5, N_E5, N_F5, N_Fs5, N_G5, N_Gs5, N_A5, N_As5, N_B5, N_C6 };
#define TN(n, d) {(n), (d)}

static const tnote_t TN_ODE[] = {   // Ode to Joy (Beethoven)
  TN(N_E4,4),TN(N_E4,4),TN(N_F4,4),TN(N_G4,4), TN(N_G4,4),TN(N_F4,4),TN(N_E4,4),TN(N_D4,4), TN(N_C4,4),TN(N_C4,4),TN(N_D4,4),TN(N_E4,4), TN(N_E4,6),TN(N_D4,2),TN(N_D4,8),
  TN(N_E4,4),TN(N_E4,4),TN(N_F4,4),TN(N_G4,4), TN(N_G4,4),TN(N_F4,4),TN(N_E4,4),TN(N_D4,4), TN(N_C4,4),TN(N_C4,4),TN(N_D4,4),TN(N_E4,4), TN(N_D4,6),TN(N_C4,2),TN(N_C4,8),
  TN(N_D4,4),TN(N_D4,4),TN(N_E4,4),TN(N_C4,4), TN(N_D4,4),TN(N_E4,2),TN(N_F4,2),TN(N_E4,4),TN(N_C4,4), TN(N_D4,4),TN(N_E4,2),TN(N_F4,2),TN(N_E4,4),TN(N_D4,4), TN(N_C4,4),TN(N_D4,4),TN(N_G3,8),
  TN(N_E4,4),TN(N_E4,4),TN(N_F4,4),TN(N_G4,4), TN(N_G4,4),TN(N_F4,4),TN(N_E4,4),TN(N_D4,4), TN(N_C4,4),TN(N_C4,4),TN(N_D4,4),TN(N_E4,4), TN(N_D4,6),TN(N_C4,2),TN(N_C4,8) };
static const tnote_t TN_TWINKLE[] = {   // Twinkle Twinkle Little Star
  TN(N_C4,4),TN(N_C4,4),TN(N_G4,4),TN(N_G4,4),TN(N_A4,4),TN(N_A4,4),TN(N_G4,8), TN(N_F4,4),TN(N_F4,4),TN(N_E4,4),TN(N_E4,4),TN(N_D4,4),TN(N_D4,4),TN(N_C4,8),
  TN(N_G4,4),TN(N_G4,4),TN(N_F4,4),TN(N_F4,4),TN(N_E4,4),TN(N_E4,4),TN(N_D4,8), TN(N_G4,4),TN(N_G4,4),TN(N_F4,4),TN(N_F4,4),TN(N_E4,4),TN(N_E4,4),TN(N_D4,8),
  TN(N_C4,4),TN(N_C4,4),TN(N_G4,4),TN(N_G4,4),TN(N_A4,4),TN(N_A4,4),TN(N_G4,8), TN(N_F4,4),TN(N_F4,4),TN(N_E4,4),TN(N_E4,4),TN(N_D4,4),TN(N_D4,4),TN(N_C4,8) };
static const tnote_t TN_JINGLE[] = {   // Jingle Bells (diep khuc)
  TN(N_E4,4),TN(N_E4,4),TN(N_E4,8), TN(N_E4,4),TN(N_E4,4),TN(N_E4,8), TN(N_E4,4),TN(N_G4,4),TN(N_C4,6),TN(N_D4,2),TN(N_E4,16),
  TN(N_F4,4),TN(N_F4,4),TN(N_F4,6),TN(N_F4,2), TN(N_F4,4),TN(N_E4,4),TN(N_E4,4),TN(N_E4,2),TN(N_E4,2), TN(N_E4,4),TN(N_D4,4),TN(N_D4,4),TN(N_E4,4),TN(N_D4,8),TN(N_G4,8),
  TN(N_E4,4),TN(N_E4,4),TN(N_E4,8), TN(N_E4,4),TN(N_E4,4),TN(N_E4,8), TN(N_E4,4),TN(N_G4,4),TN(N_C4,6),TN(N_D4,2),TN(N_E4,16),
  TN(N_F4,4),TN(N_F4,4),TN(N_F4,4),TN(N_F4,4), TN(N_F4,4),TN(N_E4,4),TN(N_E4,4),TN(N_E4,2),TN(N_E4,2), TN(N_G4,4),TN(N_G4,4),TN(N_F4,4),TN(N_D4,4),TN(N_C4,16) };
static const tnote_t TN_ELISE[] = {   // Fur Elise (Beethoven, doan mo dau)
  TN(N_E5,1),TN(N_Ds5,1), TN(N_E5,1),TN(N_Ds5,1),TN(N_E5,1),TN(N_B4,1),TN(N_D5,1),TN(N_C5,1), TN(N_A4,2),TN(N_R,1),TN(N_C4,1),TN(N_E4,1),TN(N_A4,1),
  TN(N_B4,2),TN(N_R,1),TN(N_E4,1),TN(N_Gs4,1),TN(N_B4,1), TN(N_C5,2),TN(N_R,1),TN(N_E4,1),TN(N_E5,1),TN(N_Ds5,1),
  TN(N_E5,1),TN(N_Ds5,1),TN(N_E5,1),TN(N_B4,1),TN(N_D5,1),TN(N_C5,1), TN(N_A4,2),TN(N_R,1),TN(N_C4,1),TN(N_E4,1),TN(N_A4,1),
  TN(N_B4,2),TN(N_R,1),TN(N_E4,1),TN(N_C5,1),TN(N_B4,1), TN(N_A4,6) };
static const tnote_t TN_BDAY[] = {   // Happy Birthday
  TN(N_G4,3),TN(N_G4,1),TN(N_A4,4),TN(N_G4,4),TN(N_C5,4),TN(N_B4,8), TN(N_G4,3),TN(N_G4,1),TN(N_A4,4),TN(N_G4,4),TN(N_D5,4),TN(N_C5,8),
  TN(N_G4,3),TN(N_G4,1),TN(N_G5,4),TN(N_E5,4),TN(N_C5,4),TN(N_B4,4),TN(N_A4,4), TN(N_F5,3),TN(N_F5,1),TN(N_E5,4),TN(N_C5,4),TN(N_D5,4),TN(N_C5,8) };
static const tnote_t TN_FRERE[] = {   // Frere Jacques
  TN(N_C4,4),TN(N_D4,4),TN(N_E4,4),TN(N_C4,4), TN(N_C4,4),TN(N_D4,4),TN(N_E4,4),TN(N_C4,4), TN(N_E4,4),TN(N_F4,4),TN(N_G4,8), TN(N_E4,4),TN(N_F4,4),TN(N_G4,8),
  TN(N_G4,2),TN(N_A4,2),TN(N_G4,2),TN(N_F4,2),TN(N_E4,4),TN(N_C4,4), TN(N_G4,2),TN(N_A4,2),TN(N_G4,2),TN(N_F4,2),TN(N_E4,4),TN(N_C4,4),
  TN(N_C4,4),TN(N_G3,4),TN(N_C4,8), TN(N_C4,4),TN(N_G3,4),TN(N_C4,8) };
static const tnote_t TN_GREEN[] = {   // Greensleeves (dan ca Anh)
  TN(N_A4,2), TN(N_C5,4),TN(N_D5,2),TN(N_E5,3),TN(N_F5,1),TN(N_E5,2), TN(N_D5,4),TN(N_B4,2),TN(N_G4,3),TN(N_A4,1),TN(N_B4,2), TN(N_C5,4),TN(N_A4,2),TN(N_A4,3),TN(N_Gs4,1),TN(N_A4,2),
  TN(N_B4,4),TN(N_Gs4,2),TN(N_E4,6), TN(N_A4,2), TN(N_C5,4),TN(N_D5,2),TN(N_E5,3),TN(N_F5,1),TN(N_E5,2), TN(N_D5,4),TN(N_B4,2),TN(N_G4,3),TN(N_A4,1),TN(N_B4,2),
  TN(N_C5,3),TN(N_B4,1),TN(N_A4,2),TN(N_Gs4,3),TN(N_Fs4,1),TN(N_Gs4,2), TN(N_A4,8) };
static const tnote_t TN_MARY[] = {   // Mary Had a Little Lamb
  TN(N_E4,4),TN(N_D4,4),TN(N_C4,4),TN(N_D4,4),TN(N_E4,4),TN(N_E4,4),TN(N_E4,8), TN(N_D4,4),TN(N_D4,4),TN(N_D4,8), TN(N_E4,4),TN(N_G4,4),TN(N_G4,8),
  TN(N_E4,4),TN(N_D4,4),TN(N_C4,4),TN(N_D4,4),TN(N_E4,4),TN(N_E4,4),TN(N_E4,4),TN(N_E4,4), TN(N_D4,4),TN(N_D4,4),TN(N_E4,4),TN(N_D4,4),TN(N_C4,16) };
static const tnote_t TN_CANON[] = {   // Canon in D (Pachelbel, motif chinh)
  TN(N_Fs5,8),TN(N_E5,8),TN(N_D5,8),TN(N_Cs5,8),TN(N_B4,8),TN(N_A4,8),TN(N_B4,8),TN(N_Cs5,8), TN(N_D5,16) };
// ---- v16: bai moi (9..19) ----
static const tnote_t TN_MACDONALD[] = {   // Old MacDonald Had a Farm (dan ca, phan chinh)
  TN(N_C4,4),TN(N_C4,4),TN(N_C4,4),TN(N_G3,4),TN(N_A3,4),TN(N_A3,4),TN(N_G3,8),TN(N_E4,4),
  TN(N_E4,4),TN(N_D4,4),TN(N_D4,4),TN(N_C4,16),TN(N_C4,4),TN(N_C4,4),TN(N_C4,4),TN(N_G3,4),
  TN(N_A3,4),TN(N_A3,4),TN(N_G3,8),TN(N_E4,4),TN(N_E4,4),TN(N_D4,4),TN(N_D4,4),TN(N_C4,16) };
static const tnote_t TN_ROW[] = {   // Row Row Row Your Boat
  TN(N_C4,6),TN(N_C4,6),TN(N_C4,4),TN(N_D4,2),TN(N_E4,6),TN(N_E4,4),TN(N_D4,2),TN(N_E4,4),
  TN(N_F4,2),TN(N_G4,12),TN(N_C5,2),TN(N_C5,2),TN(N_C5,2),TN(N_G4,2),TN(N_G4,2),TN(N_G4,2),
  TN(N_E4,2),TN(N_E4,2),TN(N_E4,2),TN(N_C4,2),TN(N_C4,2),TN(N_C4,2),TN(N_G4,4),TN(N_F4,2),
  TN(N_E4,4),TN(N_D4,2),TN(N_C4,12) };
static const tnote_t TN_LONDON[] = {   // London Bridge Is Falling Down
  TN(N_G4,3),TN(N_A4,1),TN(N_G4,4),TN(N_F4,4),TN(N_E4,4),TN(N_F4,4),TN(N_G4,8),TN(N_R,4),
  TN(N_D4,4),TN(N_E4,4),TN(N_F4,8),TN(N_E4,4),TN(N_F4,4),TN(N_G4,8),TN(N_G4,3),TN(N_A4,1),
  TN(N_G4,4),TN(N_F4,4),TN(N_E4,4),TN(N_F4,4),TN(N_G4,8),TN(N_R,4),TN(N_D4,8),TN(N_G4,8),
  TN(N_E4,4),TN(N_C4,12) };
static const tnote_t TN_BUNS[] = {   // Hot Cross Buns
  TN(N_E4,4),TN(N_D4,4),TN(N_C4,8),TN(N_E4,4),TN(N_D4,4),TN(N_C4,8),TN(N_C4,2),TN(N_C4,2),
  TN(N_C4,2),TN(N_C4,2),TN(N_D4,2),TN(N_D4,2),TN(N_D4,2),TN(N_D4,2),TN(N_E4,4),TN(N_D4,4),
  TN(N_C4,8) };
static const tnote_t TN_YANKEE[] = {   // Yankee Doodle
  TN(N_C4,4),TN(N_C4,4),TN(N_D4,4),TN(N_E4,4),TN(N_C4,4),TN(N_E4,4),TN(N_D4,4),TN(N_G3,4),
  TN(N_C4,4),TN(N_C4,4),TN(N_D4,4),TN(N_E4,4),TN(N_C4,8),TN(N_B3,8),TN(N_C4,4),TN(N_C4,4),
  TN(N_D4,4),TN(N_E4,4),TN(N_F4,4),TN(N_E4,4),TN(N_D4,4),TN(N_C4,4),TN(N_B3,4),TN(N_G3,4),
  TN(N_A3,4),TN(N_B3,4),TN(N_C4,4),TN(N_C4,12) };
static const tnote_t TN_LIGHTLY[] = {   // Lightly Row (dan ca Duc)
  TN(N_G4,4),TN(N_E4,4),TN(N_E4,8),TN(N_F4,4),TN(N_D4,4),TN(N_D4,8),TN(N_C4,4),TN(N_D4,4),
  TN(N_E4,4),TN(N_F4,4),TN(N_G4,4),TN(N_G4,4),TN(N_G4,8),TN(N_G4,4),TN(N_E4,4),TN(N_E4,8),
  TN(N_F4,4),TN(N_D4,4),TN(N_D4,8),TN(N_C4,4),TN(N_E4,4),TN(N_G4,4),TN(N_G4,4),TN(N_C4,16) };
static const tnote_t TN_CLAIR[] = {   // Au Clair de la Lune (dan ca Phap)
  TN(N_C4,4),TN(N_C4,4),TN(N_C4,4),TN(N_D4,4),TN(N_E4,8),TN(N_D4,8),TN(N_C4,4),TN(N_E4,4),
  TN(N_D4,4),TN(N_D4,4),TN(N_C4,16),TN(N_C4,4),TN(N_C4,4),TN(N_C4,4),TN(N_D4,4),TN(N_E4,8),
  TN(N_D4,8),TN(N_C4,4),TN(N_E4,4),TN(N_D4,4),TN(N_D4,4),TN(N_C4,16) };
static const tnote_t TN_SURPRISE[] = {   // Surprise Symphony (Haydn)
  TN(N_C4,4),TN(N_C4,4),TN(N_E4,4),TN(N_E4,4),TN(N_G4,4),TN(N_G4,4),TN(N_E4,8),TN(N_F4,4),
  TN(N_F4,4),TN(N_D4,4),TN(N_D4,4),TN(N_B3,4),TN(N_B3,4),TN(N_G3,8),TN(N_C4,4),TN(N_C4,4),
  TN(N_E4,4),TN(N_E4,4),TN(N_G4,4),TN(N_G4,4),TN(N_E4,8),TN(N_F4,4),TN(N_F4,4),TN(N_D4,4),
  TN(N_D4,4),TN(N_B3,4),TN(N_B3,4),TN(N_G3,8) };
static const tnote_t TN_SILENT[] = {   // Silent Night
  TN(N_G4,6),TN(N_A4,2),TN(N_G4,4),TN(N_E4,12),TN(N_G4,6),TN(N_A4,2),TN(N_G4,4),TN(N_E4,12),
  TN(N_D5,8),TN(N_D5,4),TN(N_B4,12),TN(N_C5,8),TN(N_C5,4),TN(N_G4,12),TN(N_A4,8),TN(N_A4,4),
  TN(N_C5,6),TN(N_B4,2),TN(N_A4,4),TN(N_G4,6),TN(N_A4,2),TN(N_G4,4),TN(N_E4,12),TN(N_A4,8),
  TN(N_A4,4),TN(N_C5,6),TN(N_B4,2),TN(N_A4,4),TN(N_G4,6),TN(N_A4,2),TN(N_G4,4),TN(N_E4,12),
  TN(N_D5,8),TN(N_D5,4),TN(N_F5,6),TN(N_D5,2),TN(N_B4,4),TN(N_C5,12),TN(N_E5,12),TN(N_C5,4),
  TN(N_G4,4),TN(N_E4,4),TN(N_G4,6),TN(N_F4,2),TN(N_D4,4),TN(N_C4,12) };
static const tnote_t TN_DECK[] = {   // Deck the Halls
  TN(N_G4,6),TN(N_F4,2),TN(N_E4,4),TN(N_D4,4),TN(N_C4,4),TN(N_D4,4),TN(N_E4,4),TN(N_C4,4),
  TN(N_D4,2),TN(N_E4,2),TN(N_F4,2),TN(N_D4,2),TN(N_E4,6),TN(N_D4,2),TN(N_C4,4),TN(N_B3,4),
  TN(N_C4,8),TN(N_G4,6),TN(N_F4,2),TN(N_E4,4),TN(N_D4,4),TN(N_C4,4),TN(N_D4,4),TN(N_E4,4),
  TN(N_C4,4),TN(N_D4,2),TN(N_E4,2),TN(N_F4,2),TN(N_D4,2),TN(N_E4,6),TN(N_D4,2),TN(N_C4,4),
  TN(N_B3,4),TN(N_C4,8) };
static const tnote_t TN_XMAS[] = {   // We Wish You a Merry Christmas
  TN(N_D4,4),TN(N_G4,4),TN(N_G4,2),TN(N_A4,2),TN(N_G4,2),TN(N_Fs4,2),TN(N_E4,4),TN(N_E4,4),
  TN(N_E4,4),TN(N_A4,4),TN(N_A4,2),TN(N_B4,2),TN(N_A4,2),TN(N_G4,2),TN(N_Fs4,4),TN(N_D4,4),
  TN(N_D4,4),TN(N_B4,4),TN(N_B4,2),TN(N_C5,2),TN(N_B4,2),TN(N_A4,2),TN(N_G4,4),TN(N_E4,4),
  TN(N_D4,2),TN(N_D4,2),TN(N_E4,4),TN(N_A4,4),TN(N_Fs4,4),TN(N_G4,12) };
// ---- v21: bai moi (20..27), deu la nhac co dien / dan ca (het ban quyen), viet tu tri nho o dang rut gon. Nốt chi dung G3..G5 (+1 quang tam = G4..G6) va khong dung G#3 / A#3 / G#5 de phim layer MUSIC choi lai duoc ----
static const tnote_t TN_TURKISH[] = {   // Rondo alla Turca (Mozart, chu de chinh, hat xuong 1 quang 5 de nằm trong G4..G5)
  TN(N_E4,1),TN(N_D4,1),TN(N_Cs4,1),TN(N_D4,1),TN(N_F4,4),TN(N_G4,1),TN(N_F4,1),TN(N_E4,1),TN(N_F4,1),TN(N_A4,4),TN(N_As4,1),TN(N_A4,1),TN(N_Gs4,1),TN(N_A4,1),TN(N_E5,1),TN(N_D5,1),TN(N_Cs5,1),TN(N_D5,1),TN(N_E5,1),TN(N_D5,1),TN(N_Cs5,1),TN(N_D5,1),TN(N_F5,4),TN(N_D5,4),TN(N_E5,2),TN(N_D5,2),TN(N_D5,8),TN(N_E4,1),TN(N_D4,1),TN(N_Cs4,1),TN(N_D4,1),TN(N_F4,4),TN(N_G4,1),TN(N_F4,1),TN(N_E4,1),TN(N_F4,1),TN(N_A4,4),TN(N_As4,1),TN(N_A4,1),TN(N_Gs4,1),TN(N_A4,1),TN(N_E5,1),TN(N_D5,1),TN(N_Cs5,1),TN(N_D5,1),TN(N_E5,1),TN(N_D5,1),TN(N_Cs5,1),TN(N_D5,1),TN(N_F5,4),TN(N_D5,4),TN(N_E5,2),TN(N_D5,2),TN(N_D5,8) };
static const tnote_t TN_KOROB[] = {   // Korobeiniki (dan ca Nga, ban nhac cua game Tetris la chinh bai nay)
  TN(N_E5,4),TN(N_B4,2),TN(N_C5,2),TN(N_D5,4),TN(N_C5,2),TN(N_B4,2),TN(N_A4,4),TN(N_A4,2),TN(N_C5,2),TN(N_E5,4),TN(N_D5,2),TN(N_C5,2),TN(N_B4,6),TN(N_C5,2),TN(N_D5,4),TN(N_E5,4),TN(N_C5,4),TN(N_A4,4),TN(N_A4,8),
  TN(N_D5,6),TN(N_F5,2),TN(N_A5,4),TN(N_G5,2),TN(N_F5,2),TN(N_E5,6),TN(N_C5,2),TN(N_E5,4),TN(N_D5,2),TN(N_C5,2),TN(N_B4,4),TN(N_B4,2),TN(N_C5,2),TN(N_D5,4),TN(N_E5,4),TN(N_C5,4),TN(N_A4,4),TN(N_A4,8) };
static const tnote_t TN_BEET5[] = {   // Giao huong so 5 (Beethoven, dong co mo dau)
  TN(N_G4,2),TN(N_G4,2),TN(N_G4,2),TN(N_Ds4,8),TN(N_R,2), TN(N_F4,2),TN(N_F4,2),TN(N_F4,2),TN(N_D4,8),TN(N_R,2),
  TN(N_Gs4,2),TN(N_Gs4,2),TN(N_Gs4,2),TN(N_F4,8),TN(N_R,2), TN(N_G4,2),TN(N_G4,2),TN(N_G4,2),TN(N_Ds4,8),TN(N_R,2),
  TN(N_F4,2),TN(N_F4,2),TN(N_F4,2),TN(N_D4,8),TN(N_R,2), TN(N_G4,2),TN(N_G4,2),TN(N_G4,2),TN(N_Ds4,12) };
static const tnote_t TN_MINUET[] = {   // Minuet in G (Petzold, thuong gan cho Bach)
  TN(N_D5,4),TN(N_G4,2),TN(N_A4,2),TN(N_B4,2),TN(N_C5,2), TN(N_D5,4),TN(N_G4,4),TN(N_G4,4), TN(N_E5,4),TN(N_C5,2),TN(N_D5,2),TN(N_E5,2),TN(N_Fs5,2), TN(N_G5,4),TN(N_G4,4),TN(N_G4,4),
  TN(N_C5,4),TN(N_D5,2),TN(N_C5,2),TN(N_B4,2),TN(N_A4,2), TN(N_B4,4),TN(N_C5,2),TN(N_B4,2),TN(N_A4,2),TN(N_G4,2), TN(N_Fs4,4),TN(N_G4,2),TN(N_A4,2),TN(N_B4,2),TN(N_G4,2), TN(N_B4,12) };
static const tnote_t TN_SAKURA[] = {   // Sakura (dan ca Nhat Ban)
  TN(N_A4,4),TN(N_A4,4),TN(N_B4,8), TN(N_A4,4),TN(N_A4,4),TN(N_B4,8), TN(N_A4,4),TN(N_B4,4),TN(N_C5,4),TN(N_B4,4), TN(N_A4,4),TN(N_B4,2),TN(N_A4,2),TN(N_F4,4),TN(N_E4,12),
  TN(N_A4,4),TN(N_A4,4),TN(N_B4,8), TN(N_A4,4),TN(N_A4,4),TN(N_B4,8), TN(N_A4,4),TN(N_B4,4),TN(N_C5,4),TN(N_B4,4), TN(N_A4,4),TN(N_B4,2),TN(N_A4,2),TN(N_F4,4),TN(N_E4,12) };
static const tnote_t TN_SUSANNA[] = {   // Oh! Susanna (Stephen Foster, 1848)
  TN(N_C4,2),TN(N_D4,2),TN(N_E4,4),TN(N_G4,4),TN(N_G4,6),TN(N_A4,2),TN(N_G4,4),TN(N_E4,4),TN(N_C4,2),TN(N_D4,2),TN(N_E4,4),TN(N_E4,4),TN(N_D4,6),TN(N_C4,2),TN(N_D4,8),
  TN(N_C4,2),TN(N_D4,2),TN(N_E4,4),TN(N_G4,4),TN(N_G4,6),TN(N_A4,2),TN(N_G4,4),TN(N_E4,4),TN(N_C4,2),TN(N_D4,2),TN(N_E4,4),TN(N_E4,4),TN(N_D4,4),TN(N_D4,4),TN(N_C4,8) };
static const tnote_t TN_ITSY[] = {   // Itsy Bitsy Spider (dong dao)
  TN(N_C4,2),TN(N_F4,4),TN(N_F4,2),TN(N_F4,4),TN(N_G4,2),TN(N_A4,4),TN(N_A4,2),TN(N_A4,4),TN(N_G4,2),TN(N_F4,4),TN(N_G4,2),TN(N_A4,4),TN(N_F4,8),
  TN(N_C4,2),TN(N_F4,4),TN(N_F4,2),TN(N_F4,4),TN(N_G4,2),TN(N_A4,4),TN(N_A4,2),TN(N_A4,4),TN(N_G4,2),TN(N_F4,4),TN(N_G4,2),TN(N_A4,4),TN(N_F4,8) };
static const tnote_t TN_CUCA[] = {   // La Cucaracha (dan ca Tay Ban Nha)
  TN(N_C4,2),TN(N_C4,2),TN(N_C4,2),TN(N_F4,6),TN(N_A4,4), TN(N_C4,2),TN(N_C4,2),TN(N_C4,2),TN(N_F4,6),TN(N_A4,4),
  TN(N_F4,2),TN(N_F4,2),TN(N_E4,2),TN(N_E4,2),TN(N_D4,2),TN(N_D4,2),TN(N_C4,8) };
// ---- v22: bai moi (28..62): 10 bai nhom 1 + 12 bai co dien + 13 bai dan ca / nhac le, deu la nhac co truyen / het ban quyen, viet tu tri nho o dang rut gon. Nốt chi dung G3..G5 (+1 quang tam = G4..G6) va khong dung G#3 / A#3 / G#5 de phim layer MUSIC choi lai duoc ----
static const tnote_t TN_MTKING[] = {   // Mountain King
  TN(N_A3,2),TN(N_B3,2),TN(N_C4,2),TN(N_D4,2),TN(N_E4,2),TN(N_C4,2),TN(N_E4,4),TN(N_R,2),TN(N_Ds4,2),TN(N_B3,2),TN(N_Ds4,2),TN(N_B3,2),TN(N_Ds4,4),TN(N_R,2),TN(N_A3,2),TN(N_B3,2),
  TN(N_C4,2),TN(N_D4,2),TN(N_E4,2),TN(N_C4,2),TN(N_E4,4),TN(N_A4,4),TN(N_G4,2),TN(N_E4,2),TN(N_C4,2),TN(N_E4,2),TN(N_G4,4),TN(N_R,4),TN(N_A3,1),TN(N_B3,1),TN(N_C4,1),TN(N_D4,1),
  TN(N_E4,1),TN(N_C4,1),TN(N_E4,2),TN(N_R,1),TN(N_Ds4,1),TN(N_B3,1),TN(N_Ds4,1),TN(N_B3,1),TN(N_Ds4,2),TN(N_R,1),TN(N_A3,1),TN(N_B3,1),TN(N_C4,1),TN(N_D4,1),TN(N_E4,1),TN(N_C4,1),
  TN(N_E4,2),TN(N_A4,2),TN(N_G4,1),TN(N_E4,1),TN(N_C4,1),TN(N_E4,1),TN(N_G4,2),TN(N_A4,8) };
static const tnote_t TN_SAILOR[] = {   // Drunk Sailor
  TN(N_A4,2),TN(N_A4,1),TN(N_A4,1),TN(N_A4,2),TN(N_A4,1),TN(N_A4,1),TN(N_A4,2),TN(N_D4,2),TN(N_F4,2),TN(N_A4,2),TN(N_G4,2),TN(N_G4,1),TN(N_G4,1),TN(N_G4,2),TN(N_G4,1),TN(N_G4,1),
  TN(N_G4,2),TN(N_C4,2),TN(N_E4,2),TN(N_G4,2),TN(N_A4,2),TN(N_A4,1),TN(N_A4,1),TN(N_A4,2),TN(N_A4,1),TN(N_A4,1),TN(N_A4,2),TN(N_B4,2),TN(N_C5,2),TN(N_D5,2),TN(N_C5,2),TN(N_A4,2),
  TN(N_G4,2),TN(N_E4,2),TN(N_D4,4),TN(N_D4,4),TN(N_A4,4),TN(N_A4,3),TN(N_A4,1),TN(N_A4,2),TN(N_D4,2),TN(N_F4,2),TN(N_A4,2),TN(N_G4,4),TN(N_G4,3),TN(N_G4,1),TN(N_G4,2),TN(N_C4,2),
  TN(N_E4,2),TN(N_G4,2),TN(N_A4,4),TN(N_A4,3),TN(N_A4,1),TN(N_A4,2),TN(N_B4,2),TN(N_C5,2),TN(N_D5,2),TN(N_C5,2),TN(N_A4,2),TN(N_G4,2),TN(N_E4,2),TN(N_D4,4),TN(N_D4,6) };
static const tnote_t TN_TELL[] = {   // William Tell
  TN(N_B4,2),TN(N_B4,2),TN(N_B4,2),TN(N_E5,6),TN(N_R,2),TN(N_B4,2),TN(N_B4,2),TN(N_B4,2),TN(N_E5,6),TN(N_R,2),TN(N_B4,2),TN(N_B4,2),TN(N_B4,2),TN(N_E5,2),TN(N_Gs4,2),TN(N_B4,2),
  TN(N_E5,4),TN(N_R,4),TN(N_E5,2),TN(N_E5,1),TN(N_E5,1),TN(N_E5,2),TN(N_E5,1),TN(N_E5,1),TN(N_E5,2),TN(N_B4,2),TN(N_E5,4),TN(N_R,4),TN(N_Fs5,2),TN(N_Fs5,1),TN(N_Fs5,1),TN(N_Fs5,2),
  TN(N_Fs5,1),TN(N_Fs5,1),TN(N_Fs5,2),TN(N_B4,2),TN(N_Fs5,4),TN(N_R,4),TN(N_E5,2),TN(N_E5,1),TN(N_E5,1),TN(N_E5,2),TN(N_E5,1),TN(N_E5,1),TN(N_E5,2),TN(N_Gs4,2),TN(N_B4,2),TN(N_E5,2),
  TN(N_B4,2),TN(N_B4,1),TN(N_B4,1),TN(N_B4,2),TN(N_E5,8) };
static const tnote_t TN_CANCAN[] = {   // Can-Can
  TN(N_E5,4),TN(N_C5,4),TN(N_A4,4),TN(N_G4,4),TN(N_G4,4),TN(N_D4,2),TN(N_E4,2),TN(N_F4,2),TN(N_E4,2),TN(N_D4,4),TN(N_C4,4),TN(N_E5,4),TN(N_C5,4),TN(N_A4,4),TN(N_G4,4),TN(N_Fs4,4),
  TN(N_G4,4),TN(N_A4,2),TN(N_B4,2),TN(N_C5,4),TN(N_G4,2),TN(N_B4,2),TN(N_G4,2),TN(N_C5,4),TN(N_G4,2),TN(N_B4,2),TN(N_G4,2),TN(N_C5,6),TN(N_R,2),TN(N_A4,2),TN(N_B4,2),TN(N_C5,4),
  TN(N_G4,2),TN(N_B4,2),TN(N_G4,2),TN(N_C5,4),TN(N_G4,2),TN(N_B4,2),TN(N_G4,2),TN(N_C5,4),TN(N_E5,4),TN(N_C5,8) };
static const tnote_t TN_WEDDING[] = {   // Wedding March
  TN(N_G4,2),TN(N_C5,6),TN(N_C5,2),TN(N_C5,2),TN(N_E5,6),TN(N_E5,2),TN(N_G5,4),TN(N_E5,4),TN(N_C5,4),TN(N_R,2),TN(N_G4,2),TN(N_C5,6),TN(N_C5,2),TN(N_C5,2),TN(N_E5,6),TN(N_E5,2),
  TN(N_G5,4),TN(N_E5,4),TN(N_C5,4),TN(N_R,2),TN(N_G5,2),TN(N_G5,2),TN(N_G5,2),TN(N_E5,4),TN(N_C5,2),TN(N_D5,2),TN(N_E5,4),TN(N_D5,2),TN(N_C5,2),TN(N_D5,4),TN(N_G4,4),TN(N_C5,8) };
static const tnote_t TN_GRACE[] = {   // Amazing Grace
  TN(N_D4,4),TN(N_G4,8),TN(N_B4,2),TN(N_G4,2),TN(N_B4,8),TN(N_A4,4),TN(N_G4,8),TN(N_E4,4),TN(N_D4,8),TN(N_D4,4),TN(N_G4,8),TN(N_B4,2),TN(N_G4,2),TN(N_B4,8),TN(N_A4,4),TN(N_D5,12),
  TN(N_B4,4),TN(N_D5,8),TN(N_D5,2),TN(N_B4,2),TN(N_D5,8),TN(N_B4,4),TN(N_G4,8),TN(N_D4,4),TN(N_E4,8),TN(N_D4,2),TN(N_E4,2),TN(N_G4,8),TN(N_E4,4),TN(N_G4,12) };
static const tnote_t TN_AULD[] = {   // Auld Lang
  TN(N_D4,4),TN(N_G4,6),TN(N_G4,2),TN(N_G4,4),TN(N_B4,4),TN(N_A4,6),TN(N_G4,2),TN(N_A4,4),TN(N_B4,2),TN(N_A4,2),TN(N_G4,6),TN(N_G4,2),TN(N_B4,4),TN(N_D5,4),TN(N_E5,12),TN(N_R,2),
  TN(N_E5,4),TN(N_D5,6),TN(N_B4,2),TN(N_B4,4),TN(N_G4,4),TN(N_A4,6),TN(N_G4,2),TN(N_A4,4),TN(N_B4,2),TN(N_A4,2),TN(N_G4,6),TN(N_E4,2),TN(N_E4,4),TN(N_D4,4),TN(N_G4,12) };
static const tnote_t TN_KALINKA[] = {   // Kalinka
  TN(N_Fs5,4),TN(N_E5,4),TN(N_Cs5,2),TN(N_D5,2),TN(N_E5,4),TN(N_Cs5,2),TN(N_D5,2),TN(N_E5,2),TN(N_D5,2),TN(N_Cs5,2),TN(N_B4,6),TN(N_R,2),TN(N_Fs5,4),TN(N_E5,4),TN(N_Cs5,2),TN(N_D5,2),
  TN(N_E5,4),TN(N_Cs5,2),TN(N_D5,2),TN(N_E5,2),TN(N_D5,2),TN(N_Cs5,2),TN(N_B4,6),TN(N_R,2),TN(N_B4,2),TN(N_B4,2),TN(N_A4,2),TN(N_B4,2),TN(N_D5,4),TN(N_D5,4),TN(N_Cs5,2),TN(N_Cs5,2),
  TN(N_B4,2),TN(N_Cs5,2),TN(N_E5,4),TN(N_E5,4),TN(N_D5,2),TN(N_D5,2),TN(N_Cs5,2),TN(N_D5,2),TN(N_Fs5,8),TN(N_B4,8) };
static const tnote_t TN_LYNGUAO[] = {   // Ly Ngua O
  TN(N_E5,2),TN(N_D5,2),TN(N_C5,4),TN(N_A4,4),TN(N_C5,2),TN(N_D5,2),TN(N_E5,4),TN(N_D5,4),TN(N_C5,2),TN(N_A4,2),TN(N_G4,4),TN(N_A4,8),TN(N_E5,2),TN(N_D5,2),TN(N_C5,4),TN(N_A4,4),
  TN(N_C5,2),TN(N_D5,2),TN(N_E5,4),TN(N_D5,4),TN(N_C5,2),TN(N_A4,2),TN(N_G4,4),TN(N_A4,8),TN(N_G4,2),TN(N_A4,2),TN(N_C5,4),TN(N_D5,2),TN(N_E5,2),TN(N_D5,4),TN(N_C5,2),TN(N_A4,2),
  TN(N_G4,4),TN(N_E4,4),TN(N_G4,2),TN(N_A4,2),TN(N_C5,4),TN(N_A4,12) };
static const tnote_t TN_BACKIM[] = {   // Bac Kim Thang
  TN(N_G4,4),TN(N_G4,4),TN(N_A4,4),TN(N_G4,4),TN(N_E4,4),TN(N_G4,8),TN(N_G4,4),TN(N_G4,4),TN(N_A4,4),TN(N_G4,4),TN(N_E4,4),TN(N_G4,8),TN(N_C5,4),TN(N_C5,4),TN(N_D5,4),TN(N_C5,4),
  TN(N_A4,4),TN(N_C5,8),TN(N_G4,4),TN(N_A4,4),TN(N_G4,4),TN(N_E4,4),TN(N_D4,4),TN(N_C4,12) };
static const tnote_t TN_KLEINE[] = {   // Kleine Nacht
  TN(N_G4,4),TN(N_R,2),TN(N_D4,2),TN(N_G4,4),TN(N_R,2),TN(N_D4,2),TN(N_G4,2),TN(N_D4,2),TN(N_G4,2),TN(N_B4,2),TN(N_D5,8),TN(N_C5,4),TN(N_R,2),TN(N_A4,2),TN(N_C5,4),TN(N_R,2),
  TN(N_A4,2),TN(N_C5,2),TN(N_A4,2),TN(N_Fs4,2),TN(N_A4,2),TN(N_D4,8),TN(N_G4,4),TN(N_R,2),TN(N_D4,2),TN(N_G4,4),TN(N_R,2),TN(N_D4,2),TN(N_G4,2),TN(N_D4,2),TN(N_G4,2),TN(N_B4,2),
  TN(N_D5,8),TN(N_C5,4),TN(N_R,2),TN(N_A4,2),TN(N_C5,4),TN(N_R,2),TN(N_A4,2),TN(N_C5,2),TN(N_A4,2),TN(N_Fs4,2),TN(N_A4,2),TN(N_D4,8),TN(N_B4,4),TN(N_G4,2),TN(N_B4,2),TN(N_D5,6),
  TN(N_B4,2),TN(N_A4,2),TN(N_G4,2),TN(N_Fs4,2),TN(N_A4,2),TN(N_G4,8) };
static const tnote_t TN_HABANERA[] = {   // Habanera
  TN(N_D5,6),TN(N_D5,2),TN(N_Cs5,2),TN(N_C5,2),TN(N_B4,2),TN(N_As4,2),TN(N_A4,6),TN(N_R,2),TN(N_D5,6),TN(N_D5,2),TN(N_Cs5,2),TN(N_C5,2),TN(N_B4,2),TN(N_As4,2),TN(N_A4,6),TN(N_R,2),
  TN(N_A4,2),TN(N_A4,2),TN(N_F4,2),TN(N_A4,2),TN(N_D5,6),TN(N_R,2),TN(N_A4,2),TN(N_A4,2),TN(N_F4,2),TN(N_A4,2),TN(N_D5,6),TN(N_R,2),TN(N_D5,6),TN(N_D5,2),TN(N_Cs5,2),TN(N_C5,2),
  TN(N_B4,2),TN(N_As4,2),TN(N_A4,4),TN(N_F4,4),TN(N_D4,4),TN(N_A4,2),TN(N_D4,10) };
static const tnote_t TN_HUNG5[] = {   // Hung Dance 5
  TN(N_G4,2),TN(N_A4,2),TN(N_As4,6),TN(N_As4,2),TN(N_C5,4),TN(N_As4,2),TN(N_A4,2),TN(N_A4,6),TN(N_A4,2),TN(N_G4,2),TN(N_A4,2),TN(N_As4,6),TN(N_As4,2),TN(N_C5,4),TN(N_As4,2),TN(N_A4,2),
  TN(N_G4,8),TN(N_D5,2),TN(N_C5,2),TN(N_As4,4),TN(N_As4,2),TN(N_As4,2),TN(N_A4,4),TN(N_G4,4),TN(N_F4,2),TN(N_G4,2),TN(N_A4,6),TN(N_A4,2),TN(N_As4,4),TN(N_A4,2),TN(N_G4,2),TN(N_G4,8),
  TN(N_R,2) };
static const tnote_t TN_DANUBE[] = {   // Blue Danube
  TN(N_D4,4),TN(N_Fs4,4),TN(N_A4,8),TN(N_A4,4),TN(N_R,4),TN(N_D4,4),TN(N_Fs4,4),TN(N_A4,8),TN(N_A4,4),TN(N_R,4),TN(N_E4,4),TN(N_G4,4),TN(N_B4,8),TN(N_B4,4),TN(N_R,4),TN(N_E4,4),
  TN(N_G4,4),TN(N_B4,8),TN(N_B4,4),TN(N_R,4),TN(N_D5,8),TN(N_A4,4),TN(N_B4,8),TN(N_G4,4),TN(N_A4,12),TN(N_R,4),TN(N_D5,8),TN(N_A4,4),TN(N_B4,8),TN(N_G4,4),TN(N_D4,12) };
static const tnote_t TN_RADETZ[] = {   // Radetzky
  TN(N_G4,2),TN(N_G4,2),TN(N_C5,4),TN(N_C5,2),TN(N_C5,2),TN(N_E5,4),TN(N_E5,2),TN(N_E5,2),TN(N_G5,4),TN(N_G5,2),TN(N_G5,2),TN(N_E5,4),TN(N_C5,4),TN(N_G4,4),TN(N_C5,8),TN(N_G4,2),
  TN(N_G4,2),TN(N_C5,4),TN(N_C5,2),TN(N_C5,2),TN(N_E5,4),TN(N_E5,2),TN(N_E5,2),TN(N_G5,4),TN(N_G5,2),TN(N_G5,2),TN(N_E5,4),TN(N_C5,4),TN(N_G4,4),TN(N_C5,8),TN(N_E5,4),TN(N_E5,2),
  TN(N_E5,2),TN(N_D5,4),TN(N_D5,2),TN(N_D5,2),TN(N_C5,4),TN(N_C5,2),TN(N_C5,2),TN(N_B4,4),TN(N_G4,4),TN(N_C5,4),TN(N_R,4) };
static const tnote_t TN_SWAN[] = {   // Swan Lake
  TN(N_B4,4),TN(N_Fs5,8),TN(N_Fs5,2),TN(N_G5,2),TN(N_Fs5,2),TN(N_E5,2),TN(N_D5,4),TN(N_Cs5,4),TN(N_B4,8),TN(N_R,4),TN(N_D5,4),TN(N_Fs5,8),TN(N_Fs5,2),TN(N_G5,2),TN(N_Fs5,2),TN(N_E5,2),
  TN(N_D5,4),TN(N_Cs5,4),TN(N_B4,12),TN(N_R,4),TN(N_E5,4),TN(N_G5,8),TN(N_G5,2),TN(N_Fs5,2),TN(N_E5,2),TN(N_D5,2),TN(N_Cs5,4),TN(N_B4,4),TN(N_A4,8),TN(N_R,4),TN(N_B4,4),TN(N_Fs5,8),
  TN(N_Fs5,2),TN(N_G5,2),TN(N_Fs5,2),TN(N_E5,2),TN(N_D5,4),TN(N_Cs5,4),TN(N_B4,12) };
static const tnote_t TN_HALLEL[] = {   // Hallelujah
  TN(N_G4,4),TN(N_G4,2),TN(N_G4,2),TN(N_B4,4),TN(N_B4,4),TN(N_D5,8),TN(N_R,2),TN(N_D5,4),TN(N_D5,2),TN(N_D5,2),TN(N_E5,4),TN(N_E5,4),TN(N_G5,8),TN(N_R,2),TN(N_G5,4),TN(N_Fs5,2),
  TN(N_E5,2),TN(N_D5,4),TN(N_B4,4),TN(N_C5,4),TN(N_A4,4),TN(N_G4,12),TN(N_G4,4),TN(N_G4,2),TN(N_G4,2),TN(N_B4,4),TN(N_B4,4),TN(N_D5,8),TN(N_R,2),TN(N_D5,4),TN(N_D5,2),TN(N_D5,2),
  TN(N_E5,4),TN(N_E5,4),TN(N_G5,8),TN(N_R,2),TN(N_G5,4),TN(N_Fs5,2),TN(N_E5,2),TN(N_D5,4),TN(N_B4,4),TN(N_C5,4),TN(N_A4,4),TN(N_G4,12) };
static const tnote_t TN_AIRG[] = {   // Air On G
  TN(N_Fs5,12),TN(N_E5,2),TN(N_D5,2),TN(N_Cs5,8),TN(N_B4,4),TN(N_A4,4),TN(N_G4,2),TN(N_Fs4,2),TN(N_E4,4),TN(N_A4,12),TN(N_R,4),TN(N_B4,12),TN(N_A4,2),TN(N_G4,2),TN(N_Fs4,8),TN(N_E4,4),
  TN(N_D4,12),TN(N_R,4),TN(N_Fs5,12),TN(N_E5,2),TN(N_D5,2),TN(N_Cs5,8),TN(N_B4,4),TN(N_A4,4),TN(N_G4,2),TN(N_Fs4,2),TN(N_E4,4),TN(N_A4,12),TN(N_R,4),TN(N_B4,12),TN(N_A4,2),TN(N_G4,2),
  TN(N_Fs4,8),TN(N_E4,4),TN(N_D4,12) };
static const tnote_t TN_BRAHMS[] = {   // Brahms Lullby
  TN(N_E4,2),TN(N_E4,2),TN(N_G4,8),TN(N_E4,2),TN(N_E4,2),TN(N_G4,8),TN(N_E4,2),TN(N_G4,2),TN(N_C5,4),TN(N_B4,4),TN(N_A4,4),TN(N_A4,4),TN(N_G4,8),TN(N_R,4),TN(N_D4,2),TN(N_E4,2),
  TN(N_F4,8),TN(N_D4,2),TN(N_E4,2),TN(N_F4,8),TN(N_D4,2),TN(N_F4,2),TN(N_B4,4),TN(N_A4,4),TN(N_G4,4),TN(N_B4,4),TN(N_C5,12) };
static const tnote_t TN_VALKYR[] = {   // Valkyries
  TN(N_A4,6),TN(N_A4,2),TN(N_C5,4),TN(N_A4,6),TN(N_A4,2),TN(N_C5,4),TN(N_A4,6),TN(N_A4,2),TN(N_C5,4),TN(N_E5,4),TN(N_C5,4),TN(N_A4,4),TN(N_E5,8),TN(N_R,4),TN(N_B4,6),TN(N_B4,2),
  TN(N_D5,4),TN(N_B4,6),TN(N_B4,2),TN(N_D5,4),TN(N_B4,6),TN(N_B4,2),TN(N_D5,4),TN(N_Fs5,4),TN(N_D5,4),TN(N_B4,4),TN(N_Fs5,8),TN(N_R,4),TN(N_A4,6),TN(N_A4,2),TN(N_C5,4),TN(N_A4,6),
  TN(N_A4,2),TN(N_C5,4),TN(N_E5,6),TN(N_E5,2),TN(N_A4,4),TN(N_E5,12) };
static const tnote_t TN_SPRING[] = {   // Spring
  TN(N_E4,4),TN(N_Gs4,2),TN(N_Gs4,2),TN(N_Gs4,2),TN(N_Gs4,2),TN(N_Fs4,2),TN(N_Gs4,2),TN(N_A4,2),TN(N_B4,2),TN(N_B4,8),TN(N_R,2),TN(N_E4,4),TN(N_Gs4,2),TN(N_Gs4,2),TN(N_Gs4,2),TN(N_Gs4,2),
  TN(N_Fs4,2),TN(N_E4,2),TN(N_Fs4,2),TN(N_Gs4,2),TN(N_E4,8),TN(N_R,2),TN(N_B4,2),TN(N_B4,2),TN(N_B4,2),TN(N_B4,2),TN(N_Cs5,2),TN(N_Ds5,2),TN(N_E5,8),TN(N_R,2),TN(N_B4,2),TN(N_B4,2),
  TN(N_B4,2),TN(N_B4,2),TN(N_Cs5,2),TN(N_Ds5,2),TN(N_E5,8) };
static const tnote_t TN_GYMNO[] = {   // Gymnopedie 1
  TN(N_Fs4,8),TN(N_A4,4),TN(N_G4,8),TN(N_Fs4,4),TN(N_Cs4,8),TN(N_B3,4),TN(N_Cs4,4),TN(N_D4,4),TN(N_A3,4),TN(N_Fs4,12),TN(N_R,4),TN(N_Fs4,8),TN(N_A4,4),TN(N_G4,8),TN(N_Fs4,4),TN(N_Cs4,8),
  TN(N_B3,4),TN(N_Cs4,4),TN(N_D4,4),TN(N_A3,4),TN(N_D4,12) };
static const tnote_t TN_SCARB[] = {   // Scarborough
  TN(N_A4,4),TN(N_A4,4),TN(N_E5,8),TN(N_E5,4),TN(N_E5,4),TN(N_D5,4),TN(N_E5,4),TN(N_F5,4),TN(N_E5,12),TN(N_R,4),TN(N_A4,4),TN(N_A4,4),TN(N_E5,8),TN(N_E5,4),TN(N_E5,4),TN(N_D5,4),
  TN(N_C5,4),TN(N_B4,4),TN(N_A4,12) };
static const tnote_t TN_DANNY[] = {   // Danny Boy
  TN(N_G4,4),TN(N_C5,6),TN(N_C5,2),TN(N_D5,4),TN(N_E5,6),TN(N_D5,2),TN(N_C5,4),TN(N_D5,4),TN(N_E5,4),TN(N_G5,8),TN(N_R,4),TN(N_E5,4),TN(N_E5,6),TN(N_D5,2),TN(N_C5,4),TN(N_D5,4),
  TN(N_E5,4),TN(N_D5,8),TN(N_C5,4),TN(N_C5,12) };
static const tnote_t TN_SAINTS[] = {   // Saints
  TN(N_C4,2),TN(N_E4,2),TN(N_F4,2),TN(N_G4,8),TN(N_R,2),TN(N_C4,2),TN(N_E4,2),TN(N_F4,2),TN(N_G4,8),TN(N_R,2),TN(N_C4,2),TN(N_E4,2),TN(N_F4,2),TN(N_G4,4),TN(N_E4,4),TN(N_C4,4),
  TN(N_E4,4),TN(N_D4,8),TN(N_R,4),TN(N_E4,2),TN(N_E4,2),TN(N_D4,2),TN(N_C4,4),TN(N_C4,2),TN(N_E4,2),TN(N_G4,4),TN(N_G4,4),TN(N_F4,12),TN(N_E4,2),TN(N_F4,2),TN(N_G4,4),TN(N_E4,4),
  TN(N_C4,4),TN(N_D4,4),TN(N_C4,8) };
static const tnote_t TN_CLEMENT[] = {   // Clementine
  TN(N_D4,2),TN(N_D4,2),TN(N_G4,6),TN(N_G4,2),TN(N_G4,4),TN(N_B4,8),TN(N_B4,2),TN(N_B4,2),TN(N_A4,6),TN(N_A4,2),TN(N_A4,4),TN(N_G4,4),TN(N_A4,4),TN(N_B4,4),TN(N_G4,8),TN(N_R,4),
  TN(N_D4,2),TN(N_D4,2),TN(N_G4,6),TN(N_G4,2),TN(N_G4,4),TN(N_B4,8),TN(N_B4,2),TN(N_B4,2),TN(N_A4,6),TN(N_A4,2),TN(N_A4,4),TN(N_D5,4),TN(N_D5,4),TN(N_B4,4),TN(N_G4,12) };
static const tnote_t TN_CAMPTWN[] = {   // Camptown
  TN(N_G4,2),TN(N_G4,2),TN(N_E4,2),TN(N_G4,2),TN(N_A4,2),TN(N_G4,2),TN(N_E4,4),TN(N_C4,2),TN(N_D4,2),TN(N_E4,4),TN(N_D4,4),TN(N_C4,4),TN(N_R,4),TN(N_G4,2),TN(N_G4,2),TN(N_E4,2),
  TN(N_G4,2),TN(N_A4,2),TN(N_G4,2),TN(N_E4,4),TN(N_D4,2),TN(N_D4,2),TN(N_C4,2),TN(N_D4,2),TN(N_E4,2),TN(N_D4,2),TN(N_C4,4),TN(N_R,4),TN(N_G4,2),TN(N_G4,2),TN(N_E4,2),TN(N_G4,2),
  TN(N_A4,2),TN(N_G4,2),TN(N_E4,4),TN(N_D4,2),TN(N_E4,2),TN(N_D4,2),TN(N_C4,2),TN(N_C4,8) };
static const tnote_t TN_DIXIE[] = {   // Dixie
  TN(N_C4,2),TN(N_E4,2),TN(N_G4,4),TN(N_G4,2),TN(N_G4,2),TN(N_A4,2),TN(N_G4,2),TN(N_E4,4),TN(N_C4,2),TN(N_E4,2),TN(N_G4,4),TN(N_G4,2),TN(N_A4,2),TN(N_G4,2),TN(N_E4,2),TN(N_C4,8),
  TN(N_G4,4),TN(N_G4,2),TN(N_G4,2),TN(N_C5,4),TN(N_C5,2),TN(N_C5,2),TN(N_B4,2),TN(N_A4,2),TN(N_G4,4),TN(N_E4,4),TN(N_G4,2),TN(N_A4,2),TN(N_G4,2),TN(N_E4,2),TN(N_C4,8) };
static const tnote_t TN_JOYW[] = {   // Joy To World
  TN(N_C5,4),TN(N_B4,3),TN(N_A4,1),TN(N_G4,6),TN(N_F4,2),TN(N_E4,4),TN(N_D4,4),TN(N_C4,6),TN(N_G4,2),TN(N_A4,4),TN(N_A4,4),TN(N_B4,4),TN(N_B4,4),TN(N_C5,12),TN(N_C5,2),TN(N_C5,2),
  TN(N_B4,6),TN(N_A4,2),TN(N_G4,6),TN(N_G4,2),TN(N_F4,2),TN(N_E4,2),TN(N_C5,2),TN(N_C5,2),TN(N_B4,6),TN(N_A4,2),TN(N_G4,6),TN(N_G4,2),TN(N_F4,2),TN(N_E4,2),TN(N_E4,2),TN(N_E4,2),
  TN(N_E4,2),TN(N_E4,2),TN(N_E4,2),TN(N_F4,2),TN(N_G4,6),TN(N_F4,2),TN(N_E4,2),TN(N_D4,2),TN(N_D4,2),TN(N_D4,2),TN(N_D4,2),TN(N_E4,2),TN(N_F4,6),TN(N_E4,2),TN(N_D4,2),TN(N_C4,2),
  TN(N_C5,4),TN(N_G4,4),TN(N_E4,2),TN(N_F4,2),TN(N_G4,6),TN(N_F4,2),TN(N_E4,2),TN(N_D4,2),TN(N_C4,12) };
static const tnote_t TN_OXMAS[] = {   // O Xmas Tree
  TN(N_G4,4),TN(N_C5,6),TN(N_C5,2),TN(N_C5,8),TN(N_D5,4),TN(N_E5,6),TN(N_E5,2),TN(N_E5,8),TN(N_E5,4),TN(N_D5,6),TN(N_E5,2),TN(N_F5,6),TN(N_E5,2),TN(N_D5,12),TN(N_G4,4),TN(N_C5,6),
  TN(N_C5,2),TN(N_C5,8),TN(N_D5,4),TN(N_E5,6),TN(N_E5,2),TN(N_E5,8),TN(N_E5,4),TN(N_D5,6),TN(N_E5,2),TN(N_D5,6),TN(N_C5,2),TN(N_C5,12) };
static const tnote_t TN_MANGER[] = {   // Away Manger
  TN(N_G4,4),TN(N_C5,6),TN(N_C5,2),TN(N_C5,4),TN(N_D5,4),TN(N_E5,6),TN(N_E5,2),TN(N_E5,4),TN(N_D5,4),TN(N_C5,6),TN(N_D5,2),TN(N_E5,4),TN(N_D5,4),TN(N_C5,12),TN(N_R,4),TN(N_G4,4),
  TN(N_C5,6),TN(N_C5,2),TN(N_C5,4),TN(N_D5,4),TN(N_E5,6),TN(N_E5,2),TN(N_E5,4),TN(N_D5,4),TN(N_C5,6),TN(N_D5,2),TN(N_E5,4),TN(N_D5,4),TN(N_C5,12) };
static const tnote_t TN_HAVANAG[] = {   // Hava Nagila
  TN(N_A4,2),TN(N_A4,2),TN(N_A4,2),TN(N_As4,2),TN(N_A4,2),TN(N_G4,2),TN(N_F4,2),TN(N_E4,2),TN(N_D4,4),TN(N_R,4),TN(N_A4,2),TN(N_A4,2),TN(N_A4,2),TN(N_As4,2),TN(N_A4,2),TN(N_G4,2),
  TN(N_F4,2),TN(N_E4,2),TN(N_D4,4),TN(N_R,4),TN(N_D5,2),TN(N_D5,2),TN(N_D5,2),TN(N_Cs5,2),TN(N_D5,2),TN(N_E5,2),TN(N_F5,4),TN(N_R,4),TN(N_D5,2),TN(N_D5,2),TN(N_D5,2),TN(N_Cs5,2),
  TN(N_D5,2),TN(N_E5,2),TN(N_F5,4),TN(N_R,4),TN(N_F5,2),TN(N_E5,2),TN(N_D5,2),TN(N_Cs5,2),TN(N_D5,2),TN(N_A4,2),TN(N_D5,8) };
static const tnote_t TN_OCHI[] = {   // Ochi Chornye
  TN(N_A4,4),TN(N_D5,6),TN(N_E5,2),TN(N_F5,4),TN(N_E5,4),TN(N_D5,4),TN(N_Cs5,4),TN(N_D5,8),TN(N_R,4),TN(N_A4,4),TN(N_D5,6),TN(N_E5,2),TN(N_F5,4),TN(N_E5,4),TN(N_D5,4),TN(N_Cs5,4),
  TN(N_D5,8),TN(N_R,4),TN(N_F5,4),TN(N_F5,6),TN(N_E5,2),TN(N_D5,4),TN(N_E5,4),TN(N_F5,4),TN(N_E5,4),TN(N_D5,8),TN(N_R,4),TN(N_A4,4),TN(N_D5,6),TN(N_E5,2),TN(N_F5,4),TN(N_E5,4),
  TN(N_D5,4),TN(N_Cs5,4),TN(N_D5,12) };
static const tnote_t TN_WEASEL[] = {   // Pop Weasel
  TN(N_D4,4),TN(N_Fs4,2),TN(N_E4,2),TN(N_G4,2),TN(N_E4,2),TN(N_Fs4,2),TN(N_A4,2),TN(N_Fs4,2),TN(N_E4,2),TN(N_Cs4,2),TN(N_A3,2),TN(N_D4,4),TN(N_Fs4,2),TN(N_E4,2),TN(N_G4,2),TN(N_E4,2),
  TN(N_Fs4,6),TN(N_D4,6),TN(N_D5,2),TN(N_Cs5,2),TN(N_D5,2),TN(N_B4,2),TN(N_Cs5,2),TN(N_D5,2),TN(N_E5,2),TN(N_D5,2),TN(N_Cs5,2),TN(N_A4,2),TN(N_B4,2),TN(N_Cs5,2),TN(N_D5,2),TN(N_Cs5,2),
  TN(N_D5,2),TN(N_B4,2),TN(N_Cs5,2),TN(N_D5,2),TN(N_Cs5,6),TN(N_A4,6),TN(N_D5,2),TN(N_Cs5,2),TN(N_D5,2),TN(N_B4,2),TN(N_Cs5,2),TN(N_D5,2),TN(N_E5,2),TN(N_Cs5,2),TN(N_A4,2),TN(N_A4,4),
  TN(N_Fs4,2),TN(N_D4,4),TN(N_Fs4,2),TN(N_E4,2),TN(N_G4,2),TN(N_E4,2),TN(N_Fs4,6),TN(N_D4,6) };
static const tnote_t TN_SKIPLOU[] = {   // Skip To Lou
  TN(N_G4,2),TN(N_G4,2),TN(N_A4,2),TN(N_G4,2),TN(N_E4,4),TN(N_G4,2),TN(N_G4,2),TN(N_A4,2),TN(N_G4,2),TN(N_E4,4),TN(N_D4,2),TN(N_D4,2),TN(N_E4,2),TN(N_D4,2),TN(N_C4,4),TN(N_R,4),
  TN(N_G4,2),TN(N_G4,2),TN(N_A4,2),TN(N_G4,2),TN(N_E4,4),TN(N_G4,2),TN(N_G4,2),TN(N_A4,2),TN(N_G4,2),TN(N_E4,4),TN(N_D4,2),TN(N_D4,2),TN(N_E4,2),TN(N_D4,2),TN(N_C4,8) };
#define TUNE_OF(nm, arr) {nm, arr, (uint16_t)(sizeof arr / sizeof arr[0])}
static const tune_t TUNE_TAB[NTUNE] = {
  TUNE_OF("ODE TO JOY", TN_ODE), TUNE_OF("TWINKLE", TN_TWINKLE), TUNE_OF("JINGLE BELLS", TN_JINGLE),
  TUNE_OF("FUR ELISE", TN_ELISE), TUNE_OF("HAPPY BDAY", TN_BDAY), TUNE_OF("FRERE JACQUES", TN_FRERE),
  TUNE_OF("GREENSLEEVES", TN_GREEN), TUNE_OF("MARY LAMB", TN_MARY), TUNE_OF("CANON D", TN_CANON),
  TUNE_OF("OLD MACDONALD", TN_MACDONALD), TUNE_OF("ROW YOUR BOAT", TN_ROW), TUNE_OF("LONDON BRIDGE", TN_LONDON), TUNE_OF("CROSS BUNS", TN_BUNS), TUNE_OF("YANKEE DOODLE", TN_YANKEE), TUNE_OF("LIGHTLY ROW", TN_LIGHTLY), TUNE_OF("AU CLAIR LUNE", TN_CLAIR), TUNE_OF("SURPRISE SYM", TN_SURPRISE), TUNE_OF("SILENT NIGHT", TN_SILENT), TUNE_OF("DECK HALLS", TN_DECK), TUNE_OF("MERRY XMAS", TN_XMAS),
  TUNE_OF("TURKISH MARCH", TN_TURKISH), TUNE_OF("KOROBEINIKI", TN_KOROB), TUNE_OF("BEETHOVEN 5", TN_BEET5), TUNE_OF("MINUET IN G", TN_MINUET), TUNE_OF("SAKURA", TN_SAKURA), TUNE_OF("OH SUSANNA", TN_SUSANNA), TUNE_OF("ITSY BITSY", TN_ITSY), TUNE_OF("LA CUCARACHA", TN_CUCA),
  TUNE_OF("MOUNTAIN KING", TN_MTKING), TUNE_OF("DRUNK SAILOR", TN_SAILOR), TUNE_OF("WILLIAM TELL", TN_TELL), TUNE_OF("CAN-CAN", TN_CANCAN),
  TUNE_OF("WEDDING MARCH", TN_WEDDING), TUNE_OF("AMAZING GRACE", TN_GRACE), TUNE_OF("AULD LANG", TN_AULD), TUNE_OF("KALINKA", TN_KALINKA),
  TUNE_OF("LY NGUA O", TN_LYNGUAO), TUNE_OF("BAC KIM THANG", TN_BACKIM), TUNE_OF("KLEINE NACHT", TN_KLEINE), TUNE_OF("HABANERA", TN_HABANERA),
  TUNE_OF("HUNG DANCE 5", TN_HUNG5), TUNE_OF("BLUE DANUBE", TN_DANUBE), TUNE_OF("RADETZKY", TN_RADETZ), TUNE_OF("SWAN LAKE", TN_SWAN),
  TUNE_OF("HALLELUJAH", TN_HALLEL), TUNE_OF("AIR ON G", TN_AIRG), TUNE_OF("BRAHMS LULLBY", TN_BRAHMS), TUNE_OF("VALKYRIES", TN_VALKYR),
  TUNE_OF("SPRING", TN_SPRING), TUNE_OF("GYMNOPEDIE 1", TN_GYMNO), TUNE_OF("SCARBOROUGH", TN_SCARB), TUNE_OF("DANNY BOY", TN_DANNY),
  TUNE_OF("SAINTS", TN_SAINTS), TUNE_OF("CLEMENTINE", TN_CLEMENT), TUNE_OF("CAMPTOWN", TN_CAMPTWN), TUNE_OF("DIXIE", TN_DIXIE),
  TUNE_OF("JOY TO WORLD", TN_JOYW), TUNE_OF("O XMAS TREE", TN_OXMAS), TUNE_OF("AWAY MANGER", TN_MANGER), TUNE_OF("HAVA NAGILA", TN_HAVANAG),
  TUNE_OF("OCHI CHORNYE", TN_OCHI), TUNE_OF("POP WEASEL", TN_WEASEL), TUNE_OF("SKIP TO LOU", TN_SKIPLOU),
};
