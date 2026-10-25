#ifndef GUARD_CONSTANTS_BATTLE_GEN4_H
#define GUARD_CONSTANTS_BATTLE_GEN4_H

// Moemon Kai: modes of the "gen4 MODE, ptr" battle script command (src/battle_gen4.c MP_Gen4_Command).
// A mode either goes on (next command) or jumps to ptr: see each one's comment for when it jumps.
#define G4_ROOST              1    // the user loses its FLYING type until the turn ends
#define G4_GRAVITY            2    // jumps if GRAVITY is already on
#define G4_MIRACLE_EYE        3
#define G4_WAKE_TARGET        4    // jumps if the (asleep) target woke up
#define G4_HEALING_WISH       5    // the next Pokémon in on the user's side is healed
#define G4_LUNAR_DANCE        6    // ...and its PP restored
#define G4_FEINT              7    // jumps if the target isn't protected; lifts its protection
#define G4_BUG_BITE           8    // jumps if the target has a BERRY: the user eats it (gBattleMoveDamage = the heal)
#define G4_TAILWIND           9    // jumps if already blowing
#define G4_ACUPRESSURE        10   // jumps if no stat can rise; else picks one (statChanger)
#define G4_METAL_BURST        11   // jumps if no damage to return
#define G4_EMBARGO            12   // jumps on failure
#define G4_FLING              13   // jumps on failure; else sets the power and the item's effect
#define G4_PSYCHO_SHIFT       14   // jumps on failure; else sets the effect to give the target
#define G4_PSYCHO_SHIFT_CURE  15
#define G4_HEAL_BLOCK         16   // jumps on failure
#define G4_POWER_TRICK        17
#define G4_GASTRO_ACID        18   // jumps on failure
#define G4_LUCKY_CHANT        19   // jumps on failure
#define G4_ME_FIRST           20   // runs the target's move, or jumps on failure
#define G4_COPYCAT            21   // runs the last move used, or jumps on failure
#define G4_POWER_SWAP         22
#define G4_GUARD_SWAP         23
#define G4_HEART_SWAP         24
#define G4_LAST_RESORT        25   // jumps on failure
#define G4_WORRY_SEED         26   // jumps on failure
#define G4_SUCKER_PUNCH       27   // jumps on failure
#define G4_TOXIC_SPIKES       28   // jumps on failure
#define G4_AQUA_RING          29   // jumps on failure
#define G4_MAGNET_RISE        30   // jumps on failure
#define G4_FANG_STATUS        31   // jumps unless the fang's status hits (10%); sets the effect
#define G4_DEFOG              32   // clears the target side's screens and hazards; jumps if there were none
#define G4_TRICK_ROOM         33   // jumps if it ended an active TRICK ROOM
#define G4_CAPTIVATE          34   // jumps on failure
#define G4_STEALTH_ROCK       35   // jumps on failure
#define G4_VANISH             36   // SHADOW FORCE's first turn
#define G4_UNVANISH           37
#define G4_FLARE_THAW         38   // FLARE BLITZ thaws its frozen user
#define G4_NATURAL_GIFT       39   // jumps on failure; else sets power and type from the BERRY
#define G4_LIFT_PROTECT       40   // SHADOW FORCE lifts protection, silently
#define G4_SWITCH_EFFECTS     41   // the arriving Pokémon's Gen 4 effects (STEALTH ROCK, TOXIC SPIKES, HEALING WISH)
#define G4_CONSUME_ITEM       42   // the user's item is used up (FLING, NATURAL GIFT)
#define G4_CHARGE_HERB        43   // jumps if a POWER HERB skips the charging turn (used up)
#define G4_RERUN_MOVE         45   // leaves the charging turn's call and runs the move's script again (POWER HERB)
#define G4_ARRIVE_SYNC        44   // HEALING WISH / LUNAR DANCE: sends the healed status and PP (jumps back to itself between them)

#endif // GUARD_CONSTANTS_BATTLE_GEN4_H
