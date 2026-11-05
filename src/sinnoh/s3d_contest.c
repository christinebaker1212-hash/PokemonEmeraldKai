// Moemon Kai: Platinum's SUPER CONTESTS (the user, 2026-11-02: "Super Contests should be added, as well as their unique
// functions"). Platinum's rules, read from pret/pokeplatinum and kept here as its code has them (docs/SUPER_CONTEST.md):
// - who enters: three opponents from contest_data.narc (gSinnohContestOpponents, by rank, type, game stage; tools/sinnoh/
//   contest_data.py makes the tables when the game is built), three judges (two, and a special one in the middle);
// - the VISUAL round: the condition score (the type's condition, its two neighbours and sheen, a scarf's bonus) in
//   hearts by the rank's thresholds, and the dress-up's points for the theme (gSinnohContestThemes) in hearts;
// - the DANCE round (s3d_contest_ui.c draws it): steps judged by their distance from the beat, the backups by the lead's;
// - the ACTING round: four turns of a move to one of three judges, the move's appeal, its contest effect (24 kinds),
//   the judges' voltage, the judges' impressions; the computer's choices by Platinum's table (gSinnohContestAi);
// - the final scoring: each round's best gets the round's share (a third each), ties broken by chance.
// What Kai has, SINNOH uses: the contest's ribbons are this game's (a win in rank N gives the type's ribbon N + 1, Hoenn's
// COOL RIBBON to COOL RIBBON MASTER, named in Platinum's own words: they are the same ribbons), and entering rank N
// asks for the ribbon of rank N - 1 as Platinum asks for its own; the scarves are the game's; POFFINS are POKéBLOCKS.
// The hall's scripts are Platinum's (scripts_contests.s), their commands come here as `sinnoh SINNOH_CONTEST`.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "easy_chat.h"
#include "event_data.h"
#include "overworld.h"
#include "constants/game_stat.h"
#include "item.h"
#include "main.h"
#include "party_menu.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "mp.h"
#include "mp_region.h"
#include "s3d.h"
#include "s3d_contest.h"

extern const u16 gSinnohContestBanks[];
extern const u8 gSinnohContestMoves[];
extern const u8 gSinnohContestEffects[];
extern const u16 gSinnohContestAi[];
extern const u8 gSinnohContestOpponents[];
extern const u8 gSinnohContestJudges[];
extern const u8 gSinnohContestDressUps[];
extern const u8 gSinnohContestThemes[];
extern u8 gSinnohStrVar[3][64];

struct SinnohContest gSinnohContest;
#define C gSinnohContest

#define OPP_SIZE 38
#define DRESS_SIZE 84
#define CONTEST_MOVES 468

// ----------------------------------------------------------------------------------------------------- the tables

static u16 Le16(const u8 *p)
{
    return p[0] | p[1] << 8;
}

static u16 OpponentCount(void)
{
    return Le16(gSinnohContestOpponents);
}

static const u8 *Opponent(u16 i)
{
    return gSinnohContestOpponents + 2 + i * OPP_SIZE;
}

u8 Contest_MoveType(u16 move)
{
    return move < CONTEST_MOVES && gSinnohContestMoves[move] != 0xFF ? gSinnohContestMoves[move] & 7 : 0xFF;
}

u8 Contest_MoveEffect(u16 move)
{
    return move < CONTEST_MOVES && gSinnohContestMoves[move] != 0xFF ? gSinnohContestMoves[move] >> 3 : 0;
}

s8 Contest_EffectAppeal(u8 effect)
{
    return effect < CONTEST_EFFECT_COUNT ? (s8)gSinnohContestEffects[effect * 26] : 0;
}

// the effect's message for its variant (0-4), in contest_acting_competition, and its format (s3d_contest_ui.c)
bool8 Contest_EffectMessage(u8 effect, u8 variant, u16 *msg, u16 *format)
{
    const u8 *p = gSinnohContestEffects + effect * 26 + 6 + variant * 4;
    if (effect >= CONTEST_EFFECT_COUNT || variant > 4 || Le16(p) == 0xFFFF)
        return FALSE;
    *msg = Le16(p);
    *format = Le16(p + 2);
    return TRUE;
}

void Contest_EffectDescription(u8 effect, u16 *line1, u16 *line2)
{
    const u8 *p = gSinnohContestEffects + effect * 26 + 2;
    *line1 = Le16(p);
    *line2 = Le16(p + 2);
}

u16 Contest_Bank(u8 which)
{
    return gSinnohContestBanks[which];
}

const u8 *Contest_Text(u8 bank, u16 msg)
{
    return S3D_Text(gSinnohContestBanks[bank], msg);
}

u8 Contest_ThemePoints(u8 theme, u16 accessory)
{
    return theme < 12 && accessory < 100 ? gSinnohContestThemes[theme * 100 + accessory] : 0;
}

// ----------------------------------------------------------------------------------------------------- chance

// Platinum's LCRNG, seeded when the contest begins (its Contest_SetLCRNGSeed): every choice of the contest's own
u16 Contest_Random(void)
{
    C.rng = C.rng * 1103515245 + 24691;
    return C.rng >> 16;
}

// ----------------------------------------------------------------------------------------------------- string slots

u8 *Contest_Slot(u16 slot)
{
    return slot == 0 ? gStringVar1 : slot == 1 ? gStringVar2 : slot == 2 ? gStringVar3 : gSinnohStrVar[(slot - 3) % 3];
}

static u8 EntryToId(u16 entry)
{
    return (CONTEST_ENTRANTS - 1 - entry) & 3;   // (Platinum's entry numbers count from the other end)
}

// ----------------------------------------------------------------------------------------------------- the contest

bool8 Contest_IsPractice(void)
{
    return C.mode == CONTEST_PRACTICE_VISUAL || C.mode == CONTEST_PRACTICE_DANCE || C.mode == CONTEST_PRACTICE_ACTING;
}

bool8 Contest_HasRound(u8 round)
{
    switch (C.mode)
    {
    case CONTEST_UNK0: return round == ROUND_VISUAL || round == ROUND_ACTING;
    case CONTEST_UNK1: return round == ROUND_VISUAL || round == ROUND_DANCE;
    case CONTEST_OFFICIAL: return TRUE;
    case CONTEST_PRACTICE_VISUAL: case CONTEST_VISUAL: return round == ROUND_VISUAL;
    case CONTEST_PRACTICE_DANCE: case CONTEST_DANCE: return round == ROUND_DANCE;
    default: return round == ROUND_ACTING;
    }
}

// each round's share of the 100 points (Platinum's ov17_02251860, in hundredths)
static u16 RoundShare(u8 round)
{
    switch (C.mode)
    {
    case CONTEST_UNK0: return round == ROUND_VISUAL ? 6000 : round == ROUND_ACTING ? 4000 : 0;
    case CONTEST_UNK1: return round == ROUND_VISUAL ? 7000 : round == ROUND_DANCE ? 3000 : 0;
    case CONTEST_OFFICIAL: return 3333;
    default: return Contest_HasRound(round) ? 10000 : 0;
    }
}

static u8 Condition(struct Pokemon *mon, u8 type)
{
    static const u8 sData[5] = {MON_DATA_COOL, MON_DATA_BEAUTY, MON_DATA_CUTE, MON_DATA_SMART, MON_DATA_TOUGH};
    return GetMonData(mon, sData[type]);
}

