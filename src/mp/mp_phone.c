// Moemon Co-op: the phone link, the game's side (docs/COMPANION_APP.md, docs/PHONE_LINK.md). The Moemon Companion App
// pairs once by a QR code (or 6 letters) on OPTIONS > CO-OP > PHONE LINK; after that it finds the game on the local
// network and talks over sealed messages (phone_proto.c). The phone never writes save bytes: it asks for a snapshot and
// sends orders, which the game carries out with its own functions when it's free on the overworld (MP_IsLocalFree),
// else keeps them in order until then. "<save>.phone" holds the save's ID (random, never the name or Trainer ID) and
// the two phones it remembers with their keys.
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "mp.h"
#include "mp_region.h"
#include "battle.h"
#include "bg.h"
#include "coins.h"
#include "decompress.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "item.h"
#include "mail.h"
#include "malloc.h"
#include "main.h"
#include "menu.h"
#include "money.h"
#include "overworld.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "mp_version.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/map_types.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "mp_phone_net.h"
#include "evolution_scene.h"
#include "party_menu.h"
u8 MP_PhoneItemUse(u16 item);   // party_menu.c
bool8 MP_PhoneItemConsumed(u16 item);
u16 MP_PhoneTmMove(struct Pokemon *mon, u16 item, bool8 *knows);
bool8 MP_Poketch_PhoneAvailable(void);   // sinnoh/poketch.c
void MP_Poketch_PhoneTouch(s16 x, s16 y, bool8 down);
void MP_Poketch_PhoneApp(s8 dir);
const u8 *MP_Poketch_PhoneFrame(u8 *app, u8 *color, u8 *flags, u32 *unlocked);
void MP_Poketch_PhoneDone(void);
const u32 *MP_BoxMonFrontPal(struct BoxPokemon *box);   // pokemon.c
const u32 *MP_SpeciesFrontPal(u16 species);
#include "phone_proto.h"
#include "monocypher/monocypher.h"
#include "region_map.h"
#include "wild_encounter.h"
#include "qrcodegen/qrcodegen.h"
#include "../data/mp_phone_nopc.h"

extern char gPlatformSavePath[512];

#define PHONE_MAGIC   0x4E48504D   // "MPHN"
#define PHONE_VERSION 1
#define PHONES_MAX    2

struct PhoneRecord
{
    u8 id[PHONE_ID_SIZE];
    u8 key[PHONE_KEY_SIZE];
    char name[24];
    u32 paired;          // (time() when it paired: the oldest goes when a third pairs)
};

struct PhoneFile
{
    u32 magic, version;
    u8 saveId[PHONE_SAVEID_SIZE];
    u32 count;
    struct PhoneRecord phones[PHONES_MAX];
};

static struct PhoneFile sFile;
static bool8 sHaveFile;          // sFile is this save's
static bool8 sDirty;             // write it with the next save

static void SpawnsLoad(void);
static void SpawnsWrite(void);
static void SpawnsReset(void);
struct Buf;
static void SendBuf(int c, struct Buf *b);

// --------------------------------------------------------------------------------------------------- the sidecar

static void Path(char *out, size_t size)
{
    snprintf(out, size, "%s.phone", gPlatformSavePath);
}

static void NewSaveId(void)
{
    memset(&sFile, 0, sizeof(sFile));
    sFile.magic = PHONE_MAGIC;
    sFile.version = PHONE_VERSION;
    PhoneProto_Random(sFile.saveId, sizeof(sFile.saveId));
    sHaveFile = TRUE;
    sDirty = TRUE;
}

static void WriteNow(void)
{
    char path[600];
    FILE *f;
    Path(path, sizeof(path));
    f = fopen(path, "wb");
    if (f == NULL)
        return;
    fwrite(&sFile, sizeof(sFile), 1, f);
    fclose(f);
    sDirty = FALSE;
}

// save.c LoadGameSave: this save's ID and phones (a save without the file gets an ID, written with its next save)
void MP_Phone_OnSaveLoaded(void)
{
    char path[600];
    FILE *f;
    Path(path, sizeof(path));
    f = fopen(path, "rb");
    sHaveFile = FALSE;
    if (f != NULL)
    {
        if (fread(&sFile, sizeof(sFile), 1, f) == 1 && sFile.magic == PHONE_MAGIC && sFile.version == PHONE_VERSION
         && sFile.count <= PHONES_MAX)
        {
            sHaveFile = TRUE;
            sDirty = FALSE;
        }
        fclose(f);
    }
    if (!sHaveFile)
        NewSaveId();
    SpawnsLoad();
}

// new_game.c: a new game is a different save, even in the same slot: its own ID and no phones
void MP_Phone_NewGame(void)
{
    NewSaveId();
    SpawnsReset();
}

// sdl2.c StoreSaveFile
void MP_Phone_WriteSidecar(void)
{
    if (sHaveFile && sDirty)
        WriteNow();
    SpawnsWrite();
}

static void EnsureFile(void)
{
    if (!sHaveFile)
        MP_Phone_OnSaveLoaded();
}

// ------------------------------------------------------------------------------------------------ JSON, in and out

struct Buf
{
    char *p;
    size_t len, cap;
};

static struct Buf sOut;

static void BufReset(struct Buf *b)
{
    b->len = 0;
    if (b->p)
        b->p[0] = '\0';
}

