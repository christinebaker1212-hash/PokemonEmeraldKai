// Moemon Co-op: team up with another player against an NPC trainer.
//
// When an NPC trainer challenges a player, that player is marked CHALLENGED.
// Another player who walks up to them (stands next to them, facing them)
// asks to join. Once the challenged player has read the trainer's speech,
// they get an unskippable "X wants to join for a Double Battle! Is this OK?"
// question. Trainers with a single Pokémon refuse ("Let's go one on one!").
//
// The battle itself is the game's own "two link players vs. computer trainer"
// mode (BATTLE_TYPE_TOWER_LINK_MULTI) carried over the network link in
// mp_battle.c. The challenged player's game is the link master: it builds
// the trainer's party, runs the trainer's AI and the battle scripts. Each
// player brings up to three Pokémon and controls their own side.
//
// Afterwards the master sends the partner the final state of their Pokémon
// (experience, moves learned, HP), the prize money and which of them leveled
// up. Each game then runs its own evolutions. Moves a Pokémon could not learn
// because it already knew four are offered after the battle through the Move
// Reminder screen.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "map_name_popup.h"
#include "platform/gamepad.h"
#include "battle.h"
#include "battle_anim.h"
#include "link.h"
#include "party_menu.h"
#include "util.h"
#include "battle_interface.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "battle_pyramid.h"
#include "data.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "evolution_scene.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "pokedex.h"
#include "load_save.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "money.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "trainer_hill.h"
#include "trainer_see.h"
#include "constants/battle.h"
#include "constants/battle_setup.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/flags.h"

void MPNet_Log(const char *fmt, ...);
extern const u8 EventScript_MpCoopLearnMove[];

bool8 gMpCoopBattle;
bool8 gMpCoopWild;

enum {
    MPR_COOP_REQUEST = MPR_COOP_FIRST,  // asker -> challenged: may I join?
    MPR_COOP_CANCEL,                    // asker -> challenged: never mind
    MPR_COOP_ACCEPT,                    // challenged -> asker: u16 trainer, u8 mode, lose text
    MPR_COOP_DECLINE,                   // challenged -> asker: u8 reason
    MPR_COOP_RESULT,                    // master -> partner after the battle
    MPR_COOP_WILD_ASK,                  // wild encounter -> closest friend in the grass: can you join?
    MPR_COOP_WILD_OK,                   // yes (they stop and wait)
    MPR_COOP_WILD_NO,                   // can't right now
    MPR_COOP_WILD_GO,                   // the battle starts
    MPR_COOP_WILD_CANCEL,               // too slow: battling alone after all
};

#define WILD_RANGE_X     8     // about one screen
#define WILD_RANGE_Y     6
#define WILD_ASK_FRAMES  45    // wait this long for an answer, then battle alone
#define WILD_GO_FRAMES   180   // a friend who said yes waits this long for the battle

enum {
    CAUGHT_NONE,
    CAUGHT_MINE,
    CAUGHT_PARTNER,
};

enum {
    DECLINE_NO,
    DECLINE_ONE_MON,
    DECLINE_TOO_LATE,
    DECLINE_BUSY,
};

enum {
    ROLE_NONE,
    ROLE_ASKING,
};

#define MAX_LEARN 8
#define RESULT_WAIT_FRAMES (60 * 15)
#define LOSE_TEXT_MAX 240
#define MODE_WILD 0xFE   // sCoop.mode of a co-op wild double

struct LearnLater
{
    u8 slot;
    u16 move;
};

const u8 *MP_FindTrainerEndScript(u16 trainerId);   // battle_setup.c
static const u8 *sPendingScript;   // a gym leader's badge script to run for the partner

// The badge a gym leader gives (0: not a gym leader)
static u16 GymBadgeFlag(u16 trainerId)
{
    switch (trainerId)
    {
    case TRAINER_ROXANNE_1:       return FLAG_BADGE01_GET;
    case TRAINER_BRAWLY_1:        return FLAG_BADGE02_GET;
    case TRAINER_WATTSON_1:       return FLAG_BADGE03_GET;
    case TRAINER_FLANNERY_1:      return FLAG_BADGE04_GET;
    case TRAINER_NORMAN_1:        return FLAG_BADGE05_GET;
    case TRAINER_WINONA_1:        return FLAG_BADGE06_GET;
    case TRAINER_TATE_AND_LIZA_1: return FLAG_BADGE07_GET;
    case TRAINER_JUAN_1:          return FLAG_BADGE08_GET;
    }
    return 0;
}

static struct
{
    // Challenged player
    bool8 challenged;
    u16 trainerId;
    u8 mode;
    s8 requester;          // player asking to join, -1 = nobody
    bool8 prompting;
    bool8 accepted;
    // Asking player
    u8 role;
    u8 target;
    u8 targetName[MP_NAME_LEN + 1];
    u16 timer;
    s8 declinedBy;         // don't ask this player again until their challenge ends
    // Battle
    bool8 isMaster;
    u8 partner;
    u8 localCount;
    u8 origIndex[MULTI_PARTY_SIZE];
    void (*afterBattle)(void);
    u32 money;
    u8 outcome;
    u8 partnerLeveled;     // master: partner's level-up bits, partner: from the result
    bool8 gotResult;
    u16 resultWait;
    struct Pokemon resultMons[MULTI_PARTY_SIZE];
    struct LearnLater learn[MAX_LEARN];
    u8 learnCount;
    u8 evoQueue;           // party slots (original order) to try evolving
    bool8 learnPromptPending;
    bool8 haveLoseText;
    u8 loseText[LOSE_TEXT_MAX + 1];
    // Co-op wild doubles
    bool8 wildAsking;      // we met a wild Pokémon and asked a friend
    u8 wildTarget;
    u8 wildReply;          // 0 = none yet, MPR_COOP_WILD_OK / _NO
    u16 wildTimer;
    s8 wildFrom;           // the friend whose battle we're waiting to join, -1 = nobody
    struct Pokemon wildMon;   // the one wild Pokémon both players battle
    u8 caught;             // CAUGHT_*
    struct Pokemon caughtMon;
} sCoop = { .requester = -1, .declinedBy = -1, .wildFrom = -1 };

static EWRAM_DATA u8 sText[160];

