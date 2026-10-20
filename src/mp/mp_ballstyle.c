// Moemon Co-op: a Moemon's POKé BALL look. Each Moemon keeps a "ball style"
// in the 16 bits of struct BoxPokemon that Emerald never uses ("unknown",
// MON_DATA_ENCRYPT_SEPARATOR): it's saved, and copied with the Moemon by
// trades, the PC and the co-op link.
//   bits 0-3   color (basic POKé BALLS only): 0 = classic red
//   bits 4-11  decoration (POKé and PREMIER BALLS only): 0 = none
//   bits 12-15 special ball (MP_SPECIAL_*, the MoeGatcha balls): 0 = none.
//              Its MON_DATA_POKEBALL stays POKé BALL, so other games (and
//              anything that only knows the 12 Hoenn balls) see a POKé BALL;
//              this game shows the special one (MP_BallStyle_DisplayItem).
// The color recolors the red top of the ball (battle sprite, summary ball and
// the item icon used as a preview). Decorations are drawn on top of the ball
// in the same places; there are none until their art is added to sDecorations.
#include "global.h"
#include "mp.h"
#include "decompress.h"
#include "item.h"
#include "item_icon.h"
#include "palette.h"
#include "pokeball.h"
#include "pokemon.h"
#include "sprite.h"
#include "constants/items.h"
#include "constants/pokemon.h"
#include "constants/rgb.h"
#include "battle.h"
#include "event_data.h"
#include "fieldmap.h"
#include "overworld.h"
#include "pokedex.h"
#include "rtc.h"
#include "wild_encounter.h"
#include "constants/battle.h"
#include "constants/map_types.h"

#define TAG_MP_BALL_COLOR 0x56A0   // + color: recolored POKé BALL battle palettes
#define TAG_MP_BALL_SPECIAL 0x56C0 // + special: the special balls' battle palettes

// The special balls: their items, and the color their thrown / summary ball
// takes (the POKé BALL sprite recolored to the main color of their icon art)
static const u16 sSpecialItems[MP_SPECIAL_COUNT] = {
    [MP_SPECIAL_LOVE - 1] = ITEM_LOVE_BALL,   [MP_SPECIAL_FRIEND - 1] = ITEM_FRIEND_BALL,
    [MP_SPECIAL_FAST - 1] = ITEM_FAST_BALL,   [MP_SPECIAL_LEVEL - 1] = ITEM_LEVEL_BALL,
    [MP_SPECIAL_LURE - 1] = ITEM_LURE_BALL,   [MP_SPECIAL_HEAVY - 1] = ITEM_HEAVY_BALL,
    [MP_SPECIAL_MOON - 1] = ITEM_MOON_BALL,   [MP_SPECIAL_DREAM - 1] = ITEM_DREAM_BALL,
    [MP_SPECIAL_HEAL - 1] = ITEM_HEAL_BALL,   [MP_SPECIAL_DUSK - 1] = ITEM_DUSK_BALL,
    [MP_SPECIAL_QUICK - 1] = ITEM_QUICK_BALL,
};

