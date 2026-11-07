// Moemon Kai: Shadow Pokémon (docs/SHADOW_POKEMON.md): Colosseum's table and rules (src/data/orre_shadow.h, from the
// game's own tables by tools/orre/shadow_data.py) and each Shadow Pokémon's state, kept in "<save>.orre" with
// Colosseum's story flags (src/orre/orre_story.c): saves never change layout. A Shadow Pokémon is known by its
// personality and Original Trainer ID (as the Garden's records are).
//
// The heart gauge is Colosseum's "dark points" (DP), kept in hundredths as the game keeps them (status 0xC5 = DP * 100):
//   a drop (pokemon_range_8011F5FC.c pokemonAddDpFormPokemonDpFilterId): DP += source * kake / waru by the nature's
//     rate for that source (scents times their strength), never below 0; ORRE_DP_EMPTY empties it
//   its level (pokemon_range_8011F77C.c pokemonGetDarkPokemonLevel): DP / the gauge's size >= 100% 0, >= 80% 1,
//     >= 60% 2, >= 40% 3, >= 20% 4, > 0 5, else 6 (open: ready to be purified)
//   steps (field_exact_801ED3B8.c): each Shadow Pokémon in the party counts its steps, and on the 257th the walking
//     drop is taken and the count starts again; the day care's the same, with the day care's drop
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "pokemon.h"
#include "orre.h"
#include "orre_shadow.h"
#include "battle.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/items.h"
#include "constants/battle_gen4.h"
#include "battle_controllers.h"
#include "battle_message.h"
#include "battle_scripts.h"
#include "battle_util.h"
#include "random.h"
#include "o3d.h"
#include "battle_anim.h"
#include "util.h"
#include "mp.h"
#include "aw_file.h"   // (sidecars written in the background: src/platform/aw_file.c)

void MPNet_Log(const char *fmt, ...);
extern char gPlatformSavePath[];
u8 *Orre_StoryFlagBytes(u32 *size);
void Orre_StoryReset(void);

#include "../data/orre_shadow.h"
#include "../data/orre_shadow_palettes.h"

#define ORRE_MAGIC 0x4552524F   // "ORRE"
#define ORRE_FORMAT 1
#define MAX_RECS 128
#define STEPS_PER_DROP 0x100

// a Shadow Pokémon's state (written as it is: the sidecar's records)
struct OrreShadowRec
{
    u32 personality, otId;
    u8 id;              // Colosseum's Shadow id
    u8 flags;           // REC_HYPER: in Hyper Mode
    u16 steps;          // toward the next step drop
    s32 dp;             // the heart gauge, hundredths of DP
    u32 storedExp;      // EXP earned while Shadow, given back when purified
    u16 gauge;          // the gauge's size (DP)
    u8 traits[6];       // (the A-Life's dark traits: docs/SHADOW_POKEMON.md "In the A-Life")
    u32 battle;         // the battle its battling drop was last taken in (not kept between runs: sBattle starts at 0)
};

#define REC_HYPER 1

static struct OrreShadowRec sRecs[MAX_RECS];
static u16 sRecCount;

// ---- Colosseum's table

u8 Orre_Shadow_Count(void)
{
    return ARRAY_COUNT(sOrreShadows);
}

const struct OrreShadowMon *Orre_Shadow_DataAt(u8 index)
{
    return index < ARRAY_COUNT(sOrreShadows) ? &sOrreShadows[index] : NULL;
}

const struct OrreShadowMon *Orre_Shadow_Data(u8 id)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sOrreShadows); i++)
        if (sOrreShadows[i].id == id)
            return &sOrreShadows[i];
    return NULL;
}

// ---- the records

static struct OrreShadowRec *Find(u32 personality, u32 otId)
{
    u32 i;
    for (i = 0; i < sRecCount; i++)
        if (sRecs[i].personality == personality && sRecs[i].otId == otId)
            return &sRecs[i];
    return NULL;
}

