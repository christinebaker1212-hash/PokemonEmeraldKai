#ifndef GUARD_MP_SPRITESCALE_H
#define GUARD_MP_SPRITESCALE_H

// Moemon Co-op: OPTIONS > DISPLAY > SPRITE SCALING (src/mp/mp_spritescale.c): followers' and the GARDEN's POKeMON's
// PMD pictures (only those: the game's own follower sprites are already drawn to the trainer's scale) shrunk toward
// their POKeDEX height next to a trainer, never below just shorter than the player, never grown. A sprite's mpScale (Q8, 0 = as drawn) is drawn
// smoothly by the renderer (gba_easy_draw.c), around the bottom middle of its picture.

#define MP_TRAINER_HEIGHT_DM 15   // a trainer's height (1.5 m) ...
#define MP_TRAINER_HEIGHT_PX 21   // ... and how tall the player's sprite is (Brendan's, standing)
#define MP_SCALE_MIN_PX 16        // nothing smaller than 3/4 of the player (the user's call)

u16 MP_SpriteScale_ForMon(u16 dexNum, u16 artHeightPx, u8 percent);   // Q8, 0: as drawn (PERCENT of the way down)
u16 MP_SpriteScale_TiledHeight(const u8 *tiles, u8 size);     // a size x size 4bpp tiled frame's drawn height
u16 MP_SpriteScale_BigHeight(const u8 *frame, u8 bpp);        // an mp_bigsprite frame's drawn height
// How far toward their POKeDEX height (OPTIONS > DISPLAY > SPRITE SCALING: 0-100%, in steps of MP_SCALE_STEP; 1.63.14)
#define MP_SCALE_STEP 5
// A follower's / GARDEN POKeMON's scale (Q8, 0: as drawn) PERCENT of the way toward its height, worked out once per POKeMON and kind of sprite
// (*KEY / *VALUE keep it): its PMD picture (PMD: its height from the ground point) or its sprite (TILES, SIZE)
struct MpPmdView;
u16 MP_SpriteScale_Mon(u8 percent, u16 dex, const struct MpPmdView *pmd, const u8 *tiles, u8 size, u32 *key, u16 *value);

#endif // GUARD_MP_SPRITESCALE_H