// the opponents Platinum would bring for this rank, type and kind of contest (unk_02094EDC.c sub_02094F04)
static void PickOpponents(bool8 gameDone)
{
    u8 list[128], n = 0, star = 0xFF, starCount = 0, single, i, k;
    single = C.mode == CONTEST_VISUAL || C.mode == CONTEST_DANCE || C.mode == CONTEST_ACTING;
    for (i = 0; i < OpponentCount() && n < sizeof(list); i++)
    {
        u16 f = Le16(Opponent(i) + 22);
        u8 stage = (f >> 10) & 3;
        if ((f & 7) != C.rank)
            continue;
        if (gameDone ? stage == 1 : (stage == 2 || stage == 3))
            continue;
        if (Contest_IsPractice() ? !(f & (1 << 9)) : single ? !(f & (1 << 8)) : (f & (3 << 8)) != 0)
            continue;
        if (!(f & (1 << (3 + C.type))))
            continue;
        list[n++] = i;
    }
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        C.e[i].opponent = 0xFF;
    if (n == 0)
        return;
    if (single)
    {
        for (i = 1; i < CONTEST_ENTRANTS && i - 1 < n; i++)
            C.e[i].opponent = list[i - 1];
        return;
    }
    for (i = 0; i < n; i++)
        starCount += ((Le16(Opponent(list[i]) + 22) >> 10) & 3) == 3;
    if (starCount)
    {
        // (one of the special guests, MASTER rank after the game: always among them)
        k = Contest_Random() % starCount;
        for (i = 0; i < n; i++)
            if (((Le16(Opponent(list[i]) + 22) >> 10) & 3) == 3 && k-- == 0)
            {
                star = list[i];
                break;
            }
    }
    for (i = 1; i < CONTEST_ENTRANTS && n; i++)
    {
        u8 pick = Contest_Random() % n;
        if (((Le16(Opponent(list[pick]) + 22) >> 10) & 3) == 3)
        {
            if (n == starCount)
                break;
            i--;
            continue;
        }
        C.e[i].opponent = list[pick];
        memmove(&list[pick], &list[pick + 1], n - pick - 1);
        n--;
    }
    if (star != 0xFF)
        C.e[1 + Contest_Random() % (CONTEST_ENTRANTS - 1)].opponent = star;
}

// the judges: two of the rank and type's, and a special one in the middle (sub_020954F0)
static void PickJudges(void)
{
    u8 plain[16], special[16], np = 0, ns = 0, i;
    u16 count = Le16(gSinnohContestJudges);
    for (i = 0; i < count && i < 16; i++)
    {
        u16 f = Le16(gSinnohContestJudges + 2 + i * 4 + 2);
        u8 mark = (f >> (C.type * 2)) & 3;
        if (((f >> 10) & 3) != (C.rank > 3 ? 3 : C.rank) || !mark)
            continue;
        if (mark > 1)
            special[ns++] = i;
        else
            plain[np++] = i;
    }
    C.judge[0] = np > 0 ? plain[0] : 0;
    C.judge[2] = np > 1 ? plain[1] : 1;
    C.judge[1] = ns ? special[Contest_Random() % ns] : 2;   // (Platinum's unk_10E: the middle one is special)
}

u16 Contest_JudgeName(u8 judge)
{
    return Le16(gSinnohContestJudges + 2 + C.judge[judge % 3] * 4);
}

static void FillOpponent(struct ContestEntrant *e)
{
    const u8 *o = Opponent(e->opponent);
    u8 i;
    e->species = Le16(o);
    for (i = 0; i < MAX_MON_MOVES; i++)
        e->moves[i] = Le16(o + 2 + i * 2);
    e->gfx = Le16(o + 10);
    StringCopyN(e->nick, Contest_Text(CB_OPPONENT_NAMES, Le16(o + 12)), sizeof(e->nick) - 1);
    e->nick[sizeof(e->nick) - 1] = EOS;
    StringCopyN(e->trainer, Contest_Text(CB_OPPONENT_NAMES, Le16(o + 14)), sizeof(e->trainer) - 1);
    e->trainer[sizeof(e->trainer) - 1] = EOS;
    for (i = 0; i < 6; i++)
        e->cond[i] = o[16 + i];
    e->gender = (Le16(o + 22) >> 12) & 3;
    e->ai = (Le16(o + 22) >> 14) & 3;
    e->fame = o[36];
    e->personality = 0x10000 * (e->opponent + 1) + e->species;
    e->otId = 0xF0F0F0F;
    e->item = ITEM_NONE;
    // the dress-up its trainer brings for this theme (member 2)
    {
        u8 d = o[24 + C.theme];
        const u8 *p = gSinnohContestDressUps + 2 + d * DRESS_SIZE;
        e->wornCount = 0;
        e->backdrop = 0;
        if (d < Le16(gSinnohContestDressUps))
        {
            for (i = 0; i < p[80] && i < CONTEST_MAX_WORN; i++)
            {
                e->worn[i].accessory = p[i * 4];
                e->worn[i].x = p[i * 4 + 1];
                e->worn[i].y = p[i * 4 + 2];
            }
            e->wornCount = i;
            e->backdrop = (s8)p[82];
        }
    }
}

static void FillPlayer(struct ContestEntrant *e, u8 slot)
{
    struct Pokemon *mon = &gPlayerParty[slot < PARTY_SIZE ? slot : 0];
    u8 i;
    e->opponent = 0xFF;
    e->species = GetMonData(mon, MON_DATA_SPECIES);
    e->personality = GetMonData(mon, MON_DATA_PERSONALITY);
    e->otId = GetMonData(mon, MON_DATA_OT_ID);
    for (i = 0; i < MAX_MON_MOVES; i++)
        e->moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);
    GetMonData(mon, MON_DATA_NICKNAME, e->nick);
    e->nick[POKEMON_NAME_LENGTH] = EOS;
    StringGet_Nickname(e->nick);
    StringCopyN(e->trainer, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    e->trainer[PLAYER_NAME_LENGTH] = EOS;
    for (i = 0; i < 5; i++)
        e->cond[i] = Condition(mon, i);
    e->cond[5] = GetMonData(mon, MON_DATA_SHEEN);
    e->item = GetMonData(mon, MON_DATA_HELD_ITEM);
    e->gender = gSaveBlock2Ptr->playerGender;
    e->ai = 0;
    e->fame = Contest_Fame(mon, C.type);
    e->gfx = 0;
    e->wornCount = 0;
    e->backdrop = 0;
}

// JUBILIFE TV's dress-up studio: the party POKéMON dressed with no contest around it (s3d_contest_ui.c)
void Contest_StudioEntrant(u8 slot)
{
    memset(&C, 0, sizeof(C));
    C.rng = Random();
    FillPlayer(&C.e[0], slot);
}

// how famous a POKéMON is in this type: one and its ribbons of the type (CalcMonContestFame)
u8 Contest_Fame(struct Pokemon *mon, u8 type)
{
    return 1 + GetMonData(mon, MON_DATA_COOL_RIBBON + type);
}

// the theme of the VISUAL round (sub_02095A74)
static u8 PickTheme(void)
{
    u8 list[12], n = 0;
    if (C.rank >= 3)
        return Contest_Random() % 12;
    list[n++] = 2;
    list[n++] = 3;
    list[n++] = 4;
    if (C.rank >= 1)
    {
        list[n++] = 0;
        list[n++] = 1;
        list[n++] = 5;
    }
    if (C.rank >= 2)
    {
        list[n++] = 6;
        list[n++] = 7;
        list[n++] = 8;
    }
    return list[Contest_Random() % n];
}

