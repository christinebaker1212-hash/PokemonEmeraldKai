// Moemon Co-op: a Moemon's POKé BALL look. Each Moemon keeps a "ball style"
// in the 16 bits of struct BoxPokemon that Emerald never uses ("unknown",
// MON_DATA_ENCRYPT_SEPARATOR): it's saved, and copied with the Moemon by
// trades, the PC and the co-op link.
//   bits 0-3   color (basic POKé BALLS only): 0 = classic red; bits 4-5 of the color: bit 11 and
//              bits 12-15 = 15 (MP_BallStyle_Color; older versions only knew 11 colors)
//   bits 4-10  decoration (POKé and PREMIER BALLS only): 0 = none
//   bits 12-15 special ball (MP_SPECIAL_*, the MoeGatcha balls): 0 = none, 15 = none and color bit 5.
//              Its MON_DATA_POKEBALL stays POKé BALL, so other games (and
//              anything that only knows the 12 Hoenn balls) see a POKé BALL;
//              this game shows the special one (MP_BallStyle_DisplayItem).
// The color recolors the red top of the ball (battle sprite, summary ball and
// the item icon used as a preview). A decoration (POKé BALLS only) is a small
// emblem drawn into the ball's own pixels: on the item icon (summary POKé BALL
// page) and on the first frame of the battle / summary ball (the thrown and
// opening frames only show up when catching). Art: tools/moemon_balls/decorations.py.
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