static void BufAdd(struct Buf *b, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void BufAdd(struct Buf *b, const char *fmt, ...)
{
    va_list ap;
    int n;
    for (;;)
    {
        size_t room = b->cap - b->len;
        va_start(ap, fmt);
        n = b->p ? vsnprintf(b->p + b->len, room, fmt, ap) : -1;
        va_end(ap);
        if (b->p && n >= 0 && (size_t)n < room)
        {
            b->len += n;
            return;
        }
        {
            size_t cap = b->cap ? b->cap * 2 : 65536;
            char *p;
            while (n >= 0 && cap < b->len + n + 1)
                cap *= 2;
            p = realloc(b->p, cap);
            if (p == NULL)
                return;
            b->p = p;
            b->cap = cap;
        }
    }
}

// The game's text bytes as hex (names: the app draws them in the game's font)
static void BufText(struct Buf *b, const u8 *s, size_t max)
{
    size_t i;
    BufAdd(b, "\"");
    for (i = 0; i < max && s[i] != EOS; i++)
        BufAdd(b, "%02X", s[i]);
    BufAdd(b, "\"");
}

// A value in a flat JSON object: "key": NUMBER (also inside one nested object found by JsonObj)
static const char *JsonKey(const char *json, const char *key)
{
    char pat[40];
    const char *p;
    snprintf(pat, sizeof(pat), "\"%s\"", key);
    p = strstr(json, pat);
    if (p == NULL)
        return NULL;
    p += strlen(pat);
    while (*p == ' ' || *p == ':')
        p++;
    return p;
}

static bool8 JsonNum(const char *json, const char *key, long long *out)
{
    const char *p = JsonKey(json, key);
    char *end;
    long long v;
    if (p == NULL)
        return FALSE;
    v = strtoll(p, &end, 10);
    if (end == p)
        return FALSE;
    *out = v;
    return TRUE;
}

static bool8 JsonStr(const char *json, const char *key, char *out, size_t size)
{
    const char *p = JsonKey(json, key);
    size_t n = 0;
    if (p == NULL || *p != '"')
        return FALSE;
    p++;
    while (*p && *p != '"' && n + 1 < size)
        out[n++] = *p++;
    out[n] = '\0';
    return *p == '"';
}

// The {...} after "key": copied out, so the keys inside are looked up apart from the others
static bool8 JsonObj(const char *json, const char *key, char *out, size_t size)
{
    const char *p = JsonKey(json, key), *q;
    if (p == NULL || *p != '{')
        return FALSE;
    q = strchr(p, '}');
    if (q == NULL || (size_t)(q - p + 1) >= size)
        return FALSE;
    memcpy(out, p, q - p + 1);
    out[q - p + 1] = '\0';
    return TRUE;
}

// ------------------------------------------------------------------------------------------ what the phone sees

static struct PokemonStorage *Storage(void)
{
    return MP_PersonalStorage();
}

static u16 Quantity(const struct ItemSlot *slot)
{
    return slot->quantity ^ gSaveBlock2Ptr->encryptionKey;
}

static void BufBoxMon(struct Buf *b, struct BoxPokemon *box)
{
    u16 species = GetBoxMonData(box, MON_DATA_SPECIES);
    u32 pid, ot, i;
    u8 nick[POKEMON_NAME_LENGTH + 1];
    if (species == SPECIES_NONE)
    {
        BufAdd(b, "0");
        return;
    }
    pid = GetBoxMonData(box, MON_DATA_PERSONALITY);
    ot = GetBoxMonData(box, MON_DATA_OT_ID);
    GetBoxMonData(box, MON_DATA_NICKNAME, nick);
    nick[POKEMON_NAME_LENGTH] = EOS;
    BufAdd(b, "{\"s\":%u,\"e\":%u,\"p\":%u,\"o\":%u,\"l\":%u,\"i\":%u,\"g\":%u,\"sh\":%u,\"nat\":%u,\"ab\":%u,\"ball\":%u,"
           "\"f\":%u,\"n\":", species, (unsigned)GetBoxMonData(box, MON_DATA_IS_EGG), (unsigned)pid, (unsigned)ot,
           GetLevelFromBoxMonExp(box), (unsigned)GetBoxMonData(box, MON_DATA_HELD_ITEM), GetBoxMonGender(box),
           IsShinyOtIdPersonality(ot, pid), GetNatureFromPersonality(pid),
           (unsigned)GetAbilityBySpecies(species, GetBoxMonData(box, MON_DATA_ABILITY_NUM)),
           (unsigned)GetBoxMonData(box, MON_DATA_POKEBALL), (unsigned)GetBoxMonData(box, MON_DATA_FRIENDSHIP));
    BufText(b, nick, POKEMON_NAME_LENGTH);
    if (!GetBoxMonData(box, MON_DATA_IS_EGG))
    {
        // (shiny, or its Stadium hue: the front picture's 16 colors as the game shows them, when not the species')
        u16 pal[16], own[16];
        LZDecompressWram(MP_BoxMonFrontPal(box), pal);
        LZDecompressWram(MP_SpeciesFrontPal(species), own);
        if (memcmp(pal, own, sizeof(pal)) != 0)
        {
            BufAdd(b, ",\"pal\":\"");
            for (i = 0; i < 16; i++)
                BufAdd(b, "%04X", pal[i]);
            BufAdd(b, "\"");
        }
    }
    BufAdd(b, ",\"m\":[");
    for (i = 0; i < MAX_MON_MOVES; i++)
        BufAdd(b, "%s%u", i ? "," : "", (unsigned)GetBoxMonData(box, MON_DATA_MOVE1 + i));
    BufAdd(b, "]");
    if (!GetBoxMonData(box, MON_DATA_IS_EGG))
    {
        // the summary's pages: stats (HP, ATTACK, DEFENSE, SP. ATK, SP. DEF, SPEED, as the game works them out), EXP
        // (now, this level's start, the next's), PP, where and at what level it was met, its Trainer
        static struct Pokemon sTmp;
        u8 ot[PLAYER_NAME_LENGTH + 1];
        u32 lvl = GetLevelFromBoxMonExp(box), exp = GetBoxMonData(box, MON_DATA_EXP);
        u8 growth = gSpeciesInfo[species].growthRate;
        BoxMonToMon(box, &sTmp);
        BufAdd(b, ",\"stats\":[%u,%u,%u,%u,%u,%u]", (unsigned)GetMonData(&sTmp, MON_DATA_MAX_HP),
               (unsigned)GetMonData(&sTmp, MON_DATA_ATK), (unsigned)GetMonData(&sTmp, MON_DATA_DEF),
               (unsigned)GetMonData(&sTmp, MON_DATA_SPATK), (unsigned)GetMonData(&sTmp, MON_DATA_SPDEF),
               (unsigned)GetMonData(&sTmp, MON_DATA_SPEED));
        BufAdd(b, ",\"xp\":[%u,%u,%u]", (unsigned)exp, (unsigned)gExperienceTables[growth][lvl],
               (unsigned)gExperienceTables[growth][lvl < MAX_LEVEL ? lvl + 1 : lvl]);
        BufAdd(b, ",\"pp\":[%u,%u,%u,%u]", (unsigned)GetBoxMonData(box, MON_DATA_PP1),
               (unsigned)GetBoxMonData(box, MON_DATA_PP2), (unsigned)GetBoxMonData(box, MON_DATA_PP3),
               (unsigned)GetBoxMonData(box, MON_DATA_PP4));
        BufAdd(b, ",\"ppb\":%u", (unsigned)GetBoxMonData(box, MON_DATA_PP_BONUSES));
        BufAdd(b, ",\"met\":[%u,%u]", (unsigned)GetBoxMonData(box, MON_DATA_MET_LOCATION),
               (unsigned)GetBoxMonData(box, MON_DATA_MET_LEVEL));
        GetBoxMonData(box, MON_DATA_OT_NAME, ot);
        ot[PLAYER_NAME_LENGTH] = EOS;
        BufAdd(b, ",\"otn\":");
        BufText(b, ot, PLAYER_NAME_LENGTH);
        BufAdd(b, ",\"otg\":%u", (unsigned)GetBoxMonData(box, MON_DATA_OT_GENDER));
    }
    BufAdd(b, "}");
}

static void BufPartyMon(struct Buf *b, struct Pokemon *mon)
{
    size_t at = b->len;
    BufBoxMon(b, &mon->box);
    if (b->len > at && b->p[b->len - 1] == '}')
    {
        b->len--;
        BufAdd(b, ",\"hp\":%u,\"mhp\":%u,\"st\":%u}", (unsigned)GetMonData(mon, MON_DATA_HP),
               (unsigned)GetMonData(mon, MON_DATA_MAX_HP), (unsigned)GetMonData(mon, MON_DATA_STATUS));
    }
}

static bool8 PcReachable(void)
{
    u16 here = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i;
    switch (gMapHeader.mapType)
    {
    case MAP_TYPE_TOWN: case MAP_TYPE_CITY: case MAP_TYPE_ROUTE: case MAP_TYPE_OCEAN_ROUTE: case MAP_TYPE_INDOOR:
        break;
    default:
        return FALSE;   // (caves, the Underground, under water, secret bases)
    }
    for (i = 0; i < ARRAY_COUNT(sPhoneNoPcMaps); i++)
        if (sPhoneNoPcMaps[i] == here)
            return FALSE;
    return TRUE;
}

// The player's own party: in a co-op or multi battle gPlayerParty holds friends' POKeMON too, and the real party is
// set aside in the save block (mp_coop.c SavePlayerParty) until the battle ends
static struct Pokemon *OwnParty(void)
{
    if (gMain.inBattle && (gMpCoopBattle || (gBattleTypeFlags & BATTLE_TYPE_MULTI)))
        return gSaveBlock1Ptr->playerParty;
    return gPlayerParty;
}

static bool8 Safe(void)
{
    return MP_IsLocalFree() && !gMain.inBattle;
}

static void BufPlace(struct Buf *b)
{
    BufAdd(b, "\"place\":{\"map\":[%u,%u],\"type\":%u,\"pc\":%u,\"safe\":%u,\"battle\":%u}",
           gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, gMapHeader.mapType, PcReachable(), Safe(),
           gMain.inBattle != 0);
}

// A hash of everything the snapshot shows that can change (party, boxes, BAG, money, POKeDEX): "rev" in "snap" and
// "state", so the app asks for a new snapshot only when the save changed (by hand in the game, too)
static u32 Fnv(u32 h, const void *data, u32 n)
{
    const u8 *p = data;
    while (n--)
        h = (h ^ *p++) * 16777619u;
    return h;
}

static u32 Rev(void)
{
    u32 h = 2166136261u, p, i, v;
    h = Fnv(h, OwnParty(), sizeof(gPlayerParty));
    h = Fnv(h, Storage(), sizeof(struct PokemonStorage));
    for (p = 0; p < POCKETS_COUNT; p++)
        for (i = 0; i < gBagPockets[p].capacity; i++)
        {
            v = (gBagPockets[p].itemSlots[i].itemId << 16) | Quantity(&gBagPockets[p].itemSlots[i]);
            h = Fnv(h, &v, 4);
        }
    v = GetMoney(&gSaveBlock1Ptr->money);
    h = Fnv(h, &v, 4);
    v = GetCoins();
    h = Fnv(h, &v, 4);
    for (i = 1; i <= NATIONAL_DEX_COUNT; i++)
    {
        v = GetSetPokedexFlag(i, FLAG_GET_SEEN) | (GetSetPokedexFlag(i, FLAG_GET_CAUGHT) << 1);
        h = Fnv(h, &v, 1);
    }
    return h;
}

static void BufBag(struct Buf *b)
{
    u32 p, i;
    BufAdd(b, "\"bag\":[");
    for (p = 0; p < POCKETS_COUNT; p++)
    {
        bool8 first = TRUE;
        BufAdd(b, "%s[", p ? "," : "");
        for (i = 0; i < gBagPockets[p].capacity; i++)
        {
            const struct ItemSlot *s = &gBagPockets[p].itemSlots[i];
            if (s->itemId == ITEM_NONE)
                continue;
            BufAdd(b, "%s[%u,%u]", first ? "" : ",", s->itemId, Quantity(s));
            first = FALSE;
        }
        BufAdd(b, "]");
    }
    BufAdd(b, "]");
}

static void BufSnapshot(struct Buf *b)
{
    struct PokemonStorage *st = Storage();
    u32 i, j, n;
    char id[2 * PHONE_SAVEID_SIZE + 1];
    PhoneProto_ToHex(sFile.saveId, PHONE_SAVEID_SIZE, id);
    BufAdd(b, "{\"t\":\"snap\",\"rev\":%u,\"save\":{\"id\":\"%s\",\"name\":", (unsigned)Rev(), id);
    BufText(b, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    // (the devices the save has, for the phone's DEVICE page: POKéTCH, POKéNAV, POKéGEAR)
    BufAdd(b, ",\"devices\":[%u,%u,%u]", MP_Poketch_PhoneAvailable(), FlagGet(FLAG_SYS_POKENAV_GET) != 0,
           CheckBagHasItem(ITEM_POKEGEAR, 1) != 0);
    BufAdd(b, ",\"tid\":%u,\"gender\":%u,\"region\":%u,\"gen\":%u,\"badges\":%u,\"money\":%u,\"coins\":%u,\"game\":\"%s\"},",
           (unsigned)(gSaveBlock2Ptr->playerTrainerId[0] | (gSaveBlock2Ptr->playerTrainerId[1] << 8)),
           gSaveBlock2Ptr->playerGender, MP_Region_Current(), MP_Generation(), MP_Region_TotalBadges(),
           (unsigned)GetMoney(&gSaveBlock1Ptr->money), GetCoins(), MOEMON_VERSION);
    BufPlace(b);
    BufAdd(b, ",\"party\":[");
    for (i = 0, n = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&OwnParty()[i], MON_DATA_SPECIES) == SPECIES_NONE)
            continue;
        BufAdd(b, "%s", n++ ? "," : "");
        BufPartyMon(b, &OwnParty()[i]);
    }
    BufAdd(b, "],\"box\":%u,\"boxes\":[", st->currentBox);
    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
    {
        BufAdd(b, "%s{\"n\":", i ? "," : "");
        BufText(b, st->boxNames[i], BOX_NAME_LENGTH);
        BufAdd(b, ",\"w\":%u,\"m\":[", st->boxWallpapers[i]);
        for (j = 0; j < IN_BOX_COUNT; j++)
        {
            BufAdd(b, "%s", j ? "," : "");
            BufBoxMon(b, &st->boxes[i][j]);
        }
        BufAdd(b, "]}");
    }
    BufAdd(b, "],");
    BufBag(b);
    // The POKeDEX: a bit per national number (1 first), as hex: seen, then caught
    {
        static const char *const sWhat[] = {"seen", "caught"};
        u32 k, count = NATIONAL_DEX_COUNT;
        BufAdd(b, ",\"dex\":{\"count\":%u", (unsigned)count);
        for (k = 0; k < 2; k++)
        {
            BufAdd(b, ",\"%s\":\"", sWhat[k]);
            for (i = 0; i < count; i += 4)
            {
                u32 nib = 0;
                for (j = 0; j < 4; j++)
                    if (i + j < count && GetSetPokedexFlag(i + j + 1, k ? FLAG_GET_CAUGHT : FLAG_GET_SEEN))
                        nib |= 8 >> j;
                BufAdd(b, "%X", nib);
            }
            BufAdd(b, "\"");
        }
        BufAdd(b, "}");
    }
    BufAdd(b, "}");
}

