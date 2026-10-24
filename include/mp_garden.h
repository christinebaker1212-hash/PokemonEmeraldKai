#ifndef GUARD_MP_GARDEN_H
#define GUARD_MP_GARDEN_H

// Moemon Co-op: the MOéMON GARDEN (src/mp/mp_garden.c, docs/POKEMON_GARDEN.md 11): PROF. BIRCH's research
// meadow west of LITTLEROOT, where the party lives freely.

bool8 MP_Garden_IsHere(void);
void MP_Garden_OnMapLoad(void);           // fieldmap.c InitMap / InitMapFromSavedGame: growth marks, the hedge
void MP_Garden_Update(void);
void MP_Garden_FrameUpdate(void);         // every frame (mp_main.c): the place and its life
void MP_Garden_OpenBoard(void);           // special: the GARDEN BOARD's menu
bool8 MP_Garden_Visited(void);            // the PC's shortcut appears after the first visit
void MP_Garden_ShortcutFromPC(void);      // player_pc.c: off to the Garden once the PC is off
bool8 MP_Garden_TryInteract(void);        // A in front of a Garden Pokémon (mp_overworld.c)
void MP_Garden_Debug(const char *args);
// mp_garden_life.c: the Garden's Pokémon (actors)
void MP_Garden_ActorReaction(s8 actor, u8 emote, u8 pose, u16 frames);
u8 MP_Garden_ActorActivity(s8 actor);       // LIFE_ACT_*
bool8 MP_Garden_ActorAt(s16 x, s16 y);      // map coords (with MAP_OFFSET): a Garden Pokémon stands there
bool8 MP_Garden_HidesFollowers(void);       // in the Garden, followers live as Garden Pokémon
void MP_GardenLife_Update(void);
void MP_GardenLife_OnMapLoad(void);
void MP_GardenNet_OnRelay(u8 from, const u8 *data, u16 len);

#endif // GUARD_MP_GARDEN_H