static struct OrreShadowRec *RecOfBox(struct BoxPokemon *box)
{
    if (box == NULL || GetBoxMonData(box, MON_DATA_SPECIES) == SPECIES_NONE)
        return NULL;
    return Find(GetBoxMonData(box, MON_DATA_PERSONALITY), GetBoxMonData(box, MON_DATA_OT_ID));
}

static struct OrreShadowRec *RecOf(struct Pokemon *mon)
{
    return mon ? RecOfBox(&mon->box) : NULL;
}

bool8 Orre_Shadow_IsKey(u32 otId, u32 personality)
{
    return sRecCount != 0 && Find(personality, otId) != NULL;
}

bool8 Orre_Shadow_Recolor(u16 *pal, u16 species, bool8 pokemonArt)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sOrreShadowPalettes); i++)
        if (sOrreShadowPalettes[i].species == species)
        {
            memcpy(pal + 1, (pokemonArt ? sOrreShadowPalettes[i].pokemon : sOrreShadowPalettes[i].moe) + 1, 15 * sizeof(u16));
            return TRUE;
        }
    return FALSE;
}

bool8 Orre_Shadow_IsBox(struct BoxPokemon *box)
{
    return RecOfBox(box) != NULL;
}

bool8 Orre_Shadow_Is(struct Pokemon *mon)
{
    return RecOf(mon) != NULL;
}

u8 Orre_Shadow_Id(struct Pokemon *mon)
{
    struct OrreShadowRec *r = RecOf(mon);
    return r ? r->id : 0;
}

bool8 Orre_Shadow_Make(struct Pokemon *mon, u8 id)
{
    const struct OrreShadowMon *data = Orre_Shadow_Data(id);
    struct OrreShadowRec *r;
    if (data == NULL || mon == NULL || GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
        return FALSE;
    if ((r = RecOf(mon)) == NULL)
    {
        if (sRecCount == MAX_RECS)
            return FALSE;
        r = &sRecs[sRecCount++];
    }
    memset(r, 0, sizeof(*r));
    r->personality = GetMonData(mon, MON_DATA_PERSONALITY);
    r->otId = GetMonData(mon, MON_DATA_OT_ID);
    r->id = id;
    r->gauge = data->gauge;
    r->dp = data->gauge * 100;   // (closed: the gauge full)
    MPNet_Log("shadow: Shadow %u (species %u) made, gauge %u", id, data->species, data->gauge);
    return TRUE;
}

void Orre_Shadow_Purified(struct Pokemon *mon)
{
    struct OrreShadowRec *r = RecOf(mon);
    if (r == NULL)
        return;
    MPNet_Log("shadow: Shadow %u purified", r->id);
    *r = sRecs[--sRecCount];
}

// ---- the heart gauge

static u8 LevelOf(const struct OrreShadowRec *r)
{
    float level;
    if (r == NULL)
        return ORRE_SHADOW_NONE;
    if (r->dp <= 0)
        return ORRE_SHADOW_OPEN;
    level = 100.0f * (r->dp / 100.0f) / (r->gauge ? r->gauge : 1);
    if (level >= 100.0f)
        return 0;
    if (level >= 80.0f)
        return 1;
    if (level >= 60.0f)
        return 2;
    if (level >= 40.0f)
        return 3;
    if (level >= 20.0f)
        return 4;
    return 5;
}

u8 Orre_Shadow_Level(struct Pokemon *mon)
{
    return LevelOf(RecOf(mon));
}

s32 Orre_Shadow_Dp(struct Pokemon *mon)
{
    struct OrreShadowRec *r = RecOf(mon);
    return r ? r->dp : -1;
}

u8 Orre_Shadow_Gauge(struct Pokemon *mon)
{
    struct OrreShadowRec *r = RecOf(mon);
    return r ? r->gauge : 0;
}

static void DropRec(struct OrreShadowRec *r, u8 nature, u8 source, u8 strength)
{
    float dp, now;
    const u8 *rate;
    if (r == NULL || source >= ARRAY_COUNT(sOrreDpSource) || nature >= ARRAY_COUNT(sOrreShadowNatures))
        return;
    if (source == ORRE_DP_EMPTY)
    {
        r->dp = 0;
        return;
    }
    dp = (float)sOrreDpSource[source];
    if (source == ORRE_DP_SCENT)
        dp *= (float)strength;
    rate = sOrreDpRate[sOrreShadowNatures[nature].rate[source] < ARRAY_COUNT(sOrreDpRate) ? sOrreShadowNatures[nature].rate[source] : 1];
    if (rate[1] == 0)
        return;
    dp = dp * (float)rate[0] / (float)rate[1];
    now = r->dp / 100.0f + dp;
    if (now < 0.0f)
        now = 0.0f;
    r->dp = (s32)(100.0f * now);
}

void Orre_Shadow_Drop(struct Pokemon *mon, u8 source, u8 strength)
{
    struct OrreShadowRec *r = RecOf(mon);
    if (r)
        DropRec(r, GetNature(mon), source, strength);
}

u8 Orre_Shadow_HyperChance(struct Pokemon *mon)
{
    u8 level = Orre_Shadow_Level(mon);
    return level <= ORRE_SHADOW_OPEN ? sOrreShadowNatures[GetNature(mon)].hyperMode[level] : 0;
}

u8 Orre_Shadow_CallChance(struct Pokemon *mon)
{
    u8 level = Orre_Shadow_Level(mon);
    return level <= ORRE_SHADOW_OPEN ? sOrreShadowNatures[GetNature(mon)].call[level] : 0;
}

// a step: field_exact_801ED3B8.c's counts (the party's walking, the day care's)
static void StepRec(struct OrreShadowRec *r, u8 nature, u8 source)
{
    if (r == NULL || r->dp <= 0)
        return;
    if (r->steps >= STEPS_PER_DROP)
    {
        DropRec(r, nature, source, 0);
        r->steps = 0;
    }
    else
        r->steps++;
}

void Orre_Shadow_Step(void)
{
    u32 i;
    if (sRecCount == 0)
        return;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG))
            StepRec(RecOf(mon), GetNature(mon), ORRE_DP_WALK);
    }
    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        struct BoxPokemon *box = &gSaveBlock1Ptr->daycare.mons[i].mon;
        struct OrreShadowRec *r = RecOfBox(box);
        if (r)
            StepRec(r, GetNatureFromPersonality(r->personality), ORRE_DP_DAYCARE);
    }
}

