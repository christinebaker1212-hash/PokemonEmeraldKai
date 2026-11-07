// Moemon Kai: Magic in the game: the built-in pack loaded once, the save's collection "<save>.mtg" (kept like the
// other card games' sidecars: written with the save, used only by the game with that Trainer ID), the map scripts'
// specials for MOSSDEEP's CARD SHOP (data/scripts/mtg_shop.inc), and "mp mtg ..." for the test harness.
#include "mp_version.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "string_util.h"
#include "text.h"
#include "mp.h"
#include "mtg/integration.h"
#include "tcg/integration.h"
#include "tcg/tcg.h"

extern char gPlatformSavePath[512];
extern const uint8_t gMtgForgePak[], gMtgForgePakEnd[];

static MtgCollection sCollection;
static int sLoaded, sDbTried, sDbOk;
static uint32_t sLoadedFor;

int Mtg_EnsureDb(void)
{
    if (!sDbTried) {
        uint32_t t = SDL_GetTicks();
        sDbTried = 1;
        sDbOk = Mtg_DbLoad(gMtgForgePak, (uint32_t)(gMtgForgePakEnd - gMtgForgePak)) > 0;
        MPNet_Log("mtg: %d cards read in %u ms", Mtg_DbCount(), SDL_GetTicks() - t);
    }
    return sDbOk;
}

static uint32_t SaveOwner(void)
{
    const u8 *id = gSaveBlock2Ptr ? gSaveBlock2Ptr->playerTrainerId : NULL;
    return id ? (uint32_t)id[0] | id[1] << 8 | id[2] << 16 | (uint32_t)id[3] << 24 : 0;
}

MtgCollection *Mtg_GetCollection(void)
{
    uint32_t owner = SaveOwner();
    Mtg_EnsureDb();
    if (!sLoaded || sLoadedFor != owner) {
        char path[600];
        Mtg_CollectionInit(&sCollection, (uint32_t)SDL_GetPerformanceCounter());
        snprintf(path, sizeof(path), "%s.mtg", gPlatformSavePath);
        if (!Mtg_CollectionLoad(&sCollection, path) || sCollection.owner != owner)
            Mtg_CollectionInit(&sCollection, getenv("MOEMON_TEST") ? 12345u   // (the test harness: the same duels)
                                                                   : (uint32_t)SDL_GetPerformanceCounter() | 1);
        sCollection.owner = owner;
        sLoadedFor = owner;
        sLoaded = 1;
    }
    return &sCollection;
}

int Mtg_Unlocked(void)
{
    return Mtg_GetCollection()->optedIn;
}

void Mtg_WriteSidecar(void)
{
    char path[600];
    if (!sLoaded || sLoadedFor != SaveOwner() || !sCollection.optedIn)
        return;
    snprintf(path, sizeof(path), "%s.mtg", gPlatformSavePath);
    if (!Mtg_CollectionSave(&sCollection, path))
        MPNet_Log("mtg: couldn't write %s", path);
}

void Mtg_ForgetSidecar(void)
{
    sLoaded = 0;
}

// ---- MOSSDEEP's CARD SHOP
void Mtg_ShopState(void)
{
    gSpecialVar_Result = Mtg_Unlocked();
}

void Mtg_ShopStarter(void)
{
    MtgUi_OpenStarter();
}

void Mtg_ShopBoosters(void)
{
    MtgUi_OpenShop();
}

void Mtg_DuelistState(void)
{
    gSpecialVar_Result = Mtg_Unlocked();
    MP_AsciiToGame(gStringVar1, Mtg_Opponent(gSpecialVar_0x8004)->name, 64);
}

// Words for the map's message box (as the other card games' duelists speak before the challenge) in gStringVar4:
// broken into lines that fit the box, two to a page
void Mtg_WrapToVar4(const char *text)
{
    const char *p;
    u8 *out = gStringVar4, line[160], word[64];
    int lines = 0, n = 0;
    char plain[400];
    Mtg_PlainText(plain, sizeof(plain), text);
    p = plain;
    line[0] = EOS;
    while (*p) {
        int len = 0;
        u8 trial[224];
        while (*p == ' ')
            p++;
        while (p[len] && p[len] != ' ' && len < 60)
            len++;
        if (!len)
            break;
        {
            char w[64];
            memcpy(w, p, (size_t)len);
            w[len] = 0;
            MP_AsciiToGame(word, w, sizeof(word));
        }
        p += len;
        StringCopy(trial, line);
        if (trial[0] != EOS)
            StringAppend(trial, (const u8 *)"\x00\xFF");   // (a space)
        StringAppend(trial, word);
        if (line[0] != EOS && GetStringWidth(FONT_NORMAL, trial, 0) > 200) {
            out = StringCopy(out, line);
            *out++ = ++lines % 2 ? 0xFE : 0xFA;   // \n, or \p after a page's two lines
            StringCopy(line, word);
        } else
            StringCopy(line, trial);
        n++;
    }
    out = StringCopy(out, line);
    *out = EOS;
    (void)n;
}