static const u8 sText_JoinPrompt[] = _("{STR_VAR_1} wants to join for a\nDouble Battle! Is this OK?");
static const u8 sText_OneMon[] = _("Hey, I only have one POKéMON!\nLet's go one on one!");
static const u8 sText_Asking[] = _("Asking {STR_VAR_1} to team up for a\nDouble Battle… (B: never mind)");
static const u8 sText_SaidNo[] = _("{STR_VAR_1} wants to battle alone\nthis time.");
static const u8 sText_TheyHaveOne[] = _("{STR_VAR_1}'s opponent only has one\nPOKéMON. It's a one-on-one battle!");
static const u8 sText_TooLate[] = _("Too late! {STR_VAR_1}'s battle has\nalready started.");
static const u8 sText_Busy[] = _("{STR_VAR_1} can't team up right now.");
static const u8 sText_Left[] = _("{STR_VAR_1} left the game.");
static const u8 sText_LearnLater[] = _("During the battle, {STR_VAR_1} wanted to\nlearn {STR_VAR_2}, but it already knows\pfour moves. Teach it a new move now?");

// ---------------------------------------------------------------------------
// Helpers

static void SetNameVar(u8 id)
{
    // A player who just left has no name any more; use the one saved when asking
    if (!gMpPlayers[id].present && sCoop.target == id && sCoop.targetName[0] != EOS)
        StringCopy(gStringVar1, sCoop.targetName);
    else
        StringCopyN(gStringVar1, gMpPlayers[id].name, MP_NAME_LEN);
    gStringVar1[MP_NAME_LEN] = EOS;
}

static void Send(u8 to, const u8 *data, u16 len)
{
    MP_SendRelay(to, data, len);
}

static void SendByte(u8 to, u8 id, u8 value)
{
    u8 buf[2] = {id, value};
    Send(to, buf, 2);
}

static bool8 IsUsableMon(struct Pokemon *mon)
{
    u32 species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
    return species != SPECIES_NONE && species != SPECIES_EGG && GetMonData(mon, MON_DATA_HP) != 0;
}

static bool8 HasUsableMon(void)
{
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (IsUsableMon(&gPlayerParty[i]))
            return TRUE;
    }
    return FALSE;
}

static bool8 PlayerValid(s8 id)
{
    return id >= 0 && id < MP_MAX_PLAYERS && id != gMpLocalId && gMpPlayers[id].present;
}

bool8 MP_Coop_IsChallenged(void)
{
    return sCoop.challenged;
}

bool8 MP_Coop_IsBusy(void)
{
    return sCoop.role != ROLE_NONE || sCoop.prompting || gMpCoopBattle || sCoop.wildAsking || sCoop.wildFrom >= 0;
}

bool8 MP_Coop_IsMaster(void)
{
    return sCoop.isMaster;
}

u16 MP_Coop_TrainerPic(void)
{
    return gTrainers[gTrainerBattleOpponent_A].trainerPic;
}

u16 MP_Coop_PartnerBackPicOr(u16 defaultPic)
{
    u16 pic = MP_Avatar_RemoteBackPic(sCoop.partner);
    return pic == 0xFFFF ? defaultPic : pic;
}

bool8 MP_Coop_PartnerHidesBack(void)
{
    return MP_Avatar_RemoteHidesBack(sCoop.partner);
}

const u8 *MP_Coop_LoseText(void)
{
    if (gMpCoopBattle && !sCoop.isMaster && sCoop.haveLoseText)
        return sCoop.loseText;
    return NULL;
}

u16 MP_Coop_VictorySong(void)
{
    if (gMpCoopWild)
        return MUS_VICTORY_WILD;
    switch (gTrainers[gTrainerBattleOpponent_A].trainerClass)
    {
    case TRAINER_CLASS_ELITE_FOUR:
    case TRAINER_CLASS_CHAMPION:
        return MUS_VICTORY_LEAGUE;
    case TRAINER_CLASS_TEAM_AQUA:
    case TRAINER_CLASS_TEAM_MAGMA:
    case TRAINER_CLASS_AQUA_ADMIN:
    case TRAINER_CLASS_AQUA_LEADER:
    case TRAINER_CLASS_MAGMA_ADMIN:
    case TRAINER_CLASS_MAGMA_LEADER:
        return MUS_VICTORY_AQUA_MAGMA;
    case TRAINER_CLASS_LEADER:
        return MUS_VICTORY_GYM_LEADER;
    default:
        return MUS_VICTORY_TRAINER;
    }
}

// ---------------------------------------------------------------------------
// The challenged player

// Runs after every trainerbattle setup on this game
void MP_Coop_OnTrainerConfigured(void)
{
    u16 mode;

    if (sCoop.role != ROLE_NONE || gMpCoopBattle)
        return;
    sCoop.challenged = FALSE;
    sCoop.accepted = FALSE;
    sCoop.requester = -1;
    if (!MP_IsConnected() || gApproachingTrainerId != 0)
        return;
    mode = GetTrainerBattleMode();
    switch (mode)
    {
    case TRAINER_BATTLE_SINGLE:
    case TRAINER_BATTLE_CONTINUE_SCRIPT:
    case TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC:
    case TRAINER_BATTLE_DOUBLE:
    case TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE:
    case TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE_NO_MUSIC:
        break;
    default:
        return;
    }
    if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE || InTrainerHillChallenge())
        return;
    if (GetTrainerFlag())
        return; // already beaten: no battle will happen
    sCoop.challenged = TRUE;
    sCoop.trainerId = gTrainerBattleOpponent_A;
    sCoop.mode = mode;
}

static void EndChallenge(u8 reason)
{
    if (PlayerValid(sCoop.requester))
        SendByte(sCoop.requester, MPR_COOP_DECLINE, reason);
    sCoop.challenged = FALSE;
    sCoop.requester = -1;
    sCoop.accepted = FALSE;
}

#define tState data[0]

enum {
    JOIN_START,
    JOIN_PRINT,
    JOIN_ASK,
    JOIN_ONE_MON,
};

static void FinishJoinCheck(u8 taskId)
{
    sCoop.prompting = FALSE;
    gTextFlags.canABSpeedUpPrint = FALSE;
    ScriptContext_Enable();
    DestroyTask(taskId);
}