static const u16 sSpecialRamps[MP_SPECIAL_COUNT][5] = {
    [MP_SPECIAL_LOVE - 1] = {RGB(13, 2, 8), RGB(19, 3, 12), RGB(27, 4, 17), RGB(28, 13, 21), RGB(30, 21, 26)},
    [MP_SPECIAL_FRIEND - 1] = {RGB(8, 12, 3), RGB(12, 17, 5), RGB(17, 24, 7), RGB(21, 26, 15), RGB(26, 28, 22)},
    [MP_SPECIAL_FAST - 1] = {RGB(13, 7, 1), RGB(20, 11, 2), RGB(28, 15, 3), RGB(29, 20, 12), RGB(30, 25, 21)},
    [MP_SPECIAL_LEVEL - 1] = {RGB(12, 6, 3), RGB(18, 10, 5), RGB(24, 13, 7), RGB(27, 19, 14), RGB(29, 25, 22)},
    [MP_SPECIAL_LURE - 1] = {RGB(2, 9, 13), RGB(3, 13, 19), RGB(4, 18, 27), RGB(13, 22, 28), RGB(21, 26, 29)},
    [MP_SPECIAL_HEAVY - 1] = {RGB(6, 7, 9), RGB(9, 11, 13), RGB(13, 15, 18), RGB(18, 20, 22), RGB(24, 25, 26)},
    [MP_SPECIAL_MOON - 1] = {RGB(3, 8, 11), RGB(5, 12, 17), RGB(7, 17, 24), RGB(15, 21, 26), RGB(22, 26, 28)},
    [MP_SPECIAL_DREAM - 1] = {RGB(14, 1, 5), RGB(21, 1, 8), RGB(29, 2, 11), RGB(30, 11, 17), RGB(30, 20, 24)},
    [MP_SPECIAL_HEAL - 1] = {RGB(14, 1, 10), RGB(21, 1, 16), RGB(30, 1, 22), RGB(30, 11, 25), RGB(31, 20, 28)},
    [MP_SPECIAL_DUSK - 1] = {RGB(4, 11, 4), RGB(6, 16, 6), RGB(9, 22, 9), RGB(16, 25, 16), RGB(23, 28, 23)},
    [MP_SPECIAL_QUICK - 1] = {RGB(14, 13, 1), RGB(21, 19, 1), RGB(29, 27, 2), RGB(30, 28, 11), RGB(30, 29, 21)},
};

// Five shades per color, dark to light (the classic red uses the game's own)
static const u16 sRamps[MP_BALL_COLORS][5] = {
    [MP_BALL_RED]    = {0},
    [MP_BALL_BLUE]   = {RGB(4, 6, 17),   RGB(5, 9, 22),   RGB(7, 13, 28),  RGB(14, 20, 31), RGB(23, 27, 31)},
    [MP_BALL_GREEN]  = {RGB(3, 12, 5),   RGB(4, 17, 6),   RGB(7, 23, 9),   RGB(15, 28, 14), RGB(25, 31, 21)},
    [MP_BALL_YELLOW] = {RGB(18, 15, 0),  RGB(25, 20, 0),  RGB(30, 25, 3),  RGB(31, 29, 11), RGB(31, 31, 23)},
    [MP_BALL_PINK]   = {RGB(20, 6, 13),  RGB(26, 9, 17),  RGB(31, 14, 22), RGB(31, 21, 26), RGB(31, 27, 29)},
    [MP_BALL_PURPLE] = {RGB(10, 4, 15),  RGB(14, 6, 20),  RGB(19, 10, 26), RGB(24, 17, 30), RGB(28, 25, 31)},
    [MP_BALL_ORANGE] = {RGB(20, 9, 0),   RGB(26, 13, 0),  RGB(31, 17, 3),  RGB(31, 23, 11), RGB(31, 28, 21)},
    [MP_BALL_CYAN]   = {RGB(0, 13, 15),  RGB(0, 18, 20),  RGB(3, 24, 26),  RGB(14, 28, 30), RGB(25, 31, 31)},
    [MP_BALL_BLACK]  = {RGB(2, 2, 3),    RGB(4, 4, 5),    RGB(7, 7, 8),    RGB(12, 12, 13), RGB(19, 19, 20)},
    [MP_BALL_SILVER] = {RGB(12, 13, 14), RGB(17, 18, 19), RGB(22, 23, 24), RGB(26, 27, 28), RGB(30, 30, 31)},
    [MP_BALL_GOLD]   = {RGB(16, 11, 1),  RGB(22, 16, 2),  RGB(27, 21, 5),  RGB(31, 26, 11), RGB(31, 30, 21)},
};

