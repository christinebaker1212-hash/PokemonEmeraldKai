// Moemon Rhythm: what the DANCE machine's files share (docs/RHYTHM_DECISIONS.md). The machine's screen is the game
// picture while it's on: everything is drawn into one 240x160 picture (rh_gfx.c) that replaces the GBA's.
#ifndef GUARD_RH_H
#define GUARD_RH_H

#include <stdint.h>
#include "rhythm/chart.h"
#include "rhythm/play.h"

#define RH_W 240
#define RH_H 160

// ---- pictures (rh_gfx.c): graphics/rhythm/rhythm.bin, built in ----
struct RhPic
{
    const uint8_t *rgba;        // frames stacked downwards
    uint16_t w, h, frames;
};

const struct RhPic *RhPic_Get(const char *name);    // NULL: not there (nothing is drawn)
const uint8_t *RhSound_Get(const char *name, uint32_t *size);
int RhSound_Count(const char *prefix);              // the announcer's lines in one folder ("ann:stage 1:")

// a picture read from a file (banners, backgrounds), shrunk to fit w x h (0: as it is)
struct RhImage
{
    uint8_t *rgba;
    int w, h;
};
int RhImage_Load(struct RhImage *img, const char *path, int w, int h, int cover);
void RhImage_Free(struct RhImage *img);

// ---- drawing ----
#define RH_RGB(r, g, b) ((uint32_t)(r) << 16 | (uint32_t)(g) << 8 | (uint32_t)(b))
enum { RH_ROT_0, RH_ROT_90, RH_ROT_180, RH_ROT_270 };    // clockwise
enum { RH_LEFT, RH_CENTER, RH_RIGHT };
enum { RH_FONT_SMALL, RH_FONT_NORMAL, RH_FONT_NARROW };

void RhGfx_Clear(uint32_t rgb);
void RhGfx_Fill(int x, int y, int w, int h, uint32_t rgb, int alpha);     // alpha 0-256
// a frame of a picture at (x, y) (its top left); rot turns it (square pictures), alpha 0-256
void RhGfx_Pic(const struct RhPic *pic, int frame, int x, int y, int rot, int alpha);
// ... scaled to w x h (centred on cx, cy)
void RhGfx_PicScaled(const struct RhPic *pic, int frame, int cx, int cy, int w, int h, int alpha);
// ... only rows sy..sy+sh of the frame (freeze bodies)
void RhGfx_PicPart(const struct RhPic *pic, int frame, int x, int y, int sy, int sh, int rot, int alpha);
// ... lighter (add 0-256 of white)
void RhGfx_PicGlow(const struct RhPic *pic, int frame, int x, int y, int rot, int alpha, int glow);
void RhGfx_Image(const struct RhImage *img, int x, int y, int alpha);
// text in the game's fonts (ASCII); returns the width
int RhGfx_Text(int font, int x, int y, const char *text, uint32_t rgb, int align);
int RhGfx_TextWidth(int font, const char *text);
// text cut to fit maxW, with "..." when it doesn't
int RhGfx_TextFit(int font, int x, int y, const char *text, uint32_t rgb, int align, int maxW);
// a number in a digits picture (frames 0-9); align RH_LEFT / CENTER / RIGHT on x; zeros: at least this many digits
int RhGfx_Number(const struct RhPic *digits, int x, int y, uint32_t value, int align, int zeros, int alpha, int spacing);
// the picture, as the GBA's 15-bit colors
void RhGfx_ToGba(uint16_t *out);

// ---- sound (rh_audio.c), mixed into the game's own at 42048 Hz ----
int RhAudio_PlayMusic(const char *path, double from, double length, int loop, double fadeIn);
void RhAudio_StopMusic(double fadeOut);
int RhAudio_MusicPlaying(void);
double RhAudio_MusicTime(void);         // seconds into the music, as heard now
double RhAudio_MusicLength(void);
void RhAudio_PlayMenuMusic(void);       // the built-in menu loop
void RhAudio_Sfx(const char *name);
int RhAudio_Announce(const char *folder);   // a line from the announcer's folder (one at a time); 0 none
int RhAudio_Announcing(void);
void RhAudio_StopAll(void);
void RhAudio_Mix(float *stereo, int frames);
double RhAudio_Now(void);               // the PC's clock, seconds

// ---- a player's field (rh_field.c) ----
enum { RH_APP_VISIBLE, RH_APP_HIDDEN, RH_APP_SUDDEN, RH_APP_STEALTH };
enum { RH_ARROWS_VIVID, RH_ARROWS_NOTE, RH_ARROWS_FLAT, RH_ARROWS_SOLO };
#define RH_SPEEDS 7
extern const float gRhSpeeds[RH_SPEEDS];          // x1, x1.5, x2, x3, x5, x8... (speed index)
struct RhOptions
{
    uint8_t speed, boost, appearance, turn, dark, reverse, little, noFreeze, arrows, marvelous;
    int8_t timing;              // 5 ms steps: the steps are judged this much later (a PC whose sound comes out late)
};

struct RhField
{
    uint8_t used, side, remote;
    struct RhPlay play;
    struct RhOptions opt;
    int diff, meter;
    // what's shown
    int judge;                  // the last row's RH_J_* (0 none)
    float judgeTime;            // seconds since it came
    int holdJudge[RH_COLS];     // RH_HOLD_OK / NG under each column
    float holdJudgeTime[RH_COLS];
    float flash[RH_COLS];       // the step's flash, fading
    uint8_t flashBright[RH_COLS];
    float press[RH_COLS];       // the receptor pushed in
    int comboShown;
    float lifeShown;
    uint8_t held[RH_COLS];      // (remote: as they said)
    // online: what the friend said about their song
    int32_t netScore;
    int netCombo, netMaxCombo;
    float netLife;
    uint8_t netFailed;
    uint16_t netSeq;            // their events applied up to this one
};

void RhField_Begin(struct RhField *f, int side, int remote);
// the events since last time: what's shown, the announcer's combo calls
void RhField_Events(struct RhField *f, float dt, int announce);
// cx: the field's middle
void RhField_Draw(struct RhField *f, const struct RhSong *song, double t, double beat, int cx);
void RhField_DrawLife(const struct RhField *f, double beat);   // into the frame across the top
void RhField_DrawScore(const struct RhField *f);
int RhField_Score(const struct RhField *f);
int RhField_Combo(const struct RhField *f);
float RhField_Life(const struct RhField *f);
int RhField_Failed(const struct RhField *f);

#endif
