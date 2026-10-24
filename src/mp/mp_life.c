// Moemon Co-op: MOéMON GARDEN, the A-Life core (docs/POKEMON_GARDEN.md).
//
// The rules everything here follows:
//   Needs create behavior. Experiences create emotions. Relationships create attachment.
//   Player neglect creates nothing, because ordinary absence isn't neglect.
// So: nothing here ever changes while a Pokémon rests in the PC or the game isn't played; needs only
// build up as preferences (they never turn into distress); feelings come from things that happened;
// Friendship (Emerald's) only rises from a good memory that involves the owner (a "moment"), once per
// memory, in the same steps as Emerald's vitamins, and the A-Life never lowers it.
//
// A record per Pokémon (personality + OT ID), created the first time it's in the party; bonds in a list
// of their own (one direction each: how A feels about B). Both are written to "<save>.life" right after
// the save file (sdl2.c StoreSaveFile), with the save's counter, and read back after a save loads; the
// save's backups keep theirs as "<save>.bakN.life".
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_life.h"
#include "daycare.h"
#include "event_data.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "string_util.h"
#include "tv.h"
#include "mp_life_emote.h"
#include "mp_life_battle.h"
#include "mp_life_social.h"
#include "mp_garden.h"
#include "constants/berry.h"
#include "constants/characters.h"
#include "constants/flags.h"
#include "constants/pokemon.h"
#include "constants/region_map_sections.h"
#include "constants/species.h"

extern char gPlatformSavePath[512];
extern u32 gSaveCounter;

#define LIFE_MAGIC     0x4546494C   // "LIFE"
#define LIFE_VERSION   2            // 2: longer names, name size and the last battle line in the header
#define LIFE_MAX_RECS  640          // party + every box + room to spare
#define LIFE_MAX_BONDS 4096
#define LIFE_MAX_NAMES 256
#define LIFE_NAME_LEN  24           // "MAY's MARSHTOMP", "ALICE's SKITTY"

struct MpLifeName
{
    u32 key;
    u8 name[LIFE_NAME_LEN];
};

static struct MpLifeRec sRecs[LIFE_MAX_RECS];
static u16 sRecCount;
static struct MpLifeBond sBonds[LIFE_MAX_BONDS];
static u16 sBondCount;
static struct MpLifeName sNames[LIFE_MAX_NAMES];
static u16 sNameCount;
static u8 sNameNext;
static bool8 sLoaded;
static u32 sStoryMask, sBadgeMask;
static bool8 sStoryKnown;   // the masks were read (or set from the save's flags)
static struct MpLifeNews sNews[LIFE_NEWS];
static u32 sLastLineMin;   // the last rare battle line (play minutes; mp_life_battle.c)
static struct MpGardenSave sGarden;

struct MpGardenSave *MP_Life_GardenSave(void)
{
    return &sGarden;
}
static u8 sNewsCount;
static u32 sFrame;          // frames on the map since the last emotion step
static u32 sLastMinute;     // play minute of the last minute tick
static u16 sLastPlace = 0xFFFF;

// ---------------------------------------------------------------------------
// Keys

static u32 Fnv(u32 h, const void *data, u32 size)
{
    const u8 *b = data;
    u32 i;
    for (i = 0; i < size; i++)
        h = (h ^ b[i]) * 16777619u;
    return h;
}

// Pokémon keys never have the top bit set; Trainer keys always do
u32 MP_Life_MonKey(u32 personality, u32 otId)
{
    u32 h = Fnv(2166136261u, &personality, 4);
    h = Fnv(h, &otId, 4) & 0x7FFFFFFF;
    return h ? h : 1;
}

u32 MP_Life_BoxKey(struct BoxPokemon *box)
{
    return MP_Life_MonKey(GetBoxMonData(box, MON_DATA_PERSONALITY), GetBoxMonData(box, MON_DATA_OT_ID));
}

u32 MP_Life_TrainerKey(u32 otId, const u8 *name)
{
    u32 h = Fnv(2166136261u, &otId, 4);
    u8 i;
    for (i = 0; i < PLAYER_NAME_LENGTH && name[i] != EOS; i++)
        h = Fnv(h, &name[i], 1);
    return h | 0x80000000;
}

u32 MP_Life_LocalTrainerKey(void)
{
    u32 otId = GetPlayerIDAsU32();
    return MP_Life_TrainerKey(otId, gSaveBlock2Ptr->playerName);
}

bool8 MP_Life_IsTrainerKey(u32 key)
{
    return (key & 0x80000000) != 0;
}

// ---------------------------------------------------------------------------
// Time and progress

u32 MP_Life_PlayMinutes(void)
{
    return (u32)gSaveBlock2Ptr->playTimeHours * 60 + gSaveBlock2Ptr->playTimeMinutes;
}

// How far the adventure has come: badges, then hours played (reunions, docs 5.3)
u16 MP_Life_ProgressMark(void)
{
    u16 badges = 0, i;
    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
        if (FlagGet(i))
            badges++;
    return badges * 1000 + (gSaveBlock2Ptr->playTimeHours > 999 ? 999 : gSaveBlock2Ptr->playTimeHours);
}

u16 MP_Life_CurrentPlace(void)
{
    return gMapHeader.regionMapSectionId;
}

// ---------------------------------------------------------------------------
// A small seeded random generator, so a Pokémon's temperament is the same whenever it's rolled

static u32 sSeed;
static u32 Roll(void)
{
    sSeed = sSeed * 1103515245 + 12345;
    return (sSeed >> 8) & 0xFFFF;
}

static u8 Clamp8(s32 v)
{
    return v < 0 ? 0 : v > 255 ? 255 : v;
}

static u8 Clamp100(s32 v)
{
    return v < 0 ? 0 : v > 100 ? 100 : v;
}

#include "../data/mp_life_instincts.h"

static u32 Instincts(u16 species)
{
    u16 dex = SpeciesToNationalPokedexNum(species);
    return dex <= NATIONAL_DEX_COUNT ? sMpLifeInstincts[dex] : 0;
}

// The pursuit that would suit it (docs 12: one that fits its interests), from its current species'
// instincts and type and its own temperament; when several fit, its own pick (from its personality)
u8 MP_Life_PursuitFor(struct MpLifeRec *rec)
{
    u32 inst = Instincts(rec->species);
    u8 c[LIFE_PURSUIT_COUNT], n = 0;
    u8 ta = MP_Life_TypeNeed(rec, FALSE), tb = MP_Life_TypeNeed(rec, TRUE);
#define TN(t) (ta == (t) || tb == (t))
    if ((inst & (INST_TREE | INST_SKY)) || TN(TNEED_HEIGHT) || TN(TNEED_SKY))
        c[n++] = LIFE_PURSUIT_CLIMB;
    if ((inst & INST_SEA) || TN(TNEED_SOAK))
        c[n++] = LIFE_PURSUIT_SWIM;
    if (inst & INST_SING)
        c[n++] = LIFE_PURSUIT_SING;
    if ((inst & INST_COLLECT) || TN(TNEED_SHINE) || TN(TNEED_STONE))
        c[n++] = LIFE_PURSUIT_COLLECT;
    if ((inst & INST_DIG) || TN(TNEED_DIG))
        c[n++] = LIFE_PURSUIT_DIG;
    if (((inst & INST_GENTLE) || rec->cur[LIFE_WARM] >= 170) && rec->cur[LIFE_SOCIAL] >= 110)
        c[n++] = LIFE_PURSUIT_TEACH;
#undef TN
    return n ? c[(rec->personality >> 4) % n] : LIFE_PURSUIT_NONE;
}

static bool8 HasEggGroup(u16 species, u8 group)
{
    return gSpeciesInfo[species].eggGroups[0] == group || gSpeciesInfo[species].eggGroups[1] == group;
}

static const u8 sTypeNeeds[NUMBER_OF_MON_TYPES] = {
    [TYPE_NORMAL] = TNEED_COZY,   [TYPE_FIGHTING] = TNEED_SPAR, [TYPE_FLYING] = TNEED_SKY,
    [TYPE_POISON] = TNEED_DAMP,   [TYPE_GROUND] = TNEED_DIG,    [TYPE_ROCK] = TNEED_STONE,
    [TYPE_BUG] = TNEED_FOLIAGE,   [TYPE_GHOST] = TNEED_SHADE,   [TYPE_STEEL] = TNEED_SHINE,
    [TYPE_FIRE] = TNEED_BASK,     [TYPE_WATER] = TNEED_SOAK,    [TYPE_GRASS] = TNEED_SUN,
    [TYPE_ELECTRIC] = TNEED_STORM, [TYPE_PSYCHIC] = TNEED_STILL, [TYPE_ICE] = TNEED_COOL,
    [TYPE_DRAGON] = TNEED_HEIGHT, [TYPE_DARK] = TNEED_NIGHT,
};

u8 MP_Life_TypeNeed(struct MpLifeRec *rec, bool8 second)
{
    u8 t = gSpeciesInfo[rec->species].types[second ? 1 : 0];
    return t < NUMBER_OF_MON_TYPES ? sTypeNeeds[t] : TNEED_NONE;
}

// The temperament it's born with: an individual roll, loosely nudged by its Nature and its species
// (never decided by either)
static void RollTemperament(struct MpLifeRec *rec, struct BoxPokemon *box)
{
    u8 nature = GetNatureFromPersonality(rec->personality);
    u16 species = rec->species;
    u32 inst = Instincts(species);
    const struct SpeciesInfo *si = &gSpeciesInfo[species];
    s32 v[LIFE_AXES];
    u8 i, f;

    sSeed = rec->personality ^ (rec->otId * 2654435761u);
    for (i = 0; i < LIFE_AXES; i++)
        v[i] = 32 + (Roll() % 97) + (Roll() % 97);   // 32..224, mostly near the middle

    // Nature: a nudge toward its raised stat's mood, away from its lowered one
    for (f = 0; f < NUM_NATURE_STATS; f++)
    {
        s8 m = gNatureStatTable[nature][f] * 18;
        switch (f)
        {
        case 0: v[LIFE_BOLD] += m; break;      // Attack
        case 1: v[LIFE_PROUD] += m / 2; v[LIFE_BOLD] += m / 2; break;   // Defense: steady
        case 2: v[LIFE_LIVELY] += m; break;    // Speed
        case 3: v[LIFE_CURIOUS] += m; break;   // Sp. Atk
        case 4: v[LIFE_WARM] += m; break;      // Sp. Def
        }
    }
    // Species: its instincts and its kind
    if (inst & INST_SHY)     v[LIFE_BOLD] -= 20;
    if (inst & INST_FIERCE)  v[LIFE_BOLD] += 20, v[LIFE_WARM] -= 10;
    if (inst & INST_PLAY)    v[LIFE_LIVELY] += 15;
    if (inst & INST_SLEEP)   v[LIFE_LIVELY] -= 15;
    if (inst & INST_CURIOUS) v[LIFE_CURIOUS] += 20;
    if (inst & INST_PACK)    v[LIFE_SOCIAL] += 20;
    if (inst & INST_LONER)   v[LIFE_SOCIAL] -= 30;
    if (inst & INST_GENTLE)  v[LIFE_WARM] += 15;
    if (inst & INST_LOYAL)   v[LIFE_WARM] += 15;
    if (inst & INST_PROUD)   v[LIFE_PROUD] += 25;
    if (HasEggGroup(species, EGG_GROUP_DRAGON)) v[LIFE_PROUD] += 15;
    if (HasEggGroup(species, EGG_GROUP_FAIRY))  v[LIFE_WARM] += 10, v[LIFE_LIVELY] += 5;
    if (HasEggGroup(species, EGG_GROUP_HUMAN_LIKE)) v[LIFE_SOCIAL] += 10;
    if (HasEggGroup(species, EGG_GROUP_MINERAL)) v[LIFE_LIVELY] -= 15;
    if (si->baseSpeed >= 100) v[LIFE_LIVELY] += 10;
    if (si->baseSpeed <= 40)  v[LIFE_LIVELY] -= 10;
    for (i = 0; i < LIFE_AXES; i++)
        rec->base[i] = rec->cur[i] = Clamp8(v[i]);

    // Tastes: Emerald's flavors (the raised stat's flavor is liked, the lowered one's disliked),
    // then a little of its own
    {
        static const u8 sStatFlavor[NUM_NATURE_STATS] = {FLAVOR_SPICY, FLAVOR_SOUR, FLAVOR_SWEET, FLAVOR_DRY, FLAVOR_BITTER};
        for (f = 0; f < FLAVOR_COUNT; f++)
            rec->taste[f] = (s8)((s32)(Roll() % 41) - 20);
        for (f = 0; f < NUM_NATURE_STATS; f++)
        {
            if (gNatureStatTable[nature][f] > 0)
                rec->taste[sStatFlavor[f]] = 60 + Roll() % 30;
            else if (gNatureStatTable[nature][f] < 0)
                rec->taste[sStatFlavor[f]] = -60 - (s8)(Roll() % 30);
        }
    }
    // Which of two type needs leads (dual types)
    rec->typeLean = Roll() & 0xFF;
    // Boundaries: what it doesn't care for, from its temperament
    rec->boundaries = 0;
    if (rec->base[LIFE_PROUD] > 175)
        rec->boundaries |= 1 << LIFE_VERB_PET;       // not petted in front of others
    if (rec->base[LIFE_BOLD] < 80)
        rec->boundaries |= 1 << LIFE_VERB_PICKUP;    // shy ones dislike being picked up
    if (rec->base[LIFE_WARM] < 70 && rec->base[LIFE_SOCIAL] < 100)
        rec->boundaries |= 1 << LIFE_VERB_GROOM;     // aloof loners dislike a fuss
    // Needs start a little varied
    for (i = 0; i < LIFE_NEEDS; i++)
        rec->need[i] = 40 + Roll() % 80;
}

