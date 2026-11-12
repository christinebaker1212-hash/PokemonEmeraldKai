// Moemon Kai: UNOVA's BATTLE SUBWAY as Black's (ov021 bsubway_scr.c, ov010 fld_btl_inst_tool.c, arm9 gds_profile.c):
// the GEAR STATION's lines run Black's own scripts in UNOVA's VM (u3d_vm.c's BSUBWAY_SCR_CmdTool comes here); its
// trainers, their sets and the tables are Black's, from UNOVA's scripts pack (tools/unova/build_scripts.py version 10;
// nothing of them is in this repository). What the save keeps (Black's play and score data) lives in UNOVA's VM state
// (u3d_subway.h). The battles are UNOVA's own (u3d_battle.c), at level 50 with copies of the chosen POKéMON; the
// BATTLE POINTS are the game's own. Closed as SINNOH's tower closes them: the wireless multi, Wi-Fi, Battle Videos,
// the BATTLE BOX (the party is always the player's own).
#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "load_save.h"
#include "pokemon.h"
#include "random.h"
#include "string_util.h"
#include "constants/abilities.h"
#include "constants/battle_frontier.h"
#include "party_menu.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "u3d.h"
#include "u3d_subway.h"

void MPNet_Log(const char *fmt, ...);
const u8 *U3D_Vm_SubwaySet(u16 id);
const u8 *U3D_Vm_SubwayTrainer(u16 member, u32 *size);
const u8 *U3D_Vm_SubwayTables(u32 *size);
u16 U3D_Vm_Item(u16 black);
bool8 U3D_SetSubwayTrainer(u8 which, u16 trainerNo, u8 blackClass, const struct Pokemon *team, u8 count);
void U3D_PrepareSubwayBattle(u8 playMode, u8 members);

#define BANK_SUBWAY_NAMES 11      // (system bank 11: the subway trainers' names, by number)
#define BANK_SUBWAY_WORDS 184     // (system bank 184: three lines a trainer, the first its greeting)
#define SUBWAY_LEVEL 50

static struct U3DSubwaySave *sSave;
static struct U3DSubwaySave sDummy;

struct SubwayTrainer { u16 no, type; u8 count; u16 nationals[4], items[4]; struct Pokemon mons[4]; };   // (items: the
                                                                                           // game's numbers)
static struct
{
    bool8 live;
    u8 playMode, memberNum;
    u8 memberIndex[4];
    u16 memberMonsNo[4], memberItemNo[4];
    u16 trainerNo[14];
    u16 winCount;
    u8 fixedBp, partnerNo, flag8, flag12, commMode, winMax;
    struct SubwayTrainer trainers[2], partners[3];
    struct U3DSubwayDraw partnerDraws[3];
    u8 partnerResult[3];
    bool8 battleWon;
} sW;

// ---- Black's tables (the pack's version 10 block, after its sets and trainers)
struct Tables
{
    u16 ranges[13][2];   // (D0 x2, D8 x3, E4 x4, F4 x4)
    u8 bp[7][10], pad[2];
    u16 typeObj[0x35][2];
    u16 items[4];
    u32 thresholds[22];
    u16 retZone[9];
    s16 retX[9], retZ[9];
    u16 pad2;
    u32 spawns;
};
static const struct Tables *T(void)
{
    u32 size;
    const u8 *t = U3D_Vm_SubwayTables(&size);
    return t != NULL && size >= sizeof(struct Tables) ? (const struct Tables *)t : NULL;
}
enum { RANGE_D0 = 0, RANGE_D8 = 2, RANGE_E4 = 5, RANGE_F4 = 9 };

void U3D_Subway_Bind(struct U3DSubwaySave *save)
{
    sSave = save != NULL ? save : &sDummy;
    if (sSave->score.level == 0)
        sSave->score.level = 1;   // (BSUBWAY_SCOREDATA_Init: the Wi-Fi rank starts at 1)
}

static struct U3DSubwaySave *S(void)
{
    if (sSave == NULL)
        U3D_Subway_Bind(NULL);
    return sSave;
}

u8 U3D_Subway_PlayMode(void)
{
    return sW.playMode;
}

static u8 MemberNum(u16 mode);
u8 U3D_Subway_MemberNum(void)
{
    return MemberNum(sW.playMode);
}

static u8 MemberNum(u16 mode)
{
    switch (mode)
    {
    case 0: case 4: case 5: return 3;
    case 1: case 6:         return 4;
    case 2: case 3: case 7: case 8: return 2;
    }
    return 0;
}

static bool8 IsMulti(u8 mode)
{
    return mode == 2 || mode == 3 || mode == 7 || mode == 8;
}

static u16 Rand16(void)
{
    return Random();
}

// ---- score data (gds_profile.c)
static bool8 HasCleared(u8 mode) { return mode < U3D_SUBWAY_MODES && S()->score.clearCount[mode] != 0; }
static u16 RepeatClearCount(u8 mode)
{
    s16 n = mode < U3D_SUBWAY_MODES ? S()->score.clearCount[mode] - 1 : 0;
    return n < 0 ? 0 : n;
}

