/* Yu-Gi-Oh! card faces (src/ygo/ygo_gfx.c): drawn the Pokemon cards' way (tcg_gfx.c), in a frame colored by the
 * card's kind. CARD ART (gMpConfig.cardArt, G_ART_*): the printed illustration in the frame, Stairway to the
 * Destined Duel's own picture in the frame, or the whole printed card. CARD BACKS (gMpConfig.cardBack): the printed
 * back or a pixel-art one. */
#ifndef MOEMON_YGO_GFX_H
#define MOEMON_YGO_GFX_H
#include <SDL2/SDL.h>
#include "ygo/ygo.h"

/* id 0: the card back. The texture is exactly w x h. */
SDL_Texture *Ygo_CardTexture(unsigned id, int w, int h);
void Ygo_DrawCard(unsigned id, float x, float y, float w, float h, double angle, unsigned alpha, unsigned flash);
/* The big picture: the illustration (or the whole card in FULL CARD), its size; smooth: scale it smoothly */
SDL_Texture *Ygo_ArtTexture(unsigned id, int *w, int *h, int *smooth);
/* Frame colors by kind: shade 0 base, 1 light, 2 dark */
unsigned Ygo_FrameColor(unsigned id, int shade);
unsigned Ygo_AttributeColor(unsigned attribute);
const char *Ygo_AttributeName(unsigned attribute);
const char *Ygo_RaceName(unsigned race);
const char *Ygo_RarityName(unsigned rarity);
void Ygo_SetArtMode(unsigned mode);
/* CARD BACKS (gMpConfig.cardBack, G_BACK_*): the printed back or the pixel-art one */
void Ygo_SetBackMode(unsigned mode);
void Ygo_ForgetTextures(void);
#endif
