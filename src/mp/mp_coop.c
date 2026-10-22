// Moemon Co-op: team up with another player against an NPC trainer.
//
// When an NPC trainer challenges a player, that player is marked CHALLENGED.
// Another player who walks up to them (stands next to them, facing them)
// asks to join. Once the challenged player has read the trainer's speech,
// they get an unskippable "X wants to join for a Double Battle! Is this OK?"
// question. Trainers with a single Pokémon refuse ("Let's go one on one!").
// If nobody has asked by then and a friend is on the same map, the challenged
// player is asked "Wait for another trainer to join you?": while they wait a
// "Join me!" bubble shows over them, other games leave that trainer alone,
// and the first friend who walks up joins. B battles alone.
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
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "random.h"
#include "mp.h"
#include "item.h"
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
#include "battle_message.h"
#include "trainer_hill.h"
#include "trainer_see.h"
#include "constants/battle.h"
#include "constants/battle_setup.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/flags.h"

extern const u8 EventScript_MpCoopLearnMove[];

bool8 gMpCoopBattle;
bool8 gMpCoopWild;
u8 gMpCoopItemSlot = 0xFF;         // party slot a healing item was used on (picked in the party menu)
u8 gMpCoopItemTargetSlot = 0xFF;
u8 gMpCoopBallTarget = 0xFF;   // the master's battle: the slot it's being applied to

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
    MPR_COOP_WAIT,                      // challenged -> all: u8 on, u8 localId, mapNum, mapGroup of the trainer
    MPR_COOP_BSTATUS,                   // in a co-op battle -> partner: u8 MP_BST_* (what we're choosing)
    MPR_COOP_BONUS,                     // battle start -> partner: u16 own EXP bonus percent (MP_ExpBonusPercent)
    MPR_COOP_REFUND,                    // master -> partner: u16 ball that went back to your bag (its target was gone)
    MPR_COOP_RESOLVE,                   // master -> owner: their level-up boxes and full-move-set learns this round
                                        // (u8 nBox, u8 nLearn, nBox * {slot, 6 u16 stats before},
                                        // nLearn * {slot, u16 move, 4 u16 moves, 4 u8 PP}), all players at once
    MPR_COOP_RESOLVE_DONE,              // owner -> master: u8 n, n * u8 move slot forgotten (0xFF = didn't learn)
};

#define WAIT_RESEND_FRAMES 60    // the waiting player repeats MPR_COOP_WAIT this often
#define WAIT_HOLD_FRAMES   240   // others forget a wait they stop hearing about

#define WILD_RANGE_X     8     // about one screen
#define WILD_RANGE_Y     6
#define WILD_ASK_FRAMES  150   // wait this long for an answer, then battle alone (4 relay trips on an NA-JP link)
#define WILD_GO_FRAMES   300   // a friend who said yes waits this long for the battle

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
    DECLINE_NO_TRIPLE,   // a third player: this trainer can't do a Triple Battle
};

enum {
    ROLE_NONE,
    ROLE_ASKING,
};

#define MAX_LEARN 8
#define RESULT_WAIT_FRAMES (60 * 15)
#define LOSE_TEXT_MAX 240
#define MODE_WILD 0xFE   // sCoop.mode of a co-op wild double
u8 gMpWildDoublePercent = 60;   // co-op wild battles that are 2 on 2 (debug: mp wilddouble N)
u8 gMpEnemyLevelBonus;          // added to trainer Pokemon levels while a co-op party is built
bool8 MP_RollExtraLandMon(struct Pokemon *out);   // wild_encounter.c

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
    s8 requester2;         // a second one while we wait for a Triple Battle, -1 = nobody
    bool8 waitingThird;    // one friend is in; waiting for a third player
    u8 joinCount;          // players in the accepted battle (2 or 3)
    u8 joinPlayers[3];
    bool8 prompting;
    bool8 accepted;
    u8 waitStage;          // WAIT_*: asked whether to wait / waiting for a partner
    u16 waitTimer;
    u8 waitLocalId, waitMapNum, waitMapGroup;   // the trainer held while we wait
    // Asking player
    u8 role;
    u8 target;
    u8 targetName[MP_NAME_LEN + 1];
    u16 timer;
    s8 declinedBy;         // don't ask this player again until their challenge ends
    s8 cancelledAsk;       // we pressed B while asking them: ask again once we've turned away and back
    s8 promptedFor;        // whose "wants to join" question is up (they may stop asking and ask again)
    // Battle. Seats: 0 = the master (who met the trainer / wild Pokémon), 1-2
    // the friends who joined, in link order. A co-op double has two seats of
    // three Pokémon (party slots 0-2, 3-5), a triple three seats of two (0-1, 2-3, 4-5).
    bool8 isMaster;
    u8 partner;            // the first other seat's player
    u8 count;              // seats (2 or 3)
    u8 seat;               // ours
    u8 seatPlayer[3];      // network player id of each seat
    u16 playerBonus[MP_MAX_PLAYERS];   // master: each friend's own EXP bonus (MPR_COOP_BONUS;
    u8 playerBonusHeard;               // bits by player: it can come before our battle is set up)
    u8 seatLeveled[3];     // master: level-up bits per seat (own slots)
    s8 bonusFrom;          // the friend whose EXP bonus we last heard (MPR_COOP_BONUS), -1 = nobody
    u16 bonusPercent;
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
    struct Pokemon wildMon;   // the wild Pokémon both players battle
    struct Pokemon wildMon2;  // a second one (2-on-2), or nothing
    struct Pokemon wildMon3;  // a triple's third
    bool8 wildDouble;
    u8 wildExtra;          // a triple: extra wild Pokémon rolled (0-2)
    s8 wildTarget2;        // a triple: the second friend asked, -1 = none
    u8 wildReply2;
    // Co-op wild catches (the master's game records them): up to one per wild Pokémon
    struct Pokemon catches[3];
    u8 catchOwner[3];         // CAUGHT_MINE / CAUGHT_PARTNER (the partner's game: all CAUGHT_MINE)
    u8 catchSeat[3];          // the seat that threw the ball
    u8 catchCount;
    u8 caught;             // CAUGHT_*
} sCoop = { .requester = -1, .requester2 = -1, .declinedBy = -1, .cancelledAsk = -1, .promptedFor = -1, .wildFrom = -1, .bonusFrom = -1, .wildTarget2 = -1 };

// Pokémon each seat brings: 3 in a co-op double, 2 in a triple
static u8 SeatSize(void)
{
    return sCoop.count == 3 ? 2 : MULTI_PARTY_SIZE;
}

static u8 SeatBase(u8 seat)
{
    return seat * SeatSize();
}

// The seat whose Pokémon sits in this battle party slot
static u8 SeatOfSlot(u8 slot)
{
    return slot / SeatSize();
}

static s8 SeatOf(u8 player)
{
    u8 i;
    for (i = 0; i < sCoop.count && i < 3; i++)
        if (sCoop.seatPlayer[i] == player)
            return i;
    return -1;
}

// A co-op battle's other player (not us)
static bool8 IsBattleMate(u8 player)
{
    s8 seat = SeatOf(player);
    return seat >= 0 && seat != sCoop.seat;
}

bool8 MP_Coop_IsTriple(void)
{
    return gMpCoopBattle && sCoop.count == 3;
}

// Network player id of a battle party slot's owner (-1 = us / nobody)
s8 MP_Coop_SlotOwnerPlayer(u8 slot)
{
    u8 seat;
    if (!gMpCoopBattle || slot >= PARTY_SIZE)
        return -1;
    seat = SeatOfSlot(slot);
    if (seat >= sCoop.count || seat == sCoop.seat)
        return -1;
    return sCoop.seatPlayer[seat];
}

// Our seat's first battle party slot
u8 MP_Coop_OwnBase(void)
{
    return gMpCoopBattle ? SeatBase(sCoop.seat) : 0;
}

u8 MP_Coop_SeatSize(void)
{
    return gMpCoopBattle ? SeatSize() : PARTY_SIZE;
}

enum { WAIT_NONE, WAIT_ASKING, WAIT_WAITING };

// Trainers other players are holding while they wait for a partner: they
// can't be talked to and don't spot anyone here
static struct
{
    u16 ttl;
    u8 localId, mapNum, mapGroup;
} sHeld[MP_MAX_PLAYERS];

static EWRAM_DATA u8 sText[160];

static const u8 sText_JoinPrompt[] = _("{STR_VAR_1} wants to join for a\nDouble Battle! Is this OK?");
static const u8 sText_OneMon[] = _("Hey, I only have one MOéMON!\nLet's go one on one!");
static const u8 sText_Asking[] = _("Asking {STR_VAR_1} to team up for a\nDouble Battle… ({B_BUTTON} never mind)");
static const u8 sText_SaidNo[] = _("{STR_VAR_1} wants to battle alone\nthis time.");
static const u8 sText_TheyHaveOne[] = _("{STR_VAR_1}'s opponent only has one\nMOéMON. It's a one-on-one battle!");
static const u8 sText_TooLate[] = _("Too late! {STR_VAR_1}'s battle has\nalready started.");
static const u8 sText_Busy[] = _("{STR_VAR_1} can't team up right now.");
static const u8 sText_NoTripleAsker[] = _("{STR_VAR_1}'s opponent can't do a Triple\nBattle. It's a Double Battle this time!");
static const u8 sText_NoTripleFew[] = _("This trainer has fewer than 3 MOéMON:\nno Triple Battle. It's a Double!");
static const u8 sText_NoTripleKind[] = _("This battle can't be a Triple Battle.\nIt's a Double!");
static const u8 sText_Left[] = _("{STR_VAR_1} left the game.");
static const u8 sText_WaitAsk[] = _("Wait for another trainer to join you?");
static const u8 sText_WaitMore[] = _("{STR_VAR_1} has joined!\nWait for another?");
static const u8 sText_Waiting[] = _("Waiting for a partner to join…\n({B_BUTTON} battle alone)");
static const u8 sText_JoinMe[] = _("Join me!");
static const u8 sText_WaitThird[] = _("{STR_VAR_1} is in! Waiting for one more\nfor a Triple Battle… ({B_BUTTON} start now)");
static const u8 sText_StartDouble[] = _("Start a Double Battle with {STR_VAR_1}\nwithout waiting for a third?");
static const u8 sText_HeldNpc[] = _("They're waiting on {STR_VAR_1}'s battle.");
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
    return sCoop.role != ROLE_NONE || sCoop.prompting || sCoop.waitStage != WAIT_NONE || gMpCoopBattle || sCoop.wildAsking || sCoop.wildFrom >= 0;
}

s8 MP_Coop_PartnerId(void)
{
    return gMpCoopBattle ? (s8)sCoop.partner : -1;
}

bool8 MP_Coop_IsMaster(void)
{
    return sCoop.isMaster;
}

u16 MP_Coop_TrainerPic(void)
{
    return gTrainers[gTrainerBattleOpponent_A].trainerPic;
}

// The player controlling a player-side battler (a triple: battler 2i = seat i)
s8 MP_Coop_BattlerPlayer(u8 battler)
{
    if (!gMpCoopBattle || battler >= MAX_BATTLERS_COUNT || (battler & BIT_SIDE))
        return -1;
    if (sCoop.count == 3)
        return battler / 2 < 3 ? (s8)sCoop.seatPlayer[battler / 2] : -1;
    return (battler & BIT_FLANK) ? (s8)sCoop.seatPlayer[1] : (s8)sCoop.seatPlayer[0];
}

// The link partner controller's battler (gActiveBattler): its player's back picture
static u8 PartnerOfActive(void)
{
    s8 p = MP_Coop_BattlerPlayer(gActiveBattler);
    return (p >= 0 && p != gMpLocalId) ? (u8)p : sCoop.partner;
}

