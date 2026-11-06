// Moemon Maker, M3a: the database (docs/MOEMON_MAKER.md "The database"). The user changes a Pokémon's stats, types,
// abilities, learnset and the rest, a move's power, type, accuracy and the rest, and an item's name, price and words
// in the editor (Tools > Database); the edits are MoemonMaker/database.txt, one block an entry, each line keeping the
// value it replaced (was=), as the maps' edits do, so a game update that changes the same value wins and the editor
// shows the clash:
//
//   species SPECIES_TREECKO
//     stats 50 45 35 70 65 55 was=40 45 35 70 65 55       HP ATTACK DEFENSE SPEED SP.ATK SP.DEF
//     types TYPE_GRASS TYPE_DRAGON was=TYPE_GRASS TYPE_GRASS
//     name "TREEKO" was="TREECKO"
//     learnset 1:MOVE_POUND 1:MOVE_LEER 6:MOVE_ABSORB was=0x1A2B3C4D     (the game's learnset's hash)
//   move MOVE_POUND
//     power 50 was=40
//   item ITEM_POTION
//     price 250 was=300
//     description "Restores 20 HP." was=0x5E6F7A8B                      (the game's words' hash)
//
// The game reads these tables through pointers (include/pokemon.h, item.h, data.h: gSpeciesInfo is gSpeciesInfoTable
// and so on), so patched copies are all it takes: battles, the summary, the POKéDEX, shops and the bag read the edited
// values like their own. Moves and abilities keep the game's own effects (decision 23: Lua hooks come in M5).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "data.h"
#include "item.h"
#include "pokemon.h"
#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/characters.h"
#include "battle_main.h"
#include "maker.h"
#include "maker_api.h"
#include "mp.h"

const char *Maker_ReadDir(void);
extern const struct SpeciesInfo gSpeciesInfoBase[];
extern const struct BattleMove gBattleMovesBase[];
extern const u8 gMoveCategoriesBase[];
extern const u32 *const gLevelUpLearnsetsBase[];
extern const struct Item gItemsBase[];
extern const u8 gSpeciesNamesBase[][POKEMON_NAME_LENGTH + 1];
extern const u8 gMoveNamesBase[MOVES_COUNT][MOVE_NAME_LENGTH + 1];

enum { KIND_SPECIES, KIND_MOVE, KIND_ITEM };

// The fields, by kind: their key, how many values, the name table their values come from (-1: numbers)
struct Field
{
    const char *key;
    u8 kind, count;
    s8 table;
};

enum
{
    F_SP_STATS, F_SP_TYPES, F_SP_ABILITIES, F_SP_CATCH, F_SP_EXP, F_SP_EVS, F_SP_ITEMS, F_SP_GENDER, F_SP_EGGCYCLES,
    F_SP_FRIENDSHIP, F_SP_GROWTH, F_SP_EGGGROUPS, F_SP_FLEE, F_SP_COLOR,
    F_MV_POWER, F_MV_TYPE, F_MV_ACCURACY, F_MV_PP, F_MV_CHANCE, F_MV_TARGET, F_MV_PRIORITY, F_MV_FLAGS, F_MV_EFFECT,
    F_MV_CATEGORY,
    F_IT_PRICE, F_IT_HOLD, F_IT_HOLDPARAM,
    F_COUNT
};

