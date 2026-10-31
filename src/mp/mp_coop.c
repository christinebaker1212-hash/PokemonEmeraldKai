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
#include "mp_region.h"
#include "mp_join.h"
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
#include "window.h"
#include "constants/battle.h"
#include "constants/battle_setup.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/qz_port.h"
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
    MPR_COOP_ACCEPT,                    // challenged -> asker: u16 trainer, u8 mode, u8 count, u8 seat, 3x u8 player,
                                        // u16 second trainer (0 = none), lose text, EOS, second lose text, EOS
    MPR_COOP_DECLINE,                   // challenged -> asker: u8 reason
    MPR_COOP_RESULT,                    // master -> partner after the battle
    MPR_COOP_WILD_ASK,                  // wild encounter -> closest friend in the grass: can you join?
    MPR_COOP_WILD_OK,                   // yes (they stop and wait)
    MPR_COOP_WILD_NO,                   // can't right now
    MPR_COOP_WILD_GO,                   // the battle starts
    MPR_COOP_WILD_CANCEL,               // too slow: battling alone after all
    MPR_COOP_WAIT,                      // challenged -> all: u8 on, u8 localId, mapNum, mapGroup of the trainer,
                                        // u8 localId of a second one (two trainers; 0 = none)
    MPR_COOP_BSTATUS,                   // in a co-op battle -> partner: u8 MP_BST_* (what we're choosing)
    MPR_COOP_BONUS,                     // battle start -> partner: u16 own EXP bonus percent (MP_ExpBonusPercent)
    MPR_COOP_REFUND,                    // master -> partner: u16 ball that went back to your bag (its target was gone)
    MPR_COOP_RESOLVE,                   // master -> owner: their level-up boxes and full-move-set learns this round
                                        // (u8 nBox, u8 nLearn, nBox * {slot, 6 u16 stats before},
                                        // nLearn * {slot, u16 move, 4 u16 moves, 4 u8 PP}), all players at once
    MPR_COOP_RESOLVE_DONE,              // owner -> master: u8 n, n * u8 move slot forgotten (0xFF = didn't learn)
    MPR_COOP_MERGE,                     // waiting -> waiting on the same trainer: we waited first, join us
    MPR_COOP_AGE_ASK,                   // waiting -> waiting on the same trainer: how long have you waited?
    MPR_COOP_AGE,                       // the answer: u16 frames (the asker adds half the round trip)
};

#define WAIT_RESEND_FRAMES 60    // the waiting player repeats MPR_COOP_WAIT this often
#define JOIN_FRIEND_FRAMES (60 * 12)   // joining a friend's battle from our trainer's script: give up after this
#define MERGE_COOLDOWN     180   // two players waiting on the same trainer: decide (again) at most this often
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
    DECLINE_JOIN_AFTER,  // TEAM PADDING: no seat at the start, but JOIN it as soon as it starts (sCoop.joinAfter)
};
static u8 ThirdDecline(void);

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

// Moemon Kai: JOHTO's and KANTO's gym leaders (their usual and their alternate team), the badge and the script
// that runs after their first battle (where their own script goes on after the battle command)
extern const u8 Johto_VioletGym_EventScript_83FF117[];
extern const u8 Johto_AzaleaGym_EventScript_83C4C1A[];
extern const u8 Johto_GoldenrodGym_EventScript_83D5919[];
extern const u8 Johto_EcruteakGym_EventScript_83CDCCB[];
extern const u8 Johto_CianwoodGym_EventScript_83CA55E[];
extern const u8 Johto_OlivineGym_EventScript_83E5064[];
extern const u8 Johto_MahoganyGym_EventScript_83DE683[];
extern const u8 Johto_BlackthornGym1F_EventScript_83C7418[];
extern const u8 Kanto_PewterCity_Gym_EventScript_841A5B6[];
extern const u8 Kanto_CeruleanCity_Gym_EventScript_841AAD2[];
extern const u8 Kanto_VermilionCity_Gym_EventScript_841B972[];
extern const u8 Kanto_CeladonCity_Gym_EventScript_841D2EF[];
extern const u8 Kanto_FuchsiaCity_Gym_EventScript_841D70C[];
extern const u8 Kanto_SaffronCity_Gym_EventScript_841EE9D[];
extern const u8 Kanto_CinnabarIsland_Gym_EventScript_841DC0C[];
extern const u8 Kanto_ViridianCity_Gym_EventScript_8419EEB[];
static const struct { u16 trainer, badge; const u8 *script; } sQzGymLeaders[] =
{
    {TRAINER_QZ_4115, FLAG_QZ_0x11B2, Johto_VioletGym_EventScript_83FF117},
    {TRAINER_QZ_4963, FLAG_QZ_0x11B2, Johto_VioletGym_EventScript_83FF117},
    {TRAINER_QZ_4692, FLAG_QZ_0x11B3, Johto_AzaleaGym_EventScript_83C4C1A},
    {TRAINER_QZ_4964, FLAG_QZ_0x11B3, Johto_AzaleaGym_EventScript_83C4C1A},
    {TRAINER_QZ_4700, FLAG_QZ_0x11B4, Johto_GoldenrodGym_EventScript_83D5919},
    {TRAINER_QZ_4965, FLAG_QZ_0x11B4, Johto_GoldenrodGym_EventScript_83D5919},
    {TRAINER_QZ_4704, FLAG_QZ_0x11B5, Johto_EcruteakGym_EventScript_83CDCCB},
    {TRAINER_QZ_4966, FLAG_QZ_0x11B5, Johto_EcruteakGym_EventScript_83CDCCB},
    {TRAINER_QZ_4606, FLAG_QZ_0x11B6, Johto_CianwoodGym_EventScript_83CA55E},
    {TRAINER_QZ_4967, FLAG_QZ_0x11B6, Johto_CianwoodGym_EventScript_83CA55E},
    {TRAINER_QZ_4747, FLAG_QZ_0x11B7, Johto_OlivineGym_EventScript_83E5064},
    {TRAINER_QZ_4971, FLAG_QZ_0x11B7, Johto_OlivineGym_EventScript_83E5064},
    {TRAINER_QZ_4803, FLAG_QZ_0x11B8, Johto_MahoganyGym_EventScript_83DE683},
    {TRAINER_QZ_4975, FLAG_QZ_0x11B8, Johto_MahoganyGym_EventScript_83DE683},
    {TRAINER_QZ_4637, FLAG_QZ_0x11B9, Johto_BlackthornGym1F_EventScript_83C7418},
    {TRAINER_QZ_4976, FLAG_QZ_0x11B9, Johto_BlackthornGym1F_EventScript_83C7418},
    {TRAINER_QZ_8606, FLAG_QZ_0x2100, Kanto_PewterCity_Gym_EventScript_841A5B6},
    {TRAINER_QZ_8607, FLAG_QZ_0x2101, Kanto_CeruleanCity_Gym_EventScript_841AAD2},
    {TRAINER_QZ_8608, FLAG_QZ_0x2102, Kanto_VermilionCity_Gym_EventScript_841B972},
    {TRAINER_QZ_8609, FLAG_QZ_0x2103, Kanto_CeladonCity_Gym_EventScript_841D2EF},
    {TRAINER_QZ_8610, FLAG_QZ_0x2104, Kanto_FuchsiaCity_Gym_EventScript_841D70C},
    {TRAINER_QZ_8612, FLAG_QZ_0x2105, Kanto_SaffronCity_Gym_EventScript_841EE9D},
    {TRAINER_QZ_8611, FLAG_QZ_0x2106, Kanto_CinnabarIsland_Gym_EventScript_841DC0C},
    {TRAINER_QZ_8542, FLAG_QZ_0x2107, Kanto_ViridianCity_Gym_EventScript_8419EEB},
};

static u8 QzGymLeader(u16 trainerId)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sQzGymLeaders); i++)
        if (sQzGymLeaders[i].trainer == trainerId)
            return i;
    return 0xFF;
}

// Any trainer of the three regions (HOENN's, then JOHTO's and KANTO's after FIRST_QZ_TRAINER)
bool8 MP_IsRealTrainer(u16 trainerId)
{
    return IS_REAL_TRAINER(trainerId);
}

// The badge a gym leader gives (0: not a gym leader)
static u16 GymBadgeFlag(u16 trainerId)
{
    if (QzGymLeader(trainerId) != 0xFF)
        return sQzGymLeaders[QzGymLeader(trainerId)].badge;
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

// The gym a leader battles in (their "you won" script is one of its objects')
const u8 *MP_FindTrainerEndScriptOnMap(u16 trainerId, u16 mapGroup, u16 mapNum);   // battle_setup.c
static const u8 *GymLeaderEndScript(u16 trainerId)
{
    u16 map;
    const u8 *script;
    if (QzGymLeader(trainerId) != 0xFF)
        return sQzGymLeaders[QzGymLeader(trainerId)].script;
    script = MP_FindTrainerEndScript(trainerId);   // (in the gym: this map's)
    if (script != NULL)
        return script;
    switch (trainerId)
    {
    case TRAINER_ROXANNE_1:       map = MAP_RUSTBORO_CITY_GYM; break;
    case TRAINER_BRAWLY_1:        map = MAP_DEWFORD_TOWN_GYM; break;
    case TRAINER_WATTSON_1:       map = MAP_MAUVILLE_CITY_GYM; break;
    case TRAINER_FLANNERY_1:      map = MAP_LAVARIDGE_TOWN_GYM_1F; break;
    case TRAINER_NORMAN_1:        map = MAP_PETALBURG_CITY_GYM; break;
    case TRAINER_WINONA_1:        map = MAP_FORTREE_CITY_GYM; break;
    case TRAINER_TATE_AND_LIZA_1: map = MAP_MOSSDEEP_CITY_GYM; break;
    case TRAINER_JUAN_1:          map = MAP_SOOTOPOLIS_CITY_GYM_1F; break;
    default:                      return NULL;
    }
    return MP_FindTrainerEndScriptOnMap(trainerId, MAP_GROUP(map), MAP_NUM(map));
}

static struct
{
    // Challenged player
    bool8 challenged;
    u16 trainerId;
    u8 mode;
    bool8 twoTrainers;     // two trainers spotted us at once: the battle is against both (Steven-style 2 vs 2)
    u16 trainerIdB;        // the second one (0: none)
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
    u8 waitLocalId2;                            // two trainers: the second one (0 = none)
    u16 waitAge;                                // frames we've been waiting (MPR_COOP_WAIT: who waited first)
    u16 mergeCooldown;
    s8 mergeTarget;        // a friend waiting on the same trainer whose battle we join (-1 = none)
    s8 mergeAsk;           // who we asked how long they've waited (-1 = nobody)
    u16 mergeAskAge;       // our waitAge when we asked
    s8 heldJoin;           // a trainer this player is waiting on spotted us: their battle (-1 = none)
    bool8 joiningFromScript;   // asking to join from inside our trainer's script (no asker messages, no B)
    bool8 joinedOther;     // we went into a friend's battle from our trainer's script (MP_Coop_TakeJoinedOther)
    // Asking player
    u8 role;
    u8 target;
    u8 targetName[MP_NAME_LEN + 1];
    u16 timer;
    s8 declinedBy;         // don't ask this player again until their challenge ends
    s8 joinAfter;          // DECLINE_JOIN_AFTER: JOIN this player's battle once it's running (MP_Coop_Update)
    u16 joinAfterTimer;
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
    u8 preEvo;             // real party slots that leveled up but left the battle when friends joined (mp_join.c)
    struct LearnLater learnOut[MAX_LEARN];   // ... and moves they wanted to learn (real party slots)
    u8 learnOutCount;
    bool8 learnPromptPending;
    bool8 haveLoseText;
    u8 loseText[LOSE_TEXT_MAX + 1];
    bool8 haveLoseTextB;   // two trainers: the second one's
    u8 loseTextB[LOSE_TEXT_MAX + 1];
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
} sCoop = { .requester = -1, .requester2 = -1, .declinedBy = -1, .joinAfter = -1, .cancelledAsk = -1, .promptedFor = -1, .wildFrom = -1, .bonusFrom = -1, .wildTarget2 = -1,
            .mergeTarget = -1, .heldJoin = -1, .mergeAsk = -1 };

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
    u8 localId2;   // two trainers waiting together (0 = none)
} sHeld[MP_MAX_PLAYERS];
static u8 sHeldBy;   // MP_Coop_IsNpcHeld: whose

static EWRAM_DATA u8 sText[160];

static const u8 sText_JoinPrompt[] = _("{STR_VAR_1} wants to join for a\nDouble Battle! Is this OK?");
static const u8 sText_OneMon[] = _("Hey, I only have one MOéMON!\nLet's go one on one!");
static const u8 sText_Asking[] = _("Asking {STR_VAR_1} to team up for a\nDouble Battle… ({B_BUTTON} never mind)");
static const u8 sText_SaidNo[] = _("{STR_VAR_1} wants to battle alone\nthis time.");
static const u8 sText_TheyHaveOne[] = _("{STR_VAR_1}'s opponent only has one\nMOéMON. It's a one-on-one battle!");
static const u8 sText_TooLate[] = _("Too late! {STR_VAR_1}'s battle has\nalready started.");
static const u8 sText_Busy[] = _("{STR_VAR_1} can't team up right now.");
static const u8 sText_NoTripleAsker[] = _("{STR_VAR_1}'s opponent can't do a Triple\nBattle. It's a Double Battle this time!");
static const u8 sText_JoinAfter[] = _("{STR_VAR_1}'s battle is starting.\nYou'll join it in a moment!");
static const u8 sText_NoTripleFew[] = _("This trainer has fewer than 3 MOéMON:\nno Triple Battle. It's a Double!");
static const u8 sText_NoTripleKind[] = _("This battle can't be a Triple Battle.\nIt's a Double!");
static const u8 sText_Left[] = _("{STR_VAR_1} left the game.");
static const u8 sText_WaitAsk[] = _("Wait for another trainer to join you?");
static const u8 sText_WaitMore[] = _("{STR_VAR_1} has joined!\nWait for another?");
static const u8 sText_Waiting[] = _("Waiting for a partner to join…\n({B_BUTTON} battle alone)");
static const u8 sText_JoinMe[] = _("Join me!");
static const u8 sText_TeamUpWith[] = _("You team up with {STR_VAR_1}!");
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

