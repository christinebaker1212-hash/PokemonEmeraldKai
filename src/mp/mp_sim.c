/* The BATTLE SIMULATOR: a counter in each of the three BATTLE TENTS (SLATEPORT, VERDANTURF, FALLARBOR) where every
 * opponent is a digital copy of another player from the last server this game played on.
 *
 * Each player's record (their LOOK and their top 6 Pokemon by battles taken part in, mp_life.h MpLifeRec.battles) is
 * sent to the server when the game connects, back on the field after a battle, and when the game closes; the server
 * keeps everyone's latest (mp_scores.c MP_ST_SIM_*) and sends the whole roster back on connecting. This game keeps
 * the roster in "<save>.sim" with the server it came from, so the simulator works offline too; a save that never
 * played online has none. Nobody can opt out: the only way to be left out is never playing online.
 *
 * The challenge: everyone at level 50 (the player's own Pokemon too, put back as they were after each battle: no EXP,
 * nothing used up), a streak of battles against random digital trainers (each once before any repeats), the Battle
 * Frontier's AI (the e-Reader trainer battle type: CHECK_BAD_MOVE | CHECK_VIABILITY | TRY_TO_FAINT, and trainers
 * switch as usual). A loss ends the streak. Each win pays prize money; a new best streak at 3 / 5 / 7 / 10 / 15 / 20
 * gives an item. data/scripts/battle_sim.inc; test sim_tent.
 */
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "money.h"
#include "mp.h"
#include "mp_life.h"
#include "mp_net.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/songs.h"

extern char gPlatformSavePath[512];
void MP_PackAppearance(const struct MpAppearance *a, u8 *p);
void MP_UnpackAppearance(const u8 *p, struct MpAppearance *a);
struct MpAppearance MP_Avatar_LocalAppearance(void);
u16 MP_Avatar_DigitalFrontPic(const struct MpAppearance *app);

#define SIM_LOOK 32
#define SIM_MONS 6
#define SIM_VERSION 1

// A record as the server keeps it and the cache holds it (opaque to the server past its first MP_SIM_KEY bytes)
struct SimRecord
{
    u8 name[8];        // game charset, EOS padded: with trainerId, who it is
    u32 trainerId;
    u8 look[SIM_LOOK];
    u8 count;
    u8 version;
    u8 gender;
    u8 pad;
    struct BattleTowerPokemon mons[SIM_MONS];
};

#define SIM_CACHE_MAGIC 0x43534D53   // "SMSC"
#define SIM_CACHE_MAX MP_SIM_PLAYERS
#define SIM_SERVER_LEN 64

static struct
{
    bool8 loaded;
    char loadedFor[512];
    char server[SIM_SERVER_LEN];
    u16 count;
    u16 best;                // best streak
    struct SimRecord rec[SIM_CACHE_MAX];
    // a roster coming in (parts)
    u16 incoming;
    u8 partsSeen;
    bool8 receiving;
    struct SimRecord in[SIM_CACHE_MAX];
    // the challenge
    u16 streak;
    s16 current;             // the record being battled
    u8 usedBits[(SIM_CACHE_MAX + 7) / 8];
    bool8 uploadPending;
    struct Pokemon saved[PARTY_SIZE];
} sSim;

bool8 gMpSimBattle;   // a BATTLE SIMULATOR battle is on (battle_controller_opponent.c, battle_message.c)

// ---------------------------------------------------------------------------
// The cache

static void CachePath(char *out, size_t size)
{
    snprintf(out, size, "%s.sim", gPlatformSavePath);
}