static const struct Field sFields[F_COUNT] = {
    [F_SP_STATS] = {"stats", KIND_SPECIES, 6, -1},
    [F_SP_TYPES] = {"types", KIND_SPECIES, 2, MAKER_NAMES_TYPE},
    [F_SP_ABILITIES] = {"abilities", KIND_SPECIES, 2, MAKER_NAMES_ABILITY},
    [F_SP_CATCH] = {"catch", KIND_SPECIES, 1, -1},
    [F_SP_EXP] = {"exp", KIND_SPECIES, 1, -1},
    [F_SP_EVS] = {"evs", KIND_SPECIES, 6, -1},
    [F_SP_ITEMS] = {"items", KIND_SPECIES, 2, MAKER_NAMES_ITEM},
    [F_SP_GENDER] = {"gender", KIND_SPECIES, 1, -1},
    [F_SP_EGGCYCLES] = {"eggcycles", KIND_SPECIES, 1, -1},
    [F_SP_FRIENDSHIP] = {"friendship", KIND_SPECIES, 1, -1},
    [F_SP_GROWTH] = {"growth", KIND_SPECIES, 1, -1},
    [F_SP_EGGGROUPS] = {"egggroups", KIND_SPECIES, 2, -1},
    [F_SP_FLEE] = {"flee", KIND_SPECIES, 1, -1},
    [F_SP_COLOR] = {"color", KIND_SPECIES, 1, -1},
    [F_MV_POWER] = {"power", KIND_MOVE, 1, -1},
    [F_MV_TYPE] = {"type", KIND_MOVE, 1, MAKER_NAMES_TYPE},
    [F_MV_ACCURACY] = {"accuracy", KIND_MOVE, 1, -1},
    [F_MV_PP] = {"pp", KIND_MOVE, 1, -1},
    [F_MV_CHANCE] = {"chance", KIND_MOVE, 1, -1},
    [F_MV_TARGET] = {"target", KIND_MOVE, 1, -1},
    [F_MV_PRIORITY] = {"priority", KIND_MOVE, 1, -1},
    [F_MV_FLAGS] = {"flags", KIND_MOVE, 1, -1},
    [F_MV_EFFECT] = {"effect", KIND_MOVE, 1, -1},
    [F_MV_CATEGORY] = {"category", KIND_MOVE, 1, -1},
    [F_IT_PRICE] = {"price", KIND_ITEM, 1, -1},
    [F_IT_HOLD] = {"hold", KIND_ITEM, 1, -1},
    [F_IT_HOLDPARAM] = {"holdparam", KIND_ITEM, 1, -1},
};

static int SpGet(const struct SpeciesInfo *s, int f, int i)
{
    switch (f)
    {
    case F_SP_STATS:
    {
        const u8 v[6] = {s->baseHP, s->baseAttack, s->baseDefense, s->baseSpeed, s->baseSpAttack, s->baseSpDefense};
        return v[i];
    }
    case F_SP_TYPES: return s->types[i];
    case F_SP_ABILITIES: return s->abilities[i];
    case F_SP_CATCH: return s->catchRate;
    case F_SP_EXP: return s->expYield;
    case F_SP_EVS:
    {
        const u8 v[6] = {s->evYield_HP, s->evYield_Attack, s->evYield_Defense, s->evYield_Speed, s->evYield_SpAttack,
                         s->evYield_SpDefense};
        return v[i];
    }
    case F_SP_ITEMS: return i ? s->itemRare : s->itemCommon;
    case F_SP_GENDER: return s->genderRatio;
    case F_SP_EGGCYCLES: return s->eggCycles;
    case F_SP_FRIENDSHIP: return s->friendship;
    case F_SP_GROWTH: return s->growthRate;
    case F_SP_EGGGROUPS: return s->eggGroups[i];
    case F_SP_FLEE: return s->safariZoneFleeRate;
    case F_SP_COLOR: return s->bodyColor;
    }
    return 0;
}

static void SpSet(struct SpeciesInfo *s, int f, int i, int v)
{
    switch (f)
    {
    case F_SP_STATS:
        switch (i)
        {
        case 0: s->baseHP = v; break;
        case 1: s->baseAttack = v; break;
        case 2: s->baseDefense = v; break;
        case 3: s->baseSpeed = v; break;
        case 4: s->baseSpAttack = v; break;
        default: s->baseSpDefense = v; break;
        }
        break;
    case F_SP_TYPES: s->types[i] = v; break;
    case F_SP_ABILITIES: s->abilities[i] = v; break;
    case F_SP_CATCH: s->catchRate = v; break;
    case F_SP_EXP: s->expYield = v; break;
    case F_SP_EVS:
        v &= 3;
        switch (i)
        {
        case 0: s->evYield_HP = v; break;
        case 1: s->evYield_Attack = v; break;
        case 2: s->evYield_Defense = v; break;
        case 3: s->evYield_Speed = v; break;
        case 4: s->evYield_SpAttack = v; break;
        default: s->evYield_SpDefense = v; break;
        }
        break;
    case F_SP_ITEMS: if (i) s->itemRare = v; else s->itemCommon = v; break;
    case F_SP_GENDER: s->genderRatio = v; break;
    case F_SP_EGGCYCLES: s->eggCycles = v; break;
    case F_SP_FRIENDSHIP: s->friendship = v; break;
    case F_SP_GROWTH: s->growthRate = v; break;
    case F_SP_EGGGROUPS: s->eggGroups[i] = v; break;
    case F_SP_FLEE: s->safariZoneFleeRate = v; break;
    case F_SP_COLOR: s->bodyColor = v; break;
    }
}