void Contest_New(u8 rank, u8 type, u8 mode, u8 slot)
{
    u8 i;
    memset(&C, 0, sizeof(C));
    C.rng = Random() | (u32)Random() << 16;
    if (getenv("MOEMON_TEST_CONTEST_SEED"))
        C.rng = strtoul(getenv("MOEMON_TEST_CONTEST_SEED"), NULL, 0);
    C.rank = rank > 3 ? 3 : rank;
    C.type = type % 5;
    C.mode = mode <= CONTEST_PRACTICE_ACTING + 1 ? mode : CONTEST_OFFICIAL;
    C.slot = slot;
    C.theme = PickTheme();
    C.made = TRUE;
    FillPlayer(&C.e[0], slot);
    PickOpponents(FlagGet(FLAG_SYS_GAME_CLEAR));
    PickJudges();
    for (i = 1; i < CONTEST_ENTRANTS; i++)
    {
        if (C.e[i].opponent == 0xFF)
            C.e[i].opponent = i - 1 < OpponentCount() ? i - 1 : 0;
        FillOpponent(&C.e[i]);
    }
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        C.order[i] = Contest_IsPractice() ? CONTEST_ENTRANTS - 1 - i : i;
    MPNet_Log("sinnoh contest: rank %d type %d mode %d theme %d, opponents %d %d %d, judges %d %d %d", C.rank, C.type,
              C.mode, C.theme, C.e[1].opponent, C.e[2].opponent, C.e[3].opponent, C.judge[0], C.judge[1], C.judge[2]);
}

// ----------------------------------------------------------------------------------------------------- VISUAL

// the condition score (ov17_0223F374): the type's condition, half the neighbours' and the sheen, a scarf's bonus
static u16 ConditionScore(const struct ContestEntrant *e)
{
    static const u8 sNeighbour[5][2] = {{4, 1}, {0, 2}, {1, 3}, {2, 4}, {3, 0}};
    static const u16 sScarf[5] = {ITEM_RED_SCARF, ITEM_BLUE_SCARF, ITEM_PINK_SCARF, ITEM_GREEN_SCARF, ITEM_YELLOW_SCARF};
    u32 s = e->cond[C.type] + (e->cond[sNeighbour[C.type][0]] + e->cond[sNeighbour[C.type][1]] + e->cond[5]) / 2;
    u32 k = 100;
    if (e->item == sScarf[C.type])
        k = 110;
    else if (e->item == sScarf[sNeighbour[C.type][0]] || e->item == sScarf[sNeighbour[C.type][1]])
        k = 105;
    return s * k / 100;
}

static u8 ConditionHearts(u16 score)
{
    static const u16 sAt[4][8] = {{10, 20, 30, 40, 50, 60, 70, 80},
                                  {90, 110, 130, 150, 170, 190, 210, 230},
                                  {170, 200, 230, 260, 290, 320, 350, 380},
                                  {320, 360, 400, 440, 480, 520, 560, 600}};
    u8 n = 0;
    while (n < 8 && score >= sAt[C.rank][n])
        n++;
    return n;
}

static u8 DressHearts(u16 points)
{
    static const u8 sAt[4][3] = {{3, 5, 8}, {5, 10, 15}, {7, 15, 23}, {10, 20, 30}};
    u8 n = 1;
    if (points == 0)
        return 0;
    while (n <= 3 && points > sAt[C.rank][n - 1])
        n++;
    return n;
}

void Contest_ScoreVisual(void)
{
    u8 i, k;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        struct ContestEntrant *e = &C.e[i];
        u16 points = 0;
        for (k = 0; k < e->wornCount; k++)
            points += Contest_ThemePoints(C.theme, e->worn[k].accessory);
        e->condScore = ConditionScore(e);
        e->condHearts = ConditionHearts(e->condScore);
        e->dressPoints = points;
        e->dressHearts = DressHearts(points);
        e->visual = e->condHearts + e->dressHearts;
        MPNet_Log("sinnoh contest: visual %d: condition %d (%d hearts), dress-up %d (%d hearts)", i, e->condScore,
                  e->condHearts, points, e->dressHearts);
    }
}

// ----------------------------------------------------------------------------------------------------- DANCE

// the song of the dance (ov17_0223DAD0.c Unk_ov17_02253084 / Unk_ov17_0225312C): its tempo, how many steps the lead
// makes, the judging distances (in Platinum's 30ths of a second) and the pattern
static const struct ContestSong sSongs[7] = {
    {120, 3, {2, 2, 3, 3}}, {120, 4, {2, 2, 3, 3}}, {100, 4, {1, 1, 2, 2}}, {60, 4, {2, 2, 3, 3}},
    {150, 4, {1, 1, 2, 2}}, {132, 4, {1, 1, 2, 2}}, {140, 4, {1, 1, 2, 2}},
};

const struct ContestSong *Contest_Song(u8 *pattern)
{
    static const u8 sPick[4][5][2] = {
        {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        {{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}},
        {{2, 1}, {3, 1}, {4, 0}, {5, 0}, {6, 0}},
        {{2, 1}, {3, 1}, {4, 0}, {5, 0}, {6, 0}},
    };
    *pattern = sPick[C.rank][C.type][1];
    return &sSongs[sPick[C.rank][C.type][0]];
}

// a step judged (ov17_0224DE54 / ov17_0224DD90): grade 0-1 EXCELLENT, 2-3 GOOD, 4 MISS by its distance from the
// half-beat; a backup's step must have the lead's at the same place, in the same direction
u8 Contest_JudgeStep(u16 distance, const u8 near[4], bool8 backup, bool8 leadThere, u8 dir, u8 leadDir)
{
    u8 grade;
    if (backup && !leadThere)
        return 4;
    grade = distance <= near[0] ? 0 : distance <= near[1] ? 1 : distance <= near[2] ? 2 : distance <= near[3] ? 3 : 4;
    if (backup && grade < 4 && dir != leadDir)
        return 4;
    return grade;
}

s8 Contest_StepPoints(u8 grade)
{
    return grade <= 1 ? 2 : grade <= 3 ? 1 : 0;
}

// ----------------------------------------------------------------------------------------------------- ACTING

static const u8 sVoltageTable[5][5] = {   // (ov17_02243AC4.c: a move's type against the contest's: +1 / 0 / -1)
    {1, 0, 2, 2, 0}, {0, 1, 0, 2, 2}, {2, 0, 1, 0, 2}, {2, 2, 0, 1, 0}, {0, 2, 2, 0, 1},
};

static s8 VoltageChange(u16 move)
{
    u8 t = Contest_MoveType(move);
    if (t > 4)
        return 0;
    return sVoltageTable[C.type][t] == 1 ? 10 : sVoltageTable[C.type][t] == 2 ? -10 : 0;
}

s8 Contest_VoltageChangeFor(u16 move)
{
    return VoltageChange(move);
}

static u8 SameJudge(u8 id)
{
    u8 i, n = 0;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        n += C.act.judgeOf[i] == C.act.judgeOf[id];
    return n - 1;
}

// a move can't be performed twice in a row (but for the effect that allows it: ov17_02243A98)
bool8 Contest_MoveUsable(u8 id, u16 move)
{
    if (move == MOVE_NONE)
        return FALSE;
    if (move == C.act.lastMove[id] && C.act.repeatOk[id] != move)
        return FALSE;
    return TRUE;
}

static void Note(u8 kind, u8 who, u8 other, u16 msg, u16 format, s16 number)
{
    if (C.act.notes >= CONTEST_NOTES)
        return;
    C.act.note[C.act.notes].kind = kind;
    C.act.note[C.act.notes].who = who;
    C.act.note[C.act.notes].other = other;
    C.act.note[C.act.notes].msg = msg;
    C.act.note[C.act.notes].format = format;
    C.act.note[C.act.notes].number = number;
    C.act.notes++;
}

static void EffectNote(u8 who, u8 variant, u8 other, s16 number)
{
    u16 msg, format;
    if (Contest_EffectMessage(C.act.p[who].effect, variant, &msg, &format))
        Note(NOTE_EFFECT, who, other, msg, format, number);
}

// the turn's order changed by an effect (ov17_02246578 / ov17_02246668 / ov17_02246AB0)
static void OrderFirst(u8 who)
{
    u8 i, held = 4;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        if (C.act.p[i].orderKind == 1 && C.act.p[i].orderPlace == 0 && i != who)
            held = i;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        if (i != who && C.act.p[i].orderKind && C.act.p[i].orderKind != 3)
            C.act.p[i].orderPlace++;
    C.act.p[who].orderKind = 1;
    C.act.p[who].orderPlace = 0;
    EffectNote(who, held != 4 ? 1 : 0, held, 0);
}

static void OrderLast(u8 who)
{
    u8 i, held = 4;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        if (C.act.p[i].orderKind == 2 && C.act.p[i].orderPlace == 3 && i != who)
            held = i;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        if (i != who && C.act.p[i].orderKind && C.act.p[i].orderKind != 3 && C.act.p[i].orderPlace > 0)
            C.act.p[i].orderPlace--;
    C.act.p[who].orderKind = 2;
    C.act.p[who].orderPlace = 3;
    EffectNote(who, held != 4 ? 1 : 0, held, 0);
}

static void OrderRandom(u8 who)
{
    u8 left[4] = {0, 1, 2, 3}, n = 4, i;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        u8 k = Contest_Random() % n;
        C.act.p[left[k]].orderKind = 3;
        C.act.p[left[k]].orderPlace = i;
        memmove(&left[k], &left[k + 1], n - k - 1);
        n--;
    }
    EffectNote(who, 0, 0, 0);
}