static void Task_JoinCheck(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (tState)
    {
    case JOIN_START:
        if (!sCoop.challenged || !PlayerValid(sCoop.requester) || gNoOfApproachingTrainers == 2)
        {
            FinishJoinCheck(taskId);
            break;
        }
        if (gTrainers[sCoop.trainerId].partySize <= 1)
        {
            // "Hey, I only have one POKéMON!"
            SendByte(sCoop.requester, MPR_COOP_DECLINE, DECLINE_ONE_MON);
            sCoop.requester = -1;
            HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
            MP_ShowFieldMessageFitted(sText_OneMon);
            tState = JOIN_ONE_MON;
            break;
        }
        SetNameVar(sCoop.requester);
        Gamepad_Rumble(40, 200);
        StringExpandPlaceholders(sText, sText_JoinPrompt);
        HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
        MP_ShowFieldMessageFitted(sText);
        sCoop.prompting = TRUE;
        tState = JOIN_PRINT;
        // fall through
    case JOIN_PRINT:
        // This message can't be sped up
        gTextFlags.canABSpeedUpPrint = FALSE;
        if (!IsTextPrinterActive(0))
        {
            DisplayYesNoMenuDefaultYes();
            tState = JOIN_ASK;
        }
        break;
    case JOIN_ASK:
    {
        s8 answer = Menu_ProcessInputNoWrapClearOnChoose();
        if (answer == MENU_NOTHING_CHOSEN)
            break;
        if (PlayerValid(sCoop.requester))
        {
            if (answer == 0)
            {
                u8 buf[4 + LOSE_TEXT_MAX + 1];
                const u8 *lose = GetTrainerALoseText();
                u16 len = 0;
                buf[0] = MPR_COOP_ACCEPT;
                buf[1] = sCoop.trainerId & 0xFF;
                buf[2] = sCoop.trainerId >> 8;
                buf[3] = sCoop.mode;
                if (lose != NULL)
                {
                    while (len < LOSE_TEXT_MAX && lose[len] != EOS)
                    {
                        buf[4 + len] = lose[len];
                        len++;
                    }
                }
                buf[4 + len] = EOS;
                Send(sCoop.requester, buf, 5 + len);
                sCoop.accepted = TRUE;
                sCoop.partner = sCoop.requester;
                MPNet_Log("co-op: player %d joins the battle against trainer %d", sCoop.partner + 1, sCoop.trainerId);
            }
            else
            {
                SendByte(sCoop.requester, MPR_COOP_DECLINE, DECLINE_NO);
                sCoop.requester = -1;
            }
        }
        FinishJoinCheck(taskId);
        break;
    }
    case JOIN_ONE_MON:
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
            FinishJoinCheck(taskId);
        break;
    }
}

// Special, after the trainer's intro speech (data/scripts/trainer_battle.inc).
// The script waits (waitstate) until the task is done.
void MP_Coop_JoinCheck(void)
{
    CreateTask(Task_JoinCheck, 80);
}

// ---------------------------------------------------------------------------
// Battle setup (both games)

static void ArrangeLocalParty(void)
{
    struct Pokemon *tmp = Alloc(sizeof(struct Pokemon) * PARTY_SIZE);
    u8 i, n = 0;

    SavePlayerParty(); // LoadPlayerParty brings the real party back afterwards
    memcpy(tmp, gPlayerParty, sizeof(struct Pokemon) * PARTY_SIZE);
    for (i = 0; i < MULTI_PARTY_SIZE; i++)
        sCoop.origIndex[i] = 0xFF;
    for (i = 0; i < PARTY_SIZE && n < MULTI_PARTY_SIZE; i++)
    {
        if (IsUsableMon(&tmp[i]))
        {
            gPlayerParty[n] = tmp[i];
            sCoop.origIndex[n] = i;
            n++;
        }
    }
    for (i = n; i < PARTY_SIZE; i++)
        ZeroMonData(&gPlayerParty[i]);
    sCoop.localCount = n;
    Free(tmp);
}

static void BeginCoopBattle(bool8 isMaster, u8 partner)
{
    sCoop.caught = CAUGHT_NONE;
    sCoop.isMaster = isMaster;
    sCoop.partner = partner;
    sCoop.money = 0;
    sCoop.learnCount = 0;
    sCoop.partnerLeveled = 0;
    sCoop.gotResult = FALSE;
    sCoop.resultWait = 0;
    sCoop.evoQueue = 0;
    ArrangeLocalParty();
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_LINK
                     | BATTLE_TYPE_MULTI | BATTLE_TYPE_TOWER_LINK_MULTI;
    gMpCoopBattle = TRUE;
    gLinkType = LINKTYPE_BATTLE;
    MP_Battle_OpenLink(partner, isMaster);
}

// BattleSetup_StartTrainerBattle on the challenged player's game, just before
// the battle transition starts
void MP_Coop_OnStartTrainerBattle(void)
{
    bool8 go = sCoop.accepted && PlayerValid(sCoop.partner) && sCoop.trainerId == gTrainerBattleOpponent_A
            && gBattleTypeFlags == BATTLE_TYPE_TRAINER && !gMpCoopBattle;

    sCoop.challenged = FALSE;
    sCoop.accepted = FALSE;
    sCoop.requester = -1;
    if (!go)
        return;
    BeginCoopBattle(TRUE, sCoop.partner);
    sCoop.haveLoseText = FALSE;
    sCoop.afterBattle = gMain.savedCallback;
    gMain.savedCallback = CB2_MpCoopEndBattle;
}

// The trainer's party is interleaved: the master's game puts the first,
// third and fifth Pokémon on the left opponent and the others on the right,
// so the trainer leads with their first two and sends the rest in order.
void MP_Coop_ArrangeEnemyParty(void)
{
    static const u8 sSlot[PARTY_SIZE] = {0, 3, 1, 4, 2, 5};
    struct Pokemon *tmp = Alloc(sizeof(struct Pokemon) * PARTY_SIZE);
    u8 i;

    memcpy(tmp, gEnemyParty, sizeof(struct Pokemon) * PARTY_SIZE);
    for (i = 0; i < PARTY_SIZE; i++)
        gEnemyParty[sSlot[i]] = tmp[i];
    Free(tmp);
}

void MP_Coop_RecordLearnLater(u8 partyIndex, u16 move)
{
    u8 i;
    for (i = 0; i < sCoop.learnCount; i++)
    {
        if (sCoop.learn[i].slot == partyIndex && sCoop.learn[i].move == move)
            return;
    }
    if (sCoop.learnCount < MAX_LEARN)
    {
        sCoop.learn[sCoop.learnCount].slot = partyIndex;
        sCoop.learn[sCoop.learnCount].move = move;
        sCoop.learnCount++;
    }
}

void MP_Coop_RecordMoney(u32 amount)
{
    sCoop.money = amount;
}

// Pokémon 0-2 belong to the master, 3-5 to the partner
bool8 MP_Coop_IsTradedMon(u8 partyIndex)
{
    u8 owner = partyIndex < MULTI_PARTY_SIZE ? 0 : 1;
    return GetMonData(&gPlayerParty[partyIndex], MON_DATA_OT_ID) != gLinkPlayers[owner].trainerId;
}

