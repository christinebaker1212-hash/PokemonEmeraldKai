/* The Triple Triad collection of this save: "<save>.tt" next to it, kept like the other card games' sidecars
 * (src/tcg/tcg_integration.c, src/ygo/ygo_integration.c): written with the save, used only by the game with that
 * Trainer ID. */
#include "mp_version.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include "global.h"
#include "mp.h"
#include "tt/integration.h"
extern char gPlatformSavePath[512];
static struct TtCollection sCollection;
static int sLoaded;
static uint32_t sLoadedFor;

static uint32_t SaveOwner(void)
{
    const u8 *id = gSaveBlock2Ptr ? gSaveBlock2Ptr->playerTrainerId : NULL;
    return id ? (uint32_t)id[0] | id[1] << 8 | id[2] << 16 | (uint32_t)id[3] << 24 : 0;
}
struct TtCollection *Tt_GetCollection(void)
{
    uint32_t owner = SaveOwner();
    if (!sLoaded || sLoadedFor != owner)
    {
        char path[600];
        Tt_CollectionInit(&sCollection, (uint32_t)SDL_GetPerformanceCounter());
        snprintf(path, sizeof(path), "%s.tt", gPlatformSavePath);
        if (!Tt_CollectionLoad(&sCollection, path) || sCollection.owner != owner)
            Tt_CollectionInit(&sCollection, (uint32_t)SDL_GetPerformanceCounter());
        sCollection.owner = owner;
        sLoadedFor = owner;
        sLoaded = 1;
    }
    return &sCollection;
}
void Tt_WriteSidecar(void)
{
    char path[600];
    if (!sLoaded || sLoadedFor != SaveOwner() || !sCollection.optedIn)
        return;
    snprintf(path, sizeof(path), "%s.tt", gPlatformSavePath);
    Tt_CollectionSave(&sCollection, path);
}