// a performer's own effect, as it performs (Platinum's phase 3 effects: ov17_02245F14.c)
static void OwnEffect(u8 who, u8 place)
{
    struct ContestPerformer *p = &C.act.p[who];
    u8 i;
    switch (p->effect)
    {
    case CONTEST_EFFECT_FIRST_NEXT_TURN:
        OrderFirst(who);
        break;
    case CONTEST_EFFECT_LAST_NEXT_TURN:
        OrderLast(who);
        break;
    case CONTEST_EFFECT_2_HEARTS_WHEN_VOLTAGE_UP:
        if (VoltageChange(p->move) > 0 && !p->suppressed)
        {
            p->bonus += 20;
            EffectNote(who, 0, 0, 0);
        }
        break;
    case CONTEST_EFFECT_CONSECUTIVE_USE:
        if (p->move != C.act.lastMove[who])
        {
            C.act.nextRepeatOk[who] = p->move;
            EffectNote(who, 0, 0, 0);
        }
        break;
    case CONTEST_EFFECT_VOLTAGE:
        p->bonus += C.act.voltageBefore[C.act.judgeOf[who]];
        EffectNote(who, 0, 0, C.act.voltageBefore[C.act.judgeOf[who]] / 10);
        break;
    case CONTEST_EFFECT_LOWERS_VOLTAGE:
        if (!p->noVoltageDown)
        {
            bool8 any = FALSE;
            for (i = 0; i < 3; i++)
                any |= C.act.voltage[i] != 0;
            if (any)
            {
                for (i = 0; i < 3; i++)
                    if (C.act.voltage[i] > 0)
                        C.act.voltage[i] -= 10;
                EffectNote(who, 0, 0, 0);
            }
        }
        break;
    case CONTEST_EFFECT_DOUBLE_NEXT_TURN:
        C.act.doubleNext[who] = p->effect;
        break;
    case CONTEST_EFFECT_STEAL_VOLTAGE:
        if (place > 0 && C.act.p[C.act.order[place - 1]].voltageBonus)
        {
            u8 before = C.act.order[place - 1];
            p->bonus += C.act.p[before].voltageBonus;
            EffectNote(who, 0, before, 0);
        }
        break;
    case CONTEST_EFFECT_SUPPRESS_VOLTAGE:
        for (i = place; i < CONTEST_ENTRANTS; i++)
            C.act.p[C.act.order[i]].suppressed = TRUE;
        EffectNote(who, 0, 0, 0);
        break;
    case CONTEST_EFFECT_RANDOM_ORDER:
        OrderRandom(who);
        break;
    case CONTEST_EFFECT_DOUBLE_FINAL_ACT:
        if (C.act.turn == CONTEST_TURNS - 1)
        {
            EffectNote(who, 0, 0, p->points / 10);
            p->bonus += p->points;
        }
        break;
    case CONTEST_EFFECT_LOW_VOLTAGE_ADVANTAGE:
    {
        static const u8 sBonus[6] = {40, 30, 20, 10, 0, 0};
        u8 v = C.act.voltageBefore[C.act.judgeOf[who]] / 10;
        p->bonus += sBonus[v > 5 ? 5 : v];
        EffectNote(who, 0, 0, sBonus[v > 5 ? 5 : v] / 10);
        break;
    }
    case CONTEST_EFFECT_FIRST_PERFORMANCE_ADVANTAGE:
        if (place == 0)
        {
            p->bonus += 20;
            EffectNote(who, 0, 0, 0);
        }
        break;
    case CONTEST_EFFECT_FINAL_PERFORMANCE_ADVANTAGE:
        if (place == CONTEST_ENTRANTS - 1)
        {
            p->bonus += 20;
            EffectNote(who, 0, 0, 0);
        }
        break;
    case CONTEST_EFFECT_NO_VOLTAGE_DOWN:
        for (i = place; i < CONTEST_ENTRANTS; i++)
            C.act.p[C.act.order[i]].noVoltageDown = TRUE;
        EffectNote(who, 0, 0, 0);
        break;
    case CONTEST_EFFECT_TWO_VOLTAGE_IN_A_ROW_ADVANTAGE:
        if (place > 0)
        {
            u8 before = C.act.order[place - 1];
            if (!p->suppressed && !C.act.p[before].suppressed && VoltageChange(p->move) > 0
             && VoltageChange(C.act.p[before].move) > 0)
            {
                p->bonus += 30;
                EffectNote(who, 0, before, 0);
            }
        }
        break;
    case CONTEST_EFFECT_HIGH_SCORE_LATER_TURN:
    {
        static const u8 sBonus[4] = {10, 20, 30, 40};
        p->bonus += sBonus[place];
        EffectNote(who, place, 0, 0);
        break;
    }
    case CONTEST_EFFECT_MAX_VOLTAGE_ADVANTAGE:
        if (place > 0 && C.act.p[C.act.order[place - 1]].voltageBonus >= 50)
        {
            p->bonus += 30;
            EffectNote(who, 0, 0, 0);
        }
        break;
    }
}