// A triple's two allies get their own back picture slots (the later seat: slot 1)
static u8 BackSlotOfActive(void)
{
    s8 seat;
    if (sCoop.count != 3 || (seat = SeatOf(PartnerOfActive())) < 0)
        return 0;
    return (seat > sCoop.seat ? seat - 1 : seat) == 1 ? 1 : 0;
}

u16 MP_Coop_PartnerBackPicOr(u16 defaultPic)
{
    u16 pic = MP_Avatar_RemoteBackPicSlot(PartnerOfActive(), BackSlotOfActive());
    return pic == 0xFFFF ? defaultPic : pic;
}

bool8 MP_Coop_PartnerHidesBack(void)
{
    return MP_Avatar_RemoteHidesBack(PartnerOfActive());
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
    if (PlayerValid(sCoop.requester2))
        SendByte(sCoop.requester2, MPR_COOP_DECLINE, reason);
    sCoop.challenged = FALSE;
    sCoop.requester = -1;
    sCoop.requester2 = -1;
    sCoop.waitingThird = FALSE;
    sCoop.accepted = FALSE;
}

// How many Moemon the trainer brings to a co-op battle: their own, plus the one
// TEAM PADDING adds (a host setting, MP_TrainerPadding). Never more than one extra.
static u8 CoopTrainerTeamSize(u16 trainerId)
{
    u8 n = gTrainers[trainerId].partySize;
    if (MP_TrainerPadding() && n < PARTY_SIZE)
        n++;
    return n;
}

// A Triple Battle needs a trainer with 3+ Moemon (TEAM PADDING counted) and an
// ordinary one-trainer battle (not Twins and the like; rematches and battles
// without intro text are fine). A Double Battle needs 2+.
static void OnResolve(const u8 *data, u32 len);
static void OnResolveDone(s8 from, const u8 *data, u32 len);

static bool8 TripleEligible(void)
{
    return CoopTrainerTeamSize(sCoop.trainerId) >= 3
        && (sCoop.mode == TRAINER_BATTLE_SINGLE || sCoop.mode == TRAINER_BATTLE_CONTINUE_SCRIPT
            || sCoop.mode == TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC || sCoop.mode == TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT
            || sCoop.mode == TRAINER_BATTLE_REMATCH);
}

// Someone besides `except` who could still come and join: anyone online, anywhere,
// whatever they're doing right now (they may be finishing a battle or a menu, or be
// on the next route). The host can always start the Double Battle without them.
static bool8 FriendAvailableExcept(s8 except)
{
    u8 i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (PlayerValid(i) && i != except && gMpPlayers[i].hasState)
            return TRUE;
    return FALSE;
}

#define tState data[0]

enum {
    JOIN_START,
    JOIN_PRINT,
    JOIN_ASK,
    JOIN_ONE_MON,
    JOIN_WAIT_PRINT,
    JOIN_WAIT_ASK,
    JOIN_WAITING,
    JOIN_WAIT_THIRD,
    JOIN_NO_TRIPLE,
    JOIN_START_DOUBLE_PRINT,   // B while waiting for a third: start the Double Battle now?
    JOIN_START_DOUBLE_ASK,
    JOIN_MORE_PRINT,           // "<friend> has joined! Wait for another?"
    JOIN_MORE_ASK,
};

enum { THIRD_NONE, THIRD_ASK, THIRD_QUEUED };

static void SendWait(bool8 on)
{
    u8 buf[5] = {MPR_COOP_WAIT, on, sCoop.waitLocalId, sCoop.waitMapNum, sCoop.waitMapGroup};
    MP_SendRelay(0xFF, buf, sizeof(buf));
}

static void StopWaiting(void)
{
    if (sCoop.waitStage == WAIT_WAITING)
    {
        SendWait(FALSE);
        if (gMpLocalId >= 0)
            MP_Emote_ClearBubbleFor(gMpLocalId);
    }
    sCoop.waitStage = WAIT_NONE;
}

// Another player is on this map (someone who could come and join)
static bool8 FriendOnMap(void)
{
    u8 i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        if (PlayerValid(i)
         && gMpPlayers[i].state.mapGroup == gSaveBlock1Ptr->location.mapGroup
         && gMpPlayers[i].state.mapNum == gSaveBlock1Ptr->location.mapNum)
            return TRUE;
    }
    return FALSE;
}

// The requester (and a second one: a Triple Battle) joins: tell them the
// trainer, who's in which seat and how it ends.
// u16 trainer, u8 mode, u8 count, u8 seat, 3x u8 player, lose text
static void AcceptRequester(void)
{
    u8 buf[9 + LOSE_TEXT_MAX + 1];
    const u8 *lose = GetTrainerALoseText();
    u16 len = 0;
    u8 i;

    sCoop.joinPlayers[0] = gMpLocalId;
    sCoop.joinPlayers[1] = sCoop.requester;
    sCoop.joinPlayers[2] = 0xFF;
    sCoop.joinCount = 2;
    if (PlayerValid(sCoop.requester2) && sCoop.requester2 != sCoop.requester && TripleEligible())
    {
        sCoop.joinPlayers[2] = sCoop.requester2;
        sCoop.joinCount = 3;
    }
    else if (PlayerValid(sCoop.requester2))
    {
        SendByte(sCoop.requester2, MPR_COOP_DECLINE, DECLINE_TOO_LATE);
    }
    sCoop.requester2 = -1;
    sCoop.waitingThird = FALSE;
    buf[0] = MPR_COOP_ACCEPT;
    buf[1] = sCoop.trainerId & 0xFF;
    buf[2] = sCoop.trainerId >> 8;
    buf[3] = sCoop.mode;
    buf[4] = sCoop.joinCount;
    buf[6] = sCoop.joinPlayers[0];
    buf[7] = sCoop.joinPlayers[1];
    buf[8] = sCoop.joinPlayers[2];
    if (lose != NULL)
    {
        while (len < LOSE_TEXT_MAX && lose[len] != EOS)
        {
            buf[9 + len] = lose[len];
            len++;
        }
    }
    buf[9 + len] = EOS;
    for (i = 1; i < sCoop.joinCount; i++)
    {
        buf[5] = i;
        Send(sCoop.joinPlayers[i], buf, 10 + len);
    }
    sCoop.accepted = TRUE;
    sCoop.partner = sCoop.requester;
    MPNet_Log("co-op: player %d joins the battle against trainer %d%s", sCoop.partner + 1, sCoop.trainerId,
              sCoop.joinCount == 3 ? " (and another: triple)" : "");
}

// The NPC the others hold while we wait (and the "waiting" message they get)
static void BeginWaitStage(void)
{
    if (sCoop.waitStage != WAIT_WAITING)
    {
        if (gSelectedObjectEvent < OBJECT_EVENTS_COUNT)
        {
            sCoop.waitLocalId = gObjectEvents[gSelectedObjectEvent].localId;
            sCoop.waitMapNum = gObjectEvents[gSelectedObjectEvent].mapNum;
            sCoop.waitMapGroup = gObjectEvents[gSelectedObjectEvent].mapGroup;
        }
        sCoop.waitStage = WAIT_WAITING;
    }
    sCoop.waitTimer = 0;
    SendWait(TRUE);
}

// One friend is in. When the trainer allows a Triple Battle and someone else is
// online: THIRD_ASK ("<friend> has joined! Wait for another?" is up), or
// THIRD_QUEUED (a third already asked: all three are here). THIRD_NONE otherwise.
static u8 TryWaitForThird(void)
{
    if (sCoop.requester2 == sCoop.requester)
        sCoop.requester2 = -1;
    if (!TripleEligible() || (!PlayerValid(sCoop.requester2) && !FriendAvailableExcept(sCoop.requester)))
    {
        MPNet_Log("co-op: no triple (trainer %d, %d Pokemon, padding %d, mode %d)", sCoop.trainerId,
                  gTrainers[sCoop.trainerId].partySize, MP_TrainerPadding(), sCoop.mode);
        return THIRD_NONE;
    }
    sCoop.waitingThird = TRUE;   // a third who asks from now on is kept
    BeginWaitStage();
    if (PlayerValid(sCoop.requester2))
        return THIRD_QUEUED;
    SetNameVar(sCoop.requester);
    StringExpandPlaceholders(sText, sText_WaitMore);
    HideMapNamePopUpWindow();
    MP_ReplaceFieldMessage(sText);
    MPNet_Log("co-op: asking whether to wait for a third player");
    return THIRD_ASK;
}

// Yes, wait for a third
static void StartWaitingThird(void)
{
    sCoop.waitingThird = TRUE;
    if (sCoop.requester2 == sCoop.requester)
        sCoop.requester2 = -1;
    BeginWaitStage();
    SetNameVar(sCoop.requester);
    StringExpandPlaceholders(sText, sText_WaitThird);
    HideMapNamePopUpWindow();
    MP_ReplaceFieldMessage(sText);
    MPNet_Log("co-op: waiting for a third player");
}

// A friend is in, another is around, but this trainer can't do a Triple Battle:
// say so before the Double Battle starts (TRUE: the note is up)
static bool8 ShowNoTripleNote(void)
{
    if (TripleEligible() || !FriendAvailableExcept(sCoop.requester))
        return FALSE;
    HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
    MP_ReplaceFieldMessage(CoopTrainerTeamSize(sCoop.trainerId) < 3 ? sText_NoTripleFew : sText_NoTripleKind);
    MPNet_Log("co-op: no triple with this trainer, told the player");
    return TRUE;
}