static const u8 sColorNames[MP_BALL_COLORS][8] = {
    [MP_BALL_RED]    = _("RED"),
    [MP_BALL_BLUE]   = _("BLUE"),
    [MP_BALL_GREEN]  = _("GREEN"),
    [MP_BALL_YELLOW] = _("YELLOW"),
    [MP_BALL_PINK]   = _("PINK"),
    [MP_BALL_PURPLE] = _("PURPLE"),
    [MP_BALL_ORANGE] = _("ORANGE"),
    [MP_BALL_CYAN]   = _("CYAN"),
    [MP_BALL_BLACK]  = _("BLACK"),
    [MP_BALL_SILVER] = _("SILVER"),
    [MP_BALL_GOLD]   = _("GOLD"),
};

// Decorations, from the labeled art sheets (none yet). Index 0 is "none".
struct MpBallDecoration
{
    const u8 *name;
    const u8 *iconGfx;      // 24x24 4bpp overlay for the item icon (preview), colors 12-15 of its palette
    const u8 *battleGfx;    // 16x16 4bpp overlay for the battle / summary ball
};

static const struct MpBallDecoration sDecorations[] = {
    {NULL, NULL, NULL},
};

static const u8 sText_None[] = _("NONE");

u8 MP_BallStyle_Color(u16 style)
{
    u8 c = style & 0xF;
    return c < MP_BALL_COLORS ? c : MP_BALL_RED;
}

u8 MP_BallStyle_Decoration(u16 style)
{
    u8 d = (style >> 4) & 0xFF;
    return d < ARRAY_COUNT(sDecorations) ? d : 0;
}

u16 MP_BallStyle_Make(u8 color, u8 decoration)
{
    return (color & 0xF) | ((u16)decoration << 4);
}

u16 MP_BallStyle_OfMon(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_ENCRYPT_SEPARATOR);
}

u16 MP_BallStyle_OfBoxMon(struct BoxPokemon *mon)
{
    return GetBoxMonData(mon, MON_DATA_ENCRYPT_SEPARATOR);
}

// ---------------------------------------------------------------------------
// Special balls

u8 MP_BallStyle_Special(u16 style)
{
    u8 s = style >> 12;
    return s <= MP_SPECIAL_COUNT ? s : MP_SPECIAL_NONE;
}

// MP_SPECIAL_* of a ball item (0: not a special ball)
u8 MP_SpecialBall_FromItem(u16 item)
{
    u8 i;
    for (i = 0; i < MP_SPECIAL_COUNT; i++)
        if (sSpecialItems[i] == item)
            return i + 1;
    return MP_SPECIAL_NONE;
}

u16 MP_SpecialBall_Item(u8 special)
{
    return (special >= 1 && special <= MP_SPECIAL_COUNT) ? sSpecialItems[special - 1] : ITEM_NONE;
}

// Any ball item, the Hoenn ones and the special ones
bool8 MP_IsBallItem(u16 item)
{
    return (item >= FIRST_BALL && item <= LAST_BALL) || MP_SpecialBall_FromItem(item) != MP_SPECIAL_NONE;
}

// The ball item to show for a Moemon (its special ball, else its real one)
u16 MP_BallStyle_DisplayItem(struct Pokemon *mon)
{
    u8 special = MP_BallStyle_Special(MP_BallStyle_OfMon(mon));
    return special ? MP_SpecialBall_Item(special) : GetMonData(mon, MON_DATA_POKEBALL);
}

// The style a ball item gives when thrown or picked (a special ball's index; 0 otherwise)
u16 MP_BallStyle_ForItem(u16 item)
{
    return (u16)MP_SpecialBall_FromItem(item) << 12;
}

bool8 MP_BallStyle_ItemCanColor(u16 item)
{
    return item == ITEM_POKE_BALL;
}

bool8 MP_BallStyle_ItemCanDecorate(u16 item)
{
    return (item == ITEM_POKE_BALL || item == ITEM_PREMIER_BALL) && ARRAY_COUNT(sDecorations) > 1;
}