// the judge's voltage (ov17_022463C4): up or down by the move's type; full (5), a bonus of 5 hearts, 8 from the special
static void Voltage(u8 who)
{
    struct ContestPerformer *p = &C.act.p[who];
    u8 j = C.act.judgeOf[who];
    s8 d = VoltageChange(p->move);
    if (d > 0 && !p->suppressed)
    {
        C.act.voltage[j] += d;
        if (C.act.voltage[j] >= 50)
            C.act.voltage[j] = 50;
        Note(NOTE_VOLTAGE_UP, who, j, 0, 0, Contest_MoveType(p->move));
    }
    else if (d < 0 && !p->noVoltageDown && C.act.voltage[j] > 0)
    {
        C.act.voltage[j] += d;
        if (C.act.voltage[j] < 0)
            C.act.voltage[j] = 0;
        Note(NOTE_VOLTAGE_DOWN, who, j, 0, 0, Contest_MoveType(p->move));
    }
    else
        d = 0;
    if (C.act.voltage[j] >= 50)
        p->voltageBonus = j == 1 ? 80 : 50;
    if (d > 0 && C.act.voltage[j] / 10 == 4)
        Note(NOTE_EXCITED, who, j, 0, 0, 0);
    else if (d > 0 && C.act.voltage[j] >= 50)
        Note(NOTE_WILD, who, j, 0, 0, p->voltageBonus / 10);
    p->points += p->voltageBonus;
}

// one performer's turn (states C100 of ov17_0223B140.c): appeal, its effect, the effects that came before, voltage
void Contest_Perform(u8 place)
{
    u8 who = C.act.order[place], i;
    struct ContestPerformer *p = &C.act.p[who];
    C.act.notes = 0;
    memcpy(C.act.voltageBefore, C.act.voltage, sizeof(C.act.voltage));
    p->points = Contest_EffectAppeal(p->effect);
    Note(NOTE_PERFORMED, who, C.act.judgeOf[who], 0, 0, 0);
    for (i = 0; i < place; i++)
        if (C.act.judgeOf[C.act.order[i]] == C.act.judgeOf[who])
        {
            Note(NOTE_SAME_JUDGE, who, 0, 0, 0, 0);
            break;
        }
    if (p->effect == CONTEST_EFFECT_UNIQUE_JUDGE || p->effect == CONTEST_EFFECT_ALL_SAME_JUDGE)
        EffectNote(who, 4, 0, 0);   // (told before the judges are counted)
    Note(NOTE_APPEAL, who, 0, 0, 0, p->points / 10);
    p->bonus = 0;
    OwnEffect(who, place);
    p->points += p->bonus;
    // the effect from the turn before that doubles this one (ov17_02246228)
    if (C.act.doubled[who] != CONTEST_EFFECT_NONE)
    {
        s16 more = p->points;
        u16 msg, format;
        p->points += more;
        if (Contest_EffectMessage(C.act.doubled[who], 0, &msg, &format))
            Note(NOTE_EFFECT, who, 0, msg, format, more / 10);
    }
    Voltage(who);
    for (i = 0; i < 3; i++)
        if (C.act.voltage[i] >= 50)
            C.act.voltage[i] = 0;   // (a full judge starts again: ov17_02246018)
}

// after everyone performed (state 6 on of ov17_0223C350): the judges' impressions, the judge effects, the pity points
void Contest_EndTurn(void)
{
    static const u8 sImpressed[4] = {30, 20, 10, 0};
    u8 i, best = 0;
    C.act.notes = 0;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        C.act.p[i].points += sImpressed[SameJudge(i)];
    for (i = 0; i < 3; i++)
    {
        u8 n = 0, k;
        for (k = 0; k < CONTEST_ENTRANTS; k++)
            n += C.act.judgeOf[k] == i;
        if (n)
            Note(NOTE_IMPRESSED, 0, i, 0, 0, n);
    }
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        u8 who = C.act.order[i];
        struct ContestPerformer *p = &C.act.p[who];
        u8 same = SameJudge(who);
        switch (p->effect)
        {
        case CONTEST_EFFECT_DOUBLED_JUDGE:
            p->points += same * 20;
            EffectNote(who, same > 3 ? 3 : same, 0, 0);
            break;
        case CONTEST_EFFECT_UNIQUE_JUDGE:
            if (same == 0)
                p->points += 30;
            EffectNote(who, same == 0 ? 0 : 1, 0, 0);
            break;
        case CONTEST_EFFECT_ALL_SAME_JUDGE:
            if (same == 3)
                p->points += 150;
            EffectNote(who, same == 3 ? 0 : 1, 0, 0);
            break;
        }
    }
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        C.act.before[i] = C.act.p[i].points;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        u8 who = C.act.order[i], k;
        struct ContestPerformer *p = &C.act.p[who];
        if (p->effect != CONTEST_EFFECT_PITY_POINTS)
            continue;
        for (k = 0; k < CONTEST_ENTRANTS; k++)
            if (k != who && C.act.before[k] < C.act.before[who])
                break;
        if (k == CONTEST_ENTRANTS)
        {
            p->points += 30;
            EffectNote(who, 0, 0, 0);
        }
    }
    (void)best;
}

// the turn's points kept, the next turn's order (ov17_02243B0C: the lowest first; effects first or last), the moves
void Contest_NextTurn(void)
{
    s16 key[4];
    u8 id[4], was[4], i, k;
    static const s16 sFirst[4] = {-30000, -29990, -29980, -29970}, sLast[4] = {29970, 29980, 29990, 30000},
                     sRandom[4] = {-10000, -500, 500, 10000};
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        C.act.total[i] += C.act.p[i].points;
        C.act.lastPoints[i] = C.act.p[i].points;
    }
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        id[i] = i;
        key[i] = C.act.lastPoints[i];
        for (k = 0; k < CONTEST_ENTRANTS; k++)
            if (C.act.order[k] == i)
                was[i] = k;
        if (C.act.p[i].orderKind == 1)
            key[i] = sFirst[C.act.p[i].orderPlace & 3];
        else if (C.act.p[i].orderKind == 2)
            key[i] = sLast[C.act.p[i].orderPlace & 3];
        else if (C.act.p[i].orderKind == 3)
            key[i] = sRandom[C.act.p[i].orderPlace & 3];
    }
    for (i = 0; i < CONTEST_ENTRANTS - 1; i++)
        for (k = CONTEST_ENTRANTS - 1; k > i; k--)
            if (key[k - 1] > key[k] || (key[k - 1] == key[k] && was[k - 1] < was[k]))
            {
                s16 t = key[k];
                u8 u;
                key[k] = key[k - 1];
                key[k - 1] = t;
                u = id[k];
                id[k] = id[k - 1];
                id[k - 1] = u;
                u = was[k];
                was[k] = was[k - 1];
                was[k - 1] = u;
            }
    memcpy(C.act.order, id, sizeof(id));
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        C.act.lastMove[i] = C.act.p[i].move;
        C.act.repeatOk[i] = C.act.nextRepeatOk[i];
        C.act.nextRepeatOk[i] = MOVE_NONE;
        C.act.doubled[i] = C.act.doubleNext[i];
        C.act.doubleNext[i] = CONTEST_EFFECT_NONE;
    }
    memset(C.act.p, 0, sizeof(C.act.p));
    C.act.turn++;
}

void Contest_BeginActing(void)
{
    u8 i;
    memset(&C.act, 0, sizeof(C.act));
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        C.act.order[i] = C.order[i];
}

// the turn's choices made: the effects and who chose which judge
void Contest_Choose(u8 who, u16 move, u8 judge)
{
    C.act.p[who].move = move;
    C.act.p[who].effect = Contest_MoveEffect(move);
    C.act.judgeOf[who] = judge % 3;
}

// ---- the computer's choice (ov17_02246ECC.c): each of its moves scored by Platinum's table of conditions

struct AiMove
{
    u16 move;
    u8 effect, type;
    s16 score, judge[3];
};