#define TAG_MP_BALL_COLOR 0x5770   // + color (41): recolored POKé BALL battle palettes (0x5770-0x5798)
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
    [MP_SPECIAL_GENESIS - 1] = ITEM_GENESIS_BALL, [MP_SPECIAL_EPOCH - 1] = ITEM_EPOCH_BALL,
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
    // (their own art takes over wherever it can: mp_legacy_balls.c)
    [MP_SPECIAL_GENESIS - 1] = {RGB(4, 4, 4), RGB(20, 10, 10), RGB(31, 16, 16), RGB(31, 24, 24), RGB(31, 31, 31)},
    [MP_SPECIAL_EPOCH - 1] = {RGB(4, 4, 4), RGB(22, 7, 4), RGB(30, 10, 6), RGB(30, 16, 15), RGB(31, 19, 24)},
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
    [MP_BALL_CRIMSON]  = {RGB(12, 1, 4), RGB(17, 1, 6), RGB(24, 2, 8), RGB(27, 13, 17), RGB(29, 23, 25)},
    [MP_BALL_SCARLET]  = {RGB(16, 2, 1), RGB(22, 3, 1), RGB(31, 4, 2), RGB(31, 14, 13), RGB(31, 23, 23)},
    [MP_BALL_CORAL]    = {RGB(16, 6, 5), RGB(22, 9, 7), RGB(31, 13, 10), RGB(31, 20, 18), RGB(31, 26, 25)},
    [MP_BALL_SALMON]   = {RGB(16, 8, 7), RGB(22, 12, 10), RGB(31, 17, 14), RGB(31, 22, 20), RGB(31, 27, 26)},
    [MP_BALL_PEACH]    = {RGB(16, 11, 8), RGB(22, 16, 12), RGB(31, 22, 16), RGB(31, 25, 22), RGB(31, 28, 27)},
    [MP_BALL_AMBER]    = {RGB(16, 10, 0), RGB(22, 14, 0), RGB(31, 20, 0), RGB(31, 24, 12), RGB(31, 28, 22)},
    [MP_BALL_LEMON]    = {RGB(16, 16, 5), RGB(22, 22, 7), RGB(31, 31, 10), RGB(31, 31, 18), RGB(31, 31, 25)},
    [MP_BALL_OLIVE]    = {RGB(8, 10, 2), RGB(12, 14, 3), RGB(17, 19, 4), RGB(22, 24, 14), RGB(27, 28, 23)},
    [MP_BALL_LIME]     = {RGB(9, 16, 2), RGB(13, 22, 3), RGB(18, 31, 4), RGB(23, 31, 14), RGB(27, 31, 23)},
    [MP_BALL_MINT]     = {RGB(7, 16, 10), RGB(10, 22, 14), RGB(14, 31, 20), RGB(20, 31, 24), RGB(26, 31, 28)},
    [MP_BALL_JADE]     = {RGB(1, 11, 7), RGB(1, 16, 10), RGB(2, 22, 14), RGB(13, 25, 20), RGB(23, 28, 26)},
    [MP_BALL_FOREST]   = {RGB(1, 6, 2), RGB(2, 10, 3), RGB(3, 15, 5), RGB(10, 21, 11), RGB(19, 27, 19)},
    [MP_BALL_TEAL]     = {RGB(1, 9, 9), RGB(1, 13, 13), RGB(2, 18, 18), RGB(13, 23, 23), RGB(23, 27, 27)},
    [MP_BALL_AQUA]     = {RGB(4, 16, 14), RGB(6, 22, 20), RGB(8, 31, 28), RGB(17, 31, 29), RGB(25, 31, 30)},
    [MP_BALL_SKY]      = {RGB(7, 12, 16), RGB(10, 17, 22), RGB(14, 24, 31), RGB(20, 27, 31), RGB(26, 29, 31)},
    [MP_BALL_AZURE]    = {RGB(2, 9, 16), RGB(3, 13, 22), RGB(4, 18, 31), RGB(14, 23, 31), RGB(23, 27, 31)},
    [MP_BALL_NAVY]     = {RGB(1, 2, 8), RGB(2, 3, 12), RGB(3, 5, 17), RGB(9, 11, 23), RGB(18, 20, 28)},
    [MP_BALL_INDIGO]   = {RGB(5, 3, 12), RGB(7, 4, 17), RGB(10, 6, 24), RGB(18, 16, 27), RGB(25, 24, 29)},
    [MP_BALL_VIOLET]   = {RGB(10, 3, 16), RGB(14, 4, 22), RGB(19, 6, 31), RGB(24, 16, 31), RGB(28, 24, 31)},
    [MP_BALL_LILAC]    = {RGB(12, 9, 16), RGB(17, 13, 22), RGB(24, 18, 31), RGB(27, 23, 31), RGB(29, 27, 31)},
    [MP_BALL_MAGENTA]  = {RGB(16, 2, 13), RGB(22, 3, 19), RGB(31, 4, 26), RGB(31, 14, 28), RGB(31, 23, 30)},
    [MP_BALL_ROSE]     = {RGB(16, 4, 8), RGB(22, 6, 12), RGB(31, 8, 16), RGB(31, 17, 22), RGB(31, 25, 27)},
    [MP_BALL_SAKURA]   = {RGB(16, 11, 13), RGB(22, 16, 19), RGB(31, 22, 26), RGB(31, 25, 28), RGB(31, 28, 30)},
    [MP_BALL_BROWN]    = {RGB(8, 4, 2), RGB(12, 6, 3), RGB(16, 9, 4), RGB(22, 17, 14), RGB(27, 25, 23)},
    [MP_BALL_COCOA]    = {RGB(5, 2, 1), RGB(8, 4, 2), RGB(12, 7, 4), RGB(18, 13, 9), RGB(25, 21, 17)},
    [MP_BALL_SAND]     = {RGB(14, 12, 8), RGB(19, 17, 11), RGB(27, 23, 15), RGB(29, 26, 21), RGB(30, 29, 27)},
    [MP_BALL_CREAM]    = {RGB(21, 19, 12), RGB(26, 24, 16), RGB(30, 28, 20), RGB(31, 30, 25), RGB(31, 31, 29)},
    [MP_BALL_WHITE]    = {RGB(18, 18, 20), RGB(23, 23, 25), RGB(27, 27, 28), RGB(30, 30, 30), RGB(31, 31, 31)},
    [MP_BALL_GRAY]     = {RGB(8, 8, 9), RGB(12, 12, 13), RGB(17, 17, 18), RGB(22, 22, 23), RGB(27, 27, 28)},
    [MP_BALL_BRONZE]   = {RGB(11, 6, 3), RGB(16, 9, 4), RGB(22, 13, 6), RGB(25, 20, 16), RGB(28, 26, 24)},
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
    [MP_BALL_CRIMSON]  = _("CRIMSON"),
    [MP_BALL_SCARLET]  = _("SCARLET"),
    [MP_BALL_CORAL]    = _("CORAL"),
    [MP_BALL_SALMON]   = _("SALMON"),
    [MP_BALL_PEACH]    = _("PEACH"),
    [MP_BALL_AMBER]    = _("AMBER"),
    [MP_BALL_LEMON]    = _("LEMON"),
    [MP_BALL_OLIVE]    = _("OLIVE"),
    [MP_BALL_LIME]     = _("LIME"),
    [MP_BALL_MINT]     = _("MINT"),
    [MP_BALL_JADE]     = _("JADE"),
    [MP_BALL_FOREST]   = _("FOREST"),
    [MP_BALL_TEAL]     = _("TEAL"),
    [MP_BALL_AQUA]     = _("AQUA"),
    [MP_BALL_SKY]      = _("SKY"),
    [MP_BALL_AZURE]    = _("AZURE"),
    [MP_BALL_NAVY]     = _("NAVY"),
    [MP_BALL_INDIGO]   = _("INDIGO"),
    [MP_BALL_VIOLET]   = _("VIOLET"),
    [MP_BALL_LILAC]    = _("LILAC"),
    [MP_BALL_MAGENTA]  = _("MAGENTA"),
    [MP_BALL_ROSE]     = _("ROSE"),
    [MP_BALL_SAKURA]   = _("SAKURA"),
    [MP_BALL_BROWN]    = _("BROWN"),
    [MP_BALL_COCOA]    = _("COCOA"),
    [MP_BALL_SAND]     = _("SAND"),
    [MP_BALL_CREAM]    = _("CREAM"),
    [MP_BALL_WHITE]    = _("WHITE"),
    [MP_BALL_GRAY]     = _("GRAY"),
    [MP_BALL_BRONZE]   = _("BRONZE"),
};

