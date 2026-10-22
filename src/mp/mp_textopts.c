// Moemon Co-op: local text options (OPTIONS > NAME CASE and OPTIONS > CREATURES).
//
// NAME CASE: names shown as they were typed (UNCORRECTED, the game as it always was),
// in ALL CAPS ("KIRA") or in Mixed Case ("Banana", "Ho-Oh"). Only names change: the
// player's, co-op friends', the party's and the foe's nicknames, every species name and
// the NPC trainers' names. Items, places and the rest of the text stay as they are.
//
// CREATURES: MOéMON (Kai's word, the default) or POKéMON. The text side swaps the word
// (Kai turned every POKé into MOé, so MOéMON, MOéDEX, MOé BALL... become POKé again);
// the pictures side is mp_monart.c.
//
// Both are this game's own display: nothing is stored or sent differently. The filter
// runs on every string handed to the text printer (text.c AddTextPrinter), which copies
// the string into a per-window buffer, so the source (a nickname, gStringVar4...) is
// never changed. The naming screen is left alone so a name is shown as it is typed.
#include <stdlib.h>
#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "item.h"
#include "data.h"
#include "link.h"
#include "main.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "string_util.h"
#include "window.h"
#include "constants/abilities.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/opponents.h"
#include "mp.h"

u8 MP_NameCaseMode(void)
{
    return gSaveBlock2Ptr->optionsNameCase;
}

bool8 MP_PokemonWording(void)
{
    return gSaveBlock2Ptr->optionsPokemonArt;
}

// ---------------------------------------------------------------------------
// Characters

static bool8 IsUpper(u8 c)
{
    return (c >= CHAR_A && c <= CHAR_Z) || (c >= CHAR_A_GRAVE && c <= CHAR_N_TILDE && c != 0x0A);
}

static bool8 IsLower(u8 c)
{
    return (c >= CHAR_a && c <= CHAR_z) || (c >= CHAR_a_GRAVE && c <= CHAR_n_TILDE && c != 0x18 && c != 0x1F);
}

static u8 ToUpper(u8 c)
{
    if (c >= CHAR_a && c <= CHAR_z)
        return c - CHAR_a + CHAR_A;
    if (c >= CHAR_a_GRAVE && c <= CHAR_n_TILDE && c != 0x18 && c != 0x1F)
        return c - (CHAR_a_GRAVE - CHAR_A_GRAVE);
    return c;
}

static u8 ToLower(u8 c)
{
    if (c >= CHAR_A && c <= CHAR_Z)
        return c - CHAR_A + CHAR_a;
    if (c >= CHAR_A_GRAVE && c <= CHAR_N_TILDE && c != 0x0A && c != 0x03)
        return c + (CHAR_a_GRAVE - CHAR_A_GRAVE);
    return c;
}

static bool8 IsLetter(u8 c)
{
    return IsUpper(c) || IsLower(c);
}

// A character that can be part of a word (a name can't start or end next to one)
static bool8 IsWordChar(u8 c)
{
    return IsLetter(c) || (c >= CHAR_0 && c <= CHAR_0 + 9);
}

// Case-blind comparison (the CREATURES word swap: MOéMON, Moémon...)
static bool8 SameLetter(u8 a, u8 b)
{
    return ToUpper(a) == ToUpper(b);
}

void MP_NameCase_Recase(u8 *s, u16 len, u8 mode)
{
    u16 i;
    bool8 wordStart = TRUE;
    for (i = 0; i < len && s[i] != EOS; i++)
    {
        u8 c = s[i];
        if (mode == MP_NAMECASE_CAPS)
            s[i] = ToUpper(c);
        else if (mode == MP_NAMECASE_MIXED && IsLetter(c))
            s[i] = wordStart ? ToUpper(c) : ToLower(c);
        // a new word after a space or a hyphen ("Mr. Mime", "Ho-Oh"); not after ' ("Farfetch'd")
        wordStart = !IsLetter(c) && c != CHAR_SGL_QUOTE_RIGHT && c != CHAR_SGL_QUOTE_LEFT && !(c >= CHAR_0 && c <= CHAR_0 + 9);
    }
}