static int MvGet(const struct BattleMove *m, u8 category, int f)
{
    switch (f)
    {
    case F_MV_POWER: return m->power;
    case F_MV_TYPE: return m->type;
    case F_MV_ACCURACY: return m->accuracy;
    case F_MV_PP: return m->pp;
    case F_MV_CHANCE: return m->secondaryEffectChance;
    case F_MV_TARGET: return m->target;
    case F_MV_PRIORITY: return m->priority;
    case F_MV_FLAGS: return m->flags;
    case F_MV_EFFECT: return m->effect;
    case F_MV_CATEGORY: return category;
    }
    return 0;
}

static void MvSet(struct BattleMove *m, u8 *category, int f, int v)
{
    switch (f)
    {
    case F_MV_POWER: m->power = v; break;
    case F_MV_TYPE: m->type = v; break;
    case F_MV_ACCURACY: m->accuracy = v; break;
    case F_MV_PP: m->pp = v; break;
    case F_MV_CHANCE: m->secondaryEffectChance = v; break;
    case F_MV_TARGET: m->target = v; break;
    case F_MV_PRIORITY: m->priority = v; break;
    case F_MV_FLAGS: m->flags = v; break;
    case F_MV_EFFECT: m->effect = v; break;
    case F_MV_CATEGORY: *category = v; break;
    }
}

static int ItGet(const struct Item *it, int f)
{
    switch (f)
    {
    case F_IT_PRICE: return it->price;
    case F_IT_HOLD: return it->holdEffect;
    case F_IT_HOLDPARAM: return it->holdEffectParam;
    }
    return 0;
}

static void ItSet(struct Item *it, int f, int v)
{
    switch (f)
    {
    case F_IT_PRICE: it->price = v; break;
    case F_IT_HOLD: it->holdEffect = v; break;
    case F_IT_HOLDPARAM: it->holdEffectParam = v; break;
    }
}

static u32 Fnv(const void *data, size_t n, u32 h)
{
    const u8 *p = data;
    while (n--)
        h = (h ^ *p++) * 16777619u;
    return h;
}

// A species' learnset as the game has it, as a hash (an edit applies only while it's the same)
static u32 LearnsetHash(const u32 *list)
{
    u32 h = 2166136261u;
    int i;
    for (i = 0; list && list[i] != LEVEL_UP_END && i < 256; i++)
        h = Fnv(&list[i], 4, h);
    return h;
}

static u32 TextHash(const u8 *s)
{
    u32 h = 2166136261u;
    while (s && *s != EOS)
        h = (h ^ *s++) * 16777619u;
    return h;
}

// The game's letters as UTF-8 (é and É kept, so a name written back is the same name)
static void ToAscii(char *dst, const u8 *src, int size)
{
    int n = 0;
    for (; src && *src != EOS && n < size - 3; src++)
    {
        if (*src == CHAR_e_ACUTE || *src == CHAR_E_ACUTE)
        {
            dst[n++] = (char)0xC3;
            dst[n++] = (char)(*src == CHAR_e_ACUTE ? 0xA9 : 0x89);
        }
        else
        {
            u8 one[2] = {*src, EOS};
            MP_GameToAscii(dst + n, one, 1);
            n++;
        }
    }
    dst[n] = 0;
}