// The other game's controller animated this experience gain; apply the same
// change to our copy so both games agree on levels and stats.
void MP_Coop_MirrorExp(u8 battler, u8 monId, s16 gainedExp)
{
    struct Pokemon *mon;
    u16 species;
    u8 level;
    u32 currExp, nextLvlExp;
    u8 b;

    if (monId >= PARTY_SIZE)
        return;
    mon = &gPlayerParty[monId];
    species = GetMonData(mon, MON_DATA_SPECIES);
    level = GetMonData(mon, MON_DATA_LEVEL);
    if (species == SPECIES_NONE || level >= MAX_LEVEL)
        return;
    currExp = GetMonData(mon, MON_DATA_EXP);
    nextLvlExp = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1];
    if (currExp + gainedExp >= nextLvlExp)
    {
        SetMonData(mon, MON_DATA_EXP, &nextLvlExp);
        CalculateMonStats(mon);
    }
    else
    {
        currExp += gainedExp;
        SetMonData(mon, MON_DATA_EXP, &currExp);
    }
    for (b = 0; b < gBattlersCount; b++)
    {
        if (GetBattlerSide(b) == B_SIDE_PLAYER && gBattlerPartyIndexes[b] == monId && !(gAbsentBattlerFlags & gBitTable[b]))
            UpdateHealthboxAttribute(gHealthboxSpriteIds[b], mon, HEALTHBOX_ALL);
    }
}

// ---------------------------------------------------------------------------
// After the battle

static void CB2_MpCoopEvolve(void);
static void CB2_MpCoopFinish(void);

// Puts our (up to three) Pokémon back into the real party
static void RestoreParty(const struct Pokemon *finalMons, u8 leveledBits)
{
    u8 i;

    LoadPlayerParty();
    sCoop.evoQueue = 0;
    for (i = 0; i < sCoop.localCount; i++)
    {
        u8 slot = sCoop.origIndex[i];
        if (slot >= PARTY_SIZE)
            continue;
        if (GetMonData((struct Pokemon *)&finalMons[i], MON_DATA_SPECIES) != SPECIES_NONE)
            gPlayerParty[slot] = finalMons[i];
        if (leveledBits & (1 << i))
            sCoop.evoQueue |= 1 << slot;
    }
    CalculatePlayerPartyCount();
}

// Learn-later entries use battle party slots; turn ours into real party slots
static void KeepOwnLearnEntries(u8 base)
{
    u8 i, n = 0;
    for (i = 0; i < sCoop.learnCount; i++)
    {
        u8 s = sCoop.learn[i].slot;
        if (s >= base && s < base + MULTI_PARTY_SIZE && sCoop.origIndex[s - base] < PARTY_SIZE)
        {
            sCoop.learn[n].slot = sCoop.origIndex[s - base];
            sCoop.learn[n].move = sCoop.learn[i].move;
            n++;
        }
    }
    sCoop.learnCount = n;
}

static void SendResult(void)
{
    // id, outcome, money(4), leveled, learnCount, learn entries (3 bytes), 3 Pokémon
    // then u8 caughtForPartner + 1 Pokémon (co-op wild doubles)
    u8 *buf = Alloc(8 + MAX_LEARN * 3 + sizeof(struct Pokemon) * (MULTI_PARTY_SIZE + 1) + 1);
    u16 len = 0;
    u8 i, count = 0;

    buf[len++] = MPR_COOP_RESULT;
    buf[len++] = sCoop.outcome;
    buf[len++] = sCoop.money & 0xFF;
    buf[len++] = (sCoop.money >> 8) & 0xFF;
    buf[len++] = (sCoop.money >> 16) & 0xFF;
    buf[len++] = (sCoop.money >> 24) & 0xFF;
    buf[len++] = sCoop.partnerLeveled;
    for (i = 0; i < sCoop.learnCount; i++)
    {
        if (sCoop.learn[i].slot >= MULTI_PARTY_SIZE)
            count++;
    }
    buf[len++] = count;
    for (i = 0; i < sCoop.learnCount; i++)
    {
        if (sCoop.learn[i].slot >= MULTI_PARTY_SIZE)
        {
            buf[len++] = sCoop.learn[i].slot;
            buf[len++] = sCoop.learn[i].move & 0xFF;
            buf[len++] = sCoop.learn[i].move >> 8;
        }
    }
    memcpy(buf + len, &gPlayerParty[MULTI_PARTY_SIZE], sizeof(struct Pokemon) * MULTI_PARTY_SIZE);
    len += sizeof(struct Pokemon) * MULTI_PARTY_SIZE;
    if (sCoop.caught == CAUGHT_PARTNER)
    {
        buf[len++] = 1;
        memcpy(buf + len, &sCoop.caughtMon, sizeof(struct Pokemon));
        len += sizeof(struct Pokemon);
    }
    if (PlayerValid(sCoop.partner))
        Send(sCoop.partner, buf, len);
    Free(buf);
}

static void FinishLink(void)
{
    gMpCoopBattle = FALSE;
    MP_Battle_CloseLink();
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
}

static void CB2_MpCoopWaitResult(void)
{
    if (!sCoop.gotResult && ++sCoop.resultWait < RESULT_WAIT_FRAMES && PlayerValid(sCoop.partner))
        return;
    if (sCoop.gotResult)
    {
        RestoreParty(sCoop.resultMons, sCoop.partnerLeveled);
    }
    else
    {
        // No result (partner left): keep what our own game saw
        struct Pokemon *mons = Alloc(sizeof(struct Pokemon) * MULTI_PARTY_SIZE);
        MPNet_Log("co-op: no battle result from the partner, keeping local data");
        memcpy(mons, &gPlayerParty[MULTI_PARTY_SIZE], sizeof(struct Pokemon) * MULTI_PARTY_SIZE);
        sCoop.learnCount = 0;
        RestoreParty(mons, 0);
        Free(mons);
    }
    FinishLink();
    SetMainCallback2(CB2_MpCoopEvolve);
}

// gMain.savedCallback of a co-op battle on both games
void CB2_MpCoopEndBattle(void)
{
    sCoop.outcome = gBattleOutcome & ~B_OUTCOME_LINK_BATTLE_RAN;
    if (sCoop.isMaster)
    {
        struct Pokemon *mons = Alloc(sizeof(struct Pokemon) * MULTI_PARTY_SIZE);
        u8 own = gLeveledUpInBattle & 7;

        sCoop.partnerLeveled = (gLeveledUpInBattle >> MULTI_PARTY_SIZE) & 7;
        gLeveledUpInBattle = 0;
        SendResult();
        memcpy(mons, gPlayerParty, sizeof(struct Pokemon) * MULTI_PARTY_SIZE);
        KeepOwnLearnEntries(0);
        RestoreParty(mons, sCoop.outcome == B_OUTCOME_WON ? own : 0);
        Free(mons);
        FinishLink();
        SetMainCallback2(CB2_MpCoopEvolve);
    }
    else
    {
        sCoop.resultWait = 0;
        SetMainCallback2(CB2_MpCoopWaitResult);
    }
}