// ------------------------------------------------------------------------------------------------------- orders

enum { LOC_NONE, LOC_PARTY, LOC_BOX };
struct Loc { u8 kind, box, slot; };

static bool8 ParseLoc(const char *json, const char *key, struct Loc *loc)
{
    char obj[96];
    long long v;
    loc->kind = LOC_NONE;
    if (!JsonObj(json, key, obj, sizeof(obj)))
        return FALSE;
    if (JsonNum(obj, "party", &v))
    {
        if (v < 0 || v >= PARTY_SIZE)
            return FALSE;
        loc->kind = LOC_PARTY;
        loc->slot = v;
        return TRUE;
    }
    if (JsonNum(obj, "box", &v) && v >= 0 && v < TOTAL_BOXES_COUNT)
    {
        loc->box = v;
        if (!JsonNum(obj, "slot", &v) || v < 0 || v >= IN_BOX_COUNT)
            return FALSE;
        loc->kind = LOC_BOX;
        loc->slot = v;
        return TRUE;
    }
    return FALSE;
}

static struct BoxPokemon *BoxAt(const struct Loc *loc)
{
    if (loc->kind == LOC_PARTY)
        return &gPlayerParty[loc->slot].box;
    return &Storage()->boxes[loc->box][loc->slot];
}

static bool8 Empty(const struct Loc *loc)
{
    if (loc->kind == LOC_PARTY && loc->slot >= gPlayerPartyCount)
        return TRUE;
    return GetBoxMonData(BoxAt(loc), MON_DATA_SPECIES) == SPECIES_NONE;
}

// The POKeMON there is the one the phone means: same personality and Trainer ID (0, 0: the place is empty)
static bool8 Is(const struct Loc *loc, u32 pid, u32 ot)
{
    if (Empty(loc))
        return pid == 0 && ot == 0;
    return GetBoxMonData(BoxAt(loc), MON_DATA_PERSONALITY) == pid && GetBoxMonData(BoxAt(loc), MON_DATA_OT_ID) == ot;
}

static u32 NonEggsInParty(void)
{
    u32 i, n = 0;
    for (i = 0; i < gPlayerPartyCount; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            n++;
    return n;
}

#define CHANGED_MAX 4
static struct Loc sChanged[CHANGED_MAX];
static u8 sChangedCount;
static bool8 sBagChanged;

static void Changed(const struct Loc *loc)
{
    if (sChangedCount < CHANGED_MAX)
        sChanged[sChangedCount++] = *loc;
}

// Party slots shift when one empties: the whole party is sent again
static void PartyChanged(void)
{
    struct Loc l = {LOC_PARTY, 0, 0xFF};
    Changed(&l);
}

static const char *DoMove(const char *json)
{
    struct Loc from, to;
    long long pid, ot, tpid = 0, tot = 0;
    bool8 fromEmpty, toEmpty;
    if (!ParseLoc(json, "from", &from) || !ParseLoc(json, "to", &to) || !JsonNum(json, "p", &pid) || !JsonNum(json, "o", &ot))
        return "bad";
    JsonNum(json, "tp", &tpid);
    JsonNum(json, "to2", &tot);
    if (from.kind == to.kind && from.box == to.box && from.slot == to.slot)
        return "same";
    if ((from.kind == LOC_PARTY || to.kind == LOC_PARTY) && !PcReachable())
        return "nopc";
    if (!Is(&from, (u32)pid, (u32)ot))
        return "gone";
    if (!Is(&to, (u32)tpid, (u32)tot))
        return "changed";
    fromEmpty = Empty(&from);
    toEmpty = Empty(&to);
    if (fromEmpty)
        return "gone";
    if (from.kind == LOC_BOX && to.kind == LOC_BOX)
    {
        struct BoxPokemon tmp = *BoxAt(&to);
        *BoxAt(&to) = *BoxAt(&from);
        *BoxAt(&from) = tmp;
        Changed(&from);
        Changed(&to);
        return NULL;
    }
    if (from.kind == LOC_PARTY && to.kind == LOC_PARTY)
    {
        struct Pokemon tmp;
        if (toEmpty)
            to.slot = gPlayerPartyCount - 1;   // (to the end: a party has no gaps)
        tmp = gPlayerParty[to.slot];
        gPlayerParty[to.slot] = gPlayerParty[from.slot];
        gPlayerParty[from.slot] = tmp;
        PartyChanged();
        return NULL;
    }
    if (from.kind == LOC_PARTY)   // into the PC
    {
        struct BoxPokemon *dst = BoxAt(&to);
        struct Pokemon *mon = &gPlayerParty[from.slot];
        bool8 incomingEgg = !toEmpty && GetBoxMonData(dst, MON_DATA_IS_EGG);
        if (!GetMonData(mon, MON_DATA_IS_EGG) && NonEggsInParty() <= 1 && (toEmpty || incomingEgg))
            return "last";
        if (toEmpty)
        {
            *dst = mon->box;
            ZeroMonData(mon);
            CompactPartySlots();
            CalculatePlayerPartyCount();
        }
        else
        {
            struct Pokemon in;
            BoxMonToMon(dst, &in);
            *dst = mon->box;
            *mon = in;
        }
        PartyChanged();
        Changed(&to);
        return NULL;
    }
    // Out of the PC into the party
    {
        struct BoxPokemon *src = BoxAt(&from);
        if (toEmpty)
        {
            if (gPlayerPartyCount >= PARTY_SIZE)
                return "full";
            BoxMonToMon(src, &gPlayerParty[gPlayerPartyCount]);
            ZeroBoxMonData(src);
            CalculatePlayerPartyCount();
        }
        else
        {
            struct Pokemon *mon = &gPlayerParty[to.slot], in;
            if (!GetMonData(mon, MON_DATA_IS_EGG) && NonEggsInParty() <= 1 && GetBoxMonData(src, MON_DATA_IS_EGG))
                return "last";
            BoxMonToMon(src, &in);
            *src = mon->box;
            *mon = in;
        }
        PartyChanged();
        Changed(&from);
        return NULL;
    }
}

static const char *DoItem(const char *json, bool8 give)
{
    struct Loc at;
    long long pid, ot, item = 0;
    struct BoxPokemon *box;
    u16 held;
    if (!ParseLoc(json, "at", &at) || !JsonNum(json, "p", &pid) || !JsonNum(json, "o", &ot) || Empty(&at))
        return "bad";
    if (!Is(&at, (u32)pid, (u32)ot))
        return "gone";
    box = BoxAt(&at);
    if (GetBoxMonData(box, MON_DATA_IS_EGG))
        return "egg";
    held = GetBoxMonData(box, MON_DATA_HELD_ITEM);
    if (held != ITEM_NONE && ItemIsMail(held))
        return "mail";
    if (give)
    {
        u16 newItem;
        if (!JsonNum(json, "item", &item) || item <= 0 || item >= ITEMS_COUNT)
            return "bad";
        newItem = item;
        if (!CheckBagHasItem(newItem, 1))
            return "noitem";
        if (!MP_ItemCanBeHeld(newItem) || ItemIsMail(newItem))
            return "cantheld";
        if (held != ITEM_NONE && !CheckBagHasSpace(held, 1))
            return "bagfull";
        RemoveBagItem(newItem, 1);
        if (held != ITEM_NONE)
            AddBagItem(held, 1);
        SetBoxMonData(box, MON_DATA_HELD_ITEM, &newItem);
    }
    else
    {
        u16 none = ITEM_NONE;
        if (held == ITEM_NONE)
            return "nothing";
        if (!AddBagItem(held, 1))
            return "bagfull";
        SetBoxMonData(box, MON_DATA_HELD_ITEM, &none);
    }
    Changed(&at);
    sBagChanged = TRUE;
    return NULL;
}

// ------------------------------------------------------------------------------------------------ the spawn viewer

// Which wild slots the player has met, per map: "<save>.spawns" (a map's 32 bits: land 0-11, water 12-16, ROCK SMASH
// 17-21, the rods 22-31; the app hides a rare one until it's met there that way: docs/COMPANION_APP.md)
#define SPAWN_MAPS_MAX 1200
#define SPAWN_MAGIC 0x4E57504D   // "MPWN"
struct SpawnMet
{
    u16 map;
    u16 pad;
    u32 bits;
};
static struct SpawnMet sMet[SPAWN_MAPS_MAX];
static u32 sMetCount;
static bool8 sMetDirty;
static const u8 sAreaBit[4] = {0, 12, 17, 22};
static const u8 sAreaSlots[4] = {12, 5, 5, 10};

static void SpawnPath(char *out, size_t size)
{
    snprintf(out, size, "%s.spawns", gPlatformSavePath);
}

static void SpawnsLoad(void)
{
    char path[600];
    FILE *f;
    u32 magic = 0, n = 0;
    sMetCount = 0;
    sMetDirty = FALSE;
    SpawnPath(path, sizeof(path));
    f = fopen(path, "rb");
    if (f == NULL)
        return;
    if (fread(&magic, 4, 1, f) == 1 && magic == SPAWN_MAGIC && fread(&n, 4, 1, f) == 1 && n <= SPAWN_MAPS_MAX)
        sMetCount = fread(sMet, sizeof(sMet[0]), n, f);
    fclose(f);
}

static void SpawnsReset(void)
{
    sMetCount = 0;
    sMetDirty = TRUE;
}

static void SpawnsWrite(void)
{
    char path[600];
    FILE *f;
    u32 magic = SPAWN_MAGIC;
    if (!sMetDirty)
        return;
    SpawnPath(path, sizeof(path));
    f = fopen(path, "wb");
    if (f == NULL)
        return;
    fwrite(&magic, 4, 1, f);
    fwrite(&sMetCount, 4, 1, f);
    fwrite(sMet, sizeof(sMet[0]), sMetCount, f);
    fclose(f);
    sMetDirty = FALSE;
}

static u32 MetBits(u16 map)
{
    u32 i;
    for (i = 0; i < sMetCount; i++)
        if (sMet[i].map == map)
            return sMet[i].bits;
    return 0;
}

// wild_encounter.c: a wild POKeMON from this map's AREA (land, water, ROCK SMASH, the rods) in SLOT
void MP_Phone_SpawnMet(u8 area, u8 slot)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i, bit;
    if (area > 3 || slot >= sAreaSlots[area])
        return;
    bit = 1u << (sAreaBit[area] + slot);
    for (i = 0; i < sMetCount && sMet[i].map != map; i++)
        ;
    if (i == sMetCount)
    {
        if (sMetCount >= SPAWN_MAPS_MAX)
            return;
        sMet[sMetCount].map = map;
        sMet[sMetCount].bits = 0;
        sMetCount++;
    }
    if (!(sMet[i].bits & bit))
    {
        sMet[i].bits |= bit;
        sMetDirty = TRUE;
    }
}

