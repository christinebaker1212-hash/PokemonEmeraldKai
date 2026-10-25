#ifndef GUARD_MP_GARDEN_H
#define GUARD_MP_GARDEN_H

// Moemon Co-op: the MOéMON GARDEN (src/mp/mp_garden.c, docs/POKEMON_GARDEN.md 11): PROF. BIRCH's research
// meadow west of LITTLEROOT, where the party lives freely.
#include "mp_life_field.h"

// What a Garden Pokémon does: LIFE_ACT_* where they match, then the Garden's own (sent to friends too)
enum
{
    GA_NONE = LIFE_ACT_NONE,
    GA_WANDER = LIFE_ACT_WANDER,
    GA_NAP = LIFE_ACT_NAP,
    GA_FORAGE = LIFE_ACT_FORAGE,
    GA_PLAY = LIFE_ACT_PLAY,
    GA_COMPANY = LIFE_ACT_COMPANY,
    GA_QUIET = LIFE_ACT_QUIET,
    GA_TYPE = LIFE_ACT_TYPE,
    GA_LOOK = LIFE_ACT_LOOK,
    GA_CHASE = LIFE_ACT_COUNT,   // tag
    GA_SQUABBLE,
    GA_DECOR,
    GA_PURSUIT,     // its own pursuit (docs 12)
    GA_RARE,        // a rare moment with another (LIFE_RARE_*)
    GA_REACT,       // going to see a friend's new body (an evolution)
    GA_SONG,        // a family song
    GA_COUNT
};

bool8 MP_Garden_IsHere(void);
void MP_Garden_OnMapLoad(void);           // fieldmap.c InitMap / InitMapFromSavedGame: growth marks, the last CENTER
void MP_Garden_Update(void);
void MP_Garden_FrameUpdate(void);         // every frame (mp_main.c): the place and its life
void MP_Garden_OpenBoard(void);           // special: the GARDEN BOARD's menu
bool8 MP_Garden_Visited(void);            // quick travel appears after the first visit
bool8 MP_Garden_QuickTravelShown(void);   // start_menu.c: START > GARDEN
void MP_Garden_QuickTravel(bool8 fromPc); // START > GARDEN, the PC's row: off to the Garden, or why not
void MP_Garden_RedirectWarp(s8 destGroup, s8 destNum);   // field_control_avatar.c SetupWarp: the CENTERs' 2F
// decoration.c's MOéMON GARDEN mode (DECORATE on the GARDEN BOARD: the SECRET BASE's decorating), map coords with MAP_OFFSET
void MP_Garden_ShowDecorationMenu(void);   // special (decoration.c)
bool8 MP_GardenDecor_CanPlaceAt(u8 decor, s16 x, s16 y);   // its bottom left tile
bool8 MP_GardenDecor_PlaceAt(u8 decor, s16 x, s16 y);
s8 MP_GardenDecor_OursAt(s16 x, s16 y);    // our decoration covering this tile (-1 none)
bool8 MP_GardenDecor_At(s16 x, s16 y);     // any decoration there (friends', the storm stone too)
void MP_GardenDecor_PutAway(u8 index);
void MP_GardenDecor_Prune(void);          // decorating: a decoration put away loses its sprite at once
void MP_GardenLife_KeepOnReload(void);     // the next map load keeps the Garden Pokémon where they are
void MP_GardenDecor_CheckPlaces(void);    // on load: decorations where nobody could reach them go back
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
