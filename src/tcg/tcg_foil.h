/* Foil for rare cards, shared by the three card games (src/tcg/tcg_foil.c): light added over the card, never a color
 * laid over the picture, so the printed or Game Boy art keeps its own colors.
 *
 * - a sheen: a soft diagonal band of light that crosses the illustration now and then, then rests;
 * - twinkles: a few tiny four-point stars that come and go in the picture;
 * - a glint along the name strip (the Rares of both games), a light running round the edge (the rarest).
 * Positions and timing follow from seed and frame only (the same every time). */
#ifndef MOEMON_TCG_FOIL_H
#define MOEMON_TCG_FOIL_H

enum
{
    FOIL_NONE,
    FOIL_GLINT,    /* a Rare: a glint along the name, nothing on the picture */
    FOIL_SILVER,   /* a Super Rare: a white sheen, two twinkles */
    FOIL_GOLD,     /* an Ultra Rare: a warm sheen, a gold glint on the name and the edge */
    FOIL_PRISM,    /* a Secret Rare: a sheen that runs through the colors, the edge too */
    FOIL_COSMOS,   /* a POKeMON holo: a pale prism sheen and a field of twinkles, like the printed starlight foil */
    FOIL_STAR,     /* a TRIPLE TRIAD five-star card: a slow pale-gold sheen, twinkles */
};

/* The card is w x h at x, y; its illustration ax, ay, aw x ah (absolute). */
void G_Foil(unsigned kind, int x, int y, int w, int h, int ax, int ay, int aw, int ah, unsigned seed, unsigned frame);
/* A soft glow round a card w x h at x, y (a rare just revealed): color, or the colors going round when prism */
void G_FoilHalo(int x, int y, int w, int h, unsigned color, unsigned frame, int prism);
/* A hue on the color wheel, 0-359, bright and soft (the prism foil's colors) */
unsigned G_FoilHue(int degrees);

#endif
