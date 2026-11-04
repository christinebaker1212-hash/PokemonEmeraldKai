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

void Mtg_DuelistState(void)
{
    unsigned i = gSpecialVar_0x8004;
    gSpecialVar_Result = Mtg_Unlocked();
    MP_AsciiToGame(gStringVar1, i < MTG_DUELISTS ? gMtgDuelists[i].name : "", 64);
}

// The duelist's words in gStringVar4 for the map's message box (as the other card games' duelists speak before the
// challenge): broken into lines that fit the box, two to a page
void Mtg_DuelistHello(void)
{
    unsigned i = gSpecialVar_0x8004;
    const char *p = i < MTG_DUELISTS ? gMtgDuelists[i].hello : "";
    u8 *out = gStringVar4, line[160], word[64];
    int lines = 0, n = 0;
    char plain[400];
    Mtg_PlainText(plain, sizeof(plain), p);
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
    } else if (sscanf(command, "duel %u", &n) == 1)
        MtgUi_OpenChallenge(n);
    else if (sscanf(command, "autoplay %u", &n) == 1)
        MtgUi_DebugAutoplay((int)n);
    else if (strncmp(command, "focus ", 6) == 0) {
        void MtgBoard_DebugFocus(const char *name);
        MtgBoard_DebugFocus(command + 6);
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