// ---------------------------------------------------------------------------
// Records

struct MpLifeRec *MP_Life_FindKey(u32 key)
{
    u16 i;
    for (i = 0; i < sRecCount; i++)
        if (MP_Life_MonKey(sRecs[i].personality, sRecs[i].otId) == key)
            return &sRecs[i];
    return NULL;
}

static struct MpLifeRec *FindRec(u32 personality, u32 otId)
{
    u16 i;
    for (i = 0; i < sRecCount; i++)
        if (sRecs[i].personality == personality && sRecs[i].otId == otId)
            return &sRecs[i];
    return NULL;
}

struct BoxPokemon *MP_Life_PartyBox(u32 key)
{
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct BoxPokemon *box = &gPlayerParty[i].box;
        if (GetBoxMonData(box, MON_DATA_SPECIES) != SPECIES_NONE && !GetBoxMonData(box, MON_DATA_IS_EGG)
         && MP_Life_BoxKey(box) == key)
            return box;
    }
    return NULL;
}

// The owned Pokémon with this key (party, then the player's own boxes)
static struct BoxPokemon *FindOwnedBox(u32 key)
{
    struct BoxPokemon *box = MP_Life_PartyBox(key);
    struct PokemonStorage *st = MP_PersonalStorage();
    u16 b, s;
    if (box == NULL)
    {
        // an EGG in the party (its record waits for the hatch)
        for (b = 0; b < PARTY_SIZE; b++)
            if (GetBoxMonData(&gPlayerParty[b].box, MON_DATA_IS_EGG) && MP_Life_BoxKey(&gPlayerParty[b].box) == key)
                return &gPlayerParty[b].box;
    }
    if (box || st == NULL)
        return box;
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
        for (s = 0; s < IN_BOX_COUNT; s++)
        {
            box = &st->boxes[b][s];
            if (GetBoxMonData(box, MON_DATA_SPECIES) != SPECIES_NONE && MP_Life_BoxKey(box) == key)
                return box;
        }
    return NULL;
}

static void DropRecord(u16 index);

// A slot for a new record; when full, the record of a Pokémon no longer owned goes first
static struct MpLifeRec *NewRecSlot(void)
{
    u16 i;
    if (sRecCount < LIFE_MAX_RECS)
        return &sRecs[sRecCount++];
    for (i = 0; i < sRecCount; i++)
        if (FindOwnedBox(MP_Life_MonKey(sRecs[i].personality, sRecs[i].otId)) == NULL)
        {
            DropRecord(i);
            return &sRecs[sRecCount++];
        }
    return NULL;
}

static void InitialOwnerBond(struct MpLifeRec *rec, struct BoxPokemon *box);
static void RememberHowWeMet(struct MpLifeRec *rec, struct BoxPokemon *box);
static void EvolutionCheck(struct MpLifeRec *rec, struct BoxPokemon *box);
static void FinishEgg(struct MpLifeRec *rec, struct BoxPokemon *box);

static bool8 InParty(struct BoxPokemon *box)
{
    return box >= &gPlayerParty[0].box && box <= &gPlayerParty[PARTY_SIZE - 1].box
        && ((u8 *)box - (u8 *)&gPlayerParty[0]) % sizeof(struct Pokemon) == 0;
}

struct MpLifeRec *MP_Life_Get(struct BoxPokemon *box, bool8 create)
{
    u32 personality, otId;
    struct MpLifeRec *rec;
    u16 species;

    if (box == NULL || (species = GetBoxMonData(box, MON_DATA_SPECIES)) == SPECIES_NONE || GetBoxMonData(box, MON_DATA_IS_EGG))
        return NULL;
    personality = GetBoxMonData(box, MON_DATA_PERSONALITY);
    otId = GetBoxMonData(box, MON_DATA_OT_ID);
    rec = FindRec(personality, otId);
    if (rec && (rec->flags & LIFE_REC_EGG))
        FinishEgg(rec, box);   // it hatched
    if (rec && species != rec->species && InParty(box))
        EvolutionCheck(rec, box);   // it just evolved (noticed right away, not at the next minute)
    if (rec || !create)
        return rec;
    rec = NewRecSlot();
    if (rec == NULL)
        return NULL;
    memset(rec, 0, sizeof(*rec));
    rec->personality = personality;
    rec->otId = otId;
    rec->species = species;
    rec->version = LIFE_VERSION;
    rec->createdMin = rec->lastTickMin = MP_Life_PlayMinutes();
    rec->lastPartyMark = MP_Life_ProgressMark();
    rec->emo[EMO_CALM] = 40;
    RollTemperament(rec, box);
    InitialOwnerBond(rec, box);
    RememberHowWeMet(rec, box);
    {
        u8 name[POKEMON_NAME_LENGTH + 1];
        GetBoxMonData(box, MON_DATA_NICKNAME, name);
        name[POKEMON_NAME_LENGTH] = EOS;
        MP_Life_NoteName(MP_Life_MonKey(personality, otId), name);
    }
    MPNet_Log("life: new record for species %u (%u records)", species, sRecCount);
    // it may have lived somewhere else first (a trade, the shared PC): a friend's game may know its story
    MP_LifeSocial_AskRecord(MP_Life_MonKey(personality, otId));
    return rec;
}

// ---------------------------------------------------------------------------
// Records travel with their Pokémon (trades, the shared PC: docs 10)

#define PACK_BONDS 24

// [u16 recSize][u16 bondSize][u8 bonds][u8 nameLen] record, bonds, then a name (LIFE_NAME_LEN) per bond
u16 MP_Life_PackRecord(u32 key, u8 *out, u16 max)
{
    struct MpLifeRec *rec = MP_Life_FindKey(key);
    u16 pos = 6, i;
    u8 n = 0;
    if (rec == NULL || max < 6 + sizeof(*rec))
        return 0;
    out[0] = sizeof(*rec) & 0xFF; out[1] = sizeof(*rec) >> 8;
    out[2] = sizeof(struct MpLifeBond) & 0xFF; out[3] = sizeof(struct MpLifeBond) >> 8;
    out[5] = LIFE_NAME_LEN;
    memcpy(out + pos, rec, sizeof(*rec));
    pos += sizeof(*rec);
    for (i = 0; i < sBondCount && n < PACK_BONDS; i++)
    {
        u8 name[LIFE_NAME_LEN];
        if (sBonds[i].from != key)
            continue;
        if (pos + sizeof(struct MpLifeBond) + LIFE_NAME_LEN > max)
            break;
        memcpy(out + pos, &sBonds[i], sizeof(struct MpLifeBond));
        pos += sizeof(struct MpLifeBond);
        memset(name, 0, sizeof(name));
        if (!MP_Life_NameOf(sBonds[i].to, name))
            name[0] = EOS;
        name[LIFE_NAME_LEN - 1] = EOS;
        memcpy(out + pos, name, LIFE_NAME_LEN);
        pos += LIFE_NAME_LEN;
        n++;
    }
    out[4] = n;
    return pos;
}

static u8 MemoryCount(const struct MpLifeRec *rec)
{
    u8 i, n = 0;
    for (i = 0; i < LIFE_MEMORIES; i++)
        n += rec->mem[i].kind != MEM_NONE;
    return n;
}

// A record from a friend's game. A trade's is the latest word on that Pokémon; an answer to our question
// only fills in a record we just made up for it (a Pokémon from the shared PC).
bool8 MP_Life_UnpackRecord(const u8 *in, u16 len, bool8 traded, const u8 *fromTrainerName, u32 fromTrainerKey)
{
    u16 recSize, bondSize, pos = 6, i;
    u8 n, nameLen;
    struct MpLifeRec incoming, *rec;
    u32 key, me = MP_Life_LocalTrainerKey();
    if (!sLoaded || len < 6)
        return FALSE;
    recSize = in[0] | (in[1] << 8);
    bondSize = in[2] | (in[3] << 8);
    n = in[4];
    nameLen = in[5];
    if (recSize == 0 || bondSize == 0 || len < pos + recSize)
        return FALSE;
    memset(&incoming, 0, sizeof(incoming));
    memcpy(&incoming, in + pos, recSize < sizeof(incoming) ? recSize : sizeof(incoming));
    pos += recSize;
    key = MP_Life_MonKey(incoming.personality, incoming.otId);
    rec = MP_Life_FindKey(key);
    if (rec && !traded && (rec->species != incoming.species || !(MemoryCount(rec) <= 1 && rec->createdMin + 30 >= MP_Life_PlayMinutes())))
        return FALSE;   // ours already lived with us (or isn't that Pokémon at all): keep it
    if (rec == NULL && (rec = NewRecSlot()) == NULL)
        return FALSE;
    *rec = incoming;
    rec->version = LIFE_VERSION;
    rec->flags &= ~LIFE_REC_IN_PARTY;
    rec->lastPartyMark = MP_Life_ProgressMark();
    rec->createdMin = rec->lastTickMin = MP_Life_PlayMinutes();
    rec->known[0] = rec->known[1] = 0;   // a new Trainer gets to know it from the start
    // its bonds: with its old Trainer, its old friends; never one with us from the other side
    for (i = 0; i < n && pos + bondSize + nameLen <= len; i++)
    {
        struct MpLifeBond b, *d;
        memset(&b, 0, sizeof(b));
        memcpy(&b, in + pos, bondSize < sizeof(b) ? bondSize : sizeof(b));
        pos += bondSize;
        if (nameLen && in[pos] != EOS)
        {
            u8 name[LIFE_NAME_LEN];
            u8 k;
            for (k = 0; k < LIFE_NAME_LEN - 1 && k < nameLen && in[pos + k] != EOS; k++)
                name[k] = in[pos + k];
            name[k] = EOS;
            if (b.to != me)
                MP_Life_NoteName(b.to, name);
        }
        pos += nameLen;
        if (b.from != key || b.to == 0)
            continue;
        if (b.to == fromTrainerKey && fromTrainerKey)
            b.flags |= LIFE_BOND_TRAINER | LIFE_BOND_MET;   // its first Trainer: greeted warmly ever after
        d = MP_Life_Bond(b.from, b.to, TRUE);
        if (d)
            *d = b;
    }
    if (fromTrainerName && fromTrainerKey)
        MP_Life_NoteName(fromTrainerKey, fromTrainerName);
    // and us, its Trainer now
    if (MP_Life_Bond(key, me, FALSE) == NULL)
    {
        struct MpLifeBond *b = MP_Life_Bond(key, me, TRUE);
        if (b)
        {
            b->flags |= LIFE_BOND_TRAINER | LIFE_BOND_MET;
            b->v[BOND_FAMILIAR] = 10;
        }
    }
    if (traded)
        MP_Life_Remember(rec, MEM_MET, 200, TIER_PART, me, 3);
    MPNet_Log("life: record for species %u from a friend (%s)", rec->species, traded ? "trade" : "shared");
    return TRUE;
}

// A small step in one part of a bond (being around each other), up to a cap; not a shared experience
void MP_Life_BondNudge(u32 from, u32 to, u8 part, u8 amount, u8 cap)
{
    struct MpLifeBond *b = MP_Life_Bond(from, to, TRUE);
    if (b == NULL || part >= BOND_PARTS || b->v[part] >= cap)
        return;
    b->v[part] = b->v[part] + amount > cap ? cap : b->v[part] + amount;
}

static void DropRecord(u16 index)
{
    u32 key = MP_Life_MonKey(sRecs[index].personality, sRecs[index].otId);
    u16 i;
    // its own bonds go with it
    for (i = 0; i < sBondCount;)
    {
        if (sBonds[i].from == key)
            sBonds[i] = sBonds[--sBondCount];
        else
            i++;
    }
    sRecs[index] = sRecs[--sRecCount];
}

// ---------------------------------------------------------------------------
// Names

