// Moemon Co-op: type icons by each Moemon's name in battle (OPTIONS > TYPE ICONS).
//
// One or two small icons in the style of the modern type chart (a colored
// square with a white symbol; tools/moemon_typeicons/gen.py) next to every
// battler's healthbox: to the right of a foe's box, to the left of yours (the
// boxes sit at the screen's edges). Placed as in the user's Triple Battle
// mock-up: the secondary type behind, 1 pixel from the box's outer ring and 2
// below its top; the primary type in front of it, 4 pixels further out and
// down. One type: only the front icon. Triple Battles use the cards from
// mp_tricards.c instead of healthboxes; the icons go beside those the same way.
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

// from the box's outer ring (its side and top): the back icon, then the front one
#define BACK_OUT 2
#define BACK_DOWN 2
#define FRONT_OUT 6
#define FRONT_DOWN 6

bool8 MP_TypeIcons_On(void)
{
    return gSaveBlock2Ptr->optionsTypeIcons;
}

// The icons fade exactly as the healthbox (or card) they belong to: palette.c MP_FadeFit
static struct MpFadeFit sFit;

static u16 Faded(u16 c)
{
    return MP_FadeFit_Apply(&sFit, c) | 0x8000;
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

// The healthbox's (singles / doubles) outer ring: its left and right columns and its top row
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
    // measured on screenshots, the same in singles and doubles: a foe's box's outer ring is 60 pixels
    // right of its first sprite's center, yours 23 left of it; the top of the ring 14 above
    *left = x - 23;
    *right = x + 60;
    *top = y - 14;
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
        s16 left, right, top;
        u8 t1, t2;
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
        if (IS_TRIPLE_BATTLE())
        {
            u8 slot = IndexOfSpritePaletteTag(MP_TRICARD_PAL_TAG);
            sFit.on = FALSE;
            if (slot != 0xFF)
                MP_FadeFit_FromPalette(&sFit, OBJ_PLTT_ID(slot));
        }
        else
        {
            MP_FadeFit_FromPalette(&sFit, OBJ_PLTT_ID(gSprites[gHealthboxSpriteIds[b]].oam.paletteNum));
        }
        BattlerTypes(b, &t1, &t2);
        // the secondary type behind, the primary in front
        if (t2 != t1 && t2 < NUMBER_OF_MON_TYPES)
            DrawIcon(frame, t2, foe ? right + BACK_OUT : left - BACK_OUT - (MP_TYPEICON_SIZE - 1), top + BACK_DOWN);
        DrawIcon(frame, t1, foe ? right + FRONT_OUT : left - FRONT_OUT - (MP_TYPEICON_SIZE - 1), top + FRONT_DOWN);
    }
}