static void FinishJoinCheck(u8 taskId)
{
    if (!sCoop.accepted && PlayerValid(sCoop.requester2))
        SendByte(sCoop.requester2, MPR_COOP_DECLINE, DECLINE_TOO_LATE);
    if (!sCoop.accepted)
    {
        sCoop.requester2 = -1;
        sCoop.waitingThird = FALSE;
    }
    StopWaiting();
    if (!IsFieldMessageBoxHidden())
        HideFieldMessageBox();
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
        if (!sCoop.challenged || gNoOfApproachingTrainers == 2)
        {
            FinishJoinCheck(taskId);
            break;
        }
        if (!PlayerValid(sCoop.requester))
        {
            // Nobody asked yet: offer to wait for a friend on this map
            if (CoopTrainerTeamSize(sCoop.trainerId) <= 1 || !FriendOnMap() || gSelectedObjectEvent >= OBJECT_EVENTS_COUNT)
            {
                FinishJoinCheck(taskId);
                break;
            }
            sCoop.waitLocalId = gObjectEvents[gSelectedObjectEvent].localId;
            sCoop.waitMapNum = gObjectEvents[gSelectedObjectEvent].mapNum;
            sCoop.waitMapGroup = gObjectEvents[gSelectedObjectEvent].mapGroup;
            sCoop.waitStage = WAIT_ASKING;
            HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
            MP_ReplaceFieldMessage(sText_WaitAsk);
            tState = JOIN_WAIT_PRINT;
            break;
        }
        if (CoopTrainerTeamSize(sCoop.trainerId) <= 1)
        {
            // "Hey, I only have one MOéMON!" (TEAM PADDING off)
            SendByte(sCoop.requester, MPR_COOP_DECLINE, DECLINE_ONE_MON);
            sCoop.requester = -1;
            HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
            MP_ReplaceFieldMessage(sText_OneMon);
            tState = JOIN_ONE_MON;
            break;
        }
        SetNameVar(sCoop.requester);
        Gamepad_Rumble(40, 200);
        StringExpandPlaceholders(sText, sText_JoinPrompt);
        HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
        MP_ReplaceFieldMessage(sText);
        sCoop.prompting = TRUE;
        sCoop.promptedFor = sCoop.requester;
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
                u8 third = TryWaitForThird();
                if (third == THIRD_ASK)
                {
                    tState = JOIN_MORE_PRINT;
                    break;
                }
                if (third == THIRD_QUEUED)
                {
                    tState = JOIN_WAIT_THIRD;   // all three are here: it begins
                    break;
                }
                if (ShowNoTripleNote())
                {
                    data[1] = 0;
                    tState = JOIN_NO_TRIPLE;
                    break;
                }
                AcceptRequester();
            }
            else
            {
                SendByte(sCoop.requester, MPR_COOP_DECLINE, DECLINE_NO);
                sCoop.requester = -1;
                if (PlayerValid(sCoop.requester2))
                    SendByte(sCoop.requester2, MPR_COOP_DECLINE, DECLINE_NO);
                sCoop.requester2 = -1;
            }
        }
        FinishJoinCheck(taskId);
        break;
    }
    case JOIN_ONE_MON:
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
            FinishJoinCheck(taskId);
        break;
    case JOIN_WAIT_PRINT:
        if (!IsTextPrinterActive(0))
        {
            DisplayYesNoMenuDefaultYes();
            tState = JOIN_WAIT_ASK;
        }
        break;
    case JOIN_WAIT_ASK:
    {
        s8 answer = Menu_ProcessInputNoWrapClearOnChoose();
        if (answer == MENU_NOTHING_CHOSEN)
            break;
        if (answer != 0)
        {
            // Battle alone (friends who asked meanwhile are told no)
            if (PlayerValid(sCoop.requester))
                SendByte(sCoop.requester, MPR_COOP_DECLINE, DECLINE_NO);
            if (PlayerValid(sCoop.requester2))
                SendByte(sCoop.requester2, MPR_COOP_DECLINE, DECLINE_NO);
            sCoop.requester = -1;
            sCoop.requester2 = -1;
            FinishJoinCheck(taskId);
            break;
        }
        sCoop.waitStage = WAIT_WAITING;
        sCoop.waitTimer = 0;
        SendWait(TRUE);
        MP_ReplaceFieldMessage(sText_Waiting);
        tState = JOIN_WAITING;
        // fall through
    }
    case JOIN_WAITING:
        if (PlayerValid(sCoop.requester))
        {
            // A friend walked up: they're in
            u8 third;
            PlaySE(SE_SUCCESS);
            third = TryWaitForThird();
            if (third == THIRD_ASK)
            {
                tState = JOIN_MORE_PRINT;
                break;
            }
            if (third == THIRD_QUEUED)
            {
                tState = JOIN_WAIT_THIRD;
                break;
            }
            if (ShowNoTripleNote())
            {
                data[1] = 0;
                tState = JOIN_NO_TRIPLE;
                break;
            }
            AcceptRequester();
            FinishJoinCheck(taskId);
            break;
        }
        if (++sCoop.waitTimer >= WAIT_RESEND_FRAMES)
        {
            sCoop.waitTimer = 0;
            SendWait(TRUE);
        }
        if (gMpLocalId >= 0)
            MP_Emote_KeepBubbleFor(gMpLocalId, sText_JoinMe);
        if (!IsTextPrinterActive(0) && JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            FinishJoinCheck(taskId);   // a single battle after all
        }
        break;
    case JOIN_NO_TRIPLE:
        // The note about no Triple Battle, then the Double Battle (A / B or a few seconds)
        if (!PlayerValid(sCoop.requester))
        {
            sCoop.requester = -1;
            FinishJoinCheck(taskId);   // they left: battle alone
            break;
        }
        if (!IsTextPrinterActive(0) && (JOY_NEW(A_BUTTON | B_BUTTON) || ++data[1] >= 150))
        {
            AcceptRequester();
            FinishJoinCheck(taskId);
        }
        break;
    case JOIN_WAIT_THIRD:
        if (!PlayerValid(sCoop.requester) && PlayerValid(sCoop.requester2))
        {
            sCoop.requester = sCoop.requester2;   // the first one left: the second takes their seat
            sCoop.requester2 = -1;
        }
        if (!PlayerValid(sCoop.requester))
        {
            // nobody left: battle alone
            sCoop.waitingThird = FALSE;
            sCoop.requester = -1;
            FinishJoinCheck(taskId);
            break;
        }
        if (PlayerValid(sCoop.requester2))
        {
            PlaySE(SE_SUCCESS);
            AcceptRequester();   // three of us: a Triple Battle
            FinishJoinCheck(taskId);
            break;
        }
        if (++sCoop.waitTimer >= WAIT_RESEND_FRAMES)
        {
            sCoop.waitTimer = 0;
            SendWait(TRUE);
        }
        if (gMpLocalId >= 0)
            MP_Emote_KeepBubbleFor(gMpLocalId, sText_JoinMe);
        if (!IsTextPrinterActive(0) && JOY_NEW(B_BUTTON))
        {
            // start a Double Battle now? (asked, so a B pressed through the text doesn't)
            PlaySE(SE_SELECT);
            SetNameVar(sCoop.requester);
            StringExpandPlaceholders(sText, sText_StartDouble);
            MP_ReplaceFieldMessage(sText);
            tState = JOIN_START_DOUBLE_PRINT;
        }
        break;
    case JOIN_MORE_PRINT:
    case JOIN_MORE_ASK:
        // "<friend> has joined! Wait for another?" A third who turns up meanwhile: all
        // three are here, it begins. The friend leaving: battle alone.
        if (!PlayerValid(sCoop.requester) || PlayerValid(sCoop.requester2))
        {
            if (tState == JOIN_MORE_ASK)
                EraseYesNoWindow();
            tState = JOIN_WAIT_THIRD;
            break;
        }
        if (++sCoop.waitTimer >= WAIT_RESEND_FRAMES)
        {
            sCoop.waitTimer = 0;
            SendWait(TRUE);
        }
        if (tState == JOIN_MORE_PRINT)
        {
            if (!IsTextPrinterActive(0))
            {
                DisplayYesNoMenuDefaultYes();
                MPNet_Log("co-op: wait for another? YES / NO");
                tState = JOIN_MORE_ASK;
            }
            break;
        }
        {
            s8 answer = Menu_ProcessInputNoWrapClearOnChoose();
            if (answer == MENU_NOTHING_CHOSEN)
                break;
            if (answer == 0)
            {
                StartWaitingThird();   // yes: wait for a third
                tState = JOIN_WAIT_THIRD;
            }
            else
            {
                AcceptRequester();     // no: a Double Battle now
                FinishJoinCheck(taskId);
            }
        }
        break;
    case JOIN_START_DOUBLE_PRINT:
    case JOIN_START_DOUBLE_ASK:
        // the third may still turn up meanwhile, and the second may leave
        if (!PlayerValid(sCoop.requester) || PlayerValid(sCoop.requester2))
        {
            if (tState == JOIN_START_DOUBLE_ASK)
                EraseYesNoWindow();
            tState = JOIN_WAIT_THIRD;
            break;
        }
        if (tState == JOIN_START_DOUBLE_PRINT)
        {
            if (!IsTextPrinterActive(0))
            {
                DisplayYesNoMenuWithDefault(1);   // NO first
                tState = JOIN_START_DOUBLE_ASK;
            }
        }
        else
        {
            s8 answer = Menu_ProcessInputNoWrapClearOnChoose();
            if (answer == MENU_NOTHING_CHOSEN)
                break;
            if (answer == 0)
            {
                AcceptRequester();   // a Double Battle now
                FinishJoinCheck(taskId);
                break;
            }
            // keep waiting
            SetNameVar(sCoop.requester);
            StringExpandPlaceholders(sText, sText_WaitThird);
            MP_ReplaceFieldMessage(sText);
            tState = JOIN_WAIT_THIRD;
        }
        if (++sCoop.waitTimer >= WAIT_RESEND_FRAMES)
        {
            sCoop.waitTimer = 0;
            SendWait(TRUE);
        }
        break;
    }
}

// The EXP bonus percent for a Pokémon in the master's battle party: party slots
// 0-2 are the master's own, 3-5 the partner's (their own bonus, MPR_COOP_BONUS).
u16 MP_Coop_ExpBonusFor(u8 partySlot)
{
    u8 seat;
    if (gMpCoopBattle && sCoop.isMaster && partySlot < PARTY_SIZE && (seat = SeatOfSlot(partySlot)) != 0 && seat < sCoop.count)
    {
        u8 player = sCoop.seatPlayer[seat];
        if (player < MP_MAX_PLAYERS && (sCoop.playerBonusHeard & (1 << player)))
            return sCoop.playerBonus[player];
        MPNet_Log("co-op: no EXP bonus heard from player %d, using ours", sCoop.seatPlayer[seat]);
    }
    return MP_ExpBonusPercent();
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
    u8 slots[MULTI_PARTY_SIZE];

    SavePlayerParty(); // LoadPlayerParty brings the real party back afterwards
    memcpy(tmp, gPlayerParty, sizeof(struct Pokemon) * PARTY_SIZE);
    for (i = 0; i < MULTI_PARTY_SIZE; i++)
        sCoop.origIndex[i] = 0xFF;
    // the co-op team (TEAM in the party menu) first, then the first usable others
    // (a triple takes two)
    n = MP_Team_BattleSlots(slots, SeatSize(), IsUsableMon);
    for (i = 0; i < n; i++)
    {
        gPlayerParty[i] = tmp[slots[i]];
        sCoop.origIndex[i] = slots[i];
    }
    for (i = n; i < PARTY_SIZE; i++)
        ZeroMonData(&gPlayerParty[i]);
    sCoop.localCount = n;
    Free(tmp);
}

// players[seat] = network ids (ours included); count 2 = double, 3 = triple
static void BeginCoopBattleGroup(u8 seat, u8 count, const u8 *players)
{
    u8 i;
    sCoop.caught = CAUGHT_NONE;
    sCoop.catchCount = 0;
    sCoop.isMaster = seat == 0;
    sCoop.seat = seat;
    sCoop.count = count;
    for (i = 0; i < 3; i++)
        sCoop.seatPlayer[i] = i < count ? players[i] : 0xFF;
    sCoop.partner = players[seat == 0 ? 1 : 0];
    memset(sCoop.seatLeveled, 0, sizeof(sCoop.seatLeveled));
    gIsTripleBattle = count == 3;
    gMpDebugTriple = getenv("MOEMON_TRIPLEDEBUG") != NULL;
    sCoop.money = 0;
    sCoop.learnCount = 0;
    sCoop.partnerLeveled = 0;
    sCoop.gotResult = FALSE;
    sCoop.resultWait = 0;
    sCoop.evoQueue = 0;
    ArrangeLocalParty();
    // our own EXP bonus for the master's EXP script (each player gets their own)
    if (seat != 0)
    {
        u16 pct = MP_ExpBonusPercent();
        u8 buf[3] = {MPR_COOP_BONUS, pct & 0xFF, pct >> 8};
        Send(players[0], buf, 3);
    }
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_LINK
                     | BATTLE_TYPE_MULTI | BATTLE_TYPE_TOWER_LINK_MULTI;
    gMpCoopBattle = TRUE;
    gLinkType = LINKTYPE_BATTLE;
    if (count == 3)
        MP_Battle_OpenLinkGroup(players, count, seat);
    else
        MP_Battle_OpenLink(sCoop.partner, seat == 0);
}