void MP_Life_NoteName(u32 key, const u8 *name)
{
    u16 i;
    struct MpLifeName *n = NULL;
    for (i = 0; i < sNameCount; i++)
        if (sNames[i].key == key)
            n = &sNames[i];
    if (n == NULL)
    {
        if (sNameCount < LIFE_MAX_NAMES)
            n = &sNames[sNameCount++];
        else
            n = &sNames[sNameNext++ % LIFE_MAX_NAMES];
    }
    n->key = key;
    for (i = 0; i < LIFE_NAME_LEN - 1 && name[i] != EOS; i++)
        n->name[i] = name[i];
    n->name[i] = EOS;
}

bool8 MP_Life_NameOf(u32 key, u8 *out)
{
    u16 i;
    struct BoxPokemon *box;
    if (key == MP_Life_LocalTrainerKey())
    {
        StringCopy(out, gSaveBlock2Ptr->playerName);
        return TRUE;
    }
    if (!MP_Life_IsTrainerKey(key) && (box = FindOwnedBox(key)) != NULL)
    {
        GetBoxMonData(box, MON_DATA_NICKNAME, out);
        out[POKEMON_NAME_LENGTH] = EOS;
        StringGet_Nickname(out);
        return TRUE;
    }
    for (i = 0; i < sNameCount; i++)
        if (sNames[i].key == key)
        {
            StringCopy(out, sNames[i].name);
            return TRUE;
        }
    return FALSE;
}

// ---------------------------------------------------------------------------
// Bonds

struct MpLifeBond *MP_Life_Bond(u32 from, u32 to, bool8 create)
{
    u16 i;
    struct MpLifeBond *b;
    if (from == 0 || to == 0 || from == to)
        return NULL;
    for (i = 0; i < sBondCount; i++)
        if (sBonds[i].from == from && sBonds[i].to == to)
            return &sBonds[i];
    if (!create)
        return NULL;
    if (sBondCount >= LIFE_MAX_BONDS)
    {
        // full: the weakest bond with someone barely known goes
        u16 weakest = 0, w = 0xFFFF;
        for (i = 0; i < sBondCount; i++)
        {
            u16 s = sBonds[i].v[BOND_FAMILIAR] + sBonds[i].shared * 4;
            if (!(sBonds[i].flags & LIFE_BOND_TRAINER) && s < w)
                w = s, weakest = i;
        }
        b = &sBonds[weakest];
    }
    else
    {
        b = &sBonds[sBondCount++];
    }
    memset(b, 0, sizeof(*b));
    b->from = from;
    b->to = to;
    if (MP_Life_IsTrainerKey(to))
        b->flags |= LIFE_BOND_TRAINER;
    // a first meeting: the opening stance comes from its temperament (docs 5.3)
    {
        struct MpLifeRec *rec = MP_Life_FindKey(from);
        if (rec)
        {
            b->v[BOND_TRUST] = 20 + rec->cur[LIFE_BOLD] / 6 + rec->cur[LIFE_WARM] / 8;
            b->v[BOND_AFFECTION] = 10 + rec->cur[LIFE_WARM] / 6;
            b->v[BOND_RESPECT] = 20;
            b->v[BOND_FRICTION] = rec->cur[LIFE_WARM] < 80 ? 10 : 0;
        }
    }
    return b;
}

static u8 AddPart(u8 v, s8 d)
{
    return Clamp8((s32)v + d);
}

void MP_Life_BondShare(u32 from, u32 to, s8 familiar, s8 trust, s8 affection, s8 respect, s8 friction)
{
    struct MpLifeBond *b = MP_Life_Bond(from, to, TRUE);
    if (b == NULL)
        return;
    b->v[BOND_FAMILIAR] = AddPart(b->v[BOND_FAMILIAR], familiar);
    b->v[BOND_TRUST] = AddPart(b->v[BOND_TRUST], trust);
    b->v[BOND_AFFECTION] = AddPart(b->v[BOND_AFFECTION], affection);
    b->v[BOND_RESPECT] = AddPart(b->v[BOND_RESPECT], respect);
    b->v[BOND_FRICTION] = AddPart(b->v[BOND_FRICTION], friction);
    if (b->shared < 0xFFFF)
        b->shared++;
    b->lastMin = MP_Life_PlayMinutes();
}

int MP_Life_BondWarmth(const struct MpLifeBond *b)
{
    if (b == NULL)
        return 0;
    return b->v[BOND_TRUST] + b->v[BOND_AFFECTION] * 2 + b->v[BOND_FAMILIAR] / 2 - b->v[BOND_FRICTION];
}

// Friendship gives the bond with its own Trainer a modest head start (docs 5.4)
static void InitialOwnerBond(struct MpLifeRec *rec, struct BoxPokemon *box)
{
    u32 me = MP_Life_MonKey(rec->personality, rec->otId);
    struct MpLifeBond *b = MP_Life_Bond(me, MP_Life_LocalTrainerKey(), TRUE);
    u8 f = GetBoxMonData(box, MON_DATA_FRIENDSHIP);
    if (b == NULL)
        return;
    b->flags |= LIFE_BOND_TRAINER | LIFE_BOND_MET;
    b->v[BOND_FAMILIAR] = 20 + f / 3;
    b->v[BOND_TRUST] = Clamp8(b->v[BOND_TRUST] + f / 5);
    b->v[BOND_AFFECTION] = Clamp8(b->v[BOND_AFFECTION] + f / 5);
    if (f > 120)
        rec->flags |= LIFE_REC_RAISED_BEFORE;
}

// ---------------------------------------------------------------------------
// Emotions

void MP_Life_Feel(struct MpLifeRec *rec, u8 emotion, u8 amount)
{
    if (rec == NULL || emotion >= LIFE_EMOTIONS)
        return;
    rec->emo[emotion] = Clamp100(rec->emo[emotion] + amount);
    // strong feelings push calm aside (and calm softens the rest)
    if (emotion == EMO_CALM)
    {
        rec->emo[EMO_FEAR] = Clamp100(rec->emo[EMO_FEAR] - amount / 2);
        rec->emo[EMO_FRUSTRATION] = Clamp100(rec->emo[EMO_FRUSTRATION] - amount / 2);
    }
    else if (emotion == EMO_FEAR || emotion == EMO_FRUSTRATION || emotion == EMO_SURPRISE)
    {
        rec->emo[EMO_CALM] = Clamp100(rec->emo[EMO_CALM] - amount / 2);
    }
}

u8 MP_Life_TopEmotion(struct MpLifeRec *rec, u8 *amount)
{
    u8 i, best = EMO_CALM, v = 0;
    for (i = 0; i < LIFE_EMOTIONS; i++)
        if (rec->emo[i] > v)
            v = rec->emo[i], best = i;
    if (amount)
        *amount = v;
    return best;
}

static bool8 IsPositive(u8 emotion)
{
    return emotion == EMO_JOY || emotion == EMO_AFFECTION || emotion == EMO_PRIDE || emotion == EMO_CALM;
}

// ---------------------------------------------------------------------------
// Traits

struct MpLifeTrait *MP_Life_FindTrait(struct MpLifeRec *rec, u8 id, u16 param)
{
    u8 i;
    for (i = 0; i < LIFE_TRAITS; i++)
        if (rec->trait[i].id == id && rec->trait[i].param == param && rec->trait[i].strength)
            return &rec->trait[i];
    return NULL;
}

void MP_Life_AddTrait(struct MpLifeRec *rec, u8 id, u16 param, u8 strength)
{
    struct MpLifeTrait *t = MP_Life_FindTrait(rec, id, param);
    u8 i, weakest = 0;
    if (t)
    {
        t->strength = Clamp8(t->strength + strength);
        return;
    }
    for (i = 0; i < LIFE_TRAITS; i++)
    {
        if (rec->trait[i].strength == 0)
        {
            weakest = i;
            break;
        }
        if (rec->trait[i].strength < rec->trait[weakest].strength)
            weakest = i;
    }
    if (rec->trait[weakest].strength && rec->trait[weakest].strength > strength)
        return;   // its other traits run deeper
    rec->trait[weakest].id = id;
    rec->trait[weakest].param = param;
    rec->trait[weakest].strength = strength;
    rec->known[0] &= ~(1u << (12 + weakest));   // a new trait: not known to the Trainer yet
}

// A fear or dislike softens (docs 4.3: mostly from experiences that prove it wrong)
void MP_Life_SoftenTrait(struct MpLifeRec *rec, u8 id, u16 param, u8 amount)
{
    struct MpLifeTrait *t = MP_Life_FindTrait(rec, id, param);
    if (t == NULL)
        return;
    if (t->strength <= amount)
    {
        bool8 deep = t->strength >= 24 || amount >= 24;
        t->strength = 0;
        t->id = TRAIT_NONE;
        if (id == TRAIT_AFRAID && deep)
        {
            MP_Life_AddTrait(rec, TRAIT_BRAVE, param, 120);
            MP_Life_Remember(rec, MEM_OVERCAME, 150, TIER_PART, 0, param);
        }
    }
    else
    {
        t->strength -= amount;
    }
}

// ---------------------------------------------------------------------------
// Needs

void MP_Life_Satisfy(struct MpLifeRec *rec, u8 need, u8 amount)
{
    if (rec && need < LIFE_NEEDS)
        rec->need[need] = rec->need[need] > amount ? rec->need[need] - amount : 0;
}

u8 MP_Life_PressingNeed(struct MpLifeRec *rec, u8 *amount)
{
    u8 i, best = NEED_REST, v = 0;
    for (i = 0; i < LIFE_NEEDS; i++)
    {
        u8 n = rec->need[i];
        if (i == NEED_TYPE_A && MP_Life_TypeNeed(rec, TRUE) != MP_Life_TypeNeed(rec, FALSE))
            n = n * (128 + rec->typeLean / 2) / 255;   // dual types: the temperament weights which leads
        if (i == NEED_TYPE_B)
        {
            if (MP_Life_TypeNeed(rec, TRUE) == MP_Life_TypeNeed(rec, FALSE))
                continue;
            n = n * (255 - rec->typeLean / 2) / 255;
        }
        if (n > v)
            v = n, best = i;
    }
    if (amount)
        *amount = v;
    return best;
}

// ---------------------------------------------------------------------------
// Places

struct MpLifePlace *MP_Life_Place(struct MpLifeRec *rec, u16 mapsec, bool8 create)
{
    u8 i, weakest = 0;
    if (mapsec >= MAPSEC_NONE)
        return NULL;
    for (i = 0; i < LIFE_PLACES; i++)
        if (rec->place[i].level && rec->place[i].map == mapsec)
            return &rec->place[i];
    if (!create)
        return NULL;
    for (i = 0; i < LIFE_PLACES; i++)
    {
        if (rec->place[i].level == 0)
        {
            weakest = i;
            break;
        }
        // places with personal meaning stay
        if (!(rec->place[i].flags & (LIFE_PLACE_MET | LIFE_PLACE_EVOLVED | LIFE_PLACE_FAVORITE | LIFE_PLACE_HOME))
         && (rec->place[weakest].flags || rec->place[i].level < rec->place[weakest].level))
            weakest = i;
    }
    memset(&rec->place[weakest], 0, sizeof(rec->place[weakest]));
    rec->place[weakest].map = mapsec;
    rec->place[weakest].level = 1;
    return &rec->place[weakest];
}

// ---------------------------------------------------------------------------
// Memories

// The Emerald step a moment adds to Friendship (the vitamins' scale)
static u8 FriendshipStep(u8 friendship)
{
    if (friendship < 100)
        return 5;
    if (friendship < 200)
        return 3;
    return 2;
}

static bool8 IsMomentKind(u8 kind)
{
    switch (kind)
    {
    case MEM_EVOLVED: case MEM_BADGE: case MEM_STORY: case MEM_NEAR_FAINT: case MEM_CLUTCH: case MEM_COMEBACK:
    case MEM_COMFORTED: case MEM_FED: case MEM_PLAYED: case MEM_PRAISED: case MEM_REUNION: case MEM_KEEPSAKE:
    case MEM_OVERCAME: case MEM_PURSUIT: case MEM_GARDEN: case MEM_NEW_PLACE: case MEM_WEATHER: case MEM_LEGEND:
        return TRUE;
    }
    return FALSE;
}