static bool8 Named(int table, const char *s, int *out)
{
    unsigned v;
    char *end;
    long n;
    if (table >= 0 && MakerApi_ValueOf(table, s, &v))
        return *out = v, TRUE;
    n = strtol(s, &end, 0);
    if (end == s)
        return FALSE;
    *out = n;
    return TRUE;
}

// The patched copies (NULL until an edit needs one)
static struct SpeciesInfo *sSpecies;
static struct BattleMove *sMoves;
static u8 *sCategories;
static const u32 **sLearnsets;
static u32 *sLearnLists[NUM_SPECIES];
static struct Item *sItems;
static u8 *sItemTexts[ITEMS_COUNT + 1];
static u8 (*sSpeciesNames)[POKEMON_NAME_LENGTH + 1];
static u8 (*sMoveNames)[MOVE_NAME_LENGTH + 1];
static int sConflicts;

static void Reset(void)
{
    int i;
    for (i = 0; i < NUM_SPECIES; i++)
        free(sLearnLists[i]), sLearnLists[i] = NULL;
    for (i = 0; i <= ITEMS_COUNT; i++)
        free(sItemTexts[i]), sItemTexts[i] = NULL;
    free(sSpecies), sSpecies = NULL;
    free(sMoves), sMoves = NULL;
    free(sCategories), sCategories = NULL;
    free(sLearnsets), sLearnsets = NULL;
    free(sItems), sItems = NULL;
    free(sSpeciesNames), sSpeciesNames = NULL;
    free(sMoveNames), sMoveNames = NULL;
    gSpeciesInfoTable = gSpeciesInfoBase;
    gBattleMovesTable = gBattleMovesBase;
    gMoveCategoriesTable = gMoveCategoriesBase;
    gLevelUpLearnsetsTable = gLevelUpLearnsetsBase;
    gItemsTable = gItemsBase;
    gSpeciesNamesTable = gSpeciesNamesBase;
    gMoveNamesTable = gMoveNamesBase;
}

static struct SpeciesInfo *Species(int id)
{
    if (!sSpecies)
    {
        sSpecies = malloc(sizeof(*sSpecies) * gSpeciesInfoCount);
        memcpy(sSpecies, gSpeciesInfoBase, sizeof(*sSpecies) * gSpeciesInfoCount);
        gSpeciesInfoTable = sSpecies;
    }
    return &sSpecies[id];
}

static struct BattleMove *Move(int id, u8 **category)
{
    if (!sMoves)
    {
        sMoves = malloc(sizeof(*sMoves) * MOVES_COUNT);
        memcpy(sMoves, gBattleMovesBase, sizeof(*sMoves) * MOVES_COUNT);
        sCategories = malloc(MOVES_COUNT);
        memcpy(sCategories, gMoveCategoriesBase, MOVES_COUNT);
        gBattleMovesTable = sMoves;
        gMoveCategoriesTable = sCategories;
    }
    *category = &sCategories[id];
    return &sMoves[id];
}

static struct Item *ItemEntry(int id)
{
    if (!sItems)
    {
        sItems = malloc(sizeof(*sItems) * gItemsCount);
        memcpy(sItems, gItemsBase, sizeof(*sItems) * gItemsCount);
        gItemsTable = sItems;
    }
    return &sItems[id];
}

// A quoted string on a line ("..." with \" inside), from p; returns the character after it, or NULL
static const char *Quoted(const char *p, char *out, int size)
{
    int n = 0;
    while (*p == ' ')
        p++;
    if (*p != '"')
        return NULL;
    for (p++; *p && *p != '"'; p++)
    {
        if (*p == '\\' && p[1])
            p++;
        if (n < size - 1)
            out[n++] = *p;
    }
    out[n] = 0;
    return *p == '"' ? p + 1 : NULL;
}

static void Conflict(const char *entry, const char *key)
{
    printf("[maker] database: %s %s changed in the game since the edit (left out)\n", entry, key);
    sConflicts++;
}

