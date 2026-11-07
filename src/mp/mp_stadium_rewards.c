// POKéMON STADIUM's rewards (docs/STADIUM.md "Rewards"): given when a cup or class is won, bigger with every one of
// the user's restrictions in place (mp_stadium_cup.c Hardness), the greatest with all four; exceptional ones for a win
// with rentals only; the STADIUM RIBBONS for the POKéMON that won it; the achievements' progress (mp_achieve.c
// AK_STADIUM).
//
// A class won for the first time at a hardness (StadiumSave.cleared keeps a bit for each of the 16) gives:
// - a prize POKéMON with perfect IVs: 3 of them, one more for each restriction in place (all six from three on), shiny
//   in a MASTER BALL class won with rentals only or with every restriction (the user, 2026-11-07); from the cup's rental list (STADIUM's own POKéMON, moves and
//   nature), the castle's and the CHALLENGE CUP's from STADIUM's gift POKéMON, MEWTWO (Round 1) and MEW (Round 2) for
//   beating MEWTWO;
// - RARE CANDIES and PP MAXes, money (more for the higher classes and each restriction), a MASTER BALL for rentals only,
//   an ASTRAL BALL in a MASTER BALL class with every restriction.
// Winning again at a hardness already won gives money and a RARE CANDY.
// Every win (a first one or not) puts the STADIUM RIBBONS on the registered team's own POKéMON that don't have them yet
// (the save's three spare gift ribbon slots, MARINE / LAND / SKY: their words are STADIUM's,
// gift_ribbon_descriptions.h): the LEGEND RIBBON with the three restrictions a team of one's own can have (LEVEL CAP,
// SPECIES, HELD ITEMS; rentals can't carry ribbons, so RENTALS ONLY isn't asked).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_stadium.h"
#include "mp_stadium_cup.h"
#include "event_data.h"
#include "item.h"
#include "money.h"
#include "new_game.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "string_util.h"
#include "constants/items.h"
#include "constants/species.h"

#define STADIUM_GIFT_RIBBON_FIRST 65   // gift_ribbon_descriptions.h: STADIUM, MASTER, LEGEND

static struct StadiumPrize sPrize;

static u8 Points(u8 hardness)
{
    u8 n = 0, i;
    for (i = 0; i < 4; i++)
        n += (hardness >> i) & 1;
    return n;
}

// A personality with this nature, shiny for the player or not
static u32 PrizePersonality(u8 nature, bool8 shiny)
{
    u32 otId = GetTrainerId(gSaveBlock2Ptr->playerTrainerId), p;
    u16 lo;
    do
    {
        lo = Random();
        if (shiny)
            p = ((u32)(HIHALF(otId) ^ LOHALF(otId) ^ lo ^ (Random() & 7)) << 16) | lo;
        else
            p = Random32();
    } while (p % NUM_NATURES != nature || (!shiny && IsShinyOtIdPersonality(otId, p)));
    return p;
}

// The perfect IVs: the stats with the best IVs already (STADIUM's DVs) first; the others no lower than 15-31
static void PerfectIvs(u8 ivs[6], u8 perfect)
{
    u8 done[6] = {0}, k, i;
    for (i = 0; i < 6; i++)
    {
        u8 floor = 15 + Random() % 17;
        if (ivs[i] < floor)
            ivs[i] = floor;
    }
    for (k = 0; k < perfect && k < 6; k++)
    {
        s8 best = -1;
        for (i = 0; i < 6; i++)
            if (!done[i] && (best < 0 || ivs[i] > ivs[best]))
                best = i;
        done[best] = TRUE;
        ivs[best] = MAX_PER_STAT_IVS;
    }
}

static const u16 sGiftSpecies[] = {   // STADIUM's gift POKéMON (the castle's prizes)
    SPECIES_BULBASAUR, SPECIES_CHARMANDER, SPECIES_SQUIRTLE, SPECIES_HITMONLEE, SPECIES_HITMONCHAN, SPECIES_EEVEE,
    SPECIES_OMANYTE, SPECIES_KABUTO,
};