static void LoadCache(void)
{
    char path[600];
    FILE *f;
    u32 magic = 0;
    if (sSim.loaded && strcmp(sSim.loadedFor, gPlatformSavePath) == 0)
        return;
    sSim.loaded = TRUE;
    snprintf(sSim.loadedFor, sizeof(sSim.loadedFor), "%s", gPlatformSavePath);
    sSim.count = 0;
    sSim.best = 0;
    sSim.server[0] = 0;
    CachePath(path, sizeof(path));
    if (!(f = fopen(path, "rb")))
        return;
    if (fread(&magic, 4, 1, f) == 1 && magic == SIM_CACHE_MAGIC
     && fread(sSim.server, SIM_SERVER_LEN, 1, f) == 1 && fread(&sSim.best, 2, 1, f) == 1 && fread(&sSim.count, 2, 1, f) == 1
     && sSim.count <= SIM_CACHE_MAX)
        sSim.count = (u16)fread(sSim.rec, sizeof(sSim.rec[0]), sSim.count, f);
    else
        sSim.count = 0;
    sSim.server[SIM_SERVER_LEN - 1] = 0;
    fclose(f);
}

static void SaveCache(void)
{
    char path[600], tmp[610];
    FILE *f;
    u32 magic = SIM_CACHE_MAGIC;
    if (MOEMON_DEV_BUILD)
        return;
    CachePath(path, sizeof(path));
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    if (!(f = fopen(tmp, "wb")))
        return;
    fwrite(&magic, 4, 1, f);
    fwrite(sSim.server, SIM_SERVER_LEN, 1, f);
    fwrite(&sSim.best, 2, 1, f);
    fwrite(&sSim.count, 2, 1, f);
    fwrite(sSim.rec, sizeof(sSim.rec[0]), sSim.count, f);
    fclose(f);
    remove(path);
    rename(tmp, path);
}

// Which server the roster comes from: the address a friend's server was reached at, else the online lobby
static void ServerName(char *out)
{
    snprintf(out, SIM_SERVER_LEN, "%s", gMpConfig.lastRoomAddr[0] ? gMpConfig.lastRoomAddr : "online lobby");
}

// ---------------------------------------------------------------------------
// This player's record: their look and their top 6 by battles taken part in

struct Candidate { struct BoxPokemon *box; struct Pokemon *mon; u16 battles; u8 level; };

static u16 BattlesOf(struct BoxPokemon *box)
{
    struct MpLifeRec *rec = MP_Life_Get(box, FALSE);
    return rec != NULL ? rec->battles : 0;
}

static void Consider(struct Candidate *best, u8 *n, struct BoxPokemon *box, struct Pokemon *mon)
{
    struct Candidate c;
    u8 i, at;
    if (!GetBoxMonData(box, MON_DATA_SANITY_HAS_SPECIES) || GetBoxMonData(box, MON_DATA_SANITY_IS_EGG))
        return;
    c.box = box;
    c.mon = mon;
    c.battles = BattlesOf(box);
    c.level = GetLevelFromBoxMonExp(box);
    // most battles first, then the higher level (the party first among equals: it's considered first)
    for (at = 0; at < *n; at++)
        if (c.battles > best[at].battles || (c.battles == best[at].battles && c.level > best[at].level))
            break;
    if (at >= SIM_MONS)
        return;
    if (*n < SIM_MONS)
        (*n)++;
    for (i = *n - 1; i > at; i--)
        best[i] = best[i - 1];
    best[at] = c;
}