// ---- in battle

const u8 gText_Orre_HiddenMove[] = _("????");

static const u8 sRevealLevel[MAX_MON_MOVES] = {0xFF, 2, 4, 5};   // (slot 0: SHADOW RUSH while Shadow)
static u32 sBattle;   // the battle under way (a record's drop is taken once per battle number)

bool8 Orre_Shadow_SlotHidden(struct Pokemon *mon, u8 slot)
{
    struct OrreShadowRec *r = RecOf(mon);
    return r != NULL && slot > 0 && slot < MAX_MON_MOVES && LevelOf(r) < sRevealLevel[slot];
}

bool8 Orre_Shadow_SlotOverlaid(struct Pokemon *mon, u8 slot)
{
    return Orre_Shadow_Is(mon) && (slot == 0 || Orre_Shadow_SlotHidden(mon, slot));
}

void Orre_Shadow_BattleMoves(struct Pokemon *mon, u16 *moves, u8 *pp)
{
    u8 i;
    if (!Orre_Shadow_Is(mon))
        return;
    moves[0] = MOVE_SHADOW_RUSH;
    pp[0] = gBattleMoves[MOVE_SHADOW_RUSH].pp;
    for (i = 1; i < MAX_MON_MOVES; i++)
        if (Orre_Shadow_SlotHidden(mon, i))
            moves[i] = MOVE_NONE, pp[i] = 0;
}

