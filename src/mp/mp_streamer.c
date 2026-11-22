// Moemon Kai: STREAMER MODE (OPTIONS > CO-OP; the user, 2026-11-05: "obfuscates TRAINER names based on Gender to one
// of 3 styles per activation, Game Style ..., Anime Style ..., and Original Style ..., as well as obfuscation of
// POKEMON NICKNAMES to a selection of 3 nicknames per-species. The name you, other players, and Pokemon receive is
// decided via TID and SID per save file, as well as NATURE for Pokémon").
//
// Only what this screen shows changes: the text printer's filter (mp_textopts.c MP_TextOpts_Filter) swaps each name
// it finds for its stand-in. Names: the player's, every friend's in the room, the link players' in a battle; a
// nickname (one that isn't the species' own name) of the party, the foe's party and the PC's open box. A trainer's
// stand-in is picked by their TID / SID from their gender's list of the style; a POKéMON's from its species' three
// by its OT's TID / SID and its nature. The naming screen shows what is typed.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "battle.h"
#include "data.h"
#include "link.h"
#include "main.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "string_util.h"
#include "constants/characters.h"
#include "mp.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);

bool8 MP_Streamer_On(void)
{
    return gMpConfig.streamer != MP_STREAMER_OFF && gMpConfig.streamer < MP_STREAMER_COUNT;
}

// ------------------------------------------------------------------------------------------------ the names
static const char *const sGameBoys[] = {
    "RED", "GOLD", "ETHAN", "BRENDAN",
    "LUCAS", "HILBERT", "NATE", "CALEM",
    "ELIO", "VICTOR", "FLORIAN", "BLUE",
    "SILVER", "BARRY", "WALLY", "CHEREN",
};
static const char *const sGameGirls[] = {
    "LEAF", "KRIS", "LYRA", "MAY",
    "DAWN", "HILDA", "ROSA", "SERENA",
    "SELENE", "GLORIA", "JULIANA", "YELLOW",
    "GREEN", "BIANCA", "SHAUNA", "LILLIE",
};
static const char *const sAnimeBoys[] = {
    "ASH", "SATOSHI", "BROCK", "TAKESHI",
    "TRACEY", "MAX", "CILAN", "CLEMONT",
    "GOH", "ROY", "PAUL", "GARY",
    "RICHIE", "KIAWE", "ALAIN", "DREW",
};
static const char *const sAnimeGirls[] = {
    "MISTY", "KASUMI", "MAY", "HARUKA",
    "DAWN", "HIKARI", "IRIS", "SERENA",
    "BONNIE", "LILLIE", "CHLOE", "LIKO",
    "MALLOW", "LANA", "ZOEY", "MIETTE",
};
static const char *const sOriginalBoys[] = {
    "JAMES", "JOHN", "MICHAEL", "DAVID",
    "DANIEL", "MATTHEW", "RYAN", "KEVIN",
    "JASON", "BRIAN", "ERIC", "ADAM",
    "LUKE", "OWEN", "HENRY", "LEO",
    "NOAH", "LIAM", "OSCAR", "MARCUS",
};
static const char *const sOriginalGirls[] = {
    "EMMA", "OLIVIA", "SOPHIA", "MIA",
    "CHLOE", "GRACE", "LILY", "EMILY",
    "HANNAH", "ZOE", "ELLA", "AVA",
    "NORA", "RUBY", "CLARA", "ALICE",
    "JULIA", "SARAH", "LAURA", "ANNA",
};

// A trainer's stand-in: by TID / SID from the style's list for their gender
static void TrainerAlias(u32 otId, u8 gender, u8 *out)
{
    const char *const *list;
    u32 n, tid = otId & 0xFFFF, sid = otId >> 16;
    switch (gMpConfig.streamer)
    {
    case MP_STREAMER_GAME:  list = gender == FEMALE ? sGameGirls : sGameBoys, n = ARRAY_COUNT(sGameBoys); break;
    case MP_STREAMER_ANIME: list = gender == FEMALE ? sAnimeGirls : sAnimeBoys, n = ARRAY_COUNT(sAnimeBoys); break;
    default:                list = gender == FEMALE ? sOriginalGirls : sOriginalBoys, n = ARRAY_COUNT(sOriginalBoys); break;
    }
    MP_AsciiToGame(out, list[(tid * 31 + sid * 17 + (tid >> 3)) % n], 8);
}

static bool8 IsVowel(u8 c)
{
    return c == CHAR_A || c == CHAR_E || c == CHAR_I || c == CHAR_O || c == CHAR_U
        || c == CHAR_a || c == CHAR_e || c == CHAR_i || c == CHAR_o || c == CHAR_u;
}

static u8 Lower(u8 c)
{
    return (c >= CHAR_A && c <= CHAR_Z) ? c - CHAR_A + CHAR_a : c;
}

static u8 Upper(u8 c)
{
    return (c >= CHAR_a && c <= CHAR_z) ? c - CHAR_a + CHAR_A : c;
}