static void BuildPrize(struct Pokemon *mon, u8 round, u8 cup, u8 perfect, bool8 shiny)
{
    u8 ivs[6], evs[6], nature, i;
    const struct StadiumMon *sm = NULL;
    u16 n = StadiumCup_RentalCount(round, cup);
    if (cup != STADIUM_CUP_CASTLE && cup != STADIUM_CUP_CHALLENGE && cup != STADIUM_CUP_MEWTWO && n > 0)
        sm = StadiumCup_Rental(round, cup, Random() % n);
    if (sm)
    {
        Stadium_Convert(sm, ivs, evs, &nature);
        PerfectIvs(ivs, perfect);
        CreateMon(mon, sm->species, sm->level, 0, TRUE, PrizePersonality(nature, shiny), OT_ID_PLAYER_ID, 0);
        for (i = 0; i < 6; i++)
        {
            SetMonData(mon, MON_DATA_HP_IV + i, &ivs[i]);
            SetMonData(mon, MON_DATA_HP_EV + i, &evs[i]);
        }
        for (i = 0; i < MAX_MON_MOVES; i++)
            SetMonMoveSlot(mon, sm->moves[i], i);
    }
    else
    {
        u16 species = cup == STADIUM_CUP_MEWTWO ? (round == 0 ? SPECIES_MEWTWO : SPECIES_MEW)
                    : sGiftSpecies[Random() % ARRAY_COUNT(sGiftSpecies)];
        u8 level = cup == STADIUM_CUP_MEWTWO ? 70 : 50;
        CreateMon(mon, species, level, USE_RANDOM_IVS, TRUE, PrizePersonality(Random() % NUM_NATURES, shiny), OT_ID_PLAYER_ID, 0);
        for (i = 0; i < 6; i++)
            ivs[i] = GetMonData(mon, MON_DATA_HP_IV + i);
        PerfectIvs(ivs, perfect);
        for (i = 0; i < 6; i++)
            SetMonData(mon, MON_DATA_HP_IV + i, &ivs[i]);
    }
    CalculateMonStats(mon);
}

static void Line(const char *fmt, const char *a, int n)
{
    if (sPrize.count >= ARRAY_COUNT(sPrize.line))
        return;
    snprintf(sPrize.line[sPrize.count++], sizeof(sPrize.line[0]), fmt, a, n);
}

static void Give(u16 item, u16 count)
{
    char name[24];
    if (count == 0 || !AddBagItem(item, count))
        return;
    CopyItemName(item, gStringVar1);
    MP_GameToAscii(name, gStringVar1, sizeof(name));
    Line(count > 1 ? "%s x%d" : "%s", name, count);
}

// The STADIUM RIBBON (slot 0), MASTER (1), LEGEND (2) on the registered team's own POKéMON (how many got it now) (the party's and the PC's;
// rentals are STADIUM's, the shared and global PCs' copies are friends' to keep as they are)
static u8 RibbonTeam(u8 cup, u8 slotIndex)
{
    struct StadiumSave *s = StadiumCup_Save();
    struct PokemonStorage *own = MP_PersonalStorage();
    u8 given = 0, one = 1, i, j;
    u32 b, p;
    gSaveBlock1Ptr->giftRibbons[slotIndex] = STADIUM_GIFT_RIBBON_FIRST + slotIndex;
    for (i = 0; i < s->team[cup].count; i++)
    {
        struct StadiumTeamSlot *t = &s->team[cup].slot[i];
        u32 pid = GetBoxMonData(&t->mon, MON_DATA_PERSONALITY), ot = GetBoxMonData(&t->mon, MON_DATA_OT_ID);
        bool8 found = FALSE, had;
        if (t->source != STADIUM_SRC_PARTY && t->source != STADIUM_SRC_BOX)
            continue;
        had = GetBoxMonData(&t->mon, MON_DATA_MARINE_RIBBON + slotIndex) != 0;
        SetBoxMonData(&t->mon, MON_DATA_MARINE_RIBBON + slotIndex, &one);
        for (j = 0; j < PARTY_SIZE && !found; j++)
            if (GetMonData(&gPlayerParty[j], MON_DATA_SPECIES) != SPECIES_NONE
             && GetMonData(&gPlayerParty[j], MON_DATA_PERSONALITY) == pid && GetMonData(&gPlayerParty[j], MON_DATA_OT_ID) == ot)
            {
                had = had && GetMonData(&gPlayerParty[j], MON_DATA_MARINE_RIBBON + slotIndex) != 0;
                SetMonData(&gPlayerParty[j], MON_DATA_MARINE_RIBBON + slotIndex, &one);
                found = TRUE;
            }
        for (b = 0; b < TOTAL_BOXES_COUNT && !found; b++)
            for (p = 0; p < IN_BOX_COUNT && !found; p++)
                if (GetBoxMonData(&own->boxes[b][p], MON_DATA_SPECIES) != SPECIES_NONE
                 && GetBoxMonData(&own->boxes[b][p], MON_DATA_PERSONALITY) == pid
                 && GetBoxMonData(&own->boxes[b][p], MON_DATA_OT_ID) == ot)
                {
                    had = had && GetBoxMonData(&own->boxes[b][p], MON_DATA_MARINE_RIBBON + slotIndex) != 0;
                    SetBoxMonData(&own->boxes[b][p], MON_DATA_MARINE_RIBBON + slotIndex, &one);
                    found = TRUE;
                }
        given += found && !had;   // (only the ones that didn't have it yet: the result screen's line)
    }
    return given;
}

