// Moemon Kai: Platinum's script commands that ask about (or change) the party and the BAG, on the game's POKéMON: its
// scrcmd_party.c / scrcmd.c / scrcmd_item.c / scrcmd_fossil.c, `sinnoh SINNOH_PARTY, OP, A, B` (gen_scripts.py). And
// Platinum's ribbons: its 80 (generated/ribbons.txt). The 32 Hoenn ones are the game's own (the contest ones a level
// 0-4, Gen 3's; SINNOH's Super Contest ribbons are those same ones, as s3d_contest.c gives them); the 48 Gen 4 ones
// (SINNOH CHAMP, the ability ribbons, Julia's days, the SYNDICATE's, the FOOTPRINT...) are kept here by POKéMON (its
// personality and trainer ID: what Kai keeps beside a POKéMON elsewhere, the region levels) in "<save>.region" chunk
// RIBN.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "trade.h"
#include "coins.h"
#include "event_data.h"
#include "item.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "mp.h"
#include "constants/easy_chat.h"
#include "easy_chat.h"
#include "s3d.h"
#include "strings.h"
#include "text.h"
#include "menu.h"
#include "window.h"
#include "constants/hold_effects.h"
#include "constants/items.h"
#include "constants/species.h"
#include "constants/moves.h"
#include "constants/coins.h"
#include "constants/weather.h"
#include "field_weather.h"
#include "money.h"
#include "data.h"
#include "battle_main.h"
#include "string_util.h"
#include "constants/game_stat.h"

#include "../data/sinnoh_footprints.h"

// ------------------------------------------------------------------------------------------------- Gen 4 ribbons

#define RIBBON_HOENN_COUNT 32   // (Platinum's RIBBON_HOENN_CHAMPION .. RIBBON_HOENN_WORLD)
#define RIBBON_SINNOH_CHAMP 32
#define RIBBON_COOL 33          // (.. RIBBON_TOUGH_MASTER 52: the Super Contest ones)
#define RIBBON_MAX 80
#define RIBBON_HOLDERS 1024

struct RibbonHolder
{
    u32 personality, otId;
    u8 bits[(RIBBON_MAX - RIBBON_HOENN_COUNT + 7) / 8];   // Platinum's ribbons 32-79
};

static struct
{
    u16 count;
    struct RibbonHolder h[RIBBON_HOLDERS];
} sRib;

void Sinnoh_RibbonsReset(void)
{
    memset(&sRib, 0, sizeof(sRib));
}

const void *Sinnoh_RibbonsData(u32 *size)
{
    *size = sizeof(sRib.count) + 2 + sRib.count * sizeof(struct RibbonHolder);
    return &sRib;
}

void Sinnoh_RibbonsLoad(const void *data, u32 size)
{
    memset(&sRib, 0, sizeof(sRib));
    memcpy(&sRib, data, size < sizeof(sRib) ? size : sizeof(sRib));
    if (sRib.count > RIBBON_HOLDERS)
        sRib.count = RIBBON_HOLDERS;
}

static struct RibbonHolder *Holder(struct Pokemon *mon, bool8 make)
{
    u32 p = GetMonData(mon, MON_DATA_PERSONALITY), o = GetMonData(mon, MON_DATA_OT_ID), i;
    for (i = 0; i < sRib.count; i++)
        if (sRib.h[i].personality == p && sRib.h[i].otId == o)
            return &sRib.h[i];
    if (!make || sRib.count >= RIBBON_HOLDERS)
        return NULL;
    memset(&sRib.h[sRib.count], 0, sizeof(sRib.h[0]));
    sRib.h[sRib.count].personality = p;
    sRib.h[sRib.count].otId = o;
    return &sRib.h[sRib.count++];
}

// Platinum's ribbon RIBBON's place among the game's: the Hoenn contest ribbons and Platinum's Super Contest ones are a
// type's level (field, level)
static bool8 ContestRibbon(u8 ribbon, u8 *field, u8 *level)
{
    static const u8 sTypeFields[5] = {MON_DATA_COOL_RIBBON, MON_DATA_BEAUTY_RIBBON, MON_DATA_CUTE_RIBBON,
                                      MON_DATA_SMART_RIBBON, MON_DATA_TOUGH_RIBBON};
    if (ribbon >= 1 && ribbon <= 20)
        ribbon -= 1;
    else if (ribbon >= RIBBON_COOL && ribbon < RIBBON_COOL + 20)
        ribbon -= RIBBON_COOL;
    else
        return FALSE;
    *field = sTypeFields[ribbon / 4];
    *level = ribbon % 4 + 1;
    return TRUE;
}