static u32 AccessFlag(s32 i, u32 op)
{
    if (i >= 11)
        return 0;
    switch (op)
    {
    case 2: S()->score.flags &= ~(1u << i); break;
    case 1: S()->score.flags |= 1u << i; break;
    case 0: return (S()->score.flags >> i) & 1;
    }
    return 0;
}

static u8 UpdateStreak(u32 kind)
{
    if (kind == 2)
        AccessFlag(0, 2), S()->score.streak = 0;
    else if (kind == 3)
    {
        if (AccessFlag(0, 0))
        {
            if (S()->score.streak < 0xFF)
                S()->score.streak++;
        }
        else
            AccessFlag(0, 1), S()->score.streak = 1;
    }
    return S()->score.streak;
}

static void AddBattlePoints(u32 n)
{
    u32 bp = gSaveBlock2Ptr->frontier.battlePoints + n;
    gSaveBlock2Ptr->frontier.battlePoints = bp > MAX_BATTLE_FRONTIER_POINTS ? MAX_BATTLE_FRONTIER_POINTS : bp;
}

// ---- the trainers (fld_btl_inst_tool.c)
static u8 IvForRank(u32 rank)
{
    if (rank < 50)  return 3;
    if (rank < 70)  return 6;
    if (rank < 90)  return 9;
    if (rank < 110) return 12;
    if (rank < 160) return 15;
    if (rank < 180) return 18;
    if (rank < 200) return 21;
    return 31;
}

static u8 PartnerIv(u32 trainerNo)   // (BSUBWAY_SCR_GetMonIv)
{
    if (trainerNo < 100) return 3;
    if (trainerNo < 120) return 6;
    if (trainerNo < 140) return 9;
    if (trainerNo < 160) return 12;
    if (trainerNo < 180) return 15;
    if (trainerNo < 200) return 18;
    if (trainerNo < 220) return 21;
    return 31;
}

static u16 Le16(const u8 *p) { return p[0] | p[1] << 8; }

struct Set { u16 species, moves[4], item, form; u8 evMask, nature; };
static bool8 ReadSet(u16 id, struct Set *s)
{
    const u8 *p = U3D_Vm_SubwaySet(id);
    u32 i;
    if (p == NULL)
        return FALSE;
    s->species = Le16(p) & 0x7FF;
    for (i = 0; i < 4; i++)
        s->moves[i] = Le16(p + 2 + 2 * i);
    s->evMask = p[10];
    s->nature = p[11];
    s->item = Le16(p + 12);
    s->form = Le16(p + 14) & 0x1F;
    return TRUE;
}

// FLD_BTL_INST_TOOL_BuildBattleMon + BattleMonToPokemon: a level 50 POKéMON of set ID, IVs all IV, the EVs shared
// between its marked stats, its nature (a personality made to fit it, never shiny), Ability by the personality's
// lowest bit, friendship full (none with FRUSTRATION); returns the personality
static const u8 sEvField[6] = {MON_DATA_HP_EV, MON_DATA_ATK_EV, MON_DATA_DEF_EV, MON_DATA_SPEED_EV, MON_DATA_SPATK_EV,
                               MON_DATA_SPDEF_EV};
static u32 BuildMon(struct Pokemon *mon, u16 setId, u32 otId, u32 pid, u8 iv, u8 itemIdx, bool8 tableItem)
{
    struct Set s;
    u16 species, item;
    u8 friendship = MAX_FRIENDSHIP, n = 0, ev, abilityNum;
    u32 i;
    ZeroMonData(mon);
    if (!ReadSet(setId, &s) || (species = NationalPokedexNumToSpecies(s.species)) == SPECIES_NONE)
        return 0;
    if (pid == 0)
    {
        do
            pid = Random32();
        while (pid % NUM_NATURES != s.nature % NUM_NATURES
               || ((otId >> 16) ^ (otId & 0xFFFF) ^ (pid >> 16) ^ (pid & 0xFFFF)) < SHINY_ODDS);
    }
    CreateMon(mon, species, SUBWAY_LEVEL, iv, TRUE, pid, OT_ID_PRESET, otId);
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        SetMonMoveSlot(mon, s.moves[i], i);   // (Black's move numbers are the national ones, as the game's)
        if (s.moves[i] == MOVE_FRUSTRATION)
            friendship = 0;
    }
    item = U3D_Vm_Item(tableItem ? T()->items[itemIdx & 3] : s.item);
    SetMonData(mon, MON_DATA_HELD_ITEM, &item);
    SetMonData(mon, MON_DATA_FRIENDSHIP, &friendship);
    for (i = 0; i < 6; i++)
        n += (s.evMask >> i) & 1;
    ev = n ? (510 / n > 255 ? 255 : 510 / n) : 0;
    for (i = 0; i < 6; i++)
        if (s.evMask & (1 << i))
            SetMonData(mon, sEvField[i], &ev);
    abilityNum = (pid & 1) && GetAbilityBySpecies(species, 1) != ABILITY_NONE;
    SetMonData(mon, MON_DATA_ABILITY_NUM, &abilityNum);
    CalculateMonStats(mon);
    return pid;
}