static const char *const sRibbonNames[3] = {"STADIUM RIBBON", "STADIUM MASTER RIBBON", "STADIUM LEGEND RIBBON"};

void StadiumRewards_CupWon(u8 round, u8 cup, u8 tier, u8 hardness, bool8 first)
{
    u8 points = Points(hardness), perfect, k;
    bool8 rentals = (hardness & STADIUM_HARD_RENTALS) != 0, all = hardness == STADIUM_HARD_ALL;
    bool8 challenge = cup == STADIUM_CUP_CHALLENGE;
    bool8 masterClass = tier == 3 && StadiumCup_Info(cup)->tiers == 4;   // (the shiny and the ASTRAL BALL: the user)
    const u8 ownHard = STADIUM_HARD_LEVEL | STADIUM_HARD_SPECIES | STADIUM_HARD_ITEMS;
    u32 money;
    memset(&sPrize, 0, sizeof(sPrize));
    sPrize.first = first;
    MPNet_Log("stadium: %s class %d round %d won (hardness %d%s)", StadiumCup_Info(cup)->name, tier, round + 1,
              hardness, first ? ", first time" : "");
    // money: the class, the round, every restriction
    money = cup == STADIUM_CUP_MEWTWO ? 50000 : cup == STADIUM_CUP_CASTLE ? 30000 : 10000 * (tier + 1);
    money = money * (round + 1) * (first ? 1 + points : 1) / (first ? 1 : 5);
    AddMoney(&gSaveBlock1Ptr->money, money);
    snprintf(sPrize.line[sPrize.count++], sizeof(sPrize.line[0]), "MONEY %u", (unsigned)money);
    // the ribbons, on every win: the team's own POKéMON that don't have them yet
    if (!challenge)
    {
        bool8 master = tier == 3 || cup == STADIUM_CUP_CASTLE || cup == STADIUM_CUP_MEWTWO;
        bool8 legend = (hardness & ownHard) == ownHard;
        for (k = 0; k < 3; k++)
            if (k == 0 || (k == 1 && master) || (k == 2 && legend))
            {
                u8 n = RibbonTeam(cup, k);
                if (n)
                    Line("%s x%d", sRibbonNames[k] + (k ? 8 : 0), n);   // ("MASTER RIBBON x6": the panel's column)
                MPNet_Log("stadium: %s for %d", sRibbonNames[k], n);
            }
    }
    if (!first)
    {
        Give(ITEM_RARE_CANDY, 1);
        MPNet_Log("stadium: prizes again: $%u and a RARE CANDY", (unsigned)money);
        return;
    }
    // the prize POKéMON
    perfect = challenge ? 6 : 3 + points;
    {
        struct Pokemon mon;
        char name[16];
        bool8 shiny = masterClass && (rentals || all);   // (the CHALLENGE CUP counts as every restriction)
        BuildPrize(&mon, round, cup, perfect, shiny);
        sPrize.species = GetMonData(&mon, MON_DATA_SPECIES);
        sPrize.personality = GetMonData(&mon, MON_DATA_PERSONALITY);
        sPrize.otId = GetMonData(&mon, MON_DATA_OT_ID);
        sPrize.shiny = shiny;
        sPrize.level = GetMonData(&mon, MON_DATA_LEVEL);
        sPrize.perfect = perfect > 6 ? 6 : perfect;
        GiveMonToPlayer(&mon);
        GetSpeciesName(gStringVar2, sPrize.species);
        MP_GameToAscii(name, gStringVar2, sizeof(name));
        MPNet_Log("stadium: prize %s Lv %d, %d perfect IVs%s", name, sPrize.level, sPrize.perfect, shiny ? ", shiny" : "");
    }
    // items
    Give(ITEM_RARE_CANDY, 3 * (tier + 1 + (cup >= STADIUM_CUP_CASTLE ? 3 : 0)) * (1 + points) * (round + 1));
    Give(ITEM_PP_MAX, points);
    if (rentals)
        Give(ITEM_MASTER_BALL, 1);
    if (all && masterClass)
        Give(ITEM_ASTRAL_BALL, 1);
}

const struct StadiumPrize *StadiumRewards_Last(void)
{
    return &sPrize;
}

// ---- The achievements' progress (mp_achieve.c AK_STADIUM)

static u16 Cleared(u8 r, u8 c, u8 t)
{
    return StadiumCup_Save()->cleared[r][c][t];
}

static bool8 WonWith(u16 bits, u8 mask)
{
    u8 h;
    for (h = 0; h < 16; h++)
        if (((bits >> h) & 1) && (h & mask) == mask)
            return TRUE;
    return FALSE;
}