void MP_NameCase_ApplyName(u8 *name, u16 maxLen)
{
    u8 mode = MP_NameCaseMode();
    if (mode != MP_NAMECASE_AS_TYPED)
        MP_NameCase_Recase(name, maxLen, mode);
}

// ---------------------------------------------------------------------------
// The names: species and NPC trainers (built once, by first letter), the player, friends,
// the parties and link players (looked at on every string)

#define MAX_STATIC_NAMES (NUM_SPECIES + TRAINERS_COUNT)
struct NameRef { const u8 *name; u8 len; };
static struct NameRef sStatic[MAX_STATIC_NAMES];
static u16 sBucketStart[27], sBucketCount[27];
static bool8 sStaticBuilt;

static u8 Bucket(u8 c)
{
    c = ToUpper(c);
    return (c >= CHAR_A && c <= CHAR_Z) ? c - CHAR_A : 26;
}

static u8 NameLen(const u8 *name, u8 max)
{
    u8 n = 0;
    while (n < max && name[n] != EOS)
        n++;
    // trailing spaces don't count
    while (n > 0 && name[n - 1] == CHAR_SPACE)
        n--;
    return n;
}

static int CompareRefs(const void *a, const void *b)
{
    const struct NameRef *x = a, *y = b;
    if (Bucket(x->name[0]) != Bucket(y->name[0]))
        return Bucket(x->name[0]) - Bucket(y->name[0]);
    return y->len - x->len;   // the longest first: "NIDORAN♂" before "NIDORAN"
}

// Move, ability and item names of more than one word stay whole, even with a name in them
// (WAILMER PAIL, SERENE GRACE, LEAF STONE, RED ORB)
#define MAX_KEEP (MOVES_COUNT + ABILITIES_COUNT + ITEMS_COUNT)
static struct NameRef sKeep[MAX_KEEP];
static u16 sKeepCount;

static void AddKeep(const u8 *name, u8 max)
{
    u8 len = NameLen(name, max), i;
    for (i = 0; i < len; i++)
        if (name[i] == CHAR_SPACE || name[i] == CHAR_HYPHEN)
            break;
    if (i < len && sKeepCount < MAX_KEEP)
    {
        sKeep[sKeepCount].name = name;
        sKeep[sKeepCount++].len = len;
    }
}

static void BuildStatic(void)
{
    u16 n = 0, i;
    u8 b;
    for (i = 1; i < NUM_SPECIES; i++)
    {
        u8 len = NameLen(gSpeciesNames[i], POKEMON_NAME_LENGTH);
        if (len >= 2 && gSpeciesNames[i][0] != CHAR_QUESTION_MARK)
        {
            sStatic[n].name = gSpeciesNames[i];
            sStatic[n++].len = len;
        }
    }
    for (i = 1; i < TRAINERS_COUNT; i++)
    {
        u8 len = NameLen(gTrainers[i].trainerName, TRAINER_NAME_LENGTH);
        if (len >= 2)
        {
            sStatic[n].name = gTrainers[i].trainerName;
            sStatic[n++].len = len;
        }
    }
    qsort(sStatic, n, sizeof(sStatic[0]), CompareRefs);
    memset(sBucketCount, 0, sizeof(sBucketCount));
    for (i = 0; i < n; i++)
    {
        b = Bucket(sStatic[i].name[0]);
        if (sBucketCount[b]++ == 0)
            sBucketStart[b] = i;
    }
    sKeepCount = 0;
    for (i = 1; i < MOVES_COUNT; i++)
        AddKeep(gMoveNames[i], MOVE_NAME_LENGTH);
    for (i = 1; i < ABILITIES_COUNT; i++)
        AddKeep(gAbilityNames[i], ABILITY_NAME_LENGTH);
    for (i = 1; i < ITEMS_COUNT; i++)
        AddKeep(GetItemName(i), ITEM_NAME_LENGTH);
    sStaticBuilt = TRUE;
}

#define MAX_DYNAMIC_NAMES 64
static struct NameRef sDynamic[MAX_DYNAMIC_NAMES];
static u8 sDynamicCount;

static void AddDynamic(const u8 *name, u8 max)
{
    u8 len;
    if (sDynamicCount >= MAX_DYNAMIC_NAMES || name == NULL)
        return;
    len = NameLen(name, max);
    if (len < 1 || name[0] == EOS)
        return;
    sDynamic[sDynamicCount].name = name;
    sDynamic[sDynamicCount++].len = len;
}

