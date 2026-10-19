// Moemon Co-op: one Pokédex for the whole group.
//
// Every Pokémon anyone sees or catches is added to everyone's Pokédex. When
// you join, everyone's Pokédex is merged with yours. The game remembers who
// caught each Pokémon first (shown in the Pokédex), in "<save file>.coopdex".
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "pokedex.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/species.h"
#include "constants/characters.h"
#include "main.h"

extern char gPlatformSavePath[512];
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

#define DEX_BYTES ((NATIONAL_DEX_COUNT + 7) / 8)

enum {
    MPR_DEX_ONE = MPR_DEX_FIRST,   // u16 national number, u8 caught
    MPR_DEX_SYNC,                  // seen[DEX_BYTES], caught[DEX_BYTES]
};

bool8 gMpDexApplying;              // TRUE while we add someone else's entries
static u8 sCaughtBy[NATIONAL_DEX_COUNT + 1][PLAYER_NAME_LENGTH + 1];   // "" = you / unknown
static bool8 sLoaded;
static char sLoadedFor[512];
static bool8 sSynced[MP_MAX_PLAYERS];

static void DexPath(char *out, size_t size)
{
    snprintf(out, size, "%s.coopdex", gPlatformSavePath);
}

static void LoadCredits(void)
{
    FILE *f;
    char path[600], line[128];
    if (sLoaded && strcmp(sLoadedFor, gPlatformSavePath) == 0)
        return;
    sLoaded = TRUE;
    snprintf(sLoadedFor, sizeof(sLoadedFor), "%s", gPlatformSavePath);
    memset(sCaughtBy, 0, sizeof(sCaughtBy));
    DexPath(path, sizeof(path));
    f = fopen(path, "r");
    if (!f)
        return;
    while (fgets(line, sizeof(line), f))
    {
        int nat, i;
        char *p;
        if (line[0] == ';')
            continue;
        nat = (int)strtol(line, &p, 10);
        if (nat < 1 || nat > NATIONAL_DEX_COUNT)
            continue;
        // the name as game-text bytes in hex
        for (i = 0; i < PLAYER_NAME_LENGTH && p && *p; i++)
        {
            unsigned v;
            while (*p == ' ')
                p++;
            if (sscanf(p, "%2x", &v) != 1)
                break;
            sCaughtBy[nat][i] = (u8)v;
            p += 2;
            if (v == EOS)
                break;
        }
        sCaughtBy[nat][PLAYER_NAME_LENGTH] = EOS;
    }
    fclose(f);
}

static void SaveCredits(void)
{
    FILE *f;
    char path[600];
    int nat, i;
    DexPath(path, sizeof(path));
    f = fopen(path, "w");
    if (!f)
        return;
    fprintf(f, "; Moemon Co-op: who caught each Pokemon first (national number, name)\n");
    for (nat = 1; nat <= NATIONAL_DEX_COUNT; nat++)
    {
        char ascii[32];
        if (sCaughtBy[nat][0] == EOS || sCaughtBy[nat][0] == 0)
            continue;
        MP_GameToAscii(ascii, sCaughtBy[nat], PLAYER_NAME_LENGTH);
        fprintf(f, "%d ", nat);
        for (i = 0; i <= PLAYER_NAME_LENGTH; i++)
        {
            fprintf(f, "%02x", sCaughtBy[nat][i]);
            if (sCaughtBy[nat][i] == EOS)
                break;
        }
        fprintf(f, "  ; %s\n", ascii);
    }
    fclose(f);
}

// Who caught it, when it was a friend (NULL: you, or nobody yet)
const u8 *MP_Dex_CaughtBy(u16 nationalNum)
{
    LoadCredits();
    if (nationalNum < 1 || nationalNum > NATIONAL_DEX_COUNT || sCaughtBy[nationalNum][0] == EOS || sCaughtBy[nationalNum][0] == 0)
        return NULL;
    return sCaughtBy[nationalNum];
}

static void SendOne(u16 nat, bool8 caught)
{
    u8 buf[4] = {MPR_DEX_ONE, nat & 0xFF, nat >> 8, caught};
    if (MP_IsConnected() && gMpLocalId >= 0)
        MP_SendRelay(0xFF, buf, sizeof(buf));
}

// From GetSetPokedexFlag: you saw or caught something new
void MP_Dex_OnLocalFlag(u16 nationalNum, bool8 caught)
{
    if (gMpDexApplying || nationalNum < 1 || nationalNum > NATIONAL_DEX_COUNT)
        return;
    SendOne(nationalNum, caught);
}

