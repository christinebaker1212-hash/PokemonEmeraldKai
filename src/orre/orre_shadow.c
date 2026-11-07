// Moemon Kai: Shadow Pokémon (docs/SHADOW_POKEMON.md): Colosseum's table and rules (src/data/orre_shadow.h, from the
// game's own tables by tools/orre/shadow_data.py) and each Shadow Pokémon's state, kept in "<save>.orre" with
// Colosseum's story flags (src/orre/orre_story.c): saves never change layout. A Shadow Pokémon is known by its
// personality and Original Trainer ID (as the Garden's records are).
//
// The heart gauge is Colosseum's "dark points" (DP), kept in hundredths as the game keeps them (status 0xC5 = DP * 100):
//   a drop (pokemon_range_8011F5FC.c pokemonAddDpFormPokemonDpFilterId): DP += source * kake / waru by the nature's
//     rate for that source (scents times their strength), never below 0; ORRE_DP_EMPTY empties it
//   its level (pokemon_range_8011F77C.c pokemonGetDarkPokemonLevel): DP / the gauge's size >= 100% 0, >= 80% 1,
//     >= 60% 2, >= 40% 3, >= 20% 4, > 0 5, else 6 (open: ready to be purified)
//   steps (field_exact_801ED3B8.c): each Shadow Pokémon in the party counts its steps, and on the 257th the walking
//     drop is taken and the count starts again; the day care's the same, with the day care's drop
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "pokemon.h"
#include "orre.h"
#include "orre_shadow.h"
#include "aw_file.h"   // (sidecars written in the background: src/platform/aw_file.c)

void MPNet_Log(const char *fmt, ...);
extern char gPlatformSavePath[];
u8 *Orre_StoryFlagBytes(u32 *size);
void Orre_StoryReset(void);

#include "../data/orre_shadow.h"

#define ORRE_MAGIC 0x4552524F   // "ORRE"
#define ORRE_FORMAT 1
#define MAX_RECS 128
#define STEPS_PER_DROP 0x100

// a Shadow Pokémon's state (written as it is: the sidecar's records)
struct OrreShadowRec
{
    u32 personality, otId;
    u8 id;              // Colosseum's Shadow id
    u8 flags;           // REC_HYPER: in Hyper Mode
    u16 steps;          // toward the next step drop
    s32 dp;             // the heart gauge, hundredths of DP
    u32 storedExp;      // EXP earned while Shadow, given back when purified
    u16 gauge;          // the gauge's size (DP)
    u8 traits[10];      // (the A-Life's dark traits: docs/SHADOW_POKEMON.md "In the A-Life")
};

#define REC_HYPER 1

static struct OrreShadowRec sRecs[MAX_RECS];
static u16 sRecCount;

// ---- Colosseum's table

u8 Orre_Shadow_Count(void)
{
    return ARRAY_COUNT(sOrreShadows);
}

const struct OrreShadowMon *Orre_Shadow_DataAt(u8 index)
{
    return index < ARRAY_COUNT(sOrreShadows) ? &sOrreShadows[index] : NULL;
}

const struct OrreShadowMon *Orre_Shadow_Data(u8 id)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sOrreShadows); i++)
        if (sOrreShadows[i].id == id)
            return &sOrreShadows[i];
    return NULL;
}

// ---- the records

static struct OrreShadowRec *Find(u32 personality, u32 otId)
{
    u32 i;
    for (i = 0; i < sRecCount; i++)
        if (sRecs[i].personality == personality && sRecs[i].otId == otId)
            return &sRecs[i];
    return NULL;
}

static struct OrreShadowRec *RecOfBox(struct BoxPokemon *box)
{
    if (box == NULL || GetBoxMonData(box, MON_DATA_SPECIES) == SPECIES_NONE)
        return NULL;
    return Find(GetBoxMonData(box, MON_DATA_PERSONALITY), GetBoxMonData(box, MON_DATA_OT_ID));
}

static struct OrreShadowRec *RecOf(struct Pokemon *mon)
{
    return mon ? RecOfBox(&mon->box) : NULL;
}

bool8 Orre_Shadow_IsBox(struct BoxPokemon *box)
{
    return RecOfBox(box) != NULL;
}

bool8 Orre_Shadow_Is(struct Pokemon *mon)
{
    return RecOf(mon) != NULL;
}

u8 Orre_Shadow_Id(struct Pokemon *mon)
{
    struct OrreShadowRec *r = RecOf(mon);
    return r ? r->id : 0;
}