// One line of an entry: a field, the name, the learnset or the words
static void ApplyLine(int kind, int id, const char *entry, const char *line)
{
    char key[32], text[400];
    const char *rest, *was;
    int f, i, vals[8], olds[8], n = 0, m = 0;
    if (sscanf(line, " %31s", key) != 1)
        return;
    rest = strstr(line, key) + strlen(key);
    was = strstr(rest, " was=");
    if (strcmp(key, "name") == 0)
    {
        const u8 *base = kind == KIND_SPECIES ? gSpeciesNamesBase[id] : kind == KIND_MOVE ? gMoveNamesBase[id] : gItemsBase[id].name;
        int len = kind == KIND_SPECIES ? POKEMON_NAME_LENGTH : kind == KIND_MOVE ? MOVE_NAME_LENGTH : ITEM_NAME_LENGTH - 1;
        char old[64], cur[64];
        u8 enc[64];
        if (!Quoted(rest, text, sizeof(text)))
            return;
        ToAscii(cur, base, sizeof(cur));
        if (was && (!Quoted(was + 5, old, sizeof(old)) || strcmp(old, cur) != 0))
        {
            Conflict(entry, key);
            return;
        }
        memset(enc, EOS, sizeof(enc));
        MP_AsciiToGame(enc, text, len);
        enc[len] = EOS;
        if (kind == KIND_SPECIES)
        {
            if (!sSpeciesNames)
            {
                sSpeciesNames = malloc(sizeof(*sSpeciesNames) * gSpeciesNamesCount);
                memcpy(sSpeciesNames, gSpeciesNamesBase, sizeof(*sSpeciesNames) * gSpeciesNamesCount);
                gSpeciesNamesTable = sSpeciesNames;
            }
            if ((u32)id < gSpeciesNamesCount)
                memcpy(sSpeciesNames[id], enc, POKEMON_NAME_LENGTH + 1);
        }
        else if (kind == KIND_MOVE)
        {
            if (!sMoveNames)
            {
                sMoveNames = malloc(sizeof(*sMoveNames) * MOVES_COUNT);
                memcpy(sMoveNames, gMoveNamesBase, sizeof(*sMoveNames) * MOVES_COUNT);
                gMoveNamesTable = sMoveNames;
            }
            memcpy(sMoveNames[id], enc, MOVE_NAME_LENGTH + 1);
        }
        else
            memcpy(ItemEntry(id)->name, enc, ITEM_NAME_LENGTH);
        return;
    }
    if (strcmp(key, "description") == 0 && kind == KIND_ITEM)
    {
        u8 *enc;
        if (!Quoted(rest, text, sizeof(text)))
            return;
        if (was && strtoul(was + 5, NULL, 0) != TextHash(gItemsBase[id].description))
        {
            Conflict(entry, key);
            return;
        }
        enc = malloc(strlen(text) + 2);
        MP_AsciiToGame(enc, text, strlen(text));
        enc[strlen(text)] = EOS;
        free(sItemTexts[id]);
        sItemTexts[id] = enc;
        ItemEntry(id)->description = enc;
        return;
    }
    if (strcmp(key, "learnset") == 0 && kind == KIND_SPECIES)
    {
        char buf[2048], *tok, *save = NULL;
        u32 *list;
        int count = 0;
        if (id >= NUM_SPECIES)
            return;
        if (was && strtoul(was + 5, NULL, 0) != LearnsetHash(gLevelUpLearnsetsBase[id]))
        {
            Conflict(entry, key);
            return;
        }
        list = malloc(sizeof(u32) * 130);
        snprintf(buf, sizeof(buf), "%.*s", was ? (int)(was - rest) : (int)strlen(rest), rest);
        for (tok = strtok_r(buf, " ", &save); tok && count < 128; tok = strtok_r(NULL, " ", &save))
        {
            char *colon = strchr(tok, ':');
            int lv, mv;
            if (!colon)
                continue;
            *colon = 0;
            lv = atoi(tok);
            if (lv < 0 || lv > 100 || !Named(MAKER_NAMES_MOVE, colon + 1, &mv) || mv <= 0 || mv >= MOVES_COUNT)
                continue;
            list[count++] = (lv << 16) | mv;
        }
        list[count] = LEVEL_UP_END;
        if (!sLearnsets)
        {
            sLearnsets = malloc(sizeof(*sLearnsets) * NUM_SPECIES);
            memcpy(sLearnsets, gLevelUpLearnsetsBase, sizeof(*sLearnsets) * NUM_SPECIES);
            gLevelUpLearnsetsTable = sLearnsets;
        }
        free(sLearnLists[id]);
        sLearnLists[id] = list;
        sLearnsets[id] = list;
        return;
    }
    for (f = 0; f < F_COUNT; f++)
        if (sFields[f].kind == kind && strcmp(sFields[f].key, key) == 0)
            break;
    if (f == F_COUNT)
        return;
    // the values, then the ones they replaced
    {
        char buf[400], *tok, *save = NULL;
        int *dst = vals, *cnt = &n;
        snprintf(buf, sizeof(buf), "%s", rest);
        for (tok = strtok_r(buf, " ", &save); tok; tok = strtok_r(NULL, " ", &save))
        {
            int v;
            if (strncmp(tok, "was=", 4) == 0)
            {
                dst = olds, cnt = &m;
                tok += 4;
                if (!*tok)
                    continue;
            }
            if (*cnt < sFields[f].count && Named(sFields[f].table, tok, &v))
                dst[(*cnt)++] = v;
        }
    }
    if (n != sFields[f].count)
        return;
    for (i = 0; was && i < sFields[f].count; i++)
    {
        int cur = kind == KIND_SPECIES ? SpGet(&gSpeciesInfoBase[id], f, i)
                : kind == KIND_MOVE ? MvGet(&gBattleMovesBase[id], gMoveCategoriesBase[id], f)
                : ItGet(&gItemsBase[id], f);
        if (i >= m || olds[i] != cur)
        {
            Conflict(entry, key);
            return;
        }
    }
    for (i = 0; i < n; i++)
    {
        if (kind == KIND_SPECIES)
            SpSet(Species(id), f, i, vals[i]);
        else if (kind == KIND_MOVE)
        {
            u8 *category;
            struct BattleMove *mv = Move(id, &category);
            MvSet(mv, category, f, vals[i]);
        }
        else
            ItSet(ItemEntry(id), f, vals[i]);
    }
}