// a trainer's list (a/2/1/5, member NO + 1): u16 trainer type, count, the sets
static const u8 *Group(u16 no, u16 *type, u16 *count)
{
    u32 size;
    const u8 *g = U3D_Vm_SubwayTrainer(no + 1, &size);
    if (g == NULL || size < 4)
        return NULL;
    *type = Le16(g);
    *count = Le16(g + 2);
    if (4 + 2 * *count > size)
        *count = (size - 4) / 2;
    return g + 4;
}

// FLD_BTL_INST_TOOL_DrawTrainerParty: COUNT sets drawn from the trainer's list, no species twice nor one of EXCL's,
// no item twice nor one of EXCLITEMS's for the first 50 tries (then the table's items instead); returns TRUE then
static bool8 CreateTrainer(struct SubwayTrainer *t, u16 no, u8 count, const u16 *excl, const u16 *exclItems,
                           struct U3DSubwayDraw *draw)
{
    u16 type, listCount, ids[4];
    const u8 *list = Group(no, &type, &listCount);
    struct Set a, b;
    u32 otId, pids[4];
    u8 drawn = 0, natures[4];
    s32 itemTries = 0, guard = 0, i;
    bool8 dropped;
    memset(t, 0, sizeof(*t));
    t->no = no;
    if (list == NULL || listCount == 0)
        return FALSE;
    t->type = type;
    while (drawn < count && guard++ < 5000)
    {
        u16 id = Le16(list + 2 * (Rand16() % listCount));
        if (!ReadSet(id, &a))
            continue;
        for (i = 0; i < drawn; i++)
            if (ReadSet(ids[i], &b) && b.species == a.species)
                break;
        if (i != drawn)
            continue;
        if (excl != NULL)
        {
            for (i = 0; i < count; i++)
                if (a.species == excl[i])
                    break;
            if (i != count)
                continue;
        }
        if (itemTries < 50)
        {
            for (i = 0; i < drawn; i++)
                if (ReadSet(ids[i], &b) && b.item != 0 && b.item == a.item)
                    break;
            if (i != drawn)
            {
                itemTries++;
                continue;
            }
            if (exclItems != NULL)
            {
                for (i = 0; i < count; i++)
                    if (U3D_Vm_Item(a.item) == exclItems[i] && exclItems[i] != ITEM_NONE)
                        break;
                if (i != count)
                {
                    itemTries++;
                    continue;
                }
            }
        }
        ids[drawn] = id;
        natures[drawn] = a.nature;
        drawn++;
    }
    otId = Rand16() | (u32)Rand16() << 16;
    dropped = itemTries >= 50;
    for (i = 0; i < drawn; i++)
    {
        pids[i] = BuildMon(&t->mons[i], ids[i], otId, 0, IvForRank(no), i, dropped);
        t->nationals[i] = SpeciesToNationalPokedexNum(GetMonData(&t->mons[i], MON_DATA_SPECIES));
        t->items[i] = GetMonData(&t->mons[i], MON_DATA_HELD_ITEM);
    }
    t->count = drawn;
    if (draw != NULL)
    {
        draw->otId = otId;
        for (i = 0; i < 2; i++)
            draw->ids[i] = ids[i], draw->pids[i] = pids[i], draw->natures[i] = natures[i];
    }
    return dropped;
}

// BSUBWAY_SCR_CreatePartnerMons: a partner's two sets again from what was drawn (a REST between)
static void CreatePartnerMons(struct SubwayTrainer *t, u16 no, bool8 useItem, const struct U3DSubwayDraw *draw)
{
    u16 type, count;
    u32 i;
    memset(t, 0, sizeof(*t));
    t->no = no;
    if (Group(no, &type, &count) != NULL)
        t->type = type;
    for (i = 0; i < 2; i++)
    {
        BuildMon(&t->mons[i], draw->ids[i], draw->otId, draw->pids[i], PartnerIv(no), i, useItem);
        t->nationals[i] = SpeciesToNationalPokedexNum(GetMonData(&t->mons[i], MON_DATA_SPECIES));
        t->items[i] = GetMonData(&t->mons[i], MON_DATA_HELD_ITEM);
    }
    t->count = 2;
}

// BSUBWAY_SCR_DrawTrainerNo: a trainer's number for battle STAGE (0-6) of train ROUND on LINE
static u16 RandIn(u32 range)
{
    const struct Tables *t = T();
    u16 lo = t->ranges[range][0], hi = t->ranges[range][1];
    return Rand16() % (hi - lo + 1) + lo;
}