static const u8 sHoennOthers[] = {   // (Platinum's RIBBON_HOENN_CHAMPION, then WINNING .. WORLD)
    MON_DATA_CHAMPION_RIBBON, MON_DATA_WINNING_RIBBON, MON_DATA_VICTORY_RIBBON, MON_DATA_ARTIST_RIBBON,
    MON_DATA_EFFORT_RIBBON, MON_DATA_MARINE_RIBBON, MON_DATA_LAND_RIBBON, MON_DATA_SKY_RIBBON, MON_DATA_COUNTRY_RIBBON,
    MON_DATA_NATIONAL_RIBBON, MON_DATA_EARTH_RIBBON, MON_DATA_WORLD_RIBBON,
};

static u8 HoennField(u8 ribbon)
{
    return ribbon == 0 ? sHoennOthers[0] : sHoennOthers[ribbon - 21 + 1];
}

bool8 Sinnoh_MonHasRibbon(struct Pokemon *mon, u8 ribbon)
{
    u8 field, level;
    struct RibbonHolder *h;
    if (ContestRibbon(ribbon, &field, &level))
        return GetMonData(mon, field) >= level;
    if (ribbon < RIBBON_HOENN_COUNT)
        return GetMonData(mon, HoennField(ribbon)) != 0;
    if (ribbon >= RIBBON_MAX || (h = Holder(mon, FALSE)) == NULL)
        return FALSE;
    ribbon -= RIBBON_HOENN_COUNT;
    return (h->bits[ribbon / 8] >> (ribbon % 8)) & 1;
}

u8 Sinnoh_MonGen4RibbonCount(struct Pokemon *mon);

void Sinnoh_MonGiveRibbon(struct Pokemon *mon, u8 ribbon)
{
    u8 field, level, one = TRUE;
    struct RibbonHolder *h;
    u8 given = ribbon;
    if (Sinnoh_MonHasRibbon(mon, ribbon))
        return;
    if (ContestRibbon(ribbon, &field, &level))
        SetMonData(mon, field, &level);
    else if (ribbon < RIBBON_HOENN_COUNT)
        SetMonData(mon, HoennField(ribbon), &one);
    else if (ribbon < RIBBON_MAX && (h = Holder(mon, TRUE)) != NULL)
    {
        ribbon -= RIBBON_HOENN_COUNT;
        h->bits[ribbon / 8] |= 1 << (ribbon % 8);
    }
    else
        return;
    IncrementGameStat(GAME_STAT_RECEIVED_RIBBONS);
    Sinnoh_TvRibbon(mon, given, GetMonData(mon, MON_DATA_RIBBON_COUNT) + Sinnoh_MonGen4RibbonCount(mon));   // (the TV)
}

// how many of Platinum's Gen 4 ribbons a POKéMON has (the summary's ribbon count adds them)
u8 Sinnoh_MonGen4RibbonCount(struct Pokemon *mon)
{
    struct RibbonHolder *h = Holder(mon, FALSE);
    u8 n = 0, i;
    if (h == NULL)
        return 0;
    for (i = RIBBON_SINNOH_CHAMP - RIBBON_HOENN_COUNT; i < RIBBON_MAX - RIBBON_HOENN_COUNT; i++)
        if (i < RIBBON_COOL - RIBBON_HOENN_COUNT || i >= RIBBON_COOL + 20 - RIBBON_HOENN_COUNT)
            n += (h->bits[i / 8] >> (i % 8)) & 1;
    return n;
}

// ----------------------------------------------------------------------------------------------------- the queries

