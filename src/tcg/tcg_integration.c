#include "mp_version.h"
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
#include "tcg/online.h"
#include "ygo/integration.h"
#include "tt/integration.h"
#include "ygo/net.h"
#include "tt/net.h"
#include <stdlib.h>
#include <time.h>
extern char gPlatformSavePath[512];
static struct TcgCollection sCollection;
static int sLoaded;
static uint32_t sLoadedFor;
/* The collection belongs to one save: "<save>.cards", next to it, written
 * with it (Tcg_WriteSidecar) and only used by the game with that Trainer ID,
 * so a new game on the same file starts without cards. */
static uint32_t SaveOwner(void)
{
    const u8 *id = gSaveBlock2Ptr ? gSaveBlock2Ptr->playerTrainerId : NULL;
    return id ? (uint32_t)id[0] | id[1] << 8 | id[2] << 16 | (uint32_t)id[3] << 24 : 0;
}
struct TcgCollection *Tcg_GetCollection(void)
{
    uint32_t owner = SaveOwner();
    if (!sLoaded || sLoadedFor != owner)
    {
        char path[600];
        Tcg_CollectionInit(&sCollection, (uint32_t)SDL_GetPerformanceCounter());
        snprintf(path, sizeof(path), "%s.cards", gPlatformSavePath);
        if (!Tcg_CollectionLoad(&sCollection, path) || sCollection.owner != owner)
            Tcg_CollectionInit(&sCollection, (uint32_t)SDL_GetPerformanceCounter());
        sCollection.owner = owner;
        sLoadedFor = owner;
        sLoaded = 1;
    }
    return &sCollection;
}
void Tcg_WriteSidecar(void)
{
    char path[600];
    Ygo_WriteSidecar();   /* the Yu-Gi-Oh! cards: "<save>.ygo" */
    Tt_WriteSidecar();    /* the Triple Triad cards: "<save>.tt" */
    if (!sLoaded || sLoadedFor != SaveOwner() || !sCollection.optedIn)
        return;
    snprintf(path, sizeof(path), "%s.cards", gPlatformSavePath);
    Tcg_CollectionSave(&sCollection, path);
}
static struct TcgLink sOnline;
static int sOnlineInit;
static void OnlineSend(void *context, unsigned to, const uint8_t *data, unsigned len)
{
    (void)context;
    MP_SendRelay(to, data, len);
}
struct TcgLink *Tcg_Online(void)
{
    if (!sOnlineInit)
    {
        TcgLink_Init(&sOnline, OnlineSend, NULL);
        sOnlineInit = 1;
    }
    return &sOnline;
}
void Tcg_OnlineRelay(unsigned from, const unsigned char *data, unsigned len)
{
    if (from >= MP_MAX_PLAYERS || from == (unsigned)gMpLocalId || !gMpPlayers[from].present)
        return;
    if (len < 7 || data[0] != TCG_LINK_RELAY || data[1] != TCG_LINK_VERSION)
        return;
    /* Questions, answers and trades (tcg_online.c); the duel itself (tcg_link.c, Yu-Gi-Oh!'s ygo_net.c, Triple
     * Triad's tt_net.c) */
    if (data[2] >= 0x10)
        Tcg_SocialReceive(from, data, len);
    else if (data[2] >= YGO_NET_FIRST_TYPE && data[2] <= YGO_NET_LAST_TYPE)
        Ygo_OnlineReceive(from, data, len, Tcg_SocialExpectsYgoInvite(from));
    else if (data[2] == TT_NET_TYPE)
        Tt_OnlineReceive(from, data, len, Tcg_SocialExpectsTtInvite(from));
    else if (data[2] == TT_WORLD_TYPE)
        Tt_WorldReceive(from, data, len);
    else
        TcgLink_Receive(Tcg_Online(), from, data, len, Tcg_SocialExpectsInvite(from), SDL_GetTicks());
}
void Tcg_OnlineTick(void)
{
    Tcg_SocialTick();
    Ygo_OnlineTick();
    Tt_OnlineTick();
    if (sOnlineInit)
        TcgLink_Tick(&sOnline, MP_IsConnected() && sOnline.peer < MP_MAX_PLAYERS && gMpPlayers[sOnline.peer].present,
                     SDL_GetTicks());
}
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
int Tcg_CardMasterStar(void)
{
    return (Tcg_GetCollection()->flags & TCG_FLAG_GOLD_CASE) != 0;
}

uint32_t Tcg_DuelSeed(uint32_t *rng)
{
    static int sTest = -1;
    if (sTest < 0)
        sTest = getenv("MOEMON_TEST") != NULL;
    if (!sTest)
        *rng ^= (uint32_t)SDL_GetPerformanceCounter() * 2654435761u ^ (uint32_t)time(NULL) * 40503u;
    if (!*rng)
        *rng = 1;
    return Tcg_Random(rng);
}
