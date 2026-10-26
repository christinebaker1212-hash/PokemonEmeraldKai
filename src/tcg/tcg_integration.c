#include "mp_version.h"
#if MOEMON_DEV_BUILD
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "main.h"
#include "mp.h"
#include "item.h"
#include "constants/items.h"
#include "tcg/integration.h"
extern char gPlatformSavePath[512];
static struct TcgCollection sCollection;
static int sLoaded;
struct TcgCollection *Tcg_GetCollection(void)
{
    if (!sLoaded)
    {
        char path[600];
        Tcg_CollectionInit(&sCollection, (uint32_t)SDL_GetPerformanceCounter());
        snprintf(path, sizeof(path), "%s.cards", gPlatformSavePath);
        Tcg_CollectionLoad(&sCollection, path);
        sLoaded = 1;
    }
    return &sCollection;
}
#define NPC(script, name, deck, pack) {script, name, deck, pack}
extern const u8 OldaleTown_EventScript_FootprintsMan[];
extern const u8 PetalburgCity_EventScript_Boy[];
extern const u8 RustboroCity_EventScript_FatMan[];
extern const u8 DewfordTown_EventScript_Woman[];
extern const u8 MauvilleCity_EventScript_Maniac[];
extern const u8 VerdanturfTown_EventScript_Camper[];
extern const u8 FallarborTown_EventScript_Gentleman[];
extern const u8 FortreeCity_EventScript_GameboyKid[];
extern const u8 LilycoveCity_EventScript_RichBoy[];
extern const u8 MossdeepCity_EventScript_NinjaBoy[];
extern const u8 PacifidlogTown_EventScript_Fisherman[];
static const struct
{
    const u8 *script;
    const char *name, *deck;
    unsigned pack;
} sOpponents[] = {
    NPC(OldaleTown_EventScript_FootprintsMan, "Oldale's Card Collector", "CharmanderAndFriends", 0),
    NPC(PetalburgCity_EventScript_Boy, "Petalburg's Young Collector", "BulbasaurAndFriends", 1),
    NPC(RustboroCity_EventScript_FatMan, "Rustboro's Card Enthusiast", "RockCrusher", 0),
    NPC(DewfordTown_EventScript_Woman, "Dewford's Island Collector", "WaterfrontPokemon", 2),
    NPC(MauvilleCity_EventScript_Maniac, "Mauville's Card Maniac", "PowerGenerator", 3),
    NPC(VerdanturfTown_EventScript_Camper, "Verdanturf's Camper", "FlowerGarden", 1),
    NPC(FallarborTown_EventScript_Gentleman, "Fallarbor's Gentleman", "Excavation", 3),
    NPC(FortreeCity_EventScript_GameboyKid, "Fortree's Card Player", "FlyinPokemon", 1),
    NPC(LilycoveCity_EventScript_RichBoy, "Lilycove's Collector", "LovelyNidoran", 2),
    NPC(MossdeepCity_EventScript_NinjaBoy, "Mossdeep's Ninja Boy", "StrangePower", 3),
    NPC(PacifidlogTown_EventScript_Fisherman, "Pacifidlog's Fisherman", "SoundoftheWaves", 2),
};
int Tcg_GrantCase(void)
{
    if (CheckBagHasItem(ITEM_CARD_CASE, 1))
        return 1;
    return AddBagItem(ITEM_CARD_CASE, 1);
}

void Tcg_ResumeGame(void (*callback)(void))
{
    if (callback)
        SetMainCallback2(callback);
}

int Tcg_TryChallenge(void)
{
    const u8 *script = NULL;
    unsigned i;
    s16 x, y;
    u8 objectId;
    if (Tcg_IsActive() || !MP_IsLocalFree() || gPlayerAvatar.tileTransitionState != T_NOT_MOVING)
        return 0;
    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    objectId = GetObjectEventIdByPosition(x, y, PlayerGetElevation());
    if (objectId == OBJECT_EVENTS_COUNT || !gMapHeader.events || MP_Coop_IsNpcHeld(&gObjectEvents[objectId]))
        return 0;
    /* Read immutable NPC templates directly. Ordinary interaction helpers also
     * change LastTalked/SelectedObject globals and must not run on a failed card
     * challenge or a press of the existing co-op button. */
    for (i = 0; i < gMapHeader.events->objectEventCount; i++)
        if (gMapHeader.events->objectEvents[i].localId == gObjectEvents[objectId].localId)
        {
            script = gMapHeader.events->objectEvents[i].script;
            break;
        }
    if (!script)
        return 0;
    for (i = 0; i < ARRAY_COUNT(sOpponents); i++)
        if (script == sOpponents[i].script)
        {
            Tcg_OpenChallenge(sOpponents[i].name, sOpponents[i].deck, sOpponents[i].pack);
            return 1;
        }
    return 0;
}
#endif