enum
{
    PARTY_ARG,               // A: the next query's extra value (a value or a variable)
    PARTY_REPEATED_SPECIES,  // A species (NONE: any kind twice), B <- Platinum's CountRepeatedSpeciesInParty
    PARTY_GET_RIBBON,        // A slot, ARG ribbon, B <- has it
    PARTY_SET_RIBBON,        // A slot, ARG ribbon
    PARTY_FRIENDSHIP,        // A amount, B slot (IncreasePartyMonFriendship)
    PARTY_NATURE,            // A slot, B <- its nature (HARDY: none there, an EGG)
    PARTY_FIND_NATURE,       // A nature, B <- the first slot with it (0xFF: none)
    PARTY_TYPES,             // ARG slot, A <- its first type, B <- its second
    PARTY_HIDDEN_POWER,      // A slot, B <- its HIDDEN POWER's type
    PARTY_CONTEST_STAT,      // A slot, ARG contest type, B <- its condition
    PARTY_COMBEE_GENDERS,    // B <- 0 no COMBEE, 1 of one gender, 2 of both
    PARTY_MOVE_COUNT,        // A slot, B <- its moves
    PARTY_ITEM_QUANTITY,     // A item, B <- in the BAG
    PARTY_FOSSIL_COUNT,      // B <- fossils in the BAG
    PARTY_FOOTPRINT,         // A slot, B <- leaves a footprint, ARG's variable <- DR. FOOTSTEP's type
    PARTY_OUTSIDER,          // A slot, B <- caught by someone else
    PARTY_SEEN,              // A species, B <- seen
    PARTY_CAN_ADD_COINS,     // A amount, B <- the COIN CASE has room
    PARTY_ADD_MONEY,         // A amount
    PARTY_REMOVE_MONEY,      // A amount
    PARTY_ENOUGH_MONEY,      // A amount, B <- the player has it
    PARTY_WEATHER,           // B <- the map's weather as Platinum's OVERWORLD_WEATHER_*
    PARTY_SELECTED_MOVE,     // B <- the move slot the summary's move choice picked (Platinum's MOVE_NOT_SELECTED: none)
    PARTY_BUFFER_MOVE,       // ARG text slot <- party slot A's move in move slot B (a variable)
    PARTY_FORGET_MOVE,       // party slot A forgets move slot B (variables; the ones after move up)
    PARTY_HAS_MOVE,          // A slot, ARG move, B <- it knows it
    PARTY_EV_TOTAL,          // A slot, B <- its EVs' total
    PARTY_BUFFER_NATURE,     // ARG text slot <- nature A's name
    PARTY_TEACH_MOVE,        // the summary for party slot A to pick a move to forget for ARG (the script waits)
    PARTY_TEACH_SLOT,        // B <- the move slot picked (4: none, Platinum's LEARNED_MOVES_MAX)
    PARTY_BUFFER_TYPE,       // ARG text slot <- type A's name
    PARTY_GET_MOVE,          // A slot, ARG move slot, B <- the move there
    PARTY_TRADE_INIT,        // A: Platinum's NPC_TRADE_* (InitNPCTrade)
    PARTY_TRADE_REQUESTED,   // B <- the species that trade asks for (GetNPCTradeRequestedSpecies)
    PARTY_TRADE_START,       // A party slot: VAR_0x8004 / 8005 set for the game's in-game trade (StartNPCTrade)
    PARTY_TRADE_END,         // VAR_0x8004 / 8005 put back (FinishNPCTrade; A 1: after the trade scene)
    PARTY_FOSSIL_FIND,       // A how many, B <- the first fossil the BAG holds that many of, ARG's variable <- its place
    PARTY_FOSSIL_SPECIES,    // A item, B <- the POKeMON it revives as
    PARTY_PRIZE,             // A: VEILSTONE's prize, B <- its item, ARG's variable <- its price in coins
    PARTY_HAS_COINS,         // A coins, B <- the COIN CASE holds that many
    PARTY_TUTOR_CAN,         // A slot, ARG tutor, B <- it can learn a move there it doesn't know
    PARTY_TUTOR_MENU,        // A slot (0xFF: every move), ARG tutor, B: the variable the menu answers in (shown next)
    PARTY_TUTOR_AFFORD,      // A move, B <- the BAG holds its SHARDS
    PARTY_TUTOR_PAY,         // A move: its SHARDS taken
    PARTY_RESET_MOVE,        // A slot, ARG move slot, B move (a value or variable): learnt there, its PP full
    PARTY_SHARD_COST,        // A move, ARG SINNOH's menu text bank, B its first SHARDS line: the cost window
    PARTY_SHARD_CLOSE,       // the cost window closed
    PARTY_LEARN_OPEN,        // A slot, ARG move: the game's move relearner offering only it (OpenMoveTutorMenu; the
                             // script runs special TeachMoveRelearnerMove after)
    PARTY_LEARN_RESULT,      // B <- the move slot it went in, 0xFF: not learnt (CheckLearnedTutorMove)
    PARTY_TOUGH_WORD,        // A text slot <- a new word for the EASY CHAT, B <- its number (0xFFFF: none left)
    PARTY_TOUGH_ALL,         // B <- every such word is known
    PARTY_FORMS_ON,          // the POKeDEX's form viewer turned on (TurnOnPokedexFormDetection: pokedex.c MpDexForms*)
};

extern u16 gMoveRelearnerOnlyMove;
static u16 sLearnSlot, sLearnKept;

// (Platinum's move tutors: ROUTE 212's, the SURVIVAL AREA's and SNOWPOINT's, src/data/sinnoh_tutors.h from
// tools/sinnoh/gen_tutors.py; Platinum's src/overlay005/scrcmd_move_tutor.c)
#include "../data/sinnoh_tutors.h"
static const u16 sShards[4] = {ITEM_RED_SHARD, ITEM_BLUE_SHARD, ITEM_YELLOW_SHARD, ITEM_GREEN_SHARD};
static u8 sShardWindow = WINDOW_NONE;