// Decorations: 1..MP_DECO_COUNT (0 = none)
#include "mp_balldeco_data.h"

#define TAG_MP_BALL_DECO 0x5710    // + decoration: a POKé BALL sheet with it drawn in
#define TAG_MP_BALL_LEGACY 0x5760  // + special: the GENESIS / EPOCH BALL's own art in every frame
#define DECO_ICON_X 7              // where the 5x5 icon art goes on the 24x24 item icon
#define DECO_ICON_Y 7
#define DECO_BALL_X 6              // where the 3x4 ball art goes on the 16x16 ball's first frame
#define DECO_BALL_Y 3
#define DECO_ICON_FIRST_SLOT 12    // the item icon's palette slots 12-15 are free

static const u8 sText_None[] = _("NONE");

#define STYLE_COLOR_HIGH 0xF   // bits 12-15 = 15: not a special ball, color bit 5 set

u8 MP_BallStyle_Color(u16 style)
{
    u8 c = (style & 0xF) | (((style >> 11) & 1) << 4) | (((style >> 12) == STYLE_COLOR_HIGH) << 5);
    return c < MP_BALL_COLORS ? c : MP_BALL_RED;
}

u8 MP_BallStyle_Decoration(u16 style)
{
    u8 d = (style >> 4) & 0x7F;
    return d <= MP_DECO_COUNT ? d : 0;
}

u16 MP_BallStyle_Make(u8 color, u8 decoration)
{
    return (color & 0xF) | ((u16)(decoration & 0x7F) << 4) | ((u16)((color >> 4) & 1) << 11)
         | ((color >> 5) & 1 ? (u16)STYLE_COLOR_HIGH << 12 : 0);
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
    return s <= MP_SPECIAL_COUNT ? s : MP_SPECIAL_NONE;   // 15: a color bit (STYLE_COLOR_HIGH)
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
    return item == ITEM_POKE_BALL;
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
    return MP_DECO_COUNT + 1;   // including "none"
}

const u8 *MP_BallStyle_DecorationName(u8 decoration)
{
    static u8 sName[24];
    if (decoration == 0 || decoration > MP_DECO_COUNT)
        return sText_None;
    MP_AsciiToGame(sName, sDecoNames[decoration - 1], sizeof(sName) - 1);
    return sName;
}

// L / R on the DECOR row: the first decoration of the next / previous group
// (shapes, types, letters, numbers; "none" counts as the start of the list)
u8 MP_BallStyle_DecorationJump(u8 decoration, s8 dir)
{
    s8 i, n = ARRAY_COUNT(sDecoGroupStart);
    if (dir > 0)
    {
        for (i = 0; i < n; i++)
            if (sDecoGroupStart[i] > decoration)
                return sDecoGroupStart[i];
        return 0;
    }
    if (decoration == 0)
        return sDecoGroupStart[n - 1];
    for (i = n - 1; i >= 0; i--)
        if (sDecoGroupStart[i] < decoration)
            return sDecoGroupStart[i];
    return 0;
}