static bool8 HasEffect(const struct AiMove *m, u8 effect)
{
    u8 i;
    for (i = 0; i < MAX_MON_MOVES; i++)
        if (m[i].move && m[i].effect == effect)
            return TRUE;
    return FALSE;
}

static bool8 HasType(const struct AiMove *m, u8 type)
{
    u8 i;
    for (i = 0; i < MAX_MON_MOVES; i++)
        if (m[i].move && m[i].type == type)
            return TRUE;
    return FALSE;
}

static bool8 VoltageIs(u8 *marks, u8 at, u8 how, u8 want)   // how: 0 ==, 1 <=, 2 <, 3 >; want: the count asked (0 any)
{
    u8 i, n = 0;
    for (i = 0; i < 3; i++)
    {
        s8 v = C.act.voltage[i];
        bool8 ok = how == 0 ? v == at : how == 1 ? v <= at : how == 2 ? v < at : v > at;
        if (ok)
        {
            marks[i] = 1;
            n++;
        }
    }
    return want ? n == want : n > 0;
}

static bool8 Lowest(u8 who)
{
    u8 i;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        if (C.act.total[who] > C.act.total[i])
            return FALSE;
    return TRUE;
}

static bool8 AiCondition(u8 cond, u8 who, const struct AiMove *m, u8 *marks)
{
    u8 i;
    switch (cond)
    {
    case 1: return C.act.turn == CONTEST_TURNS;   // (never: Platinum's table asks it as written)
    case 2: return C.act.doubled[who] != CONTEST_EFFECT_NONE;
    case 3: return HasEffect(m, CONTEST_EFFECT_BASIC);
    case 4: return HasEffect(m, CONTEST_EFFECT_ALL_SAME_JUDGE);
    case 5: return HasEffect(m, CONTEST_EFFECT_STEAL_VOLTAGE);
    case 6: return HasEffect(m, CONTEST_EFFECT_DOUBLE_FINAL_ACT);
    case 7: return HasEffect(m, CONTEST_EFFECT_FIRST_PERFORMANCE_ADVANTAGE);
    case 8: return HasEffect(m, CONTEST_EFFECT_FINAL_PERFORMANCE_ADVANTAGE);
    case 9: return HasEffect(m, CONTEST_EFFECT_NO_VOLTAGE_DOWN);
    case 10: return HasEffect(m, CONTEST_EFFECT_TWO_VOLTAGE_IN_A_ROW_ADVANTAGE);
    case 11: return HasEffect(m, CONTEST_EFFECT_HIGH_SCORE_LATER_TURN);
    case 12: return Lowest(who);
    case 13: return C.act.turn == CONTEST_TURNS && Lowest(who);
    case 14:
        for (i = 0; i < MAX_MON_MOVES; i++)
            if (m[i].move && m[i].effect == CONTEST_EFFECT_2_HEARTS_WHEN_VOLTAGE_UP && m[i].type == C.type)
                return TRUE;
        return FALSE;
    case 15: return HasType(m, C.type);
    case 16: return HasType(m, C.type) && VoltageIs(marks, 40, 0, 0);
    case 17: return HasType(m, C.type) && VoltageIs(marks, 30, 0, 0);
    case 18: return HasEffect(m, CONTEST_EFFECT_VOLTAGE);
    case 19: return HasEffect(m, CONTEST_EFFECT_SUPPRESS_VOLTAGE);
    case 20: return VoltageIs(marks, 40, 0, 0);
    case 21: return VoltageIs(marks, 30, 0, 0);
    case 22: return VoltageIs(marks, 10, 1, 0);
    case 23:
        for (i = 0; i < 3; i++)
            if (C.act.voltage[i] < 20)
                return FALSE;
        marks[0] = marks[1] = marks[2] = 1;
        return TRUE;
    case 24:
        for (i = 0; i < 3; i++)
            if (C.act.voltage[i] > 20)
                return FALSE;
        marks[0] = marks[1] = marks[2] = 1;
        return TRUE;
    case 25: return VoltageIs(marks, 20, 1, 1);
    case 26: return VoltageIs(marks, 40, 0, 2);
    case 27: return VoltageIs(marks, 0, 0, 1);
    case 28: return HasType(m, C.type) && VoltageIs(marks, 40, 0, 0);
    }
    return FALSE;
}

void Contest_AiChoose(u8 who, u16 *move, u8 *judge)
{
    struct AiMove m[MAX_MON_MOVES];
    u16 rows = gSinnohContestAi[0], r, rnd[4], jr[3];
    u8 place = 0, i, k, best, bestJudge;
    const struct ContestEntrant *e = &C.e[who];
    memset(m, 0, sizeof(m));
    for (i = 0; i < MAX_MON_MOVES; i++)
        if (Contest_MoveUsable(who, e->moves[i]) && Contest_MoveType(e->moves[i]) <= 4)
        {
            m[i].move = e->moves[i];
            m[i].effect = Contest_MoveEffect(e->moves[i]);
            m[i].type = Contest_MoveType(e->moves[i]);
        }
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        if (C.act.order[i] == who)
            place = i;
    for (r = 0; r < rows; r++)
    {
        const u16 *row = &gSinnohContestAi[1 + r * 6];
        u8 marks[3] = {0, 0, 0}, target = row[1] & 0xFF;
        s8 rule = (s8)(row[1] >> 8);
        s16 w;
        if ((row[0] & 0xFF) != place + 1)
            continue;
        if (!AiCondition(row[0] >> 8, who, m, marks))
            continue;
        if (rule == 0)
            marks[0] = marks[1] = marks[2] = 0;
        else if (rule == 2 || rule == 3)
            for (k = 0; k < 3; k++)
                marks[k] ^= 1;
        w = (s16)row[2 + e->ai];
        if (e->ai)
            w += (s16)row[2];
        for (i = 0; i < MAX_MON_MOVES; i++)
        {
            bool8 hit;
            if (!m[i].move)
                continue;
            hit = target == 240 ? m[i].type == C.type
                : target == 241 ? Contest_EffectAppeal(m[i].effect) >= 20
                : m[i].effect == target;
            if (!hit)
                continue;
            m[i].score += w;
            for (k = 0; k < 3; k++)
                if (marks[k])
                    m[i].judge[k] += w;
        }
    }
    for (i = 0; i < 4; i++)
        rnd[i] = Contest_Random();
    for (i = 0; i < 3; i++)
        jr[i] = Contest_Random();
    for (best = 0; best < MAX_MON_MOVES && !m[best].move; best++)
        ;
    if (best == MAX_MON_MOVES)
    {
        // (no move it may perform: the first it knows, to the first judge)
        *move = e->moves[0] ? e->moves[0] : MOVE_TACKLE;
        *judge = 0;
        return;
    }
    for (i = best + 1; i < MAX_MON_MOVES; i++)
        if (m[i].move && (m[i].score > m[best].score || (m[i].score == m[best].score && rnd[i] > rnd[best])))
            best = i;
    // (sometimes, by rank, it stays away from the player's judge when nothing pulls it anywhere)
    if (!m[best].judge[0] && !m[best].judge[1] && !m[best].judge[2])
    {
        static const u8 sAvoid[4] = {230, 128, 51, 0};
        if ((Contest_Random() & 0xFF) < sAvoid[C.rank])
            m[best].judge[C.act.judgeOf[0] % 3] -= 100;
    }
    bestJudge = 0;
    for (k = 1; k < 3; k++)
        if (m[best].judge[k] > m[best].judge[bestJudge]
         || (m[best].judge[k] == m[best].judge[bestJudge] && jr[k] > jr[bestJudge]))
            bestJudge = k;
    *move = m[best].move;
    *judge = bestJudge;
}

