#ifndef GUARD_MP_LIFE_BATTLE_H
#define GUARD_MP_LIFE_BATTLE_H

// Moemon Co-op: MOéMON GARDEN in battle (src/mp/mp_life_battle.c): standout moments become memories,
// partners and foes become bonds, Pokémon battle emotes, and the exceptionally rare battle lines.
// Presentation only: nothing here changes how a battle goes.

void MP_LifeBattle_Update(void);       // every frame (mp_main.c), in and out of battle
bool8 MP_LifeBattle_TextHold(void);    // battle controllers, when a message finished printing: TRUE = wait
extern bool8 gMpBattleTextPink;        // battle_message.c BattlePutTextOnWindow: the next B_WIN_MSG text is a line
void MP_LifeBattle_Debug(const char *args);
void MP_LifeBattle_OnPeerEmote(u8 battler, u8 icon, bool8 happy);   // mp_battle.c: MPR_BATTLE_MONEMOTE

#endif // GUARD_MP_LIFE_BATTLE_H