static const u8 sMemFeeling[MEM_KINDS] = {
    [MEM_MET] = EMO_SURPRISE, [MEM_EVOLVED] = EMO_PRIDE, [MEM_BADGE] = EMO_PRIDE, [MEM_STORY] = EMO_SURPRISE,
    [MEM_LEGEND] = EMO_SURPRISE, [MEM_NEW_PLACE] = EMO_JOY, [MEM_WEATHER] = EMO_SURPRISE, [MEM_FRIGHT] = EMO_FEAR,
    [MEM_OVERCAME] = EMO_PRIDE, [MEM_NEAR_FAINT] = EMO_PRIDE, [MEM_FAINTED] = EMO_SADNESS, [MEM_CLUTCH] = EMO_PRIDE,
    [MEM_COMEBACK] = EMO_JOY, [MEM_PARTNER] = EMO_AFFECTION, [MEM_RIVAL] = EMO_PRIDE, [MEM_COMFORTED] = EMO_AFFECTION,
    [MEM_FED] = EMO_JOY, [MEM_PLAYED] = EMO_JOY, [MEM_PRAISED] = EMO_PRIDE, [MEM_REUNION] = EMO_JOY,
    [MEM_KEEPSAKE] = EMO_AFFECTION, [MEM_HEARD] = EMO_SURPRISE, [MEM_RARE] = EMO_JOY, [MEM_PURSUIT] = EMO_PRIDE,
    [MEM_FRIEND] = EMO_AFFECTION, [MEM_FAMILY] = EMO_AFFECTION, [MEM_GARDEN] = EMO_JOY,
};

// How the temperament drifts after a strong memory (docs 4.4: slowly, within a range of its innate self)
static void Drift(struct MpLifeRec *rec, u8 axis, s8 d)
{
    s32 v = rec->cur[axis] + d, lo = rec->base[axis] - 40, hi = rec->base[axis] + 40;
    if (v < lo) v = lo;
    if (v > hi) v = hi;
    rec->cur[axis] = Clamp8(v);
}

static void DriftFrom(struct MpLifeRec *rec, u8 kind, u8 strength)
{
    s8 d = strength >= 200 ? 2 : 1;
    switch (kind)
    {
    case MEM_FRIGHT: Drift(rec, LIFE_BOLD, -d); break;
    case MEM_OVERCAME: case MEM_CLUTCH: case MEM_COMEBACK: Drift(rec, LIFE_BOLD, d); break;
    case MEM_COMFORTED: case MEM_FRIEND: case MEM_FAMILY: case MEM_PARTNER: Drift(rec, LIFE_WARM, d); break;
    case MEM_PRAISED: case MEM_BADGE: Drift(rec, LIFE_PROUD, d); break;
    case MEM_NEW_PLACE: case MEM_LEGEND: case MEM_WEATHER: Drift(rec, LIFE_CURIOUS, d); break;
    case MEM_PLAYED: Drift(rec, LIFE_LIVELY, d); break;
    }
}

static void FriendshipMoment(struct MpLifeRec *rec, struct MpLifeMemory *m)
{
    struct BoxPokemon *box = FindOwnedBox(MP_Life_MonKey(rec->personality, rec->otId));
    u8 f, step;
    if (box == NULL)
        return;
    f = GetBoxMonData(box, MON_DATA_FRIENDSHIP);
    step = FriendshipStep(f);
    f = f + step > MAX_FRIENDSHIP ? MAX_FRIENDSHIP : f + step;
    SetBoxMonData(box, MON_DATA_FRIENDSHIP, &f);
    MPNet_Log("life: moment (memory %u) friendship +%u -> %u", m->kind, step, f);
}

// Something happened that it may remember. Strong, new or story experiences become memories; the rest
// only stir its feelings (docs 4.2). Returns the memory, or NULL when it didn't become one.
struct MpLifeMemory *MP_Life_Remember(struct MpLifeRec *rec, u8 kind, u8 strength, u8 tier, u32 who, u16 param)
{
    u8 i, slot = 0xFF, feeling;
    struct MpLifeMemory *m;
    u32 now = MP_Life_PlayMinutes();
    u32 owner = MP_Life_LocalTrainerKey();

    if (rec == NULL || kind == MEM_NONE || kind >= MEM_KINDS)
        return NULL;
    feeling = sMemFeeling[kind];
    if (tier == TIER_WITNESS)
        strength = strength * 2 / 3;
    else if (tier == TIER_HEARD)
        strength = strength / 2;
    MP_Life_Feel(rec, feeling, strength / 3 + 10);
    if (strength < 80 && kind != MEM_MET)
        return NULL;   // stirred it, but not a memory

    // the same thing remembered again recently strengthens the old memory instead
    for (i = 0; i < LIFE_MEMORIES; i++)
    {
        m = &rec->mem[i];
        if (m->kind == kind && m->param == param && m->who == who && now - m->when < 30 && kind != MEM_MET)
        {
            m->strength = Clamp8(m->strength + strength / 4);
            return NULL;
        }
    }
    // a free slot, or the weakest memory that isn't a story memory or the day you met
    for (i = 0; i < LIFE_MEMORIES; i++)
        if (rec->mem[i].kind == MEM_NONE)
        {
            slot = i;
            break;
        }
    if (slot == 0xFF)
    {
        u16 weakest = 0xFFFF;
        for (i = 0; i < LIFE_MEMORIES; i++)
        {
            u16 w;
            m = &rec->mem[i];
            if (m->kind == MEM_MET || m->kind == MEM_BADGE || m->kind == MEM_STORY || m->kind == MEM_LEGEND
             || m->kind == MEM_FAMILY)
                continue;
            w = m->strength + (now - m->when < 600 ? 64 : 0);   // recent ones weigh a little more
            if (w < weakest && m->strength < strength + 20)
                weakest = w, slot = i;
        }
        if (slot == 0xFF)
            return NULL;   // everything it remembers matters more
    }
    m = &rec->mem[slot];
    m->kind = kind;
    m->feeling = feeling;
    m->strength = strength;
    m->tier = tier;
    m->place = MP_Life_CurrentPlace();
    m->param = param;
    m->who = who;
    m->when = now;
    rec->known[(28 + slot) / 32] &= ~(1u << ((28 + slot) % 32));
    // The Trainer was there (or is who it's about): they know this happened
    if (tier != TIER_HEARD && (who == owner || MP_Life_PartyBox(MP_Life_MonKey(rec->personality, rec->otId))))
        MP_Life_Reveal(rec, 28 + slot);
    DriftFrom(rec, kind, strength);
    MPNet_Log("life: memory %u (strength %u, tier %u) for species %u", kind, strength, tier, rec->species);

    // A good memory that involves its own Trainer is a moment: Friendship rises once (docs 5.4)
    if (IsMomentKind(kind) && IsPositive(feeling) && strength >= 96 && tier != TIER_HEARD
     && (who == owner || (who == 0 && MP_Life_PartyBox(MP_Life_MonKey(rec->personality, rec->otId)))))
        FriendshipMoment(rec, m);
    return m;
}

void MP_Life_Reveal(struct MpLifeRec *rec, u8 fact)
{
    if (rec && fact < 64)
        rec->known[fact / 32] |= 1u << (fact % 32);
}

static void RememberHowWeMet(struct MpLifeRec *rec, struct BoxPokemon *box)
{
    u8 metLoc = GetBoxMonData(box, MON_DATA_MET_LOCATION);
    u8 metLevel = GetBoxMonData(box, MON_DATA_MET_LEVEL);
    u16 how = 0;
    struct MpLifeMemory *m;
    struct MpLifePlace *p;
    if (metLevel == 0)
        how = 1;                                   // hatched
    else if (rec->otId != GetPlayerIDAsU32())
        how = metLoc == METLOC_IN_GAME_TRADE ? 2 : 3;   // received / traded
    m = MP_Life_Remember(rec, MEM_MET, 200, TIER_PART, MP_Life_LocalTrainerKey(), how);
    if (m)
    {
        m->place = metLoc < MAPSEC_NONE ? metLoc : MAPSEC_NONE;
        MP_Life_Reveal(rec, 28 + (m - rec->mem));
    }
    if (metLoc < MAPSEC_NONE && (p = MP_Life_Place(rec, metLoc, TRUE)) != NULL)
    {
        p->flags |= LIFE_PLACE_MET | (how == 1 ? LIFE_PLACE_HOME : 0);
        p->level = p->level < 40 ? 40 : p->level;
    }
}

// ---------------------------------------------------------------------------
// The minute tick (party Pokémon only: the PC is a calm rest, nothing builds up there)

static void NeedsMinute(struct MpLifeRec *rec, u32 minutes)
{
    u32 inst = Instincts(rec->species);
    s32 m = minutes > 120 ? 120 : minutes;   // a long gap (the game wasn't running) counts as 2 hours at most
    rec->need[NEED_HUNGER] = Clamp8(rec->need[NEED_HUNGER] + m * ((inst & INST_GLUTTON) ? 4 : 3));
    rec->need[NEED_REST] = Clamp8(rec->need[NEED_REST] + m * ((inst & INST_SLEEP) ? 3 : 2));
    rec->need[NEED_PLAY] = Clamp8(rec->need[NEED_PLAY] + m * (1 + rec->cur[LIFE_LIVELY] / 96));
    rec->need[NEED_TYPE_A] = Clamp8(rec->need[NEED_TYPE_A] + m * 2);
    rec->need[NEED_TYPE_B] = Clamp8(rec->need[NEED_TYPE_B] + m * 2);
    if (rec->cur[LIFE_SOCIAL] >= 128)
    {
        rec->need[NEED_COMPANY] = Clamp8(rec->need[NEED_COMPANY] + m * (1 + (rec->cur[LIFE_SOCIAL] - 128) / 64));
        rec->need[NEED_QUIET] = Clamp8(rec->need[NEED_QUIET] - m * 4);
    }
    else
    {
        rec->need[NEED_QUIET] = Clamp8(rec->need[NEED_QUIET] + m * (1 + (128 - rec->cur[LIFE_SOCIAL]) / 64));
        rec->need[NEED_COMPANY] = Clamp8(rec->need[NEED_COMPANY] - m * 4);
    }
    {
        // exploring a place it knows well makes it curious for somewhere new; a new place satisfies that
        struct MpLifePlace *p = MP_Life_Place(rec, MP_Life_CurrentPlace(), FALSE);
        if (p && p->level > 60)
            rec->need[NEED_EXPLORE] = Clamp8(rec->need[NEED_EXPLORE] + m * (1 + rec->cur[LIFE_CURIOUS] / 96));
        else
            MP_Life_Satisfy(rec, NEED_EXPLORE, m * 4);
    }
}

static void MoodMinute(struct MpLifeRec *rec)
{
    s32 target = (rec->emo[EMO_JOY] + rec->emo[EMO_AFFECTION] + rec->emo[EMO_PRIDE] + rec->emo[EMO_CALM]
               - rec->emo[EMO_FEAR] - rec->emo[EMO_FRUSTRATION] - rec->emo[EMO_SADNESS]) / 2;
    if (target > 100) target = 100;
    if (target < -100) target = -100;
    // the slow mood moves a little toward how it feels, and rests at content when nothing happens
    if (target == 0 && rec->mood < 10)
        target = 10;
    rec->mood += (target - rec->mood) / 8 + (target > rec->mood ? 1 : target < rec->mood ? -1 : 0);
}

static void ArrivedAt(struct MpLifeRec *rec, u16 place, bool8 present)
{
    if (MP_Life_Place(rec, place, FALSE) == NULL)
    {
        MP_Life_Place(rec, place, TRUE);
        // first time in a town or city: a memory (routes are just familiar)
        if (place <= MAPSEC_EVER_GRANDE_CITY && present)
            MP_Life_Remember(rec, MEM_NEW_PLACE, 100 + rec->cur[LIFE_CURIOUS] / 4, TIER_WITNESS, 0, place);
    }
}

static void PlacesMinute(struct MpLifeRec *rec)
{
    struct MpLifePlace *p = MP_Life_Place(rec, MP_Life_CurrentPlace(), TRUE);
    u8 i;
    if (p && p->level < 255)
        p->level++;
    // a place it knows deeply and feels good in becomes a favorite (one at a time)
    if (p && p->level >= 200 && rec->mood > 20)
    {
        for (i = 0; i < LIFE_PLACES; i++)
            if (rec->place[i].flags & LIFE_PLACE_FAVORITE)
                return;
        p->flags |= LIFE_PLACE_FAVORITE;
        MP_Life_AddTrait(rec, TRAIT_LOVES_PLACE, p->map, 90);
        MPNet_Log("life: favorite place %u for species %u", p->map, rec->species);
    }
}

static void ReunionCheck(struct MpLifeRec *rec, bool8 inParty)
{
    if (inParty && !(rec->flags & LIFE_REC_IN_PARTY))
    {
        // back with its Trainer: a reunion if the adventure moved on meaningfully meanwhile (docs 5.3)
        u16 now = MP_Life_ProgressMark(), was = rec->lastPartyMark;
        bool8 badge = now / 1000 > was / 1000;
        bool8 hours = (now % 1000) >= (was % 1000) + 3;
        if (badge || hours)
        {
            MP_Life_Remember(rec, MEM_REUNION, 120 + (badge ? 60 : 0), TIER_PART, MP_Life_LocalTrainerKey(), 0);
            MP_Life_BondShare(MP_Life_MonKey(rec->personality, rec->otId), MP_Life_LocalTrainerKey(), 2, 2, 4, 0, 0);
        }
    }
    if (inParty)
    {
        rec->flags |= LIFE_REC_IN_PARTY;
        rec->lastPartyMark = MP_Life_ProgressMark();
    }
    else
    {
        rec->flags &= ~LIFE_REC_IN_PARTY;
    }
}

extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];

static bool8 EvolvesInto(u16 from, u16 to)
{
    u8 i;
    if (from == SPECIES_NONE || from >= NUM_SPECIES)
        return FALSE;
    for (i = 0; i < EVOS_PER_MON; i++)
        if (gEvolutionTable[from][i].targetSpecies == to)
            return TRUE;
    return FALSE;
}

static void EvolutionCheck(struct MpLifeRec *rec, struct BoxPokemon *box)
{
    u16 species = GetBoxMonData(box, MON_DATA_SPECIES);
    // (only a real evolution: SHEDINJA shares NINCADA's personality, and must not keep "evolving" it back)
    if (species != rec->species && species != SPECIES_NONE && EvolvesInto(rec->species, species))
    {
        // same self, new body (docs 12): its species instincts change, everything else stays
        struct MpLifePlace *p;
        rec->species = species;
        MP_Life_Remember(rec, MEM_EVOLVED, 210, TIER_PART, MP_Life_LocalTrainerKey(), species);
        if ((p = MP_Life_Place(rec, MP_Life_CurrentPlace(), TRUE)) != NULL)
            p->flags |= LIFE_PLACE_EVOLVED;
    }
}

static bool8 HasMemory(struct MpLifeRec *rec, u8 kind, s32 param, u32 when)
{
    u8 i;
    for (i = 0; i < LIFE_MEMORIES; i++)
        if (rec->mem[i].kind == kind && (param < 0 || rec->mem[i].param == param) && rec->mem[i].when >= when)
            return TRUE;
    return FALSE;
}

bool8 MP_Life_NewBody(struct MpLifeRec *rec)
{
    u32 now = MP_Life_PlayMinutes();
    return rec != NULL && HasMemory(rec, MEM_EVOLVED, -1, now >= 15 ? now - 15 : 0);
}

// ---------------------------------------------------------------------------
// Friends react to an evolution (docs 12): proud, awed, a rival a little jealous. Once per pair and
// evolution (the newest few pairs are kept; a record of which isn't needed past the quarter hour).

static struct { u32 watcher, evolved, when; } sEvoSeen[16];
static u8 sEvoSeenNext;

u8 MP_Life_EvolutionReaction(struct MpLifeRec *watcher, struct MpLifeRec *evolved)
{
    u32 wk, ek, now = MP_Life_PlayMinutes();
    struct MpLifeBond *b;
    u8 i, kind;
    if (watcher == NULL || evolved == NULL || watcher == evolved || !MP_Life_NewBody(evolved))
        return LIFE_EVO_REACT_NONE;
    wk = MP_Life_MonKey(watcher->personality, watcher->otId);
    ek = MP_Life_MonKey(evolved->personality, evolved->otId);
    for (i = 0; i < ARRAY_COUNT(sEvoSeen); i++)
        if (sEvoSeen[i].watcher == wk && sEvoSeen[i].evolved == ek && now - sEvoSeen[i].when <= 20)
            return LIFE_EVO_REACT_NONE;
    b = MP_Life_Bond(wk, ek, FALSE);
    if (b == NULL || b->v[BOND_FAMILIAR] < 20)
        return LIFE_EVO_REACT_NONE;   // it hardly knows it
    if ((b->flags & LIFE_BOND_RIVAL) || b->v[BOND_FRICTION] >= 90)
    {
        kind = LIFE_EVO_REACT_JEALOUS;
        MP_Life_Feel(watcher, EMO_FRUSTRATION, 15);
        MP_Life_BondShare(wk, ek, 1, 0, 0, 3, 2);
    }
    else if (MP_Life_BondWarmth(b) >= 250 || (b->flags & LIFE_BOND_FAMILY))
    {
        kind = LIFE_EVO_REACT_PROUD;
        MP_Life_Feel(watcher, EMO_PRIDE, 20);
        MP_Life_Feel(watcher, EMO_JOY, 10);
        MP_Life_BondShare(wk, ek, 2, 1, 2, 2, 0);
    }
    else
    {
        kind = LIFE_EVO_REACT_AWED;
        MP_Life_Feel(watcher, EMO_SURPRISE, 20);
        MP_Life_BondShare(wk, ek, 1, 0, 0, 3, 0);
    }
    sEvoSeen[sEvoSeenNext].watcher = wk;
    sEvoSeen[sEvoSeenNext].evolved = ek;
    sEvoSeen[sEvoSeenNext].when = now;
    sEvoSeenNext = (sEvoSeenNext + 1) % ARRAY_COUNT(sEvoSeen);
    MPNet_Log("life: evolution reaction %u from species %u to species %u", kind, watcher->species, evolved->species);
    return kind;
}

// ---------------------------------------------------------------------------
// Eggs and family (docs 12): a hatchling's slight leanings from its parents, sometimes a family trait,
// and family bonds with its parents (and siblings) once they're together.

struct MpLifeTrait *MP_Life_FamilyTrait(struct MpLifeRec *rec, u8 id)
{
    u8 i;
    if (rec == NULL)
        return NULL;
    for (i = 0; i < LIFE_TRAITS; i++)
        if (rec->trait[i].id == id && rec->trait[i].strength)
            return &rec->trait[i];
    return NULL;
}

bool8 MP_Life_IsFamily(u32 a, u32 b)
{
    struct MpLifeBond *x = MP_Life_Bond(a, b, FALSE);
    return x != NULL && (x->flags & LIFE_BOND_FAMILY);
}

static void FamilyBond(u32 a, u32 b)
{
    struct MpLifeBond *x = MP_Life_Bond(a, b, TRUE), *y = MP_Life_Bond(b, a, TRUE);
    if (x)
        x->flags |= LIFE_BOND_FAMILY;
    if (y)
        y->flags |= LIFE_BOND_FAMILY;
}

// the EGG left the DAY CARE (daycare.c): its record is made now, from its parents
void MP_Life_NoteEgg(struct BoxPokemon *egg, struct BoxPokemon *parentA, struct BoxPokemon *parentB)
{
    struct BoxPokemon *parents[2] = {parentA, parentB};
    struct MpLifeRec *rec, *pr[2] = {NULL, NULL};
    u32 personality, otId;
    s32 sum[LIFE_AXES];
    u8 i, n = 0, p;
    if (egg == NULL || GetBoxMonData(egg, MON_DATA_SPECIES) == SPECIES_NONE)
        return;
    personality = GetBoxMonData(egg, MON_DATA_PERSONALITY);
    otId = GetBoxMonData(egg, MON_DATA_OT_ID);
    if (FindRec(personality, otId) != NULL || (rec = NewRecSlot()) == NULL)
        return;
    memset(rec, 0, sizeof(*rec));
    rec->personality = personality;
    rec->otId = otId;
    rec->species = GetBoxMonData(egg, MON_DATA_SPECIES);
    rec->version = LIFE_VERSION;
    rec->flags = LIFE_REC_EGG;
    rec->createdMin = rec->lastTickMin = MP_Life_PlayMinutes();
    rec->lastPartyMark = MP_Life_ProgressMark();
    rec->emo[EMO_CALM] = 40;
    RollTemperament(rec, egg);
    memset(sum, 0, sizeof(sum));
    for (p = 0; p < 2; p++)
    {
        if (parents[p] == NULL || GetBoxMonData(parents[p], MON_DATA_SPECIES) == SPECIES_NONE)
            continue;
        rec->parents[p] = MP_Life_BoxKey(parents[p]);
        if ((pr[p] = MP_Life_Get(parents[p], FALSE)) == NULL)
            continue;
        for (i = 0; i < LIFE_AXES; i++)
            sum[i] += pr[p]->base[i];
        n++;
    }
    // slight leanings: a quarter of the way toward its parents' temperaments, never a copy
    if (n)
        for (i = 0; i < LIFE_AXES; i++)
        {
            s32 lean = (sum[i] / n - rec->base[i]) / 4;
            rec->base[i] = rec->cur[i] = Clamp8(rec->base[i] + lean);
        }
    // sometimes a small family trait (a third of the time): a parent's comfort food, a song, a greeting
    if (n && (personality >> 8) % 3 == 0)
    {
        struct MpLifeRec *from = pr[0] ? pr[0] : pr[1];
        u8 which = (personality >> 16) % 3;
        u32 fromKey = MP_Life_MonKey(from->personality, from->otId);
        struct MpLifeTrait *food = NULL;
        for (i = 0; i < LIFE_TRAITS; i++)
            if (from->trait[i].id == TRAIT_COMFORT_FOOD && from->trait[i].strength)
                food = &from->trait[i];
        if (which == 0)
        {
            // the family's favorite food: its parent's comfort food, or the flavor its parent loves most
            u8 flavor = 0;
            if (food)
                flavor = food->param;
            else
                for (i = 1; i < FLAVOR_COUNT; i++)
                    if (from->taste[i] > from->taste[flavor])
                        flavor = i;
            if (flavor < FLAVOR_COUNT)
            {
                rec->taste[flavor] = rec->taste[flavor] < 60 ? 60 : rec->taste[flavor];
                MP_Life_AddTrait(rec, TRAIT_COMFORT_FOOD, flavor, 90);
            }
        }
        else
        {
            MP_Life_AddTrait(rec, which == 1 ? TRAIT_FAMILY_SONG : TRAIT_FAMILY_GREETING, fromKey & 0xFFFF, 90);
        }
    }
    MPNet_Log("life: egg record for species %u (parents known %u)", rec->species, n);
}

// it hatched: everything a new Pokémon gets, plus knowing its parents
static void FinishEgg(struct MpLifeRec *rec, struct BoxPokemon *box)
{
    u32 me = MP_Life_MonKey(rec->personality, rec->otId);
    u8 p;
    rec->flags &= ~LIFE_REC_EGG;
    rec->species = GetBoxMonData(box, MON_DATA_SPECIES);
    rec->createdMin = rec->lastTickMin = MP_Life_PlayMinutes();
    InitialOwnerBond(rec, box);
    RememberHowWeMet(rec, box);
    {
        u8 name[POKEMON_NAME_LENGTH + 1];
        GetBoxMonData(box, MON_DATA_NICKNAME, name);
        name[POKEMON_NAME_LENGTH] = EOS;
        MP_Life_NoteName(me, name);
    }
    for (p = 0; p < 2; p++)
        if (rec->parents[p] && MP_Life_FindKey(rec->parents[p]))
            FamilyBond(me, rec->parents[p]);
    rec->flags |= LIFE_REC_FAMILY_KNOWN;
    MPNet_Log("life: hatched species %u", rec->species);
}

static bool8 MetFamily(u32 a, u32 b)
{
    struct MpLifeBond *x = MP_Life_Bond(a, b, FALSE);
    return x != NULL && (x->flags & LIFE_BOND_MET);
}

// in the party together: parents and children (and siblings) meet (MEM_FAMILY param: 0 its little one,
// 1 its parent, 2 its sibling)
static void FamilyCheck(struct MpLifeRec *rec)
{
    u32 me = MP_Life_MonKey(rec->personality, rec->otId);
    u8 i, p, q;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct MpLifeRec *o = MP_Life_Get(&gPlayerParty[i].box, FALSE);
        u32 ok;
        u8 role = 0xFF;
        if (o == NULL || o == rec)
            continue;
        ok = MP_Life_MonKey(o->personality, o->otId);
        if (rec->parents[0] == ok || rec->parents[1] == ok)
            role = 1;
        else if (o->parents[0] == me || o->parents[1] == me)
            role = 0;
        else
            for (p = 0; p < 2; p++)
                for (q = 0; q < 2; q++)
                    if (rec->parents[p] && rec->parents[p] == o->parents[q])
                        role = 2;
        if (role == 0xFF || MetFamily(me, ok))
            continue;
        FamilyBond(me, ok);
        {
            struct MpLifeBond *x = MP_Life_Bond(me, ok, TRUE), *y = MP_Life_Bond(ok, me, TRUE);
            if (x)
                x->flags |= LIFE_BOND_MET;
            if (y)
                y->flags |= LIFE_BOND_MET;
        }
        MP_Life_BondShare(me, ok, 10, 8, 8, 0, 0);
        MP_Life_BondShare(ok, me, 10, 8, 8, 0, 0);
        MP_Life_Remember(rec, MEM_FAMILY, 180, TIER_PART, ok, role);
        MP_Life_Remember(o, MEM_FAMILY, 180, TIER_PART, me, role == 2 ? 2 : role ^ 1);
        MPNet_Log("life: family met (role %u) species %u and %u", role, rec->species, o->species);
    }
}