static u16 DrawTrainerNo(u32 round, u32 stage, u32 line, u8 alt)
{
    bool8 super = line >= 5 && line <= 8;
    s32 boss = -1;
    if (stage == 6)
    {
        if (!super && round == 2)
            boss = 0;
        else if (super && round == 6)
            boss = 1;
    }
    if (boss != -1)
    {
        switch (line)
        {
        case 0: case 5: return boss == 0 ? 0x132 : 0x133;
        case 1: case 6: return boss == 0 ? 0x134 : 0x135;
        default:        return boss == 0 ? (alt == 0 ? 0x137 : 0x136) : (alt == 0 ? 0x139 : 0x138);
        }
    }
    if (!super)
    {
        if (round >= 3)
            return 0;
        if (stage < 6)
            return RandIn(RANGE_D8 + round);
        return round < 2 ? RandIn(RANGE_D0 + round) : 0;
    }
    if (round >= 4)
        return Rand16() % 100 + 200;
    return stage < 6 ? RandIn(RANGE_E4 + round) : RandIn(RANGE_F4 + round);
}

static bool8 Used(const u16 *list, u16 value, u16 count)
{
    u16 i;
    for (i = 0; i < count; i++)
        if (list[i] == value)
            return TRUE;
    return FALSE;
}

static void DrawTrainerNos(void)
{
    u16 stage = sW.winCount / 7, no;
    u32 i, guard;
    if (T() == NULL)
        return;
    if (IsMulti(sW.playMode))
        for (i = 0; i < 14; i++)
        {
            guard = 0;
            do
                no = DrawTrainerNo(stage, i / 2, sW.playMode, i & 1);
            while (Used(sW.trainerNo, no, i) && ++guard < 200);
            sW.trainerNo[i] = no;
        }
    else
        for (i = 0; i < 7; i++)
        {
            guard = 0;
            do
                no = DrawTrainerNo(stage, i, sW.playMode, 0);
            while (Used(sW.trainerNo, no, i) && ++guard < 200);
            sW.trainerNo[i] = no;
        }
}

static void CreateRoundTrainers(void)
{
    u32 round = S()->play.round;
    if (IsMulti(sW.playMode))   // (the second trainer: none of the first's species or items)
    {
        CreateTrainer(&sW.trainers[0], sW.trainerNo[(round * 2) % 14], sW.memberNum, NULL, NULL, NULL);
        CreateTrainer(&sW.trainers[1], sW.trainerNo[(round * 2 + 1) % 14], sW.memberNum, sW.trainers[0].nationals,
                      sW.trainers[0].items, NULL);
    }
    else
        CreateTrainer(&sW.trainers[0], sW.trainerNo[round % 14], sW.memberNum, NULL, NULL, NULL);
    MPNet_Log("subway: round %u, trainers %u %u", round, sW.trainers[0].no, sW.trainers[1].no);
}

// BSUBWAY_SCR_CreatePartners: the three teams of the other hero (HILDA beside a boy, HILBERT beside a girl)
static void CreatePartners(void)
{
    u16 base = 300 + (gSaveBlock2Ptr->playerGender == MALE ? 3 : 0), mons[4];
    u32 i;
    for (i = 0; i < 4; i++)
        mons[i] = SpeciesToNationalPokedexNum(sW.memberMonsNo[i]);
    for (i = 0; i < 3; i++)
        sW.partnerResult[i] = CreateTrainer(&sW.partners[i], base + i, sW.memberNum, mons, sW.memberItemNo,
                                            &sW.partnerDraws[i]);
}

// ---- the work (BSUBWAY_SCR_CreateWork / DeleteWork)
void U3D_Subway_CreateWork(u16 resume, u16 playMode)
{
    struct U3DSubwaySave *s = S();
    u32 i;
    memset(&sW, 0, sizeof(sW));
    sW.live = TRUE;
    s->play.flag01 = 0;
    if (!resume)
    {
        sW.playMode = playMode;
        sW.memberNum = MemberNum(playMode);
        for (i = 0; i < 4; i++)
            sW.memberIndex[i] = 0xFF;
        for (i = 0; i < 14; i++)
            sW.trainerNo[i] = 0xFFFF;
        memset(&s->play, 0, sizeof(s->play));
        if (HasCleared(sW.playMode))
            sW.winCount = s->score.winCount[sW.playMode];
        s->play.mode = sW.playMode;
    }
    else
    {
        sW.playMode = s->play.mode;
        sW.memberNum = MemberNum(sW.playMode);
        memcpy(sW.memberIndex, s->play.members, sizeof(sW.memberIndex));
        memcpy(sW.trainerNo, s->play.trainerNo, sizeof(sW.trainerNo));
        for (i = 0; i < sW.memberNum && i < 4; i++)
            if (sW.memberIndex[i] < PARTY_SIZE)
            {
                sW.memberMonsNo[i] = GetMonData(&gPlayerParty[sW.memberIndex[i]], MON_DATA_SPECIES);
                sW.memberItemNo[i] = GetMonData(&gPlayerParty[sW.memberIndex[i]], MON_DATA_HELD_ITEM);
            }
        if (sW.playMode == 2 || sW.playMode == 7)
        {
            u16 base = gSaveBlock2Ptr->playerGender == MALE ? 303 : 300;
            sW.partnerNo = s->play.partnerNo % 3;
            sW.partnerDraws[sW.partnerNo] = s->play.draw;
            CreatePartnerMons(&sW.partners[sW.partnerNo], base + sW.partnerNo, s->play.partnerResult,
                              &sW.partnerDraws[sW.partnerNo]);
        }
        if (HasCleared(sW.playMode))
            sW.winCount = RepeatClearCount(sW.playMode) * 7 + s->play.round;
    }
    MPNet_Log("subway: work, mode %u%s, wins %u", sW.playMode, resume ? " (resumed)" : "", sW.winCount);
}