static void BufMapName(struct Buf *b, u8 group, u8 num)
{
    u8 name[40];
    const struct MapHeader *h = Overworld_GetMapHeaderByGroupAndId(group, num);
    name[0] = EOS;
    if (h != NULL)
        GetMapNameGeneric(name, h->regionMapSectionId);
    BufText(b, name, sizeof(name) - 1);
}

// Every map with wild POKeMON: which ways they're found and how many of its slots were met
static void SendSpawnMaps(int c)
{
    u32 i, n = 0;
    BufReset(&sOut);
    BufAdd(&sOut, "{\"t\":\"spawnmaps\",\"maps\":[");
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        const struct WildPokemonHeader *h = &gWildMonHeaders[i];
        u16 map = (h->mapGroup << 8) | h->mapNum;
        BufAdd(&sOut, "%s{\"map\":[%u,%u],\"name\":", n++ ? "," : "", h->mapGroup, h->mapNum);
        BufMapName(&sOut, h->mapGroup, h->mapNum);
        BufAdd(&sOut, ",\"has\":[%u,%u,%u,%u],\"met\":%u,\"region\":%u}", h->landMonsInfo != NULL,
               h->waterMonsInfo != NULL, h->rockSmashMonsInfo != NULL, h->fishingMonsInfo != NULL, (unsigned)MetBits(map),
               MP_Region_OfGroup(h->mapGroup));
    }
    BufAdd(&sOut, "]}");
    SendBuf(c, &sOut);
}

// One map's slots as the game would make them now (the GENERATION's pools: MP_Region_WildInfo, MP_WildSlotSpecies)
static void SendSpawns(int c, const char *json)
{
    long long g = -1, m = -1;
    const char *p = JsonKey(json, "map");
    u32 i, k, met;
    const struct WildPokemonHeader *h = NULL;
    if (p != NULL && *p == '[')
        sscanf(p, "[%lld,%lld]", &g, &m);
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
        if (gWildMonHeaders[i].mapGroup == g && gWildMonHeaders[i].mapNum == m)
            h = &gWildMonHeaders[i];
    BufReset(&sOut);
    BufAdd(&sOut, "{\"t\":\"spawns\",\"map\":[%lld,%lld]", g, m);
    if (h != NULL)
    {
        const struct WildPokemonInfo *infos[4] = {h->landMonsInfo, h->waterMonsInfo, h->rockSmashMonsInfo, h->fishingMonsInfo};
        static const char *const sNames[4] = {"land", "water", "rock", "fish"};
        met = MetBits((g << 8) | m);
        BufAdd(&sOut, ",\"name\":");
        BufMapName(&sOut, g, m);
        for (k = 0; k < 4; k++)
        {
            const struct WildPokemonInfo *info = infos[k] ? MP_Region_WildInfo(infos[k]) : NULL;
            if (info == NULL)
                continue;
            BufAdd(&sOut, ",\"%s\":{\"rate\":%u,\"slots\":[", sNames[k], info->encounterRate);
            for (i = 0; i < sAreaSlots[k]; i++)
                BufAdd(&sOut, "%s[%u,%u,%u,%u]", i ? "," : "", MP_WildSlotSpecies(info, i), info->wildPokemon[i].minLevel,
                       info->wildPokemon[i].maxLevel, (met >> (sAreaBit[k] + i)) & 1);
            BufAdd(&sOut, "]}");
        }
    }
    BufAdd(&sOut, "}");
    SendBuf(c, &sOut);
}

// --------------------------------------------------------------------------------------------------- sessions

enum { SESS_NONE, SESS_NEW, SESS_LIVE };

struct Session
{
    u8 state;
    s8 phone;                       // its record in sFile
    u8 key[PHONE_KEY_SIZE];         // the session's key
    u32 seqIn, seqOut;
    bool8 watch;                    // the phone shows the POKéTCH: its LCD is sent when it changes
    u32 watchHash;
    u8 watchTimer;
};

#define ORDERS_MAX 64
#define ORDER_LEN  384
struct Order
{
    u8 conn;
    u32 id;
    char json[ORDER_LEN];
};

#define DONE_MAX 64
struct Done
{
    s8 phone;
    u32 id;
    char why[12];   // "" when it was done
};

static struct Session sSess[PHONE_NET_CONNS];
static struct Order sOrders[ORDERS_MAX];
static u8 sOrderCount;
static struct Done sDone[DONE_MAX];
static u8 sDoneNext;
static int sPort = -1;
static bool8 sStarted;
static u8 *sSealBuf;
static size_t sSealCap;
static u32 sApplied;   // orders carried out, for the corner's phone icon and the tests

// Pairing (the PHONE LINK screen open)
static bool8 sPairing;
static u8 sPairRaw[PHONE_QR_SECRET_SIZE], sPairSecret[PHONE_KEY_SIZE], sCodeSecret[PHONE_KEY_SIZE];
static char sPairCode[PHONE_CODE_LEN + 1];
static u8 sPairFails;
static char sJustPaired[24];

static void SendPlain(int c, const char *json)
{
    size_t n = strlen(json);
    u8 *body = malloc(n + 1);
    if (body == NULL)
        return;
    body[0] = PHONE_BODY_PLAIN;
    memcpy(body + 1, json, n);
    PhoneNet_Send(c, body, n + 1);
    free(body);
}

static void SendSealed(int c, const char *json, size_t n)
{
    struct Session *s = &sSess[c];
    size_t need = 1 + n + PHONE_SEAL_EXTRA;
    if (s->state != SESS_LIVE)
        return;
    if (need > sSealCap)
    {
        u8 *p = realloc(sSealBuf, need * 2);
        if (p == NULL)
            return;
        sSealBuf = p;
        sSealCap = need * 2;
    }
    sSealBuf[0] = PHONE_BODY_SEALED;
    PhoneProto_Seal(s->key, ++s->seqOut, json, n, sSealBuf + 1);
    PhoneNet_Send(c, sSealBuf, need);
}

static void SendBuf(int c, struct Buf *b)
{
    SendSealed(c, b->p, b->len);
}

static void Bye(int c, const char *why)
{
    char json[96];
    snprintf(json, sizeof(json), "{\"t\":\"bye\",\"why\":\"%s\",\"v\":%d}", why, PHONE_LINK_VERSION);
    SendPlain(c, json);
    MPNet_Log("phone: conn %d bye (%s)", c, why);
    sSess[c].state = SESS_NONE;
}

