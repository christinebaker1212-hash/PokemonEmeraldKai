/* The Triple Triad collection of this save: "<save>.tt" next to it, kept like the other card games' sidecars
 * (src/tcg/tcg_integration.c, src/ygo/ygo_integration.c): written with the save, used only by the game with that
 * Trainer ID. */
#include "mp_version.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "tt/integration.h"
#include "tt/net.h"
#include "tcg/link.h"
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
static void WorldForSave(int own);
void Tt_WriteSidecar(void)
{
    char path[600];
    if (!sLoaded || sLoadedFor != SaveOwner() || !sCollection.optedIn)
        return;
    snprintf(path, sizeof(path), "%s.tt", gPlatformSavePath);
    WorldForSave(1);   /* (the save keeps the player's own HOENN, never the host's) */
    Tt_CollectionSave(&sCollection, path);
    WorldForSave(0);
}

/* ---- games with friends (tt_net.c) */
static struct TtNet sOnline;
static int sOnlineInit;
static void OnlineSend(void *context, unsigned to, const uint8_t *data, unsigned len)
{
    (void)context;
    MP_SendRelay((u8)to, data, (u16)len);
}
struct TtNet *Tt_Online(void)
{
    if (!sOnlineInit)
    {
        TtNet_Init(&sOnline, OnlineSend, NULL);
        sOnlineInit = 1;
    }
    return &sOnline;
}
void Tt_OnlineReceive(unsigned from, const unsigned char *data, unsigned len, int willing)
{
    TtNet_Receive(Tt_Online(), from, data, len, willing, SDL_GetTicks());
}
static void WorldTick(void);
void Tt_OnlineTick(void)
{
    WorldTick();
    if (sOnlineInit)
        TtNet_Tick(&sOnline, MP_IsConnected() && sOnline.peer < MP_MAX_PLAYERS && gMpPlayers[sOnline.peer].present,
                   SDL_GetTicks());
}

/* ---- HOENN's rules online. The room host sends its HOENN (every region's rules and TRADE RULE, where the CARD QUEEN
 * is and since when) to everyone in the room whenever it changes, every few seconds, and at once to someone new; a
 * friend's collection holds the host's while in the room (its own kept aside: the save always gets that one) and
 * sends the host what changes there (a rule spread or abolished, a CARD QUEEN favor or journey), which the host takes
 * and passes on. Each player's carried rules stay their own. */
struct World
{
    uint32_t rules[TT_REGIONS];
    uint8_t trade[TT_REGIONS], queen;
    uint16_t queenDay;
};
#define WORLD_BYTES (TT_REGIONS * 5 + 3)
#define WORLD_QUEEN_BIT (1u << TT_REGIONS)
static struct
{
    int using;                  /* the collection holds the host's HOENN */
    int8_t host;
    struct World own, last, sent;
    uint8_t knew[MP_MAX_PLAYERS], sentOk;
    unsigned timer, beat;
} sW;