bool8 Orre_Shadow_Make(struct Pokemon *mon, u8 id)
{
    const struct OrreShadowMon *data = Orre_Shadow_Data(id);
    struct OrreShadowRec *r;
    if (data == NULL || mon == NULL || GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
        return FALSE;
    if ((r = RecOf(mon)) == NULL)
    {
        if (sRecCount == MAX_RECS)
            return FALSE;
        r = &sRecs[sRecCount++];
    }
    memset(r, 0, sizeof(*r));
    r->personality = GetMonData(mon, MON_DATA_PERSONALITY);
    r->otId = GetMonData(mon, MON_DATA_OT_ID);
    r->id = id;
    r->gauge = data->gauge;
    r->dp = data->gauge * 100;   // (closed: the gauge full)
    MPNet_Log("shadow: Shadow %u (species %u) made, gauge %u", id, data->species, data->gauge);
    return TRUE;
}

void Orre_Shadow_Purified(struct Pokemon *mon)
{
    struct OrreShadowRec *r = RecOf(mon);
    if (r == NULL)
        return;
    MPNet_Log("shadow: Shadow %u purified", r->id);
    *r = sRecs[--sRecCount];
}

// ---- the heart gauge

static u8 LevelOf(const struct OrreShadowRec *r)
{
    float level;
    if (r == NULL)
        return ORRE_SHADOW_NONE;
    if (r->dp <= 0)
        return ORRE_SHADOW_OPEN;
    level = 100.0f * (r->dp / 100.0f) / (r->gauge ? r->gauge : 1);
    if (level >= 100.0f)
        return 0;
    if (level >= 80.0f)
        return 1;
    if (level >= 60.0f)
        return 2;
    if (level >= 40.0f)
        return 3;
    if (level >= 20.0f)
        return 4;
    return 5;
}

u8 Orre_Shadow_Level(struct Pokemon *mon)
{
    return LevelOf(RecOf(mon));
}

s32 Orre_Shadow_Dp(struct Pokemon *mon)
{
    struct OrreShadowRec *r = RecOf(mon);
    return r ? r->dp : -1;
}

u8 Orre_Shadow_Gauge(struct Pokemon *mon)
{
    struct OrreShadowRec *r = RecOf(mon);
    return r ? r->gauge : 0;
}

static void DropRec(struct OrreShadowRec *r, u8 nature, u8 source, u8 strength)
{
    float dp, now;
    const u8 *rate;
    if (r == NULL || source >= ARRAY_COUNT(sOrreDpSource) || nature >= ARRAY_COUNT(sOrreShadowNatures))
        return;
    if (source == ORRE_DP_EMPTY)
    {
        r->dp = 0;
        return;
    }
    dp = (float)sOrreDpSource[source];
    if (source == ORRE_DP_SCENT)
        dp *= (float)strength;
    rate = sOrreDpRate[sOrreShadowNatures[nature].rate[source] < ARRAY_COUNT(sOrreDpRate) ? sOrreShadowNatures[nature].rate[source] : 1];
    if (rate[1] == 0)
        return;
    dp = dp * (float)rate[0] / (float)rate[1];
    now = r->dp / 100.0f + dp;
    if (now < 0.0f)
        now = 0.0f;
    r->dp = (s32)(100.0f * now);
}

void Orre_Shadow_Drop(struct Pokemon *mon, u8 source, u8 strength)
{
    struct OrreShadowRec *r = RecOf(mon);
    if (r)
        DropRec(r, GetNature(mon), source, strength);
}

u8 Orre_Shadow_HyperChance(struct Pokemon *mon)
{
    u8 level = Orre_Shadow_Level(mon);
    return level <= ORRE_SHADOW_OPEN ? sOrreShadowNatures[GetNature(mon)].hyper[level] : 0;
}

u8 Orre_Shadow_CallChance(struct Pokemon *mon)
{
    u8 level = Orre_Shadow_Level(mon);
    return level <= ORRE_SHADOW_OPEN ? sOrreShadowNatures[GetNature(mon)].call[level] : 0;
}

// a step: field_exact_801ED3B8.c's counts (the party's walking, the day care's)
static void StepRec(struct OrreShadowRec *r, u8 nature, u8 source)
{
    if (r == NULL || r->dp <= 0)
        return;
    if (r->steps >= STEPS_PER_DROP)
    {
        DropRec(r, nature, source, 0);
        r->steps = 0;
    }
    else
        r->steps++;
}

void Orre_Shadow_Step(void)
{
    u32 i;
    if (sRecCount == 0)
        return;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG))
            StepRec(RecOf(mon), GetNature(mon), ORRE_DP_WALK);
    }
    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        struct BoxPokemon *box = &gSaveBlock1Ptr->daycare.mons[i].mon;
        struct OrreShadowRec *r = RecOfBox(box);
        if (r)
            StepRec(r, GetNatureFromPersonality(r->personality), ORRE_DP_DAYCARE);
    }
}

// ---- "<save>.orre": {magic, format, story flag bytes, records}

static void Path(char *out, size_t size, int backup)
{
    if (backup)
        snprintf(out, size, "%s.bak%d.orre", gPlatformSavePath, backup);
    else
        snprintf(out, size, "%s.orre", gPlatformSavePath);
}