static void SendSnapshot(int c)
{
    BufReset(&sOut);
    BufSnapshot(&sOut);
    SendBuf(c, &sOut);
}

static void SendState(int c)
{
    BufReset(&sOut);
    BufAdd(&sOut, "{\"t\":\"state\",\"rev\":%u,\"queued\":%u,\"applied\":%u,", (unsigned)Rev(), sOrderCount,
           (unsigned)sApplied);
    BufPlace(&sOut);
    BufAdd(&sOut, "}");
    SendBuf(c, &sOut);
}

static struct Done *FindDone(s8 phone, u32 id)
{
    u32 i;
    for (i = 0; i < DONE_MAX; i++)
        if (sDone[i].phone == phone && sDone[i].id == id && id != 0)
            return &sDone[i];
    return NULL;
}

// The answer to an order: done (with the places it changed, as they are now) or why not
static u16 sUseLearned[4], sUseWants[4];
static u8 sUseLearnedN, sUseWantsN;
static bool8 sUseEvolve;
static u16 sEvoSpecies;
static s8 sEvoSlot = -1;
static bool8 sEvoCanStop;

// (fresh: "rev" after it, so the app knows its model, with the changes applied, is the game's; a repeated answer or
// a refusal has none)
static void SendDone(int c, u32 id, const char *why, bool8 fresh)
{
    u32 i;
    BufReset(&sOut);
    BufAdd(&sOut, "{\"t\":\"done\",\"id\":%u,\"ok\":%u,\"why\":\"%s\",\"changed\":[", (unsigned)id, why == NULL,
           why ? why : "");
    for (i = 0; why == NULL && i < sChangedCount; i++)
    {
        const struct Loc *l = &sChanged[i];
        BufAdd(&sOut, "%s", i ? "," : "");
        if (l->kind == LOC_PARTY)
        {
            u32 k, n = 0;
            BufAdd(&sOut, "{\"party\":[");
            for (k = 0; k < PARTY_SIZE; k++)
            {
                if (GetMonData(&OwnParty()[k], MON_DATA_SPECIES) == SPECIES_NONE)
                    continue;
                BufAdd(&sOut, "%s", n++ ? "," : "");
                BufPartyMon(&sOut, &OwnParty()[k]);
            }
            BufAdd(&sOut, "]}");
        }
        else
        {
            BufAdd(&sOut, "{\"box\":%u,\"slot\":%u,\"mon\":", l->box, l->slot);
            BufBoxMon(&sOut, &Storage()->boxes[l->box][l->slot]);
            BufAdd(&sOut, "}");
        }
    }
    BufAdd(&sOut, "]");
    if (why == NULL && sBagChanged)
    {
        BufAdd(&sOut, ",");
        BufBag(&sOut);
    }
    if (fresh && why == NULL && (sUseLearnedN || sUseWantsN || sUseEvolve))
    {
        // (a USE: the moves it learned, the ones it wants to (four known), whether it's evolving in the game)
        BufAdd(&sOut, ",\"learned\":[");
        for (i = 0; i < sUseLearnedN; i++)
            BufAdd(&sOut, "%s%u", i ? "," : "", sUseLearned[i]);
        BufAdd(&sOut, "],\"wants\":[");
        for (i = 0; i < sUseWantsN; i++)
            BufAdd(&sOut, "%s%u", i ? "," : "", sUseWants[i]);
        BufAdd(&sOut, "],\"evolve\":%u", sUseEvolve);
        sUseLearnedN = sUseWantsN = 0;
        sUseEvolve = FALSE;
    }
    if (fresh && why == NULL)
        BufAdd(&sOut, ",\"rev\":%u", (unsigned)Rev());
    BufAdd(&sOut, "}");
    SendBuf(c, &sOut);
}

// USE: an item on a party POKeMON (the BAG's USE, as the party menu carries it out: its own effects and rules), what
// came of it for the answer, and an evolution to play in the game once it's free

static const char *DoUse(const char *json)
{
    struct Loc at;
    long long pid, ot, item = 0, move = -1;
    struct Pokemon *mon;
    u8 kind, slot;
    sUseLearnedN = sUseWantsN = 0;
    sUseEvolve = FALSE;
    if (!ParseLoc(json, "at", &at) || !JsonNum(json, "p", &pid) || !JsonNum(json, "o", &ot))
        return "bad";
    if (at.kind != LOC_PARTY)
        return "notparty";   // (as in the game: items are used on the party)
    if (Empty(&at))
        return "bad";
    if (!Is(&at, (u32)pid, (u32)ot))
        return "gone";
    if (!JsonNum(json, "item", &item) || item <= 0 || item >= ITEMS_COUNT)
        return "bad";
    JsonNum(json, "move", &move);
    if (!CheckBagHasItem(item, 1))
        return "noitem";
    slot = at.slot;
    mon = &gPlayerParty[slot];
    if (GetMonData(mon, MON_DATA_IS_EGG))
        return "egg";
    if (sEvoSlot >= 0)
        return "busy";
    kind = MP_PhoneItemUse(item);
    switch (kind)
    {
    case 0:
        return "cantuse";
    case 3:   // SACRED ASH: every fainted one
    {
        bool8 any = FALSE;
        u32 i;
        for (i = 0; i < gPlayerPartyCount; i++)
            if (!ExecuteTableBasedItemEffect(&gPlayerParty[i], item, i, 0))
                any = TRUE;
        if (!any)
            return "noeffect";
        break;
    }
    case 5:
    case 6:   // one move's PP
        if (move < 0 || move > 3 || GetMonData(mon, MON_DATA_MOVE1 + move) == MOVE_NONE)
            return "bad";
        if (ExecuteTableBasedItemEffect(mon, item, slot, move))
            return "noeffect";
        break;
    case 7:   // RARE CANDY: the level (up to the cap), its new moves, then maybe an evolution
    {
        u16 learned, evo;
        if (GetMonData(mon, MON_DATA_LEVEL) >= MP_RareCandyCap())
            return "cap";
        if (GetMonData(mon, MON_DATA_LEVEL) == MAX_LEVEL || ExecuteTableBasedItemEffect(mon, item, slot, 0))
            return "noeffect";
        learned = MonTryLearningNewMove(mon, TRUE);
        while (learned != MOVE_NONE)
        {
            if (learned == MON_HAS_MAX_MOVES)
            {
                if (sUseWantsN < 4)
                    sUseWants[sUseWantsN++] = gMoveToLearn;   // (full: it wants to; taught in the game, as ever)
            }
            else if (learned != MON_ALREADY_KNOWS_MOVE && sUseLearnedN < 4)
                sUseLearned[sUseLearnedN++] = learned;
            learned = MonTryLearningNewMove(mon, FALSE);
        }
        evo = GetEvolutionTargetSpecies(mon, EVO_MODE_NORMAL, ITEM_NONE);
        if (evo != SPECIES_NONE)
        {
            sEvoSpecies = evo, sEvoSlot = slot, sEvoCanStop = TRUE;
            sUseEvolve = TRUE;
        }
        break;
    }
    case 8:   // TM / HM: taught, in place of the move chosen when it knows four
    {
        bool8 knows;
        u16 mv = MP_PhoneTmMove(mon, item, &knows);
        if (knows)
            return "knows";
        if (mv == MOVE_NONE)
            return "cantlearn";
        if (GiveMoveToMon(mon, mv) == MON_HAS_MAX_MOVES)
        {
            if (move < 0 || move > 3)
                return "full";
            RemoveMonPPBonus(mon, move);
            SetMonMoveSlot(mon, mv, move);
        }
        sUseLearned[sUseLearnedN++] = mv;
        break;
    }
    case 9:   // an evolution stone: the evolution plays in the game
    {
        u16 evo = GetEvolutionTargetSpecies(mon, EVO_MODE_ITEM_USE, item);
        if (evo == SPECIES_NONE)
            return "noeffect";
        sEvoSpecies = evo, sEvoSlot = slot, sEvoCanStop = FALSE;
        sUseEvolve = TRUE;
        break;
    }
    default:
        if (ExecuteTableBasedItemEffect(mon, item, slot, 0))
            return "noeffect";
        break;
    }
    if (kind != 8 && MP_PhoneItemConsumed(item))   // (TMs and HMs stay: Kai's TMs are reusable)
        RemoveBagItem(item, 1);
    PlaySE(SE_USE_ITEM);
    PartyChanged();
    sBagChanged = TRUE;
    return NULL;
}

static const char *Apply(const char *json)
{
    char op[16];
    sChangedCount = 0;
    sBagChanged = FALSE;
    if (!JsonStr(json, "op", op, sizeof(op)))
        return "bad";
    if (strcmp(op, "move") == 0)
        return DoMove(json);
    if (strcmp(op, "give") == 0)
        return DoItem(json, TRUE);
    if (strcmp(op, "take") == 0)
        return DoItem(json, FALSE);
    if (strcmp(op, "use") == 0)
        return DoUse(json);
    return "unknown";
}

static void Finish(int c, u32 id, const char *json)
{
    const char *why = Apply(json);
    struct Done *d = &sDone[sDoneNext];
    sDoneNext = (sDoneNext + 1) % DONE_MAX;
    d->phone = sSess[c].phone;
    d->id = id;
    snprintf(d->why, sizeof(d->why), "%s", why ? why : "");
    if (why == NULL)
        sApplied++;
    MPNet_Log("phone: order %u %s%s", (unsigned)id, why ? "not done: " : "done", why ? why : "");
    SendDone(c, id, why, TRUE);
}