// ----------------------------------------------------------------------------------------------------- results

// each round's points in its share of 100 (ov17_02251930 / ov17_02251A1C), the placing (ties by chance)
void Contest_ScoreAll(void)
{
    u8 r, i, k;
    s32 key[4], rnd[4];
    u8 id[4];
    for (r = 0; r < ROUND_COUNT; r++)
    {
        s32 raw[4], best = 0, unit;
        for (i = 0; i < CONTEST_ENTRANTS; i++)
        {
            raw[i] = r == ROUND_VISUAL ? C.e[i].visual : r == ROUND_DANCE ? C.e[i].dance : C.e[i].acting;
            if (raw[i] > best)
                best = raw[i];
        }
        unit = best > 0 ? RoundShare(r) / best : 0;
        for (i = 0; i < CONTEST_ENTRANTS; i++)
        {
            C.e[i].round[r] = Contest_HasRound(r) ? (raw[i] * unit + 50) / 100 : 0;
            C.e[i].bar[r] = 0;
        }
        best = 0;
        for (i = 0; i < CONTEST_ENTRANTS; i++)
            if (C.e[i].round[r] > best)
                best = C.e[i].round[r];
        for (i = 0; i < CONTEST_ENTRANTS; i++)
            C.e[i].bar[r] = best ? 100 * C.e[i].round[r] / best : 0;
    }
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        C.e[i].total = C.e[i].round[0] + C.e[i].round[1] + C.e[i].round[2];
        id[i] = i;
        key[i] = C.e[i].total;
        rnd[i] = Contest_Random();
    }
    for (i = 0; i < CONTEST_ENTRANTS - 1; i++)
        for (k = CONTEST_ENTRANTS - 1; k > i; k--)
            if (key[k - 1] < key[k] || (key[k - 1] == key[k] && rnd[k - 1] < rnd[k]))
            {
                s32 t = key[k];
                u8 u;
                key[k] = key[k - 1];
                key[k - 1] = t;
                t = rnd[k];
                rnd[k] = rnd[k - 1];
                rnd[k - 1] = t;
                u = id[k];
                id[k] = id[k - 1];
                id[k - 1] = u;
            }
    if (C.conceded)
    {
        // Moemon Co-op: the player conceded: after everyone else
        for (i = 0; i < CONTEST_ENTRANTS - 1; i++)
            if (id[i] == 0)
            {
                id[i] = id[i + 1];
                id[i + 1] = 0;
            }
    }
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        C.ranking[i] = id[i];
        C.e[id[i]].place = i;
    }
    C.scored = TRUE;
    MPNet_Log("sinnoh contest: results: %d %d %d %d (totals %d %d %d %d), the player %d", id[0], id[1], id[2], id[3],
              C.e[0].total, C.e[1].total, C.e[2].total, C.e[3].total, C.e[0].place + 1);
}

static u8 Winner(void)
{
    u8 i;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        if (C.e[i].place == 0)
            return i;
    return 0;
}

// ----------------------------------------------------------------------------------------------------- the end

// Platinum's Contest_CalcFirstTimeVictoryAccessoryReward: the accessory a first win of this rank and type gives
static u16 FirstWinAccessory(void)
{
    static const u8 sGift[5][4] = {
        {ACCESSORY_RED_BARRETTE, ACCESSORY_RED_BALLOONS, ACCESSORY_TOP_HAT, ACCESSORY_GOLD_PEDESTAL},
        {ACCESSORY_BLUE_BARRETTE, ACCESSORY_BLUE_BALLOONS, ACCESSORY_SILK_VEIL, ACCESSORY_GLASS_STAGE},
        {ACCESSORY_PINK_BARRETTE, ACCESSORY_PINK_BALLOON, ACCESSORY_LACE_HEADDRESS, ACCESSORY_FLOWER_STAGE},
        {ACCESSORY_GREEN_BARRETTE, ACCESSORY_GREEN_BALLOONS, ACCESSORY_PROFESSOR_HAT, ACCESSORY_CUBE_STAGE},
        {ACCESSORY_YELLOW_BARRETTE, ACCESSORY_YELLOW_BALLOONS, ACCESSORY_HEROIC_HEADBAND, ACCESSORY_AWARD_PODIUM},
    };
    u16 a;
    if (C.e[0].place > 0)
        return 0xFFFF;
    a = sGift[C.type][C.rank];
    return Sinnoh_CanFitAccessory(a, 1) ? a : 0xFFFF;
}

static void End(void)
{
    struct Pokemon *mon = &gPlayerParty[C.slot < PARTY_SIZE ? C.slot : 0];
    if (!C.made || !C.scored)
        return;
    if (C.mode != CONTEST_OFFICIAL && C.mode != CONTEST_UNK0 && C.mode != CONTEST_UNK1)
        return;   // (practice and the single rounds give nothing)
    IncrementGameStat(GAME_STAT_ENTERED_CONTEST);   // (the game's contest records: JUBILIFE TV's rankings read them)
    if (C.e[0].place == 0)
    {
        u8 level = GetMonData(mon, MON_DATA_COOL_RIBBON + C.type);
        IncrementGameStat(GAME_STAT_WON_CONTEST);
        if (level < C.rank + 1)
        {
            level = C.rank + 1;
            SetMonData(mon, MON_DATA_COOL_RIBBON + C.type, &level);
            IncrementGameStat(GAME_STAT_RECEIVED_RIBBONS);
        }
        AdjustFriendship(mon, FRIENDSHIP_EVENT_LEAGUE_BATTLE);   // (Platinum's FRIENDSHIP_EVENT_CONTEST_WIN: as much)
        Sinnoh_ContestKeepPhoto(C.type, &C.e[0]);
    }
    Sinnoh_TvContestResult(mon, C.type, C.rank, C.e[0].place + 1);
    MPNet_Log("sinnoh contest: ended, the player placed %d", C.e[0].place + 1);
}

// ----------------------------------------------------------------------------------------------------- the party

static u16 sPartyResult, sPartyVar, sPartyRank, sPartyType;
static u16 sStudioResult;
static u16 sPickKept, sPicked = 0xFF;   // (CC_PICK_*: Platinum's SelectMoveTutorPokemon on the game's party menu)   // (the studio's answer variable: Contest_StudioDone)

void Contest_StudioDone(bool8 taken)
{
    if (sStudioResult >= VARS_START)
        VarSet(sStudioResult, taken);
}

static bool8 CanEnter(struct Pokemon *mon, u8 rank, u8 type)
{
    if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
        return FALSE;
    return rank == 0 || GetMonData(mon, MON_DATA_COOL_RIBBON + (type % 5)) >= rank;
}

// ----------------------------------------------------------------------------------------------------- the commands

// Platinum's national POKéDEX count (pokedex.c Pokedex_NumCaught_National): 1-493 caught, its eleven mythicals left out
static u16 NationalCaught(void)
{
    static const u16 sLeftOut[] = {151, 249, 250, 251, 385, 386, 489, 490, 491, 492, 493};
    u16 n = 0, i, k;
    for (i = 1; i <= 493; i++)
    {
        for (k = 0; k < ARRAY_COUNT(sLeftOut) && sLeftOut[k] != i; k++)
            ;
        if (k == ARRAY_COUNT(sLeftOut) && GetSetPokedexFlag(i, FLAG_GET_CAUGHT))
            n++;
    }
    return n;
}

