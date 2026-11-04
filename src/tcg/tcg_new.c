/* Moemon Kai: NEW on cards in the BINDER of every card game. "<save>.cardnew" keeps, per game, how many copies of
 * each card the BINDER has shown (the cursor on it); a card owned more times than that is NEW (got from a pack, a prize,
 * a trade, a duel won: nothing needs to tell this file). The first time (no file, or another game begun on the save)
 * everything owned counts as seen. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "tcg_gfx.h"
#include "global.h"
#include "tcg/tcg.h"
#include "tcg/integration.h"
#include "ygo/card_ids.h"
#include "ygo/collection.h"
#include "tt/card_ids.h"
#include "tt/collection.h"
#include "mtg/collection.h"
#include "mtg/integration.h"
#include "tcg_new.h"

extern char gPlatformSavePath[512];
struct YgoCollection *Ygo_GetCollection(void);
struct TtCollection *Tt_GetCollection(void);

#define NEW_MAGIC 0x57454E43   /* "CNEW" */
/* (MAGIC's cards by their place in the pack, MTG_COLLECTION_CARDS of them; a file from before MAGIC had three games) */
static const unsigned sCount[CARDNEW_GAMES] = {TCG_CARD_COUNT + 1, YGO_CARD_COUNT + 1, TT_CARD_MAX + 1, MTG_COLLECTION_CARDS};
static uint8_t sSeenPokemon[TCG_CARD_COUNT + 1], sSeenYugioh[YGO_CARD_COUNT + 1], sSeenTriad[TT_CARD_MAX + 1];
static uint8_t sSeenMagic[MTG_COLLECTION_CARDS];
static uint8_t *const sSeen[CARDNEW_GAMES] = {sSeenPokemon, sSeenYugioh, sSeenTriad, sSeenMagic};
static int sLoaded;
static uint32_t sOwner;

static unsigned Owned(unsigned game, unsigned card)
{
    switch (game)
    {
    case CARDNEW_POKEMON: return Tcg_GetCollection()->owned[card];
    case CARDNEW_YUGIOH:  return Ygo_GetCollection()->owned[card];
    case CARDNEW_MAGIC:   return Mtg_GetCollection()->owned[card];
    default:              return Tt_GetCollection()->owned[card];
    }
}

static void Path(char *out, size_t size)
{
    snprintf(out, size, "%s.cardnew", gPlatformSavePath);
}

static void Save(void)
{
    char path[600];
    FILE *f;
    unsigned g;
    uint32_t head[2] = {NEW_MAGIC, sOwner};
    Path(path, sizeof(path));
    f = fopen(path, "wb");
    if (f == NULL)
        return;
    fwrite(head, sizeof(head), 1, f);
    for (g = 0; g < CARDNEW_GAMES; g++)
    {
        uint32_t n = sCount[g];
        fwrite(&n, 4, 1, f);
        fwrite(sSeen[g], 1, n, f);
    }
    fclose(f);
}

static void Load(void)
{
    char path[600];
    FILE *f;
    unsigned g, k;
    uint32_t head[2] = {0, 0};
    sOwner = Tcg_GetCollection()->owner;
    sLoaded = 1;
    Path(path, sizeof(path));
    f = fopen(path, "rb");
    if (f != NULL && fread(head, sizeof(head), 1, f) == 1 && head[0] == NEW_MAGIC && head[1] == sOwner)
    {
        for (g = 0; g < CARDNEW_GAMES; g++)
        {
            uint32_t n = 0;
            memset(sSeen[g], 0, sCount[g]);
            if (fread(&n, 4, 1, f) != 1)
            {
                /* (a game the file doesn't have yet: what's owned now counts as seen) */
                for (; g < CARDNEW_GAMES; g++)
                    for (k = 0; k < sCount[g]; k++)
                    {
                        unsigned o = Owned(g, k);
                        sSeen[g][k] = o > 255 ? 255 : (uint8_t)o;
                    }
                break;
            }
            for (k = 0; k < n; k++)
            {
                int b = fgetc(f);
                if (b == EOF)
                    break;
                if (k < sCount[g])
                    sSeen[g][k] = (uint8_t)b;
            }
        }
        fclose(f);
        return;
    }
    if (f != NULL)
        fclose(f);
    /* the first time: everything owned now has been seen */
    for (g = 0; g < CARDNEW_GAMES; g++)
        for (k = 0; k < sCount[g]; k++)
        {
            unsigned o = Owned(g, k);
            sSeen[g][k] = o > 255 ? 255 : (uint8_t)o;
        }
    Save();
}

static void Check(void)
{
    if (!sLoaded || sOwner != Tcg_GetCollection()->owner)
        Load();
}

int CardNew_Is(unsigned game, unsigned card, unsigned owned)
{
    if (game >= CARDNEW_GAMES || card >= sCount[game] || owned == 0)
        return 0;
    Check();
    return owned > sSeen[game][card];
}

void CardNew_Saw(unsigned game, unsigned card, unsigned owned)
{
    uint8_t v = owned > 255 ? 255 : (uint8_t)owned;
    if (game >= CARDNEW_GAMES || card >= sCount[game])
        return;
    Check();
    if (sSeen[game][card] != v)
    {
        sSeen[game][card] = v;
        Save();
    }
}

void CardNew_Badge(int x, int y, unsigned frame)
{
    unsigned glow = (frame / 16) % 2 ? 0xffe050 : 0xf8c828;
    G_RoundFill(x, y, 19, 9, 0xd02818, 240);
    G_Text(x + 2, y - 1, glow, 0x501008, "NEW", 0, TF_NARROW);
}

void CardNew_Forget(void)
{
    sLoaded = 0;
}