// Classes won (in ROUND, or either: a class counts once), MASK: only wins whose hardness has all these bits (then each
// round's win counts, and the CHALLENGE CUP's don't: its POKéMON are given)
static s32 ClassesWon(s8 round, s8 cup, u8 mask)
{
    s32 n = 0;
    u8 r, c, t;
    for (c = 0; c < STADIUM_CUP_COUNT; c++)
    {
        if ((cup >= 0 && c != cup) || (mask && c == STADIUM_CUP_CHALLENGE))
            continue;
        for (t = 0; t < StadiumCup_Info(c)->tiers; t++)
        {
            bool8 any = FALSE;
            for (r = 0; r < STADIUM_ROUNDS; r++)
                if ((round < 0 || r == round) && WonWith(Cleared(r, c, t), mask))
                {
                    if (mask)
                        n++;
                    any = TRUE;
                }
            if (any && !mask)
                n++;
        }
    }
    return n;
}

s32 StadiumRewards_Progress(u16 what)
{
    switch (what)
    {
    case STADIUM_ACH_ANY:       return ClassesWon(-1, -1, 0);
    case STADIUM_ACH_PIKA:      return ClassesWon(-1, STADIUM_CUP_PIKA, 0);
    case STADIUM_ACH_PETIT:     return ClassesWon(-1, STADIUM_CUP_PETIT, 0);
    case STADIUM_ACH_POKE:      return ClassesWon(-1, STADIUM_CUP_POKE, 0);
    case STADIUM_ACH_PRIME:     return ClassesWon(-1, STADIUM_CUP_PRIME, 0);
    case STADIUM_ACH_CASTLE:    return ClassesWon(-1, STADIUM_CUP_CASTLE, 0);
    case STADIUM_ACH_MEWTWO:    return ClassesWon(-1, STADIUM_CUP_MEWTWO, 0);
    case STADIUM_ACH_CHALLENGE: return ClassesWon(-1, STADIUM_CUP_CHALLENGE, 0);
    case STADIUM_ACH_ROUND2:    return ClassesWon(1, -1, 0) - ClassesWon(1, STADIUM_CUP_CHALLENGE, 0);
    case STADIUM_ACH_RENTALS:   return ClassesWon(-1, -1, STADIUM_HARD_RENTALS);
    case STADIUM_ACH_HARD:      return ClassesWon(-1, -1, STADIUM_HARD_ALL);
    case STADIUM_ACH_HARD_R1:   return ClassesWon(0, -1, STADIUM_HARD_ALL);
    case STADIUM_ACH_BATTLES:   return StadiumCup_Save()->battlesWon;
    }
    return 0;
}

// Tests (mp stadiumwin): the party registered for CUP and the class won at HARDNESS, as StadiumCup_Advance does
void MP_StadiumRewards_Debug(u8 round, u8 cup, u8 tier, u8 hardness)
{
    struct StadiumSave *s = StadiumCup_Save();
    u8 i, n = 0, a;
    bool8 first;
    if (round >= STADIUM_ROUNDS || cup >= STADIUM_CUP_COUNT || tier >= 4)
        return;
    memset(&s->team[cup], 0, sizeof(s->team[cup]));
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE)
        {
            s->team[cup].slot[n].source = STADIUM_SRC_PARTY;
            s->team[cup].slot[n++].mon = gPlayerParty[i].box;
        }
    s->team[cup].count = n;
    first = !(s->cleared[round][cup][tier] & (1 << hardness));
    s->cleared[round][cup][tier] |= 1 << hardness;
    s->cupsWon++;
    StadiumCup_SaveNow();
    StadiumRewards_CupWon(round, cup, tier, hardness, first);
    for (i = 0; i < sPrize.count; i++)
        MPNet_Log("stadium: prize line %s", sPrize.line[i]);
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE)
            MPNet_Log("stadium: party %d ribbons %d%d%d ivs %d %d %d %d %d %d", i,
                      GetMonData(&gPlayerParty[i], MON_DATA_MARINE_RIBBON), GetMonData(&gPlayerParty[i], MON_DATA_LAND_RIBBON),
                      GetMonData(&gPlayerParty[i], MON_DATA_SKY_RIBBON), GetMonData(&gPlayerParty[i], MON_DATA_HP_IV),
                      GetMonData(&gPlayerParty[i], MON_DATA_ATK_IV), GetMonData(&gPlayerParty[i], MON_DATA_DEF_IV),
                      GetMonData(&gPlayerParty[i], MON_DATA_SPEED_IV), GetMonData(&gPlayerParty[i], MON_DATA_SPATK_IV),
                      GetMonData(&gPlayerParty[i], MON_DATA_SPDEF_IV));
    for (a = 0; a <= STADIUM_ACH_BATTLES; a++)
        MPNet_Log("stadium: achievement progress %d = %d", a, (int)StadiumRewards_Progress(a));
}