static void CB2_MpCoopEvolve(void)
{
    while (sCoop.evoQueue != 0)
    {
        u8 i;
        for (i = 0; i < PARTY_SIZE; i++)
        {
            if (sCoop.evoQueue & (1 << i))
            {
                u16 species;
                sCoop.evoQueue &= ~(1 << i);
                species = GetEvolutionTargetSpecies(&gPlayerParty[i], EVO_MODE_NORMAL, ITEM_NONE);
                if (species != SPECIES_NONE)
                {
                    gCB2_AfterEvolution = CB2_MpCoopEvolve;
                    EvolutionScene(&gPlayerParty[i], species, TRUE, i);
                    return;
                }
                break;
            }
        }
    }
    CB2_MpCoopFinish();
}

static void GiveCaughtMon(void)
{
    char msg[96], nick[16];
    u8 nickGame[POKEMON_NAME_LENGTH + 1];
    u16 species = GetMonData(&sCoop.caughtMon, MON_DATA_SPECIES);
    u16 nat = SpeciesToNationalPokedexNum(species);
    u8 result;

    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return;
    GetMonData(&sCoop.caughtMon, MON_DATA_NICKNAME, nickGame);
    nickGame[POKEMON_NAME_LENGTH] = EOS;
    MP_GameToAscii(nick, nickGame, POKEMON_NAME_LENGTH);
    result = GiveMonToPlayer(&sCoop.caughtMon);
    GetSetPokedexFlag(nat, FLAG_SET_SEEN);
    GetSetPokedexFlag(nat, FLAG_SET_CAUGHT);
    if (result == MON_GIVEN_TO_PARTY)
        snprintf(msg, sizeof(msg), "Gotcha! %s\njoined your party!", nick);
    else if (result == MON_GIVEN_TO_PC)
        snprintf(msg, sizeof(msg), "Gotcha! %s\nwas sent to the PC.", nick);
    else
        snprintf(msg, sizeof(msg), "No room for %s\nin the PC...", nick);
    MP_Events_QueueToast(msg);
    MPNet_Log("co-op wild: caught species %d (%s)", species, result == MON_CANT_GIVE ? "no room" : "given");
}

// After a co-op wild double: the catch goes to whoever threw the ball, and
// each catch or win is counted once for the team
static void FinishWild(void)
{
    u8 level = GetMonData(&sCoop.wildMon, MON_DATA_LEVEL);
    if (sCoop.outcome == B_OUTCOME_CAUGHT && sCoop.caught == CAUGHT_MINE)
    {
        level = GetMonData(&sCoop.caughtMon, MON_DATA_LEVEL);
        GiveCaughtMon();
    }
    if (sCoop.isMaster)
    {
        // CB2_EndWildBattle counts the battle for the leader: not the partner's catch
        if (sCoop.outcome == B_OUTCOME_CAUGHT && sCoop.caught != CAUGHT_MINE)
            gBattleOutcome = 0;
    }
    else if (sCoop.outcome == B_OUTCOME_CAUGHT && sCoop.caught == CAUGHT_MINE)
    {
        MP_Events_OnWildEnd(B_OUTCOME_CAUGHT, level);
    }
    sCoop.caught = CAUGHT_NONE;
    gMpCoopWild = FALSE;
}

static bool8 IsLoss(u8 outcome)
{
    return outcome == B_OUTCOME_LOST || outcome == B_OUTCOME_DREW || outcome == B_OUTCOME_FORFEITED;
}

static void CB2_MpCoopFinish(void)
{
    MPNet_Log("co-op: battle over, outcome %d, prize %u, lead Lv%d %u exp, money %u, %d move(s) to learn",
              sCoop.outcome, (unsigned)sCoop.money, GetMonData(&gPlayerParty[0], MON_DATA_LEVEL),
              (unsigned)GetMonData(&gPlayerParty[0], MON_DATA_EXP), (unsigned)GetMoney(&gSaveBlock1Ptr->money), sCoop.learnCount);
    gSpecialVar_Result = sCoop.outcome;
    gBattleOutcome = sCoop.outcome;
    if (sCoop.outcome == B_OUTCOME_WON)
    {
        if (sCoop.isMaster)
            MP_Events_OnCoopWin(sCoop.partner); // counted once, by the battle leader
        else if (!gMpCoopWild)
            MP_Events_OnTrainerWin(); // the leader's game counts it in CB2_EndTrainerBattle
    }
    if (gMpCoopWild)
        FinishWild();
    if (sCoop.learnCount != 0 && !IsLoss(sCoop.outcome))
        sCoop.learnPromptPending = TRUE;
    else
        sCoop.learnCount = 0;

    if (sCoop.isMaster)
    {
        SetMainCallback2(sCoop.afterBattle != NULL ? sCoop.afterBattle : CB2_ReturnToField);
        return;
    }

    if (IsLoss(sCoop.outcome))
    {
        SetMainCallback2(CB2_WhiteOut);
        return;
    }
    if (sCoop.outcome == B_OUTCOME_WON && sCoop.mode != MODE_WILD)
    {
        AddMoney(&gSaveBlock1Ptr->money, sCoop.money);
        // Ordinary trainers count as beaten for the partner too. Story battles
        // (gym leaders, rivals...) keep their own event scripts, so the partner
        // can still battle them on their own later.
        if (sCoop.mode == TRAINER_BATTLE_SINGLE || sCoop.mode == TRAINER_BATTLE_DOUBLE)
            FlagSet(TRAINER_FLAGS_START + sCoop.trainerId);
        // A team rematch with a gym leader: the partner gets the reward too
        else if (MP_Rematch_IsRematchTrainer(sCoop.trainerId))
            MP_Rematch_Reward();
        // Gym leaders: the partner gets the badge too. Their own copy of the
        // leader's "you won" script runs once back on the field.
        else if (GymBadgeFlag(sCoop.trainerId) && !FlagGet(GymBadgeFlag(sCoop.trainerId)))
        {
            sPendingScript = MP_FindTrainerEndScript(sCoop.trainerId);
            if (sPendingScript)
                FlagSet(TRAINER_FLAGS_START + sCoop.trainerId);
        }
    }
    Overworld_ResetMapMusic();
    SetMainCallback2(CB2_ReturnToField);
}

// Runs the gym leader's badge script for the partner (from MP_Coop_Update)
static void TryRunPendingScript(void)
{
    if (sPendingScript == NULL || !MP_IsLocalFree() || ScriptContext_IsEnabled())
        return;
    LockPlayerFieldControls();
    ScriptContext_SetupScript(sPendingScript);
    sPendingScript = NULL;
}