// A POKéMON's stand-in: one of three made from its species' name ("Bulby", "Lil Bulba", "Bulster"), by its OT's TID /
// SID and its nature
static void MonAlias(u16 species, u32 otId, u8 nature, u8 *out)
{
    u8 stem[16], n = 0, i, o = 0, pick, name[POKEMON_NAME_LENGTH + 4];
    GetSpeciesName(name, species);
    for (i = 0; name[i] != EOS && n < 8; i++)
        if ((name[i] >= CHAR_A && name[i] <= CHAR_Z) || (name[i] >= CHAR_a && name[i] <= CHAR_z))
            stem[n++] = n == 0 ? Upper(name[i]) : Lower(name[i]);
    if (n < 2)
    {
        StringCopy(out, name);
        return;
    }
    pick = ((otId & 0xFFFF) ^ (otId >> 16) ^ nature) % 3;
    switch (pick)
    {
    case 0:   // the first four letters and a pet ending
        for (i = 0; i < n && i < 4; i++)
            out[o++] = stem[i];
        if (IsVowel(out[o - 1]))
            out[o++] = CHAR_k, out[o++] = CHAR_o;
        else
            out[o++] = CHAR_y;
        break;
    case 1:   // "Lil " and the first five
        out[o++] = CHAR_L, out[o++] = CHAR_i, out[o++] = CHAR_l, out[o++] = CHAR_SPACE;
        for (i = 0; i < n && i < 5; i++)
            out[o++] = stem[i];
        break;
    default:  // the first three and "ster"
        for (i = 0; i < n && i < 3; i++)
            out[o++] = stem[i];
        out[o++] = CHAR_s, out[o++] = CHAR_t, out[o++] = CHAR_e, out[o++] = CHAR_r;
        break;
    }
    out[o] = EOS;
}

// ------------------------------------------------------------------------------------------------ what's on screen
#define MAX_PAIRS 128   // (the players, then up to two a POKeMON: its nickname and its Original Trainer)
static struct { u8 name[POKEMON_NAME_LENGTH + 2], alias[POKEMON_NAME_LENGTH + 2]; u8 len, aliasLen; } sPairs[MAX_PAIRS];
static u8 sPairCount;

static void AddPair(const u8 *name, u8 maxLen, const u8 *alias)
{
    u8 len, i;
    if (sPairCount >= MAX_PAIRS || name[0] == EOS || name[0] == 0xFF)
        return;
    for (len = 0; len < maxLen && name[len] != EOS; len++)
        ;
    if (len < 2)
        return;
    for (i = 0; i < sPairCount; i++)
        if (sPairs[i].len == len && memcmp(sPairs[i].name, name, len) == 0)
            return;
    memcpy(sPairs[sPairCount].name, name, len);
    sPairs[sPairCount].len = len;
    StringCopy(sPairs[sPairCount].alias, alias);
    sPairs[sPairCount].aliasLen = StringLength(alias);
    sPairCount++;
}

static void AddTrainer(const u8 *name, u8 maxLen, u32 otId, u8 gender)
{
    u8 alias[16];
    TrainerAlias(otId, gender, alias);
    AddPair(name, maxLen, alias);
}

// A nickname only (the species' own name stays)
static void AddMon(struct BoxPokemon *box)
{
    u8 nick[POKEMON_NAME_LENGTH + 1], alias[POKEMON_NAME_LENGTH + 2], own[POKEMON_NAME_LENGTH + 4];
    u16 species = GetBoxMonData(box, MON_DATA_SPECIES);
    u32 otId;
    if (species == SPECIES_NONE || species >= NUM_SPECIES || GetBoxMonData(box, MON_DATA_IS_EGG))
        return;
    {
        // its Original Trainer too (a friend's, traded: the summary's OT line), by their own TID / SID and gender
        u8 ot[PLAYER_NAME_LENGTH + 1];
        GetBoxMonData(box, MON_DATA_OT_NAME, ot);
        ot[PLAYER_NAME_LENGTH] = EOS;
        AddTrainer(ot, PLAYER_NAME_LENGTH, GetBoxMonData(box, MON_DATA_OT_ID),
                   GetBoxMonData(box, MON_DATA_OT_GENDER) ? FEMALE : MALE);
    }
    GetBoxMonData(box, MON_DATA_NICKNAME, nick);
    nick[POKEMON_NAME_LENGTH] = EOS;
    GetSpeciesName(own, species);
    if (StringCompare(nick, own) == 0)
        return;
    otId = GetBoxMonData(box, MON_DATA_OT_ID);
    MonAlias(species, otId, GetNatureFromPersonality(GetBoxMonData(box, MON_DATA_PERSONALITY)), alias);
    AddPair(nick, POKEMON_NAME_LENGTH, alias);
}