static void Credit(u16 nat, u8 from)
{
    LoadCredits();
    if (sCaughtBy[nat][0] != EOS && sCaughtBy[nat][0] != 0)
        return;
    StringCopyN(sCaughtBy[nat], gMpPlayers[from].name, PLAYER_NAME_LENGTH);
    sCaughtBy[nat][PLAYER_NAME_LENGTH] = EOS;
}

// Adds one entry from a friend. Returns TRUE if it was new to you.
static bool8 Apply(u16 nat, bool8 caught, u8 from)
{
    bool8 isNew = FALSE;
    if (nat < 1 || nat > NATIONAL_DEX_COUNT)
        return FALSE;
    gMpDexApplying = TRUE;
    if (!GetSetPokedexFlag(nat, FLAG_GET_SEEN))
    {
        GetSetPokedexFlag(nat, FLAG_SET_SEEN);
        isNew = !caught;
    }
    if (caught && !GetSetPokedexFlag(nat, FLAG_GET_CAUGHT))
    {
        GetSetPokedexFlag(nat, FLAG_SET_CAUGHT);
        Credit(nat, from);
        isNew = TRUE;
    }
    gMpDexApplying = FALSE;
    return isNew;
}

static void SendSync(u8 target)
{
    u8 buf[1 + DEX_BYTES * 2];
    u16 nat;
    memset(buf, 0, sizeof(buf));
    buf[0] = MPR_DEX_SYNC;
    for (nat = 1; nat <= NATIONAL_DEX_COUNT; nat++)
    {
        u16 i = (nat - 1) / 8, bit = 1 << ((nat - 1) % 8);
        if (GetSetPokedexFlag(nat, FLAG_GET_SEEN))
            buf[1 + i] |= bit;
        if (GetSetPokedexFlag(nat, FLAG_GET_CAUGHT))
            buf[1 + DEX_BYTES + i] |= bit;
    }
    MP_SendRelay(target, buf, sizeof(buf));
}

void MP_Dex_OnRelay(u8 from, const u8 *data, u16 len)
{
    if (from >= MP_MAX_PLAYERS || !gMpPlayers[from].present)
        return;
    switch (data[0])
    {
    case MPR_DEX_ONE:
        if (len >= 4)
        {
            u16 nat = data[1] | (data[2] << 8);
            if (Apply(nat, data[3] != 0, from) && data[3])
            {
                // "BOB caught PIKACHU! (added to your POKéDEX)"
                u8 text[64], species[POKEMON_NAME_LENGTH + 1];
                u8 name[PLAYER_NAME_LENGTH + 1];
                StringCopyN(name, gMpPlayers[from].name, PLAYER_NAME_LENGTH);
                name[PLAYER_NAME_LENGTH] = EOS;
                GetSpeciesName(species, NationalPokedexNumToSpecies(nat));
                StringCopy(text, name);
                MP_AsciiToGame(text + StringLength(text), " got", 10);
                {
                    u8 nl[] = {CHAR_NEWLINE, EOS};
                    StringAppend(text, nl);
                }
                StringAppend(text, species);
                MP_AsciiToGame(text + StringLength(text), "! New in your dex!", 20);
                MP_Toast(text);
                SaveCredits();
            }
        }
        break;
    case MPR_DEX_SYNC:
        if (len >= 1 + DEX_BYTES * 2)
        {
            u16 nat, added = 0;
            for (nat = 1; nat <= NATIONAL_DEX_COUNT; nat++)
            {
                u16 i = (nat - 1) / 8, bit = 1 << ((nat - 1) % 8);
                bool8 caught = (data[1 + DEX_BYTES + i] & bit) != 0;
                if ((data[1 + i] & bit) || caught)
                    if (Apply(nat, caught, from) && caught)
                        added++;
            }
            if (added)
            {
                char ascii[64], name[16];
                u8 text[64];
                MP_GameToAscii(name, gMpPlayers[from].name, PLAYER_NAME_LENGTH);
                snprintf(ascii, sizeof(ascii), "%s added %d\nPokemon to your dex!", name, added);
                MP_AsciiToGame(text, ascii, 60);
                MP_Toast(text);
                SaveCredits();
            }
        }
        break;
    }
}

// Every frame while in a room: share your Pokédex with anyone new
void MP_Dex_Update(void)
{
    u8 i;
    if (!MP_IsInOverworld())
        return;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        bool8 there = i != gMpLocalId && gMpPlayers[i].present && gMpPlayers[i].hasInfo;
        if (there && !sSynced[i])
        {
            SendSync(i);
            sSynced[i] = TRUE;
        }
        else if (!there)
        {
            sSynced[i] = FALSE;
        }
    }
}