static bool8 TutorKnows(struct Pokemon *mon, u16 move)
{
    u32 i;
    for (i = 0; i < MAX_MON_MOVES; i++)
        if (GetMonData(mon, MON_DATA_MOVE1 + i) == move)
            return TRUE;
    return FALSE;
}

// The tutor's moves this POKeMON can learn and doesn't know, in Platinum's order (NULL: every move the tutor teaches)
static u32 TutorMoves(struct Pokemon *mon, u16 tutor, u16 *out)
{
    u32 i, n = 0;
    u16 species = mon ? Maker_SpeciesBase(GetMonData(mon, MON_DATA_SPECIES)) : SPECIES_NONE;   // (Maker M3b: a new one, its base's)
    for (i = 0; i < SINNOH_TUTOR_COUNT; i++)
    {
        if (sSinnohTutorMoves[i][5] != tutor)
            continue;
        if (mon != NULL && (species >= NUM_SPECIES || !(sSinnohTutorMask[species][i / 8] & (1 << (i % 8)))
                            || TutorKnows(mon, sSinnohTutorMoves[i][0])))
            continue;
        if (out)
            out[n] = sSinnohTutorMoves[i][0];
        n++;
    }
    return n;
}

static const u16 *TutorCost(u16 move)
{
    u32 i;
    for (i = 0; i < SINNOH_TUTOR_COUNT; i++)
        if (sSinnohTutorMoves[i][0] == move)
            return &sSinnohTutorMoves[i][1];
    return NULL;
}

// (the PRIZE EXCHANGE's prizes, Platinum's src/scrcmd_game_corner_prize.c sGameCornerPrizeData)
static const u16 sPrizes[][2] = {
    {ITEM_SILK_SCARF, 1000}, {ITEM_WIDE_LENS, 1000}, {ITEM_ZOOM_LENS, 1000}, {ITEM_METRONOME, 1000}, {ITEM_TM90, 2000},
    {ITEM_TM58, 2000}, {ITEM_TM75, 4000}, {ITEM_TM32, 4000}, {ITEM_TM44, 6000}, {ITEM_TM89, 6000}, {ITEM_TM10, 6000},
    {ITEM_TM27, 8000}, {ITEM_TM21, 8000}, {ITEM_TM35, 10000}, {ITEM_TM24, 10000}, {ITEM_TM13, 10000},
    {ITEM_TM29, 10000}, {ITEM_TM74, 15000}, {ITEM_TM68, 20000},
};

// (Platinum's fossils and what they revive as, in its order: src/field/fossil.c sFossilPokemonMap)
static const u16 sFossils[][2] = {{ITEM_OLD_AMBER, SPECIES_AERODACTYL}, {ITEM_HELIX_FOSSIL, SPECIES_OMANYTE},
                                  {ITEM_DOME_FOSSIL, SPECIES_KABUTO}, {ITEM_ROOT_FOSSIL, SPECIES_LILEEP},
                                  {ITEM_CLAW_FOSSIL, SPECIES_ANORITH}, {ITEM_ARMOR_FOSSIL, SPECIES_SHIELDON},
                                  {ITEM_SKULL_FOSSIL, SPECIES_CRANIDOS}};

// (Platinum's in-game trades are SINNOH's in the game's own table: src/trade.c InGameTrade, src/data/sinnoh_ingame_trades.h)
u16 GetInGameTradeSpeciesInfo(void);
static u16 sTrade, sTradeKept[2];
static bool8 sTradeStarted;

static u16 sArg, sArgVar;   // (the extra value, and the variable it came from: a result's place)

static struct Pokemon *Slot(u16 slot)
{
    return &gPlayerParty[slot < PARTY_SIZE ? slot : 0];
}

static void Out(u16 var, u16 value)
{
    if (var >= VARS_START)
        VarSet(var, value);
}

// Gen 3's HIDDEN POWER type from the IVs (FIGHTING .. DARK)
static u8 HiddenPowerType(struct Pokemon *mon)
{
    static const u8 sTypes[] = {TYPE_FIGHTING, TYPE_FLYING, TYPE_POISON, TYPE_GROUND, TYPE_ROCK, TYPE_BUG, TYPE_GHOST,
                                TYPE_STEEL, TYPE_FIRE, TYPE_WATER, TYPE_GRASS, TYPE_ELECTRIC, TYPE_PSYCHIC, TYPE_ICE,
                                TYPE_DRAGON, TYPE_DARK};
    u32 bits = (GetMonData(mon, MON_DATA_HP_IV) & 1) | ((GetMonData(mon, MON_DATA_ATK_IV) & 1) << 1)
             | ((GetMonData(mon, MON_DATA_DEF_IV) & 1) << 2) | ((GetMonData(mon, MON_DATA_SPEED_IV) & 1) << 3)
             | ((GetMonData(mon, MON_DATA_SPATK_IV) & 1) << 4) | ((GetMonData(mon, MON_DATA_SPDEF_IV) & 1) << 5);
    return sTypes[bits * 15 / 63];
}