// Before a string is filtered (mp_textopts.c): the names on screen now and their stand-ins
void MP_Streamer_Build(void)
{
    u8 i;
    static u32 sBuiltAt = 0xFFFFFFFF;
    static u8 sBuiltStyle;
    if (sBuiltAt == gMain.vblankCounter1 && sBuiltStyle == gMpConfig.streamer)
        return;   // (once a frame is enough: the overlays' labels ask for every one they draw)
    sBuiltAt = gMain.vblankCounter1;
    sBuiltStyle = gMpConfig.streamer;
    sPairCount = 0;
    if (gSaveBlock2Ptr == NULL)
        return;
    AddTrainer(gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH,
               gSaveBlock2Ptr->playerTrainerId[0] | (gSaveBlock2Ptr->playerTrainerId[1] << 8)
               | (gSaveBlock2Ptr->playerTrainerId[2] << 16) | ((u32)gSaveBlock2Ptr->playerTrainerId[3] << 24),
               gSaveBlock2Ptr->playerGender);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (gMpPlayers[i].present)
            AddTrainer(gMpPlayers[i].name, MP_NAME_LEN, gMpPlayers[i].otId, gMpPlayers[i].app.gender);
    for (i = 0; i < PARTY_SIZE; i++)
        AddMon(&gPlayerParty[i].box);
    if (gMain.inBattle)
    {
        for (i = 0; i < PARTY_SIZE; i++)
            AddMon(&gEnemyParty[i].box);
        for (i = 0; i < MAX_LINK_PLAYERS; i++)
            if (gLinkPlayers[i].name[0] != EOS && gLinkPlayers[i].name[0] != 0)
                AddTrainer(gLinkPlayers[i].name, PLAYER_NAME_LENGTH, gLinkPlayers[i].trainerId, gLinkPlayers[i].gender);
    }
    else if (gPokemonStoragePtr)
    {
        u8 box = StorageGetCurrentBox();
        for (i = 0; i < IN_BOX_COUNT && box < TOTAL_BOXES_COUNT; i++)
            AddMon(GetBoxedMonPtr(box, i));
    }
}

static bool8 IsNameChar(u8 c)
{
    return (c >= CHAR_A && c <= CHAR_Z) || (c >= CHAR_a && c <= CHAR_z) || (c >= CHAR_0 && c <= CHAR_9);
}

// A name starting at S (the caller knows a word doesn't go on before it): its stand-in into OUT; the stand-in's length,
// USED the name's. 0: none here
u8 MP_Streamer_At(const u8 *s, u8 *out, u16 *used)
{
    u8 i;
    for (i = 0; i < sPairCount; i++)
    {
        u8 len = sPairs[i].len;
        if (memcmp(s, sPairs[i].name, len) == 0 && !IsNameChar(s[len]))
        {
            memcpy(out, sPairs[i].alias, sPairs[i].aliasLen);
            *used = len;
            return sPairs[i].aliasLen;
        }
    }
    return 0;
}

// The game's own drawn labels, outside the text printer (the user, 2026-11-22: "Streamer mode doesn't affect these
// names"): the name tags over players, the speech bubbles (pings, door labels), the FRIEND FINDER's arrows, the
// SPECTATE panel's labels and windows' titles, the Triple Battle cards, the battle chat, the PC's grids, the POKeDEX's
// catchers. Each passes its text through here first: every name on screen becomes its stand-in, as in the text boxes.

// IN (N letters, or to its EOS when N is 0xFFFF) into OUT (CAP bytes with its EOS), names swapped; its length
u16 MP_Streamer_FilterN(const u8 *in, u16 n, u8 *out, u16 cap)
{
    u16 i = 0, o = 0, used;
    bool8 prevWord = FALSE, on = MP_Streamer_On();
    if (cap == 0)
        return 0;
    if (on)
        MP_Streamer_Build();
    while (i < n && in[i] != EOS && o + 1 < cap)
    {
        u8 buf[POKEMON_NAME_LENGTH + 4], k;
        if (on && !prevWord && (k = MP_Streamer_At(&in[i], buf, &used)) != 0 && used <= n - i)
        {
            if (o + k + 1 > cap)
                break;
            memcpy(&out[o], buf, k);
            o += k;
            i += used;
            prevWord = TRUE;
            continue;
        }
        prevWord = IsNameChar(in[i]);
        out[o++] = in[i++];
    }
    out[o] = EOS;
    return o;
}

// The same for an EOS-ended string: OUT, or IN itself when STREAMER MODE is off
const u8 *MP_Streamer_Filter(const u8 *in, u8 *out, u16 cap)
{
    if (!MP_Streamer_On() || in == NULL)
        return in;
    MP_Streamer_FilterN(in, 0xFFFF, out, cap);
    return out;
}

// The same for an ASCII line (the SPECTATE windows' titles, the POKeDEX's own lettering)
void MP_Streamer_FilterAscii(const char *in, char *out, size_t cap)
{
    u8 g[96], f[96];
    if (cap == 0)
        return;
    if (!MP_Streamer_On())
    {
        snprintf(out, cap, "%s", in);
        return;
    }
    MP_AsciiToGame(g, in, sizeof(g) - 1);
    MP_Streamer_FilterN(g, 0xFFFF, f, sizeof(f));
    MP_GameToAscii(out, f, (int)cap - 1);
    out[cap - 1] = 0;
}