void MP_Coop_OnBattleAborted(void)
{
    gMpCoopBattle = FALSE;
    gMpCoopWild = FALSE;
    sCoop.caught = CAUGHT_NONE;
    sCoop.role = ROLE_NONE;
    sCoop.challenged = FALSE;
    sCoop.accepted = FALSE;
    sCoop.requester = -1;
    sCoop.learnCount = 0;
}

// Special: next Pokémon that missed a move. VAR_RESULT = FALSE when none left.
void MP_Coop_PrepareLearnMove(void)
{
    while (sCoop.learnCount > 0)
    {
        struct LearnLater e = sCoop.learn[0];
        u8 i;
        for (i = 1; i < sCoop.learnCount; i++)
            sCoop.learn[i - 1] = sCoop.learn[i];
        sCoop.learnCount--;
        if (e.slot >= PARTY_SIZE || GetMonData(&gPlayerParty[e.slot], MON_DATA_SPECIES_OR_EGG) == SPECIES_NONE)
            continue;
        if (MonKnowsMove(&gPlayerParty[e.slot], e.move))
            continue;
        GetMonNickname(&gPlayerParty[e.slot], gStringVar1);
        StringCopy(gStringVar2, gMoveNames[e.move]);
        StringExpandPlaceholders(gStringVar4, sText_LearnLater);
        gSpecialVar_0x8004 = e.slot;
        gSpecialVar_Result = TRUE;
        return;
    }
    gSpecialVar_Result = FALSE;
}

// ---------------------------------------------------------------------------
// The player who walks up

static void Task_AskerMessage(u8 taskId);

static void ShowAskerMessage(const u8 *text, u8 who)
{
    u8 taskId;
    SetNameVar(who);
    StringExpandPlaceholders(sText, text);
    taskId = FindTaskIdByFunc(Task_AskerMessage);
    if (taskId == TASK_NONE)
        taskId = CreateTask(Task_AskerMessage, 80);
    if (!IsFieldMessageBoxHidden() || IsTextPrinterActive(0))
        HideFieldMessageBox();
    gTasks[taskId].tState = 0;
}

// Shows sText; closes on A/B and gives the controls back
static void Task_AskerMessage(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (tState)
    {
    case 0:
        HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
        MP_ShowFieldMessageFitted(sText);
        tState++;
        break;
    case 1:
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            HideFieldMessageBox();
            tState++;
        }
        break;
    case 2:
        if (IsFieldMessageBoxHidden())
        {
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
        }
        break;
    }
}

static void StopAsking(void)
{
    sCoop.role = ROLE_NONE;
}

static void Task_StartJoinerBattle(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!gMpCoopBattle)
    {
        // The battle was called off before it started
        UnlockPlayerFieldControls();
        DestroyTask(taskId);
        return;
    }
    switch (tState)
    {
    case 0:
        if (!IsFieldMessageBoxHidden())
            HideFieldMessageBox();
        PlayMapChosenOrBattleBGM(0);
        BattleTransition_StartOnField(B_TRANSITION_POKEBALLS_TRAIL);
        tState++;
        break;
    case 1:
        if (IsBattleTransitionDone())
        {
            CleanupOverworldWindowsAndTilemaps();
            gMain.savedCallback = CB2_MpCoopEndBattle;
            SetMainCallback2(CB2_InitBattle);
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
        }
        break;
    }
}

static void OnAccepted(u8 from, const u8 *data, u16 len)
{
    u16 i;

    StopAsking();
    if (len < 4)
        return;
    sCoop.trainerId = data[1] | (data[2] << 8);
    sCoop.mode = data[3];
    sCoop.haveLoseText = FALSE;
    if (len > 4)
    {
        for (i = 0; i < LOSE_TEXT_MAX && 4 + i < len && data[4 + i] != EOS; i++)
            sCoop.loseText[i] = data[4 + i];
        sCoop.loseText[i] = EOS;
        sCoop.haveLoseText = i > 0;
    }
    if (sCoop.trainerId >= TRAINERS_COUNT)
        return;
    {
        u8 t = FindTaskIdByFunc(Task_AskerMessage);
        if (t != TASK_NONE)
            DestroyTask(t);
    }
    gTrainerBattleOpponent_A = sCoop.trainerId;
    gTrainerBattleOpponent_B = 0;
    BeginCoopBattle(FALSE, from);
    sCoop.afterBattle = NULL;
    CreateTask(Task_StartJoinerBattle, 1);
}

static bool8 TryAsk(void)
{
    struct ObjectEvent *me;
    s16 x, y;
    u8 i;

    if (!MP_IsLocalFree() || !MP_IsConnected() || MP_Trainer_IsEngaged() || MP_Trainer_IsActive())
        return FALSE;
    me = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (me->heldMovementActive && !me->heldMovementFinished)
        return FALSE;
    x = me->currentCoords.x;
    y = me->currentCoords.y;
    MoveCoords(me->facingDirection, &x, &y);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        s16 rx, ry;
        if (!PlayerValid(i) || !(gMpPlayers[i].state.flags & MPF_CHALLENGED) || i == sCoop.declinedBy)
            continue;
        if (gMpPlayers[i].state.mapGroup != gSaveBlock1Ptr->location.mapGroup
         || gMpPlayers[i].state.mapNum != gSaveBlock1Ptr->location.mapNum)
            continue;
        if (!MP_GetRemoteMapPos(i, &rx, &ry) || rx != x || ry != y)
            continue;
        if (!HasUsableMon())
            return FALSE;
        sCoop.role = ROLE_ASKING;
        sCoop.target = i;
        StringCopyN(sCoop.targetName, gMpPlayers[i].name, MP_NAME_LEN);
        sCoop.targetName[MP_NAME_LEN] = EOS;
        sCoop.timer = 0;
        SendByte(i, MPR_COOP_REQUEST, 0);
        LockPlayerFieldControls();
        SetNameVar(i);
        StringExpandPlaceholders(sText, sText_Asking);
        HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
        MP_ShowFieldMessageFitted(sText);
        MPNet_Log("co-op: asking player %d to team up", i + 1);
        return TRUE;
    }
    return FALSE;
}

static void UpdateAsking(void)
{
    u8 target = sCoop.target;

    sCoop.timer++;
    if (!PlayerValid(target))
    {
        StopAsking();
        ShowAskerMessage(sText_Left, target);
        return;
    }
    if (!IsTextPrinterActive(0) && JOY_NEW(B_BUTTON))
    {
        SendByte(target, MPR_COOP_CANCEL, 0);
        StopAsking();
        sCoop.declinedBy = target;
        HideFieldMessageBox();
        UnlockPlayerFieldControls();
        return;
    }
    // Their challenge ended without an answer
    if (sCoop.timer > 90 && !(gMpPlayers[target].state.flags & MPF_CHALLENGED))
    {
        StopAsking();
        sCoop.declinedBy = target;
        ShowAskerMessage(sText_TooLate, target);
    }
}

