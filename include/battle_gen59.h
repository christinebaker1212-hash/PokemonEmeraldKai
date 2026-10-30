#ifndef GUARD_BATTLE_GEN59_H
#define GUARD_BATTLE_GEN59_H

// Moemon Kai: the Gen 5-9 moves' effects (src/battle_gen59.c), on top of Gen 4's rules (battle_gen4.c, whose
// hooks call these). Most of the new moves are data: gGen59Moves (src/data/battle_moves_gen59_data.h) says what
// stats they change on whom and which status they may cause; the rest is per effect here.

struct Gen59Move
{
    s8 userStages[NUM_BATTLE_STATS - 1];     // ATK DEF SPEED SPATK SPDEF ACC EVASION
    u8 userChance;                           // percent for a hit; 0 = always
    s8 targetStages[NUM_BATTLE_STATS - 1];
    u8 targetChance;
    u8 status;                               // a MOVE_EFFECT_* the hit may cause
    u8 statusChance;                         // 0 = always
    u32 arg;                                 // a type, a STATUS1 mask, a hold effect, a terrain...
};

#define G59_TERRAIN_NONE     0
#define G59_TERRAIN_GRASSY   1
#define G59_TERRAIN_MISTY    2
#define G59_TERRAIN_ELECTRIC 3
#define G59_TERRAIN_PSYCHIC  4

struct Gen59Battler
{
    u8 hitsTaken;          // RAGE FIST (kept while it stays in)
    u8 lastMoveFailed:1;   // STOMPING TANTRUM
    u8 statsLowered:1;     // LASH OUT: its stats fell this turn
    u8 smackedDown:1;      // SMACK DOWN / THOUSAND ARROWS: grounded
    u8 ateBerry:1;         // BELCH
    u8 saltCure:1;
    u8 throatChop;         // turns left without sound moves
    u8 laserFocus;         // its next hit is critical
    u8 tarShot:1;          // FIRE hurts it twice as much
    u8 noRetreat:1;
    u8 octolock:1;
    u8 thirdType;          // TRICK-OR-TREAT / FOREST'S CURSE (TYPE_MYSTERY = none)
    u8 arrived:1;          // its arrival ability has acted (MP_Gen59_SwitchInAbility)
};

struct Gen59Side
{
    u8 stickyWeb:1;
    u8 auroraVeil;         // turns left
    u8 faintedLastTurn:1;  // RETALIATE
    u8 faintedThisTurn:1;
    u8 fainted;            // LAST RESPECTS
};

struct Gen59Field
{
    u8 terrain;            // G59_TERRAIN_*
    u8 terrainTurns;
    u8 wonderRoom;
    u8 magicRoom;
    u8 fairyLock;
    u8 echoedVoice;        // ECHOED VOICE: how many turns in a row it was used
    u8 echoedThisTurn:1;
    u8 ionDeluge:1;        // NORMAL moves are ELECTRIC this turn
    u8 moveEndDone;        // MP_Gen59_MoveEnd's steps done for this move
};

extern struct Gen59Battler gGen59Battlers[];
extern struct Gen59Side gGen59Sides[];
extern struct Gen59Field gGen59Field;
extern const struct Gen59Move gGen59Moves[];

#define IS_GEN59_MOVE(move) ((move) >= FIRST_GEN58_MOVE && (move) <= LAST_GEN58_MOVE)

void MP_Gen59_BattleStart(void);
void MP_Gen59_SwitchIn(u8 battler);
void MP_Gen59_TurnEnd(void);
void MP_Gen59_OnDamaged(u8 battler, u8 attacker, s32 damage);
void MP_Gen59_MoveDone(u8 battler, u16 move, u8 resultFlags);
void MP_Gen59_OnStatLowered(u8 battler);
void MP_Gen59_OnFainted(u8 battler);
u8 MP_Gen59_Command(u8 mode);
const u8 *MP_Gen59_FieldEndTurn(void);
const u8 *MP_Gen59_ArrivalEffect(u8 battler);

u16 MP_Gen59_MovePower(u16 move, u8 attacker, u8 target, u16 power);
void MP_Gen59_AttackStats(u16 move, u8 attacker, u8 target, u16 *attack, u16 *spAttack, u16 *defense, u16 *spDefense);
s32 MP_Gen59_FinalDamage(s32 damage, u16 move, u8 type, u8 attacker, u8 target, u8 resultFlags);
bool8 MP_Gen59_AlwaysCrit(u16 move, u8 attacker);
u8 MP_Gen59_MoveType(u16 move, u8 battler, u8 type);
s8 MP_Gen59_IsPhysical(u16 move, u8 battler);        // -1: as its category says
bool8 MP_Gen59_StatusBlocked(u8 battler, u8 moveEffect);  // MISTY / ELECTRIC TERRAIN
bool8 MP_Gen59_IsGrounded(u8 battler);
bool8 MP_Gen59_HoldsItem(u8 battler, u16 item);      // the Gen 5-9 held items (not under EMBARGO, KLUTZ, MAGIC ROOM)
u8 MP_Gen59_AteType(u16 move, u8 battler, u8 type);   // PIXILATE...
u32 MP_Gen59_AbilityPower(u16 move, u8 attacker, u32 power);
s8 MP_Gen59_Priority(u8 battler, u16 move);           // PRANKSTER, GALE WINGS, TRIAGE, GRASSY GLIDE
u32 MP_Gen59_Speed(u8 battler, u32 speed);            // SAND RUSH, SLUSH RUSH, SURGE SURFER
void MP_Gen59_OnMoveStart(u8 battler, u16 move);      // PROTEAN, LIBERO
u8 MP_Gen59_SwitchInAbility(u8 battler);              // the SURGES, INTREPID SWORD, DAUNTLESS SHIELD
const u8 *MP_Gen59_MoveEnd(void);                     // MOXIE, BEAST BOOST

#endif // GUARD_BATTLE_GEN59_H