static void BeginCoopBattle(bool8 isMaster, u8 partner)
{
    u8 players[2];
    players[0] = isMaster ? gMpLocalId : partner;
    players[1] = isMaster ? partner : gMpLocalId;
    BeginCoopBattleGroup(isMaster ? 0 : 1, 2, players);
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
    sCoop.requester2 = -1;
    if (!go)
        return;
    if (sCoop.joinCount == 3 && PlayerValid(sCoop.joinPlayers[2]))
        BeginCoopBattleGroup(0, 3, sCoop.joinPlayers);
    else
        BeginCoopBattle(TRUE, sCoop.partner);
    sCoop.haveLoseText = FALSE;
    sCoop.afterBattle = gMain.savedCallback;
    gMain.savedCallback = CB2_MpCoopEndBattle;
}

// The trainer's party is interleaved: the master's game puts the first,
// third and fifth Pokémon on the left opponent and the others on the right,
// so the trainer leads with their first two and sends the rest in order.
// ---------------------------------------------------------------------------
// Two players against one trainer is easy on vanilla numbers, so the master
// builds a tougher party: +2 levels (+3 for gym leaders, admins, bosses and
// rematches), more Pokémon (route trainers half again as many, bosses a full
// six) placed before the ace, the trainer's items in play and smarter AI.

bool8 MP_Coop_IsBossTrainer(u16 trainerId)
{
    switch (gTrainers[trainerId].trainerClass)
    {
    case TRAINER_CLASS_LEADER:
    case TRAINER_CLASS_AQUA_ADMIN:
    case TRAINER_CLASS_AQUA_LEADER:
    case TRAINER_CLASS_MAGMA_ADMIN:
    case TRAINER_CLASS_MAGMA_LEADER:
    case TRAINER_CLASS_ELITE_FOUR:
    case TRAINER_CLASS_CHAMPION:
        return TRUE;
    }
    return GymBadgeFlag(trainerId) != 0 || MP_Rematch_IsRematchTrainer(trainerId);
}

bool8 MP_Coop_TrainerBattleBoost(void)
{
    return gMpCoopBattle && !gMpCoopWild && (gBattleTypeFlags & BATTLE_TYPE_TRAINER);
}

void MP_Coop_PrepareTrainerParty(u16 trainerId)
{
    gMpEnemyLevelBonus = MP_Coop_IsBossTrainer(trainerId) ? 3 : 2;
}

static u16 BaseStatTotal(u16 species)
{
    const struct SpeciesInfo *b = &gSpeciesInfo[species];
    return b->baseHP + b->baseAttack + b->baseDefense + b->baseSpeed + b->baseSpAttack + b->baseSpDefense;
}

static bool8 ShareType(u16 a, u16 b)
{
    const u8 *ta = gSpeciesInfo[a].types, *tb = gSpeciesInfo[b].types;
    return ta[0] == tb[0] || ta[0] == tb[1] || ta[1] == tb[0] || ta[1] == tb[1];
}

// A Moemon to pad a co-op trainer's party with, in place of a copy of `src`:
// it shares a type with it, is about as strong (base stats within 15%), isn't
// legendary or a baby (no-eggs group), and isn't in the party yet. Always the
// same for the same trainer and slot. Falls back to src's own species.
static u16 PickFillerSpecies(struct Pokemon *src, u8 slot, u16 trainerId)
{
    static u16 sCandidates[NUM_SPECIES];
    u16 srcSpecies = GetMonData(src, MON_DATA_SPECIES), species, count = 0;
    u16 bst = BaseStatTotal(srcSpecies);
    u8 tries;
    u32 seed = trainerId * 2654435761u + slot * 40503u;

    for (tries = 0; tries < 2 && count == 0; tries++)
    {
        u16 lo = bst * (tries ? 75 : 85) / 100, hi = bst * (tries ? 125 : 115) / 100;
        for (species = 1; species < NUM_SPECIES; species++)
        {
            u8 k;
            bool8 used = FALSE;
            if (species >= SPECIES_OLD_UNOWN_B && species <= SPECIES_OLD_UNOWN_Z)
                continue;
            if (species == SPECIES_SHEDINJA || species == SPECIES_UNOWN || species == SPECIES_DITTO
             || gSpeciesInfo[species].eggGroups[0] == EGG_GROUP_NO_EGGS_DISCOVERED)
                continue;
            if (!ShareType(species, srcSpecies) || BaseStatTotal(species) < lo || BaseStatTotal(species) > hi)
                continue;
            for (k = 0; k < PARTY_SIZE; k++)
                if (GetMonData(&gEnemyParty[k], MON_DATA_SPECIES) == species)
                    used = TRUE;
            if (!used)
                sCandidates[count++] = species;
        }
    }
    if (count == 0)
        return srcSpecies;
    seed ^= seed >> 13;
    seed *= 0x5bd1e995u;
    seed ^= seed >> 15;
    return sCandidates[seed % count];
}

// TEAM PADDING (a host setting): the trainer gets exactly one extra Moemon in a
// co-op battle, Double or Triple, never more. Off: the trainer's own team only.
void MP_Coop_FinishTrainerParty(u16 trainerId)
{
    u8 n = 0, ace;
    struct Pokemon *src;
    u16 species;
    gMpEnemyLevelBonus = 0;
    while (n < PARTY_SIZE && GetMonData(&gEnemyParty[n], MON_DATA_SPECIES) != SPECIES_NONE)
        n++;
    if (!MP_TrainerPadding() || n == 0 || n >= PARTY_SIZE)
    {
        MPNet_Log("coop: trainer %d party %d, no padding", trainerId, n);
        return;
    }
    // The ace (the last one) moves to the end; the new Moemon goes in before it,
    // like the one before the ace (or the ace itself when it's alone; see PickFillerSpecies)
    ace = n;
    gEnemyParty[ace] = gEnemyParty[n - 1];
    src = n >= 2 ? &gEnemyParty[n - 2] : &gEnemyParty[ace];
    species = PickFillerSpecies(src, n - 1, trainerId);
    CreateMon(&gEnemyParty[n - 1], species, GetMonData(src, MON_DATA_LEVEL), GetMonData(src, MON_DATA_ATK_IV), TRUE,
              Random32(), OT_ID_RANDOM_NO_SHINY, 0);
    MPNet_Log("coop: trainer %d party %d -> %d: %d %d %d %d %d %d", trainerId, n, n + 1,
              GetMonData(&gEnemyParty[0], MON_DATA_SPECIES), GetMonData(&gEnemyParty[1], MON_DATA_SPECIES),
              GetMonData(&gEnemyParty[2], MON_DATA_SPECIES), GetMonData(&gEnemyParty[3], MON_DATA_SPECIES),
              GetMonData(&gEnemyParty[4], MON_DATA_SPECIES), GetMonData(&gEnemyParty[5], MON_DATA_SPECIES));
}

void MP_Coop_ArrangeEnemyParty(void)
{
    static const u8 sSlot[PARTY_SIZE] = {0, 3, 1, 4, 2, 5};
    struct Pokemon *tmp;
    u8 i;

    if (sCoop.count == 3)
        return;   // a triple: the first three come out (battlers 1, 3, 5), the rest wait
    tmp = Alloc(sizeof(struct Pokemon) * PARTY_SIZE);

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
// Master: a ball's target was gone, so it goes back to the thrower's bag
// (ours, or the partner's on their game)
void MP_Coop_RefundBall(u8 thrower, u16 item)
{
    if (!gMpCoopBattle || !sCoop.isMaster)
        return;
    u8 seat = SeatOfSlot(gBattlerPartyIndexes[thrower]);
    if (seat == 0 || seat >= sCoop.count)
    {
        AddBagItem(item, 1);
        MPNet_Log("co-op: ball %d went back to the bag", item);
    }
    else if (PlayerValid(sCoop.seatPlayer[seat]))
    {
        u8 buf[3] = {MPR_COOP_REFUND, item & 0xFF, item >> 8};
        Send(sCoop.seatPlayer[seat], buf, 3);
    }
}

// The partner's game: a ball we threw caught a wild Pokémon (our screen played
// the 3-shake catch). The master's result replaces these; they're only used
// when the battle is called off before the result comes.
void MP_Coop_NoteOwnCatch(u8 target)
{
    struct Pokemon *mon;
    if (!gMpCoopWild || sCoop.isMaster || target >= MAX_BATTLERS_COUNT || sCoop.catchCount >= ARRAY_COUNT(sCoop.catches))
        return;
    mon = &sCoop.catches[sCoop.catchCount];
    *mon = gEnemyParty[gBattlerPartyIndexes[target]];
    MP_Ball_SetCaught(mon, gLastUsedItem);
    sCoop.catchOwner[sCoop.catchCount] = CAUGHT_MINE;
    sCoop.catchSeat[sCoop.catchCount] = sCoop.seat;
    sCoop.catchCount++;
}

bool8 MP_Coop_IsTradedMon(u8 partyIndex)
{
    u8 owner = SeatOfSlot(partyIndex);   // the seat = its link player
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
        if (s >= base && s < base + SeatSize() && sCoop.origIndex[s - base] < PARTY_SIZE)
        {
            sCoop.learn[n].slot = sCoop.origIndex[s - base];
            sCoop.learn[n].move = sCoop.learn[i].move;
            n++;
        }
    }
    sCoop.learnCount = n;
}

// Master: each other seat gets its own Pokémon back (EXP, moves, HP), its
// level-ups, the moves it couldn't learn, its catches and the prize money.
// id, outcome, money(4), leveled, size, learnCount, learn entries (slot within
// the seat, u16 move), `size` Pokémon, then u8 count + that many caught Pokémon
static void SendResultTo(u8 seat)
{
    u8 base = SeatBase(seat), size = SeatSize();
    u8 *buf = Alloc(9 + MAX_LEARN * 3 + sizeof(struct Pokemon) * (MULTI_PARTY_SIZE + 3) + 1);
    u16 len = 0;
    u8 i, count = 0;

    buf[len++] = MPR_COOP_RESULT;
    buf[len++] = sCoop.outcome;
    buf[len++] = sCoop.money & 0xFF;
    buf[len++] = (sCoop.money >> 8) & 0xFF;
    buf[len++] = (sCoop.money >> 16) & 0xFF;
    buf[len++] = (sCoop.money >> 24) & 0xFF;
    buf[len++] = sCoop.seatLeveled[seat];
    buf[len++] = size;
    for (i = 0; i < sCoop.learnCount; i++)
    {
        if (sCoop.learn[i].slot >= base && sCoop.learn[i].slot < base + size)
            count++;
    }
    buf[len++] = count;
    for (i = 0; i < sCoop.learnCount; i++)
    {
        if (sCoop.learn[i].slot >= base && sCoop.learn[i].slot < base + size)
        {
            buf[len++] = sCoop.learn[i].slot - base;
            buf[len++] = sCoop.learn[i].move & 0xFF;
            buf[len++] = sCoop.learn[i].move >> 8;
        }
    }
    memcpy(buf + len, &gPlayerParty[base], sizeof(struct Pokemon) * size);
    len += sizeof(struct Pokemon) * size;
    count = 0;
    for (i = 0; i < sCoop.catchCount; i++)
        if (sCoop.catchSeat[i] == seat)
            count++;
    buf[len++] = count;
    for (i = 0; i < sCoop.catchCount; i++)
    {
        if (sCoop.catchSeat[i] != seat)
            continue;
        memcpy(buf + len, &sCoop.catches[i], sizeof(struct Pokemon));
        len += sizeof(struct Pokemon);
    }
    if (PlayerValid(sCoop.seatPlayer[seat]))
        Send(sCoop.seatPlayer[seat], buf, len);
    Free(buf);
}

static void SendResult(void)
{
    u8 seat;
    for (seat = 1; seat < sCoop.count; seat++)
        SendResultTo(seat);
}