// ---------------------------------------------------------------------------
// Co-op wild doubles: meet a wild Pokémon while a friend is close by in the
// grass and they jump in. No questions asked: it just happens, as long as
// they're free (not in a menu, a battle, a trade...).

static bool8 OnLandEncounterTile(s16 x, s16 y)
{
    return MetatileBehavior_IsLandWildEncounter(MapGridGetMetatileBehaviorAt(x, y));
}

static bool8 FriendCanJoinWild(u8 i, s16 myX, s16 myY, u16 *distance)
{
    const u16 needed = MPF_IN_OVERWORLD | MPF_FREE;
    const u16 busy = MPF_IN_BATTLE | MPF_ENGAGED | MPF_TRAINER_MODE | MPF_CHALLENGED;
    s16 rx, ry, dx, dy;

    if (!PlayerValid(i) || (gMpPlayers[i].state.flags & needed) != needed || (gMpPlayers[i].state.flags & busy))
        return FALSE;
    if (gMpPlayers[i].state.mapGroup != gSaveBlock1Ptr->location.mapGroup
     || gMpPlayers[i].state.mapNum != gSaveBlock1Ptr->location.mapNum)
        return FALSE;
    if (!MP_GetRemoteMapPos(i, &rx, &ry) || !OnLandEncounterTile(rx, ry))
        return FALSE;
    dx = rx > myX ? rx - myX : myX - rx;
    dy = ry > myY ? ry - myY : myY - ry;
    if (dx > WILD_RANGE_X || dy > WILD_RANGE_Y)
        return FALSE;
    *distance = dx + dy;
    return TRUE;
}

// BattleSetup_StartWildBattle. gEnemyParty[0] holds the wild Pokémon we met.
// TRUE: we're asking the closest friend in the grass, and the battle (co-op
// or alone) starts in a moment.
bool8 MP_Coop_TryWildTeamUp(void)
{
    struct ObjectEvent *me;
    u16 best = 0xFFFF;
    s8 target = -1;
    u8 i;

    if (!MP_IsConnected() || gMpLocalId < 0 || gMpCoopBattle || MP_Coop_IsBusy() || sCoop.challenged)
        return FALSE;
    if (gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT || TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING))
        return FALSE;
    if (MP_Trainer_IsActive() || MP_Trainer_IsEngaged() || CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
        return FALSE;
    me = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (!OnLandEncounterTile(me->currentCoords.x, me->currentCoords.y))
        return FALSE;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        u16 d;
        if (FriendCanJoinWild(i, me->currentCoords.x, me->currentCoords.y, &d) && d < best)
        {
            best = d;
            target = i;
        }
    }
    if (target < 0)
        return FALSE;
    sCoop.wildMon = gEnemyParty[0];

    LockPlayerFieldControls();
    FreezeObjectEvents();
    StopPlayerAvatar();
    sCoop.wildAsking = TRUE;
    sCoop.wildTarget = target;
    sCoop.wildReply = 0;
    sCoop.wildTimer = 0;
    SendByte(target, MPR_COOP_WILD_ASK, 0);
    MPNet_Log("co-op wild: asking player %d (distance %d) to jump in", target + 1, best);
    return TRUE;
}

static void StartWildCoop(void)
{
    u8 partner = sCoop.wildTarget;
    gTrainerBattleOpponent_A = 0;
    gTrainerBattleOpponent_B = 0;
    gMpCoopWild = TRUE;
    BeginCoopBattle(TRUE, partner);
    sCoop.mode = MODE_WILD;
    sCoop.trainerId = 0;
    sCoop.haveLoseText = FALSE;
    sCoop.afterBattle = MP_EndWildBattleCallback();
    IncrementGameStat(GAME_STAT_TOTAL_BATTLES);
    IncrementGameStat(GAME_STAT_WILD_BATTLES);
    SendByte(partner, MPR_COOP_WILD_GO, 0);
    CreateTask(Task_StartJoinerBattle, 1);
    MPNet_Log("co-op wild: battle with player %d", partner + 1);
}

static void UpdateWildAsking(void)
{
    sCoop.wildTimer++;
    if (sCoop.wildReply == MPR_COOP_WILD_OK && PlayerValid(sCoop.wildTarget))
    {
        sCoop.wildAsking = FALSE;
        StartWildCoop();
    }
    else if (sCoop.wildReply == MPR_COOP_WILD_NO || sCoop.wildTimer > WILD_ASK_FRAMES || !PlayerValid(sCoop.wildTarget))
    {
        if (sCoop.wildReply != MPR_COOP_WILD_NO && PlayerValid(sCoop.wildTarget))
            SendByte(sCoop.wildTarget, MPR_COOP_WILD_CANCEL, 0);
        sCoop.wildAsking = FALSE;
        gEnemyParty[0] = sCoop.wildMon;
        MPNet_Log("co-op wild: battling alone");
        MP_StartNormalWildBattle();
    }
}

// The master's game, building the battle: one wild Pokémon for both players
void MP_Coop_SetWildEnemyParty(void)
{
    ZeroEnemyPartyMons();
    gEnemyParty[0] = sCoop.wildMon;
}

// Cmd_givecaughtmon in a co-op wild double (the master's game decides)
void MP_Coop_OnWildCaught(u8 throwerPartyIndex, struct Pokemon *mon)
{
    if (!sCoop.isMaster)
        return;
    sCoop.caughtMon = *mon;
    // The master's Pokémon are battle party slots 0-2, the partner's 3-5
    sCoop.caught = throwerPartyIndex < MULTI_PARTY_SIZE ? CAUGHT_MINE : CAUGHT_PARTNER;
}

static bool8 CanJoinWild(void)
{
    return MP_IsLocalFree() && !gMpCoopBattle && sCoop.role == ROLE_NONE && !sCoop.challenged
        && !sCoop.prompting && !sCoop.wildAsking && sCoop.wildFrom < 0 && HasUsableMon()
        && !MP_Trainer_IsEngaged() && !MP_Trainer_IsActive()
        && !TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING);
}

