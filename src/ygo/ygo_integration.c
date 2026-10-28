/* The Yu-Gi-Oh! collection of this save: "<save>.ygo" next to it, kept like the Pokemon cards' "<save>.cards"
 * (src/tcg/tcg_integration.c): written with the save, used only by the game with that Trainer ID. */
#include "mp_version.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include "global.h"
#include "ygo/integration.h"
extern char gPlatformSavePath[512];
static struct YgoCollection sCollection;
static int sLoaded;
static uint32_t sLoadedFor;

static uint32_t SaveOwner(void)
{
    const u8 *id = gSaveBlock2Ptr ? gSaveBlock2Ptr->playerTrainerId : NULL;
    return id ? (uint32_t)id[0] | id[1] << 8 | id[2] << 16 | (uint32_t)id[3] << 24 : 0;
}
struct YgoCollection *Ygo_GetCollection(void)
{
    uint32_t owner = SaveOwner();
    if (!sLoaded || sLoadedFor != owner)
    {
        char path[600];
        Ygo_CollectionInit(&sCollection, (uint32_t)SDL_GetPerformanceCounter());
        snprintf(path, sizeof(path), "%s.ygo", gPlatformSavePath);
        if (!Ygo_CollectionLoad(&sCollection, path) || sCollection.owner != owner)
            Ygo_CollectionInit(&sCollection, (uint32_t)SDL_GetPerformanceCounter());
        sCollection.owner = owner;
        sLoadedFor = owner;
        sLoaded = 1;
    }
    return &sCollection;
}
void Ygo_WriteSidecar(void)
{
    char path[600];
    if (!sLoaded || sLoadedFor != SaveOwner() || !sCollection.optedIn)
        return;
    snprintf(path, sizeof(path), "%s.ygo", gPlatformSavePath);
    Ygo_CollectionSave(&sCollection, path);
}