static bool8 HasMemoryAt(struct MpLifeRec *rec, u32 when)
{
    u8 i;
    for (i = 0; i < LIFE_MEMORIES; i++)
        if (rec->mem[i].kind && rec->mem[i].when == when)
            return TRUE;
    return FALSE;
}

// Word spreads (docs 8.1): a teammate who lived it tells one who wasn't there, in its own way. Only between
// Pokémon together in the party (the Garden too, mp_garden.c); one in the PC hears once it's back.
static bool8 sSpreadForce;   // tests: the next telling happens for sure

static void SpreadWord(u32 now)
{
    s8 n;
    u8 count, i, j;
    const struct MpLifeNews *news = MP_Life_News(&count);
    if (count == 0 || (!sSpreadForce && (Random() % 3) != 0))
        return;
    sSpreadForce = FALSE;
    for (n = count - 1; n >= 0; n--)
    {
        const struct MpLifeNews *nw = &news[n];
        u16 heardParam = (nw->kind << 8) | (nw->param & 0xFF);
        if (now < nw->when + 1)
            continue;   // not right away: once things have settled
        for (i = 0; i < PARTY_SIZE; i++)
        {
            struct MpLifeRec *l = MP_Life_Get(&gPlayerParty[i].box, FALSE), *t = NULL;
            u32 lk;
            if (l == NULL || GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
                continue;
            lk = MP_Life_MonKey(l->personality, l->otId);
            if (lk == nw->teller || HasMemory(l, MEM_HEARD, heardParam, nw->when) || HasMemory(l, MEM_HEARD, -1, now > 30 ? now - 30 : 0))
                continue;
            if (nw->teller ? HasMemoryAt(l, nw->when) : HasMemory(l, nw->kind, nw->param, 0))
                continue;   // it was there
            for (j = 0; j < PARTY_SIZE && t == NULL; j++)
            {
                struct MpLifeRec *c = MP_Life_Get(&gPlayerParty[j].box, FALSE);
                u32 ck;
                if (c == NULL || c == l)
                    continue;
                ck = MP_Life_MonKey(c->personality, c->otId);
                if (nw->teller ? ck == nw->teller : HasMemory(c, nw->kind, nw->param, 0))
                    t = c;
            }
            if (t == NULL)
                continue;
            {
                u32 tk = MP_Life_MonKey(t->personality, t->otId);
                struct MpLifeBond *bond = MP_Life_Bond(lk, tk, FALSE);
                u8 strength = 170;
                // telling is in character: a boastful one makes it sound bigger, a nervous one scarier
                if (t->cur[LIFE_PROUD] >= 170)
                    strength = 200, MP_Life_Feel(l, EMO_PRIDE, 15);
                if (t->cur[LIFE_BOLD] < 80 && (nw->kind == MEM_LEGEND || (nw->kind == MEM_STORY && nw->param >= LIFE_STORY_GROUDON && nw->param <= LIFE_STORY_RAYQUAZA)))
                    MP_Life_Feel(l, EMO_FEAR, 15);
                MP_Life_Remember(l, MEM_HEARD, strength, TIER_HEARD, tk, heardParam);
                // the listener's reaction depends on the teller: it believes a friend; a rival's boasting irritates it
                if (bond && (bond->flags & LIFE_BOND_RIVAL) && t->cur[LIFE_PROUD] >= 150)
                {
                    MP_Life_Feel(l, EMO_FRUSTRATION, 12);
                    MP_Life_BondShare(lk, tk, 2, 0, 0, 0, 2);
                }
                else
                {
                    MP_Life_Feel(l, EMO_PRIDE, 10);   // proud by association
                    MP_Life_BondShare(lk, tk, 2, 1, 1, 0, 0);
                }
                MPNet_Log("life: word spread (news %u) from species %u to species %u", nw->kind, t->species, l->species);
                return;   // one telling at a time
            }
        }
    }
}

static void MinuteTick(u32 now, u32 minutes)
{
    u8 i;
    u16 place = MP_Life_CurrentPlace();
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct BoxPokemon *box = &gPlayerParty[i].box;
        struct MpLifeRec *rec = MP_Life_Get(box, TRUE);
        if (rec == NULL)
            continue;
        EvolutionCheck(rec, box);
        FamilyCheck(rec);
        if (place != sLastPlace)
            ArrivedAt(rec, place, TRUE);
        NeedsMinute(rec, minutes);
        PlacesMinute(rec);
        MoodMinute(rec);
        rec->lastTickMin = now;
    }
    sLastPlace = place;
    SpreadWord(now);
    // reunions: who is in the party now, who isn't
    {
        u16 r;
        for (r = 0; r < sRecCount; r++)
        {
            struct MpLifeRec *rec = &sRecs[r];
            bool8 here = MP_Life_PartyBox(MP_Life_MonKey(rec->personality, rec->otId)) != NULL;
            ReunionCheck(rec, here);
        }
    }
}

// ---------------------------------------------------------------------------
// The story and badges (docs 8.1-8.2): beats the save's flags mark, lived by the party

static const struct { u16 flag; u8 beat; u16 legend; u8 strength; } sStory[] = {
    {FLAG_DEFEATED_RIVAL_ROUTE103, LIFE_STORY_RIVAL, 0, 150},
    {FLAG_DEFEATED_RIVAL_ROUTE_104, LIFE_STORY_RIVAL, 0, 150},
    {FLAG_DEFEATED_RIVAL_RUSTBORO, LIFE_STORY_RIVAL, 0, 160},
    {FLAG_DEFEATED_WALLY_MAUVILLE, LIFE_STORY_WALLY, 0, 150},
    {FLAG_DEFEATED_WALLY_VICTORY_ROAD, LIFE_STORY_WALLY, 0, 190},
    {FLAG_BEAT_MAGMA_GRUNT_JAGGED_PASS, LIFE_STORY_MAGMA, 0, 150},
    {FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY, LIFE_STORY_MT_CHIMNEY, 0, 200},
    {FLAG_RECEIVED_CASTFORM, LIFE_STORY_WEATHER_INST, 0, 170},
    {FLAG_DEFEATED_MAGMA_SPACE_CENTER, LIFE_STORY_SPACE_CENTER, 0, 200},
    {FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE, LIFE_STORY_AQUA, 0, 170},
    {FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT, LIFE_STORY_GROUDON, 0, 220},
    {FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN, LIFE_STORY_KYOGRE, 0, 220},
    {FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE, LIFE_STORY_RAYQUAZA, 0, 230},
    {FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE, LIFE_STORY_SOOTOPOLIS, 0, 220},
    {FLAG_DEFEATED_ELITE_4_DRAKE, LIFE_STORY_ELITE_FOUR, 0, 220},
    {FLAG_SYS_GAME_CLEAR, LIFE_STORY_CHAMPION, 0, 250},
    {FLAG_DEFEATED_GROUDON, 0, SPECIES_GROUDON, 220},
    {FLAG_DEFEATED_KYOGRE, 0, SPECIES_KYOGRE, 220},
    {FLAG_DEFEATED_RAYQUAZA, 0, SPECIES_RAYQUAZA, 230},
};

static u32 CurrentStoryMask(void)
{
    u32 m = 0;
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sStory); i++)
        if (FlagGet(sStory[i].flag))
            m |= 1u << i;
    return m;
}

static u32 CurrentBadgeMask(void)
{
    u32 m = 0;
    u8 i;
    for (i = 0; i < 8; i++)
        if (FlagGet(FLAG_BADGE01_GET + i))
            m |= 1u << i;
    return m;
}

// News for word spreading (mp_life_social.c): who in the party lived it tells the others later
void MP_Life_AddNews(u8 kind, u16 param, u32 teller)
{
    if (sNewsCount == LIFE_NEWS)
    {
        memmove(&sNews[0], &sNews[1], sizeof(sNews[0]) * (LIFE_NEWS - 1));
        sNewsCount--;
    }
    sNews[sNewsCount].kind = kind;
    sNews[sNewsCount].param = param;
    sNews[sNewsCount].teller = teller;
    sNews[sNewsCount].when = MP_Life_PlayMinutes();
    sNewsCount++;
}

u32 MP_Life_LastLineMin(void)
{
    return sLastLineMin;
}

void MP_Life_SetLastLineMin(u32 minute)
{
    sLastLineMin = minute;
}

const struct MpLifeNews *MP_Life_News(u8 *count)
{
    *count = sNewsCount;
    return sNews;
}

static void PartyLives(u8 kind, u16 param, u8 strength)
{
    u8 i;
    u32 follower = gMpConfig.followerPersonality;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct BoxPokemon *box = &gPlayerParty[i].box;
        struct MpLifeRec *rec = MP_Life_Get(box, TRUE);
        bool8 inFront;
        if (rec == NULL || GetMonData(&gPlayerParty[i], MON_DATA_HP) == 0)
            continue;
        // the follower and the lead were right there; the rest watched from their balls
        inFront = rec->personality == follower || i == 0;
        MP_Life_Remember(rec, kind, strength, inFront ? TIER_PART : TIER_WITNESS, 0, param);
        if (kind == MEM_BADGE || kind == MEM_STORY)
            MP_Life_Feel(rec, EMO_PRIDE, 30);
    }
}

static void StoryTick(void)
{
    u32 story, badges, changed;
    u8 i;
    if (gSaveBlock1Ptr == NULL)
        return;
    story = CurrentStoryMask();
    badges = CurrentBadgeMask();
    if (!sStoryKnown)
    {
        // a save from before the Garden: what happened before isn't relived now
        sStoryMask = story;
        sBadgeMask = badges;
        sStoryKnown = TRUE;
        return;
    }
    changed = badges & ~sBadgeMask;
    for (i = 0; i < 8; i++)
        if (changed & (1u << i))
        {
            PartyLives(MEM_BADGE, i, 230);
            MP_Life_AddNews(MEM_BADGE, i, 0);
            MP_LifeEmote_Show(LIFE_EMOTE_SLOT_LOCAL, LIFE_EMOTE_SPARKLE);
            MPNet_Log("life: badge %u", i + 1);
        }
    changed = story & ~sStoryMask;
    for (i = 0; i < ARRAY_COUNT(sStory); i++)
        if (changed & (1u << i))
        {
            if (sStory[i].legend)
            {
                PartyLives(MEM_LEGEND, sStory[i].legend, sStory[i].strength);
                MP_Life_AddNews(MEM_LEGEND, sStory[i].legend, 0);
            }
            else
            {
                PartyLives(MEM_STORY, sStory[i].beat, sStory[i].strength);
                MP_Life_AddNews(MEM_STORY, sStory[i].beat, 0);
            }
            MP_LifeEmote_Show(LIFE_EMOTE_SLOT_LOCAL, sStory[i].beat == LIFE_STORY_GROUDON || sStory[i].beat == LIFE_STORY_KYOGRE
                              ? LIFE_EMOTE_EXCLAIM : LIFE_EMOTE_SPARKLE);
            MPNet_Log("life: story %u (beat %u)", i, sStory[i].beat);
        }
    sStoryMask = story;
    sBadgeMask = badges;
}

// Emotions settle on their own over a few minutes (every 2 seconds: -1)
static void EmotionStep(void)
{
    u8 i, e;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct MpLifeRec *rec = MP_Life_Get(&gPlayerParty[i].box, FALSE);
        if (rec == NULL)
            continue;
        for (e = 0; e < LIFE_EMOTIONS; e++)
        {
            if (e == EMO_CALM)
            {
                if (rec->emo[e] < 40)
                    rec->emo[e]++;
            }
            else if (rec->emo[e])
            {
                rec->emo[e]--;
            }
        }
    }
}

void MP_Life_FrameUpdate(void)
{
    u32 now;
    if (!sLoaded || gSaveBlock2Ptr == NULL || !MP_IsInOverworld())
        return;
    now = MP_Life_PlayMinutes();
    if (sLastMinute == 0 || now < sLastMinute)
        sLastMinute = now;
    if (now != sLastMinute)
    {
        MinuteTick(now, now - sLastMinute);
        sLastMinute = now;
    }
    if (++sFrame >= 120)
    {
        u8 i;
        sFrame = 0;
        EmotionStep();
        // everyone in the party has a record (a new catch, a Pokémon taken from the PC)
        for (i = 0; i < PARTY_SIZE; i++)
            MP_Life_Get(&gPlayerParty[i].box, TRUE);
        StoryTick();
    }
}

// ---------------------------------------------------------------------------
// The save sidecar

struct LifeHeader
{
    u32 magic;
    u32 version;
    u32 saveCounter;
    u32 recSize, recCount;
    u32 bondSize, bondCount;
    u32 nameSize, nameCount;
    u32 lastLineMin;        // play minute of the last rare battle line (mp_life_battle.c), 0 = never
    struct MpGardenSave garden;   // the MOéMON GARDEN (mp_garden.c)
    u32 storyMask;          // story beats already lived (sStory bits)
    u32 badgeMask;          // badges already celebrated
    u32 newsCount;
    struct MpLifeNews news[LIFE_NEWS];
};