static void OnOrder(int c, const char *json)
{
    long long id;
    struct Done *d;
    u32 i;
    if (!JsonNum(json, "id", &id) || id <= 0)
        return;
    if ((d = FindDone(sSess[c].phone, (u32)id)) != NULL)
    {
        // (sent again after a lost answer: the same answer, never done twice)
        sChangedCount = 0;
        sBagChanged = FALSE;
        SendDone(c, (u32)id, d->why[0] ? d->why : NULL, FALSE);
        return;
    }
    for (i = 0; i < sOrderCount; i++)
        if (sOrders[i].id == (u32)id && sOrders[i].conn == c)
            return;   // (already waiting)
    if (sOrderCount == 0 && Safe())
    {
        Finish(c, (u32)id, json);
        return;
    }
    if (sOrderCount >= ORDERS_MAX || strlen(json) >= ORDER_LEN)
    {
        SendDone(c, (u32)id, "busy", FALSE);
        return;
    }
    sOrders[sOrderCount].conn = c;
    sOrders[sOrderCount].id = (u32)id;
    strcpy(sOrders[sOrderCount].json, json);
    sOrderCount++;
    BufReset(&sOut);
    BufAdd(&sOut, "{\"t\":\"queued\",\"id\":%u}", (unsigned)id);
    SendBuf(c, &sOut);
}

// Where a species lives, for the POKéDEX (the app's WHERE TO FIND): every wild slot that makes it, as the game would
// now (the GENERATION's pools), each [group, map, method (0 grass, 1 surf, 2 rock smash, 3 rods), slot, min level,
// max level, met there]; the app applies the spawn viewer's rules (rare ones unnamed until met)
static void SendWhere(int c, const char *json)
{
    long long sp = 0;
    u32 i, k, j, n = 0;
    JsonNum(json, "s", &sp);
    BufReset(&sOut);
    BufAdd(&sOut, "{\"t\":\"where\",\"s\":%lld,\"slots\":[", sp);
    for (i = 0; sp > 0 && gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        const struct WildPokemonHeader *h = &gWildMonHeaders[i];
        const struct WildPokemonInfo *infos[4] = {h->landMonsInfo, h->waterMonsInfo, h->rockSmashMonsInfo, h->fishingMonsInfo};
        u32 met = MetBits((h->mapGroup << 8) | h->mapNum);
        for (k = 0; k < 4; k++)
        {
            const struct WildPokemonInfo *info = infos[k] ? MP_Region_WildInfo(infos[k]) : NULL;
            if (info == NULL)
                continue;
            for (j = 0; j < sAreaSlots[k]; j++)
                if (MP_WildSlotSpecies(info, j) == sp)
                    BufAdd(&sOut, "%s[%u,%u,%u,%u,%u,%u,%u]", n++ ? "," : "", h->mapGroup, h->mapNum, k, j,
                           info->wildPokemon[j].minLevel, info->wildPokemon[j].maxLevel, (met >> (sAreaBit[k] + j)) & 1);
        }
    }
    BufAdd(&sOut, "]}");
    SendBuf(c, &sOut);
}

static void OnSealed(int c, const char *json)
{
    char t[16];
    if (!JsonStr(json, "t", t, sizeof(t)))
        return;
    if (strcmp(t, "snap") == 0)
        SendSnapshot(c);
    else if (strcmp(t, "spawnmaps") == 0)
        SendSpawnMaps(c);
    else if (strcmp(t, "spawns") == 0)
        SendSpawns(c, json);
    else if (strcmp(t, "order") == 0)
        OnOrder(c, json);
    else if (strcmp(t, "ping") == 0)
        SendState(c);
    else if (strcmp(t, "where") == 0)
        SendWhere(c, json);
    else if (strcmp(t, "watch") == 0)
    {
        // the phone shows the POKéTCH (on: 1) or stopped (on: 0)
        long long on = 0;
        JsonNum(json, "on", &on);
        if (sSess[c].watch && !on)
            MP_Poketch_PhoneDone();
        sSess[c].watch = on != 0;
        sSess[c].watchHash = 0;
        if (on && !MP_Poketch_PhoneAvailable())
        {
            BufReset(&sOut);
            BufAdd(&sOut, "{\"t\":\"watch\",\"none\":1}");
            SendBuf(c, &sOut);
            sSess[c].watch = FALSE;
        }
    }
    else if (strcmp(t, "touch") == 0 && sSess[c].watch)
    {
        long long x = 0, y = 0, d = 0;
        JsonNum(json, "x", &x);
        JsonNum(json, "y", &y);
        JsonNum(json, "d", &d);
        MP_Poketch_PhoneTouch((s16)x, (s16)y, d != 0);
    }
    else if (strcmp(t, "watchapp") == 0 && sSess[c].watch)
    {
        long long dir = 1;
        JsonNum(json, "dir", &dir);
        MP_Poketch_PhoneApp(dir < 0 ? -1 : 1);
    }
}

// The POKéTCH's LCD for a phone showing it, when it changed: 192x160 shades 0-3, run-length coded (a byte a run: the
// shade in the low 2 bits, the run's length less one above), as hex
static void SendWatch(int c)
{
    u8 app, color, flags;
    u32 unlocked, i, h = 2166136261u, n = 192 * 160;
    const u8 *lcd = MP_Poketch_PhoneFrame(&app, &color, &flags, &unlocked);
    for (i = 0; i < n; i++)
        h = (h ^ lcd[i]) * 16777619u;
    h ^= app | (color << 8) | (flags << 16);
    if (h == sSess[c].watchHash)
        return;
    sSess[c].watchHash = h;
    BufReset(&sOut);
    BufAdd(&sOut, "{\"t\":\"watch\",\"app\":%u,\"color\":%u,\"flags\":%u,\"apps\":%u,\"rle\":\"", app, color, flags,
           (unsigned)unlocked);
    for (i = 0; i < n;)
    {
        u32 run = 1;
        while (i + run < n && run < 64 && lcd[i + run] == lcd[i])
            run++;
        BufAdd(&sOut, "%02X", (unsigned)(((run - 1) << 2) | (lcd[i] & 3)));
        i += run;
    }
    BufAdd(&sOut, "\"}");
    SendBuf(c, &sOut);
}

static int FindPhone(const u8 id[PHONE_ID_SIZE])
{
    u32 i;
    for (i = 0; i < sFile.count; i++)
        if (memcmp(sFile.phones[i].id, id, PHONE_ID_SIZE) == 0)
            return i;
    return -1;
}

static void OnPair(int c, const char *json)
{
    char hex[2 * PHONE_KEY_SIZE + 4], name[24];
    u8 phoneId[PHONE_ID_SIZE], saveId[PHONE_SAVEID_SIZE], proof[PHONE_KEY_SIZE], want[PHONE_KEY_SIZE];
    const u8 *secret = NULL;
    int i;
    if (!sPairing)
    {
        Bye(c, "notpairing");
        return;
    }
    if (!JsonStr(json, "phone", hex, sizeof(hex)) || !PhoneProto_FromHex(hex, phoneId, PHONE_ID_SIZE)
     || !JsonStr(json, "save", hex, sizeof(hex)) || !PhoneProto_FromHex(hex, saveId, PHONE_SAVEID_SIZE)
     || !JsonStr(json, "proof", hex, sizeof(hex)) || !PhoneProto_FromHex(hex, proof, PHONE_KEY_SIZE))
    {
        Bye(c, "bad");
        return;
    }
    if (memcmp(saveId, sFile.saveId, PHONE_SAVEID_SIZE) != 0)
    {
        Bye(c, "othersave");
        return;
    }
    PhoneProto_PairProof(sPairSecret, phoneId, saveId, want);
    if (crypto_verify32(want, proof) == 0)
        secret = sPairSecret;
    else
    {
        PhoneProto_PairProof(sCodeSecret, phoneId, saveId, want);
        if (crypto_verify32(want, proof) == 0)
            secret = sCodeSecret;
    }
    if (secret == NULL)
    {
        Bye(c, "wrongcode");
        if (++sPairFails >= 5)
        {
            // (too many guesses at the letters: new ones)
            void MP_Phone_PairStart(void);
            MP_Phone_PairStart();
        }
        return;
    }
    if (!JsonStr(json, "name", name, sizeof(name)))
        snprintf(name, sizeof(name), "PHONE");
    for (i = 0; name[i]; i++)
        if (name[i] == '"' || name[i] == '\\' || (u8)name[i] < 32 || (u8)name[i] > 126)
            name[i] = '?';
    // The phone's record: a new key (the one before, for a phone pairing again); a third phone pushes out the oldest
    i = FindPhone(phoneId);
    if (i < 0)
    {
        if (sFile.count < PHONES_MAX)
            i = sFile.count++;
        else
            i = sFile.phones[0].paired <= sFile.phones[1].paired ? 0 : 1;
    }
    memcpy(sFile.phones[i].id, phoneId, PHONE_ID_SIZE);
    PhoneProto_Random(sFile.phones[i].key, PHONE_KEY_SIZE);
    snprintf(sFile.phones[i].name, sizeof(sFile.phones[i].name), "%s", name);
    sFile.phones[i].paired = (u32)time(NULL);
    WriteNow();
    {
        u8 sealed[PHONE_KEY_SIZE + PHONE_SEAL_EXTRA];
        char keyHex[2 * sizeof(sealed) + 1], json2[256], saveHex[2 * PHONE_SAVEID_SIZE + 1];
        PhoneProto_Seal(secret, 0, sFile.phones[i].key, PHONE_KEY_SIZE, sealed);
        PhoneProto_ToHex(sealed, sizeof(sealed), keyHex);
        PhoneProto_ToHex(sFile.saveId, PHONE_SAVEID_SIZE, saveHex);
        snprintf(json2, sizeof(json2), "{\"t\":\"paired\",\"key\":\"%s\",\"save\":\"%s\",\"v\":%d}", keyHex, saveHex,
                 PHONE_LINK_VERSION);
        SendPlain(c, json2);
    }
    snprintf(sJustPaired, sizeof(sJustPaired), "%s", name);
    MPNet_Log("phone: paired with \"%s\" (%s, slot %d)", name, secret == sPairSecret ? "QR" : "code", i);
}

