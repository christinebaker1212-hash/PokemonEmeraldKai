/* Magic card faces (src/mtg/mtg_gfx.c), drawn the other card games' way (ygo_gfx.c, tcg_gfx.c): the game's own frame
 * colored by the card's color (white, blue, black, red, green, gold for more than one, silver artifacts, brown lands)
 * with the mana cost as dots, the printed illustration (Scryfall's scans: graphics/mtg/printed.bin, built by
 * tools/mtg/build_printed.py), a type bar, text lines and a power / toughness plate. CARD ART (gMpConfig.cardArt):
 * the printed illustration in the frame (also for GAME BOY, Magic having none), or the whole printed card. CARD BACKS
 * (gMpConfig.cardBack): the printed back or the game's own. A card is its definition index (Mtg_Card); MTG_GFX_BACK
 * is the back. */
#ifndef MOEMON_MTG_GFX_H
#define MOEMON_MTG_GFX_H
#include <SDL2/SDL.h>

#define MTG_GFX_BACK (-1)

SDL_Texture *MtgGfx_CardTexture(int card, int w, int h);
void MtgGfx_DrawCard(int card, float x, float y, float w, float h, double angle, unsigned alpha, unsigned flash);
/* Where the illustration sits on a face of w x h (the foil over it) */
void MtgGfx_ArtRect(int card, int w, int h, int *ax, int *ay, int *aw, int *ah);
/* The big picture: the illustration (or the whole card in FULL CARD) and its size */
SDL_Texture *MtgGfx_ArtTexture(int card, int *w, int *h);
/* The printed illustration's artist (Scryfall asks that they be named), NULL without a printed card */
const char *MtgGfx_Artist(int card);
int MtgGfx_HasPrinted(int card);
/* The frame's colors for a card: shade 0 base, 1 light, 2 dark */
unsigned MtgGfx_FrameColor(int card, int shade);
/* A mana symbol's color (MTG_W..MTG_C) */
unsigned MtgGfx_ManaColor(int kind);
/* A card's mana cost as symbols at x, y (r: the dots' radius), right-aligned when right; returns its width */
int MtgGfx_Cost(int card, int x, int y, int right, int big);
/* A card's keywords in words, for under its name (FLYING  HASTE  MOUNTAINWALK...): kw the keyword bits it has now
 * (MTG_KW_BIT; on the table Mtg_Keywords), with its printed landwalk and protection; "" when it has none */
void MtgGfx_KeywordLine(int card, uint32_t kw, char *out, unsigned cap);
void MtgGfx_SetModes(unsigned art, unsigned back);
void MtgGfx_Forget(void);
#endif