static void SidecarPath(char *out, size_t size, int backup)
{
    if (backup)
        snprintf(out, size, "%s.bak%d.life", gPlatformSavePath, backup);
    else
        snprintf(out, size, "%s.life", gPlatformSavePath);
}

void MP_Life_Reset(void)
{
    sStoryKnown = FALSE;
    sLastLineMin = 0;
    memset(&sGarden, 0, sizeof(sGarden));
    sNewsCount = 0;
    memset(sNews, 0, sizeof(sNews));
    sRecCount = 0;
    sBondCount = 0;
    sNameCount = 0;
    sNameNext = 0;
    sLastMinute = 0;
    sLastPlace = 0xFFFF;
    sLoaded = TRUE;
}

// Right after the save file was written (sdl2.c StoreSaveFile)
void MP_Life_WriteSidecar(void)
{
    char path[600], tmp[620];
    FILE *f;
    struct LifeHeader h;
    if (!sLoaded || gSaveBlock1Ptr == NULL)
        return;
    SidecarPath(path, sizeof(path), 0);
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (f == NULL)
        return;
    h.magic = LIFE_MAGIC;
    h.version = LIFE_VERSION;
    h.saveCounter = gSaveCounter;
    h.recSize = sizeof(struct MpLifeRec);
    h.recCount = sRecCount;
    h.bondSize = sizeof(struct MpLifeBond);
    h.bondCount = sBondCount;
    h.nameSize = sizeof(struct MpLifeName);
    h.nameCount = sNameCount;
    h.lastLineMin = sLastLineMin;
    h.garden = sGarden;
    h.storyMask = sStoryMask;
    h.badgeMask = sBadgeMask;
    h.newsCount = sNewsCount;
    memcpy(h.news, sNews, sizeof(h.news));
    fwrite(&h, sizeof(h), 1, f);
    fwrite(sRecs, sizeof(struct MpLifeRec), sRecCount, f);
    fwrite(sBonds, sizeof(struct MpLifeBond), sBondCount, f);
    fwrite(sNames, sizeof(struct MpLifeName), sNameCount, f);
    fclose(f);
    remove(path);
    rename(tmp, path);
}

static bool8 ReadSidecar(const char *path, bool8 needCounter)
{
    FILE *f = fopen(path, "rb");
    struct LifeHeader h;
    u32 i;
    if (f == NULL)
        return FALSE;
    if (fread(&h, sizeof(h), 1, f) != 1 || h.magic != LIFE_MAGIC || h.version < 2 || (needCounter && h.saveCounter != gSaveCounter)
     || h.recSize == 0 || h.bondSize == 0 || h.nameSize < sizeof(u32) + 2)
    {
        fclose(f);
        return FALSE;
    }
    // records and bonds of another size (a newer or older build): copy what both know
    sRecCount = 0;
    for (i = 0; i < h.recCount; i++)
    {
        u8 buf[sizeof(struct MpLifeRec) * 2];
        u32 n = h.recSize < sizeof(buf) ? h.recSize : sizeof(buf);
        if (fread(buf, 1, n, f) != n)
            break;
        if (h.recSize > n)
            fseek(f, h.recSize - n, SEEK_CUR);
        if (sRecCount < LIFE_MAX_RECS)
        {
            memset(&sRecs[sRecCount], 0, sizeof(struct MpLifeRec));
            memcpy(&sRecs[sRecCount], buf, n < sizeof(struct MpLifeRec) ? n : sizeof(struct MpLifeRec));
            sRecCount++;
        }
    }
    sBondCount = 0;
    for (i = 0; i < h.bondCount; i++)
    {
        u8 buf[sizeof(struct MpLifeBond) * 2];
        u32 n = h.bondSize < sizeof(buf) ? h.bondSize : sizeof(buf);
        if (fread(buf, 1, n, f) != n)
            break;
        if (h.bondSize > n)
            fseek(f, h.bondSize - n, SEEK_CUR);
        if (sBondCount < LIFE_MAX_BONDS)
        {
            memset(&sBonds[sBondCount], 0, sizeof(struct MpLifeBond));
            memcpy(&sBonds[sBondCount], buf, n < sizeof(struct MpLifeBond) ? n : sizeof(struct MpLifeBond));
            sBondCount++;
        }
    }
    sNameCount = 0;
    for (i = 0; i < h.nameCount; i++)
    {
        u8 buf[sizeof(struct MpLifeName) * 2];
        u32 n = h.nameSize < sizeof(buf) ? h.nameSize : sizeof(buf);
        if (fread(buf, 1, n, f) != n)
            break;
        if (h.nameSize > n)
            fseek(f, h.nameSize - n, SEEK_CUR);
        if (sNameCount < LIFE_MAX_NAMES)
        {
            struct MpLifeName *nm = &sNames[sNameCount++];
            memset(nm, 0, sizeof(*nm));
            memcpy(nm, buf, n < sizeof(*nm) ? n : sizeof(*nm));
            nm->name[LIFE_NAME_LEN - 1] = EOS;   // a longer name from a newer build: cut, still ended
        }
    }
    sLastLineMin = h.lastLineMin;
    sGarden = h.garden;
    sStoryMask = h.storyMask;
    sBadgeMask = h.badgeMask;
    sNewsCount = h.newsCount <= LIFE_NEWS ? h.newsCount : LIFE_NEWS;
    memcpy(sNews, h.news, sizeof(sNews));
    sStoryKnown = TRUE;
    fclose(f);
    return TRUE;
}

// After a save was loaded (save.c): this save's own sidecar (or its backup's); failing that, the latest
// one (records are kept per Pokémon, so a sidecar from a slightly different save still fits)
void MP_Life_LoadFromSave(void)
{
    char path[600];
    int i;
    bool8 found = FALSE;
    MP_Life_Reset();
    for (i = 0; i <= 3 && !found; i++)
    {
        SidecarPath(path, sizeof(path), i);
        found = ReadSidecar(path, TRUE);
    }
    if (!found)
    {
        SidecarPath(path, sizeof(path), 0);
        found = ReadSidecar(path, FALSE);
    }
    // Who's in the party now counts as with its Trainer (no reunion on loading)
    {
        u16 r;
        for (r = 0; r < sRecCount; r++)
        {
            struct MpLifeRec *rec = &sRecs[r];
            if (MP_Life_PartyBox(MP_Life_MonKey(rec->personality, rec->otId)))
                rec->flags |= LIFE_REC_IN_PARTY;
        }
    }
    sLastPlace = 0xFFFF;
    MPNet_Log("life: loaded %u records, %u bonds%s", sRecCount, sBondCount, found ? "" : " (new)");
}

// ---------------------------------------------------------------------------
// Debug: mp life ...

static const char *const sEmoNames[LIFE_EMOTIONS] = {"joy", "fear", "frustration", "sadness", "surprise", "affection", "pride", "calm"};
static const char *const sNeedNames[LIFE_NEEDS] = {"hunger", "rest", "play", "explore", "company", "quiet", "typeA", "typeB"};

void MP_GameToAscii(char *dst, const u8 *src, int maxLen);

static bool8 sWantsForce;   // tests: skip the 10-minute spells

static void DebugRec(u8 slot)
{
    struct MpLifeRec *rec = MP_Life_Get(&gPlayerParty[slot].box, FALSE);
    struct MpLifeBond *b;
    u8 i;
    if (rec == NULL)
    {
        printf("[test] life %u: no record\n", slot);
        return;
    }
    printf("[test] life %u: species %u temper %u/%u %u/%u %u/%u %u/%u %u/%u %u/%u mood %d\n", slot, rec->species,
           rec->base[0], rec->cur[0], rec->base[1], rec->cur[1], rec->base[2], rec->cur[2], rec->base[3], rec->cur[3],
           rec->base[4], rec->cur[4], rec->base[5], rec->cur[5], rec->mood);
    printf("[test] life %u: needs", slot);
    for (i = 0; i < LIFE_NEEDS; i++)
        printf(" %s %u", sNeedNames[i], rec->need[i]);
    printf(" | type needs %u %u lean %u\n", MP_Life_TypeNeed(rec, FALSE), MP_Life_TypeNeed(rec, TRUE), rec->typeLean);
    printf("[test] life %u: emotions", slot);
    for (i = 0; i < LIFE_EMOTIONS; i++)
        printf(" %s %u", sEmoNames[i], rec->emo[i]);
    printf("\n[test] life %u: tastes %d %d %d %d %d boundaries 0x%x\n", slot, rec->taste[0], rec->taste[1], rec->taste[2],
           rec->taste[3], rec->taste[4], rec->boundaries);
    for (i = 0; i < LIFE_MEMORIES; i++)
        if (rec->mem[i].kind)
            printf("[test] life %u: memory %u kind %u feeling %u strength %u tier %u place %u param %u who %08x\n", slot, i,
                   rec->mem[i].kind, rec->mem[i].feeling, rec->mem[i].strength, rec->mem[i].tier, rec->mem[i].place,
                   rec->mem[i].param, (unsigned)rec->mem[i].who);
    for (i = 0; i < LIFE_TRAITS; i++)
        if (rec->trait[i].strength)
            printf("[test] life %u: trait %u param %u strength %u\n", slot, rec->trait[i].id, rec->trait[i].param, rec->trait[i].strength);
    printf("[test] life %u: pursuit %u level %u rare seen 0x%x flags 0x%x parents %08x %08x new body %d\n", slot, rec->pursuit,
           rec->pursuitLevel, (unsigned)rec->rareSeen, rec->flags, (unsigned)rec->parents[0], (unsigned)rec->parents[1],
           MP_Life_NewBody(rec));
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct MpLifeRec *o = MP_Life_Get(&gPlayerParty[i].box, FALSE);
        struct MpLifeBond *ob;
        if (o == NULL || o == rec)
            continue;
        ob = MP_Life_Bond(MP_Life_MonKey(rec->personality, rec->otId), MP_Life_MonKey(o->personality, o->otId), FALSE);
        if (ob)
            printf("[test] life %u: bond with %u fam %u trust %u aff %u resp %u fric %u flags 0x%x shared %u\n", slot, i,
                   ob->v[0], ob->v[1], ob->v[2], ob->v[3], ob->v[4], ob->flags, ob->shared);
    }
    b = MP_Life_Bond(MP_Life_MonKey(rec->personality, rec->otId), MP_Life_LocalTrainerKey(), FALSE);
    if (b)
        printf("[test] life %u: owner bond fam %u trust %u aff %u resp %u fric %u shared %u\n", slot, b->v[0], b->v[1], b->v[2],
               b->v[3], b->v[4], b->shared);
}