static u8 sNickBuf[PARTY_SIZE * 2 + IN_BOX_COUNT][POKEMON_NAME_LENGTH + 1];

static void BuildDynamic(void)
{
    u8 i, nicks = 0;
    sDynamicCount = 0;
    AddDynamic(gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (gMpPlayers[i].present)
            AddDynamic(gMpPlayers[i].name, MP_NAME_LEN);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == SPECIES_NONE)
            continue;
        GetMonData(&gPlayerParty[i], MON_DATA_NICKNAME, sNickBuf[nicks]);
        AddDynamic(sNickBuf[nicks++], POKEMON_NAME_LENGTH);
    }
    if (gMain.inBattle)
    {
        for (i = 0; i < PARTY_SIZE; i++)
        {
            if (GetMonData(&gEnemyParty[i], MON_DATA_SPECIES) == SPECIES_NONE)
                continue;
            GetMonData(&gEnemyParty[i], MON_DATA_NICKNAME, sNickBuf[nicks]);
            AddDynamic(sNickBuf[nicks++], POKEMON_NAME_LENGTH);
        }
        for (i = 0; i < MAX_LINK_PLAYERS; i++)
            if (gLinkPlayers[i].name[0] != EOS && gLinkPlayers[i].name[0] != 0)
                AddDynamic(gLinkPlayers[i].name, PLAYER_NAME_LENGTH);
    }
    else if (gPokemonStoragePtr)
    {
        // the PC's current box
        u8 box = StorageGetCurrentBox();
        for (i = 0; i < IN_BOX_COUNT && box < TOTAL_BOXES_COUNT; i++)
        {
            if (GetBoxMonDataAt(box, i, MON_DATA_SPECIES) == SPECIES_NONE)
                continue;
            GetBoxMonData(GetBoxedMonPtr(box, i), MON_DATA_NICKNAME, sNickBuf[nicks]);
            AddDynamic(sNickBuf[nicks++], POKEMON_NAME_LENGTH);
        }
    }
}

// Names are matched exactly as they are written: the game's text always spells a name as it
// is stored (STEVEN, PIKACHU, the player's name as typed), while the same letters in another
// case are ordinary words ("If I may...", "I hope...", "red").
static bool8 MatchAt(const u8 *s, const struct NameRef *ref)
{
    u8 i;
    for (i = 0; i < ref->len; i++)
        if (s[i] == EOS || s[i] != ref->name[i])
            return FALSE;
    return !IsWordChar(s[ref->len]);
}

static u8 KeepAt(const u8 *s)
{
    u16 i;
    u8 best = 0;
    for (i = 0; i < sKeepCount; i++)
        if (sKeep[i].len > best && sKeep[i].name[0] == s[0] && MatchAt(s, &sKeep[i]))
            best = sKeep[i].len;
    return best;
}


// The length of the longest name starting at s (0: none)
static u8 NameAt(const u8 *s)
{
    u8 best = 0, b = Bucket(s[0]);
    u16 i;
    for (i = 0; i < sDynamicCount; i++)
        if (sDynamic[i].len > best && MatchAt(s, &sDynamic[i]))
            best = sDynamic[i].len;
    for (i = 0; i < sBucketCount[b]; i++)
    {
        const struct NameRef *ref = &sStatic[sBucketStart[b] + i];
        if (ref->len <= best)
            break;   // sorted longest first
        if (MatchAt(s, ref))
            best = ref->len;
    }
    return best;
}

// ---------------------------------------------------------------------------
// The filter

#define FILTER_BUF 1100
static u8 sFiltered[WINDOWS_MAX + 1][FILTER_BUF];

static u16 CodeLength(const u8 *s)
{
    switch (s[0])
    {
    case EXT_CTRL_CODE_BEGIN:
        return 1 + GetExtCtrlCodeLength(s[1]);
    case PLACEHOLDER_BEGIN:
    case CHAR_DYNAMIC:
    case CHAR_KEYPAD_ICON:
    case CHAR_EXTRA_SYMBOL:
        return 2;
    }
    return 0;
}