static void OnHello(int c, const char *json)
{
    char hex[64];
    u8 phoneId[PHONE_ID_SIZE], saveId[PHONE_SAVEID_SIZE], pn[PHONE_NONCE_SIZE], gn[PHONE_NONCE_SIZE];
    long long v = 0;
    int i;
    if (!JsonStr(json, "phone", hex, sizeof(hex)) || !PhoneProto_FromHex(hex, phoneId, PHONE_ID_SIZE)
     || !JsonStr(json, "save", hex, sizeof(hex)) || !PhoneProto_FromHex(hex, saveId, PHONE_SAVEID_SIZE)
     || !JsonStr(json, "nonce", hex, sizeof(hex)) || !PhoneProto_FromHex(hex, pn, PHONE_NONCE_SIZE))
    {
        Bye(c, "bad");
        return;
    }
    JsonNum(json, "v", &v);
    if (v != PHONE_LINK_VERSION)
    {
        Bye(c, "version");
        return;
    }
    if (memcmp(saveId, sFile.saveId, PHONE_SAVEID_SIZE) != 0)
    {
        Bye(c, "othersave");
        return;
    }
    i = FindPhone(phoneId);
    if (i < 0)
    {
        Bye(c, "unknown");
        return;
    }
    PhoneProto_Random(gn, sizeof(gn));
    PhoneProto_SessionKey(sFile.phones[i].key, pn, gn, sSess[c].key);
    sSess[c].phone = i;
    sSess[c].seqIn = sSess[c].seqOut = 0;
    {
        char json2[160], gnHex[2 * PHONE_NONCE_SIZE + 1];
        PhoneProto_ToHex(gn, sizeof(gn), gnHex);
        snprintf(json2, sizeof(json2), "{\"t\":\"welcome\",\"nonce\":\"%s\",\"v\":%d,\"game\":\"%s\"}", gnHex,
                 PHONE_LINK_VERSION, MOEMON_VERSION);
        SendPlain(c, json2);
    }
    sSess[c].state = SESS_LIVE;
    MPNet_Log("phone: \"%s\" linked (conn %d)", sFile.phones[i].name, c);
    PlaySE(SE_POKENAV_CALL);
}

static void OnFrame(int c, const u8 *body, size_t len)
{
    static char *sPlain;
    static size_t sPlainCap;
    if (len + 1 > sPlainCap)
    {
        char *p = realloc(sPlain, len + 1);
        if (p == NULL)
            return;
        sPlain = p;
        sPlainCap = len + 1;
    }
    if (body[0] == PHONE_BODY_PLAIN)
    {
        char t[16];
        memcpy(sPlain, body + 1, len - 1);
        sPlain[len - 1] = '\0';
        if (!JsonStr(sPlain, "t", t, sizeof(t)))
            return;
        if (strcmp(t, "pair") == 0)
            OnPair(c, sPlain);
        else if (strcmp(t, "hello") == 0)
            OnHello(c, sPlain);
        return;
    }
    if (body[0] == PHONE_BODY_SEALED && sSess[c].state == SESS_LIVE)
    {
        u32 seq;
        int n = PhoneProto_Open(sSess[c].key, body + 1, len - 1, &seq, (u8 *)sPlain);
        if (n < 0 || seq <= sSess[c].seqIn)
        {
            MPNet_Log("phone: conn %d sent a damaged or replayed message", c);
            PhoneNet_Close(c);
            sSess[c].state = SESS_NONE;
            return;
        }
        sSess[c].seqIn = seq;
        sPlain[n] = '\0';
        OnSealed(c, sPlain);
    }
}

// Is there any phone to listen for (or the pairing screen open)?
static bool8 Wanted(void)
{
    return sPairing || (sHaveFile && sFile.count > 0) || getenv("MOEMON_PHONE_PORT") != NULL;
}

// Every frame (mp_main.c MP_FrameUpdate)
void MP_Phone_Update(void)
{
    int c;
    const u8 *frame;
    size_t len;
    char udp[96], idHex[2 * PHONE_SAVEID_SIZE + 1];
    if (!sStarted)
    {
        if (!sHaveFile || !Wanted())
            return;
        sPort = PhoneNet_Listen(getenv("MOEMON_PHONE_PORT") ? atoi(getenv("MOEMON_PHONE_PORT")) : PHONE_TCP_PORT);
        sStarted = TRUE;
        if (sPort > 0)
            MPNet_Log("phone: listening on port %d", sPort);
    }
    if (sPort <= 0)
        return;
    PhoneProto_ToHex(sFile.saveId, PHONE_SAVEID_SIZE, idHex);
    snprintf(udp, sizeof(udp), "MOEPHONE!%s:%d:%d", idHex, sPort, PHONE_LINK_VERSION);
    PhoneNet_Poll(udp);
    while ((c = PhoneNet_NewConn()) >= 0)
    {
        sSess[c].state = SESS_NEW;
        sSess[c].phone = -1;
    }
    for (c = 0; c < PHONE_NET_CONNS; c++)
    {
        int got, budget = 64;
        if (!PhoneNet_IsOpen(c))
        {
            if (sSess[c].state != SESS_NONE)
            {
                u32 i, k = 0;
                sSess[c].state = SESS_NONE;
                if (sSess[c].watch)
                    MP_Poketch_PhoneDone();
                sSess[c].watch = FALSE;
                for (i = 0; i < sOrderCount; i++)   // (its waiting orders go: the phone sends them again)
                    if (sOrders[i].conn != c)
                        sOrders[k++] = sOrders[i];
                sOrderCount = k;
            }
            continue;
        }
        while (budget-- > 0 && (got = PhoneNet_Recv(c, &frame, &len)) == 1)
            OnFrame(c, frame, len);
    }
    // The POKéTCH for phones showing it (20 times a second at most; only what changed is sent)
    for (c = 0; c < PHONE_NET_CONNS; c++)
        if (sSess[c].state == SESS_LIVE && sSess[c].watch && ++sSess[c].watchTimer >= 3)
        {
            sSess[c].watchTimer = 0;
            SendWatch(c);
        }
    // Waiting orders, in order, once the game is free
    while (sOrderCount > 0 && Safe())
    {
        struct Order o = sOrders[0];
        memmove(&sOrders[0], &sOrders[1], (sOrderCount - 1) * sizeof(sOrders[0]));
        sOrderCount--;
        if (sSess[o.conn].state == SESS_LIVE)
            Finish(o.conn, o.id, o.json);
    }
    // An evolution a USE started (an evolution stone, a RARE CANDY's level): the game's own scene, as from the BAG
    if (sEvoSlot >= 0 && Safe())
    {
        s8 slot = sEvoSlot;
        sEvoSlot = -1;
        MPNet_Log("phone: evolution of party slot %d", slot);
        gCB2_AfterEvolution = CB2_ReturnToField;
        BeginEvolutionScene(&gPlayerParty[slot], sEvoSpecies, sEvoCanStop, slot);
    }
    PhoneNet_Poll(udp);   // (send what was just answered)
}

u32 MP_Phone_Count(void)
{
    EnsureFile();
    return sFile.count;
}

u32 MP_Phone_Linked(void)
{
    u32 c, n = 0;
    for (c = 0; c < PHONE_NET_CONNS; c++)
        n += sSess[c].state == SESS_LIVE;
    return n;
}

// ------------------------------------------------------------------------------------------ the PHONE LINK screen

void MP_Phone_PairStart(void)
{
    EnsureFile();
    PhoneProto_Random(sPairRaw, sizeof(sPairRaw));
    PhoneProto_QrSecret(sPairRaw, sPairSecret);
    PhoneProto_NewCode(sPairCode);
    PhoneProto_CodeSecret(sPairCode, sCodeSecret);
    sPairFails = 0;
    sPairing = TRUE;
    sJustPaired[0] = '\0';
}

void MP_Phone_PairStop(void)
{
    sPairing = FALSE;
    memset(sPairRaw, 0, sizeof(sPairRaw));
    memset(sPairSecret, 0, sizeof(sPairSecret));
    memset(sCodeSecret, 0, sizeof(sCodeSecret));
}

void MP_Phone_ForgetAll(void)
{
    u32 c;
    EnsureFile();
    for (c = 0; c < PHONE_NET_CONNS; c++)
    {
        if (sSess[c].state != SESS_NONE)
            Bye(c, "forgotten");
        PhoneNet_Close(c);
        sSess[c].state = SESS_NONE;
    }
    memset(sFile.phones, 0, sizeof(sFile.phones));
    sFile.count = 0;
    WriteNow();
    MPNet_Log("phone: every phone forgotten");
}