void MP_Life_Debug(const char *args)
{
    char cmd[32] = "";
    int a = 0, b = 0, c = 0, d = 0;
    sscanf(args, "%31s %d %d %d %d", cmd, &a, &b, &c, &d);
    if (strcmp(cmd, "show") == 0 && a >= 0 && a < PARTY_SIZE)
    {
        DebugRec(a);
    }
    else if (strcmp(cmd, "remember") == 0 && a >= 0 && a < PARTY_SIZE)
    {
        // remember SLOT KIND STRENGTH [WHO: 0 nobody, 1 the owner]
        struct MpLifeRec *rec = MP_Life_Get(&gPlayerParty[a].box, TRUE);
        struct MpLifeMemory *m = MP_Life_Remember(rec, b, c, TIER_PART, d == 1 ? MP_Life_LocalTrainerKey() : 0, 0);
        printf("[test] life remember %d kind %d -> %s friendship %u\n", a, b, m ? "memory" : "no memory",
               (unsigned)GetMonData(&gPlayerParty[a], MON_DATA_FRIENDSHIP));
    }
    else if (strcmp(cmd, "minutes") == 0)
    {
        // minutes N: as if N minutes of play went by
        u32 now = MP_Life_PlayMinutes();
        MinuteTick(now, a);
        printf("[test] life minutes %d\n", a);
    }
    else if (strcmp(cmd, "trait") == 0 && a >= 0 && a < PARTY_SIZE)
    {
        struct MpLifeRec *rec = MP_Life_Get(&gPlayerParty[a].box, TRUE);
        if (d < 0)
            MP_Life_SoftenTrait(rec, b, c, -d);
        else
            MP_Life_AddTrait(rec, b, c, d);
        printf("[test] life trait %d %d %d %d\n", a, b, c, d);
    }
    else if (strcmp(cmd, "need") == 0 && a >= 0 && a < PARTY_SIZE)
    {
        // need SLOT NEED VALUE
        struct MpLifeRec *rec = MP_Life_Get(&gPlayerParty[a].box, TRUE);
        if (rec && b >= 0 && b < LIFE_NEEDS)
            rec->need[b] = c;
        printf("[test] life need %d %d %d\n", a, b, c);
    }
    else if (strcmp(cmd, "feel") == 0 && a >= 0 && a < PARTY_SIZE)
    {
        // feel SLOT EMOTION AMOUNT
        struct MpLifeRec *rec = MP_Life_Get(&gPlayerParty[a].box, TRUE);
        if (rec && b >= 0 && b < LIFE_EMOTIONS)
            rec->emo[b] = c;
        printf("[test] life feel %d %d %d\n", a, b, c);
    }
    else if (strcmp(cmd, "temper") == 0 && a >= 0 && a < PARTY_SIZE)
    {
        // temper SLOT AXIS VALUE
        struct MpLifeRec *rec = MP_Life_Get(&gPlayerParty[a].box, TRUE);
        if (rec && b >= 0 && b < LIFE_AXES)
            rec->cur[b] = rec->base[b] = c;
        printf("[test] life temper %d %d %d\n", a, b, c);
    }
    else if (strcmp(cmd, "verb") == 0)
    {
        // verb V: as if V was picked on the wheel (LIFE_VERBS = SIT WITH)
        void MP_LifeInteract_Debug(s8 verb);
        MP_LifeInteract_Debug(a);
        printf("[test] life verb %d\n", a);
    }
    else if (strcmp(cmd, "emote") == 0)
    {
        MP_LifeEmote_Show(LIFE_EMOTE_SLOT_LOCAL, a);
        printf("[test] life emote %d\n", a);
    }
    else if (strcmp(cmd, "act") == 0)
    {
        // act N: the follower's next activity is LIFE_ACT_N (if this place allows it)
        void MP_LifeField_DebugForce(u8 act);
        MP_LifeField_DebugForce(a);
        printf("[test] life act %d\n", a);
    }
    else if (strcmp(cmd, "field") == 0)
    {
        void MP_LifeField_Debug(void);
        MP_LifeField_Debug();
    }
    else if (strcmp(cmd, "garden") == 0)
    {
        MP_Garden_Debug(args + 7);
    }
    else if (strcmp(cmd, "egg") == 0 && a >= 0 && a < PARTY_SIZE && b >= 0 && b < PARTY_SIZE && a != b)
    {
        // egg A B: party POKéMON A and B make an EGG through the DAY CARE's own code (copies of them stand in
        // for its guests for a moment; the real DAY CARE is put back after), and the Trainer takes it
        void GiveEggFromDaycare(void);
        struct DayCare saved = gSaveBlock1Ptr->daycare;
        memset(&gSaveBlock1Ptr->daycare, 0, sizeof(gSaveBlock1Ptr->daycare));
        gSaveBlock1Ptr->daycare.mons[0].mon = gPlayerParty[a].box;
        gSaveBlock1Ptr->daycare.mons[1].mon = gPlayerParty[b].box;
        TriggerPendingDaycareEgg();
        GiveEggFromDaycare();
        gSaveBlock1Ptr->daycare = saved;
        printf("[test] life egg: party %u\n", CalculatePlayerPartyCount());
    }
    else if (strcmp(cmd, "familiar") == 0 && a >= 0 && a < PARTY_SIZE && b >= 0 && b < PARTY_SIZE)
    {
        // familiar A B V: how well A knows B
        struct MpLifeRec *ra = MP_Life_Get(&gPlayerParty[a].box, TRUE), *rb = MP_Life_Get(&gPlayerParty[b].box, TRUE);
        struct MpLifeBond *bd = (ra && rb) ? MP_Life_Bond(MP_Life_MonKey(ra->personality, ra->otId), MP_Life_MonKey(rb->personality, rb->otId), TRUE) : NULL;
        if (bd)
            bd->v[BOND_FAMILIAR] = c;
        printf("[test] life familiar %d -> %d %d\n", a, b, c);
    }
    else if (strcmp(cmd, "hatchsoon") == 0)
    {
        // hatchsoon: the party's EGGs are ready, and the next step hatches them
        u8 i, zero = 0;
        for (i = 0; i < PARTY_SIZE; i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
                SetMonData(&gPlayerParty[i], MON_DATA_FRIENDSHIP, &zero);
        gSaveBlock1Ptr->daycare.stepCounter = 254;
        printf("[test] life hatchsoon\n");
    }
    else if (strcmp(cmd, "evolve") == 0 && a >= 0 && a < PARTY_SIZE)
    {
        // evolve SLOT SPECIES: it becomes that species (the A-Life side of an evolution; no scene)
        u16 sp = b;
        SetMonData(&gPlayerParty[a], MON_DATA_SPECIES, &sp);
        CalculateMonStats(&gPlayerParty[a]);
        MP_Life_Get(&gPlayerParty[a].box, TRUE);
        printf("[test] life evolve %d -> %d\n", a, b);
    }
    else if (strcmp(cmd, "evoreact") == 0 && a >= 0 && a < PARTY_SIZE && b >= 0 && b < PARTY_SIZE)
    {
        // evoreact WATCHER EVOLVED: how WATCHER takes EVOLVED's new body
        u8 k = MP_Life_EvolutionReaction(MP_Life_Get(&gPlayerParty[a].box, TRUE), MP_Life_Get(&gPlayerParty[b].box, TRUE));
        printf("[test] life evoreact %d %d: %u\n", a, b, k);
    }
    else if (strcmp(cmd, "social") == 0)
    {
        MP_LifeSocial_Debug(args + 7);
    }
    else if (strcmp(cmd, "battle") == 0)
    {
        // battle lineforce 1 | battle state
        MP_LifeBattle_Debug(args + 7);
    }
    else if (strcmp(cmd, "bond") == 0 && a >= 0 && a < PARTY_SIZE && b >= 0 && b < PARTY_SIZE)
    {
        // bond A B FAMILIAR TRUST AFFECTION...: set A's bond with B (c = affection, d = trust)
        struct MpLifeRec *ra = MP_Life_Get(&gPlayerParty[a].box, TRUE), *rb = MP_Life_Get(&gPlayerParty[b].box, TRUE);
        struct MpLifeBond *bd = (ra && rb) ? MP_Life_Bond(MP_Life_MonKey(ra->personality, ra->otId), MP_Life_MonKey(rb->personality, rb->otId), TRUE) : NULL;
        if (bd)
        {
            bd->v[BOND_AFFECTION] = c;
            bd->v[BOND_TRUST] = d;
        }
        printf("[test] life bond %d -> %d affection %d trust %d\n", a, b, c, d);
    }
    else if (strcmp(cmd, "forget") == 0 && a >= 0 && a < PARTY_SIZE)
    {
        // forget SLOT KIND: its memories of that kind are gone (as if it hadn't been there)
        struct MpLifeRec *rec = MP_Life_Get(&gPlayerParty[a].box, TRUE);
        u8 i;
        for (i = 0; rec && i < LIFE_MEMORIES; i++)
            if (rec->mem[i].kind == b)
                memset(&rec->mem[i], 0, sizeof(rec->mem[i]));
        printf("[test] life forget %d kind %d\n", a, b);
    }
    else if (strcmp(cmd, "spread") == 0)
    {
        // spread: one telling now (a couple of minutes later than the news)
        sSpreadForce = TRUE;
        SpreadWord(MP_Life_PlayMinutes() + 2);
        printf("[test] life spread\n");
    }
    else if (strcmp(cmd, "wants") == 0)
    {
        // wants 1: it asks whenever it's eager (no 10-minute spells); wants 0: back to normal
        sWantsForce = a;
        printf("[test] life wants force %d\n", a);
    }
    else if (strcmp(cmd, "story") == 0)
    {
        printf("[test] life story mask %08x badges %02x news %u\n", (unsigned)sStoryMask, (unsigned)sBadgeMask, sNewsCount);
    }
    else if (strcmp(cmd, "count") == 0)
    {
        printf("[test] life count records %u bonds %u names %u\n", sRecCount, sBondCount, sNameCount);
    }
    else if (strcmp(cmd, "save") == 0)
    {
        MP_Life_WriteSidecar();
        printf("[test] life saved\n");
    }
    else if (strcmp(cmd, "load") == 0)
    {
        MP_Life_LoadFromSave();
        printf("[test] life loaded %u\n", sRecCount);
    }
    else if (strcmp(cmd, "reveal") == 0 && a >= 0 && a < PARTY_SIZE)
    {
        struct MpLifeRec *rec = MP_Life_Get(&gPlayerParty[a].box, TRUE);
        if (rec)
            rec->known[0] = rec->known[1] = 0xFFFFFFFF;
        printf("[test] life reveal %d\n", a);
    }
    else if (strcmp(cmd, "journal") == 0 && a >= 0 && a < PARTY_SIZE)
    {
        static u8 lines[LIFE_JOURNAL_LINES][LIFE_JOURNAL_TEXT];
        char ascii[160];
        u8 n = MP_Life_Journal(&gPlayerParty[a].box, lines, LIFE_JOURNAL_LINES), i;
        for (i = 0; i < n; i++)
        {
            MP_GameToAscii(ascii, lines[i], sizeof(ascii) - 1);
            printf("[test] journal %d: %s\n", a, ascii);
        }
        printf("[test] journal %d: %u lines\n", a, n);
    }
}

// ---------------------------------------------------------------------------
// Looking up bonds (the journal)

struct MpLifeBond *MP_Life_BondByLow(u32 from, u16 low)
{
    u16 i;
    for (i = 0; i < sBondCount; i++)
        if (sBonds[i].from == from && (u16)sBonds[i].to == low)
            return &sBonds[i];
    return NULL;
}

// Its closest friend among Pokémon, a rival, family, and a Trainer it loves more than its own (0 = none)
void MP_Life_Closest(u32 from, u32 *friendKey, u32 *rivalKey, u32 *familyKey, u32 *favoriteKey)
{
    u16 i;
    int bestFriend = 0, bestRival = 0, bestFav;
    struct MpLifeBond *owner = MP_Life_Bond(from, MP_Life_LocalTrainerKey(), FALSE);
    *friendKey = *rivalKey = *familyKey = *favoriteKey = 0;
    bestFav = owner ? MP_Life_BondWarmth(owner) : 0;
    for (i = 0; i < sBondCount; i++)
    {
        struct MpLifeBond *b = &sBonds[i];
        int w;
        if (b->from != from)
            continue;
        w = MP_Life_BondWarmth(b);
        if (b->flags & LIFE_BOND_TRAINER)
        {
            if (b->to != MP_Life_LocalTrainerKey() && w > bestFav + 40)
                bestFav = w, *favoriteKey = b->to;
            continue;
        }
        if ((b->flags & LIFE_BOND_FAMILY) && *familyKey == 0)
            *familyKey = b->to;
        if (b->v[BOND_AFFECTION] >= 150 && b->v[BOND_TRUST] >= 120 && b->v[BOND_FRICTION] < 60 && w > bestFriend)
            bestFriend = w, *friendKey = b->to;
        if (((b->flags & LIFE_BOND_RIVAL) || (b->v[BOND_RESPECT] >= 150 && b->v[BOND_FRICTION] >= 80))
         && b->v[BOND_RESPECT] + b->v[BOND_FRICTION] > bestRival)
            bestRival = b->v[BOND_RESPECT] + b->v[BOND_FRICTION], *rivalKey = b->to;
    }
}

// ---------------------------------------------------------------------------
// Asking to come out (docs 7): sometimes a party Pokémon would like to walk with its Trainer: a curious
// one near a place it likes, a social one who misses company, a lively one bursting to play. The cue
// changes now and then (in ten-minute spells) and saying no leaves no mark.
bool8 MP_Life_WantsOut(struct BoxPokemon *box)
{
    struct MpLifeRec *rec = MP_Life_Get(box, FALSE);
    struct MpLifePlace *p;
    bool8 liked, eager = FALSE;
    u32 spell;
    if (rec == NULL || rec->personality == gMpConfig.followerPersonality)
        return FALSE;
    p = MP_Life_Place(rec, MP_Life_CurrentPlace(), FALSE);
    liked = p && ((p->flags & LIFE_PLACE_FAVORITE) || MP_Life_FindTrait(rec, TRAIT_LOVES_PLACE, p->map) || p->level >= 150);
    if (rec->cur[LIFE_CURIOUS] >= 150 && (liked || rec->need[NEED_EXPLORE] >= 180))
        eager = TRUE;
    if (rec->cur[LIFE_SOCIAL] >= 170 && rec->need[NEED_COMPANY] >= 200)
        eager = TRUE;
    if (rec->cur[LIFE_LIVELY] >= 150 && rec->need[NEED_PLAY] >= 230)
        eager = TRUE;
    if (!eager)
        return FALSE;
    if (sWantsForce)
        return TRUE;
    spell = MP_Life_PlayMinutes() / 10;
    return ((rec->personality ^ (spell * 2654435761u)) >> 7) % 3 == 0;
}