bool8 MP_BallStyle_CanColor(u8 ballId)
{
    return ballId == BALL_POKE;
}

bool8 MP_BallStyle_CanDecorate(u8 ballId)
{
    return ballId == BALL_POKE;
}

static void PutPixel4bpp(u8 *tiles, u8 tilesWide, u8 x, u8 y, u8 c)
{
    u8 *p = tiles + ((y / 8) * tilesWide + x / 8) * TILE_SIZE_4BPP + (y % 8) * 4 + (x % 8) / 2;
    if (x & 1)
        *p = (*p & 0x0F) | (c << 4);
    else
        *p = (*p & 0xF0) | c;
}

// Decorated sheets nothing shows any more go (a battle can see a few of them)
static void FreeUnusedDecoSheets(void)
{
    u8 d, i;
    for (d = 1; d <= MP_DECO_COUNT; d++)
    {
        u16 start = GetSpriteTileStartByTag(TAG_MP_BALL_DECO + d);
        bool8 used = FALSE;
        if (start == 0xFFFF)
            continue;
        for (i = 0; i < MAX_SPRITES && !used; i++)
            used = gSprites[i].inUse && gSprites[i].sheetTileStart == start;
        if (!used)
            FreeSpriteTilesByTag(TAG_MP_BALL_DECO + d);
    }
}

static void FreeUnusedLegacySheets(void)
{
    u8 special, i;
    for (special = MP_SPECIAL_GENESIS; special <= MP_SPECIAL_EPOCH; special++)
    {
        u16 start = GetSpriteTileStartByTag(TAG_MP_BALL_LEGACY + special);
        bool8 used = FALSE;
        if (start == 0xFFFF)
            continue;
        for (i = 0; i < MAX_SPRITES && !used; i++)
            used = gSprites[i].inUse && gSprites[i].sheetTileStart == start;
        if (!used)
            FreeSpriteTilesByTag(TAG_MP_BALL_LEGACY + special);
    }
}

// The GENESIS / EPOCH BALL: the supplied art (mp_legacy_ball_art.h) in place of
// the POKé BALL's, in every frame of the sheet (the opening frames too)
#include "mp_legacy_ball_art.h"

static void LegacyBallSprite(u8 spriteId, u8 special)
{
    struct Sprite *sprite = &gSprites[spriteId];
    u8 art = special == MP_SPECIAL_GENESIS ? 0 : 1, x, y, f;
    u16 tag = TAG_MP_BALL_LEGACY + special, start = GetSpriteTileStartByTag(tag);
    if (start == 0xFFFF)
    {
        static u8 sSheet[384];   // three 16x16 frames, like the POKé BALL's
        struct SpriteSheet sheet = {sSheet, sizeof(sSheet), tag};
        FreeUnusedLegacySheets();
        memset(sSheet, 0, sizeof(sSheet));
        for (f = 0; f < 3; f++)
            for (y = 0; y < 16; y++)
                for (x = 0; x < 16; x++)
                    if (sLegacyBallArt[art][y][x])
                        PutPixel4bpp(sSheet, 2, x, f * 16 + y, sLegacyBallArt[art][y][x]);
        LoadSpriteSheet(&sheet);
        start = GetSpriteTileStartByTag(tag);
        if (start == 0xFFFF)
            return;   // no room: the POKé BALL in its colors
    }
    sprite->oam.tileNum = start + (sprite->oam.tileNum - sprite->sheetTileStart);
    sprite->sheetTileStart = start;
}