static void StopWaitingForWild(void)
{
    sCoop.wildFrom = -1;
    UnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

static void OnWildRelay(u8 from, u8 id)
{
    switch (id)
    {
    case MPR_COOP_WILD_ASK:
        if (CanJoinWild())
        {
            LockPlayerFieldControls();
            FreezeObjectEvents();
            StopPlayerAvatar();
            sCoop.wildFrom = from;
            sCoop.wildTimer = 0;
            SendByte(from, MPR_COOP_WILD_OK, 0);
        }
        else
        {
            SendByte(from, MPR_COOP_WILD_NO, 0);
        }
        break;
    case MPR_COOP_WILD_OK:
    case MPR_COOP_WILD_NO:
        if (sCoop.wildAsking && from == sCoop.wildTarget && sCoop.wildReply == 0)
            sCoop.wildReply = id;
        else if (id == MPR_COOP_WILD_OK)
            SendByte(from, MPR_COOP_WILD_CANCEL, 0);   // we already gave up
        break;
    case MPR_COOP_WILD_GO:
        if (sCoop.wildFrom == from)
        {
            sCoop.wildFrom = -1;
            gTrainerBattleOpponent_A = 0;
            gTrainerBattleOpponent_B = 0;
            gMpCoopWild = TRUE;
            BeginCoopBattle(FALSE, from);
            sCoop.mode = MODE_WILD;
            sCoop.trainerId = 0;
            sCoop.haveLoseText = FALSE;
            sCoop.afterBattle = NULL;
            IncrementGameStat(GAME_STAT_TOTAL_BATTLES);
            IncrementGameStat(GAME_STAT_WILD_BATTLES);
            CreateTask(Task_StartJoinerBattle, 1);
            MPNet_Log("co-op wild: joining player %d's battle", from + 1);
        }
        break;
    case MPR_COOP_WILD_CANCEL:
        if (sCoop.wildFrom == from)
            StopWaitingForWild();
        break;
    }
}

static void UpdateWildWaiting(void)
{
    if (!PlayerValid(sCoop.wildFrom) || ++sCoop.wildTimer > WILD_GO_FRAMES)
        StopWaitingForWild();
}

// ---------------------------------------------------------------------------

static void TryRunPendingScript(void);

void MP_Coop_Update(void)
{
    TryRunPendingScript();

    if (sCoop.wildAsking)
        UpdateWildAsking();
    if (sCoop.wildFrom >= 0)
        UpdateWildWaiting();

    // Challenge over (the script ended without a battle)
    if (sCoop.challenged && !sCoop.prompting && MP_IsLocalFree())
        EndChallenge(DECLINE_TOO_LATE);

    if (sCoop.declinedBy >= 0
     && (!PlayerValid(sCoop.declinedBy) || !(gMpPlayers[sCoop.declinedBy].state.flags & MPF_CHALLENGED)))
        sCoop.declinedBy = -1;

    if (sCoop.role == ROLE_ASKING)
        UpdateAsking();
    else if (!gMpCoopBattle && !sCoop.challenged)
        TryAsk();

    if (sCoop.learnPromptPending && MP_IsLocalFree())
    {
        sCoop.learnPromptPending = FALSE;
        ScriptContext_SetupScript(EventScript_MpCoopLearnMove);
    }
}

void MP_Coop_OnRelay(u8 from, const u8 *data, u16 len)
{
    switch (data[0])
    {
    case MPR_COOP_REQUEST:
        if (!sCoop.challenged || sCoop.accepted || sCoop.prompting
         || (sCoop.requester >= 0 && sCoop.requester != from && PlayerValid(sCoop.requester)))
            SendByte(from, MPR_COOP_DECLINE, sCoop.challenged ? DECLINE_BUSY : DECLINE_TOO_LATE);
        else
            sCoop.requester = from;
        break;
    case MPR_COOP_CANCEL:
        if (sCoop.requester == from && !sCoop.accepted)
            sCoop.requester = -1;
        break;
    case MPR_COOP_ACCEPT:
        if (sCoop.role == ROLE_ASKING && from == sCoop.target)
            OnAccepted(from, data, len);
        break;
    case MPR_COOP_DECLINE:
        if (sCoop.role == ROLE_ASKING && from == sCoop.target)
        {
            static const u8 *const sReasons[] = {sText_SaidNo, sText_TheyHaveOne, sText_TooLate, sText_Busy};
            u8 reason = len > 1 && data[1] < ARRAY_COUNT(sReasons) ? data[1] : DECLINE_BUSY;
            StopAsking();
            sCoop.declinedBy = from;
            ShowAskerMessage(sReasons[reason], from);
        }
        break;
    case MPR_COOP_WILD_ASK:
    case MPR_COOP_WILD_OK:
    case MPR_COOP_WILD_NO:
    case MPR_COOP_WILD_GO:
    case MPR_COOP_WILD_CANCEL:
        OnWildRelay(from, data[0]);
        break;
    case MPR_COOP_RESULT:
        if (gMpCoopBattle && !sCoop.isMaster && from == sCoop.partner && len >= 8)
        {
            u16 pos = 8;
            u8 i, count = data[7];
            sCoop.outcome = data[1];
            sCoop.money = data[2] | (data[3] << 8) | (data[4] << 16) | ((u32)data[5] << 24);
            sCoop.partnerLeveled = sCoop.outcome == B_OUTCOME_WON ? data[6] : 0;
            sCoop.learnCount = 0;
            for (i = 0; i < count && pos + 3 <= len; i++, pos += 3)
            {
                u8 slot = data[pos];
                u16 move = data[pos + 1] | (data[pos + 2] << 8);
                if (slot >= MULTI_PARTY_SIZE && slot < PARTY_SIZE && move < MOVES_COUNT
                 && sCoop.origIndex[slot - MULTI_PARTY_SIZE] < PARTY_SIZE && sCoop.learnCount < MAX_LEARN)
                {
                    sCoop.learn[sCoop.learnCount].slot = sCoop.origIndex[slot - MULTI_PARTY_SIZE];
                    sCoop.learn[sCoop.learnCount].move = move;
                    sCoop.learnCount++;
                }
            }
            if (pos + sizeof(struct Pokemon) * MULTI_PARTY_SIZE <= len)
            {
                memcpy(sCoop.resultMons, data + pos, sizeof(struct Pokemon) * MULTI_PARTY_SIZE);
                sCoop.gotResult = TRUE;
                pos += sizeof(struct Pokemon) * MULTI_PARTY_SIZE;
                // We caught a wild Pokémon
                if (gMpCoopWild && pos + 1 + sizeof(struct Pokemon) <= len && data[pos] == 1)
                {
                    memcpy(&sCoop.caughtMon, data + pos + 1, sizeof(struct Pokemon));
                    sCoop.caught = CAUGHT_MINE;
                }
            }
        }
        break;
    }
}

void MP_Coop_OnPlayerLeft(u8 id)
{
    if (sCoop.wildFrom == id)
        StopWaitingForWild();
    if (sCoop.requester == id)
        sCoop.requester = -1;
    if (sCoop.accepted && sCoop.partner == id)
        sCoop.accepted = FALSE;
    if (sCoop.declinedBy == id)
        sCoop.declinedBy = -1;
    // An asker waiting on this player is told by UpdateAsking
}
