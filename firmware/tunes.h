#pragma once
// Bai nhac cho che do nhac cu (v13). Moi nốt {midi, do dai}: do dai tinh theo 1/16 nốt (4 = nốt den), midi 0 = nghi.
// Cac bai viet o quang tam 4-5; khi phat dich len 1 quang tam (coi nho phat ro hon tu ~500 Hz). Toan bo la ban nhac dan gian / co truyen (khong con ban quyen).
#include <stdint.h>
#define NTUNE 20                           // v16: 9 bai goc (0..8) + 11 bai thieu nhi / dan ca (9..19). Ma phim: 0..15 = 0x7500|n, 16.. = 0x7600|n
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
#define TUNE_OF(nm, arr) {nm, arr, (uint16_t)(sizeof arr / sizeof arr[0])}
static const tune_t TUNE_TAB[NTUNE] = {
  TUNE_OF("ODE TO JOY", TN_ODE), TUNE_OF("TWINKLE", TN_TWINKLE), TUNE_OF("JINGLE BELLS", TN_JINGLE),
  TUNE_OF("FUR ELISE", TN_ELISE), TUNE_OF("HAPPY BDAY", TN_BDAY), TUNE_OF("FRERE JACQUES", TN_FRERE),
  TUNE_OF("GREENSLEEVES", TN_GREEN), TUNE_OF("MARY LAMB", TN_MARY), TUNE_OF("CANON D", TN_CANON),
  TUNE_OF("OLD MACDONALD", TN_MACDONALD), TUNE_OF("ROW YOUR BOAT", TN_ROW), TUNE_OF("LONDON BRIDGE", TN_LONDON), TUNE_OF("CROSS BUNS", TN_BUNS), TUNE_OF("YANKEE DOODLE", TN_YANKEE), TUNE_OF("LIGHTLY ROW", TN_LIGHTLY), TUNE_OF("AU CLAIR LUNE", TN_CLAIR), TUNE_OF("SURPRISE SYM", TN_SURPRISE), TUNE_OF("SILENT NIGHT", TN_SILENT), TUNE_OF("DECK HALLS", TN_DECK), TUNE_OF("MERRY XMAS", TN_XMAS),
};
