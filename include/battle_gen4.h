#ifndef GUARD_BATTLE_GEN4_H
#define GUARD_BATTLE_GEN4_H

// Moemon Kai: Gen 4's battle rules (src/battle_gen4.c). The battle engine stays Gen 3's; these hooks add what
// Gen 4 changed: the physical / special split (OPTIONS > GAME > MOVE KIND), the new moves', abilities' and held
// items' effects, and the field / side / Pokémon states they need. Everything here runs where the battle
// scripts run (the master in link battles); the other games get the results like any other effect.

struct Gen4Battler
{
    u8 embargo;          // turns left
    u8 healBlock;
    u8 magnetRise;
    u8 slowStart;        // turns left of SLOW START
    u8 aquaRing:1;
    u8 miracleEye:1;
    u8 roosted:1;        // ROOST: no FLYING type until the turn ends
    u8 unburden:1;       // lost its item: UNBURDEN doubles its Speed
    u8 powerTrick:1;
    u8 hitThisTurn:1;    // took damage this turn (ASSURANCE)
    u8 movedThisTurn:1;  // has moved this turn (PAYBACK, SUCKER PUNCH)
    u8 choiceLocked:1;
    u16 choiceMove;      // CHOICE SCARF / SPECS lock (CHOICE BAND has its own)
    u16 metronomeMove;   // METRONOME (the item): the move used in a row, and how often
    u8 metronomeCount;
    u8 usedMoves;        // move slots used since it came in (LAST RESORT)
    u8 vanished:1;       // SHADOW FORCE's first turn
    u8 meFirst:1;        // ME FIRST's 1.5x
    u16 lastDamage;      // damage taken this turn and who dealt it (METAL BURST)
    u8 lastAttacker;
    u8 switchInDone;     // the arrival effects already handled this switch-in (G4_ARRIVE_*)
    u8 endStep;          // where its end-of-turn effects are (MP_Gen4_BattlerEndTurn)
    u8 sashUsed:1;       // FOCUS SASH held on this hit: used up when the move ends
    u8 abilityAnnounced:1; // its arrival ability (DOWNLOAD, FRISK...) has acted
    u8 resistBerry:1;    // a type-resist BERRY halved this hit: eaten when the move ends
    u8 micle:1;          // MICLE BERRY eaten: its next move is more accurate
};

struct Gen4Side
{
    u8 tailwind;
    u8 luckyChant;
    u8 toxicSpikes;      // layers, 0-2
    u8 stealthRock:1;
    u8 healingWish:1;    // the next Pokémon in is healed (HEALING WISH)
    u8 lunarDance:1;     // ...and its PP restored (LUNAR DANCE)
};

struct Gen4Field
{
    u8 gravity;
    u8 trickRoom;
    u16 lastMoveUsed;    // COPYCAT
    u8 endStep;          // where the field's end-of-turn counters are (MP_Gen4_FieldEndTurn)
    u8 moveEndDone;      // MP_Gen4_MoveEnd effects already done for this move (bits)
    u8 dealtDamage:1;    // the current move hurt a target
    u8 arriveLunar:1;    // the arriving Pokémon also gets its PP back (LUNAR DANCE)
    u8 arriveSyncStep:1;
};

extern struct Gen4Battler gGen4Battlers[];
extern struct Gen4Side gGen4Sides[];
extern struct Gen4Field gGen4Field;

void MP_Gen4_BattleStart(void);
void MP_Gen4_SwitchIn(u8 battler, bool8 batonPass);
void MP_Gen4_TurnEnd(void);
void MP_Gen4_OnMoveStart(u8 battler, u16 move);
void MP_Gen4_OnDamaged(u8 battler, u8 attacker, s32 damage);
u8 MP_Gen4_Command(u8 mode);                            // the "gen4" battle script command
const u8 *MP_Gen4_ArrivalEffect(u8 battler, u8 bsArg); // STEALTH ROCK, TOXIC SPIKES, HEALING WISH (Cmd_switchineffects)
void MP_Gen4_ArrivalDone(u8 battler);
const u8 *MP_Gen4_BattlerEndTurn(u8 battler);           // one end-of-turn effect at a time; NULL when done
const u8 *MP_Gen4_FieldEndTurn(void);
bool8 MP_Gen4_PoisonHeal(u8 battler);                   // POISON HEAL turns this turn's poison damage into healing
bool8 MP_Gen4_NoIndirectDamage(u8 battler);             // MAGIC GUARD
extern const u16 gGen4TurnStringIds[];

#define MP_IS_CHOICE_ITEM(hold) ((hold) == HOLD_EFFECT_CHOICE_BAND || (hold) == HOLD_EFFECT_CHOICE_SCARF || (hold) == HOLD_EFFECT_CHOICE_SPECS)
u8 MP_Gen4_SwitchInAbility(u8 battler);                 // DOWNLOAD, ANTICIPATION, FOREWARN, FRISK, SNOW WARNING...
const u8 *MP_Gen4_MoveBanned(u8 battler, u16 move);     // GRAVITY / HEAL BLOCK: the script that stops the move
bool8 MP_Gen4_FocusSash(u8 target);
void MP_Gen4_ItemLost(u8 battler);
u8 MP_Gen4_ScreenTurns(u8 battler);                     // LIGHT CLAY
u8 MP_Gen4_WeatherTurns(u8 battler, u8 rockHoldEffect); // the weather rocks
const u8 *MP_Gen4_MoveEnd(void);                        // one move-end effect at a time; NULL when done
const u8 *MP_Gen4_BeforeMove(u8 battler);               // CUSTAP BERRY: the script to run first (it then returns), or NULL

bool8 MP_IsPhysicalMove(u16 move, u8 type);
u8 MP_Gen4_HoldEffect(u8 battler);                      // its item's hold effect, none under EMBARGO / KLUTZ
u8 MP_Gen4_PlateType(u16 item);
bool8 MP_Gen4_IsPunchingMove(u16 move);
s32 MP_Gen4_Stab(s32 damage, u8 attacker);              // ADAPTABILITY
u8 MP_Gen4_MoveType(u16 move, u8 battler);              // NORMALIZE, JUDGMENT, NATURAL GIFT...
u16 MP_Gen4_TargetAbility(u8 attacker, u8 target);      // MOLD BREAKER: the target's ability, or none
bool8 MP_Gen4_IsGrounded(u8 battler);                    // LEVITATE / FLYING / MAGNET RISE, unless GRAVITY / IRON BALL
bool8 MP_Gen4_HasType(u8 battler, u8 type);             // ROOST's lost FLYING type

// damage
u16 MP_Gen4_MovePower(u16 move, u8 attacker, u8 target, u16 power);
void MP_Gen4_AttackStats(u16 move, u8 type, u8 attacker, u8 target, u16 *attack, u16 *spAttack, u16 *defense, u16 *spDefense);
s32 MP_Gen4_FinalDamage(s32 damage, u16 move, u8 type, u8 attacker, u8 target, u8 resultFlags);
u8 MP_Gen4_CritStage(u8 attacker);
u8 MP_Gen4_CritMultiplier(u8 attacker);
u16 MP_Gen4_Accuracy(u16 move, u8 attacker, u8 target, u16 calc);
bool8 MP_Gen4_IgnoresAccuracy(u16 move, u8 attacker, u8 target);
u32 MP_Gen4_OrderKey(u8 battler, u32 speed, u8 holdEffect);

#endif // GUARD_BATTLE_GEN4_H