void Orre_Shadow_Reset(void)
{
    sRecCount = 0;
    memset(sRecs, 0, sizeof(sRecs));
    Orre_StoryReset();
}

static bool8 LoadFrom(const char *path)
{
    FILE *f = fopen(path, "rb");
    u32 head[4], size;
    u8 *flags = Orre_StoryFlagBytes(&size);
    bool8 ok;
    if (f == NULL)
        return FALSE;
    ok = fread(head, sizeof(head), 1, f) == 1 && head[0] == ORRE_MAGIC && head[1] == ORRE_FORMAT
      && head[2] == size && head[3] <= MAX_RECS;
    if (ok)
        ok = fread(flags, size, 1, f) == 1 && (head[3] == 0 || fread(sRecs, sizeof(sRecs[0]), head[3], f) == head[3]);
    fclose(f);
    if (!ok)
    {
        Orre_Shadow_Reset();
        return FALSE;
    }
    sRecCount = head[3];
    return TRUE;
}

void Orre_Shadow_LoadFromSave(void)
{
    char path[600];
    int i;
    Orre_Shadow_Reset();
    for (i = 0; i <= 3; i++)
    {
        Path(path, sizeof(path), i);
        if (LoadFrom(path))
        {
            MPNet_Log("orre: %u Shadow Pokemon%s", sRecCount, i ? " (from a backup)" : "");
            return;
        }
    }
}

void Orre_Shadow_WriteSidecar(void)
{
    char path[600];
    AwFile *f;
    u32 head[4], size;
    u8 *flags = Orre_StoryFlagBytes(&size);
    if (gSaveBlock2Ptr == NULL || MOEMON_DEV_BUILD)   // (a dev build never writes a save or its side files)
        return;
    Path(path, sizeof(path), 0);
    if ((f = AwFile_Open(path)) == NULL)
        return;
    head[0] = ORRE_MAGIC;
    head[1] = ORRE_FORMAT;
    head[2] = size;
    head[3] = sRecCount;
    AwFile_Write(head, sizeof(head), 1, f);
    AwFile_Write(flags, size, 1, f);
    if (sRecCount)
        AwFile_Write(sRecs, sizeof(sRecs[0]), sRecCount, f);
    AwFile_Close(f);
}

// ---- tests: `mp shadow ...`

void Orre_Shadow_Debug(const char *arg)
{
    int slot = 0, a = 0, b = 0;
    char cmd[16] = "";
    struct Pokemon *mon;
    sscanf(arg, "%15s %d %d %d", cmd, &slot, &a, &b);
    mon = slot >= 0 && slot < PARTY_SIZE ? &gPlayerParty[slot] : NULL;
    if (strcmp(cmd, "make") == 0)   // make SLOT ID: the party's SLOT made Colosseum's Shadow ID
        printf("[test] shadow make %d %d: %d\n", slot, a, Orre_Shadow_Make(mon, a));
    else if (strcmp(cmd, "drop") == 0)   // drop SLOT SOURCE [STRENGTH]
        Orre_Shadow_Drop(mon, a, b);
    else if (strcmp(cmd, "steps") == 0)   // steps N: N steps taken
        for (a = 0; a < slot; a++)
            Orre_Shadow_Step();
    else if (strcmp(cmd, "purify") == 0)
        Orre_Shadow_Purified(mon);
    else if (strcmp(cmd, "reload") == 0)   // reload: "<save>.orre" read back (after a save)
    {
        void AwFile_Flush(void);
        AwFile_Flush();
        Orre_Shadow_LoadFromSave();
    }
    else if (strcmp(cmd, "stepcount") == 0)   // stepcount SLOT: the steps toward its next walking drop
    {
        struct OrreShadowRec *r = RecOf(mon);
        printf("[test] shadow steps %d: %d\n", slot, r ? r->steps : -1);
        return;
    }
    else if (strcmp(cmd, "table") == 0)
    {
        u32 i;
        for (i = 0; i < ARRAY_COUNT(sOrreShadows); i++)
            printf("[test] shadow table %u species %u level %u gauge %u catch %u\n", sOrreShadows[i].id,
                   sOrreShadows[i].species, sOrreShadows[i].level, sOrreShadows[i].gauge, sOrreShadows[i].catchRate);
        return;
    }
    if (strcmp(cmd, "table") != 0 && mon)   // and every command: the slot's state
        printf("[test] shadow %d: id %u dp %d gauge %u level %u hyper %u call %u nature %u\n", slot,
               Orre_Shadow_Id(mon), Orre_Shadow_Dp(mon), Orre_Shadow_Gauge(mon), Orre_Shadow_Level(mon),
               Orre_Shadow_HyperChance(mon), Orre_Shadow_CallChance(mon), GetNature(mon));
}