// A Moemon was caught in `item`: a special ball is kept as POKé BALL + the
// special index (with FRIEND / HEAL BALL's effects), any other ball as itself
void MP_Ball_SetCaught(struct Pokemon *mon, u16 item)
{
    u8 special = MP_SpecialBall_FromItem(item);
    u32 ball = special ? ITEM_POKE_BALL : item;
    u16 style = (u16)special << 12;
    SetMonData(mon, MON_DATA_POKEBALL, &ball);
    SetMonData(mon, MON_DATA_ENCRYPT_SEPARATOR, &style);
    if (special == MP_SPECIAL_FRIEND)
    {
        u8 friendship = 200;
        SetMonData(mon, MON_DATA_FRIENDSHIP, &friendship);
    }
    else if (special == MP_SPECIAL_HEAL)
    {
        u16 hp = GetMonData(mon, MON_DATA_MAX_HP);
        u32 status = STATUS1_NONE;
        SetMonData(mon, MON_DATA_HP, &hp);
        SetMonData(mon, MON_DATA_STATUS, &status);
        MonRestorePP(mon);
    }
}

extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];

static bool8 EvolvesByMoonStone(u16 species)
{
    u8 i;
    for (i = 0; i < EVOS_PER_MON; i++)
        if (gEvolutionTable[species][i].method == EVO_ITEM && gEvolutionTable[species][i].param == ITEM_MOON_STONE)
            return TRUE;
    return FALSE;
}

static bool8 IsNightOrCave(void)
{
    if (gMapHeader.mapType == MAP_TYPE_UNDERGROUND || gBattleEnvironment == BATTLE_ENVIRONMENT_CAVE)
        return TRUE;
    RtcCalcLocalTime();
    return gLocalTime.hours >= 20 || gLocalTime.hours < 6;
}

// A special ball's catch multiplier (x10, like the battle's), for the thrower's
// battler against the target; HEAVY BALL changes the catch rate instead
u8 MP_SpecialBall_Multiplier(u16 item, u8 attacker, u8 target, u8 *catchRate)
{
    struct BattlePokemon *me = &gBattleMons[attacker], *foe = &gBattleMons[target];
    switch (MP_SpecialBall_FromItem(item))
    {
    case MP_SPECIAL_LOVE:
    {
        u8 g1 = GetGenderFromSpeciesAndPersonality(me->species, me->personality);
        u8 g2 = GetGenderFromSpeciesAndPersonality(foe->species, foe->personality);
        if (me->hp != 0 && me->species == foe->species && g1 != MON_GENDERLESS && g2 != MON_GENDERLESS && g1 != g2)
            return 80;
        return 10;
    }
    case MP_SPECIAL_FAST:
        return gSpeciesInfo[foe->species].baseSpeed >= 100 ? 40 : 10;
    case MP_SPECIAL_LEVEL:
        if (me->hp == 0 || me->level <= foe->level)
            return 10;
        if (me->level >= foe->level * 4)
            return 80;
        if (me->level > foe->level * 2)
            return 40;
        return 20;
    case MP_SPECIAL_LURE:
        return gMpFishingEncounter ? 40 : 10;
    case MP_SPECIAL_HEAVY:
    {
        // Weight in tenths of a kilogram (the POKéDEX's)
        u16 weight = GetPokedexHeightWeight(SpeciesToNationalPokedexNum(foe->species), 1);
        s16 rate = *catchRate;
        if (weight < 1000)
            rate -= 20;
        else if (weight >= 3000)
            rate += 30;
        else if (weight >= 2000)
            rate += 20;
        *catchRate = rate < 1 ? 1 : (rate > 255 ? 255 : rate);
        return 10;
    }
    case MP_SPECIAL_MOON:
        return EvolvesByMoonStone(foe->species) ? 40 : 10;
    case MP_SPECIAL_DREAM:
        return (foe->status1 & STATUS1_SLEEP) ? 40 : 10;
    case MP_SPECIAL_DUSK:
        return IsNightOrCave() ? 30 : 10;
    case MP_SPECIAL_QUICK:
        return gBattleResults.battleTurnCounter == 0 ? 50 : 10;
    case MP_SPECIAL_FRIEND:
    case MP_SPECIAL_HEAL:
    default:
        return 10;
    }
}