// MOé -> POKé (any case), MOEMON -> POKéMON
static u8 WordSwapAt(const u8 *s, u8 *out, u16 *consumed)
{
    if (SameLetter(s[0], CHAR_M) && SameLetter(s[1], CHAR_O) && (s[2] == CHAR_e_ACUTE || s[2] == CHAR_E_ACUTE))
    {
        bool8 upper = IsUpper(s[1]);
        out[0] = IsUpper(s[0]) ? CHAR_P : CHAR_p;
        out[1] = upper ? CHAR_O : CHAR_o;
        out[2] = upper ? CHAR_K : CHAR_k;
        out[3] = s[2];
        *consumed = 3;
        return 4;
    }
    if (SameLetter(s[0], CHAR_M) && SameLetter(s[1], CHAR_O) && SameLetter(s[2], CHAR_E)
     && SameLetter(s[3], CHAR_M) && SameLetter(s[4], CHAR_O) && SameLetter(s[5], CHAR_N) && !IsWordChar(s[6]))
    {
        bool8 upper = IsUpper(s[1]);
        out[0] = IsUpper(s[0]) ? CHAR_P : CHAR_p;
        out[1] = upper ? CHAR_O : CHAR_o;
        out[2] = upper ? CHAR_K : CHAR_k;
        out[3] = CHAR_e_ACUTE;
        out[4] = upper ? CHAR_M : CHAR_m;
        out[5] = upper ? CHAR_O : CHAR_o;
        out[6] = upper ? CHAR_N : CHAR_n;
        *consumed = 6;
        return 7;
    }
    return 0;
}

bool8 gMpTextFilterOff;   // set around text that must show as written (the options' own choices)

const u8 *MP_TextOpts_Filter(const u8 *str, u8 windowId)
{
    bool8 NamingScreen_IsActive(void);
    u8 mode = MP_NameCaseMode();
    bool8 swap = MP_PokemonWording();
    u8 *out;
    u16 i = 0, o = 0;
    bool8 prevWord = FALSE;

    if (str == NULL || gMpTextFilterOff || (mode == MP_NAMECASE_AS_TYPED && !swap) || NamingScreen_IsActive())
        return str;
    out = sFiltered[windowId < WINDOWS_MAX ? windowId : WINDOWS_MAX];
    if (str >= &sFiltered[0][0] && str < &sFiltered[WINDOWS_MAX + 1][0])
        return str;   // already done (a printer started again from its own text)
    if (mode != MP_NAMECASE_AS_TYPED)
    {
        if (!sStaticBuilt)
            BuildStatic();
        BuildDynamic();
    }
    while (str[i] != EOS)
    {
        u16 code = CodeLength(&str[i]), used;
        u8 n;
        if (o + 16 >= FILTER_BUF)
            return str;   // too long to copy: shown as it is
        if (code)
        {
            u16 k;
            for (k = 0; k < code && str[i] != EOS; k++)
                out[o++] = str[i++];
            prevWord = FALSE;
            continue;
        }
        if (swap && (n = WordSwapAt(&str[i], &out[o], &used)) != 0)
        {
            o += n;
            i += used;
            prevWord = TRUE;
            continue;
        }
        if (mode != MP_NAMECASE_AS_TYPED && !prevWord && IsLetter(str[i]) && (n = KeepAt(&str[i])) != 0)
        {
            memcpy(&out[o], &str[i], n);   // a move / ability / item name: as it is
            o += n;
            i += n;
            prevWord = TRUE;
            continue;
        }
        if (mode != MP_NAMECASE_AS_TYPED && !prevWord && IsLetter(str[i]) && (n = NameAt(&str[i])) != 0)
        {
            memcpy(&out[o], &str[i], n);
            MP_NameCase_Recase(&out[o], n, mode);
            o += n;
            i += n;
            prevWord = TRUE;
            continue;
        }
        prevWord = IsWordChar(str[i]);
        out[o++] = str[i++];
    }
    out[o] = EOS;
    return out;
}

// A whole name (name tags, the co-op screens' own text) in this game's chosen case
void MP_NameCase_CopyName(u8 *dst, const u8 *name, u16 maxLen)
{
    StringCopyN(dst, name, maxLen);
    dst[maxLen] = EOS;
    MP_NameCase_ApplyName(dst, maxLen);
}
