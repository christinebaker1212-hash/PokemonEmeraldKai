// Moemon Co-op: joining a battle in progress (mp_join.c protocol and screens, mp_coop.c party conversion).
#ifndef GUARD_MP_JOIN_H
#define GUARD_MP_JOIN_H

#include "global.h"
#include "link.h"
#include "pokemon.h"
#include "constants/battle.h"

#define JOIN_MAX_NEW 2   // friends who can join one battle at once (a solo battle becomes a triple)
#define JOIN_MAX_LEARN_OUT 4

// What the battle becomes, decided by its master (seat 0) and sent to every game
struct MpJoinPlan
{
    u8 oldCount;               // seats before (1 = a solo battle)
    u8 count;                  // seats after (2 or 3)
    u8 seatPlayer[3];          // network player id of each seat
    u8 mode;                   // mp_coop.c MODE_*
    u16 trainerId;
    u8 environment;
    u8 padded;                 // TEAM PADDING's one extra Moemon is in the trainer's party
    u32 battleTypeFlags;       // without BATTLE_TYPE_IS_MASTER / LINK_IN_BATTLE
    u8 partyIdx[MAX_BATTLERS_COUNT];
    u8 absent;                 // gAbsentBattlerFlags after the conversion (entering battlers included)
    u8 entering;               // battlers that come in: the friends who joined, the trainer's new ones
    u16 weather;
    u8 oldLocalToNew[3][3];    // per old seat: each of its old local slots -> new party slot (0xFF = left out)
    u8 droppedLeveled[3];      // per old seat: bits of its old local slots that leveled up and were left out
    u8 droppedLearnCount[3];   // per old seat: moves its left-out Pokémon wanted to learn (offered after the battle)
    u8 droppedLearnSlot[3][JOIN_MAX_LEARN_OUT];   // (old local slot)
    u16 droppedLearnMove[3][JOIN_MAX_LEARN_OUT];
    struct LinkPlayer linkPlayers[3];
};

#define JOIN_UNUSED 0xFF
#define JOIN_LOSE_TEXT_MAX 240

// mp_join.c
bool8 MP_Join_IsEntering(u8 battler);
u8 MP_Join_EnteringMask(void);
void MP_Join_OnTurnStart(void);
bool8 MP_Join_TurnHook(void);
bool8 MP_Join_NextEntering(bool8 foes);
void MP_Join_Ask(u8 target);
bool8 MP_Join_IsAsking(void);
void MP_Join_OnRelay(u8 from, const u8 *data, u16 len);
void MP_Join_OnPlayerLeft(u8 id);
void MP_Join_Update(void);
void MP_Join_OnBattleEnd(void);
void MP_Join_ApplySnapshot(void);
void MP_Join_CreateReadyTask(void);
extern bool8 gMpJoinedBattle;

// mp_battle.c
void NetLink_FillLocalLinkPlayer(struct LinkPlayer *out);

// mp_coop.c
u8 MP_Coop_Seat(void);
u8 MP_Coop_SeatCount(void);
u8 MP_Coop_SeatPlayer(u8 seat);
u8 MP_Coop_JoinerPick(u8 seatSize, u8 *slots);
bool8 MP_Coop_JoinTrainerOk(void);
const u8 *MP_Coop_JoinLoseText(void);
void MP_Coop_JoinConvertMaster(u8 newPlayers, const u8 *joiner, struct Pokemon (*joinMons)[MULTI_PARTY_SIZE],
                               const u8 *joinCount, const struct LinkPlayer *joinLink, const u16 *joinBonus,
                               struct MpJoinPlan *plan);
void MP_Coop_JoinSetBattlers(const struct MpJoinPlan *plan, u8 seat);
void MP_Coop_JoinApplyJoiner(const struct MpJoinPlan *plan, u8 seat, const u8 *picks, u8 nPicks, const u8 *loseText);
void MP_Coop_JoinApplyPartner(const struct MpJoinPlan *plan, u8 seat, const struct Pokemon *party,
                              const struct Pokemon *enemy, const struct Pokemon *oldSeatMons);

#endif // GUARD_MP_JOIN_H
