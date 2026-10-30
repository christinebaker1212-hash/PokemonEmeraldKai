/* Triple Triad card faces (src/tt/tt_gfx.c): Final Fantasy XIV's pictures from graphics/tt/cards.bin over the grey
 * plate that shows the owner's color, the numbers in their diamond at the bottom, the stars and the type icon. */
#ifndef MOEMON_TT_GFX_H
#define MOEMON_TT_GFX_H
#include <SDL2/SDL.h>
#include "tt/tt.h"

#define TT_BOARD_W 40   /* a card on the table (tools/tt/build_art.py BOARD) */
#define TT_BOARD_H 49
#define TT_BIG_W 104    /* a card up close */
#define TT_BIG_H 128

enum { TT_SEAT_BLUE, TT_SEAT_RED, TT_SEAT_NONE };
unsigned Tt_SeatColor(unsigned seat, int shade);   /* shade 0 base, 1 light, 2 dark */

/* What a card shows besides itself */
struct TtFace
{
    int8_t mod[4];        /* +/- on each number (ELEMENTAL, ASCENSION...): drawn colored */
    uint8_t seat;         /* TT_SEAT_*: the plate's color */
    uint8_t back;         /* face down */
    uint8_t noNumbers;    /* (the list's small faces) */
    uint8_t glow;         /* 0-255: a light over the card (a flip's flash) */
};
/* A card at any size: w/h around the board's size use the board pictures, larger the big ones. scaleX / scaleY
 * squash it about its center (a flip); alpha 0-255. */
void Tt_DrawCard(unsigned id, float x, float y, float w, float h, float scaleX, float scaleY, unsigned alpha,
                 const struct TtFace *face);
/* The 40x40 icon (the card list) */
void Tt_DrawIcon(unsigned id, float x, float y, float size, unsigned alpha, int dim);
/* The type's icon (TT_TYPE_*) and a star, size in pixels */
void Tt_DrawTypeIcon(unsigned type, float x, float y, float size, unsigned alpha);
void Tt_DrawStar(float x, float y, float size, unsigned alpha);
void Tt_DrawStars(unsigned stars, int x, int y, int size, unsigned alpha);
/* The lettering: a 5x7 pixel font with a dark edge (capitals, digits, ! ? . - + ' : /); edge G_NO_SHADOW: none.
 * Tt_Char's sx / sy: the size of one of its pixels */
void Tt_Char(float x, float y, char ch, float sx, float sy, unsigned fill, unsigned edge, unsigned alpha);
void Tt_Text(float x, float y, const char *text, float scale, unsigned fill, unsigned edge, unsigned alpha);
float Tt_TextWidth(const char *text, float scale);
/* A card's number (1-9, 10 = A); scale 1 = 5x7 */
void Tt_Number(int x, int y, int value, int scale, unsigned fill, unsigned alpha);
void Tt_ForgetTextures(void);
#endif