void U3D_Subway_DeleteWork(void)
{
    sW.live = FALSE;
}

// ---- choosing (BSUBWAY_SCR_CMD_START_PARTY_SELECT: the game's frontier party menu, any level, Black's banned list)
static bool8 sChoosing;
static const u16 sBanned[] = {150, 151, 249, 250, 251, 382, 383, 384, 385, 386, 483, 484, 487, 489, 490, 491, 492,
                              493, 494, 643, 644, 646, 647, 648, 649};   // (Black's regulations 20-22: the same list)

bool8 U3D_Subway_Choosing(void)
{
    return sChoosing;
}

bool8 U3D_Subway_Banned(u16 species)
{
    u16 national = SpeciesToNationalPokedexNum(species);
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sBanned); i++)
        if (sBanned[i] == national)
            return TRUE;
    return FALSE;
}

// POKE_REGULATION_CheckPartyUsable for Black's regulations 20-22 (singles 3, doubles 4, multi 2): enough POKéMON of
// different kinds that may enter (no EGGS, none of the banned list); any other regulation: usable
bool8 U3D_Subway_PartyUsable(u16 regulation)
{
    u16 kinds[PARTY_SIZE], need;
    u32 i, j, n = 0;
    switch (regulation)
    {
    case 20: need = 3; break;
    case 21: need = 4; break;
    case 22: need = 2; break;
    default: return TRUE;
    }
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 sp = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);
        if (sp == SPECIES_NONE || GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG) || U3D_Subway_Banned(sp))
            continue;
        for (j = 0; j < n; j++)
            if (kinds[j] == sp)
                break;
        if (j == n)
            kinds[n++] = sp;
    }
    return n >= need;
}

void U3D_Subway_PartyChosen(void)
{
    sChoosing = FALSE;
}

static bool8 SetSelectedMembers(void)
{
    u32 i;
    if (gSelectedOrderFromParty[0] == 0)
        return FALSE;
    for (i = 0; i < sW.memberNum && i < 4; i++)
    {
        u8 pos = gSelectedOrderFromParty[i];
        if (pos - 1 >= PARTY_SIZE)
            pos = 1;
        sW.memberIndex[i] = pos - 1;
        sW.memberMonsNo[i] = GetMonData(&gPlayerParty[sW.memberIndex[i]], MON_DATA_SPECIES);
        sW.memberItemNo[i] = GetMonData(&gPlayerParty[sW.memberIndex[i]], MON_DATA_HELD_ITEM);
    }
    return TRUE;
}

// ---- the battle (NAMEIN_SETUP_StartEncounterScene / FLD_BTL_INST_TOOL_CreateBtlSetup): the chosen POKéMON as
// level 50 copies (the party saved first, put back after), the round's trainers, the partner beside a multi
static bool8 sInBattle;
void U3D_Subway_StartBattle(void)
{
    u32 i;
    SavePlayerParty();
    ZeroPlayerPartyMons();
    for (i = 0; i < sW.memberNum && i < 4; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (sW.memberIndex[i] >= PARTY_SIZE)
            continue;
        *mon = gSaveBlock1Ptr->playerParty[sW.memberIndex[i]];
        if (GetMonData(mon, MON_DATA_LEVEL) != SUBWAY_LEVEL)   // (POKE_TOOL_SetLevelAndRecalc: down or up to 50)
        {
            u32 exp = gExperienceTables[gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES)].growthRate][SUBWAY_LEVEL];
            SetMonData(mon, MON_DATA_EXP, &exp);
            CalculateMonStats(mon);
        }
    }
    CalculatePlayerPartyCount();
    U3D_SetSubwayTrainer(0, sW.trainers[0].no, sW.trainers[0].type, sW.trainers[0].mons, sW.trainers[0].count);
    if (IsMulti(sW.playMode))
    {
        struct SubwayTrainer *p = &sW.partners[sW.partnerNo % 3];
        U3D_SetSubwayTrainer(1, sW.trainers[1].no, sW.trainers[1].type, sW.trainers[1].mons, sW.trainers[1].count);
        U3D_SetSubwayTrainer(2, p->no, p->type, p->mons, p->count);
    }
    sInBattle = TRUE;
    sW.battleWon = FALSE;
    U3D_PrepareSubwayBattle(sW.playMode, sW.memberNum);
}