void Orre_Shadow_BattleStart(void)
{
    u32 i;
    sBattle++;
    for (i = 0; i < PARTY_SIZE; i++)   // (fainted: out of HYPER MODE)
    {
        struct OrreShadowRec *r = RecOf(&gPlayerParty[i]);
        if (r && GetMonData(&gPlayerParty[i], MON_DATA_HP) == 0)
            r->flags &= ~REC_HYPER;
    }
}

void Orre_Shadow_SentOut(struct Pokemon *mon)
{
    struct OrreShadowRec *r = RecOf(mon);
    if (r == NULL || r->battle == sBattle)
        return;
    r->battle = sBattle;
    DropRec(r, GetNature(mon), ORRE_DP_BATTLE, 0);
    MPNet_Log("shadow: Shadow %u sent out, gauge %d", r->id, r->dp);
}

bool8 Orre_Shadow_TakeExp(struct Pokemon *mon, u32 exp)
{
    struct OrreShadowRec *r = RecOf(mon);
    if (r == NULL)
        return FALSE;
    if (LevelOf(r) >= 3)
        r->storedExp = r->storedExp + exp < r->storedExp ? 0xFFFFFFFF : r->storedExp + exp;
    MPNet_Log("shadow: Shadow %u EXP %u (stored %u)", r->id, exp, r->storedExp);
    return TRUE;
}

u32 Orre_Shadow_StoredExp(struct Pokemon *mon)
{
    struct OrreShadowRec *r = RecOf(mon);
    return r ? r->storedExp : 0;
}

// ---- HYPER MODE and CALL: Colosseum's words (fight_common/fight.msg 0x7704-0x771D), its rules (fight_range_8022A504.c,
// fight_range_exact_8022DCB8.c, fight_range_80211A00.c, fight_seq.c)

enum { T_HYPER_ENTER, T_SELFISH, T_ATK_FOE, T_ATK_YOU, T_ATK_SELF, T_CONFUSED, T_ITEM_NOHOW, T_ITEM_NONE,
       T_CALL, T_CALL_RESPONDED, T_CALL_SENSES, T_CALL_IGNORING, T_CALL_NOT_LISTENING, T_CALL_FAILED, T_HYPER_STILL,
       T_SENSES, T_COUNT };