// Applies the edits' text (the whole file); returns how many entries it changed
int Maker_SetDatabase(const char *text)
{
    char line[2600], entry[96] = "";
    const char *p = text;
    int kind = -1, id = 0, count = 0;
    Reset();
    sConflicts = 0;
    while (p && *p)
    {
        const char *nl = strchr(p, '\n');
        int len = nl ? (int)(nl - p) : (int)strlen(p);
        char a[96];
        if (len >= (int)sizeof(line))
            len = sizeof(line) - 1;
        memcpy(line, p, len);
        line[len] = 0;
        if (len && line[len - 1] == '\r')
            line[len - 1] = 0;
        p = nl ? nl + 1 : p + strlen(p);
        if (line[0] != ' ' && line[0] != '\t')
        {
            int v;
            kind = -1;
            if (sscanf(line, "species %95s", a) == 1 && Named(MAKER_NAMES_SPECIES, a, &v) && v > 0 && (u32)v < gSpeciesInfoCount)
                kind = KIND_SPECIES;
            else if (sscanf(line, "move %95s", a) == 1 && Named(MAKER_NAMES_MOVE, a, &v) && v > 0 && v < MOVES_COUNT)
                kind = KIND_MOVE;
            else if (sscanf(line, "item %95s", a) == 1 && Named(MAKER_NAMES_ITEM, a, &v) && v > 0 && (u32)v < gItemsCount && v <= ITEMS_COUNT)
                kind = KIND_ITEM;
            else if (line[0] && line[0] != '#')
                printf("[maker] database: %s (left out)\n", line);
            if (kind >= 0)
            {
                id = v;
                snprintf(entry, sizeof(entry), "%s", a);
                count++;
            }
            continue;
        }
        if (kind >= 0)
            ApplyLine(kind, id, entry, line);
    }
    if (count)
        printf("[maker] database: %d edited entr%s%s\n", count, count == 1 ? "y" : "ies", sConflicts ? " (some left out)" : "");
    return count;
}