void U3D_Subway_BattleEnded(bool8 won)
{
    if (!sInBattle)
        return;
    sInBattle = FALSE;
    sW.battleWon = won;
    LoadPlayerParty();   // (the party as it was: the copies' level, HP and PP stay behind)
    MPNet_Log("subway: battle %s (wins %u)", won ? "won" : "lost", sW.winCount);
}

bool8 U3D_Subway_InBattle(void)
{
    return sInBattle;
}

// ---- BATTLE POINTS (BSUBWAY_SCR_AddBattlePoint: by the line and how many trains this streak has cleared)
static u32 AddBattlePoint(void)
{
    const struct Tables *t = T();
    s32 points = 0, row = 0, idx;
    u16 count;
    if (t == NULL)
        return 0;
    if (sW.playMode == 4)
        points = 5;
    else
    {
        count = RepeatClearCount(sW.playMode);
        switch (sW.playMode)
        {
        case 0: row = 0; break;
        case 5: row = 1; points = 1; break;
        case 1: row = 2; break;
        case 6: row = 3; points = 1; break;
        case 2: case 3: row = 4; break;
        case 7: case 8: row = 5; points = 1; break;
        default: row = 0; break;
        }
        count = count - 1;
        idx = (s16)count < 0 ? 0 : count >= 10 ? 9 : count;
        if (sW.fixedBp)
            points = points == 1 ? 30 : 10;
        else
            points = t->bp[row][idx];
    }
    if (points == 0)
        points = 1;
    AddBattlePoints(points);
    return points;
}

static void SetClearScore(void)
{
    u8 mode = sW.playMode;
    struct U3DSubwaySave *s = S();
    s->score.winCount[mode] = sW.winCount;
    if (s->score.maxWin[mode] < s->score.winCount[mode])
        s->score.maxWin[mode] = s->score.winCount[mode];
    if (s->score.clearCount[mode] < 0xFFFE)
        s->score.clearCount[mode]++;
    s->play.round = 0;
}

static void SetLoseScore(void)
{
    u8 mode = sW.playMode;
    struct U3DSubwaySave *s = S();
    s->score.winCount[mode] = sW.winCount;
    if (s->score.maxWin[mode] < sW.winCount)
        s->score.maxWin[mode] = sW.winCount;
    s->score.clearCount[mode] = 0;
    s->play.round = 0;
}

static void SaveRest(bool8 trainers)
{
    struct U3DSubwaySave *s = S();
    s->play.mode = sW.playMode;
    if (!trainers)
        s->play.round = 0;
    memcpy(s->play.members, sW.memberIndex, sizeof(s->play.members));
    if (trainers)
        memcpy(s->play.trainerNo, sW.trainerNo, sizeof(s->play.trainerNo));
    s->play.flag01 = 1;
    if (sW.playMode == 2 || sW.playMode == 7)
    {
        s->play.partnerNo = sW.partnerNo;
        s->play.draw = sW.partnerDraws[sW.partnerNo % 3];
        s->play.partnerResult = sW.partnerResult[sW.partnerNo % 3];
    }
}

// the return places (data_021DADAC / 021DAE50): a line's platform, by the line (or a train's index)
bool8 U3D_Subway_Return(u8 index, u16 *zone, s16 *x, s16 *z)
{
    const struct Tables *t = T();
    if (t == NULL || index >= 9)
        return FALSE;
    *zone = t->retZone[index], *x = t->retX[index], *z = t->retZ[index];
    return TRUE;
}

// the passengers (BSUBWAY_SCR_CreateMarkerObjects): those whose threshold is this streak's cleared trains
bool8 U3D_Subway_Passenger(u32 i, u16 *code, s16 *x, s16 *z, u16 *param)
{
    const struct Tables *t = T();
    const u8 *e;
    u16 threshold, no;
    if (t == NULL || i >= t->spawns || sW.playMode == 4)
        return FALSE;
    e = (const u8 *)(t + 1) + 16 * i;
    threshold = RepeatClearCount(sW.playMode) + (sW.playMode >= 5 ? 3 : 0);
    no = Le16(e + 2);
    if (no == 0 || no > 22 || threshold != t->thresholds[no - 1])
    {
        *code = 0xFFFF;
        return TRUE;
    }
    *code = Le16(e), *x = (s16)Le16(e + 4), *z = (s16)Le16(e + 8), *param = Le16(e + 10);
    return TRUE;
}