// What the QR code holds: MOEPHONE:VERSION:ADDRESS:PORT:SAVE ID:SECRET:LETTERS (QR alphanumeric characters only)
static void PairPayload(char *out, size_t size)
{
    char addr[64], id[2 * PHONE_SAVEID_SIZE + 1], secret[2 * PHONE_QR_SECRET_SIZE + 1];
    PhoneNet_LocalAddress(addr, sizeof(addr));
    PhoneProto_ToHex(sFile.saveId, PHONE_SAVEID_SIZE, id);
    PhoneProto_ToHex(sPairRaw, PHONE_QR_SECRET_SIZE, secret);
    snprintf(out, size, "MOEPHONE:%d:%s:%d:%s:%s:%s", PHONE_LINK_VERSION, addr, sPort > 0 ? sPort : PHONE_TCP_PORT, id,
             secret, sPairCode);
}

enum { WIN_QR, WIN_TEXT, WIN_HELP, WIN_COUNT };

static const struct BgTemplate sBgTemplates[] = {
    {.bg = 0, .charBaseIndex = 2, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0},
    {.bg = 1, .charBaseIndex = 0, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 3, .baseTile = 0},
};

static const struct WindowTemplate sWindows[] = {
    [WIN_QR] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 14, .height = 14, .paletteNum = 15, .baseBlock = 0x020},
    [WIN_TEXT] = {.bg = 0, .tilemapLeft = 16, .tilemapTop = 1, .width = 13, .height = 14, .paletteNum = 15, .baseBlock = 0x0F0},
    [WIN_HELP] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 17, .width = 28, .height = 2, .paletteNum = 15, .baseBlock = 0x1B0},
    DUMMY_WIN_TEMPLATE,
};

static EWRAM_DATA bool8 sLeaving = FALSE;
static EWRAM_DATA u8 sShownLinks = 0;
static MainCallback sPhoneReturn;

static void VBlankCB_Phone(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Phone(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
    DoScheduledBgTilemapCopiesToVram();
}

static void Print(u8 win, const char *ascii, u8 x, u8 y)
{
    u8 buf[96];
    MP_AsciiToGame(buf, ascii, sizeof(buf) - 1);
    AddTextPrinterParameterized(win, FONT_NARROW, buf, x, y, TEXT_SKIP_DRAW, NULL);
}

static void DrawQr(void)
{
    static u8 sQr[qrcodegen_BUFFER_LEN_FOR_VERSION(10)], sTmp[qrcodegen_BUFFER_LEN_FOR_VERSION(10)];
    char payload[200];
    int size, x, y, scale, origin;
    PairPayload(payload, sizeof(payload));
    FillWindowPixelBuffer(WIN_QR, PIXEL_FILL(1));
    if (qrcodegen_encodeText(payload, sTmp, sQr, qrcodegen_Ecc_LOW, 1, 10, qrcodegen_Mask_AUTO, TRUE))
    {
        size = qrcodegen_getSize(sQr);
        scale = (14 * 8 - 8) / size;   // (a quiet zone of at least 4 pixels around)
        if (scale < 1)
            scale = 1;
        origin = (14 * 8 - size * scale) / 2;
        for (y = 0; y < size; y++)
            for (x = 0; x < size; x++)
                if (qrcodegen_getModule(sQr, x, y))
                    FillWindowPixelRect(WIN_QR, PIXEL_FILL(2), origin + x * scale, origin + y * scale, scale, scale);
        MPNet_Log("phone: pairing QR %d modules, %d px each: %s", size, scale, payload);
    }
    PutWindowTilemap(WIN_QR);
    CopyWindowToVram(WIN_QR, COPYWIN_FULL);
}

static void DrawText(void)
{
    char line[64];
    u32 i, linked = MP_Phone_Linked();
    FillWindowPixelBuffer(WIN_TEXT, PIXEL_FILL(1));
    Print(WIN_TEXT, "PHONE LINK", 2, 1);
    Print(WIN_TEXT, "Scan this with the", 2, 17);
    Print(WIN_TEXT, "Moemon app, or type:", 2, 29);
    snprintf(line, sizeof(line), "   %s", sPairCode);
    Print(WIN_TEXT, line, 2, 43);
    if (sJustPaired[0])
    {
        snprintf(line, sizeof(line), "Paired: %.14s!", sJustPaired);
        Print(WIN_TEXT, line, 2, 61);
    }
    else
        Print(WIN_TEXT, "Waiting for a phone...", 2, 61);
    for (i = 0; i < sFile.count && i < PHONES_MAX; i++)
    {
        snprintf(line, sizeof(line), "%.14s%s", sFile.phones[i].name, linked ? "" : "");
        Print(WIN_TEXT, line, 2, 81 + 12 * i);
    }
    if (sFile.count == 0)
        Print(WIN_TEXT, "No phone yet.", 2, 81);
    PutWindowTilemap(WIN_TEXT);
    CopyWindowToVram(WIN_TEXT, COPYWIN_FULL);
}

static void DrawHelp(void)
{
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(1));
    Print(WIN_HELP, "B: BACK      SELECT: FORGET EVERY PHONE", 4, 1);
    PutWindowTilemap(WIN_HELP);
    CopyWindowToVram(WIN_HELP, COPYWIN_FULL);
}

static void Task_Phone(u8 taskId)
{
    static char sShownPaired[24];
    if (sLeaving)
    {
        if (!gPaletteFade.active)
        {
            DestroyTask(taskId);
            FreeAllWindowBuffers();
            Free(GetBgTilemapBuffer(1));
            MP_Phone_PairStop();
            SetMainCallback2(sPhoneReturn);
        }
        return;
    }
    if (strcmp(sShownPaired, sJustPaired) != 0 || sShownLinks != MP_Phone_Linked())
    {
        if (sJustPaired[0] && strcmp(sShownPaired, sJustPaired) != 0)
            PlayFanfare(MUS_LEVEL_UP);
        snprintf(sShownPaired, sizeof(sShownPaired), "%s", sJustPaired);
        sShownLinks = MP_Phone_Linked();
        DrawText();
    }
    if (gPaletteFade.active)
        return;
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sLeaving = TRUE;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    }
    else if (JOY_NEW(SELECT_BUTTON) && sFile.count > 0)
    {
        PlaySE(SE_PC_OFF);
        MP_Phone_ForgetAll();
        DrawText();
    }
}

void CB2_MpPhoneLink(void)
{
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    SetBgTilemapBuffer(0, AllocZeroed(BG_SCREEN_SIZE));
    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);
    ResetPaletteFade();
    ScanlineEffect_Stop();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetTasks();
    InitWindows(sWindows);
    DeactivateAllTextPrinters();
    LoadPalette(GetTextWindowPalette(0), BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    {
        // The backdrop: a layer of one solid tile in the CO-OP menus' blue (the picture leaves the GBA's backdrop out)
        static const u16 sBack[2] = {RGB(4, 10, 18), RGB(4, 10, 18)};
        static u32 sSolid[8];
        u32 i;
        for (i = 0; i < 8; i++)
            sSolid[i] = 0x11111111;
        LoadPalette(sBack, BG_PLTT_ID(0), sizeof(sBack));
        SetBgTilemapBuffer(1, AllocZeroed(BG_SCREEN_SIZE));
        LoadBgTiles(1, sSolid, sizeof(sSolid), 1);
        FillBgTilemapBufferRect(1, 1, 0, 0, 32, 32, 0);
        CopyBgTilemapBufferToVram(1);
    }
    LoadUserWindowBorderGfx(WIN_TEXT, 0x214, BG_PLTT_ID(14));
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    ShowBg(1);
    sLeaving = FALSE;
    EnsureFile();
    if (!sStarted)
        sPort = -1;
    MP_Phone_PairStart();
    MP_Phone_Update();   // (listening before the QR code says where)
    DrawStdFrameWithCustomTileAndPalette(WIN_QR, FALSE, 0x214, 14);   // (a frame clears its window: first)
    DrawStdFrameWithCustomTileAndPalette(WIN_TEXT, FALSE, 0x214, 14);
    DrawStdFrameWithCustomTileAndPalette(WIN_HELP, FALSE, 0x214, 14);
    DrawQr();
    DrawText();
    DrawHelp();
    sShownLinks = MP_Phone_Linked();
    CreateTask(Task_Phone, 80);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(VBlankCB_Phone);
    SetMainCallback2(CB2_Phone);
}

// OPTIONS > CO-OP > PHONE LINK: the screen, then back to returnTo
void MP_Phone_OpenScreen(MainCallback returnTo)
{
    sPhoneReturn = returnTo;
    SetMainCallback2(CB2_MpPhoneLink);
}

// Test harness: "mp phone" prints the link's state; the code and secret for a test client (MOEMON_PHONE_PORT runs)
void MP_Phone_Debug(const char *arg)
{
    char id[2 * PHONE_SAVEID_SIZE + 1], secret[2 * PHONE_QR_SECRET_SIZE + 1];
    EnsureFile();
    if (strcmp(arg, "pair") == 0)
        MP_Phone_PairStart();
    else if (strcmp(arg, "forget") == 0)
        MP_Phone_ForgetAll();
    PhoneProto_ToHex(sFile.saveId, PHONE_SAVEID_SIZE, id);
    PhoneProto_ToHex(sPairRaw, PHONE_QR_SECRET_SIZE, secret);
    printf("[test] phone: save %s port %d phones %u linked %u pairing %d code %s secret %s queued %u applied %u\n", id,
           sPort, (unsigned)sFile.count, (unsigned)MP_Phone_Linked(), sPairing, sPairing ? sPairCode : "-",
           sPairing ? secret : "-", sOrderCount, (unsigned)sApplied);
    fflush(stdout);
}