// When the game starts and when the edits are read again: MoemonMaker/database.txt
void Maker_LoadDatabase(void)
{
    char path[1200];
    FILE *f;
    long size;
    char *text;
    snprintf(path, sizeof(path), "%s/database.txt", Maker_ReadDir());
    if (!(f = fopen(path, "rb")))
    {
        Reset();
        return;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    text = malloc(size + 1);
    if (text && fread(text, 1, size, f) == (size_t)size)
    {
        text[size] = 0;
        Maker_SetDatabase(text);
    }
    free(text);
    fclose(f);
}

// The editor: the database's edits as it has them now
int MakerApi_SetDatabase(const char *text)
{
    return Maker_SetDatabase(text ? text : "");
}

int MakerApi_DatabaseConflicts(void)
{
    return sConflicts;
}

int MakerApi_DatabaseCount(int kind)
{
    switch (kind)
    {
    case MAKER_DB_SPECIES: return NUM_SPECIES;
    case MAKER_DB_MOVE: return MOVES_COUNT;
    case MAKER_DB_ITEM: return gItemsCount < ITEMS_COUNT + 1 ? gItemsCount : ITEMS_COUNT + 1;
    }
    return 0;
}

// A Pokémon as the game has it (its own, never edited), for the editor
int MakerApi_SpeciesInfo(int id, struct MakerSpeciesInfo *info)
{
    const struct SpeciesInfo *s;
    const u32 *list;
    int i;
    if (id <= 0 || id >= NUM_SPECIES || (u32)id >= gSpeciesInfoCount)
        return 0;
    s = &gSpeciesInfoBase[id];
    memset(info, 0, sizeof(*info));
    if ((u32)id < gSpeciesNamesCount)
        ToAscii(info->name, gSpeciesNamesBase[id], sizeof(info->name));
    for (i = 0; i < 6; i++)
        info->stats[i] = SpGet(s, F_SP_STATS, i), info->evs[i] = SpGet(s, F_SP_EVS, i);
    for (i = 0; i < 2; i++)
    {
        info->types[i] = s->types[i];
        info->abilities[i] = s->abilities[i];
        info->items[i] = SpGet(s, F_SP_ITEMS, i);
        info->eggGroups[i] = s->eggGroups[i];
    }
    info->catchRate = s->catchRate;
    info->expYield = s->expYield;
    info->gender = s->genderRatio;
    info->eggCycles = s->eggCycles;
    info->friendship = s->friendship;
    info->growth = s->growthRate;
    info->flee = s->safariZoneFleeRate;
    info->color = s->bodyColor;
    list = gLevelUpLearnsetsBase[id];
    for (i = 0; list && list[i] != LEVEL_UP_END && i < MAKER_LEARNSET_MAX; i++)
    {
        info->learnLevel[i] = (list[i] & LEVEL_UP_MOVE_LV) >> 16;
        info->learnMove[i] = list[i] & LEVEL_UP_MOVE_ID;
    }
    info->learnCount = i;
    info->learnHash = LearnsetHash(list);
    return 1;
}

int MakerApi_MoveInfo(int id, struct MakerMoveInfo *info)
{
    const struct BattleMove *m;
    if (id <= 0 || id >= MOVES_COUNT)
        return 0;
    m = &gBattleMovesBase[id];
    memset(info, 0, sizeof(*info));
    ToAscii(info->name, gMoveNamesBase[id], sizeof(info->name));
    info->power = m->power;
    info->type = m->type;
    info->accuracy = m->accuracy;
    info->pp = m->pp;
    info->chance = m->secondaryEffectChance;
    info->target = m->target;
    info->priority = m->priority;
    info->flags = m->flags;
    info->effect = m->effect;
    info->category = gMoveCategoriesBase[id];
    return 1;
}

int MakerApi_ItemInfo(int id, struct MakerItemInfo *info)
{
    const struct Item *it;
    if (id <= 0 || id > ITEMS_COUNT || (u32)id >= gItemsCount)
        return 0;
    it = &gItemsBase[id];
    memset(info, 0, sizeof(*info));
    ToAscii(info->name, it->name, sizeof(info->name));
    if (it->description)
        ToAscii(info->description, it->description, sizeof(info->description));
    info->price = it->price;
    info->holdEffect = it->holdEffect;
    info->holdParam = it->holdEffectParam;
    info->pocket = it->pocket;
    info->descriptionHash = TextHash(it->description);
    return 1;
}

// The battle bench's damage estimate (M3): ATTACKER at its level uses MOVE on DEFENDER, both with perfect IVs, no EVs
// and a neutral nature, from the tables as they are now (the edits included): the game's damage formula with the
// physical / special split, STAB and the type chart, before abilities, items, weather and critical hits.
// Returns 0 for a move that deals no set damage (status moves, power 0)
extern const u8 gTypeEffectiveness[];

static int TypeTimes10(u8 atk, u8 def)
{
    int i;
    for (i = 0; gTypeEffectiveness[i] != TYPE_ENDTABLE; i += 3)
    {
        if (gTypeEffectiveness[i] == TYPE_FORESIGHT)
            continue;
        if (gTypeEffectiveness[i] == atk && gTypeEffectiveness[i + 1] == def)
            return gTypeEffectiveness[i + 2];   // (TYPE_MUL_*: 0, 5, 10, 20)
    }
    return TYPE_MUL_NORMAL;
}

int MakerApi_Damage(int atkSpecies, int atkLevel, int defSpecies, int defLevel, int move, struct MakerDamage *out)
{
    struct Pokemon *mon = malloc(sizeof(struct Pokemon));
    u32 atkStat, defStat, a[2], d[2];
    s32 base, mult = 100;
    u8 type;
    memset(out, 0, sizeof(*out));
    if (!mon || atkSpecies <= 0 || defSpecies <= 0 || atkSpecies >= NUM_SPECIES || defSpecies >= NUM_SPECIES
     || move <= 0 || move >= MOVES_COUNT)
    {
        free(mon);
        return 0;
    }
    CreateMon(mon, atkSpecies, atkLevel, 31, TRUE, 0, OT_ID_PLAYER_ID, 0);
    a[0] = GetMonData(mon, MON_DATA_ATK);
    a[1] = GetMonData(mon, MON_DATA_SPATK);
    CreateMon(mon, defSpecies, defLevel, 31, TRUE, 0, OT_ID_PLAYER_ID, 0);
    d[0] = GetMonData(mon, MON_DATA_DEF);
    d[1] = GetMonData(mon, MON_DATA_SPDEF);
    out->defenderHp = GetMonData(mon, MON_DATA_MAX_HP);
    free(mon);
    type = gBattleMoves[move].type;
    if (gBattleMoves[move].power == 0 || gMoveCategories[move] == MOVE_CATEGORY_STATUS)
        return 0;
    atkStat = gMoveCategories[move] == MOVE_CATEGORY_SPECIAL ? a[1] : a[0];
    defStat = gMoveCategories[move] == MOVE_CATEGORY_SPECIAL ? d[1] : d[0];
    if (defStat == 0)
        defStat = 1;
    base = ((2 * atkLevel / 5 + 2) * gBattleMoves[move].power * atkStat / defStat) / 50 + 2;
    out->stab = gSpeciesInfo[atkSpecies].types[0] == type || gSpeciesInfo[atkSpecies].types[1] == type;
    if (out->stab)
        base = base * 15 / 10;
    mult = mult * TypeTimes10(type, gSpeciesInfo[defSpecies].types[0]) / 10;
    if (gSpeciesInfo[defSpecies].types[1] != gSpeciesInfo[defSpecies].types[0])
        mult = mult * TypeTimes10(type, gSpeciesInfo[defSpecies].types[1]) / 10;
    out->effectiveness = mult;   // (percent: 0, 25, 50, 100, 200, 400)
    base = base * mult / 100;
    out->max = base;
    out->min = base * 85 / 100;
    if (mult && out->min < 1)
        out->min = 1;
    if (mult && out->max < 1)
        out->max = 1;
    return 1;
}
