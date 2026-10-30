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
    u8 statDropPending:1;  // a foe lowered a stat: DEFIANT / COMPETITIVE at the move's end
    u8 disguiseUsed:1;     // DISGUISE / ICE FACE took a hit
    u8 disguiseJust:1;     //   (this move: its message at the move's end)
    u8 boosted:1;          // PROTOSYNTHESIS / QUARK DRIVE on (BOOSTER ENERGY or the field)
    u8 boostStat;          //   which stat (STAT_ATK..STAT_SPEED)
    u8 eatenBerry;         // CUD CHEW: the berry to eat again (ITEM - FIRST_BERRY) + 1, 0 = none
    u8 cudChewTurns;
    u16 hpBeforeHit;       // BERSERK / ANGER SHELL: crossing half
    u8 form;               // STANCE CHANGE / SCHOOLING / SHIELDS DOWN / ZERO TO HERO / TERA SHIFT: 1 = the other form
    u8 gulp;               // GULP MISSILE: 1 = ARROKUDA (DEFENSE down), 2 = PIKACHU (paralysis)
    u8 hangry:1;           // HUNGER SWITCH: AURA WHEEL is DARK
    u8 cheekPouch:1;       // CHEEK POUCH: a BERRY was just eaten
    u8 emergencyExit:1;    // EMERGENCY EXIT / WIMP OUT: fell below half this move
    u8 copyPending:1;      // OPPORTUNIST: a foe's stats rose
    u8 glaiveRush:1;       // GLAIVE RUSH: hits on it never miss and do double
    u8 received:1;         // RECEIVER / POWER OF ALCHEMY took a fallen ally's ability (its message)
    s8 copyStages[NUM_BATTLE_STATS];
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
    u16 moveEndDone;       // MP_Gen59_MoveEnd's steps done for this move
    u8 faintedThisMove:1;  // SOUL-HEART
    u8 ballFetched:1;      // BALL FETCH: once a battle
    u8 heroSlots[2];       // ZERO TO HERO: party slots (bits) that left the field once, per side
    u8 danced;             // DANCER: the battlers that followed this move (bits)
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
bool8 MP_Gen59_FlowerVeil(u8 battler);                // stat drops / status kept off (a GRASS type by FLOWER VEIL)
u8 MP_Gen59_EndTurnAbility(u8 battler);               // MOODY, HEALER, CUD CHEW, HARVEST (1 if a script was started)
u8 MP_Gen59_AbsorbAbility(u8 battler, u16 move, u8 type);   // SAP SIPPER, WELL-BAKED BODY... (gBattlescriptCurrInstr)
bool8 MP_Gen59_BlockMove(u8 battler, u16 move);       // BULLETPROOF, QUEENLY MAJESTY, GOOD AS GOLD... (sets the script)
u32 MP_Gen59_Accuracy(u8 attacker, u8 target, u16 move, u32 acc);   // VICTORY STAR, WONDER SKIN
bool8 MP_Gen59_IgnoresAbilities(u8 attacker);         // MOLD BREAKER's kin: TERAVOLT, TURBOBLAZE
bool8 MP_Gen59_IntimidateRaises(u8 battler);
void MP_Gen59_AteBerry(u8 battler, u16 berry);          // a held BERRY was eaten (CHEEK POUCH, CUD CHEW)
bool8 MP_Gen59_BerriesBlocked(u8 battler);              // UNNERVE / AS ONE on the other side
u8 MP_Gen59_Ripen(u8 battler);                          // 2 with RIPEN, else 1
void MP_Gen59_OnStatRaised(u8 battler, u8 stat, s8 by); // OPPORTUNIST
bool8 MP_Gen59_MovesLast(u8 battler, u16 move);         // MYCELIUM MIGHT
void MP_Gen59_SwitchedOut(u8 battler);                  // ZERO TO HERO
void MP_Gen59_BallFailed(u8 battler, u16 ball);         // BALL FETCH          // GUARD DOG

#endif // GUARD_BATTLE_GEN59_H