const u8 *MP_BallStyle_ColorName(u8 color)
{
    return sColorNames[color < MP_BALL_COLORS ? color : 0];
}

u8 MP_BallStyle_DecorationCount(void)
{
    return ARRAY_COUNT(sDecorations);   // including "none"
}

const u8 *MP_BallStyle_DecorationName(u8 decoration)
{
    if (decoration == 0 || decoration >= ARRAY_COUNT(sDecorations) || !sDecorations[decoration].name)
        return sText_None;
    return sDecorations[decoration].name;
}

bool8 MP_BallStyle_CanColor(u8 ballId)
{
    return ballId == BALL_POKE;
}

bool8 MP_BallStyle_CanDecorate(u8 ballId)
{
    return (ballId == BALL_POKE || ballId == BALL_PREMIER) && ARRAY_COUNT(sDecorations) > 1;
}

// The battle / summary ball: palette indices 1-4 are the red top
static void RecolorBallPalette(u16 *pal, u8 color)
{
    pal[1] = sRamps[color][0];
    pal[2] = sRamps[color][1];
    pal[3] = sRamps[color][2];
    pal[4] = sRamps[color][3];
}

// Gives a ball sprite of a Moemon its color (call right after creating it).
// A special ball is the POKé BALL sprite in its own colors.
void MP_BallStyle_ApplyToBallSprite(u8 spriteId, u8 ballId, u16 style)
{
    u8 color = MP_BallStyle_Color(style), slot, special = MP_BallStyle_Special(style);
    u16 tag;
    if (spriteId >= MAX_SPRITES)
        return;
    if (special)
    {
        tag = TAG_MP_BALL_SPECIAL + special;
        slot = IndexOfSpritePaletteTag(tag);
        if (slot == 0xFF)
        {
            static u16 sPal[16];
            struct SpritePalette pal = {sPal, tag};
            LZDecompressWram(gBallSpritePalettes[BALL_POKE].data, sPal);
            sPal[1] = sSpecialRamps[special - 1][0];
            sPal[2] = sSpecialRamps[special - 1][1];
            sPal[3] = sSpecialRamps[special - 1][2];
            sPal[4] = sSpecialRamps[special - 1][3];
            slot = LoadSpritePalette(&pal);
            if (slot == 0xFF)
                return;
        }
        gSprites[spriteId].oam.paletteNum = slot;
        return;
    }
    if (!MP_BallStyle_CanColor(ballId) || color == MP_BALL_RED)
        return;
    tag = TAG_MP_BALL_COLOR + color;
    slot = IndexOfSpritePaletteTag(tag);
    if (slot == 0xFF)
    {
        static u16 sPal[16];
        struct SpritePalette pal = {sPal, tag};
        LZDecompressWram(gBallSpritePalettes[BALL_POKE].data, sPal);
        RecolorBallPalette(sPal, color);
        slot = LoadSpritePalette(&pal);
        if (slot == 0xFF)
            return;
    }
    gSprites[spriteId].oam.paletteNum = slot;
}

// The item icon (the preview): palette indices 7-11 are the red top
void MP_BallStyle_ApplyToIconPalette(u8 paletteNum, u8 ballId, u16 style)
{
    static const u8 sIdx[5] = {7, 8, 9, 10, 11};
    static const u8 sShade[5] = {0, 1, 1, 2, 4};
    u8 color = MP_BallStyle_Color(style), i;
    if (MP_BallStyle_Special(style) || !MP_BallStyle_CanColor(ballId) || color == MP_BALL_RED)
        return;   // special balls have their own icon art
    for (i = 0; i < 5; i++)
    {
        u16 c = sRamps[color][sShade[i]];
        gPlttBufferUnfaded[OBJ_PLTT_ID(paletteNum) + sIdx[i]] = c;
        gPlttBufferFaded[OBJ_PLTT_ID(paletteNum) + sIdx[i]] = c;
    }
}