void Sinnoh_PartyCall(u16 op, u16 a, u16 b)
{
    u16 value = VarGet(a);
    u32 i;
    switch (op)
    {
    case PARTY_ARG:
        sArg = value;
        sArgVar = a;
        break;
    case PARTY_REPEATED_SPECIES:
    {
        u16 seen[PARTY_SIZE] = {0}, n = 0;
        for (i = 0; i < gPlayerPartyCount && i < PARTY_SIZE; i++)
        {
            u16 s;
            u32 j;
            if (GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
                continue;
            s = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);
            if (value == SPECIES_NONE)
            {
                seen[i] = s;
                for (j = 0; j < i; j++)
                    if (seen[j] == s)
                    {
                        Out(b, 1);
                        return;
                    }
            }
            else if (s == value)
                n++;
        }
        Out(b, n);
        break;
    }
    case PARTY_GET_RIBBON:
        Out(b, Sinnoh_MonHasRibbon(Slot(value), sArg));
        break;
    case PARTY_SET_RIBBON:
        Sinnoh_MonGiveRibbon(Slot(value), sArg);
        MPNet_Log("sinnoh: slot %d got ribbon %d", value, sArg);
        break;
    case PARTY_FRIENDSHIP:
    {
        struct Pokemon *mon = Slot(VarGet(b));
        u16 f = GetMonData(mon, MON_DATA_FRIENDSHIP);
        if (value > 0)
        {
            if (GetItemHoldEffect(GetMonData(mon, MON_DATA_HELD_ITEM)) == HOLD_EFFECT_FRIENDSHIP_UP)
                value = value * 150 / 100;
            if (GetMonData(mon, MON_DATA_POKEBALL) == ITEM_LUXURY_BALL)
                value++;
            if (GetMonData(mon, MON_DATA_MET_LOCATION) == GetCurrentRegionMapSectionId())
                value++;
        }
        f = f + value > MAX_FRIENDSHIP ? MAX_FRIENDSHIP : f + value;
        SetMonData(mon, MON_DATA_FRIENDSHIP, &f);
        break;
    }
    case PARTY_NATURE:
        if (value >= gPlayerPartyCount || GetMonData(Slot(value), MON_DATA_IS_EGG))
            Out(b, 0);   // (NATURE_HARDY)
        else
            Out(b, GetNature(Slot(value)));
        break;
    case PARTY_FIND_NATURE:
        for (i = 0; i < gPlayerPartyCount; i++)
            if (!GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG) && GetNature(&gPlayerParty[i]) == value)
                break;
        Out(b, i < gPlayerPartyCount ? i : 0xFF);
        break;
    case PARTY_TYPES:
    {
        u16 s = GetMonData(Slot(sArg), MON_DATA_SPECIES);
        Out(a, gSpeciesInfo[s].types[0]);
        Out(b, gSpeciesInfo[s].types[1]);
        break;
    }
    case PARTY_HIDDEN_POWER:
        Out(b, HiddenPowerType(Slot(value)));
        break;
    case PARTY_CONTEST_STAT:
        Out(b, GetMonData(Slot(value), MON_DATA_COOL + (sArg % 5)));
        break;
    case PARTY_COMBEE_GENDERS:
    {
        bool8 male = FALSE, female = FALSE;
        for (i = 0; i < gPlayerPartyCount; i++)
        {
            struct Pokemon *mon = &gPlayerParty[i];
            if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_COMBEE || GetMonData(mon, MON_DATA_IS_EGG))
                continue;
            if (GetMonGender(mon) == MON_MALE)
                male = TRUE;
            if (GetMonGender(mon) == MON_FEMALE)
                female = TRUE;
        }
        Out(b, male && female ? 2 : male || female ? 1 : 0);
        break;
    }
    case PARTY_MOVE_COUNT:
    {
        u16 n = 0;
        if (!GetMonData(Slot(value), MON_DATA_IS_EGG))
            for (i = 0; i < MAX_MON_MOVES; i++)
                n += GetMonData(Slot(value), MON_DATA_MOVE1 + i) != MOVE_NONE;
        Out(b, n);
        break;
    }
    case PARTY_ITEM_QUANTITY:
        Out(b, CountTotalItemQuantityInBag(value));
        break;
    case PARTY_FOSSIL_COUNT:
    {
        u16 n = 0;
        for (i = 0; i < ARRAY_COUNT(sFossils); i++)
            n += CountTotalItemQuantityInBag(sFossils[i][0]);
        Out(b, n);
        break;
    }
    case PARTY_FOSSIL_FIND:
        // (Platinum's FindFossilAtThreshold: the first fossil the BAG holds A of; none: 0 and the list's end)
        for (i = 0; i < ARRAY_COUNT(sFossils); i++)
            if (CountTotalItemQuantityInBag(sFossils[i][0]) >= value)
                break;
        Out(b, i < ARRAY_COUNT(sFossils) ? sFossils[i][0] : ITEM_NONE);
        Out(sArgVar, i);
        break;
    case PARTY_FOSSIL_SPECIES:
        // (Platinum's GetSpeciesFromFossil: 0 for anything not a fossil)
        for (i = 0; i < ARRAY_COUNT(sFossils); i++)
            if (sFossils[i][0] == value)
                break;
        Out(b, i < ARRAY_COUNT(sFossils) ? sFossils[i][1] : SPECIES_NONE);
        MPNet_Log("party: fossil %d revives as %d", value, i < ARRAY_COUNT(sFossils) ? sFossils[i][1] : 0);
        break;
    case PARTY_FOOTPRINT:
    {
        u16 s = GetMonData(Slot(value), MON_DATA_SPECIES);
        Out(b, s < NUM_SPECIES ? sSinnohFootprints[s][0] : 0);
        Out(sArgVar, s < NUM_SPECIES ? sSinnohFootprints[s][1] : 0);
        break;
    }
    case PARTY_OUTSIDER:
        Out(b, (GetMonData(Slot(value), MON_DATA_OT_ID) & 0xFFFF) != (u16)(gSaveBlock2Ptr->playerTrainerId[0]
                                                                           | (gSaveBlock2Ptr->playerTrainerId[1] << 8)));
        break;
    case PARTY_SEEN:
        Out(b, GetSetPokedexFlag(SpeciesToNationalPokedexNum(value), FLAG_GET_SEEN));
        break;
    case PARTY_CAN_ADD_COINS:
        Out(b, GetCoins() + value <= MAX_COINS);
        break;
    case PARTY_ADD_MONEY:
        AddMoney(&gSaveBlock1Ptr->money, value);
        break;
    case PARTY_REMOVE_MONEY:
        RemoveMoney(&gSaveBlock1Ptr->money, value);
        break;
    case PARTY_ENOUGH_MONEY:
        Out(b, IsEnoughMoney(&gSaveBlock1Ptr->money, value));
        break;
    case PARTY_SELECTED_MOVE:
        Out(b, gSpecialVar_0x8005 < MAX_MON_MOVES ? gSpecialVar_0x8005 : 0xFF);
        break;
    case PARTY_BUFFER_MOVE:
    {
        u8 *StringVar(u16 n);
        StringCopy(StringVar(sArg), gMoveNames[GetMonData(Slot(value), MON_DATA_MOVE1 + (VarGet(b) % MAX_MON_MOVES))]);
        break;
    }
    case PARTY_FORGET_MOVE:
    {
        // (Emerald's move deleter's, on its special variables, put back after)
        void MoveDeleterForgetMove(void);
        u16 keep4 = gSpecialVar_0x8004, keep5 = gSpecialVar_0x8005;
        gSpecialVar_0x8004 = value;
        gSpecialVar_0x8005 = VarGet(b) % MAX_MON_MOVES;
        MPNet_Log("party: slot %d forgets move %d", value, (int)GetMonData(Slot(value), MON_DATA_MOVE1 + gSpecialVar_0x8005));
        MoveDeleterForgetMove();
        for (i = 0; i < MAX_MON_MOVES && GetMonData(Slot(value), MON_DATA_MOVE1 + i); i++)
            ;
        MPNet_Log("party: slot %d knows %d moves", value, (int)i);
        gSpecialVar_0x8004 = keep4, gSpecialVar_0x8005 = keep5;
        break;
    }
    case PARTY_HAS_MOVE:
    {
        u16 n = FALSE;
        if (!GetMonData(Slot(value), MON_DATA_IS_EGG))
            for (i = 0; i < MAX_MON_MOVES; i++)
                n |= GetMonData(Slot(value), MON_DATA_MOVE1 + i) == sArg;
        Out(b, n);
        break;
    }
    case PARTY_EV_TOTAL:
    {
        u32 n = 0;
        for (i = 0; i < NUM_STATS; i++)
            n += GetMonData(Slot(value), MON_DATA_HP_EV + i);
        Out(b, n);
        break;
    }
    case PARTY_BUFFER_NATURE:
    {
        u8 *StringVar(u16 n);
        extern const u8 *const gNatureNamePointers[];
        StringCopy(StringVar(sArg), gNatureNamePointers[value % NUM_NATURES]);
        break;
    }
    case PARTY_TEACH_MOVE:
    {
        // (the game's summary picking a move to forget for a new one, as its TMs and tutors do)
        void ShowSelectMovePokemonSummaryScreen(struct Pokemon *mons, u8 monIndex, u8 maxMonIndex, void (*callback)(void),
                                                u16 newMove);
        void CB2_ReturnToFieldContinueScript(void);
        ShowSelectMovePokemonSummaryScreen(gPlayerParty, value, gPlayerPartyCount - 1, CB2_ReturnToFieldContinueScript, sArg);
        break;
    }
    case PARTY_TEACH_SLOT:
    {
        u8 GetMoveSlotToReplace(void);
        Out(b, GetMoveSlotToReplace() < MAX_MON_MOVES ? GetMoveSlotToReplace() : MAX_MON_MOVES);
        break;
    }
    case PARTY_BUFFER_TYPE:
    {
        u8 *StringVar(u16 n);
        StringCopy(StringVar(sArg), gTypeNames[value % NUMBER_OF_MON_TYPES]);
        break;
    }
    case PARTY_GET_MOVE:
        Out(b, GetMonData(Slot(value), MON_DATA_MOVE1 + (sArg % MAX_MON_MOVES)));
        break;
    case PARTY_PRIZE:
        Out(b, sPrizes[value % ARRAY_COUNT(sPrizes)][0]);
        Out(sArgVar, sPrizes[value % ARRAY_COUNT(sPrizes)][1]);
        break;
    case PARTY_HAS_COINS:
        Out(b, GetCoins() >= value);
        break;
    case PARTY_TUTOR_CAN:
        Out(b, GetMonData(Slot(value), MON_DATA_IS_EGG) ? FALSE : TutorMoves(Slot(value), sArg, NULL) != 0);
        break;
    case PARTY_TUTOR_MENU:
    {
        void Sinnoh_MenuBegin(u16 var, bool8 canExit);
        void Sinnoh_MenuAddLine(const u8 *text, u16 value);
        u16 moves[SINNOH_TUTOR_COUNT];
        u32 n = TutorMoves(value < PARTY_SIZE ? Slot(value) : NULL, sArg, moves);
        Sinnoh_MenuBegin(b, TRUE);
        for (i = 0; i < n; i++)
            Sinnoh_MenuAddLine(gMoveNames[moves[i]], moves[i]);
        Sinnoh_MenuAddLine(gText_Exit, 0xFFFE);   // (Platinum's EXIT: MENU_CANCEL)
        MPNet_Log("party: tutor %d teaches %d moves here", sArg, (int)n);
        break;
    }
    case PARTY_TUTOR_AFFORD:
    {
        const u16 *cost = TutorCost(value);
        u16 ok = cost != NULL;
        for (i = 0; ok && i < 4; i++)
            if (cost[i] && !CheckBagHasItem(sShards[i], cost[i]))
                ok = FALSE;
        Out(b, ok);
        break;
    }
    case PARTY_TUTOR_PAY:
    {
        const u16 *cost = TutorCost(value);
        for (i = 0; cost && i < 4; i++)
            if (cost[i])
                RemoveBagItem(sShards[i], cost[i]);
        break;
    }
    case PARTY_RESET_MOVE:
    {
        struct Pokemon *mon = Slot(value);
        u16 move = VarGet(b);
        SetMonMoveSlot(mon, move, sArg % MAX_MON_MOVES);
        RemoveMonPPBonus(mon, sArg % MAX_MON_MOVES);
        MPNet_Log("party: slot %d learns move %d in move slot %d", value, move, sArg % MAX_MON_MOVES);
        break;
    }
    case PARTY_SHARD_COST:
    {
        // (Platinum's window: "RED SHARDS  X 2" and the rest the move costs, at the top right)
        const u16 *cost = TutorCost(value);
        struct WindowTemplate t;
        u32 rows = 0, y = 0;
        for (i = 0; cost && i < 4; i++)
            rows += cost[i] != 0;
        if (rows == 0 || sShardWindow != WINDOW_NONE)
            break;
        t = CreateWindowTemplate(0, 19, 1, 10, rows * 2, 15, 0x141 - 10 * 8);
        sShardWindow = AddWindow(&t);
        FillWindowPixelBuffer(sShardWindow, PIXEL_FILL(1));
        PutWindowTilemap(sShardWindow);
        DrawStdWindowFrame(sShardWindow, FALSE);
        for (i = 0; i < 4; i++)
        {
            u8 line[64];
            if (!cost[i])
                continue;
            AddTextPrinterParameterized(sShardWindow, FONT_NARROW, S3D_Text(sArg, VarGet(b) + i), 0, y * 16 + 1, TEXT_SKIP_DRAW, NULL);
            ConvertIntToDecimalStringN(gStringVar1, cost[i], STR_CONV_MODE_LEFT_ALIGN, 2);
            StringExpandPlaceholders(line, S3D_Text(sArg, VarGet(b) + 4));
            AddTextPrinterParameterized(sShardWindow, FONT_NARROW, line, 56, y * 16 + 1, TEXT_SKIP_DRAW, NULL);
            y++;
        }
        CopyWindowToVram(sShardWindow, COPYWIN_FULL);
        break;
    }
    case PARTY_SHARD_CLOSE:
        if (sShardWindow != WINDOW_NONE)
        {
            ClearStdWindowAndFrame(sShardWindow, TRUE);
            RemoveWindow(sShardWindow);
            sShardWindow = WINDOW_NONE;
        }
        break;
    case PARTY_LEARN_OPEN:
        sLearnKept = gSpecialVar_0x8004;
        sLearnSlot = value;
        gSpecialVar_0x8004 = value;
        gMoveRelearnerOnlyMove = sArg;
        MPNet_Log("party: slot %d may learn move %d", value, sArg);
        break;
    case PARTY_LEARN_RESULT:
    {
        u16 slot = 0xFF;
        if (gSpecialVar_0x8004 == TRUE && gMoveRelearnerOnlyMove != MOVE_NONE)
            for (i = 0; i < MAX_MON_MOVES; i++)
                if (GetMonData(Slot(sLearnSlot), MON_DATA_MOVE1 + i) == gMoveRelearnerOnlyMove)
                    slot = i;
        MPNet_Log("party: move %d learnt in move slot %d", gMoveRelearnerOnlyMove, slot);
        gMoveRelearnerOnlyMove = MOVE_NONE;
        gSpecialVar_0x8004 = sLearnKept;
        Out(b, slot);
        break;
    }
    case PARTY_TOUGH_WORD:
    {
        // (Platinum's "tough words" a day from SNOWPOINT's man: the game's EASY CHAT's TRENDY SAYINGS, unlocked one at
        // a time as MAUVILLE's old man does: easy_chat.c UnlockRandomTrendySaying)
        u8 *StringVar(u16 n);
        u16 word = UnlockRandomTrendySaying();
        if (word == EC_EMPTY_WORD)
            Out(b, 0xFFFF);
        else
        {
            CopyEasyChatWord(StringVar(value), word);
            Out(b, word & EC_MASK_INDEX);
        }
        MPNet_Log("party: a trendy saying unlocked: %d", word == EC_EMPTY_WORD ? -1 : (int)(word & EC_MASK_INDEX));
        break;
    }
    case PARTY_TOUGH_ALL:
    {
        u16 n = 0;
        for (i = 0; i < NUM_TRENDY_SAYINGS; i++)
            n += (gSaveBlock1Ptr->unlockedTrendySayings[i / 8] >> (i % 8)) & 1;
        Out(b, n == NUM_TRENDY_SAYINGS);
        break;
    }
    case PARTY_FORMS_ON:
    {
        void Sinnoh_FormViewerEnable(void);
        Sinnoh_FormViewerEnable();
        break;
    }
    case PARTY_TRADE_INIT:
        sTrade = value;
        break;
    case PARTY_TRADE_REQUESTED:
    {
        u16 kept = gSpecialVar_0x8004;
        gSpecialVar_0x8004 = sTrade;
        Out(b, GetInGameTradeSpeciesInfo());   // (it fills STR_VAR_1 / 2 too, as the game's trades do)
        gSpecialVar_0x8004 = kept;
        break;
    }
    case PARTY_TRADE_START:
        sTradeKept[0] = gSpecialVar_0x8004;
        sTradeKept[1] = gSpecialVar_0x8005;
        sTradeStarted = TRUE;
        gSpecialVar_0x8005 = value;
        gSpecialVar_0x8004 = sTrade;
        MPNet_Log("party: Platinum's trade %d for slot %d's %d", sTrade, value, (int)GetMonData(Slot(value), MON_DATA_SPECIES));
        break;
    case PARTY_TRADE_END:
        if (sTradeStarted)
        {
            gSpecialVar_0x8004 = sTradeKept[0];
            gSpecialVar_0x8005 = sTradeKept[1];
            sTradeStarted = FALSE;
        }
        break;
    case PARTY_WEATHER:
    {
        // (the game's weather, as SINNOH's maps were given it, back in Platinum's numbers)
        u8 w = GetSavedWeather();
        Out(b, w == WEATHER_RAIN ? 2 : w == WEATHER_DOWNPOUR ? 3 : w == WEATHER_RAIN_THUNDERSTORM ? 4
             : w == WEATHER_SNOW ? 5 : w == WEATHER_VOLCANIC_ASH ? 9 : w == WEATHER_SANDSTORM ? 10
             : w == WEATHER_FOG_HORIZONTAL || w == WEATHER_FOG_DIAGONAL ? 14 : w == WEATHER_SHADE ? 1 : 0);
        break;
    }
    }
}
