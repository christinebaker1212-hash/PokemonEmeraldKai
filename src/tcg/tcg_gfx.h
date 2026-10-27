/* Card system drawing: native 240x160 primitives, the Emerald fonts and
 * window frames, and card faces built from the printed scans (when the art
 * pack is installed) or the original Game Boy illustrations. */
#ifndef MOEMON_TCG_GFX_H
#define MOEMON_TCG_GFX_H
#include <SDL2/SDL.h>
#include <stdint.h>
#include "tcg/tcg.h"

/* The game links no libm: SDL's own math keeps every platform the same. */
#define sinf SDL_sinf
#define cosf SDL_cosf
#define sqrtf SDL_sqrtf
#define atan2f SDL_atan2f
#define ceilf SDL_ceilf
#define fabsf SDL_fabsf
static inline float G_MinF(float a, float b) { return a < b ? a : b; }
static inline float G_MaxF(float a, float b) { return a > b ? a : b; }

enum
{
    TF_NORMAL,
    TF_SMALL,
    TF_NARROW
};
enum
{
    TA_LEFT,
    TA_CENTER,
    TA_RIGHT
};
enum
{
    G_BUTTON_NORMAL,
    G_BUTTON_FOCUS,
    G_BUTTON_DISABLED,
    G_BUTTON_PRESSED
};
#define G_INK 0x383838
#define G_INK_SHADOW 0xd8d8d8
#define G_WHITE 0xf8f8f8
#define G_NO_SHADOW 0xff000000u

/* Software surfaces (0xAARRGGBB) for textures built once. */
struct GSurface
{
    int w, h;
    uint32_t *px;
};
struct GSurface G_SurfaceNew(int w, int h);
void G_SurfaceFree(struct GSurface *s);
SDL_Texture *G_SurfaceTexture(const struct GSurface *s);
void S_Px(struct GSurface *s, int x, int y, uint32_t argb);
void S_Blend(struct GSurface *s, int x, int y, unsigned rgb, unsigned alpha);
void S_Fill(struct GSurface *s, int x, int y, int w, int h, uint32_t argb);
void S_Frame(struct GSurface *s, int x, int y, int w, int h, uint32_t argb);
void S_RoundFrame(struct GSurface *s, int x, int y, int w, int h, int radius, uint32_t argb);
void S_RoundFill(struct GSurface *s, int x, int y, int w, int h, int radius, uint32_t argb);
void S_Ring(struct GSurface *s, float cx, float cy, float radius, float thickness, uint32_t argb);
void S_Disc(struct GSurface *s, float cx, float cy, float radius, uint32_t argb);
#define ARGB(rgb, a) ((uint32_t)(a) << 24 | ((rgb) & 0xffffff))

void G_Begin(SDL_Renderer *r);
SDL_Renderer *G_R(void);
void G_Destroy(void);
unsigned G_Rgb15(unsigned c);
unsigned G_Mix(unsigned a, unsigned b, float t);
void G_Fill(int x, int y, int w, int h, unsigned rgb);
void G_FillA(int x, int y, int w, int h, unsigned rgb, unsigned alpha);
void G_Frame(int x, int y, int w, int h, unsigned rgb, unsigned alpha);
void G_RoundFill(int x, int y, int w, int h, unsigned rgb, unsigned alpha);
void G_RoundFrame(int x, int y, int w, int h, unsigned rgb, unsigned alpha);
void G_Line(int x0, int y0, int x1, int y1, unsigned rgb, unsigned alpha);
int G_Measure(const char *s, int font);
int G_LineHeight(int font);
/* Word-wrapped text; returns the y after the last line. shadow may be G_NO_SHADOW. */
int G_Text(int x, int y, unsigned color, unsigned shadow, const char *text, int width, int font);
void G_TextAlign(int x, int y, int align, unsigned color, unsigned shadow, const char *text, int font);
void G_Window(int x, int y, int w, int h);
void G_Cursor(int x, int y);
void G_Arrow(int x, int y, int dir, unsigned rgb); /* 0 up 1 down 2 left 3 right, 7x4 */
/* Pixel digits: size 0 = 3x5, 1 = 5x7. Characters 0-9, H P x + - / %. */
int G_DigitsWidth(const char *s, int size, int scale);
void G_Digits(int x, int y, const char *s, int size, int scale, unsigned rgb, unsigned alpha);
void G_DigitsOutlined(int x, int y, const char *s, int size, int scale, unsigned fill, unsigned outline,
                      unsigned alpha);
void G_HpPlate(int x, int y, int hp, unsigned maxHp, int small, unsigned alpha);
int G_HpPlateWidth(int small);
void G_EnergyOrb(int x, int y, unsigned type, int big);
void G_StatusBadge(int x, int y, unsigned condition, unsigned poison);
int G_StatusBadges(int x, int y, unsigned condition, unsigned poison); /* returns width */
void G_Button(int x, int y, int w, int h, const char *label, int state);
void G_Banner(int cx, int cy, const char *text, unsigned color, float scale, unsigned alpha);
void G_Coin(int cx, int cy, float spin, int heads, unsigned alpha);
/* Controller buttons ({A} {B} {L} {R} {START} {SELECT} {UP}... in hints),
 * centered on y. G_Hint returns the width; draw = 0 only measures. */
int G_ButtonIcon(int x, int y, unsigned id);
int G_ButtonIconWidth(unsigned id);
int G_Hint(int x, int y, unsigned color, unsigned shadow, const char *text, int font, int draw);
unsigned G_TypeColor(unsigned type, int shade); /* shade 0 base, 1 light, 2 dark */
const char *G_TypeName(unsigned type);

/* Card faces. id 0 = card back. The texture is exactly w x h. */
SDL_Texture *G_CardTexture(unsigned id, int w, int h);
void G_Card(unsigned id, float x, float y, float w, float h, double angle, unsigned alpha, unsigned flash);
/* A big face without small details: the caller prints text over it. */
SDL_Texture *G_ArtTexture(unsigned id, int full, int *w, int *h, int *scan);
int G_HasScans(void);
void G_SetGameBoyArt(int on);
#endif