static u16 BuildRecord(struct SimRecord *r)
{
    struct Candidate best[SIM_MONS];
    struct PokemonStorage *pc = MP_PersonalStorage();
    struct MpAppearance app = MP_Avatar_LocalAppearance();
    u8 n = 0, i, b;
    memset(r, 0, sizeof(*r));
    memset(r->name, EOS, sizeof(r->name));
    StringCopyN(r->name, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    r->trainerId = T1_READ_32(gSaveBlock2Ptr->playerTrainerId);
    MP_PackAppearance(&app, r->look);
    r->gender = gSaveBlock2Ptr->playerGender;
    r->version = SIM_VERSION;
    // (during a simulator battle the party is at level 50: the record is always of the real one)
    struct Pokemon *party = gMpSimBattle ? sSim.saved : gPlayerParty;
    for (i = 0; i < PARTY_SIZE; i++)
        Consider(best, &n, &party[i].box, &party[i]);
    if (pc != NULL)
        for (b = 0; b < TOTAL_BOXES_COUNT; b++)
            for (i = 0; i < IN_BOX_COUNT; i++)
                Consider(best, &n, &pc->boxes[b][i], NULL);
    for (i = 0; i < n; i++)
    {
        struct Pokemon mon;
        if (best[i].mon != NULL)
            mon = *best[i].mon;
        else
            BoxMonToMon(best[i].box, &mon);
        ConvertPokemonToBattleTowerPokemon(&mon, &r->mons[i]);
        MPNet_Log("sim: our record's Pokemon %d: species %d Lv%d (%d battles)", i, r->mons[i].species,
                  r->mons[i].level, best[i].battles);
    }
    r->count = n;
    return sizeof(*r);
}

// To the server: on connecting, after a battle (once back on the field), and when the game closes
void MP_Sim_Upload(void)
{
    static u8 buf[1 + sizeof(struct SimRecord)];
    struct SimRecord r;
    u16 len;
    if (!MP_IsConnected() || gMpLocalId < 0)
        return;
    len = BuildRecord(&r);
    if (r.count == 0)
        return;
    buf[0] = MP_ST_SIM_PUT;
    memcpy(buf + 1, &r, len);
    MPNet_ClientSend(MPMSG_C_STORE, buf, 1 + len);
    MPNet_Log("sim: sent our record (%d Pokemon)", r.count);
}

// mp_main.c: in a room (the server takes store requests from here on): our record, then everyone's
void MP_Sim_OnConnected(void)
{
    u8 op = MP_ST_SIM_GET;
    LoadCache();
    MP_Sim_Upload();
    sSim.receiving = FALSE;
    MPNet_ClientSend(MPMSG_C_STORE, &op, 1);
}

// mp_life_battle.c: a battle is over: the record goes when it costs nothing (back on the field, MP_Sim_Update)
void MP_Sim_BattleOver(void)
{
    sSim.uploadPending = TRUE;
}

void MP_Sim_Update(void)
{
    if (sSim.uploadPending && MP_IsLocalFree() && !gMpSimBattle)
    {
        sSim.uploadPending = FALSE;
        MP_Sim_Upload();
    }
}

// sdl2.c: the game is closing
void MP_Sim_OnClose(void)
{
    if (!MP_IsConnected())
        return;
    MP_Sim_Upload();
    MPNet_ClientFlush();
}

// The server's roster, a part at a time (mp_sharedpc.c MP_Store_OnMessage)
void MP_Sim_OnStore(const u8 *data, u16 len)
{
    u16 pos = 4, i;
    u8 part, parts, count;
    if (len < 4)
        return;
    part = data[1], parts = data[2], count = data[3];
    if (part == 0xFF)
    {
        // a friend in the room just sent theirs: into the roster we keep (theirs replaced, or added)
        struct SimRecord r;
        u16 l = len >= 6 ? (data[4] | data[5] << 8) : 0, k;
        if (l < offsetof(struct SimRecord, mons) || 6 + l > len)
            return;
        memset(&r, 0, sizeof(r));
        memcpy(&r, data + 6, l < sizeof(r) ? l : sizeof(r));
        if (r.version != SIM_VERSION || r.count == 0)
            return;
        if (r.count > SIM_MONS)
            r.count = SIM_MONS;
        LoadCache();
        for (k = 0; k < sSim.count; k++)
            if (!memcmp(sSim.rec[k].name, r.name, sizeof(r.name)) && sSim.rec[k].trainerId == r.trainerId)
                break;
        if (k == sSim.count && sSim.count < SIM_CACHE_MAX)
            sSim.count++;
        if (k < sSim.count)
            sSim.rec[k] = r;
        ServerName(sSim.server);
        SaveCache();
        MPNet_Log("sim: a record arrived (%d player(s) kept)", sSim.count);
        return;
    }
    if (part == 0)
    {
        sSim.receiving = TRUE;
        sSim.incoming = 0;
        sSim.partsSeen = 0;
    }
    if (!sSim.receiving)
        return;
    for (i = 0; i < count && pos + 2 <= len; i++)
    {
        u16 l = data[pos] | data[pos + 1] << 8;
        pos += 2;
        if (pos + l > len)
            break;
        if (l >= offsetof(struct SimRecord, mons) && sSim.incoming < SIM_CACHE_MAX)
        {
            struct SimRecord *r = &sSim.in[sSim.incoming];
            memset(r, 0, sizeof(*r));
            memcpy(r, data + pos, l < sizeof(*r) ? l : sizeof(*r));
            if (r->count > SIM_MONS)
                r->count = SIM_MONS;
            if (r->version == SIM_VERSION && r->count > 0)
                sSim.incoming++;
        }
        pos += l;
    }
    sSim.partsSeen++;
    if (part + 1 >= parts)
    {
        LoadCache();
        memcpy(sSim.rec, sSim.in, sSim.incoming * sizeof(sSim.in[0]));
        sSim.count = sSim.incoming;
        ServerName(sSim.server);
        sSim.receiving = FALSE;
        SaveCache();
        MPNet_Log("sim: the roster from %s: %d player(s)", sSim.server, sSim.count);
    }
}

// ---------------------------------------------------------------------------
// The challenge (data/scripts/battle_sim.inc)

static bool8 IsMe(const struct SimRecord *r)
{
    u8 name[8];
    memset(name, EOS, sizeof(name));
    StringCopyN(name, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    return r->trainerId == T1_READ_32(gSaveBlock2Ptr->playerTrainerId) && memcmp(r->name, name, PLAYER_NAME_LENGTH) == 0;
}

static u16 Opponents(void)
{
    u16 i, n = 0;
    LoadCache();
    for (i = 0; i < sSim.count; i++)
        if (!IsMe(&sSim.rec[i]) && sSim.rec[i].count > 0)
            n++;
    return n;
}

static void NameOf(const struct SimRecord *r, u8 *out)
{
    u8 i;
    for (i = 0; i < PLAYER_NAME_LENGTH && r->name[i] != EOS; i++)
        out[i] = r->name[i];
    out[i] = EOS;
}

// VAR_RESULT: digital trainers there are (0: nobody to battle); STR_VAR_1 how many, STR_VAR_2 the best streak
void MP_Sim_Intro(void)
{
    u16 n = Opponents();
    gSpecialVar_Result = n;
    ConvertIntToDecimalStringN(gStringVar1, n, STR_CONV_MODE_LEFT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar2, sSim.best, STR_CONV_MODE_LEFT_ALIGN, 4);
    sSim.streak = 0;
    sSim.current = -1;
    memset(sSim.usedBits, 0, sizeof(sSim.usedBits));
    MPNet_Log("sim: %d digital trainer(s) here, best streak %d", n, sSim.best);
}

// VAR_RESULT: TRUE when the party has a Pokemon that can battle
void MP_Sim_CanEnter(void)
{
    u8 i;
    gSpecialVar_Result = FALSE;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            gSpecialVar_Result = TRUE;
}

// The next opponent: each digital trainer once before anyone repeats. STR_VAR_1 their name, STR_VAR_2 the battle's number
void MP_Sim_Next(void)
{
    u16 i, n = 0, pick, opts[SIM_CACHE_MAX];
    LoadCache();
    for (i = 0; i < sSim.count; i++)
        if (!IsMe(&sSim.rec[i]) && sSim.rec[i].count > 0 && !(sSim.usedBits[i / 8] & (1 << (i % 8))))
            opts[n++] = i;
    if (n == 0)
    {
        memset(sSim.usedBits, 0, sizeof(sSim.usedBits));   // everyone met: around again
        for (i = 0; i < sSim.count; i++)
            if (!IsMe(&sSim.rec[i]) && sSim.rec[i].count > 0 && (u16)i != (u16)sSim.current)
                opts[n++] = i;
        if (n == 0)
            for (i = 0; i < sSim.count; i++)
                if (!IsMe(&sSim.rec[i]) && sSim.rec[i].count > 0)
                    opts[n++] = i;
    }
    if (n == 0)
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    pick = opts[Random() % n];
    sSim.usedBits[pick / 8] |= 1 << (pick % 8);
    sSim.current = pick;
    NameOf(&sSim.rec[pick], gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, sSim.streak + 1, STR_CONV_MODE_LEFT_ALIGN, 4);
    gSpecialVar_Result = TRUE;
    MPNet_Log("sim: battle %d against %d's data (%d Pokemon)", sSim.streak + 1, pick, sSim.rec[pick].count);
}

// Level 50: the EXP of that level, the stats it gives, everything restored
static void SetLevel50(struct Pokemon *mon)
{
    u32 exp = gExperienceTables[gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES)].growthRate][50];
    u8 level = 50;
    u16 hp;
    u32 status = 0;
    SetMonData(mon, MON_DATA_EXP, &exp);
    SetMonData(mon, MON_DATA_LEVEL, &level);
    CalculateMonStats(mon);
    hp = GetMonData(mon, MON_DATA_MAX_HP);
    SetMonData(mon, MON_DATA_HP, &hp);
    SetMonData(mon, MON_DATA_STATUS, &status);
    MonRestorePP(mon);
}