void Sinnoh_ContestCall(u16 call, u16 a, u16 b)
{
    struct ContestEntrant *e;
    switch (call)
    {
    case CC_NEW_RANK_TYPE:
        C.newRank = VarGet(a);
        C.newType = VarGet(b);
        break;
    case CC_NEW_MODE_SLOT:
        Contest_New(C.newRank, C.newType, VarGet(a), VarGet(b));
        break;
    case CC_END:
        End();
        C.made = FALSE;
        break;
    case CC_JUDGE_NAME:
        StringCopy(Contest_Slot(VarGet(b)), Contest_Text(CB_JUDGE_NAMES, Contest_JudgeName(VarGet(a))));
        break;
    case CC_TRAINER_NAME:
        StringCopy(Contest_Slot(VarGet(b)), C.e[EntryToId(VarGet(a))].trainer);
        break;
    case CC_MON_NAME:
        StringCopy(Contest_Slot(VarGet(b)), C.e[EntryToId(VarGet(a))].nick);
        break;
    case CC_NUMBER:
        ConvertIntToDecimalStringN(Contest_Slot(VarGet(b)), VarGet(a), STR_CONV_MODE_LEFT_ALIGN, 2);
        break;
    case CC_RANK_NAME:
    {
        char name[32];
        snprintf(name, sizeof(name), Contest_IsPractice() ? "Contest_Text_Practice"
                 : C.rank == 0 ? "Contest_Text_NormalRank" : C.rank == 1 ? "Contest_Text_GreatRank"
                 : C.rank == 2 ? "Contest_Text_UltraRank" : "Contest_Text_MasterRank");
        StringCopy(Contest_Slot(VarGet(a)), S3D_TextByName(name));
        break;
    }
    case CC_TYPE_NAME:
    {
        static const char *const sNames[5] = {"Contest_Text_CoolContest", "Contest_Text_BeautyContest",
                                              "Contest_Text_CuteContest", "Contest_Text_SmartContest",
                                              "Contest_Text_ToughContest"};
        StringCopy(Contest_Slot(VarGet(a)), S3D_TextByName(sNames[C.type]));
        break;
    }
    case CC_WINNER_TRAINER:
        StringCopy(Contest_Slot(VarGet(a)), C.e[Winner()].trainer);
        break;
    case CC_WINNER_MON:
        StringCopy(Contest_Slot(VarGet(a)), C.e[Winner()].nick);
        break;
    case CC_PLACEMENT:
        VarSet(a, C.e[0].place);
        break;
    case CC_WINNER_ENTRY:
        VarSet(a, CONTEST_ENTRANTS - 1 - Winner());
        break;
    case CC_PLAYER_ENTRY:
        VarSet(a, CONTEST_ENTRANTS - 1);
        break;
    case CC_OBJ_GFX:
        e = &C.e[EntryToId(VarGet(a))];
        VarSet(b, e->opponent == 0xFF ? (gSaveBlock2Ptr->playerGender ? OBJ_EVENT_GFX_MAY_NORMAL : OBJ_EVENT_GFX_BRENDAN_NORMAL) : e->gfx);
        break;
    case CC_FAME:
        VarSet(b, C.e[EntryToId(VarGet(a))].fame);
        break;
    case CC_MODE:
        VarSet(a, Contest_IsPractice() ? 2 : 0);   // (CONTEST_MODE_OFFICIAL / _PRACTICE; links come with co-op)
        break;
    case CC_INFO_RANK_TYPE:
        VarSet(a, C.rank);
        VarSet(b, C.type);
        break;
    case CC_INFO_MODE_SLOT:
        VarSet(a, C.mode);
        VarSet(b, C.slot);
        break;
    case CC_HAS_RIBBON:
        VarSet(a, GetMonData(&gPlayerParty[C.slot % PARTY_SIZE], MON_DATA_COOL_RIBBON + C.type) >= C.rank + 1);
        break;
    case CC_RIBBON_NAME:
    {
        static const char *const sType[5] = {"Cool", "Beauty", "Cute", "Smart", "Tough"};
        static const char *const sLevel[4] = {"", "Super", "Hyper", "Master"};
        char name[48];
        snprintf(name, sizeof(name), "RibbonName_Text_Hoenn%s%s", sType[C.type], sLevel[C.rank]);
        StringCopy(Contest_Slot(VarGet(a)), S3D_TextByName(name));
        break;
    }
    case CC_FIRST_ACCESSORY:
        VarSet(a, FirstWinAccessory());
        break;
    case CC_SKIP_AWARD:
        VarSet(a, Winner() != 0 || C.mode != CONTEST_OFFICIAL);
        break;
    case CC_PARTY_MENU:
        // (before the game's party menu, special ChoosePartyMon: it answers in VAR_0x8004, where the rank is kept)
        sPartyVar = a;
        sPartyRank = VarGet(a);
        sPartyType = VarGet(b);
        break;
    case CC_PARTY_PICKED:
        sPartyResult = gSpecialVar_0x8004 < PARTY_SIZE ? gSpecialVar_0x8004 : 0xFF;
        if (sPartyResult != 0xFF && !CanEnter(&gPlayerParty[sPartyResult], sPartyRank, sPartyType))
            sPartyResult = 0xFF;   // (Platinum's menu doesn't let one without the rank's ribbon be chosen)
        if (sPartyVar >= VARS_START)
            VarSet(sPartyVar, sPartyRank);
        break;
    case CC_PARTY_RESULT:
        VarSet(a, sPartyResult);
        VarSet(b, FALSE);   // (the summary: the game's party menu shows it itself)
        break;
    case CC_RUN:
        Contest_RunScenes();
        break;
    case CC_SHOW_PHOTO:
        VarSet(b, Contest_ShowPhoto(VarGet(a)));
        break;
    case CC_STUDIO:
        sStudioResult = b;
        VarSet(b, FALSE);
        Contest_RunStudio(VarGet(a));
        break;
    case CC_STUDIO_PHOTO:
        VarSet(b, VarGet(a) == 0 && Contest_ShowPhoto(PHOTO_STUDIO) ? 0 : 1);
        break;
    case CC_STUDIO_TITLE:
        *Sinnoh_PhotoTitle(PHOTO_STUDIO) = VarGet(a);
        break;
    case CC_PICK_BEGIN:
        sPickKept = gSpecialVar_0x8004;
        break;
    case CC_PICK_END:
        sPicked = gSpecialVar_0x8004 < PARTY_SIZE ? gSpecialVar_0x8004 : 0xFF;
        gSpecialVar_0x8004 = sPickKept;
        break;
    case CC_PICK_RESULT:
        VarSet(a, sPicked);
        break;
    case CC_FRIENDSHIP:
        VarSet(a, VarGet(b) < PARTY_SIZE ? GetMonData(&gPlayerParty[VarGet(b)], MON_DATA_FRIENDSHIP) : 0);
        break;
    case CC_DEX_CAUGHT:
        VarSet(a, NationalCaught());
        break;
    case CC_DEX_DONE:
        VarSet(a, NationalCaught() >= 493 - 11);
        break;
    case CC_PARTY_NON_EGGS:
    {
        u8 i, n = 0;
        for (i = 0; i < PARTY_SIZE; i++)
            n += GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG);
        VarSet(a, n);
        break;
    }
    case CC_BUFFER_WORD:
        CopyEasyChatWord(Contest_Slot(VarGet(a)), VarGet(b));
        break;
    }
}

bool8 Contest_IsMade(void)
{
    return C.made;
}