static void WorldGet(struct World *w)
{
    memcpy(w->rules, sCollection.regionRules, sizeof(w->rules));
    memcpy(w->trade, sCollection.regionTrade, sizeof(w->trade));
    w->queen = sCollection.queenRegion;
    w->queenDay = sCollection.queenDay;
}
static void WorldSet(const struct World *w)
{
    memcpy(sCollection.regionRules, w->rules, sizeof(w->rules));
    memcpy(sCollection.regionTrade, w->trade, sizeof(w->trade));
    sCollection.queenRegion = w->queen < TT_REGIONS ? w->queen : sCollection.queenRegion;
    sCollection.queenDay = w->queenDay;
}
static unsigned WorldDiff(const struct World *a, const struct World *b)
{
    unsigned r, mask = 0;
    for (r = 0; r < TT_REGIONS; r++)
        if (a->rules[r] != b->rules[r] || a->trade[r] != b->trade[r])
            mask |= 1u << r;
    if (a->queen != b->queen || a->queenDay != b->queenDay)
        mask |= WORLD_QUEEN_BIT;
    return mask;
}
static void WorldSend(unsigned to, const struct World *w, unsigned kind, unsigned mask)
{
    uint8_t b[8 + WORLD_BYTES];
    unsigned r, n = 0;
    b[n++] = TCG_LINK_RELAY;
    b[n++] = TCG_LINK_VERSION;
    b[n++] = TT_WORLD_TYPE;
    b[n++] = (uint8_t)kind;
    b[n++] = (uint8_t)mask;
    b[n++] = (uint8_t)(mask >> 8);
    for (r = 0; r < TT_REGIONS; r++)
    {
        b[n++] = (uint8_t)w->rules[r];
        b[n++] = (uint8_t)(w->rules[r] >> 8);
        b[n++] = (uint8_t)(w->rules[r] >> 16);
        b[n++] = (uint8_t)(w->rules[r] >> 24);
        b[n++] = w->trade[r];
    }
    b[n++] = w->queen;
    b[n++] = (uint8_t)w->queenDay;
    b[n++] = (uint8_t)(w->queenDay >> 8);
    MP_SendRelay((u8)to, b, (u16)n);
}
static int WorldRead(const unsigned char *d, unsigned len, struct World *w, unsigned *kind, unsigned *mask)
{
    unsigned r, n = 6;
    if (len < 6 + WORLD_BYTES)
        return 0;
    *kind = d[3];
    *mask = d[4] | d[5] << 8;
    for (r = 0; r < TT_REGIONS; r++, n += 5)
    {
        w->rules[r] = ((uint32_t)d[n] | (uint32_t)d[n + 1] << 8 | (uint32_t)d[n + 2] << 16 | (uint32_t)d[n + 3] << 24) &
                      ((1u << TT_RULE_COUNT) - 1);
        w->trade[r] = d[n + 4] < TT_TRADE_COUNT ? d[n + 4] : TT_TRADE_ONE;
    }
    w->queen = d[n] < TT_REGIONS ? d[n] : 0;
    w->queenDay = (uint16_t)(d[n + 1] | d[n + 2] << 8);
    return 1;
}
/* back to the player's own HOENN (left the room, or the host left) */
static void WorldRestore(void)
{
    if (!sW.using)
        return;
    WorldSet(&sW.own);
    sW.using = 0;
    MPNet_Log("tt world: back to our own HOENN");
}
static void WorldForSave(int own)
{
    static struct World host;
    if (!sW.using)
        return;
    if (own)
        WorldGet(&host), WorldSet(&sW.own);
    else
        WorldSet(&host);
}
/* a friend: what changed here since the host's last word goes to the host */
static void WorldSendChanges(void)
{
    struct World cur;
    unsigned mask;
    if (!sW.using || sW.host < 0)
        return;
    WorldGet(&cur);
    mask = WorldDiff(&cur, &sW.last);
    if (!mask)
        return;
    WorldSend((unsigned)sW.host, &cur, 1, mask);
    MPNet_Log("tt world: sent the host a change (%x)", mask);
    sW.last = cur;
}
static void WorldTick(void)
{
    s8 host;
    unsigned i;
    int anyone = 0, someoneNew = 0;
    if (MP_IsConnected() && gMpLocalId >= 0 && gSaveBlock2Ptr)
        Tt_GetCollection();   /* (the host's HOENN goes out even if the case was never opened) */
    if (!sLoaded || !MP_IsConnected() || gMpLocalId < 0)
    {
        if (sLoaded)
            WorldRestore();
        memset(sW.knew, 0, sizeof(sW.knew));
        return;
    }
    host = MP_RoomHostId();
    if (!MP_Lobby_IsCreator())
    {
        memset(sW.knew, 0, sizeof(sW.knew));
        if (host < 0 || (sW.using && host != sW.host))
            WorldRestore();
        WorldSendChanges();
        return;
    }
    /* the host: our HOENN for everyone in the room */
    WorldRestore();
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        int here = (int)i != gMpLocalId && gMpPlayers[i].present;
        someoneNew |= here && !sW.knew[i];
        sW.knew[i] = (uint8_t)here;
        anyone |= here;
    }
    if (!anyone)
    {
        sW.sentOk = 0;
        return;
    }
    sW.beat++;
    if (++sW.timer < 10 && !someoneNew)
        return;
    sW.timer = 0;
    {
        struct World cur;
        WorldGet(&cur);
        if (!someoneNew && sW.sentOk && sW.beat < 60 * 5 && !WorldDiff(&cur, &sW.sent))
            return;
        sW.sent = cur;
        sW.sentOk = 1;
        sW.beat = 0;
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (sW.knew[i])
                WorldSend(i, &cur, 0, 0);
    }
}
void Tt_WorldReceive(unsigned from, const unsigned char *data, unsigned len)
{
    struct World w;
    unsigned kind, mask, r;
    if (!WorldRead(data, len, &w, &kind, &mask))
        return;
    Tt_GetCollection();
    if (kind == 0)
    {
        /* the host's HOENN */
        if (MP_Lobby_IsCreator() || (int)from != MP_RoomHostId())
            return;
        if (!sW.using || sW.host != (int8_t)from)
        {
            if (!sW.using)
                WorldGet(&sW.own);
            sW.using = 1;
            sW.host = (int8_t)from;
            MPNet_Log("tt world: the host's HOENN (queen in %s, %s rules %x)", Tt_RegionName(w.queen),
                      Tt_RegionName(0), w.rules[0]);
        }
        else
            WorldSendChanges();   /* (ours first, in case both crossed) */
        WorldSet(&w);
        sW.last = w;
        return;
    }
    /* a friend's change to our HOENN */
    if (kind != 1 || !MP_Lobby_IsCreator())
        return;
    for (r = 0; r < TT_REGIONS; r++)
        if (mask >> r & 1)
        {
            sCollection.regionRules[r] = w.rules[r];
            sCollection.regionTrade[r] = w.trade[r];
        }
    if (mask & WORLD_QUEEN_BIT)
        sCollection.queenRegion = w.queen, sCollection.queenDay = w.queenDay;
    MPNet_Log("tt world: player %u changed HOENN (%x)", from + 1, mask);
    sW.timer = 10;   /* (everyone hears it at once) */
}
int Tt_WorldFromHost(void) { return sW.using; }