static void FinishLink(void)
{
    sCoop.playerBonusHeard = 0;   // the next battle's come fresh
    gMpCoopBattle = FALSE;
    MP_Battle_CloseLink();
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
}

static void CB2_MpCoopWaitResult(void)
{
    if (!sCoop.gotResult && ++sCoop.resultWait < RESULT_WAIT_FRAMES && PlayerValid(sCoop.seatPlayer[0]))
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
        memset(mons, 0, sizeof(struct Pokemon) * MULTI_PARTY_SIZE);
        memcpy(mons, &gPlayerParty[SeatBase(sCoop.seat)], sizeof(struct Pokemon) * SeatSize());
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
        u8 mask = (1 << SeatSize()) - 1, own = gLeveledUpInBattle & mask, seat;

        for (seat = 0; seat < sCoop.count; seat++)
            sCoop.seatLeveled[seat] = (gLeveledUpInBattle >> SeatBase(seat)) & mask;
        sCoop.partnerLeveled = sCoop.seatLeveled[1];
        gLeveledUpInBattle = 0;
        SendResult();
        memset(mons, 0, sizeof(struct Pokemon) * MULTI_PARTY_SIZE);
        memcpy(mons, gPlayerParty, sizeof(struct Pokemon) * SeatSize());
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

static void GiveCaughtMon(struct Pokemon *caughtMon)
{
    char msg[96], nick[16];
    u8 nickGame[POKEMON_NAME_LENGTH + 1];
    u16 species = GetMonData(caughtMon, MON_DATA_SPECIES);
    u16 nat = SpeciesToNationalPokedexNum(species);
    u8 result;

    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return;
    GetMonData(caughtMon, MON_DATA_NICKNAME, nickGame);
    nickGame[POKEMON_NAME_LENGTH] = EOS;
    MP_GameToAscii(nick, nickGame, POKEMON_NAME_LENGTH);
    result = GiveMonToPlayer(caughtMon);
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

// After a co-op wild battle: each catch goes to whoever threw the ball, and
// each catch or win is counted once for the team
static void FinishWild(void)
{
    u8 i;
    for (i = 0; i < sCoop.catchCount; i++)
    {
        if (sCoop.catchSeat[i] != sCoop.seat)
            continue;
        GiveCaughtMon(&sCoop.catches[i]);
        MP_Events_OnWildEnd(B_OUTCOME_CAUGHT, &sCoop.catches[i]);
    }
    // CB2_EndWildBattle counts the leader's battle: the catches are counted above
    if (sCoop.isMaster && sCoop.outcome == B_OUTCOME_CAUGHT)
        gBattleOutcome = 0;
    sCoop.catchCount = 0;
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
        // Both players' achievements
        if (sCoop.count == 3)
            MP_Achieve_Count("triple", 1);   // a Triple Battle won with two friends (wild or trainer)
        if (!gMpCoopWild)
        {
            MP_Events_Did(MP_DID_COOPWIN);
            if (GymBadgeFlag(sCoop.trainerId) || gTrainers[sCoop.trainerId].trainerClass == TRAINER_CLASS_LEADER)
                MP_Events_Did(MP_DID_COOPGYM);
        }
        if (sCoop.isMaster && sCoop.count == 3)
            MP_Events_OnCoopWinGroup(sCoop.seatPlayer, sCoop.count); // every pair, counted once by the leader
        else if (sCoop.isMaster)
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
    u8 i;

    // Called off (a player left): catches made so far stay with whoever threw
    // the ball; the party itself is back to how it was before the battle
    for (i = 0; i < sCoop.catchCount; i++)
    {
        if (sCoop.catchSeat[i] != sCoop.seat)
            continue;
        MPNet_Log("co-op: battle called off, keeping a catch");
        GiveCaughtMon(&sCoop.catches[i]);
        MP_Events_OnWildEnd(B_OUTCOME_CAUGHT, &sCoop.catches[i]);
    }
    sCoop.catchCount = 0;
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
        {
            // The same transition a single battle of this kind would use
            u8 MP_GetWildBattleTransition(void);
            u8 MP_GetTrainerBattleTransition(void);
            BattleTransition_StartOnField(gMpCoopWild ? MP_GetWildBattleTransition() : MP_GetTrainerBattleTransition());
        }
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

    u8 count, seat, players[3];

    StopAsking();
    if (len < 9)
        return;
    sCoop.trainerId = data[1] | (data[2] << 8);
    sCoop.mode = data[3];
    count = data[4];
    seat = data[5];
    players[0] = data[6];
    players[1] = data[7];
    players[2] = data[8];
    if (count < 2 || count > 3 || seat == 0 || seat >= count)
        return;
    sCoop.haveLoseText = FALSE;
    if (len > 9)
    {
        for (i = 0; i < LOSE_TEXT_MAX && 9 + i < len && data[9 + i] != EOS; i++)
            sCoop.loseText[i] = data[9 + i];
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
    BeginCoopBattleGroup(seat, count, players);
    sCoop.afterBattle = NULL;
    CreateTask(Task_StartJoinerBattle, 1);
}

static bool8 TryAsk(void)
{
    struct ObjectEvent *me;
    s16 x, y;
    u8 i;
    bool8 facingCancelled = FALSE;

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
        if (i == sCoop.cancelledAsk)
        {
            facingCancelled = TRUE;   // still facing the friend we just stopped asking
            continue;
        }
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
    if (!facingCancelled && sCoop.cancelledAsk >= 0)
    {
        MPNet_Log("co-op: turned away from player %d, can ask again", sCoop.cancelledAsk + 1);
        sCoop.cancelledAsk = -1;   // turned away (or they moved): facing them again asks again
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
        sCoop.cancelledAsk = target;   // not a "no": turn away and face them again to ask again
        MPNet_Log("co-op: stopped asking player %d", target + 1);
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
    u16 best = 0xFFFF, best2 = 0xFFFF;
    s8 target = -1, target2 = -1;
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
    // the two closest (ties: the lower player id)
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        u16 d;
        if (!FriendCanJoinWild(i, me->currentCoords.x, me->currentCoords.y, &d))
            continue;
        if (d < best)
        {
            best2 = best, target2 = target;
            best = d, target = i;
        }
        else if (d < best2)
        {
            best2 = d, target2 = i;
        }
    }
    if (target < 0)
        return FALSE;
    sCoop.wildMon = gEnemyParty[0];
    // Now and then a second Pokémon jumps out too: 2 on 2 (1 on 1 each is more common).
    // A triple (rolled when the battle starts) usually has three.
    sCoop.wildDouble = (Random() % 100) < gMpWildDoublePercent && MP_RollExtraLandMon(&sCoop.wildMon2);

    LockPlayerFieldControls();
    FreezeObjectEvents();
    StopPlayerAvatar();
    sCoop.wildAsking = TRUE;
    sCoop.wildTarget = target;
    sCoop.wildReply = 0;
    sCoop.wildTarget2 = target2;
    sCoop.wildReply2 = 0;
    sCoop.wildTimer = 0;
    SendByte(target, MPR_COOP_WILD_ASK, 0);
    if (target2 >= 0)
        SendByte(target2, MPR_COOP_WILD_ASK, 0);
    MPNet_Log("co-op wild: asking player %d (distance %d)%s to jump in", target + 1, best, target2 >= 0 ? " and another" : "");
    return TRUE;
}

// players[]: us + one or two friends who said yes
static void StartWildCoop(const u8 *players, u8 count)
{
    u8 partner = players[1], i;
    gTrainerBattleOpponent_A = 0;
    gTrainerBattleOpponent_B = 0;
    gMpCoopWild = TRUE;
    if (count == 3)
    {
        // usually three wild Pokémon: the extra rolls can fail
        sCoop.wildExtra = 0;
        if (MP_RollExtraLandMon(&sCoop.wildMon2))
        {
            sCoop.wildExtra++;
            if (MP_RollExtraLandMon(&sCoop.wildMon3))
                sCoop.wildExtra++;
        }
    }
    BeginCoopBattleGroup(0, count, players);
    sCoop.mode = MODE_WILD;
    sCoop.trainerId = 0;
    sCoop.haveLoseText = FALSE;
    sCoop.afterBattle = MP_EndWildBattleCallback();
    IncrementGameStat(GAME_STAT_TOTAL_BATTLES);
    IncrementGameStat(GAME_STAT_WILD_BATTLES);
    for (i = 1; i < count; i++)
    {
        // u8 seat count, u8 their seat, the seats' players
        u8 buf[6] = {MPR_COOP_WILD_GO, count, i, players[0], players[1], count == 3 ? players[2] : 0xFF};
        Send(players[i], buf, sizeof(buf));
    }
    CreateTask(Task_StartJoinerBattle, 1);
    MPNet_Log("co-op wild: battle with player %d%s", partner + 1, count == 3 ? " and another (triple)" : "");
}

// Both asked friends answered (or the time is up): a triple with two yeses,
// a double with one, alone with none
static void UpdateWildAsking(void)
{
    bool8 ok1, ok2, done1, done2;
    u8 players[3], n = 1;

    sCoop.wildTimer++;
    ok1 = sCoop.wildReply == MPR_COOP_WILD_OK && PlayerValid(sCoop.wildTarget);
    ok2 = sCoop.wildTarget2 >= 0 && sCoop.wildReply2 == MPR_COOP_WILD_OK && PlayerValid(sCoop.wildTarget2);
    done1 = sCoop.wildReply != 0 || !PlayerValid(sCoop.wildTarget);
    done2 = sCoop.wildTarget2 < 0 || sCoop.wildReply2 != 0 || !PlayerValid(sCoop.wildTarget2);
    if (!(done1 && done2) && sCoop.wildTimer <= WILD_ASK_FRAMES)
        return;
    sCoop.wildAsking = FALSE;
    // the ones who didn't answer in time stop waiting
    if (!ok1 && sCoop.wildReply != MPR_COOP_WILD_NO && PlayerValid(sCoop.wildTarget))
        SendByte(sCoop.wildTarget, MPR_COOP_WILD_CANCEL, 0);
    if (sCoop.wildTarget2 >= 0 && !ok2 && sCoop.wildReply2 != MPR_COOP_WILD_NO && PlayerValid(sCoop.wildTarget2))
        SendByte(sCoop.wildTarget2, MPR_COOP_WILD_CANCEL, 0);
    players[0] = gMpLocalId;
    if (ok1)
        players[n++] = sCoop.wildTarget;
    if (ok2)
        players[n++] = sCoop.wildTarget2;
    sCoop.wildTarget2 = -1;
    if (n >= 2)
    {
        StartWildCoop(players, n);
        return;
    }
    gEnemyParty[0] = sCoop.wildMon;
    MPNet_Log("co-op wild: battling alone");
    MP_StartNormalWildBattle();
}

// The master's game, building the battle: one wild Pokémon for both players
// A wild Pokémon two levels up for a team of two (default moves for the new level)
static void BoostWild(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u8 level = GetMonData(mon, MON_DATA_LEVEL);
    u32 exp;
    if (species == SPECIES_NONE || level + 2 > MAX_LEVEL)
        return;
    exp = gExperienceTables[gSpeciesInfo[species].growthRate][level + 2];
    SetMonData(mon, MON_DATA_EXP, &exp);
    CalculateMonStats(mon);
    GiveMonInitialMoveset(mon);
}

void MP_Coop_SetWildEnemyParty(void)
{
    ZeroEnemyPartyMons();
    gEnemyParty[0] = sCoop.wildMon;
    BoostWild(&gEnemyParty[0]);
    if (sCoop.count == 3)
    {
        // A triple: battlers 1, 3, 5 take slots 0, 1, 2 (no interleaving). With
        // two, they stand left and right; the center stays empty.
        if (sCoop.wildExtra >= 1)
        {
            gEnemyParty[1] = sCoop.wildMon2;
            BoostWild(&gEnemyParty[1]);
        }
        if (sCoop.wildExtra >= 2)
        {
            gEnemyParty[2] = sCoop.wildMon3;
            BoostWild(&gEnemyParty[2]);
        }
        return;
    }
    if (sCoop.wildDouble)
    {
        gEnemyParty[1] = sCoop.wildMon2;   // MP_Coop_ArrangeEnemyParty puts it on the right
        BoostWild(&gEnemyParty[1]);
    }
}

// Both games: a co-op wild battle against two wild Pokémon (the right one is enemy party slot 3)
bool8 MP_Coop_WildHasTwo(void)
{
    if (gMpCoopWild && IS_TRIPLE_BATTLE())
        return GetMonData(&gEnemyParty[1], MON_DATA_SPECIES) != SPECIES_NONE;
    return gMpCoopWild && GetMonData(&gEnemyParty[3], MON_DATA_SPECIES) != SPECIES_NONE;
}

// Both games: this foe battler's wild Pokémon slot is empty (it starts absent)
bool8 MP_Coop_WildSlotEmpty(u8 battler)
{
    if (!gMpCoopWild || GetBattlerSide(battler) != B_SIDE_OPPONENT)
        return FALSE;
    return GetMonData(&gEnemyParty[gBattlerPartyIndexes[battler]], MON_DATA_SPECIES) == SPECIES_NONE;
}

// Both games: after a catch, is another wild Pokémon still out?
bool8 MP_Coop_OtherWildOut(u8 caughtBattler)
{
    u8 b;
    if (!gMpCoopWild)
        return FALSE;
    for (b = 1; b < gBattlersCount; b += 2)
        if (b != caughtBattler && !(gAbsentBattlerFlags & gBitTable[b]) && gBattleMons[b].hp != 0)
            return TRUE;
    return FALSE;
}

// Cmd_givecaughtmon in a co-op wild double (the master's game decides)
void MP_Coop_OnWildCaught(u8 throwerPartyIndex, struct Pokemon *mon)
{
    if (!sCoop.isMaster || sCoop.catchCount >= ARRAY_COUNT(sCoop.catches))
        return;
    // The thrower's seat from its battle party slot
    sCoop.catches[sCoop.catchCount] = *mon;
    sCoop.catchSeat[sCoop.catchCount] = SeatOfSlot(throwerPartyIndex);
    sCoop.catchOwner[sCoop.catchCount] = sCoop.catchSeat[sCoop.catchCount] == 0 ? CAUGHT_MINE : CAUGHT_PARTNER;
    MPNet_Log("co-op wild: catch %d for seat %d: species %d, HP %d/%d", sCoop.catchCount, sCoop.catchSeat[sCoop.catchCount],
              (int)GetMonData(mon, MON_DATA_SPECIES), (int)GetMonData(mon, MON_DATA_HP), (int)GetMonData(mon, MON_DATA_MAX_HP));
    sCoop.catchCount++;
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

static void OnWildRelay(u8 from, const u8 *data, u16 len)
{
    u8 id = data[0];
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
        else if (sCoop.wildAsking && from == sCoop.wildTarget2 && sCoop.wildReply2 == 0)
            sCoop.wildReply2 = id;
        else if (id == MPR_COOP_WILD_OK)
            SendByte(from, MPR_COOP_WILD_CANCEL, 0);   // we already gave up
        break;
    case MPR_COOP_WILD_GO:
        if (sCoop.wildFrom == from)
        {
            // u8 count, u8 our seat, the seats' players (older: nothing = a double)
            u8 players[3] = {from, gMpLocalId, 0xFF}, count = 2, seat = 1;
            if (len >= 6 && data[1] >= 2 && data[1] <= 3 && data[2] >= 1 && data[2] < data[1])
            {
                count = data[1];
                seat = data[2];
                players[0] = data[3];
                players[1] = data[4];
                players[2] = data[5];
            }
            sCoop.wildFrom = -1;
            gTrainerBattleOpponent_A = 0;
            gTrainerBattleOpponent_B = 0;
            gMpCoopWild = TRUE;
            BeginCoopBattleGroup(seat, count, players);
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

static u8 sHeldBy;

// ---------------------------------------------------------------------------
// Co-op battles: instead of "Link standby...", a player who has chosen sees
// what their partner is still doing (each game sends its menu state).

// Per seat (a triple has two allies)
static u8 sMyBattleStatus, sAllyStatus[3];
static struct { u8 kind, target; u16 value; } sAllyIntent[3];
#define sPartnerBattleStatus sAllyStatus[sShownSeat]
#define sPartnerIntent sAllyIntent[sShownSeat]
static u8 sShownSeat = 1;   // the ally the standby line is about

static void SendToMates(const u8 *buf, u16 len)
{
    u8 i;
    for (i = 0; i < sCoop.count && i < 3; i++)
        if (i != sCoop.seat && PlayerValid(sCoop.seatPlayer[i]))
            Send(sCoop.seatPlayer[i], buf, len);
}

// The ally to talk about: the first (in seat order) still choosing, else the first one
static u8 PickShownSeat(void)
{
    u8 i, first = 0xFF;
    for (i = 0; i < sCoop.count && i < 3; i++)
    {
        if (i == sCoop.seat)
            continue;
        if (first == 0xFF)
            first = i;
        if (sAllyStatus[i] != MP_BST_DONE)
            return i;
    }
    return first == 0xFF ? 1 : first;
}
static bool8 sStandbyShown, sStandbyDirty, sStandbyPrinting;
static EWRAM_DATA u8 sStandbyText[80];

static const u8 sText_PartnerDeciding[] = _("{STR_VAR_1} is deciding\nwhat to do…");
static const u8 sText_PartnerMove[] = _("{STR_VAR_1} is selecting\na move to use…");
static const u8 sText_PartnerBag[] = _("{STR_VAR_1} is choosing\nan item to use…");
static const u8 sText_PartnerParty[] = _("{STR_VAR_1} is choosing a\nMOéMON to switch in…");
static const u8 sText_PartnerDone[] = _("Waiting for {STR_VAR_1}…");

// We chose (a move and target, an item, a switch, RUN): tell the partner
void MP_Coop_SetIntent(u8 kind, u16 value, u8 target)
{
    u8 buf[6] = {MPR_COOP_BSTATUS, MP_BST_DONE, kind, value & 0xFF, value >> 8, target};
    if (!gMpCoopBattle)
        return;
    sMyBattleStatus = MP_BST_DONE;
    MPNet_Log("co-op: intent %d value %d target %d", kind, value, target);
    SendToMates(buf, sizeof(buf));
}

static u8 *AppendBattlerName(u8 *p, u8 battler)
{
    u8 name[POKEMON_NAME_LENGTH + 2];
    struct Pokemon *party = GetBattlerSide(battler) == B_SIDE_PLAYER ? gPlayerParty : gEnemyParty;
    GetMonData(&party[gBattlerPartyIndexes[battler]], MON_DATA_NICKNAME, name);
    StringGet_Nickname(name);
    return StringCopy(p, name);
}

static const u8 sText_IntentWantsRun[] = _(" wants to RUN!");
static const u8 sText_IntentFoes[] = _("the foes");
static const u8 sText_IntentEveryone[] = _("everyone");
static const u8 sText_IntentSwitch[] = _("switch to ");
static const u8 sText_IntentRunShort[] = _("RUN");

// A triple: one short line per ally that has chosen ("BOB: EMBER")
static bool8 TripleIntentText(u8 *dst)
{
    u8 *p = dst, i, lines = 0;
    for (i = 0; i < 3; i++)
    {
        u8 kind = sAllyIntent[i].kind;
        u16 value = sAllyIntent[i].value;
        if (i == sCoop.seat || sAllyStatus[i] != MP_BST_DONE || kind == MP_INTENT_NONE || !PlayerValid(sCoop.seatPlayer[i]))
            continue;
        if (lines++)
            *p++ = CHAR_NEWLINE;
        SetNameVar(sCoop.seatPlayer[i]);
        p = StringCopy(p, gStringVar1);
        *p++ = CHAR_COLON;
        *p++ = CHAR_SPACE;
        switch (kind)
        {
        case MP_INTENT_MOVE:
            if (value != MOVE_NONE && value < MOVES_COUNT)
                p = StringCopy(p, gMoveNames[value]);
            break;
        case MP_INTENT_BALL:
        case MP_INTENT_ITEM:
            CopyItemName(value, p);
            p += StringLength(p);
            break;
        case MP_INTENT_SWITCH:
            if (value < PARTY_SIZE)
            {
                u8 name[POKEMON_NAME_LENGTH + 2];
                GetMonData(&gPlayerParty[value], MON_DATA_NICKNAME, name);
                StringGet_Nickname(name);
                p = StringCopy(p, name);
            }
            break;
        case MP_INTENT_RUN:
            p = StringCopy(p, sText_IntentRunShort);
            break;
        }
    }
    *p = EOS;
    return lines != 0;
}

// "BOB: EMBER" / "→ ZIGZAGOON" (game text, two lines) once the partner has chosen this turn
bool8 MP_Coop_PartnerIntentText(u8 *dst)
{
    u8 *p = dst;
    u8 kind, target;
    u16 value;

    if (!gMpCoopBattle)
        return FALSE;
    if (sCoop.count == 3)
        return TripleIntentText(dst);
    sShownSeat = sCoop.seat == 0 ? 1 : 0;
    kind = sPartnerIntent.kind, target = sPartnerIntent.target;
    value = sPartnerIntent.value;
    if (sPartnerBattleStatus != MP_BST_DONE || kind == MP_INTENT_NONE || !PlayerValid(sCoop.partner))
        return FALSE;
    SetNameVar(sCoop.partner);
    p = StringCopy(p, gStringVar1);
    if (kind == MP_INTENT_RUN)
    {
        StringCopy(p, sText_IntentWantsRun);
        return TRUE;
    }
    *p++ = CHAR_COLON;
    *p++ = CHAR_SPACE;
    switch (kind)
    {
    case MP_INTENT_MOVE:
    {
        u8 flags;
        if (value == MOVE_NONE || value >= MOVES_COUNT)
            return FALSE;
        p = StringCopy(p, gMoveNames[value]);
        flags = gBattleMoves[value].target;
        if (flags & (MOVE_TARGET_USER | MOVE_TARGET_OPPONENTS_FIELD | MOVE_TARGET_DEPENDS))
            break;
        *p++ = CHAR_NEWLINE;
        *p++ = CHAR_RIGHT_ARROW;
        *p++ = CHAR_SPACE;
        if (flags & MOVE_TARGET_BOTH)
            p = StringCopy(p, sText_IntentFoes);
        else if (flags & MOVE_TARGET_FOES_AND_ALLY)
            p = StringCopy(p, sText_IntentEveryone);
        else if (target < MAX_BATTLERS_COUNT)
            p = AppendBattlerName(p, target);
        else
            p -= 3;
        break;
    }
    case MP_INTENT_BALL:
        CopyItemName(value, p);
        p += StringLength(p);
        if (target < MAX_BATTLERS_COUNT)
        {
            *p++ = CHAR_NEWLINE;
            *p++ = CHAR_RIGHT_ARROW;
            *p++ = CHAR_SPACE;
            p = AppendBattlerName(p, target);
        }
        break;
    case MP_INTENT_ITEM:
        CopyItemName(value, p);
        p += StringLength(p);
        if (target < PARTY_SIZE)
        {
            u8 name[POKEMON_NAME_LENGTH + 2];
            *p++ = CHAR_NEWLINE;
            *p++ = CHAR_RIGHT_ARROW;
            *p++ = CHAR_SPACE;
            GetMonData(&gPlayerParty[target], MON_DATA_NICKNAME, name);
            StringGet_Nickname(name);
            p = StringCopy(p, name);
        }
        break;
    case MP_INTENT_SWITCH:
        p = StringCopy(p, sText_IntentSwitch);
        if (value < PARTY_SIZE)
        {
            u8 name[POKEMON_NAME_LENGTH + 2];
            GetMonData(&gPlayerParty[value], MON_DATA_NICKNAME, name);
            StringGet_Nickname(name);
            p = StringCopy(p, name);
        }
        break;
    }
    *p = EOS;
    return TRUE;
}

void MP_Coop_SetBattleStatus(u8 status)
{
    if (!gMpCoopBattle || status == sMyBattleStatus)
        return;
    sMyBattleStatus = status;
    {
        u8 buf[2] = {MPR_COOP_BSTATUS, status};
        SendToMates(buf, 2);
    }
}

static void PrintStandby(void)
{
    static const u8 *const sTexts[] = {
        [MP_BST_NONE] = sText_PartnerDone,
        [MP_BST_ACTION] = sText_PartnerDeciding,
        [MP_BST_MOVE] = sText_PartnerMove,
        [MP_BST_BAG] = sText_PartnerBag,
        [MP_BST_PARTY] = sText_PartnerParty,
        [MP_BST_DONE] = sText_PartnerDone,
    };
    u8 st;
    sShownSeat = PickShownSeat();
    st = sPartnerBattleStatus < ARRAY_COUNT(sTexts) ? sPartnerBattleStatus : MP_BST_NONE;
    SetNameVar(sCoop.seatPlayer[sShownSeat]);
    StringExpandPlaceholders(sStandbyText, sTexts[st]);
    sStandbyPrinting = TRUE;
    BattlePutTextOnWindow(sStandbyText, B_WIN_MSG);
    sStandbyPrinting = FALSE;
    sStandbyShown = TRUE;
    sStandbyDirty = FALSE;
}

// The local player chose and waits: TRUE when the partner's status was shown
bool8 MP_Coop_ShowStandby(void)
{
    if (!gMpCoopBattle || !PlayerValid(sCoop.partner))
        return FALSE;
    PrintStandby();
    return TRUE;
}

void MP_Coop_OnBattleMessage(void)
{
    if (!sStandbyPrinting)
        sStandbyShown = FALSE;
}

static void UpdateBattleStatus(void)
{
    if (!gMpCoopBattle)
    {
        sMyBattleStatus = MP_BST_NONE;
        memset(sAllyStatus, MP_BST_NONE, sizeof(sAllyStatus));
        memset(sAllyIntent, 0, sizeof(sAllyIntent));
        sStandbyShown = FALSE;
        return;
    }
    // Their status changed while we show it: redraw (once the last print is done)
    if (sStandbyShown && sStandbyDirty && gMain.inBattle && !IsTextPrinterActive(B_WIN_MSG))
        PrintStandby();
}

// A trainer some other player is holding while they wait for a partner
bool8 MP_Coop_IsNpcHeld(struct ObjectEvent *obj)
{
    u8 i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        if (sHeld[i].ttl && PlayerValid(i) && obj->localId == sHeld[i].localId
         && obj->mapNum == sHeld[i].mapNum && obj->mapGroup == sHeld[i].mapGroup)
        {
            sHeldBy = i;
            return TRUE;
        }
    }
    return FALSE;
}

// Talking to a held trainer (right after MP_Coop_IsNpcHeld said so)
void MP_Coop_OnHeldNpcTalk(void)
{
    SetNameVar(sHeldBy);
    StringExpandPlaceholders(sText, sText_HeldNpc);
    MP_Toast(sText);
}

static void UpdateHeld(void)
{
    u8 i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        if (!sHeld[i].ttl)
            continue;
        if (--sHeld[i].ttl == 0 || !PlayerValid(i))
        {
            sHeld[i].ttl = 0;
            MP_Emote_ClearBubbleFor(i);
        }
        else
        {
            MP_Emote_KeepBubbleFor(i, sText_JoinMe);
        }
    }
}

void MP_Coop_Update(void)
{
    UpdateBattleStatus();
    TryRunPendingScript();
    UpdateHeld();

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
    case MPR_COOP_WAIT:
        if (from < MP_MAX_PLAYERS && len >= 5)
        {
            if (data[1])
            {
                if (!sHeld[from].ttl)
                    MPNet_Log("co-op: player %d is waiting for a partner at NPC %d", from + 1, data[2]);
                sHeld[from].ttl = WAIT_HOLD_FRAMES;
                sHeld[from].localId = data[2];
                sHeld[from].mapNum = data[3];
                sHeld[from].mapGroup = data[4];
            }
            else if (sHeld[from].ttl)
            {
                sHeld[from].ttl = 0;
                MP_Emote_ClearBubbleFor(from);
            }
        }
        break;
    case MPR_COOP_REFUND:
        if (gMpCoopBattle && from == sCoop.seatPlayer[0] && len >= 3)
        {
            u16 item = data[1] | (data[2] << 8);
            if (MP_IsBallItem(item))
            {
                AddBagItem(item, 1);
                MPNet_Log("co-op: ball %d went back to the bag", item);
            }
        }
        break;
    case MPR_COOP_BONUS:
        // sent before the link handshake, so it's here long before any EXP
        if (len >= 3)
        {
            sCoop.bonusFrom = from;
            sCoop.bonusPercent = data[1] | (data[2] << 8);
            if (from < MP_MAX_PLAYERS)
            {
                sCoop.playerBonus[from] = sCoop.bonusPercent;
                sCoop.playerBonusHeard |= 1 << from;
            }
        }
        break;
    case MPR_COOP_BSTATUS:
        if (gMpCoopBattle && IsBattleMate(from) && len >= 2)
        {
            // + u8 kind, u16 value, u8 target once they've chosen (MP_Coop_SetIntent)
            u8 kind = len >= 6 ? data[2] : MP_INTENT_NONE, seat = SeatOf(from);
            if (data[1] == sAllyStatus[seat] && kind == sAllyIntent[seat].kind)
                break;
            sAllyStatus[seat] = data[1];
            sAllyIntent[seat].kind = kind;
            sAllyIntent[seat].value = len >= 6 ? (data[3] | (data[4] << 8)) : 0;
            sAllyIntent[seat].target = len >= 6 ? data[5] : 0xFF;
            sStandbyDirty = TRUE;
            MP_BattleRefreshActionPrompt();
        }
        break;
    case MPR_COOP_REQUEST:
        if (sCoop.challenged && sCoop.waitingThird && from != sCoop.requester && !PlayerValid(sCoop.requester2))
        {
            sCoop.requester2 = from;   // the third player
            break;
        }
        // A third asking while we're still answering the second (or a moment before we
        // start waiting for one): keep them for a Triple Battle instead of turning them away
        if (sCoop.challenged && !sCoop.accepted && PlayerValid(sCoop.requester) && from != sCoop.requester
         && !PlayerValid(sCoop.requester2) && TripleEligible())
        {
            sCoop.requester2 = from;
            MPNet_Log("co-op: player %d asked too, kept for a triple", from + 1);
            break;
        }
        if (sCoop.challenged && !sCoop.accepted && PlayerValid(sCoop.requester) && from != sCoop.requester
         && !TripleEligible())
        {
            SendByte(from, MPR_COOP_DECLINE, DECLINE_NO_TRIPLE);
            break;
        }
        if (!sCoop.challenged || sCoop.accepted)
        {
            SendByte(from, MPR_COOP_DECLINE, DECLINE_TOO_LATE);   // the battle is on its way
            MPNet_Log("co-op: player %d asked too late", from + 1);
        }
        else if (sCoop.prompting && from == sCoop.promptedFor && !PlayerValid(sCoop.requester))
        {
            sCoop.requester = from;   // stopped asking, then asked again while their question is up
            MPNet_Log("co-op: player %d asked again", from + 1);
        }
        else if (sCoop.prompting || (sCoop.requester >= 0 && sCoop.requester != from && PlayerValid(sCoop.requester)))
        {
            SendByte(from, MPR_COOP_DECLINE, DECLINE_BUSY);
            MPNet_Log("co-op: player %d asked while busy (requester %d, requester2 %d, waiting third %d)",
                      from + 1, sCoop.requester + 1, sCoop.requester2 + 1, sCoop.waitingThird);
        }
        else
            sCoop.requester = from;
        break;
    case MPR_COOP_CANCEL:
        if (sCoop.requester == from && !sCoop.accepted)
            sCoop.requester = -1;
        if (sCoop.requester2 == from && !sCoop.accepted)
            sCoop.requester2 = -1;
        break;
    case MPR_COOP_ACCEPT:
        if (sCoop.role == ROLE_ASKING && from == sCoop.target)
            OnAccepted(from, data, len);
        break;
    case MPR_COOP_DECLINE:
        if (sCoop.role == ROLE_ASKING && from == sCoop.target)
        {
            static const u8 *const sReasons[] = {sText_SaidNo, sText_TheyHaveOne, sText_TooLate, sText_Busy, sText_NoTripleAsker};
            u8 reason = len > 1 && data[1] < ARRAY_COUNT(sReasons) ? data[1] : DECLINE_BUSY;
            StopAsking();
            // Only a real "no" stops us asking them again (busy / too late: try again)
            if (reason == DECLINE_NO || reason == DECLINE_ONE_MON)
                sCoop.declinedBy = from;
            ShowAskerMessage(sReasons[reason], from);
        }
        break;
    case MPR_COOP_WILD_ASK:
    case MPR_COOP_WILD_OK:
    case MPR_COOP_WILD_NO:
    case MPR_COOP_WILD_GO:
    case MPR_COOP_WILD_CANCEL:
        OnWildRelay(from, data, len);
        break;
    case MPR_COOP_RESOLVE:
        if (gMpCoopBattle && !sCoop.isMaster && from == sCoop.seatPlayer[0])
            OnResolve(data, len);
        break;
    case MPR_COOP_RESOLVE_DONE:
        if (gMpCoopBattle && sCoop.isMaster)
            OnResolveDone(from, data, len);
        break;
    case MPR_COOP_RESULT:
        if (gMpCoopBattle && !sCoop.isMaster && from == sCoop.seatPlayer[0] && len >= 9)
        {
            u16 pos = 9;
            u8 i, size = data[7], count = data[8];
            sCoop.outcome = data[1];
            sCoop.money = data[2] | (data[3] << 8) | (data[4] << 16) | ((u32)data[5] << 24);
            sCoop.partnerLeveled = sCoop.outcome == B_OUTCOME_WON ? data[6] : 0;
            sCoop.learnCount = 0;
            if (size > MULTI_PARTY_SIZE)
                break;
            for (i = 0; i < count && pos + 3 <= len; i++, pos += 3)
            {
                u8 slot = data[pos];   // within our seat
                u16 move = data[pos + 1] | (data[pos + 2] << 8);
                if (slot < size && move < MOVES_COUNT && sCoop.origIndex[slot] < PARTY_SIZE && sCoop.learnCount < MAX_LEARN)
                {
                    sCoop.learn[sCoop.learnCount].slot = sCoop.origIndex[slot];
                    sCoop.learn[sCoop.learnCount].move = move;
                    sCoop.learnCount++;
                }
            }
            if (pos + sizeof(struct Pokemon) * size <= len)
            {
                memset(sCoop.resultMons, 0, sizeof(sCoop.resultMons));
                memcpy(sCoop.resultMons, data + pos, sizeof(struct Pokemon) * size);
                sCoop.gotResult = TRUE;
                pos += sizeof(struct Pokemon) * size;
                sCoop.catchCount = 0;   // the master's list replaces what our screen noted
                // We caught wild Pokémon
                if (gMpCoopWild && pos < len)
                {
                    u8 n = data[pos++], k;
                    for (k = 0; k < n && k < ARRAY_COUNT(sCoop.catches) && pos + sizeof(struct Pokemon) <= len; k++)
                    {
                        memcpy(&sCoop.catches[k], data + pos, sizeof(struct Pokemon));
                        sCoop.catchOwner[k] = CAUGHT_MINE;
                        sCoop.catchSeat[k] = sCoop.seat;
                        pos += sizeof(struct Pokemon);
                    }
                    sCoop.catchCount = k;
                }
            }
        }
        break;
    }
}

void MP_Coop_OnPlayerLeft(u8 id)
{
    if (id < MP_MAX_PLAYERS && sHeld[id].ttl)
    {
        sHeld[id].ttl = 0;
        MP_Emote_ClearBubbleFor(id);
    }
    if (sCoop.bonusFrom == id)
        sCoop.bonusFrom = -1;
    if (sCoop.wildFrom == id)
        StopWaitingForWild();
    if (sCoop.requester == id)
        sCoop.requester = -1;
    if (sCoop.requester2 == id)
        sCoop.requester2 = -1;
    if (sCoop.accepted && sCoop.partner == id)
        sCoop.accepted = FALSE;
    if (sCoop.declinedBy == id)
        sCoop.declinedBy = -1;
    if (sCoop.cancelledAsk == id)
        sCoop.cancelledAsk = -1;
    // An asker waiting on this player is told by UpdateAsking
}

// ---------------------------------------------------------------------------
// Level-ups in a co-op battle, everyone at once: each round the master hands
// every player their own level-up stat boxes and the moves that need one
// forgotten; all players (the master too) go through theirs on their own game
// at the same time (battle_script_commands.c, MP_Coop_StartResolveTask) and the
// master waits until everyone is done (mpcoopresolve).

#define MAX_RESOLVE_LEARN 12
static EWRAM_DATA struct
{
    u8 nBox, nLearn;
    u8 boxSlot[PARTY_SIZE];
    u16 before[PARTY_SIZE][NUM_STATS];
    u8 learnSlot[MAX_RESOLVE_LEARN];
    u16 learnMove[MAX_RESOLVE_LEARN];
    u8 answer[MAX_RESOLVE_LEARN];
    bool8 pending[MP_MAX_PLAYERS];
    bool8 selfPending;
} sResolve = {0};

void MP_Coop_ResolveReset(void)
{
    memset(&sResolve, 0, sizeof(sResolve));
}

void MP_Coop_ResolveAddBox(u8 slot, const u16 *before)
{
    if (sResolve.nBox >= PARTY_SIZE)
        return;
    sResolve.boxSlot[sResolve.nBox] = slot;
    memcpy(sResolve.before[sResolve.nBox], before, sizeof(sResolve.before[0]));
    sResolve.nBox++;
}

void MP_Coop_ResolveAddLearn(u8 slot, u16 move)
{
    if (sResolve.nLearn >= MAX_RESOLVE_LEARN)
        return;
    sResolve.learnSlot[sResolve.nLearn] = slot;
    sResolve.learnMove[sResolve.nLearn] = move;
    sResolve.answer[sResolve.nLearn] = 0xFF;
    sResolve.nLearn++;
}

bool8 MP_Coop_ResolveAny(void)
{
    return sResolve.nBox || sResolve.nLearn;
}

static bool8 ResolveMine(u8 slot, s8 player)
{
    return MP_Coop_SlotOwnerPlayer(slot) == player;
}

// Sends each player their part and starts ours
void MP_Coop_ResolveStart(void)
{
    u8 buf[4 + PARTY_SIZE * 13 + MAX_RESOLVE_LEARN * 15];
    u8 boxSlot[PARTY_SIZE], learnSlot[MAX_RESOLVE_LEARN], nb, nl, i, k;
    u16 before[PARTY_SIZE][NUM_STATS], learnMove[MAX_RESOLVE_LEARN];
    s8 p;

    for (p = 0; p < MP_MAX_PLAYERS; p++)
    {
        u32 len = 3;
        sResolve.pending[p] = FALSE;
        if (!PlayerValid(p))
            continue;
        nb = nl = 0;
        buf[0] = MPR_COOP_RESOLVE;
        for (i = 0; i < sResolve.nBox; i++)
        {
            if (!ResolveMine(sResolve.boxSlot[i], p))
                continue;
            buf[len++] = sResolve.boxSlot[i];
            for (k = 0; k < NUM_STATS; k++)
            {
                buf[len++] = sResolve.before[i][k] & 0xFF;
                buf[len++] = sResolve.before[i][k] >> 8;
            }
            nb++;
        }
        for (i = 0; i < sResolve.nLearn; i++)
        {
            struct Pokemon *mon = &gPlayerParty[sResolve.learnSlot[i]];
            if (!ResolveMine(sResolve.learnSlot[i], p))
                continue;
            buf[len++] = sResolve.learnSlot[i];
            buf[len++] = sResolve.learnMove[i] & 0xFF;
            buf[len++] = sResolve.learnMove[i] >> 8;
            // its moves as we have them (their copy may miss one learned earlier in the battle)
            for (k = 0; k < MAX_MON_MOVES; k++)
            {
                u16 m = GetMonData(mon, MON_DATA_MOVE1 + k);
                buf[len++] = m & 0xFF;
                buf[len++] = m >> 8;
            }
            for (k = 0; k < MAX_MON_MOVES; k++)
                buf[len++] = GetMonData(mon, MON_DATA_PP1 + k);
            nl++;
        }
        if (nb == 0 && nl == 0)
            continue;
        buf[1] = nb;
        buf[2] = nl;
        sResolve.pending[p] = TRUE;
        MPNet_Log("co-op levels: player %d has %d box(es), %d move(s) to choose", p + 1, nb, nl);
        Send(p, buf, len);
    }

    nb = nl = 0;
    for (i = 0; i < sResolve.nBox; i++)
    {
        if (!ResolveMine(sResolve.boxSlot[i], -1))
            continue;
        boxSlot[nb] = sResolve.boxSlot[i];
        memcpy(before[nb], sResolve.before[i], sizeof(before[0]));
        nb++;
    }
    for (i = 0; i < sResolve.nLearn; i++)
    {
        if (!ResolveMine(sResolve.learnSlot[i], -1))
            continue;
        learnSlot[nl] = sResolve.learnSlot[i];
        learnMove[nl] = sResolve.learnMove[i];
        nl++;
    }
    sResolve.selfPending = (nb || nl);
    if (sResolve.selfPending)
        MP_Coop_StartResolveTask(nb, boxSlot, before, nl, learnSlot, learnMove);
}

// The answers for one player's learns, in the order they were sent
static void ResolveAnswers(s8 player, const u8 *answers, u8 n)
{
    u8 i, k = 0;
    for (i = 0; i < sResolve.nLearn; i++)
    {
        if (!ResolveMine(sResolve.learnSlot[i], player))
            continue;
        sResolve.answer[i] = (k < n && answers[k] < MAX_MON_MOVES) ? answers[k] : 0xFF;
        k++;
    }
}

static void OnResolveDone(s8 from, const u8 *data, u32 len)
{
    if (from < 0 || from >= MP_MAX_PLAYERS || !sResolve.pending[from] || len < 2 || len < 2u + data[1])
        return;
    ResolveAnswers(from, data + 2, data[1]);
    sResolve.pending[from] = FALSE;
    MPNet_Log("co-op levels: player %d is done", from + 1);
}

// TRUE while someone is still looking at a stat box or choosing a move
bool8 MP_Coop_ResolveWaiting(void)
{
    s8 p;
    if (sResolve.selfPending && !MP_Coop_ResolveTaskActive())
        sResolve.selfPending = FALSE;
    if (sResolve.selfPending)
        return TRUE;
    for (p = 0; p < MP_MAX_PLAYERS; p++)
    {
        if (!sResolve.pending[p])
            continue;
        if (!PlayerValid(p) || !gMpCoopBattle)
            sResolve.pending[p] = FALSE;   // gone: their moves stay as they were
        else
            return TRUE;
    }
    return FALSE;
}

u8 MP_Coop_ResolveLearnCount(void)
{
    return sResolve.nLearn;
}

// The learn and the move slot forgotten for it (0xFF = didn't learn it)
u8 MP_Coop_ResolveLearn(u8 i, u8 *slot, u16 *move)
{
    *slot = sResolve.learnSlot[i];
    *move = sResolve.learnMove[i];
    return sResolve.answer[i];
}

// Our task is done (any game): the master keeps the answers, the others send them
void MP_Coop_ResolveFinished(const u8 *answers, u8 n)
{
    u8 buf[2 + MAX_RESOLVE_LEARN], i;
    if (sCoop.isMaster)
    {
        ResolveAnswers(-1, answers, n);
        sResolve.selfPending = FALSE;
        return;
    }
    if (n > MAX_RESOLVE_LEARN)
        n = MAX_RESOLVE_LEARN;
    buf[0] = MPR_COOP_RESOLVE_DONE;
    buf[1] = n;
    for (i = 0; i < n; i++)
        buf[2 + i] = answers[i];
    MPNet_Log("co-op levels: done (%d move(s) chosen)", n);
    Send(sCoop.seatPlayer[0], buf, 2 + n);
}

static void OnResolve(const u8 *data, u32 len)
{
    u8 boxSlot[PARTY_SIZE], learnSlot[MAX_RESOLVE_LEARN], nb, nl, i, k;
    u16 before[PARTY_SIZE][NUM_STATS], learnMove[MAX_RESOLVE_LEARN];
    u32 pos = 3;

    if (len < 3)
        return;
    nb = data[1];
    nl = data[2];
    if (nb > PARTY_SIZE || nl > MAX_RESOLVE_LEARN || len < 3u + nb * 13 + nl * 15)
        return;
    for (i = 0; i < nb; i++)
    {
        boxSlot[i] = data[pos++];
        for (k = 0; k < NUM_STATS; k++, pos += 2)
            before[i][k] = data[pos] | (data[pos + 1] << 8);
    }
    for (i = 0; i < nl; i++)
    {
        struct Pokemon *mon;
        u8 slot = data[pos++];
        learnSlot[i] = slot;
        learnMove[i] = data[pos] | (data[pos + 1] << 8);
        pos += 2;
        mon = slot < PARTY_SIZE ? &gPlayerParty[slot] : NULL;
        // the first time we see this Pokémon, its moves as the master has them
        for (k = 0; k < i && learnSlot[k] != slot; k++)
            ;
        if (mon != NULL && k == i)
        {
            for (k = 0; k < MAX_MON_MOVES; k++)
            {
                u16 m = data[pos + k * 2] | (data[pos + k * 2 + 1] << 8);
                u8 pp = data[pos + MAX_MON_MOVES * 2 + k];
                SetMonData(mon, MON_DATA_MOVE1 + k, &m);
                SetMonData(mon, MON_DATA_PP1 + k, &pp);
            }
        }
        pos += MAX_MON_MOVES * 3;
    }
    MPNet_Log("co-op levels: %d box(es), %d move(s) to choose", nb, nl);
    MP_Coop_StartResolveTask(nb, boxSlot, before, nl, learnSlot, learnMove);
}