// How much of a text fits in MAX bytes: all of it, else up to the end of the last whole paragraph that fits (never
// half a control code: a speech cut after {PAUSE_MUSIC} and before {RESUME_MUSIC} left the music paused)
u16 MP_TextFitLength(const u8 *s, u16 max)
{
    u16 i = 0, lastPara = 0, lastChar = 0;
    if (s == NULL)
        return 0;
    while (s[i] != EOS)
    {
        u16 n = 1;
        if (s[i] == EXT_CTRL_CODE_BEGIN)
            n = 1 + GetExtCtrlCodeLength(s[i + 1]);
        else if (s[i] == PLACEHOLDER_BEGIN)
            n = 2;
        if (i + n > max)
            return lastPara != 0 ? lastPara : lastChar;
        if (s[i] == CHAR_PROMPT_CLEAR)
            lastPara = i;   // (the text ends where the next paragraph would start)
        i += n;
        lastChar = i;
    }
    return i;
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
    return sCoop.challenged && sCoop.heldJoin < 0 && !sCoop.joiningFromScript;
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

// A co-op battle against two trainers at once (they spotted the player together)
bool8 MP_Coop_TwoTrainers(void)
{
    return gMpCoopBattle && !gMpCoopWild && (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) && gTrainerBattleOpponent_B != 0;
}

// Two trainers: is the Pokémon that battler sends out (party slot partyIdx) the second trainer's? Their own side in a
// double; in a Triple Battle against both (a friend joined) they share the field, and the slot says whose it is
bool8 MP_Coop_SecondTrainersMon(u8 battler, u8 partyIdx)
{
    if (!MP_Coop_TwoTrainers() || !(battler & BIT_SIDE))
        return FALSE;
    if (IS_TRIPLE_BATTLE())
        return partyIdx >= MULTI_PARTY_SIZE && partyIdx < PARTY_SIZE;
    return GetBattlerPosition(battler) == B_POSITION_OPPONENT_RIGHT;
}

// The trainer picture of an opponent battler: two trainers, the right one is the second
u16 MP_Coop_TrainerPicFor(u8 battler)
{
    if (MP_Coop_TwoTrainers() && GetBattlerPosition(battler) == B_POSITION_OPPONENT_RIGHT)
        return gTrainers[gTrainerBattleOpponent_B].trainerPic;
    return MP_Coop_TrainerPic();
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

const u8 *MP_Coop_LoseTextB(void)
{
    if (gMpCoopBattle && !sCoop.isMaster && sCoop.haveLoseTextB)
        return sCoop.loseTextB;
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
    if (gNoOfApproachingTrainers == 2 && gApproachingTrainerId == 1)
    {
        // The second of two trainers who spotted us at once: the battle is against both of them
        if (sCoop.challenged && sCoop.twoTrainers)
        {
            mode = GetTrainerBattleMode();
            if (mode == TRAINER_BATTLE_SINGLE || mode == TRAINER_BATTLE_CONTINUE_SCRIPT
             || mode == TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC)
                sCoop.trainerIdB = gTrainerBattleOpponent_B;
            else
                sCoop.challenged = FALSE;   // a kind of trainer this can't handle: the game's own battle
        }
        return;
    }
    sCoop.challenged = FALSE;
    sCoop.accepted = FALSE;
    sCoop.requester = -1;
    sCoop.twoTrainers = FALSE;
    sCoop.trainerIdB = 0;
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
    if (gNoOfApproachingTrainers == 2)
    {
        if (mode == TRAINER_BATTLE_SINGLE || mode == TRAINER_BATTLE_CONTINUE_SCRIPT
         || mode == TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC)
            sCoop.twoTrainers = TRUE;   // the second one is configured next
        else
            sCoop.challenged = FALSE;
    }
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
    if (sCoop.twoTrainers && sCoop.trainerIdB != 0 && trainerId == sCoop.trainerId)
    {
        // two trainers: up to three each, and TEAM PADDING's one extra
        u8 b = gTrainers[sCoop.trainerIdB].partySize;
        n = (n > 3 ? 3 : n) + (b > 3 ? 3 : b);
        if (MP_TrainerPadding() && n < PARTY_SIZE)
            n++;
        return n;
    }
    if (MP_TrainerPadding() && n < PARTY_SIZE)
        n++;
    return n;
}

// A Triple Battle needs a trainer with 3+ Moemon (TEAM PADDING counted) and an
// ordinary one-trainer battle (not Twins and the like; rematches and battles
// without intro text are fine). A Double Battle needs 2+.
static void OnResolve(const u8 *data, u32 len);
static void OnResolveDone(s8 from, const u8 *data, u32 len);

// With TEAM PADDING on, a third player is never turned away from one trainer: the trainer is padded up to three
// Moemon for the Triple Battle (MP_Coop_FinishTrainerParty), whatever kind of battle it was (Twins too).
static bool8 TripleEligible(void)
{
    bool8 single = sCoop.mode == TRAINER_BATTLE_SINGLE || sCoop.mode == TRAINER_BATTLE_CONTINUE_SCRIPT
                || sCoop.mode == TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC || sCoop.mode == TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT
                || sCoop.mode == TRAINER_BATTLE_REMATCH;
    if (sCoop.twoTrainers)
        return FALSE;   // two trainers: 2 players vs both of them
    if (MP_TrainerPadding())
        return single || sCoop.mode == TRAINER_BATTLE_DOUBLE || sCoop.mode == TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE
            || sCoop.mode == TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE_NO_MUSIC || sCoop.mode == TRAINER_BATTLE_REMATCH_DOUBLE;
    return single && CoopTrainerTeamSize(sCoop.trainerId) >= 3;
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
    JOIN_JOINING,              // a friend waiting on the same trainer waited first: we join their battle
};

enum { THIRD_NONE, THIRD_ASK, THIRD_QUEUED };

static void SendWait(bool8 on)
{
    u8 buf[8] = {MPR_COOP_WAIT, on, sCoop.waitLocalId, sCoop.waitMapNum, sCoop.waitMapGroup, sCoop.waitLocalId2,
                 sCoop.waitAge & 0xFF, sCoop.waitAge >> 8};
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
    u8 buf[11 + (LOSE_TEXT_MAX + 1) * 2 + 2];
    const u8 *lose;
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
        SendByte(sCoop.requester2, MPR_COOP_DECLINE, ThirdDecline());
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
    buf[9] = sCoop.twoTrainers ? (sCoop.trainerIdB & 0xFF) : 0;
    buf[10] = sCoop.twoTrainers ? (sCoop.trainerIdB >> 8) : 0;
    // the lose texts as this game has them (the second trainer's after the first's)
    lose = GetTrainerALoseText();
    {
        u16 fit = MP_TextFitLength(lose, LOSE_TEXT_MAX);
        while (len < fit)
        {
            buf[11 + len] = lose[len];
            len++;
        }
    }
    buf[11 + len++] = EOS;
    if (sCoop.twoTrainers)
    {
        u16 k = 0, fit;
        lose = GetTrainerBLoseText();
        fit = MP_TextFitLength(lose, LOSE_TEXT_MAX);
        while (k < fit)
            buf[11 + len++] = lose[k++];
    }
    buf[11 + len++] = EOS;
    {
        // the weather here: the battle starts in it on every screen (only this game runs the battle)
        u16 field = MP_BattleWeather_FieldFlags();
        buf[11 + len++] = field & 0xFF;
        buf[11 + len++] = field >> 8;
    }
    for (i = 1; i < sCoop.joinCount; i++)
    {
        buf[5] = i;
        Send(sCoop.joinPlayers[i], buf, 11 + len);
    }
    sCoop.accepted = TRUE;
    sCoop.partner = sCoop.requester;
    MPNet_Log("co-op: player %d joins the battle against trainer %d%s%s", sCoop.partner + 1, sCoop.trainerId,
              sCoop.twoTrainers ? " and the second trainer" : "", sCoop.joinCount == 3 ? " (and another: triple)" : "");
}

// The trainer (or the two) that spotted us: others leave them alone while we wait
static void NoteHeldTrainers(void)
{
    u8 obj = gSelectedObjectEvent;
    sCoop.waitLocalId2 = 0;
    if (sCoop.twoTrainers)
    {
        obj = gApproachingTrainers[0].objectEventId;
        if (gApproachingTrainers[1].objectEventId < OBJECT_EVENTS_COUNT)
            sCoop.waitLocalId2 = gObjectEvents[gApproachingTrainers[1].objectEventId].localId;
    }
    if (obj < OBJECT_EVENTS_COUNT)
    {
        sCoop.waitLocalId = gObjectEvents[obj].localId;
        sCoop.waitMapNum = gObjectEvents[obj].mapNum;
        sCoop.waitMapGroup = gObjectEvents[obj].mapGroup;
    }
}

// The NPC the others hold while we wait (and the "waiting" message they get)
static void BeginWaitStage(void)
{
    if (sCoop.waitStage != WAIT_WAITING)
    {
        if (sCoop.waitStage == WAIT_NONE)
            NoteHeldTrainers();
        sCoop.waitStage = WAIT_WAITING;
        sCoop.mergeTarget = -1;
    }
    sCoop.waitTimer = 0;
    SendWait(TRUE);
}

// A friend who asked but gets no seat when the battle starts (two trainers, or a moment too late): with TEAM PADDING
// on nobody is ever turned away: they JOIN it as soon as it's running (mp_join.c; the battle grows to a triple)
static u8 ThirdDecline(void)
{
    return MP_TrainerPadding() ? DECLINE_JOIN_AFTER : DECLINE_TOO_LATE;
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

static void StartJoiningFriend(u8 host);
static void JoinedFriend(void);
static void StopAsking(void);

static void FinishJoinCheck(u8 taskId)
{
    if (!sCoop.accepted && PlayerValid(sCoop.requester2))
        SendByte(sCoop.requester2, MPR_COOP_DECLINE, ThirdDecline());
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

static bool8 sAloneChosen;   // BATTLE ALONE picked when this trainer walked up (no "wait for a partner?" then)

static void Task_JoinCheck(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (tState)
    {
    case JOIN_START:
        if (sCoop.challenged && sCoop.twoTrainers && gApproachingTrainerId == 0)
        {
            // Two trainers: the co-op steps come after the second one has spoken. Nothing changes meanwhile
            // (a friend who asks now is answered then).
            ScriptContext_Enable();
            DestroyTask(taskId);
            break;
        }
        if (!sCoop.challenged || (gNoOfApproachingTrainers == 2 && !sCoop.twoTrainers))
        {
            FinishJoinCheck(taskId);
            break;
        }
        if (sCoop.twoTrainers)
            MPNet_Log("co-op: two trainers (%d and %d): the team-up steps", sCoop.trainerId, sCoop.trainerIdB);
        if (!PlayerValid(sCoop.requester))
        {
            // Nobody asked yet: offer to wait for a friend on this map
            if (CoopTrainerTeamSize(sCoop.trainerId) <= 1 || !FriendOnMap() || gSelectedObjectEvent >= OBJECT_EVENTS_COUNT
             || sAloneChosen)
            {
                sAloneChosen = FALSE;
                FinishJoinCheck(taskId);
                break;
            }
            NoteHeldTrainers();
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
        sCoop.waitAge = 0;
        sCoop.mergeTarget = -1;
        sCoop.mergeCooldown = 0;
        SendWait(TRUE);
        MP_ReplaceFieldMessage(sText_Waiting);
        MPNet_Log("co-op: waiting for a partner (trainer %d%s)", sCoop.trainerId, sCoop.twoTrainers ? " and another" : "");
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
        if (PlayerValid(sCoop.mergeTarget))
        {
            // a friend waiting on the same trainer waited first: their battle, we join it
            u8 host = sCoop.mergeTarget;
            sCoop.mergeTarget = -1;
            StopWaiting();
            StartJoiningFriend(host);
            data[1] = 0;
            tState = JOIN_JOINING;
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
    case JOIN_JOINING:
        if (gMpCoopBattle)
        {
            // they let us in (OnAccepted): the battle starts; our trainer's script goes on after it
            // (MP_Coop_TakeJoinedOther ends it there)
            JoinedFriend();
            sCoop.prompting = FALSE;
            gTextFlags.canABSpeedUpPrint = FALSE;
            DestroyTask(taskId);
            break;
        }
        if (sCoop.role != ROLE_ASKING || !PlayerValid(sCoop.target) || ++data[1] >= JOIN_FRIEND_FRAMES)
        {
            // not after all: back to waiting
            if (sCoop.role == ROLE_ASKING)
            {
                if (PlayerValid(sCoop.target))
                    SendByte(sCoop.target, MPR_COOP_CANCEL, 0);
                StopAsking();
            }
            sCoop.joiningFromScript = FALSE;
            MPNet_Log("co-op: couldn't join, waiting again");
            BeginWaitStage();
            MP_ReplaceFieldMessage(sText_Waiting);
            tState = JOIN_WAITING;
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
    sCoop.joinedOther = FALSE;
    CreateTask(Task_JoinCheck, 80);
}

// ---------------------------------------------------------------------------
// Joining a friend's battle from inside our own trainer's script: their trainer spotted us (they're waiting on it),
// or we both wait on the same trainer and they waited first. The trainer script waits; once they let us in, the
// battle starts and the script ends afterwards (CB2_MpCoopFinish returns to it).

// Tests (mp coopask N): ask player N to team up, as walking up to them does
void MP_Coop_DebugAsk(u8 i)
{
    if (!PlayerValid(i) || sCoop.role != ROLE_NONE)
        return;
    sCoop.role = ROLE_ASKING;
    sCoop.target = i;
    StringCopyN(sCoop.targetName, gMpPlayers[i].name, MP_NAME_LEN);
    sCoop.targetName[MP_NAME_LEN] = EOS;
    sCoop.timer = 0;
    SendByte(i, MPR_COOP_REQUEST, 0);
    MPNet_Log("co-op: asking player %d to team up", i + 1);
}

static void StartJoiningFriend(u8 host)
{
    sCoop.role = ROLE_ASKING;
    sCoop.target = host;
    StringCopyN(sCoop.targetName, gMpPlayers[host].name, MP_NAME_LEN);
    sCoop.targetName[MP_NAME_LEN] = EOS;
    sCoop.timer = 0;
    sCoop.joiningFromScript = TRUE;
    sCoop.joinedOther = FALSE;
    SendByte(host, MPR_COOP_REQUEST, 1);
    SetNameVar(host);
    StringExpandPlaceholders(sText, sText_TeamUpWith);
    HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
    MP_ReplaceFieldMessage(sText);
    MPNet_Log("co-op: joining player %d's battle (the same trainer)", host + 1);
}

static void JoinedFriend(void)
{
    sCoop.joiningFromScript = FALSE;
    sCoop.joinedOther = TRUE;
    sCoop.challenged = FALSE;   // our own challenge is theirs now
    sCoop.accepted = FALSE;
    sCoop.requester = -1;
    sCoop.requester2 = -1;
    MPNet_Log("co-op: in player %d's battle", sCoop.target + 1);
}

// A trainer some friend is waiting on may spot us (trainer_see.c): we're free to team up
bool8 MP_Coop_CanJoinHeldTrainer(void)
{
    return MP_IsConnected() && gMpLocalId >= 0 && !gMpCoopBattle && sCoop.role == ROLE_NONE && !sCoop.challenged
        && sCoop.waitStage == WAIT_NONE && !sCoop.wildAsking && sCoop.wildFrom < 0 && HasUsableMon();
}

// trainer_see.c: a held trainer (MP_Coop_IsNpcHeld just said whose) spotted us
void MP_Coop_NoteHeldSpotter(void)
{
    sCoop.heldJoin = sHeldBy;
    MPNet_Log("co-op: spotted by the trainer player %d is waiting on", sHeldBy + 1);
}

static void Task_JoinHeld(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (gMpCoopBattle)
    {
        JoinedFriend();
        DestroyTask(taskId);   // the battle starts (Task_StartJoinerBattle); the script goes on after it
        return;
    }
    // (while they're still on that trainer, answering "wait for another?", we wait for them: up to a minute)
    if (sCoop.role != ROLE_ASKING || !PlayerValid(sCoop.target)
     || ++data[0] >= ((gMpPlayers[sCoop.target].state.flags & MPF_CHALLENGED) ? JOIN_FRIEND_FRAMES * 5 : JOIN_FRIEND_FRAMES))
    {
        // not after all: our own battle with this trainer
        if (sCoop.role == ROLE_ASKING)
        {
            if (PlayerValid(sCoop.target))
                SendByte(sCoop.target, MPR_COOP_CANCEL, 0);
            StopAsking();
        }
        sCoop.joiningFromScript = FALSE;
        MPNet_Log("co-op: couldn't join player %d, battling alone", sCoop.target + 1);
        if (!IsFieldMessageBoxHidden())
            HideFieldMessageBox();
        gSpecialVar_Result = FALSE;
        ScriptContext_Enable();
        DestroyTask(taskId);
    }
}

static bool8 TryOfferBattleInProgress(void);

// Special, after a trainer walked up to us (data/scripts/trainer_battle.inc): VAR_RESULT TRUE = asking to join the
// friend waiting on that trainer, or asking whether to join the battle a friend is having with it (the script
// waits either way), FALSE = the usual speech and battle
void MP_Coop_TryJoinHeldTrainer(void)
{
    s8 host = sCoop.heldJoin;
    gSpecialVar_Result = FALSE;
    sCoop.heldJoin = -1;
    sCoop.joinedOther = FALSE;
    sAloneChosen = FALSE;
    if (gApproachingTrainerId != 0)
        return;
    if (host < 0)
    {
        gSpecialVar_Result = TryOfferBattleInProgress();
        return;
    }
    if (!PlayerValid(host) || !sHeld[host].ttl || sCoop.role != ROLE_NONE || gMpCoopBattle)
        return;
    StartJoiningFriend(host);
    CreateTask(Task_JoinHeld, 80);
    gSpecialVar_Result = TRUE;
}

// ---------------------------------------------------------------------------
// A trainer some friend is battling spots us: JOIN BATTLE (their battle in progress, mp_join.c) or BATTLE ALONE.
// Whoever runs a battle against a trainer on the field says so to everyone (MPR_COOP_BATTLING) every second.

#define BATTLING_PERIOD 60
#define BATTLING_TTL    (BATTLING_PERIOD * 3)

static struct
{
    u16 ttl;
    u8 mapGroup, mapNum, localId;
    u16 trainerId;
    bool8 joinable;   // their battle can take a friend now (mp_join.c)
    u8 localId2;      // two trainers at once: the second (0 = one trainer)
    u16 trainerId2;
} sBattling[MP_MAX_PLAYERS];
static u16 sBattlingTimer;
static bool8 sBattlingSent;

static void SendBattling(bool8 on)
{
    u16 trainer = gTrainerBattleOpponent_A, trainer2;
    u8 localId, localId2;
    MP_TrainerBattleLocalIds(&localId, &localId2);
    trainer2 = localId2 != 0 ? gTrainerBattleOpponent_B : 0;
    {
        u8 buf[11] = {MPR_COOP_BATTLING, on, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum,
                      localId, trainer & 0xFF, trainer >> 8, on && MP_Join_CanBeJoined(),
                      localId2, trainer2 & 0xFF, trainer2 >> 8};
        MP_SendRelay(0xFF, buf, sizeof(buf));
    }
    sBattlingSent = on;
}

// MP_Coop_Update: in a battle against a trainer met on the field, led by this game
static void UpdateBattling(void)
{
    u8 i;
    bool8 on = gMain.inBattle && (gBattleTypeFlags & BATTLE_TYPE_TRAINER) && gBattleOutcome == 0
            && MP_IsRealTrainer(gTrainerBattleOpponent_A)
            && (gMpCoopBattle ? sCoop.isMaster : !gNetLinkActive);
    // (a gym leader or other story trainer has no trainer object to spot anyone: localId 0, but friends can JOIN)
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (sBattling[i].ttl && (--sBattling[i].ttl == 0 || !PlayerValid(i)))
            sBattling[i].ttl = 0;
    if (!on)
    {
        if (sBattlingSent && MP_IsConnected())
            SendBattling(FALSE);
        sBattlingSent = FALSE;
        sBattlingTimer = 0;
        return;
    }
    if (sBattlingTimer)
    {
        sBattlingTimer--;
        return;
    }
    sBattlingTimer = BATTLING_PERIOD;
    if (MP_IsConnected())
        SendBattling(TRUE);
}

static void OnBattling(u8 from, const u8 *data, u16 len)
{
    if (from >= MP_MAX_PLAYERS || len < 11)
        return;
    if (!data[1])
    {
        sBattling[from].ttl = 0;
        return;
    }
    if (!sBattling[from].ttl)
        MPNet_Log("co-op: player %d is battling trainer %d (NPC %d)%s%s", from + 1, data[5] | (data[6] << 8), data[4],
                  data[8] ? " and a second trainer" : "", data[7] ? "" : ", not joinable");
    sBattling[from].ttl = BATTLING_TTL;
    sBattling[from].mapGroup = data[2];
    sBattling[from].mapNum = data[3];
    sBattling[from].localId = data[4];
    sBattling[from].trainerId = data[5] | (data[6] << 8);
    sBattling[from].joinable = data[7];
    sBattling[from].localId2 = data[8];
    sBattling[from].trainerId2 = data[9] | (data[10] << 8);
}

static const u8 sText_BattlingThem[] = _("{STR_VAR_1} is battling this trainer!\nJoin their battle, or battle alone?");
static const u8 sText_BattlingBoth[] = _("{STR_VAR_1} is battling these trainers!\nJoin their battle, or battle alone?");
static const u8 sText_ChoiceJoinBattle[] = _("JOIN BATTLE");
static const u8 sText_ChoiceBattleAlone[] = _("BATTLE ALONE");
static const struct MenuAction sInProgressChoices[] =
{
    {sText_ChoiceJoinBattle, {NULL}},
    {sText_ChoiceBattleAlone, {NULL}},
};
// beside the message (the yes / no box's tiles: it isn't up)
static const struct WindowTemplate sInProgressWindow =
{
    .bg = 0,
    .tilemapLeft = 17,
    .tilemapTop = 9,
    .width = 12,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x125,
};

#define tFriend data[1]
#define tWindow data[2]

static void Task_OfferBattleInProgress(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (tState)
    {
    case 0:
        if (!IsTextPrinterActive(0))
        {
            tWindow = AddWindow(&sInProgressWindow);
            DrawStdWindowFrame(tWindow, FALSE);
            PrintMenuTable(tWindow, ARRAY_COUNT(sInProgressChoices), sInProgressChoices);
            InitMenuInUpperLeftCornerNormal(tWindow, ARRAY_COUNT(sInProgressChoices), 0);
            CopyWindowToVram(tWindow, COPYWIN_FULL);
            MPNet_Log("co-op: JOIN BATTLE / BATTLE ALONE");
            tState++;
        }
        break;
    case 1:
    {
        s8 answer = Menu_ProcessInputNoWrap();
        u8 f = tFriend;
        if (answer == MENU_NOTHING_CHOSEN)
            break;
        ClearStdWindowAndFrameToTransparent(tWindow, TRUE);
        RemoveWindow(tWindow);
        HideFieldMessageBox();
        DestroyTask(taskId);
        if (answer == 0 && PlayerValid(f) && (gMpPlayers[f].state.flags & MPF_IN_BATTLE))
        {
            MPNet_Log("co-op: joining player %d's battle with this trainer", f + 1);
            if (MP_Join_AskFromScript(f))
                return;   // (mp_join.c goes on with the script)
        }
        else
        {
            MPNet_Log("co-op: battling the trainer alone (player %d is battling it too)", f + 1);
            sAloneChosen = TRUE;
        }
        ScriptContext_Enable();   // our own battle with the trainer
        break;
    }
    }
}

#undef tFriend
#undef tWindow

// The trainer walking up to us is the one a friend is battling now, and they can take a friend in: ask
static bool8 TryOfferBattleInProgress(void)
{
    struct ObjectEvent *trainer, *trainer2 = NULL;
    u8 i, taskId;
    if ((gNoOfApproachingTrainers != 1 && gNoOfApproachingTrainers != 2) || sCoop.role != ROLE_NONE || gMpCoopBattle
     || sCoop.waitStage != WAIT_NONE || MP_Join_IsAsking() || !MP_IsConnected())
        return FALSE;
    trainer = &gObjectEvents[gApproachingTrainers[0].objectEventId];
    if (gNoOfApproachingTrainers == 2)
        trainer2 = &gObjectEvents[gApproachingTrainers[1].objectEventId];
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        bool8 first, second;
        if (!sBattling[i].ttl || !sBattling[i].joinable || !PlayerValid(i) || !(gMpPlayers[i].state.flags & MPF_IN_BATTLE))
            continue;
        if (sBattling[i].mapGroup != gSaveBlock1Ptr->location.mapGroup || sBattling[i].mapNum != gSaveBlock1Ptr->location.mapNum)
            continue;
        // the trainer walking up is the one they battle, or one of the two (two trainers at once)
        first = sBattling[i].localId == trainer->localId && sBattling[i].trainerId == gTrainerBattleOpponent_A;
        second = sBattling[i].localId2 != 0 && sBattling[i].localId2 == trainer->localId
              && sBattling[i].trainerId2 == gTrainerBattleOpponent_A;
        if (!first && !second)
            continue;
        // both of them spotted us: both must be that battle's (the second one's id isn't set up yet: its object says)
        if (trainer2 != NULL && (sBattling[i].localId2 == 0
                                 || trainer2->localId != (first ? sBattling[i].localId2 : sBattling[i].localId)))
            continue;
        SetNameVar(i);
        StringExpandPlaceholders(sText, sBattling[i].localId2 != 0 ? sText_BattlingBoth : sText_BattlingThem);
        HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
        MP_ShowFieldMessageFitted(sText);
        taskId = CreateTask(Task_OfferBattleInProgress, 80);
        gTasks[taskId].data[0] = 0;
        gTasks[taskId].data[1] = i;
        MPNet_Log("co-op: spotted by the trainer player %d is battling: join or battle alone?", i + 1);
        return TRUE;
    }
    return FALSE;
}

// mp_join.c: we're in the friend's battle now (asked from our trainer's script): our own challenge is theirs, and
// the script goes on after the battle as after our own (MP_Coop_TakeJoinedOther)
void MP_Coop_JoinedFromTrainerScript(void)
{
    sCoop.joinedOther = TRUE;
    sCoop.challenged = FALSE;
    sCoop.accepted = FALSE;
    sCoop.requester = -1;
    sCoop.requester2 = -1;
}

// Specialvar: we went into a friend's battle from this trainer script. 1: won (the script ends as after its own
// battle), 2: not won (it just ends), 0: no
u8 MP_Coop_TakeJoinedOther(void)
{
    bool8 joined = sCoop.joinedOther;
    sCoop.joinedOther = FALSE;
    if (!joined)
        return 0;
    return sCoop.outcome == B_OUTCOME_WON ? 1 : 2;
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
    sCoop.preEvo = 0;
    sCoop.learnOutCount = 0;
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
    if (!gMpCoopWild && sCoop.twoTrainers && gTrainerBattleOpponent_B != 0)
        gBattleTypeFlags |= BATTLE_TYPE_TWO_OPPONENTS;   // each trainer has their own side of the field
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
            && !gMpCoopBattle
            && (sCoop.twoTrainers
                ? (gBattleTypeFlags == (BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_TRAINER)
                   && sCoop.trainerIdB == gTrainerBattleOpponent_B)
                : gBattleTypeFlags == BATTLE_TYPE_TRAINER);

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
    case TRAINER_CLASS_QZ_ROCKET_ADMIN:   // Moemon Kai: ARIANA, PROTON, PETREL, ARCHER, GIOVANNI
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
    bool8 boss = MP_Coop_IsBossTrainer(trainerId);
    if ((gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) && gTrainerBattleOpponent_B != 0)
        boss |= MP_Coop_IsBossTrainer(gTrainerBattleOpponent_B);
    gMpEnemyLevelBonus = boss ? 3 : 2;
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
            if (IS_GEN4_SPECIES(species) && !IS_GEN4_SPECIES(srcSpecies))
                continue;   // Moemon Kai: Gen 4 Moemon only where the game puts them (after the ELITE FOUR, other regions)
            if (MP_Gen_SealedSpecies(species) || !MP_Gen_Allowed(species))
                continue;   // Moemon Kai: nothing past the GENERATION (or the region's own cap)
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
// One trainer's Moemon in gEnemyParty[base .. base + size): how many
static u8 CountEnemyMons(u8 base, u8 size)
{
    u8 n = 0;
    while (n < size && GetMonData(&gEnemyParty[base + n], MON_DATA_SPECIES) != SPECIES_NONE)
        n++;
    return n;
}

// TEAM PADDING's extra Moemon for the trainer whose team is gEnemyParty[base .. base + n), room for one more
static void PadTrainer(u16 trainerId, u8 base, u8 n)
{
    u8 ace = base + n;
    struct Pokemon *src;
    u16 species;
    // The ace (the last one) moves to the end; the new Moemon goes in before it,
    // like the one before the ace (or the ace itself when it's alone; see PickFillerSpecies)
    gEnemyParty[ace] = gEnemyParty[ace - 1];
    src = n >= 2 ? &gEnemyParty[ace - 2] : &gEnemyParty[ace];
    species = PickFillerSpecies(src, n - 1, trainerId);
    MPNet_Log("coop: filler %d into slot %d (ace %d)", species, ace - 1, ace);
    CreateMon(&gEnemyParty[ace - 1], species, GetMonData(src, MON_DATA_LEVEL), GetMonData(src, MON_DATA_ATK_IV), TRUE,
              Random32(), OT_ID_RANDOM_NO_SHINY, 0);
}

// Two trainers (gEnemyParty 0-2 the first's, 3-5 the second's): TEAM PADDING gives the one with fewer Moemon (the
// first on a tie) one more, if they have room. Still one extra in all.
void MP_Coop_FinishTwoTrainerParty(u16 trainerA, u16 trainerB)
{
    u8 a = CountEnemyMons(0, 3), b = CountEnemyMons(3, 3);
    gMpEnemyLevelBonus = 0;
    if (MP_TrainerPadding() && (a < 3 || b < 3) && a != 0 && b != 0)
    {
        if (b < a)
            PadTrainer(trainerB, 3, b);
        else
            PadTrainer(trainerA, 0, a);
    }
    MPNet_Log("coop: trainers %d + %d party %d + %d -> %d %d %d / %d %d %d", trainerA, trainerB, a, b,
              GetMonData(&gEnemyParty[0], MON_DATA_SPECIES), GetMonData(&gEnemyParty[1], MON_DATA_SPECIES),
              GetMonData(&gEnemyParty[2], MON_DATA_SPECIES), GetMonData(&gEnemyParty[3], MON_DATA_SPECIES),
              GetMonData(&gEnemyParty[4], MON_DATA_SPECIES), GetMonData(&gEnemyParty[5], MON_DATA_SPECIES));
}

void MP_Coop_FinishTrainerParty(u16 trainerId)
{
    u8 n, start;
    gMpEnemyLevelBonus = 0;
    n = CountEnemyMons(0, PARTY_SIZE);
    if (!MP_TrainerPadding() || n == 0 || n >= PARTY_SIZE)
    {
        MPNet_Log("coop: trainer %d party %d, no padding", trainerId, n);
        return;
    }
    start = n;
    PadTrainer(trainerId, 0, n++);
    // a Triple Battle with TEAM PADDING: never fewer than three (one per player), so a third player is never refused
    while (sCoop.count == 3 && n < 3)
        PadTrainer(trainerId, 0, n++);
    MPNet_Log("coop: trainer %d party %d -> %d: %d %d %d %d %d %d", trainerId, start, n,
              GetMonData(&gEnemyParty[0], MON_DATA_SPECIES), GetMonData(&gEnemyParty[1], MON_DATA_SPECIES),
              GetMonData(&gEnemyParty[2], MON_DATA_SPECIES), GetMonData(&gEnemyParty[3], MON_DATA_SPECIES),
              GetMonData(&gEnemyParty[4], MON_DATA_SPECIES), GetMonData(&gEnemyParty[5], MON_DATA_SPECIES));
}

void MP_Coop_ArrangeEnemyParty(void)
{
    static const u8 sSlot[PARTY_SIZE] = {0, 3, 1, 4, 2, 5};
    struct Pokemon *tmp;
    u8 i;

    if (getenv("MOEMON_DEBUG_FOE_HP"))
    {
        // Tests: the trainer's (or wild) Moemon start with at most this much HP
        u16 hp = atoi(getenv("MOEMON_DEBUG_FOE_HP"));
        for (i = 0; i < PARTY_SIZE; i++)
            if (GetMonData(&gEnemyParty[i], MON_DATA_SPECIES) != SPECIES_NONE && GetMonData(&gEnemyParty[i], MON_DATA_HP) > hp)
                SetMonData(&gEnemyParty[i], MON_DATA_HP, &hp);
    }

    if (sCoop.count == 3)
        return;   // a triple: the first three come out (battlers 1, 3, 5), the rest wait
    if (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS)
        return;   // two trainers: each one's own half already (0-2 left, 3-5 right)
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

static void AddLearnEntry(u8 slot, u16 move)
{
    u8 i;
    for (i = 0; i < sCoop.learnCount; i++)
        if (sCoop.learn[i].slot == slot && sCoop.learn[i].move == move)
            return;
    if (sCoop.learnCount < MAX_LEARN)
    {
        sCoop.learn[sCoop.learnCount].slot = slot;
        sCoop.learn[sCoop.learnCount].move = move;
        sCoop.learnCount++;
    }
}

static void AddLearnOut(u8 slot, u16 move)
{
    if (slot < PARTY_SIZE && sCoop.learnOutCount < MAX_LEARN)
    {
        sCoop.learnOut[sCoop.learnOutCount].slot = slot;
        sCoop.learnOut[sCoop.learnOutCount].move = move;
        sCoop.learnOutCount++;
    }
}

// Puts our (up to three) Pokémon back into the real party
// Level-ups evolve after a win, or after a catch (catch EXP)
static bool8 OutcomeEvolves(void)
{
    return sCoop.outcome == B_OUTCOME_WON || sCoop.outcome == B_OUTCOME_CAUGHT;
}

static void RestoreParty(const struct Pokemon *finalMons, u8 leveledBits)
{
    u8 i;

    LoadPlayerParty();
    // the ones that leveled up before friends joined and sat the rest out still evolve after a win
    sCoop.evoQueue = OutcomeEvolves() ? sCoop.preEvo : 0;
    sCoop.preEvo = 0;
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
    // moves the ones that left the battle (friends joined) wanted to learn
    for (i = 0; i < sCoop.learnOutCount; i++)
        AddLearnEntry(sCoop.learnOut[i].slot, sCoop.learnOut[i].move);
    sCoop.learnOutCount = 0;
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
    MP_Join_OnBattleEnd();
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
        RestoreParty(mons, OutcomeEvolves() ? own : 0);
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

// A route trainer (their battle's script after it only signs them up for MATCH CALL), not a gym leader, admin, boss,
// rival or Team Aqua / Magma member whose script after the battle moves the story on
static bool8 OrdinaryTrainer(u16 trainerId)
{
    if (!MP_IsRealTrainer(trainerId) || MP_Coop_IsBossTrainer(trainerId))
        return FALSE;
    switch (gTrainers[trainerId].trainerClass)
    {
    case TRAINER_CLASS_RIVAL:
    case TRAINER_CLASS_TEAM_AQUA:
    case TRAINER_CLASS_TEAM_MAGMA:
    case TRAINER_CLASS_QZ_RIVAL:          // Moemon Kai: JOHTO's and KANTO's story trainers
    case TRAINER_CLASS_QZ_TEAM_ROCKET:
    case TRAINER_CLASS_QZ_ROCKET_ADMIN:
        return FALSE;
    }
    return TRUE;
}

static void CB2_MpCoopFinish(void)
{
    MPNet_Log("co-op: battle over, outcome %d, prize %u, lead Lv%d %u exp, money %u, %d move(s) to learn (mode %d, trainers %d %d%s)",
              sCoop.outcome, (unsigned)sCoop.money, GetMonData(&gPlayerParty[0], MON_DATA_LEVEL),
              (unsigned)GetMonData(&gPlayerParty[0], MON_DATA_EXP), (unsigned)GetMoney(&gSaveBlock1Ptr->money), sCoop.learnCount,
              sCoop.mode, sCoop.trainerId, sCoop.twoTrainers ? sCoop.trainerIdB : 0, sCoop.isMaster ? ", master" : "");
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
        // Our own trainer script was waiting on this trainer (it spotted us too): beaten here as after our own battle,
        // and the script goes on the same way (MP_Coop_TakeJoinedOther)
        if (sCoop.joinedOther)
        {
            MP_EndJoinedTrainerBattle();
            // two trainers: our script's trainer may have been either one, and both are beaten
            if (sCoop.twoTrainers && sCoop.trainerIdB != 0)
            {
                FlagSet(TRAINER_FLAG(sCoop.trainerId));
                FlagSet(TRAINER_FLAG(sCoop.trainerIdB));
            }
        }
        // Ordinary trainers count as beaten for the partner too. Story battles
        // (gym leaders, rivals...) keep their own event scripts, so the partner
        // can still battle them on their own later.
        else if (sCoop.mode == TRAINER_BATTLE_SINGLE || sCoop.mode == TRAINER_BATTLE_DOUBLE
              || (OrdinaryTrainer(sCoop.trainerId) && (!sCoop.twoTrainers || OrdinaryTrainer(sCoop.trainerIdB))
                  && (sCoop.mode == TRAINER_BATTLE_CONTINUE_SCRIPT || sCoop.mode == TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE)))
        {
            FlagSet(TRAINER_FLAG(sCoop.trainerId));
            if (sCoop.twoTrainers && sCoop.trainerIdB != 0)
                FlagSet(TRAINER_FLAG(sCoop.trainerIdB));
        }
        // Gym leaders: the partner gets the badge too (before the rematch check: a leader's first battle is in the
        // rematch table too). Their own copy of the leader's "you won" script runs once back on the field, wherever
        // they are (a friend who joined from elsewhere: the gym's script, GymLeaderEndScript).
        else if (GymBadgeFlag(sCoop.trainerId) && !FlagGet(GymBadgeFlag(sCoop.trainerId)))
        {
            sPendingScript = GymLeaderEndScript(sCoop.trainerId);
            if (sPendingScript)
                FlagSet(TRAINER_FLAG(sCoop.trainerId));
            MPNet_Log("co-op: the badge of gym leader %d %s", sCoop.trainerId,
                      sPendingScript ? "comes once back on the field" : "has no script found");
        }
        // A team rematch with a gym leader: the partner gets the reward too
        else if (MP_Rematch_IsRematchTrainer(sCoop.trainerId))
            MP_Rematch_Reward();
    }
    if (sCoop.joinedOther)
    {
        // we came from our own trainer's script (the same trainer as theirs): it ends now (MP_Coop_TakeJoinedOther)
        SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
        return;
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
    MPNet_Log("co-op: the gym leader's \"you won\" script runs for us");
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
    sCoop.preEvo = 0;
    sCoop.learnOutCount = 0;
    MP_Join_OnBattleEnd();
    gMpCoopBattle = FALSE;
    gMpCoopWild = FALSE;
    sCoop.caught = CAUGHT_NONE;
    sCoop.role = ROLE_NONE;
    sCoop.challenged = FALSE;
    sCoop.accepted = FALSE;
    sCoop.requester = -1;
    sCoop.learnCount = 0;
    sCoop.joiningFromScript = FALSE;
    sCoop.joinedOther = FALSE;
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
    u16 trainerB, pos;

    StopAsking();
    if (len < 11)
        return;
    sCoop.trainerId = data[1] | (data[2] << 8);
    sCoop.mode = data[3];
    count = data[4];
    seat = data[5];
    players[0] = data[6];
    players[1] = data[7];
    players[2] = data[8];
    trainerB = data[9] | (data[10] << 8);
    if (count < 2 || count > 3 || seat == 0 || seat >= count)
        return;
    // the lose texts: the first trainer's, then the second's (two trainers)
    pos = 11;
    for (i = 0; i < LOSE_TEXT_MAX && pos < len && data[pos] != EOS; i++)
        sCoop.loseText[i] = data[pos++];
    sCoop.loseText[i] = EOS;
    sCoop.haveLoseText = i > 0;
    pos++;
    for (i = 0; i < LOSE_TEXT_MAX && pos < len && data[pos] != EOS; i++)
        sCoop.loseTextB[i] = data[pos++];
    sCoop.loseTextB[i] = EOS;
    sCoop.haveLoseTextB = i > 0;
    pos++;
    if (pos + 2 <= len)
        MP_BattleWeather_SetMasterField(data[pos] | (data[pos + 1] << 8));   // (the master's field weather)
    if ((sCoop.trainerId != 0 && !MP_IsRealTrainer(sCoop.trainerId)) || (trainerB != 0 && !MP_IsRealTrainer(trainerB)))
        return;
    sCoop.twoTrainers = trainerB != 0;
    sCoop.trainerIdB = trainerB;
    {
        u8 t = FindTaskIdByFunc(Task_AskerMessage);
        if (t != TASK_NONE)
            DestroyTask(t);
    }
    gTrainerBattleOpponent_A = sCoop.trainerId;
    gTrainerBattleOpponent_B = trainerB;
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
    sCoop.twoTrainers = FALSE;
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
        // u8 seat count, u8 their seat, the seats' players, u16 the weather here (the battle starts in it)
        u16 field = MP_BattleWeather_FieldFlags();
        u8 buf[8] = {MPR_COOP_WILD_GO, count, i, players[0], players[1], count == 3 ? players[2] : 0xFF,
                     field & 0xFF, field >> 8};
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

// Level scaling (mp_levelscale.c): raised to the whole team's level, then the team-of-two boost
static void ScaleAndBoostWild(struct Pokemon *mon)
{
    MP_LevelScale_CoopWild(mon);
    BoostWild(mon);
}

void MP_Coop_SetWildEnemyParty(void)
{
    ZeroEnemyPartyMons();
    gEnemyParty[0] = sCoop.wildMon;
    ScaleAndBoostWild(&gEnemyParty[0]);
    if (sCoop.count == 3)
    {
        // A triple: battlers 1, 3, 5 take slots 0, 1, 2 (no interleaving). With
        // two, they stand left and right; the center stays empty.
        if (sCoop.wildExtra >= 1)
        {
            gEnemyParty[1] = sCoop.wildMon2;
            ScaleAndBoostWild(&gEnemyParty[1]);
        }
        if (sCoop.wildExtra >= 2)
        {
            gEnemyParty[2] = sCoop.wildMon3;
            ScaleAndBoostWild(&gEnemyParty[2]);
        }
        return;
    }
    if (sCoop.wildDouble)
    {
        gEnemyParty[1] = sCoop.wildMon2;   // MP_Coop_ArrangeEnemyParty puts it on the right
        ScaleAndBoostWild(&gEnemyParty[1]);
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
            if (len >= 8)
                MP_BattleWeather_SetMasterField(data[6] | (data[7] << 8));
            sCoop.wildFrom = -1;
            gTrainerBattleOpponent_A = 0;
            gTrainerBattleOpponent_B = 0;
            sCoop.twoTrainers = FALSE;
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
        if (sHeld[i].ttl && PlayerValid(i) && (obj->localId == sHeld[i].localId || (sHeld[i].localId2 != 0 && obj->localId == sHeld[i].localId2))
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

// Tests (MOEMON_PARTYTRACE=1): every party change in a co-op battle, with BAD EGGS marked
static void TraceParties(void)
{
    static s8 sOn = -1;
    static u32 sLast;
    u32 h = 2166136261u, k;
    u8 i, side;
    const u8 *p;
    if (sOn < 0)
        sOn = getenv("MOEMON_PARTYTRACE") != NULL;
    if (!sOn || !gMpCoopBattle || !gMain.inBattle)
        return;
    p = (const u8 *)gEnemyParty;
    for (k = 0; k < sizeof(gEnemyParty); k++)
        h = (h ^ p[k]) * 16777619u;
    p = (const u8 *)gPlayerParty;
    for (k = 0; k < sizeof(gPlayerParty); k++)
        h = (h ^ p[k]) * 16777619u;
    if (gBattleStruct != NULL)
        for (i = 0; i < MAX_BATTLERS_COUNT; i++)
            h = (h ^ (gBattlerPartyIndexes[i] | (gBattleStruct->monToSwitchIntoId[i] << 8))) * 16777619u;
    h = (h ^ gAbsentBattlerFlags) * 16777619u;
    if (h == sLast)
        return;
    sLast = h;
    for (side = 0; side < 2; side++)
    {
        char line[160];
        int n = 0;
        struct Pokemon *party = side ? gEnemyParty : gPlayerParty;
        for (i = 0; i < PARTY_SIZE; i++)
        {
            struct BoxPokemon copy = party[i].box;   // checking a copy: GetMonData marks a broken one
            bool8 bad = MP_IsBoxMonBadEgg(&copy);
            n += snprintf(line + n, sizeof(line) - n, " %d%s/%d", GetBoxMonData(&copy, MON_DATA_SPECIES),
                          bad ? "BADEGG" : "", party[i].hp);
        }
        MPNet_Log("partytrace: %s%s", side ? "enemy" : "player", line);
    }
    if (gBattleStruct != NULL)
        MPNet_Log("partytrace: battlers %d %d %d %d %d %d, switching to %d %d %d %d %d %d, absent %02x",
                  gBattlerPartyIndexes[0], gBattlerPartyIndexes[1], gBattlerPartyIndexes[2], gBattlerPartyIndexes[3],
                  gBattlerPartyIndexes[4], gBattlerPartyIndexes[5], gBattleStruct->monToSwitchIntoId[0],
                  gBattleStruct->monToSwitchIntoId[1], gBattleStruct->monToSwitchIntoId[2],
                  gBattleStruct->monToSwitchIntoId[3], gBattleStruct->monToSwitchIntoId[4],
                  gBattleStruct->monToSwitchIntoId[5], gAbsentBattlerFlags);
}

// DECLINE_JOIN_AFTER: once their battle says it can be joined (MPR_COOP_BATTLING) and we're free, JOIN asks for us
#define JOIN_AFTER_FRAMES (60 * 30)
static void UpdateJoinAfter(void)
{
    s8 who = sCoop.joinAfter;
    if (who < 0)
        return;
    if (!PlayerValid(who) || gMpCoopBattle || ++sCoop.joinAfterTimer > JOIN_AFTER_FRAMES)
    {
        MPNet_Log("co-op: player %d's battle never came to join", who + 1);
        sCoop.joinAfter = -1;
        return;
    }
    if (sBattling[who].ttl && sBattling[who].joinable && MP_IsLocalFree() && !MP_Join_IsAsking())
    {
        sCoop.joinAfter = -1;
        MPNet_Log("co-op: player %d's battle is on: asking to join it", who + 1);
        MP_Join_Ask(who);
    }
}

void MP_Coop_Update(void)
{
    TraceParties();
    UpdateBattleStatus();
    TryRunPendingScript();
    UpdateHeld();
    UpdateBattling();

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
    UpdateJoinAfter();

    if (sCoop.waitStage == WAIT_WAITING && sCoop.waitAge < 0xFFFF)
        sCoop.waitAge++;
    if (sCoop.mergeCooldown)
        sCoop.mergeCooldown--;
    if (sCoop.role == ROLE_ASKING)
    {
        if (!sCoop.joiningFromScript)   // (that task handles it)
            UpdateAsking();
    }
    else if (!gMpCoopBattle && !sCoop.challenged)
        TryAsk();

    if (sCoop.learnPromptPending && MP_IsLocalFree())
    {
        sCoop.learnPromptPending = FALSE;
        ScriptContext_SetupScript(EventScript_MpCoopLearnMove);
    }
}

// Only waiting for a partner, nobody in yet
static bool8 PlainWaiting(void)
{
    return sCoop.challenged && !sCoop.accepted && sCoop.waitStage == WAIT_WAITING && !sCoop.waitingThird
        && !PlayerValid(sCoop.requester) && sCoop.role == ROLE_NONE && !PlayerValid(sCoop.mergeTarget);
}

// Are we only waiting for a partner on the trainer in this MPR_COOP_WAIT?
static bool8 WaitingOnSameTrainer(const u8 *data, u16 len)
{
    u8 theirs2 = len >= 6 ? data[5] : 0;
    if (!PlainWaiting())
        return FALSE;
    if (data[3] != sCoop.waitMapNum || data[4] != sCoop.waitMapGroup)
        return FALSE;
    return data[2] == sCoop.waitLocalId || (theirs2 != 0 && theirs2 == sCoop.waitLocalId)
        || (sCoop.waitLocalId2 != 0 && (data[2] == sCoop.waitLocalId2 || theirs2 == sCoop.waitLocalId2));
}

// Two of us wait on the same trainer: whoever waited first leads, the other joins. The player with the lower
// number decides (both deciding could cross): it asks how long the other has waited (MPR_COOP_AGE_ASK) and adds
// half the round trip to the answer; MPR_COOP_MERGE tells the other one to join.
static void TryMergeWaits(u8 from, const u8 *data, u16 len)
{
    u8 buf[2] = {MPR_COOP_AGE_ASK, 0};
    if (!WaitingOnSameTrainer(data, len) || gMpLocalId < 0 || gMpLocalId > from || sCoop.mergeCooldown)
        return;
    sCoop.mergeCooldown = MERGE_COOLDOWN;
    sCoop.mergeAsk = from;
    sCoop.mergeAskAge = sCoop.waitAge;
    Send(from, buf, 2);
}

static void DecideMerge(u8 from, u16 theirAgeThen)
{
    u16 rtt = sCoop.waitAge - sCoop.mergeAskAge;
    u32 theirAge = theirAgeThen + rtt / 2;   // their age now (the answer took about half the round trip)
    sCoop.mergeAsk = -1;
    if (sCoop.waitAge >= theirAge)
    {
        u8 buf[2] = {MPR_COOP_MERGE, 0};
        MPNet_Log("co-op: player %d waits on our trainer too; we waited first (%d vs %d frames, round trip %d), they join us",
                  from + 1, sCoop.waitAge, (int)theirAge, rtt);
        Send(from, buf, 2);
    }
    else
    {
        MPNet_Log("co-op: player %d waits on our trainer too and waited first (%d vs %d frames, round trip %d): we join them",
                  from + 1, (int)theirAge, sCoop.waitAge, rtt);
        sCoop.mergeTarget = from;
    }
}

void MP_Coop_OnRelay(u8 from, const u8 *data, u16 len)
{
    switch (data[0])
    {
    case MPR_COOP_BATTLING:
        OnBattling(from, data, len);
        break;
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
                sHeld[from].localId2 = len >= 6 ? data[5] : 0;
                TryMergeWaits(from, data, len);
            }
            else if (sHeld[from].ttl)
            {
                sHeld[from].ttl = 0;
                MP_Emote_ClearBubbleFor(from);
            }
        }
        break;
    case MPR_COOP_AGE_ASK:
        if (from < MP_MAX_PLAYERS && PlainWaiting())
        {
            u8 buf[3] = {MPR_COOP_AGE, sCoop.waitAge & 0xFF, sCoop.waitAge >> 8};
            Send(from, buf, 3);
        }
        break;
    case MPR_COOP_AGE:
        if (len >= 3 && from == sCoop.mergeAsk && PlainWaiting() && PlayerValid(from) && sHeld[from].ttl)
            DecideMerge(from, data[1] | (data[2] << 8));
        break;
    case MPR_COOP_MERGE:
        // a friend waiting on the same trainer waited first: we join them (if we're still only waiting)
        if (from < MP_MAX_PLAYERS && PlayerValid(from) && sHeld[from].ttl && sCoop.challenged && !sCoop.accepted
         && sCoop.waitStage == WAIT_WAITING && !sCoop.waitingThird && !PlayerValid(sCoop.requester)
         && sCoop.role == ROLE_NONE)
        {
            MPNet_Log("co-op: player %d waited first on our trainer: we join them", from + 1);
            sCoop.mergeTarget = from;
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
            SendByte(from, MPR_COOP_DECLINE, ThirdDecline());
            break;
        }
        if (!sCoop.challenged || sCoop.accepted)
        {
            SendByte(from, MPR_COOP_DECLINE, sCoop.challenged ? ThirdDecline() : DECLINE_TOO_LATE);   // the battle is on its way
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
        if (sCoop.role == ROLE_ASKING && from == sCoop.target && sCoop.joiningFromScript)
        {
            StopAsking();   // joining from our trainer's script: its task goes on without them
            MPNet_Log("co-op: player %d can't take us (%d)", from + 1, len > 1 ? data[1] : 0);
        }
        else if (sCoop.role == ROLE_ASKING && from == sCoop.target)
        {
            static const u8 *const sReasons[] = {sText_SaidNo, sText_TheyHaveOne, sText_TooLate, sText_Busy, sText_NoTripleAsker,
                                                       sText_JoinAfter};
            u8 reason = len > 1 && data[1] < ARRAY_COUNT(sReasons) ? data[1] : DECLINE_BUSY;
            StopAsking();
            // Only a real "no" stops us asking them again (busy / too late: try again)
            if (reason == DECLINE_NO || reason == DECLINE_ONE_MON)
                sCoop.declinedBy = from;
            if (reason == DECLINE_JOIN_AFTER)
            {
                sCoop.joinAfter = from;
                sCoop.joinAfterTimer = 0;
                MPNet_Log("co-op: joining player %d's battle once it starts", from + 1);
            }
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
            sCoop.partnerLeveled = OutcomeEvolves() ? data[6] : 0;
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
    u8 buf[4 + PARTY_SIZE * 25 + MAX_RESOLVE_LEARN * 15];
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
            // its moves and PP as we have them now: the moves it learned without forgetting one are
            // already in (the owner's own copy gets them now, not after the battle)
            for (k = 0; k < MAX_MON_MOVES; k++)
            {
                u16 m = GetMonData(&gPlayerParty[sResolve.boxSlot[i]], MON_DATA_MOVE1 + k);
                buf[len++] = m & 0xFF;
                buf[len++] = m >> 8;
            }
            for (k = 0; k < MAX_MON_MOVES; k++)
                buf[len++] = GetMonData(&gPlayerParty[sResolve.boxSlot[i]], MON_DATA_PP1 + k);
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
    if (nb > PARTY_SIZE || nl > MAX_RESOLVE_LEARN || len < 3u + nb * 25 + nl * 15)
        return;
    for (i = 0; i < nb; i++)
    {
        boxSlot[i] = data[pos++];
        for (k = 0; k < NUM_STATS; k++, pos += 2)
            before[i][k] = data[pos] | (data[pos + 1] << 8);
        // its moves as the master has them (with the ones it learned without forgetting one)
        if (boxSlot[i] < PARTY_SIZE)
            for (k = 0; k < MAX_MON_MOVES; k++)
            {
                u16 m = data[pos + k * 2] | (data[pos + k * 2 + 1] << 8);
                u8 pp = data[pos + MAX_MON_MOVES * 2 + k];
                SetMonData(&gPlayerParty[boxSlot[i]], MON_DATA_MOVE1 + k, &m);
                SetMonData(&gPlayerParty[boxSlot[i]], MON_DATA_PP1 + k, &pp);
            }
        pos += MAX_MON_MOVES * 3;
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

// ---------------------------------------------------------------------------
// Joining a battle in progress (mp_join.c runs the protocol and the screens; the parties change here)
//
// A solo battle becomes a co-op double (or a triple when two friends join at once), a co-op double becomes a triple.
// Each player keeps the Pokémon they see first in the battle's party menu (the one out now first): the first 3 that
// can battle in a double, the first 2 in a triple. The rest stay in the player's real party exactly as they are now
// (SavePlayerParty / the saved copy), and anything that leveled up among them still gets to evolve after the battle.


static void SetBattleControllerFor(u8 battler, void (*setter)(void))
{
    u8 saved = gActiveBattler;
    gActiveBattler = battler;
    setter();
    gActiveBattler = saved;
}

static u8 JoinPick(const u8 *order, u8 n, u8 size, struct Pokemon *party, u8 *pick)
{
    u8 i, got = 0;
    for (i = 0; i < n && got < size; i++)
        if (order[i] < PARTY_SIZE && IsUsableMon(&party[order[i]]))
            pick[got++] = order[i];
    return got;
}

// The battle's party menu order for battler 0 of a solo battle (the one out first)
static void SoloMenuOrder(u8 *order)
{
    u8 i;
    for (i = 0; i < PARTY_SIZE / 2; i++)
    {
        order[i * 2] = gBattleStruct->battlerPartyOrders[0][i] >> 4;
        order[i * 2 + 1] = gBattleStruct->battlerPartyOrders[0][i] & 0xF;
    }
    // (it always starts with the active one; make sure even if it got out of step)
    if (order[0] != gBattlerPartyIndexes[0])
    {
        u8 k, active = gBattlerPartyIndexes[0];
        for (k = 0; k < PARTY_SIZE && order[k] != active; k++)
            ;
        if (k < PARTY_SIZE)
        {
            for (; k > 0; k--)
                order[k] = order[k - 1];
            order[0] = active;
        }
    }
}

static u8 RemapBits(u8 bits, const u8 *map)
{
    u8 i, out = 0;
    for (i = 0; i < PARTY_SIZE; i++)
        if ((bits & (1 << i)) && map[i] < PARTY_SIZE)
            out |= 1 << map[i];
    return out;
}

static u16 sJoinFillerSpecies(struct Pokemon *party, u8 n, u16 trainerId)
{
    struct Pokemon *src = n >= 2 ? &party[n - 2] : &party[0];
    return PickFillerSpecies(src, n, trainerId);
}

static void JoinSetLinkPlayers(const u8 *seatPlayer, u8 count, const struct LinkPlayer *lp)
{
    static const u8 sIds[3] = {0, 2, 4};
    u8 i;
    for (i = 0; i < count; i++)
    {
        gLinkPlayers[i] = lp[i];
        gLinkPlayers[i].id = sIds[i];
    }
    for (i = count; i < MAX_LINK_PLAYERS; i++)
    {
        memset(&gLinkPlayers[i], 0, sizeof(gLinkPlayers[i]));
        // (like the battle's start: the trainer's name, a wild battle's TRAINER_NONE)
        StringCopyN(gLinkPlayers[i].name, gTrainers[gTrainerBattleOpponent_A].trainerName, PLAYER_NAME_LENGTH);
        gLinkPlayers[i].name[PLAYER_NAME_LENGTH] = EOS;
        gLinkPlayers[i].language = GAME_LANGUAGE;
        gLinkPlayers[i].gender = MALE;
    }
    if (count == 2)
    {
        gLinkPlayers[2].id = 1;
        gLinkPlayers[3].id = 3;
    }
    else
    {
        gLinkPlayers[3].id = 3;
    }
}

// Battlers, positions and controllers for `count` seats; `mySeat` = our seat (0 = master). Existing controllers are
// left alone when `keep` says so (they're idle at a turn's start).
static void JoinSetBattlers(u8 count, u8 mySeat, u8 keepMask)
{
    static const u8 sPos[TRIPLE_BATTLERS_COUNT] = {
        B_POSITION_PLAYER_LEFT, B_POSITION_OPPONENT_LEFT, B_POSITION_PLAYER_RIGHT,
        B_POSITION_OPPONENT_RIGHT, B_POSITION_PLAYER_CENTER, B_POSITION_OPPONENT_CENTER,
    };
    u8 b, n = count * 2;
    gBattlersCount = n;
    for (b = 0; b < n; b++)
    {
        gBattlerPositions[b] = sPos[b];
        if (keepMask & (1 << b))
            continue;
        if (b & BIT_SIDE)
            SetBattleControllerFor(b, mySeat == 0 ? SetControllerToOpponent : SetControllerToLinkOpponent);
        else
            SetBattleControllerFor(b, (b / 2 == mySeat) ? SetControllerToPlayer : SetControllerToLinkPartner);
    }
    if (count == 3)
    {
        for (b = 0; b < n; b++)
            BufferBattlePartyCurrentOrderBySide(b, 0);
    }
    else
    {
        BufferBattlePartyCurrentOrderBySide(0, 0);
        BufferBattlePartyCurrentOrderBySide(1, 0);
        BufferBattlePartyCurrentOrderBySide(2, 1);
        BufferBattlePartyCurrentOrderBySide(3, 1);
    }
}

bool8 MP_Coop_JoinIsSolo(void)
{
    return !gMpCoopBattle;
}

u8 MP_Coop_SeatCount(void)
{
    return gMpCoopBattle ? sCoop.count : 1;
}

u8 MP_Coop_SeatPlayer(u8 seat)
{
    return seat < 3 ? sCoop.seatPlayer[seat] : 0xFF;
}

// The Pokémon a joiner brings (their CO-OP TEAM, else their first that can battle), as their seat will hold them
u8 MP_Coop_JoinerPick(u8 seatSize, u8 *slots)
{
    return MP_Team_BattleSlots(slots, seatSize, IsUsableMon);
}

u8 MP_Coop_Seat(void)
{
    return gMpCoopBattle ? sCoop.seat : 0;
}

// A trainer friends may join: any but the Elite Four and the Champion. Gym leaders, rivals, admins, bosses and story
// grunts too: the master's own script goes on after the battle as it would have alone, a friend who joined a gym
// leader's battle gets the badge (MP_Coop_OnBattleEnd: their own copy of the leader's "you won" script), and the rest
// keep their own event scripts (a friend can still battle them on their own later).
static bool8 JoinTrainerIdOk(u16 trainer)
{
    if (!MP_IsRealTrainer(trainer))
        return FALSE;
    switch (gTrainers[trainer].trainerClass)
    {
    case TRAINER_CLASS_ELITE_FOUR:
    case TRAINER_CLASS_CHAMPION:
        return FALSE;
    }
    return TRUE;
}

bool8 MP_Coop_JoinTrainerOk(void)
{
    u16 trainer = gMpCoopBattle ? sCoop.trainerId : gTrainerBattleOpponent_A;
    u8 mode = gMpCoopBattle ? sCoop.mode : GetTrainerBattleMode();
    // two trainers at once: neither may be one of the Elite Four or the Champion
    u16 trainerB = gMpCoopBattle ? (sCoop.twoTrainers ? sCoop.trainerIdB : 0)
                                 : ((gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) ? gTrainerBattleOpponent_B : 0);
    if (!JoinTrainerIdOk(trainer) || (trainerB != 0 && !JoinTrainerIdOk(trainerB)))
        return FALSE;
    // a battle beside an in-game partner (STEVEN; JOHTO's LANCE, GIOVANNI, SILVER: battle_tower.c special battles) has
    // no seat to give, and its trainerbattle mode is an earlier battle's
    if (!gMpCoopBattle && (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER))
        return FALSE;
    switch (mode)
    {
    case TRAINER_BATTLE_SINGLE:
    case TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC:
    case TRAINER_BATTLE_CONTINUE_SCRIPT:
    case TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT:
    case TRAINER_BATTLE_DOUBLE:                   // a Double Battle the player started alone
    case TRAINER_BATTLE_REMATCH:
    case TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE:
    case TRAINER_BATTLE_REMATCH_DOUBLE:
    case TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE_NO_MUSIC:
        return TRUE;
    }
    return FALSE;   // (the Battle Pyramid, the Trainer Hill, the frontier's set trainers)
}

// The trainer's "you won" line, for a friend's game
const u8 *MP_Coop_JoinLoseText(void)
{
    if (gMpCoopWild || !(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return NULL;
    return GetTrainerALoseText();
}

// The second trainer's (two trainers at once)
const u8 *MP_Coop_JoinLoseTextB(void)
{
    if (gMpCoopWild || !(gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) || gTrainerBattleOpponent_B == 0)
        return NULL;
    return GetTrainerBLoseText();
}

// Two trainers, a Triple Battle: the third foe spot's Pokémon comes from the trainer with more left in reserve (the
// first one on a tie), JOIN_UNUSED = neither has one
static u8 JoinTwoTrainersNextFoe(void)
{
    u8 half, i, b, best = JOIN_UNUSED, bestCount = 0;
    for (half = 0; half < 2; half++)
    {
        u8 n = 0, first = JOIN_UNUSED;
        for (i = half * MULTI_PARTY_SIZE; i < (half + 1) * MULTI_PARTY_SIZE; i++)
        {
            bool8 out = FALSE;
            for (b = 1; b < gBattlersCount; b += 2)
                if (gBattlerPartyIndexes[b] == i && !(gAbsentBattlerFlags & (1 << b)))
                    out = TRUE;
            if (out || !IsUsableMon(&gEnemyParty[i]))
                continue;
            if (first == JOIN_UNUSED)
                first = i;
            n++;
        }
        if (n > bestCount)
        {
            bestCount = n;
            best = first;
        }
    }
    return best;
}

// Master: the conversion. joinMons[j] / joinCount[j] / joinLink[j] / joinBonus[j] for the new seats in order.
void MP_Coop_JoinConvertMaster(u8 newPlayers, const u8 *joiner, struct Pokemon (*joinMons)[MULTI_PARTY_SIZE],
                               const u8 *joinCount, const struct LinkPlayer *joinLink, const u16 *joinBonus,
                               struct MpJoinPlan *plan)
{
    struct Pokemon *oldParty = Alloc(sizeof(struct Pokemon) * PARTY_SIZE);
    struct Pokemon *oldEnemy = Alloc(sizeof(struct Pokemon) * PARTY_SIZE);
    u8 map[PARTY_SIZE], emap[PARTY_SIZE];
    u8 oldCount = MP_Coop_SeatCount(), count = oldCount + newPlayers;
    u8 newSize = count == 3 ? 2 : 3, oldSize = oldCount == 2 ? 3 : PARTY_SIZE;
    u8 s, i, k, b, newLeveled = 0, keepMask = 0;
    u8 oldOrig[MULTI_PARTY_SIZE];
    bool8 wasCoop = gMpCoopBattle, wild;
    // a Double Battle the player started alone: their second Pokemon went back to its ball (mp_join.c
    // BattleScript_MpJoinRecall) and battler 2 becomes the first friend's
    bool8 soloDouble = !wasCoop && (gBattleTypeFlags & BATTLE_TYPE_DOUBLE);
    // two trainers at once: each one's team stays on its own slots (0-2 / 3-5), so a Pokémon's slot says whose it is
    bool8 twoTrainers = (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) && gTrainerBattleOpponent_B != 0
                     && (wasCoop ? (sCoop.twoTrainers && !gMpCoopWild) : (gBattleTypeFlags & BATTLE_TYPE_TRAINER) != 0);
    struct LinkPlayer lp[3];

    memset(plan, 0, sizeof(*plan));
    memset(map, JOIN_UNUSED, sizeof(map));
    memset(emap, JOIN_UNUSED, sizeof(emap));
    memcpy(oldParty, gPlayerParty, sizeof(struct Pokemon) * PARTY_SIZE);
    memcpy(oldEnemy, gEnemyParty, sizeof(struct Pokemon) * PARTY_SIZE);
    memcpy(oldOrig, sCoop.origIndex, sizeof(oldOrig));
    plan->oldCount = oldCount;
    plan->count = count;

    if (!wasCoop)
    {
        // A solo battle: what the party is right now is the real party (nothing was set aside at its start)
        SavePlayerParty();
        sCoop.mode = (gBattleTypeFlags & BATTLE_TYPE_TRAINER) ? GetTrainerBattleMode() : MODE_WILD;
        sCoop.trainerId = (gBattleTypeFlags & BATTLE_TYPE_TRAINER) ? gTrainerBattleOpponent_A : 0;
        sCoop.seatPlayer[0] = gMpLocalId;
        sCoop.money = 0;
        sCoop.learnCount = 0;
        sCoop.catchCount = 0;
        sCoop.caught = CAUGHT_NONE;
        sCoop.preEvo = 0;
        sCoop.learnOutCount = 0;
        memset(sCoop.seatLeveled, 0, sizeof(sCoop.seatLeveled));
        sCoop.partnerLeveled = 0;
        sCoop.gotResult = FALSE;
        sCoop.resultWait = 0;
        sCoop.evoQueue = 0;
        sCoop.haveLoseText = FALSE;
        sCoop.afterBattle = gMain.savedCallback;   // CB2_EndWildBattle / CB2_EndTrainerBattle (MP_Join JoinableReason)
        gMain.savedCallback = CB2_MpCoopEndBattle;
        sCoop.wildDouble = FALSE;
        sCoop.wildExtra = 0;
        sCoop.twoTrainers = twoTrainers;
        sCoop.trainerIdB = twoTrainers ? gTrainerBattleOpponent_B : 0;
    }
    wild = sCoop.mode == MODE_WILD;
    for (s = 0; s < oldCount; s++)
        plan->seatPlayer[s] = sCoop.seatPlayer[s];
    for (s = 0; s < newPlayers; s++)
        plan->seatPlayer[oldCount + s] = joiner[s];
    plan->mode = sCoop.mode;
    plan->trainerId = sCoop.trainerId;
    plan->trainerIdB = twoTrainers ? gTrainerBattleOpponent_B : 0;

    // --- the players' side ---
    ZeroPlayerPartyMons();
    memset(plan->oldLocalToNew, JOIN_UNUSED, sizeof(plan->oldLocalToNew));
    for (s = 0; s < oldCount; s++)
    {
        u8 order[PARTY_SIZE], pick[MULTI_PARTY_SIZE], n, base = s * newSize, oldBase = s * oldSize, got;
        if (!wasCoop)
        {
            SoloMenuOrder(order);
            n = PARTY_SIZE;
        }
        else
        {
            // a co-op seat: the one out now, then the seat's others in order
            u8 active = gBattlerPartyIndexes[s * 2];
            n = 0;
            if (active >= oldBase && active < oldBase + oldSize)
                order[n++] = active;
            for (k = oldBase; k < oldBase + oldSize; k++)
                if (k != active)
                    order[n++] = k;
        }
        got = JoinPick(order, n, newSize, oldParty, pick);
        for (k = 0; k < got; k++)
        {
            gPlayerParty[base + k] = oldParty[pick[k]];
            map[pick[k]] = base + k;
            if (wasCoop)
                plan->oldLocalToNew[s][pick[k] - oldBase] = base + k;
        }
        // left out: their state now goes back to their owner's real party; level-ups still evolve later
        for (k = oldBase; k < oldBase + oldSize; k++)
        {
            bool8 kept = map[k] != JOIN_UNUSED;
            if (kept)
            {
                if (gLeveledUpInBattle & (1 << k))
                    newLeveled |= 1 << map[k];
                continue;
            }
            if (!wasCoop)
            {
                if (gLeveledUpInBattle & (1 << k))
                    sCoop.preEvo |= 1 << k;
            }
            else if (s == 0)
            {
                if (oldOrig[k - oldBase] < PARTY_SIZE && GetMonData(&oldParty[k], MON_DATA_SPECIES) != SPECIES_NONE)
                {
                    gSaveBlock1Ptr->playerParty[oldOrig[k - oldBase]] = oldParty[k];
                    if (gLeveledUpInBattle & (1 << k))
                        sCoop.preEvo |= 1 << oldOrig[k - oldBase];
                }
            }
            else if (gLeveledUpInBattle & (1 << k))
            {
                plan->droppedLeveled[s] |= 1 << (k - oldBase);
            }
        }
        if (s == 0)
        {
            memset(sCoop.origIndex, 0xFF, sizeof(sCoop.origIndex));
            for (k = 0; k < got; k++)
                sCoop.origIndex[k] = wasCoop ? oldOrig[pick[k] - oldBase] : pick[k];
            sCoop.localCount = got;
        }
    }
    for (s = 0; s < newPlayers; s++)
    {
        u8 base = (oldCount + s) * newSize;
        for (k = 0; k < joinCount[s] && k < newSize; k++)
            gPlayerParty[base + k] = joinMons[s][k];
        lp[oldCount + s] = joinLink[s];
        sCoop.playerBonus[joiner[s]] = joinBonus[s];
        sCoop.playerBonusHeard |= 1 << joiner[s];
    }
    // moves to learn after the battle (by battle party slot): a kept Pokémon's follow it to its new slot, a
    // left-out one's go with it to its real party (ours now, a partner's through the plan)
    {
        u8 n = 0;
        for (i = 0; i < sCoop.learnCount; i++)
        {
            u8 slot = sCoop.learn[i].slot;
            u16 move = sCoop.learn[i].move;
            if (slot < PARTY_SIZE && map[slot] != JOIN_UNUSED)
            {
                sCoop.learn[n].slot = map[slot];
                sCoop.learn[n].move = move;
                n++;
            }
            else if (wasCoop && slot < oldCount * oldSize)
            {
                u8 owner = slot / oldSize, local = slot - owner * oldSize;
                if (owner == 0)
                    AddLearnOut(oldOrig[local], move);
                else if (plan->droppedLearnCount[owner] < JOIN_MAX_LEARN_OUT)
                {
                    plan->droppedLearnSlot[owner][plan->droppedLearnCount[owner]] = local;
                    plan->droppedLearnMove[owner][plan->droppedLearnCount[owner]] = move;
                    plan->droppedLearnCount[owner]++;
                }
            }
        }
        sCoop.learnCount = n;
    }
    gLeveledUpInBattle = newLeveled;
    for (b = 0; b < MAX_BATTLERS_COUNT; b += 2)
    {
        if (b >= gBattlersCount || (soloDouble && b == 2))
            continue;
        if (gBattlerPartyIndexes[b] < PARTY_SIZE && map[gBattlerPartyIndexes[b]] != JOIN_UNUSED)
            gBattlerPartyIndexes[b] = map[gBattlerPartyIndexes[b]];
        else
            gBattlerPartyIndexes[b] = (b / 2) * newSize;   // out of Pokemon (absent): its own seat, never a friend's slot
    }
    for (i = 0; i < ARRAY_COUNT(gSentPokesToOpponent); i++)
        gSentPokesToOpponent[i] = RemapBits(gSentPokesToOpponent[i], map);
    gWishFutureKnock.knockedOffMons[B_SIDE_PLAYER] = RemapBits(gWishFutureKnock.knockedOffMons[B_SIDE_PLAYER], map);
    for (b = 0; b < MAX_BATTLERS_COUNT; b += 2)
        if (gWishFutureKnock.wishMonId[b] < PARTY_SIZE && map[gWishFutureKnock.wishMonId[b]] != JOIN_UNUSED)
            gWishFutureKnock.wishMonId[b] = map[gWishFutureKnock.wishMonId[b]];
    gBattleStruct->expGetterMonId = 0;

    // --- the trainer's (or wild) side ---
    {
        u8 order[PARTY_SIZE], n = 0, actives[3], nActive = 0, slotOf[PARTY_SIZE];
        u8 alive[PARTY_SIZE], nAlive = 0, rest[PARTY_SIZE], nRest = 0;
        // out now, in battler order
        for (b = 1; b < gBattlersCount; b += 2)
            if (!(gAbsentBattlerFlags & (1 << b)) && gBattlerPartyIndexes[b] < PARTY_SIZE
             && GetMonData(&oldEnemy[gBattlerPartyIndexes[b]], MON_DATA_HP) != 0)
                actives[nActive++] = gBattlerPartyIndexes[b];
        for (i = 0; i < PARTY_SIZE; i++)
        {
            bool8 isActive = FALSE;
            for (k = 0; k < nActive; k++)
                if (actives[k] == i)
                    isActive = TRUE;
            if (isActive)
                continue;
            if (!wild && GetMonData(&oldEnemy[i], MON_DATA_HP) != 0 && GetMonData(&oldEnemy[i], MON_DATA_SPECIES) != SPECIES_NONE)
                alive[nAlive++] = i;
            else
                rest[nRest++] = i;
        }
        // TEAM PADDING: one extra Moemon in a team-up, never more (a co-op battle got it at its start)
        // It only goes into an empty slot: one of the trainer's fainted Moemon must stay as it is (its "already gave
        // EXP" mark would pass on to the newcomer, and then beating it would give none)
        for (k = nRest; k > 0 && GetMonData(&oldEnemy[rest[k - 1]], MON_DATA_SPECIES) != SPECIES_NONE; k--)
            ;
        if (!wild && !wasCoop && MP_TrainerPadding() && nActive + nAlive < PARTY_SIZE && k > 0)
        {
            u8 slot = rest[k - 1];
            u8 total = nActive + nAlive;
            struct Pokemon *src = nAlive > 0 ? &oldEnemy[alive[nAlive - 1]] : &oldEnemy[actives[0]];
            u16 species = PickFillerSpecies(src, total, sCoop.trainerId);
            for (; k < nRest; k++)
                rest[k - 1] = rest[k];
            nRest--;
            gBattleStruct->givenExpMons &= ~(1 << slot);
            gWishFutureKnock.knockedOffMons[B_SIDE_OPPONENT] &= ~(1 << slot);
            CreateMon(&oldEnemy[slot], species, GetMonData(src, MON_DATA_LEVEL), GetMonData(src, MON_DATA_ATK_IV), TRUE,
                      Random32(), OT_ID_RANDOM_NO_SHINY, 0);
            // before the ace (the last one), like a co-op team-up's padding
            if (nAlive > 0)
            {
                u8 ace = alive[nAlive - 1];
                alive[nAlive - 1] = slot;
                alive[nAlive++] = ace;
            }
            else
            {
                alive[nAlive++] = slot;
            }
            plan->padded = TRUE;
            MPNet_Log("join: TEAM PADDING adds species %d", species);
        }
        for (k = 0; k < nActive; k++)
            order[n++] = actives[k];
        for (k = 0; k < nAlive; k++)
            order[n++] = alive[k];
        for (k = 0; k < nRest; k++)
            order[n++] = rest[k];
        // order[] = the trainer's Pokémon in sending-out order. Lay them out for the new battle:
        if (count == 3)
        {
            // a triple: battlers 1 / 3 / 5 on slots 0 / 1 / 2, the rest a shared pool
            for (k = 0; k < PARTY_SIZE; k++)
                slotOf[k] = k;
        }
        else
        {
            // a double: battler 1 sends slots 0-2, battler 3 slots 3-5 (the next one for battler 3 goes first)
            static const u8 sDouble[PARTY_SIZE] = {0, 3, 1, 4, 2, 5};
            for (k = 0; k < PARTY_SIZE; k++)
                slotOf[k] = sDouble[k];
        }
        if (wild)
        {
            // wild Pokémon keep their places: the ones out stay on their battlers, empty slots stay empty
            for (k = 0; k < PARTY_SIZE; k++)
                slotOf[k] = JOIN_UNUSED;
        }
        if (twoTrainers)
        {
            // two trainers: everything stays where it is (slots 0-2 the first trainer's, 3-5 the second's)
            for (k = 0; k < PARTY_SIZE; k++)
                order[k] = slotOf[k] = k;
        }
        ZeroEnemyPartyMons();
        if (!wild)
        {
            for (k = 0; k < PARTY_SIZE; k++)
            {
                gEnemyParty[slotOf[k]] = oldEnemy[order[k]];
                emap[order[k]] = slotOf[k];
            }
        }
        else
        {
            // one wild battler per seat side position: b1 keeps slot 0; a wild double's second (battler 3) goes to
            // the slot the new layout gives battler 3 (1 in a triple, 3 in a double)
            for (k = 0; k < PARTY_SIZE; k++)
                if (GetMonData(&oldEnemy[k], MON_DATA_SPECIES) != SPECIES_NONE)
                    emap[k] = JOIN_UNUSED;
            for (b = 1; b < gBattlersCount; b += 2)
            {
                u8 idx = gBattlerPartyIndexes[b], to = (count == 3) ? b / 2 : (b == 1 ? 0 : 3);
                if (idx < PARTY_SIZE && GetMonData(&oldEnemy[idx], MON_DATA_SPECIES) != SPECIES_NONE)
                {
                    gEnemyParty[to] = oldEnemy[idx];
                    emap[idx] = to;
                }
            }
        }
        gBattleStruct->givenExpMons = RemapBits(gBattleStruct->givenExpMons, emap);
        gWishFutureKnock.knockedOffMons[B_SIDE_OPPONENT] = RemapBits(gWishFutureKnock.knockedOffMons[B_SIDE_OPPONENT], emap);
        for (b = 1; b < MAX_BATTLERS_COUNT; b += 2)
            if (gWishFutureKnock.wishMonId[b] < PARTY_SIZE && emap[gWishFutureKnock.wishMonId[b]] != JOIN_UNUSED)
                gWishFutureKnock.wishMonId[b] = emap[gWishFutureKnock.wishMonId[b]];
        for (b = 1; b < gBattlersCount; b += 2)
            if (gBattlerPartyIndexes[b] < PARTY_SIZE && emap[gBattlerPartyIndexes[b]] != JOIN_UNUSED)
                gBattlerPartyIndexes[b] = emap[gBattlerPartyIndexes[b]];
    }

    // --- the battlers ---
    for (b = 0; b < gBattlersCount; b++)
        if (!(soloDouble && b == 2))
            keepMask |= 1 << b;
    {
        u8 oldBattlers = gBattlersCount, newCountB = count * 2, absent = gAbsentBattlerFlags;
        for (b = 0; b < MAX_BATTLERS_COUNT; b++)
        {
            // nobody is switching in yet (a new spot's newcomer comes in with the entrance)
            gBattleStruct->monToSwitchIntoId[b] = PARTY_SIZE;
            gBattleStruct->AI_monToSwitchIntoId[b] = PARTY_SIZE;
        }
        for (b = 0; b < newCountB; b++)
        {
            if (b < oldBattlers && !(soloDouble && b == 2))
                continue;
            // a spot someone else takes over: none of the old one's sprite state (hidden by FLY / DIG, behind a
            // SUBSTITUTE, transformed, a CASTFORM form)
            memset(&gBattleSpritesDataPtr->battlerData[b], 0, sizeof(gBattleSpritesDataPtr->battlerData[b]));
            gBattleMonForms[b] = 0;
            u8 *ptr = (u8 *)&gBattleMons[b];
            for (i = 0; i < sizeof(struct BattlePokemon); i++)
                ptr[i] = 0;
            gBattleStruct->monToSwitchIntoId[b] = PARTY_SIZE;
            gBattleStruct->AI_monToSwitchIntoId[b] = PARTY_SIZE;
            gChosenActionByBattler[b] = B_ACTION_NONE;
            gChosenMoveByBattler[b] = MOVE_NONE;
            if (b & BIT_SIDE)
            {
                // the trainer's next Pokémon for this battler, if it has one
                s32 first, last;
                u8 idx = JOIN_UNUSED;
                if (twoTrainers)
                    idx = JoinTwoTrainersNextFoe();
                else if (count == 3)
                    first = 3, last = PARTY_SIZE, idx = b / 2;
                else
                    first = 3, last = PARTY_SIZE, idx = 3;
                if (wild || idx >= PARTY_SIZE || !IsUsableMon(&gEnemyParty[idx]))
                {
                    // nothing to send: an empty spot (an empty slot, so nothing is drawn there)
                    u8 empty = JOIN_UNUSED;
                    for (i = 0; i < PARTY_SIZE; i++)
                        if (GetMonData(&gEnemyParty[i], MON_DATA_SPECIES) == SPECIES_NONE)
                        {
                            empty = i;
                            break;
                        }
                    gBattlerPartyIndexes[b] = empty != JOIN_UNUSED ? empty : idx;
                    absent |= 1 << b;
                }
                else
                {
                    gBattlerPartyIndexes[b] = idx;
                    absent |= 1 << b;
                    plan->entering |= 1 << b;
                }
                (void)first;
                (void)last;
            }
            else
            {
                gBattlerPartyIndexes[b] = (b / 2) * newSize;
                absent |= 1 << b;
                plan->entering |= 1 << b;
            }
        }
        gAbsentBattlerFlags = absent;
        gBattleStruct->absentBattlerFlags = absent;
    }

    for (s = 0; s < oldCount; s++)
    {
        if (s == 0)
            NetLink_FillLocalLinkPlayer(&lp[0]);
        else
            lp[s] = gLinkPlayers[s];
    }
    lp[0].id = 0;
    gMpCoopBattle = TRUE;
    gMpCoopWild = wild;
    gIsTripleBattle = count == 3;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_LINK | BATTLE_TYPE_MULTI
                     | BATTLE_TYPE_TOWER_LINK_MULTI | BATTLE_TYPE_IS_MASTER | (gBattleTypeFlags & BATTLE_TYPE_LINK_IN_BATTLE)
                     | (twoTrainers ? BATTLE_TYPE_TWO_OPPONENTS : 0);   // (each trainer's names, pictures, both prizes)
    gLinkType = LINKTYPE_BATTLE;
    sCoop.isMaster = TRUE;
    sCoop.seat = 0;
    sCoop.count = count;
    for (s = 0; s < 3; s++)
        sCoop.seatPlayer[s] = s < count ? plan->seatPlayer[s] : 0xFF;
    sCoop.partner = sCoop.seatPlayer[1];
    JoinSetLinkPlayers(sCoop.seatPlayer, count, lp);
    JoinSetBattlers(count, 0, keepMask);

    plan->environment = gBattleEnvironment;
    plan->padded = plan->padded || wasCoop;
    plan->battleTypeFlags = gBattleTypeFlags & ~(BATTLE_TYPE_IS_MASTER | BATTLE_TYPE_LINK_IN_BATTLE);
    for (b = 0; b < MAX_BATTLERS_COUNT; b++)
        plan->partyIdx[b] = gBattlerPartyIndexes[b];
    plan->absent = gAbsentBattlerFlags;
    plan->weather = gBattleWeather;
    for (s = 0; s < count; s++)
        plan->linkPlayers[s] = gLinkPlayers[s];
    Free(oldParty);
    Free(oldEnemy);
    MPNet_Log("join: battle now %d seats (was %d), entering %02x, absent %02x, party %d %d %d %d %d %d, foes %d %d %d %d %d %d",
              count, oldCount, plan->entering, plan->absent,
              GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), GetMonData(&gPlayerParty[1], MON_DATA_SPECIES),
              GetMonData(&gPlayerParty[2], MON_DATA_SPECIES), GetMonData(&gPlayerParty[3], MON_DATA_SPECIES),
              GetMonData(&gPlayerParty[4], MON_DATA_SPECIES), GetMonData(&gPlayerParty[5], MON_DATA_SPECIES),
              GetMonData(&gEnemyParty[0], MON_DATA_SPECIES), GetMonData(&gEnemyParty[1], MON_DATA_SPECIES),
              GetMonData(&gEnemyParty[2], MON_DATA_SPECIES), GetMonData(&gEnemyParty[3], MON_DATA_SPECIES),
              GetMonData(&gEnemyParty[4], MON_DATA_SPECIES), GetMonData(&gEnemyParty[5], MON_DATA_SPECIES));
}

// A game other than the master: the battle's battlers, positions, controllers and party slots as the plan says
// (keepMask: battlers already in this game's battle keep their controllers)
static void JoinSetBattlersKeep(const struct MpJoinPlan *plan, u8 seat, u8 keepMask)
{
    u8 b;
    gBattleTypeFlags = plan->battleTypeFlags | (gBattleTypeFlags & BATTLE_TYPE_LINK_IN_BATTLE);
    gIsTripleBattle = plan->count == 3;
    gMpCoopBattle = TRUE;
    gMpCoopWild = plan->mode == MODE_WILD;
    for (b = 0; b < MAX_BATTLERS_COUNT; b++)
        gBattlerPartyIndexes[b] = plan->partyIdx[b];
    gAbsentBattlerFlags = plan->absent;
    JoinSetBattlers(plan->count, seat, keepMask);
    JoinSetLinkPlayers(plan->seatPlayer, plan->count, plan->linkPlayers);
}

void MP_Coop_JoinSetBattlers(const struct MpJoinPlan *plan, u8 seat)
{
    JoinSetBattlersKeep(plan, seat, 0);
}

static void JoinSetSeats(const struct MpJoinPlan *plan, u8 seat)
{
    u8 s;
    sCoop.isMaster = FALSE;
    sCoop.seat = seat;
    sCoop.count = plan->count;
    for (s = 0; s < 3; s++)
        sCoop.seatPlayer[s] = s < plan->count ? plan->seatPlayer[s] : 0xFF;
    sCoop.partner = sCoop.seatPlayer[0];
    sCoop.mode = plan->mode;
    sCoop.trainerId = plan->trainerId;
    sCoop.twoTrainers = plan->trainerIdB != 0;
    sCoop.trainerIdB = plan->trainerIdB;
}

// The friend who joined: their real party is set aside (the battle's party is the master's snapshot) and their
// picks are their seat's Pokémon
void MP_Coop_JoinApplyJoiner(const struct MpJoinPlan *plan, u8 seat, const u8 *picks, u8 nPicks, const u8 *loseText,
                             const u8 *loseTextB)
{
    u8 i;
    SavePlayerParty();   // LoadPlayerParty brings the real party back afterwards
    JoinSetSeats(plan, seat);
    sCoop.caught = CAUGHT_NONE;
    sCoop.catchCount = 0;
    memset(sCoop.seatLeveled, 0, sizeof(sCoop.seatLeveled));
    sCoop.money = 0;
    sCoop.learnCount = 0;
    sCoop.learnOutCount = 0;
    sCoop.partnerLeveled = 0;
    sCoop.gotResult = FALSE;
    sCoop.resultWait = 0;
    sCoop.evoQueue = 0;
    sCoop.preEvo = 0;
    memset(sCoop.origIndex, 0xFF, sizeof(sCoop.origIndex));
    for (i = 0; i < nPicks && i < MULTI_PARTY_SIZE && i < (plan->count == 3 ? 2 : 3); i++)
        sCoop.origIndex[i] = picks[i];
    sCoop.localCount = i;
    sCoop.haveLoseText = FALSE;
    if (loseText != NULL)
    {
        for (i = 0; i < LOSE_TEXT_MAX && loseText[i] != EOS; i++)
            sCoop.loseText[i] = loseText[i];
        sCoop.loseText[i] = EOS;
        sCoop.haveLoseText = i > 0;
    }
    sCoop.haveLoseTextB = FALSE;
    if (loseTextB != NULL)
    {
        for (i = 0; i < LOSE_TEXT_MAX && loseTextB[i] != EOS; i++)
            sCoop.loseTextB[i] = loseTextB[i];
        sCoop.loseTextB[i] = EOS;
        sCoop.haveLoseTextB = i > 0;
    }
    sCoop.afterBattle = NULL;
    sCoop.role = ROLE_NONE;
    sCoop.challenged = FALSE;
    sCoop.accepted = FALSE;
    sCoop.wildDouble = FALSE;
    gMpCoopBattle = TRUE;
    gMpCoopWild = plan->mode == MODE_WILD;
    gIsTripleBattle = plan->count == 3;
    gMpDebugTriple = getenv("MOEMON_TRIPLEDEBUG") != NULL;
    gLinkType = LINKTYPE_BATTLE;
    gMain.savedCallback = CB2_MpCoopEndBattle;
    MPNet_Log("join: we are seat %d of %d, bringing %d (slots %d %d %d)", seat, plan->count, sCoop.localCount,
              sCoop.origIndex[0], sCoop.origIndex[1], sCoop.origIndex[2]);
}

// A partner already in the battle (a co-op double becoming a triple): their seat shrinks to two. The ones left out
// go back to their real party now, as the master's game had them (oldSeatMons), and still get to evolve / learn
// their moves after a win.
void MP_Coop_JoinApplyPartner(const struct MpJoinPlan *plan, u8 seat, const struct Pokemon *party,
                              const struct Pokemon *enemy, const struct Pokemon *oldSeatMons)
{
    u8 newSize = plan->count == 3 ? 2 : 3, newOrig[MULTI_PARTY_SIZE], i, kept = 0, keepMask = 0, b;

    memset(newOrig, 0xFF, sizeof(newOrig));
    for (i = 0; i < sCoop.localCount && i < MULTI_PARTY_SIZE; i++)
    {
        u8 to = plan->oldLocalToNew[seat][i], real = sCoop.origIndex[i];
        if (to != JOIN_UNUSED)
        {
            u8 local = to - seat * newSize;
            if (local < MULTI_PARTY_SIZE)
            {
                newOrig[local] = real;
                kept++;
            }
            continue;
        }
        if (real < PARTY_SIZE && GetMonData((struct Pokemon *)&oldSeatMons[i], MON_DATA_SPECIES) != SPECIES_NONE)
        {
            gSaveBlock1Ptr->playerParty[real] = oldSeatMons[i];
            if (plan->droppedLeveled[seat] & (1 << i))
                sCoop.preEvo |= 1 << real;
            MPNet_Log("join: our party slot %d sits the rest out", real);
        }
    }
    for (i = 0; i < plan->droppedLearnCount[seat] && i < JOIN_MAX_LEARN_OUT; i++)
    {
        u8 local = plan->droppedLearnSlot[seat][i];
        if (local < MULTI_PARTY_SIZE)
            AddLearnOut(sCoop.origIndex[local], plan->droppedLearnMove[seat][i]);
    }
    memcpy(sCoop.origIndex, newOrig, sizeof(sCoop.origIndex));
    sCoop.localCount = kept;
    memcpy(gPlayerParty, party, sizeof(struct Pokemon) * PARTY_SIZE);
    memcpy(gEnemyParty, enemy, sizeof(struct Pokemon) * PARTY_SIZE);
    JoinSetSeats(plan, seat);
    for (b = 0; b < gBattlersCount; b++)
        keepMask |= 1 << b;
    for (b = gBattlersCount; b < plan->count * 2; b++)
        memset(&gBattleMons[b], 0, sizeof(gBattleMons[b]));
    JoinSetBattlersKeep(plan, seat, keepMask);
    MPNet_Log("join: our seat %d now has %d (slots %d %d), %d players", seat, kept, sCoop.origIndex[0], sCoop.origIndex[1], plan->count);
}