// A POKé BALL sprite gets its own copy of the sheet with the decoration drawn
// into the first frame, so other POKé BALLS on the screen keep the plain art
static void DecorateBallSprite(u8 spriteId, u8 deco)
{
    struct Sprite *sprite = &gSprites[spriteId];
    u16 tag = TAG_MP_BALL_DECO + deco, start = GetSpriteTileStartByTag(tag);
    u8 x, y, i;

    if (start == 0xFFFF)
    {
        static u8 sSheet[384];
        struct SpriteSheet sheet = {sSheet, sizeof(sSheet), tag};
        FreeUnusedDecoSheets();
        LZDecompressWram(gBallSpriteSheets[BALL_POKE].data, sSheet);
        for (y = 0; y < MP_DECO_BALL_H; y++)
            for (x = 0; x < MP_DECO_BALL_W; x++)
                if (sDecoArt[deco - 1].ball[y][x])
                    PutPixel4bpp(sSheet, 2, DECO_BALL_X + x, DECO_BALL_Y + y, sDecoArt[deco - 1].ball[y][x]);
        LoadSpriteSheet(&sheet);
        start = GetSpriteTileStartByTag(tag);
        if (start == 0xFFFF)
            return;   // no room: the plain ball
    }
    sprite->oam.tileNum = start + (sprite->oam.tileNum - sprite->sheetTileStart);
    sprite->sheetTileStart = start;
    // Its colors go into the ball palette's free slots 9-15
    for (i = 0; i < ARRAY_COUNT(sDecoBallColors); i++)
    {
        u16 at = OBJ_PLTT_ID(sprite->oam.paletteNum) + MP_DECO_BALL_FIRST_SLOT + i;
        gPlttBufferUnfaded[at] = gPlttBufferFaded[at] = sDecoBallColors[i];
    }
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
            if (special == MP_SPECIAL_GENESIS || special == MP_SPECIAL_EPOCH)
                memcpy(&sPal[1], sLegacyBallColors[special == MP_SPECIAL_GENESIS ? 0 : 1], 4 * sizeof(u16));
            else
            {
                sPal[1] = sSpecialRamps[special - 1][0];
                sPal[2] = sSpecialRamps[special - 1][1];
                sPal[3] = sSpecialRamps[special - 1][2];
                sPal[4] = sSpecialRamps[special - 1][3];
            }
            slot = LoadSpritePalette(&pal);
            if (slot == 0xFF)
                return;
        }
        gSprites[spriteId].oam.paletteNum = slot;
        if (special == MP_SPECIAL_GENESIS || special == MP_SPECIAL_EPOCH)
            LegacyBallSprite(spriteId, special);
        return;
    }
    if (MP_BallStyle_CanColor(ballId) && color != MP_BALL_RED)
    {
        tag = TAG_MP_BALL_COLOR + color;
        slot = IndexOfSpritePaletteTag(tag);
        if (slot == 0xFF)
        {
            static u16 sPal[16];
            struct SpritePalette pal = {sPal, tag};
            LZDecompressWram(gBallSpritePalettes[BALL_POKE].data, sPal);
            RecolorBallPalette(sPal, color);
            slot = LoadSpritePalette(&pal);
        }
        if (slot != 0xFF)
            gSprites[spriteId].oam.paletteNum = slot;
    }
    if (MP_BallStyle_CanDecorate(ballId) && MP_BallStyle_Decoration(style))
        DecorateBallSprite(spriteId, MP_BallStyle_Decoration(style));
}

// The item icon (the summary's preview): the decoration drawn into its tiles
// (a 32x32 sprite, the 24x24 art in the top-left) and its colors into slots 12-15
void MP_BallStyle_DecorateIcon(u8 spriteId, u8 ballId, u16 style)
{
    u8 deco = MP_BallStyle_Decoration(style), x, y, i;
    u8 *tiles;
    const struct MpDecoArt *art;
    if (spriteId >= MAX_SPRITES || deco == 0 || !MP_BallStyle_CanDecorate(ballId) || MP_BallStyle_Special(style))
        return;
    art = &sDecoArt[deco - 1];
    tiles = (u8 *)(OBJ_VRAM0 + gSprites[spriteId].oam.tileNum * TILE_SIZE_4BPP);
    for (y = 0; y < MP_DECO_ICON_SIZE; y++)
        for (x = 0; x < MP_DECO_ICON_SIZE; x++)
            if (art->icon[y][x])
                PutPixel4bpp(tiles, 4, DECO_ICON_X + x, DECO_ICON_Y + y, DECO_ICON_FIRST_SLOT + art->icon[y][x] - 1);
    for (i = 0; i < 4; i++)
    {
        u16 at = OBJ_PLTT_ID(gSprites[spriteId].oam.paletteNum) + DECO_ICON_FIRST_SLOT + i;
        gPlttBufferUnfaded[at] = gPlttBufferFaded[at] = art->iconColors[i];
    }
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