static void CB2_EndSimBattle(void)
{
    bool8 won = gBattleOutcome == B_OUTCOME_WON;
    // the player's Pokemon as they were before (the battle at level 50 changes nothing of theirs)
    memcpy(gPlayerParty, sSim.saved, sizeof(gPlayerParty));
    CalculatePlayerPartyCount();
    gMpSimBattle = FALSE;
    MP_MUSIC_SCENE_RELEASE();
    gSpecialVar_Result = won;
    MPNet_Log("sim: battle %d %s (our lead back at Lv%d)", sSim.streak + 1, won ? "won" : "lost",
              (int)GetMonData(&gPlayerParty[0], MON_DATA_LEVEL));
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void Task_StartSimBattle(u8 taskId)
{
    if (IsBattleTransitionDone() == TRUE)
    {
        gMain.savedCallback = CB2_EndSimBattle;
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

// The battle against the chosen digital trainer (waitstate: the script goes on after it, VAR_RESULT TRUE = won)
void MP_Sim_Battle(void)
{
    const struct SimRecord *r;
    u8 i;
    if (sSim.current < 0 || sSim.current >= sSim.count)
    {
        gSpecialVar_Result = FALSE;
        ScriptContext_Enable();
        return;
    }
    r = &sSim.rec[sSim.current];
    memcpy(sSim.saved, gPlayerParty, sizeof(gPlayerParty));
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            SetLevel50(&gPlayerParty[i]);
    ZeroEnemyPartyMons();
    for (i = 0; i < r->count && i < SIM_MONS; i++)
    {
        struct BattleTowerPokemon m = r->mons[i];
        m.level = 50;
        CreateBattleTowerMon(&gEnemyParty[i], &m);
        SetLevel50(&gEnemyParty[i]);
        MPNet_Log("sim: their Pokemon %d: species %d Lv%d", i, m.species, (int)GetMonData(&gEnemyParty[i], MON_DATA_LEVEL));
    }
    gMpSimBattle = TRUE;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_EREADER_TRAINER;
    gTrainerBattleOpponent_A = 0;
    CreateTask(Task_StartSimBattle, 1);
    // custom music: the battle's songs (the game's trainer battle and victory songs) are this scene's until it ends
    MP_MUSIC_SCENE_HOLD(MUS_VS_TRAINER, MUS_VICTORY_TRAINER);
    PlayMapChosenOrBattleBGM(0);
    BattleTransition_StartOnField(GetSpecialBattleTransition(B_TRANSITION_GROUP_E_READER));
}

// A win: the streak, its prize money, and an item for a new best at 3 / 5 / 7 / 10 / 15 / 20.
// STR_VAR_1 the streak, STR_VAR_2 the money, STR_VAR_3 the item (VAR_RESULT: the item, 0 = none)
void MP_Sim_Won(void)
{
    static const struct { u16 streak, item; } sMilestones[] = {
        {3, ITEM_RARE_CANDY}, {5, ITEM_PP_UP}, {7, ITEM_NUGGET}, {10, ITEM_PP_MAX}, {15, ITEM_LUCKY_EGG}, {20, ITEM_MASTER_BALL},
    };
    u32 money;
    u16 item = ITEM_NONE;
    u8 i;
    sSim.streak++;
    money = 500 + 250 * sSim.streak;
    if (money > 5000)
        money = 5000;
    AddMoney(&gSaveBlock1Ptr->money, money);
    if (sSim.streak > sSim.best)
    {
        for (i = 0; i < ARRAY_COUNT(sMilestones); i++)
            if (sMilestones[i].streak == sSim.streak && CheckBagHasSpace(sMilestones[i].item, 1))
            {
                item = sMilestones[i].item;
                AddBagItem(item, 1);
            }
        sSim.best = sSim.streak;
        SaveCache();
    }
    ConvertIntToDecimalStringN(gStringVar1, sSim.streak, STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(gStringVar2, money, STR_CONV_MODE_LEFT_ALIGN, 6);
    if (item != ITEM_NONE)
        CopyItemName(item, gStringVar3);
    gSpecialVar_Result = item;
    MPNet_Log("sim: streak %d (best %d), prize %u, item %d", sSim.streak, sSim.best, (unsigned)money, item);
}

// The streak's end (a loss or stopping): STR_VAR_1 the streak, STR_VAR_2 the best
void MP_Sim_End(void)
{
    ConvertIntToDecimalStringN(gStringVar1, sSim.streak, STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(gStringVar2, sSim.best, STR_CONV_MODE_LEFT_ALIGN, 4);
    MPNet_Log("sim: the challenge ends at %d", sSim.streak);
    sSim.streak = 0;
    sSim.current = -1;
}

// ---------------------------------------------------------------------------
// The battle's side (battle_controller_opponent.c, battle_message.c)

u16 MP_Sim_TrainerPic(void)
{
    struct MpAppearance app;
    if (sSim.current < 0 || sSim.current >= sSim.count)
        return 0;
    MP_UnpackAppearance(sSim.rec[sSim.current].look, &app);
    app.hasCustom = FALSE;   // (a custom picture pack never reaches the server: the look's own picture)
    return MP_Avatar_DigitalFrontPic(&app);
}

static const u8 sText_SimClass[] = _("DIGITAL");
static const u8 sText_SimLose[] = _("…Battle data saved.\nThat was a good simulation!");
static const u8 sText_SimWin[] = _("Simulation complete.\nThe data wins this time!");

const u8 *MP_Sim_ClassName(void)
{
    return sText_SimClass;
}

void MP_Sim_TrainerName(u8 *out)
{
    if (sSim.current < 0 || sSim.current >= sSim.count)
    {
        out[0] = EOS;
        return;
    }
    NameOf(&sSim.rec[sSim.current], out);
}

const u8 *MP_Sim_LoseText(void)
{
    return sText_SimLose;
}

const u8 *MP_Sim_WinText(void)
{
    return sText_SimWin;
}
