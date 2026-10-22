// Moemon Co-op: type icons by each Moemon's name in battle (OPTIONS > TYPE ICONS).
//
// One or two small icons in the style of the modern type chart (a colored
// square with a white symbol; tools/moemon_typeicons/gen.py) next to every
// battler's healthbox, on the name's line: to the right of a foe's box, to the
// left of yours (the boxes sit at the screen's edges). Triple Battles use the
// cards from mp_tricards.c instead of healthboxes; the icons go beside those.
// They're drawn over the frame like the cards (sdl2.c VDraw), so they take no
// sprites, tiles or palettes, follow the boxes as they slide in and bounce,
// fade with the screen, and show the battler's current types (gBattleMons:
// Color Change, Conversion...).
#include <stdlib.h>
#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_anim.h"
#include "main.h"
#include "palette.h"
#include "pokemon.h"
#include "sprite.h"
#include "constants/battle.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "mp.h"

#include "mp_typeicon_gfx.h"

#define ICON_GAP 1

bool8 MP_TypeIcons_On(void)
{
    return gSaveBlock2Ptr->optionsTypeIcons;
}

static inline u16 Mix555(u16 a, u16 b, u8 k)   // k/16 of b over a
{
    u16 r = ((a & 31) * (16 - k) + (b & 31) * k) >> 4;
    u16 g = (((a >> 5) & 31) * (16 - k) + ((b >> 5) & 31) * k) >> 4;
    u16 bl = (((a >> 10) & 31) * (16 - k) + ((b >> 10) & 31) * k) >> 4;
    return r | (g << 5) | (bl << 10) | 0x8000;
}

// The game's own whole-screen fade, applied to the icons too
static u16 Faded(u16 c)
{
    if (gPaletteFade.active || gPaletteFade.y)
    {
        u8 y = gPaletteFade.y > 16 ? 16 : gPaletteFade.y;
        if (y)
            return Mix555(c, gPaletteFade.blendColor, y);
    }
    return c | 0x8000;
}

static void DrawIcon(u16 *frame, u8 type, s16 x0, s16 y0)
{
    const u16 *src;
    int x, y;
    if (type >= NUMBER_OF_MON_TYPES)
        return;
    src = sTypeIcons[type];
    for (y = 0; y < MP_TYPEICON_SIZE; y++)
    {
        int sy = y0 + y;
        if (sy < 0 || sy >= DISPLAY_HEIGHT)
            continue;
        for (x = 0; x < MP_TYPEICON_SIZE; x++)
        {
            int sx = x0 + x;
            u16 c = src[y * MP_TYPEICON_SIZE + x];
            if (c == 0 || sx < 0 || sx >= DISPLAY_WIDTH)
                continue;
            frame[sy * DISPLAY_WIDTH + sx] = Faded(c);
        }
    }
}

// The healthbox's (singles / doubles) visible left and right edges and the top of the name's line
static bool8 HealthboxPlace(u8 battler, s16 *left, s16 *right, s16 *top)
{
    struct Sprite *box;
    s16 x, y;
    if (gHealthboxSpriteIds[battler] >= MAX_SPRITES)
        return FALSE;
    box = &gSprites[gHealthboxSpriteIds[battler]];
    if (!box->inUse || box->invisible)
        return FALSE;
    x = box->x + box->x2 + (box->coordOffsetEnabled ? gSpriteCoordOffsetX : 0);
    y = box->y + box->y2 + (box->coordOffsetEnabled ? gSpriteCoordOffsetY : 0);
    if (getenv("MOEMON_TYPEICON_DEBUG"))
    {
        static u32 sLast[MAX_BATTLERS_COUNT];
        u32 key = (u16)x | ((u32)(u16)y << 16);
        if (key != sLast[battler])
        {
            struct Sprite *other = &gSprites[box->oam.affineParam];
            MPNet_Log("typeicons: b%d box %d at %d,%d shape %d size %d; other at %d,%d shape %d size %d", battler,
                      gHealthboxSpriteIds[battler], x, y, box->oam.shape, box->oam.size,
                      other->x + other->x2, other->y + other->y2, other->oam.shape, other->oam.size);
            sLast[battler] = key;
        }
    }
    // measured on screenshots, the same in singles and doubles: a foe's box art ends 60 pixels right of
    // its first sprite's center, yours starts 23 left of it; the name's line starts 12 above
    *left = x - 23;
    *right = x + 60;
    *top = y - 12;
    return TRUE;
}

// The battle's own types (Color Change, Conversion...) where this game runs the battle engine; in a link
// battle the other games' gBattleMons isn't filled in, so the Moemon's species types there
static void BattlerTypes(u8 battler, u8 *t1, u8 *t2)
{
    struct Pokemon *party = GetBattlerSide(battler) == B_SIDE_PLAYER ? gPlayerParty : gEnemyParty;
    u16 species = GetMonData(&party[gBattlerPartyIndexes[battler] % PARTY_SIZE], MON_DATA_SPECIES);
    if (gBattleMons[battler].species == species && species != SPECIES_NONE
     && (gBattleMons[battler].types[0] != TYPE_NORMAL || gBattleMons[battler].types[1] != TYPE_NORMAL
         || gSpeciesInfo[species].types[0] == TYPE_NORMAL))
    {
        *t1 = gBattleMons[battler].types[0];
        *t2 = gBattleMons[battler].types[1];
    }
    else
    {
        *t1 = gSpeciesInfo[species].types[0];
        *t2 = gSpeciesInfo[species].types[1];
    }
}

void MP_TypeIcons_Composite(u16 *frame)
{
    u8 b;
    if (!gMain.inBattle || gMain.callback2 != BattleMainCB2 || !MP_TypeIcons_On() || !MP_OBJ_LAYER_SHOWN())
        return;
    for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
    {
        s16 left, right, top, x;
        u8 t1, t2, n;
        bool8 foe = GetBattlerSide(b) == B_SIDE_OPPONENT;

        if (gBattleTypeFlags & BATTLE_TYPE_SAFARI && !foe)
            continue;
        if (IS_TRIPLE_BATTLE())
        {
            if (!MP_TriCards_Place(b, &left, &right, &top))
                continue;
        }
        else if (!HealthboxPlace(b, &left, &right, &top))
            continue;
        BattlerTypes(b, &t1, &t2);
        n = (t2 != t1 && t2 < NUMBER_OF_MON_TYPES) ? 2 : 1;
        if (foe)
            x = right + 2;
        else
            x = left - 2 - n * MP_TYPEICON_SIZE - (n - 1) * ICON_GAP;
        DrawIcon(frame, t1, x, top);
        if (n == 2)
            DrawIcon(frame, t2, x + MP_TYPEICON_SIZE + ICON_GAP, top);
    }
}