// ---- BSUBWAY_SCR_CmdTool: Black's command CMD; what the VM does next (u3d_subway.h); a message's bank and line
u8 U3D_Subway_Tool(u16 cmd, u16 op1, u16 op2, u16 *result, u16 *msgBank, u16 *msg)
{
    struct U3DSubwaySave *s = S();
    u8 mode = s->play.mode;
    if (!sW.live && cmd >= 300)
        return U3D_SUBWAY_NONE;
    switch (cmd)
    {
    // (the field's people and places: the VM's own)
    case 0: case 4: case 5: case 13: case 23: case 24: case 25: case 26: case 35: case 36: case 37: case 45: case 322:
    case 329:
        return U3D_SUBWAY_VM;
    case 1:   // RESET_SYSTEM (a REST: the save made, back to the title screen)
        return U3D_SUBWAY_RESET;
    case 2:  memset(&s->play, 0, sizeof(s->play)); break;   // CLEAR_PLAY_DATA
    case 3:  *result = s->play.flag01; break;               // GET_PLAY_DATA_STATE
    case 6:  *result = op1 < U3D_SUBWAY_MODES ? min(s->score.winCount[op1], 9999) : 0; break;
    case 7:  // BEGIN_PLAY_GET_MODE
        s->play.round = 0;
        if (mode < U3D_SUBWAY_MODES)
            s->score.clearCount[mode] = 0;
        *result = mode;
        break;
    case 8:  *result = s->play.round + 1; break;            // GET_NEXT_ROUND_FROM_SAVE
    case 11: *result = s->play.round + 1; break;            // GET_NEXT_ROUND
    case 12: *result = gSaveBlock2Ptr->playerGender == MALE ? 1 : 4; break;   // GET_PLAYER_LOOK (Black's object
                                                                              // codes: HILBERT 1, HILDA 4)
    case 14: {   // GET_SCORE_FLAG (data_021DAD36: the flag of each line; the Wi-Fi line's is always set)
        static const u8 sFlagOf[9] = {4, 5, 6, 6, 11, 7, 8, 9, 9};
        *result = op1 < 9 && sFlagOf[op1] == 11 ? 1 : AccessFlag(op1 < 9 ? sFlagOf[op1] : 11, 0);
        break;
    }
    case 15: {   // SET_SCORE_FLAG
        static const u8 sFlagOf[9] = {4, 5, 6, 6, 11, 7, 8, 9, 9};
        if (op1 < 9 && sFlagOf[op1] != 11)
            AccessFlag(sFlagOf[op1], 1);
        break;
    }
    case 16: *result = AccessFlag(10, 0); break;
    case 17: AccessFlag(10, 1); break;
    case 18: case 19: case 20: case 34: case 350:   // (the train's sign and doors: the train isn't drawn)
        break;
    case 21: *result = mode; break;                          // GET_SAVED_PLAY_MODE
    case 22: *result = IsMulti(op1); break;                  // IS_MULTI_PLAY_MODE
    case 27: *result = s->score.level ? s->score.level : 1; break;   // GET_RANK
    case 28:   // LOWER_RANK
        AccessFlag(0, 2);
        if (s->score.level >= 10)
            *result = 0;
        else
            s->score.level++, *result = 1;
        break;
    case 29: {   // RAISE_RANK
        static const u8 sNeed[12] = {0, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0};   // (data_021DACD0)
        u8 rank = s->score.level, streak = UpdateStreak(3);
        *result = 0;
        if (rank > 1 && streak >= sNeed[rank - 1])
        {
            s->score.level--;
            UpdateStreak(2);
            AccessFlag(0, 2);
            *result = 1;
        }
        break;
    }
    case 30: *result = op1 < U3D_SUBWAY_MODES ? RepeatClearCount(op1) : 0; break;   // GET_SCORE_ROUND_VALUE
    case 31: *result = gSaveBlock2Ptr->playerGender == MALE ? 4 : 1; break;   // GET_PARTNER_LOOK
    case 33: case 43: case 107: case 112: case 113: case 200: case 203: case 416:
        *result = 0;   // (Battle Videos, the BATTLE BOX, Wi-Fi: none)
        break;
    case 38: *result = HasCleared(op1); break;               // GET_MODE_CLEARED
    case 39: case 40: case 41: case 42:   // (the NPCs on the platform: Wi-Fi records, none)
        *result = cmd == 39 ? 0 : cmd == 42 ? 4 : 0;
        break;
    case 44:   // START_SCREEN_TASK: a yes / no menu (op1 1: B is no)
        return U3D_SUBWAY_YESNO;
    case 100: AccessFlag(1, op1 == 0 ? 2 : 1); break;        // SET_SCORE_FLAG_FIRST
    case 101: *result = AccessFlag(1, 0); break;
    case 103: case 104: case 105: case 111: case 408:
        *result = 0;   // (the wireless and Wi-Fi screens: closed)
        break;
    case 108: s->play.byte11 = op1; break;
    case 201: case 202: case 501: break;
    case 300:   // START_PARTY_SELECT
        sChoosing = TRUE;
        return U3D_SUBWAY_PARTY;
    case 301: *result = SetSelectedMembers(); break;
    case 303:   // IS_WIN_MAX
        if (!sW.winMax && s->play.round >= 7)
            sW.winMax = TRUE;
        *result = sW.winMax;
        break;
    case 304: SetLoseScore(); break;
    case 305: SetClearScore(); break;
    case 306: SaveRest(TRUE); break;
    case 307: CreateRoundTrainers(); break;
    case 308: {   // GET_TRAINER_OBJ_CODE (FLD_BTL_INST_TOOL_LookupKeyValue)
        const struct Tables *t = T();
        u32 i;
        *result = 0xA;
        for (i = 0; t != NULL && i < 0x35; i++)
            if (t->typeObj[i][0] == sW.trainers[op1 & 1].type)
                *result = t->typeObj[i][1];
        break;
    }
    case 309: return U3D_SUBWAY_BATTLE;                      // START_ENCOUNTER_TASK
    case 310: *result = sW.playMode; break;
    case 311: sW.fixedBp = op1; break;
    case 312: *result = sW.fixedBp; break;
    case 313: *result = AddBattlePoint(); break;
    case 314: sW.partnerNo = op1 % 3; break;
    case 315: *result = sW.partnerNo; break;
    case 316: DrawTrainerNos(); break;
    case 317: *result = op1 < 4 ? sW.memberIndex[op1] : 0; break;
    case 319: break;                                          // (CLEAR_RECV_BUF)
    case 320: case 352:   // START_PARTNER_TASK_A / B: trainer OP1's greeting in a balloon by person OP2
        *msgBank = BANK_SUBWAY_WORDS;
        *msg = (cmd == 320 ? sW.trainers[op1 & 1].no : sW.partners[sW.partnerNo % 3].no) * 3;
        if (cmd == 352 && op1 < 2)
            *msg = sW.trainers[op1].no * 3;
        return U3D_SUBWAY_MESSAGE;
    case 321: SaveRest(FALSE); break;
    case 323: break;                                          // (LOAD_MEMBERS: done as they're chosen)
    case 324: s->play.byte10 = 0; break;                      // (the BATTLE BOX: never)
    case 325: break;                                          // (CHANGE_COMM_MULTI_MODE: no wireless)
    case 326: *result = mode >= 5 && mode <= 8; break;        // IS_UPPER_PLAY_MODE
    case 330: sW.flag8 = op1; break;
    case 331: *result = sW.flag8; break;
    case 332: *result = sW.battleWon; break;
    case 333: sW.winMax = FALSE, sW.fixedBp = FALSE; break;
    case 334: case 345: case 347: case 353: break;            // (records, Battle Videos, the setup, tallies)
    case 335: CreatePartners(); break;
    case 336: *result = op1 < 4 ? sW.memberMonsNo[op1] : 0; break;
    case 337: sW.commMode = op1; break;
    case 338: *result = sW.commMode; break;
    case 339: {   // CHECK_PARTY_REGULATION: enough POKéMON that may enter (eggs and Black's banned list out)
        u32 i, n = 0;
        for (i = 0; i < PARTY_SIZE; i++)
        {
            u16 sp = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);
            if (sp != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG) && !U3D_Subway_Banned(sp))
                n++;
        }
        *result = n >= MemberNum(mode);
        break;
    }
    case 340: break;
    case 342: *result = MemberNum(mode); break;
    case 343: break;
    case 344: *result = 0; break;   // (GET_BATTLE_REC_EXISTS)
    case 346: break;                                          // (START_RESULT_MESSAGE: the Battle Video's save)
    case 349: s->play.round = 0; if (mode < U3D_SUBWAY_MODES) s->score.clearCount[mode] = 0; break;
    case 351: {   // GET_TRAINER_NO
        u32 round = s->play.round;
        *result = IsMulti(sW.playMode) ? sW.trainerNo[(round * 2 + op1) % 14] : sW.trainerNo[round % 14];
        break;
    }
    case 354: sW.flag12 = op1; break;
    case 355: *result = 1; break;   // (GET_STATE_FLAG12: set once a battle can't be recorded; Battle Videos are
                                    // closed, so RECORD and the save-a-video question never come up)
    case 356: *result = s->play.byte10; break;
    case 357:   // NEXT_ROUND_GET_WIN_COUNT
        if (s->play.round < 255)
            s->play.round++;
        if (sW.winCount < 0xFFFF)
            sW.winCount++;
        *result = min(sW.winCount, 9999);
        break;
    case 358: *result = min(sW.winCount, 9999); break;
    case 359:   // COMMIT_FIRST_CLEAR
        if (!HasCleared(sW.playMode))
        {
            s->score.clearCount[sW.playMode] = 1;
            s->play.round = 0;
            sW.winCount = 0;
        }
        break;
    case 109: *result = s->play.byte11; break;
    case 400: case 401: case 405: case 406: case 410: case 413: case 414: break;   // (wireless: closed)
    case 402: *result = 1; break;
    case 403: case 404: *result = 0; break;
    case 407: *result = 0; break;
    case 409: *result = 0; break;
    case 411: case 412: *result = 0; break;
    case 415: *result = 0; break;
    default:
        MPNet_Log("subway: command %u (%u, %u) not played yet", cmd, op1, op2);
        break;
    }
    return U3D_SUBWAY_NONE;
}