// VAR_0x8004 the duelist: their words, in lines, in gStringVar4
void Mtg_DuelistHello(void)
{
    Mtg_WrapToVar4(Mtg_Opponent(gSpecialVar_0x8004)->hello);
}

void Mtg_DuelistChallenge(void)
{
    MtgUi_OpenChallenge(gSpecialVar_0x8004);
}

// ---- "mp mtg ..." (the test harness)
void MtgUi_DebugAutoplay(int on);
void MtgUi_DebugEnd(int win);
void Mtg_Debug(const char *command)
{
    MtgCollection *c = Mtg_GetCollection();
    unsigned n = 0;
    if (sscanf(command, "starter %u", &n) == 1) {
        Mtg_GiveStarter(c, n);
        MPNet_Log("mtg: starter %s given", gMtgStarters[c->starter].name);
    } else if (strncmp(command, "state", 5) == 0) {
        int i, owned = 0, kinds = 0;
        for (i = 0; i < Mtg_DbCount(); i++)
            owned += c->owned[i], kinds += c->owned[i] != 0;
        MPNet_Log("mtg: state optedIn %u deck %u (%s, %u cards) wins %u losses %u beaten %x owned %d (%d kinds)",
                  c->optedIn, c->activeDeck, c->decks[c->activeDeck].name, c->decks[c->activeDeck].count, c->wins,
                  c->losses, c->beaten, owned, kinds);
        MPNet_Log("mtg: counter cp %u decks bought %u prizes %x", Tcg_GetCollection()->dust, c->preconsBought,
                  c->rewards);
        for (i = 0; i < MTG_DECKS; i++)
            if (c->decks[i].count)
                MPNet_Log("mtg: deck slot %d %s (%u cards)", i, c->decks[i].name, c->decks[i].count);
    } else if (strncmp(command, "club", 4) == 0) {   // club N: the play room of half hour N, the next time in
        Mtg_ClubDebug(command + 4);
    } else if (strncmp(command, "own ", 4) == 0) {   // own N NAME: copies of a card in the collection
        int at = 0, id;
        if (sscanf(command + 4, "%u %n", &n, &at) == 1 && at && (id = Mtg_CardByName(command + 4 + at)) >= 0) {
            c->owned[id] = (uint8_t)(n > MTG_OWNED_MAX ? MTG_OWNED_MAX : n);
            MPNet_Log("mtg: own %u %s", n, Mtg_Card(id)->name);
        } else
            MPNet_Log("mtg: own N NAME");
    } else if (strncmp(command, "shop", 4) == 0) {
        void MtgUi_OpenShop(void);
        MtgUi_OpenShop();
    } else if (sscanf(command, "duel %u", &n) == 1)
        MtgUi_OpenChallenge(n);
    else if (sscanf(command, "autoplay %u", &n) == 1)
        MtgUi_DebugAutoplay((int)n);
    else if (strncmp(command, "focus ", 6) == 0) {
        void MtgBoard_DebugFocus(const char *name);
        MtgBoard_DebugFocus(command + 6);
    } else if (sscanf(command, "points %u", &n) == 1) {
        Tcg_GetCollection()->dust = n;
        MPNet_Log("mtg: %u card points", n);
    } else if (strncmp(command, "pack ", 5) == 0) {
        char code[8];
        int set;
        if (sscanf(command + 5, "%7s %u", code, &n) == 2 && (set = Mtg_SetByCode(code)) >= 0) {
            c->packs[set] = (uint8_t)(n > MTG_PACKS_MAX ? MTG_PACKS_MAX : n);
            MPNet_Log("mtg: %u %s boosters", n, code);
        } else
            MPNet_Log("mtg: pack CODE COUNT");
    } else if (strncmp(command, "put ", 4) == 0) {
        void MtgBoard_DebugPut(int player, const char *zone, const char *name);
        char zone[16];
        int at = 0;
        if (sscanf(command + 4, "%u %15s %n", &n, zone, &at) == 2 && at)
            MtgBoard_DebugPut((int)n, zone, command + 4 + at);
        else
            MPNet_Log("mtg: put PLAYER hand|field|grave NAME");
    } else if (strncmp(command, "search ", 7) == 0) {   // SEARCH & SORT's settings, as "mp cards search" (tcg_search.c)
        void MtgUi_DebugSearch(const char *what);
        MtgUi_DebugSearch(command + 7);
    } else if (strncmp(command, "table", 5) == 0) {
        void MtgBoard_DebugState(void);
        MtgBoard_DebugState();
    } else if (strncmp(command, "win", 3) == 0)
        MtgUi_DebugEnd(1);
    else if (strncmp(command, "lose", 4) == 0)
        MtgUi_DebugEnd(0);
    else if (strncmp(command, "save", 4) == 0) {
        Mtg_WriteSidecar();
        MPNet_Log("mtg: saved");
    } else
        MPNet_Log("mtg: unknown command %s", command);
}