static const u8 sT_HyperEnter[] = _("{B_ATK_NAME_WITH_PREFIX}'s emotions rose to\na fever pitch!\pIt entered HYPER MODE!");
static const u8 sT_Selfish[] = _("{B_ATK_NAME_WITH_PREFIX} selfishly used\n{B_BUFF1} without orders!");
static const u8 sT_AtkFoe[] = _("{B_ATK_NAME_WITH_PREFIX} ignored orders and\nattacked the opposing trainer!");
static const u8 sT_AtkYou[] = _("Unbelievable!\n{B_ATK_NAME_WITH_PREFIX} ignored orders and\lattacked {B_PLAYER_NAME}!");
static const u8 sT_AtkSelf[] = _("{B_ATK_NAME_WITH_PREFIX} is in HYPER MODE!\nIt attacked itself!");
static const u8 sT_Confused[] = _("{B_ATK_NAME_WITH_PREFIX} is in HYPER MODE!\nIt's too confused to do anything!");
static const u8 sT_ItemNoHow[] = _("{B_ATK_NAME_WITH_PREFIX} tried to use its item\nwithout permission!\pBut it didn't know how!");
static const u8 sT_ItemNone[] = _("{B_ATK_NAME_WITH_PREFIX} tried to use its item\nwithout permission!\pBut it's not holding anything!");
static const u8 sT_Call[] = _("{B_PLAYER_NAME} called {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sT_CallResponded[] = _("{B_ATK_NAME_WITH_PREFIX} eagerly responded to\nthe call!");
static const u8 sT_CallSenses[] = _("{B_ATK_NAME_WITH_PREFIX} came to its senses\nfrom the trainer's call!");
static const u8 sT_CallIgnoring[] = _("It's no good!\nIt's ignoring orders!");
static const u8 sT_CallNotListening[] = _("It's no good!\nIt's not listening!");
static const u8 sT_CallFailed[] = _("{B_ATK_NAME_WITH_PREFIX} failed to hear!");
static const u8 sT_HyperStill[] = _("{B_ATK_NAME_WITH_PREFIX} is in HYPER MODE!");          // (0x771C)
static const u8 sT_Senses[] = _("{B_ATK_NAME_WITH_PREFIX} came to its senses.");            // (0x771E)
static const u8 *const sTexts[T_COUNT] = {sT_HyperEnter, sT_Selfish, sT_AtkFoe, sT_AtkYou, sT_AtkSelf, sT_Confused,
    sT_ItemNoHow, sT_ItemNone, sT_Call, sT_CallResponded, sT_CallSenses, sT_CallIgnoring, sT_CallNotListening, sT_CallFailed,
    sT_HyperStill, sT_Senses};
static u8 sLine[2];

const u8 gText_Orre_BattleMenuCall[] = _("FIGHT{CLEAR_TO 56}BAG\nMOéMON{CLEAR_TO 56}CALL");

const u8 *Orre_Shadow_BattleText(u8 line)
{
    return sTexts[sLine[line & 1] < T_COUNT ? sLine[line & 1] : 0];
}

// a battler's Shadow record: the player's own POKéMON in a battle of their own (co-op: the master doesn't know a
// partner's Shadow Pokémon yet)
static struct OrreShadowRec *BattlerRec(u8 battler)
{
    if (battler >= MAX_BATTLERS_COUNT || GetBattlerSide(battler) != B_SIDE_PLAYER || gMpCoopBattle)
        return NULL;
    return RecOf(&gPlayerParty[gBattlerPartyIndexes[battler]]);
}

bool8 Orre_Shadow_InHyper(u8 battler)
{
    struct OrreShadowRec *r = BattlerRec(battler);
    return r != NULL && (r->flags & REC_HYPER) && gBattleMons[battler].hp != 0;
}

u8 Orre_Shadow_BattleCommand(u8 mode)
{
    u8 atk = gBattlerAttacker;
    struct OrreShadowRec *r;
    switch (mode)
    {
    case ORRE_HYPER_ROLL:   // (fight_range_exact_8022DCB8.c: not asleep, not already in it, the nature's chance)
        r = BattlerRec(atk);
        if (r == NULL || gCurrentMove != MOVE_SHADOW_RUSH || gBattleMons[atk].hp == 0 || (r->flags & REC_HYPER)
         || (gBattleMons[atk].status1 & STATUS1_SLEEP))
            return 0;
        if (Random() % 100 >= Orre_Shadow_HyperChance(&gPlayerParty[gBattlerPartyIndexes[atk]]))
            return 0;
        r->flags |= REC_HYPER;
        sLine[0] = T_HYPER_ENTER;
        Orre_Aura_Burst(atk, ORRE_AURA_HYPER);
        MPNet_Log("shadow: Shadow %u entered HYPER MODE", r->id);
        return 1;
    }
    return 0;
}

bool8 Orre_Shadow_HyperCrit(void)
{
    return gCurrentMove == MOVE_SHADOW_RUSH && Orre_Shadow_InHyper(gBattlerAttacker) && Random() % 100 < 90;
}

u8 Orre_Shadow_HyperDisobedience(void)
{
    u8 atk = gBattlerAttacker, k, roll, sum = 0, way = 0xFF;
    struct OrreShadowRec *r = BattlerRec(atk);
    struct Pokemon *mon;
    const u8 *chance;
    if (r == NULL)
        return 0xFF;
    if (!(r->flags & REC_HYPER) || gBattleMons[atk].hp == 0 || gCurrentMove == MOVE_SHADOW_RUSH)
        return DISOBEDIENCE_OBEDIENT;   // (a Shadow Pokémon out of HYPER MODE obeys; SHADOW RUSH always)
    mon = &gPlayerParty[gBattlerPartyIndexes[atk]];
    chance = sOrreShadowNatures[GetNature(mon)].disobey;
    roll = Random() % 100;
    for (k = 0; k < 6 && way == 0xFF; k++)
        if (roll < (sum += chance[k]))
            way = k;
    if (way == 0xFF)
        return DISOBEDIENCE_OBEDIENT;
    MPNet_Log("shadow: Shadow %u in HYPER MODE ignores orders (way %u)", r->id, way);
    Orre_Aura_Burst(atk, ORRE_AURA_HYPER);
    if (way == 0)
    {
        // a usable move of its own choosing (not one it can't use now, not a hidden one), else too confused to act
        u8 pick[MAX_MON_MOVES], n = 0, limits = CheckMoveLimitations(atk, 0, MOVE_LIMITATIONS_ALL);
        for (k = 0; k < MAX_MON_MOVES; k++)
            if (gBattleMons[atk].moves[k] != MOVE_NONE && !(limits & gBitTable[k]))
                pick[n++] = k;
        if (n)
        {
            gCurrMovePos = gChosenMovePos = pick[Random() % n];
            gCalledMove = gBattleMons[atk].moves[gCurrMovePos];
            PREPARE_MOVE_BUFFER(gBattleTextBuff1, gCalledMove);
            sLine[0] = T_SELFISH;
            gBattlescriptCurrInstr = BattleScript_OrreHyperOwnMove;
            gBattlerTarget = GetMoveTarget(gCalledMove, NO_TARGET_OVERRIDE);
            gHitMarker |= HITMARKER_DISOBEDIENT_MOVE;
            return DISOBEDIENCE_OTHER;
        }
        way = 4;
    }
    if (way == 3)
    {
        // a blow to itself (fightSeqCalcDamage at power 40, as confusion's)
        gBattleMoveDamage = CalculateBaseDamage(&gBattleMons[atk], &gBattleMons[atk], MOVE_POUND, 0, 40, 0, atk, atk);
        gBattlerTarget = atk;
        sLine[0] = T_ATK_SELF;
        gBattlescriptCurrInstr = BattleScript_OrreHyperHitsItself;
        gHitMarker |= HITMARKER_UNABLE_TO_USE_MOVE;
        return DISOBEDIENCE_OTHER;
    }
    sLine[0] = way == 1 ? T_ATK_FOE : way == 2 ? T_ATK_YOU : way == 4 ? T_CONFUSED
             : gBattleMons[atk].item == ITEM_NONE ? T_ITEM_NONE : T_ITEM_NOHOW;
    gBattlescriptCurrInstr = BattleScript_OrreHyperIgnored;
    return DISOBEDIENCE_IGNORED;
}

// fight_range_exact_8022DCB8.c _fightCheckDarkPokemonAppear: each turn in HYPER MODE it may come to its senses on its
// own (1 in 256: message 0x771E), else at the turn's end its aura flares and "...is in HYPER MODE!" (0x771C)
const u8 *Orre_Shadow_EndTurn(u8 battler)
{
    struct OrreShadowRec *r = BattlerRec(battler);
    if (r == NULL || !(r->flags & REC_HYPER) || gBattleMons[battler].hp == 0)
        return NULL;
    gBattlerAttacker = battler;
    if (Random() % 256 == 0)
    {
        r->flags &= ~REC_HYPER;
        sLine[0] = T_SENSES;
        MPNet_Log("shadow: Shadow %u came to its senses", r->id);
    }
    else
    {
        sLine[0] = T_HYPER_STILL;
        Orre_Aura_Burst(battler, ORRE_AURA_PULSE);
        MPNet_Log("shadow: Shadow %u is in HYPER MODE (end of turn)", r->id);
    }
    return BattleScript_OrreTurnMsg;
}

bool8 Orre_Shadow_CallAvailable(void)
{
    return O3D_IsOrreGroup(gSaveBlock1Ptr->location.mapGroup) && (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
        && !(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK)) && !gMpCoopBattle;
}

static void Wake(u8 battler)
{
    gBattleMons[battler].status1 &= ~STATUS1_SLEEP;
    gBattleMons[battler].status2 &= ~STATUS2_NIGHTMARE;
    gActiveBattler = battler;
    BtlController_EmitSetMonData(B_COMM_TO_CONTROLLER, REQUEST_STATUS_BATTLE, 0, sizeof(gBattleMons[battler].status1),
                                 &gBattleMons[battler].status1);
    MarkBattlerForControllerExec(battler);
}

void Orre_Shadow_Call(u8 battler)
{
    struct OrreShadowRec *r = BattlerRec(battler);
    bool8 asleep = (gBattleMons[battler].status1 & STATUS1_SLEEP) != 0;
    sLine[0] = T_CALL;
    if (r != NULL && (r->flags & REC_HYPER))
    {
        u8 chance = Orre_Shadow_CallChance(&gPlayerParty[gBattlerPartyIndexes[battler]]);
        if (Random() % 100 < chance)
        {
            // came to its senses: HYPER MODE over, CALL's drop of the heart gauge, and awake
            sLine[1] = T_CALL_SENSES;
            r->flags &= ~REC_HYPER;
            DropRec(r, GetNature(&gPlayerParty[gBattlerPartyIndexes[battler]]), ORRE_DP_CALL, 0);
            if (asleep)
                Wake(battler);
        }
        else
        {
            sLine[1] = T_CALL_IGNORING;
            if (asleep && Random() % 100 < chance / 2)
                Wake(battler);
        }
    }
    else if (asleep)
    {
        sLine[1] = T_CALL_RESPONDED;
        Wake(battler);
    }
    else
        sLine[1] = r != NULL ? T_CALL_FAILED : T_CALL_NOT_LISTENING;
    MPNet_Log("shadow: CALL to battler %u: outcome %u", battler, sLine[1]);
}

// ---- "<save>.orre": {magic, format, story flag bytes, records}

static void Path(char *out, size_t size, int backup)
{
    if (backup)
        snprintf(out, size, "%s.bak%d.orre", gPlatformSavePath, backup);
    else
        snprintf(out, size, "%s.orre", gPlatformSavePath);
}

void Orre_Shadow_Reset(void)
{
    sRecCount = 0;
    memset(sRecs, 0, sizeof(sRecs));
    Orre_StoryReset();
}

static bool8 LoadFrom(const char *path)
{
    FILE *f = fopen(path, "rb");
    u32 head[4], size;
    u8 *flags = Orre_StoryFlagBytes(&size);
    bool8 ok;
    if (f == NULL)
        return FALSE;
    ok = fread(head, sizeof(head), 1, f) == 1 && head[0] == ORRE_MAGIC && head[1] == ORRE_FORMAT
      && head[2] == size && head[3] <= MAX_RECS;
    if (ok)
        ok = fread(flags, size, 1, f) == 1 && (head[3] == 0 || fread(sRecs, sizeof(sRecs[0]), head[3], f) == head[3]);
    fclose(f);
    if (!ok)
    {
        Orre_Shadow_Reset();
        return FALSE;
    }
    sRecCount = head[3];
    {
        u32 i;
        for (i = 0; i < sRecCount; i++)
            sRecs[i].battle = 0xFFFFFFFF;   // (a battle number of the run that wrote it)
    }
    return TRUE;
}

void Orre_Shadow_LoadFromSave(void)
{
    char path[600];
    int i;
    Orre_Shadow_Reset();
    for (i = 0; i <= 3; i++)
    {
        Path(path, sizeof(path), i);
        if (LoadFrom(path))
        {
            MPNet_Log("orre: %u Shadow Pokemon%s", sRecCount, i ? " (from a backup)" : "");
            return;
        }
    }
}

void Orre_Shadow_WriteSidecar(void)
{
    char path[600];
    AwFile *f;
    u32 head[4], size;
    u8 *flags = Orre_StoryFlagBytes(&size);
    if (gSaveBlock2Ptr == NULL || MOEMON_DEV_BUILD)   // (a dev build never writes a save or its side files)
        return;
    Path(path, sizeof(path), 0);
    if ((f = AwFile_Open(path)) == NULL)
        return;
    head[0] = ORRE_MAGIC;
    head[1] = ORRE_FORMAT;
    head[2] = size;
    head[3] = sRecCount;
    AwFile_Write(head, sizeof(head), 1, f);
    AwFile_Write(flags, size, 1, f);
    if (sRecCount)
        AwFile_Write(sRecs, sizeof(sRecs[0]), sRecCount, f);
    AwFile_Close(f);
}

// ---- tests: `mp shadow ...`

void Orre_Shadow_Debug(const char *arg)
{
    int slot = 0, a = 0, b = 0;
    char cmd[16] = "";
    struct Pokemon *mon;
    sscanf(arg, "%15s %d %d %d", cmd, &slot, &a, &b);
    mon = slot >= 0 && slot < PARTY_SIZE ? &gPlayerParty[slot] : NULL;
    if (strcmp(cmd, "make") == 0)   // make SLOT ID: the party's SLOT made Colosseum's Shadow ID
        printf("[test] shadow make %d %d: %d\n", slot, a, Orre_Shadow_Make(mon, a));
    else if (strcmp(cmd, "drop") == 0)   // drop SLOT SOURCE [STRENGTH]
        Orre_Shadow_Drop(mon, a, b);
    else if (strcmp(cmd, "steps") == 0)   // steps N: N steps taken
        for (a = 0; a < slot; a++)
            Orre_Shadow_Step();
    else if (strcmp(cmd, "purify") == 0)
        Orre_Shadow_Purified(mon);
    else if (strcmp(cmd, "hyper") == 0)   // hyper SLOT 0|1: HYPER MODE off / on
    {
        struct OrreShadowRec *r = RecOf(mon);
        if (r)
            r->flags = a ? (r->flags | REC_HYPER) : (r->flags & ~REC_HYPER);
        printf("[test] shadow hyper %d: %d\n", slot, r ? (r->flags & REC_HYPER) != 0 : -1);
        return;
    }
    else if (strcmp(cmd, "reload") == 0)   // reload: "<save>.orre" read back (after a save)
    {
        void AwFile_Flush(void);
        AwFile_Flush();
        Orre_Shadow_LoadFromSave();
    }
    else if (strcmp(cmd, "stepcount") == 0)   // stepcount SLOT: the steps toward its next walking drop
    {
        struct OrreShadowRec *r = RecOf(mon);
        printf("[test] shadow steps %d: %d\n", slot, r ? r->steps : -1);
        return;
    }
    else if (strcmp(cmd, "table") == 0)
    {
        u32 i;
        for (i = 0; i < ARRAY_COUNT(sOrreShadows); i++)
            printf("[test] shadow table %u species %u level %u gauge %u catch %u\n", sOrreShadows[i].id,
                   sOrreShadows[i].species, sOrreShadows[i].level, sOrreShadows[i].gauge, sOrreShadows[i].catchRate);
        return;
    }
    if (strcmp(cmd, "table") != 0 && mon)   // and every command: the slot's state
        printf("[test] shadow %d: id %u dp %d gauge %u level %u hyper %u call %u nature %u moves %u/%u %u/%u %u/%u %u/%u"
               " exp %u stored %u\n", slot,
               Orre_Shadow_Id(mon), Orre_Shadow_Dp(mon), Orre_Shadow_Gauge(mon), Orre_Shadow_Level(mon),
               Orre_Shadow_HyperChance(mon), Orre_Shadow_CallChance(mon), GetNature(mon),
               GetMonData(mon, MON_DATA_MOVE1), GetMonData(mon, MON_DATA_PP1), GetMonData(mon, MON_DATA_MOVE2),
               GetMonData(mon, MON_DATA_PP2), GetMonData(mon, MON_DATA_MOVE3), GetMonData(mon, MON_DATA_PP3),
               GetMonData(mon, MON_DATA_MOVE4), GetMonData(mon, MON_DATA_PP4), GetMonData(mon, MON_DATA_EXP),
               Orre_Shadow_StoredExp(mon));
}
