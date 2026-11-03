// Moemon Co-op: the phone's device apps, the game's side (docs/COMPANION_APP.md "Pillar 1: the second screen",
// docs/PHONE_LINK.md "Phase 2"). Everything here is read from the game as its own screens read it:
// - the live map: the place's spot on the town map (region_map.c MP_TownMap_*), friends in the room on it, and what
//   the player has found at a place (trainers beaten, items picked up, hidden items, BERRY trees, wild POKéMON seen)
// - the POKéNAV's MATCH CALL: its list as pokenav_match_call_list.c builds it, a call's words from the game's own
//   message builders, the check pages' words
// - the POKéGEAR: its PHONE's calls and its RADIO's shows (mp_pokegear.c MP_Pokegear_Phone*)
// - battle mode: the battlers on the field (only what the player sees: a foe's HP as a share, never its stats) and
//   the player's own party, sent when they change
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "battle.h"
#include "berry.h"
#include "data.h"
#include "event_data.h"
#include "main.h"
#include "match_call.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokenav.h"
#include "rtc.h"
#include "string_util.h"
#include "item.h"
#include "constants/items.h"
#include "constants/characters.h"
#include "battle_anim.h"
#include "strings.h"
#include "wild_encounter.h"
#include "mp_region.h"
#include "constants/event_bg.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/map_groups.h"
#include "constants/region_map_sections.h"
#include "constants/rematches.h"
#include "constants/trainer_types.h"
#include "constants/vars.h"
#include "mp_phone_int.h"
#include "../data/mp_map_group_sizes.h"

// region_map.c
u32 MP_TownMap_Pages(void);
bool8 MP_TownMap_Player(u8 *page, u8 *x, u8 *y);
bool8 MP_TownMap_SecSpot(mapsec_u16_t sec, u8 *page, u8 *x, u8 *y);
// mp_pokegear.c
u8 MP_Pokegear_PhoneCall(u8 who, u8 out[][40], u8 max, bool8 *ask);
u8 MP_Pokegear_PhoneAnswer(bool8 yes, u8 out[][40], u8 max);
bool8 MP_Pokegear_PhoneRadio(u8 freq, const u8 **name, u16 *song, u8 line[40]);
// pokenav's
extern const u8 *const gMatchCallFlavorTexts[REMATCH_TABLE_ENTRIES][CHECK_PAGE_ENTRY_COUNT];
bool32 GetTrainerFlagFromScriptPointer(const u8 *data);
u16 BerryTypeToItemId(u16 berry);   // berry.c

#define SCR_SETORCOPYVAR  0x1A   // (data/script_cmd_table.inc)
#define SCR_TRAINERBATTLE 0x5C

static const struct MapHeader *Header(u32 group, u32 num)
{
    return group < MAP_GROUPS_COUNT && num < sMapGroupSizes[group] ? Overworld_GetMapHeaderByGroupAndId(group, num) : NULL;
}

// ------------------------------------------------------------------------------------------------- the live map

// The snapshot's and "state"'s place, more: the town map spot ("tm": page, x, y), the time (the POKéGEAR's clock: the
// day of the week, hour, minute), friends in the room ([name, page, x, y, battling]; page -1: on no map)
void PhoneDev_Place(struct Buf *b)
{
    u8 page, x, y;
    u32 p, n = 0;
    if (gMain.callback2 != NULL && MP_TownMap_Player(&page, &x, &y))
        BufAdd(b, ",\"tm\":[%u,%u,%u]", page, x, y);
    BufAdd(b, ",\"sec\":%u", gMapHeader.regionMapSectionId);
    RtcCalcLocalTime();
    BufAdd(b, ",\"time\":[%u,%u,%u]", (unsigned)(gLocalTime.days % 7), gLocalTime.hours, gLocalTime.minutes);
    BufAdd(b, ",\"friends\":[");
    for (p = 0; p < MP_MAX_PLAYERS; p++)
    {
        const struct MapHeader *h;
        bool8 on = FALSE;
        if (p == (u32)gMpLocalId || !gMpPlayers[p].present || !gMpPlayers[p].hasState)
            continue;
        h = Header(gMpPlayers[p].state.mapGroup, gMpPlayers[p].state.mapNum);
        if (h != NULL)
            on = MP_TownMap_SecSpot(h->regionMapSectionId, &page, &x, &y);
        BufAdd(b, "%s[", n++ ? "," : "");
        BufText(b, gMpPlayers[p].name, MP_NAME_LEN);
        BufAdd(b, ",%d,%u,%u,%u]", on ? page : -1, on ? x : 0, on ? y : 0,
               (gMpPlayers[p].state.flags & MPF_IN_BATTLE) != 0);
    }
    BufAdd(b, "]");
}

// What the player has found at map section SEC, over all its maps: {"t":"place","sec":..,
//  "trainers":[[trainer, beaten, class name, name],...], "items":[[item, picked up, hidden],...],
//  "berries":[[tree, the BERRY's item (0 none), stage],...], "wild":[[species, seen, caught],...]}
static void SendPlace(int c, const char *json)
{
    long long sec = -1;
    u32 g, m, i, k, j, nt = 0, ni = 0, nb = 0, nw = 0;
    static u16 sSpecies[96];
    JsonNum(json, "sec", &sec);
    BufReset(&sOut);
    BufAdd(&sOut, "{\"t\":\"place\",\"sec\":%lld,\"trainers\":[", sec);
    // trainers: a person whose script starts with a trainer battle
    for (g = 0; g < MAP_GROUPS_COUNT; g++)
        for (m = 0; m < sMapGroupSizes[g]; m++)
        {
            const struct MapHeader *h = Header(g, m);
            if (h == NULL || h->regionMapSectionId != sec || h->events == NULL)
                continue;
            for (i = 0; i < h->events->objectEventCount; i++)
            {
                const struct ObjectEventTemplate *o = &h->events->objectEvents[i];
                u16 trainer;
                if (o->trainerType == TRAINER_TYPE_NONE || o->script == NULL || o->script[0] != SCR_TRAINERBATTLE)
                    continue;
                trainer = o->script[2] | (o->script[3] << 8);
                if (trainer == 0 || trainer >= TRAINERS_COUNT)
                    continue;
                BufAdd(&sOut, "%s[%u,%u,", nt++ ? "," : "", trainer, GetTrainerFlagFromScriptPointer(o->script) != 0);
                BufText(&sOut, gTrainerClassNames[gTrainers[trainer].trainerClass], 13);
                BufAdd(&sOut, ",");
                BufText(&sOut, gTrainers[trainer].trainerName, TRAINER_NAME_LENGTH);
                BufAdd(&sOut, "]");
            }
        }
    BufAdd(&sOut, "],\"items\":[");
    for (g = 0; g < MAP_GROUPS_COUNT; g++)
        for (m = 0; m < sMapGroupSizes[g]; m++)
        {
            const struct MapHeader *h = Header(g, m);
            if (h == NULL || h->regionMapSectionId != sec || h->events == NULL)
                continue;
            // item balls: "finditem ITEM" (setorcopyvar VAR_0x8000, ITEM first), gone once their flag is set
            for (i = 0; i < h->events->objectEventCount; i++)
            {
                const struct ObjectEventTemplate *o = &h->events->objectEvents[i];
                const u8 *s = o->script;
                if (TEMPLATE_GFX_ID(o) != OBJ_EVENT_GFX_ITEM_BALL || s == NULL || s[0] != SCR_SETORCOPYVAR
                 || (s[1] | (s[2] << 8)) != VAR_0x8000 || o->flagId == 0)
                    continue;
                BufAdd(&sOut, "%s[%u,%u,0]", ni++ ? "," : "", s[3] | (s[4] << 8), FlagGet(o->flagId) != 0);
            }
            for (i = 0; i < h->events->bgEventCount; i++)
            {
                const struct BgEvent *e = &h->events->bgEvents[i];
                if (e->kind != BG_EVENT_HIDDEN_ITEM)
                    continue;
                BufAdd(&sOut, "%s[%u,%u,1]", ni++ ? "," : "", e->bgUnion.hiddenItem.item,
                       FlagGet(FLAG_HIDDEN_ITEMS_START + e->bgUnion.hiddenItem.hiddenItemId) != 0);
            }
        }
    BufAdd(&sOut, "],\"berries\":[");
    for (g = 0; g < MAP_GROUPS_COUNT; g++)
        for (m = 0; m < sMapGroupSizes[g]; m++)
        {
            const struct MapHeader *h = Header(g, m);
            if (h == NULL || h->regionMapSectionId != sec || h->events == NULL)
                continue;
            for (i = 0; i < h->events->objectEventCount; i++)
            {
                const struct ObjectEventTemplate *o = &h->events->objectEvents[i];
                struct BerryTree *t;
                if (TEMPLATE_GFX_ID(o) != OBJ_EVENT_GFX_BERRY_TREE || o->trainerRange_berryTreeId == 0)
                    continue;
                t = GetBerryTreeInfo(o->trainerRange_berryTreeId);
                BufAdd(&sOut, "%s[%u,%u,%u]", nb++ ? "," : "", o->trainerRange_berryTreeId,
                       t->berry ? BerryTypeToItemId(t->berry) : 0, t->stage);
            }
        }
    BufAdd(&sOut, "],\"wild\":[");
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        const struct WildPokemonHeader *w = &gWildMonHeaders[i];
        const struct MapHeader *h = Header(w->mapGroup, w->mapNum);
        const struct WildPokemonInfo *infos[4] = {w->landMonsInfo, w->waterMonsInfo, w->rockSmashMonsInfo, w->fishingMonsInfo};
        static const u8 sSlots[4] = {12, 5, 5, 10};
        if (h == NULL || h->regionMapSectionId != sec)
            continue;
        for (k = 0; k < 4; k++)
        {
            const struct WildPokemonInfo *info = infos[k] ? MP_Region_WildInfo(infos[k]) : NULL;
            if (info == NULL)
                continue;
            for (j = 0; j < sSlots[k]; j++)
            {
                u16 sp = MP_WildSlotSpecies(info, j);
                u32 d, num;
                if (sp == SPECIES_NONE || sp >= NUM_SPECIES)
                    continue;
                for (d = 0; d < nw && sSpecies[d] != sp; d++)
                    ;
                if (d < nw || nw >= ARRAY_COUNT(sSpecies))
                    continue;
                sSpecies[nw] = sp;
                num = SpeciesToNationalPokedexNum(sp);
                BufAdd(&sOut, "%s[%u,%u,%u]", nw++ ? "," : "", sp, GetSetPokedexFlag(num, FLAG_GET_SEEN) != 0,
                       GetSetPokedexFlag(num, FLAG_GET_CAUGHT) != 0);
            }
        }
    }
    BufAdd(&sOut, "]}");
    SendBuf(c, &sOut);
}

// -------------------------------------------------------------------------------------------------- MATCH CALL

// The POKéNAV's MATCH CALL list as it builds it (LoopedTask_BuildMatchCallList): the special people first, then the
// registered trainers; each {"k": 0 special / 1 trainer, "i": its number, "c": class, "n": name, "sec": where,
// "r": ready for a rematch, "pic": trainer picture, "chk": the check page's four texts}
static void SendCalls(int c)
{
    u32 i, k, n = 0;
    BufReset(&sOut);
    BufAdd(&sOut, "{\"t\":\"calls\",\"has\":%u,\"list\":[", FlagGet(FLAG_SYS_POKENAV_GET) != 0);
    for (i = 0; i < MC_HEADER_COUNT; i++)
    {
        const u8 *desc = NULL, *name = NULL;
        u32 rematch, pic;
        if (!MatchCall_GetEnabled(i))
            continue;
        MatchCall_GetNameAndDesc(i, &desc, &name);
        rematch = MatchCall_GetRematchTableIdx(i);
        pic = rematch != REMATCH_TABLE_ENTRIES ? gTrainers[GetTrainerIdxByRematchIdx(rematch)].trainerPic
                                               : gFacilityClassToPicIndex[MatchCall_GetOverrideFacilityClass(i)];
        BufAdd(&sOut, "%s{\"k\":0,\"i\":%u,\"c\":", n++ ? "," : "", i);
        BufText(&sOut, desc ? desc : (const u8 *)"\xFF", 40);
        BufAdd(&sOut, ",\"n\":");
        BufText(&sOut, name ? name : (const u8 *)"\xFF", 40);
        BufAdd(&sOut, ",\"sec\":%u,\"r\":%u,\"pic\":%u", MatchCall_GetMapSec(i),
               rematch != REMATCH_TABLE_ENTRIES && gSaveBlock1Ptr->trainerRematches[rematch] != 0, pic);
        if (MatchCall_HasCheckPage(i))
        {
            BufAdd(&sOut, ",\"chk\":[");
            for (k = 0; k < CHECK_PAGE_ENTRY_COUNT; k++)
            {
                const u8 *t = rematch == REMATCH_TABLE_ENTRIES ? MatchCall_GetOverrideFlavorText(i, k)
                                                               : gMatchCallFlavorTexts[rematch][k];
                BufAdd(&sOut, "%s", k ? "," : "");
                BufText(&sOut, t ? t : (const u8 *)"\xFF", 120);
            }
            BufAdd(&sOut, "]");
        }
        BufAdd(&sOut, "}");
    }
    for (i = 0; i < REMATCH_TABLE_ENTRIES; i++)
    {
        u32 trainer;
        if (MatchCall_HasRematchId(i) || !IsRematchEntryRegistered(i))
            continue;
        trainer = GetTrainerIdxByRematchIdx(i);
        BufAdd(&sOut, "%s{\"k\":1,\"i\":%u,\"c\":", n++ ? "," : "", i);
        BufText(&sOut, gTrainerClassNames[gTrainers[trainer].trainerClass], 13);
        BufAdd(&sOut, ",\"n\":");
        BufText(&sOut, gTrainers[trainer].trainerName, TRAINER_NAME_LENGTH);
        BufAdd(&sOut, ",\"sec\":%u,\"r\":%u,\"pic\":%u,\"chk\":[", GetMatchTableMapSectionId(i),
               gSaveBlock1Ptr->trainerRematches[i] != 0, gTrainers[trainer].trainerPic);
        for (k = 0; k < CHECK_PAGE_ENTRY_COUNT; k++)
        {
            BufAdd(&sOut, "%s", k ? "," : "");
            BufText(&sOut, gMatchCallFlavorTexts[i][k], 120);
        }
        BufAdd(&sOut, "]}");
    }
    BufAdd(&sOut, "]}");
    SendBuf(c, &sOut);
}

// A call (GetMatchCallMessageText): the game's own words for it, made only while the game is free (they use the
// shared string buffers a message on screen would); "busy" else. "r": the call asked for a rematch.
static void SendCall(int c, const char *json)
{
    long long k = 0, i = 0;
    bool8 rematch = FALSE;
    static u8 sText[1000];
    JsonNum(json, "k", &k);
    JsonNum(json, "i", &i);
    BufReset(&sOut);
    if (!MP_IsLocalFree() || gMain.inBattle || !FlagGet(FLAG_SYS_POKENAV_GET))
    {
        BufAdd(&sOut, "{\"t\":\"call\",\"k\":%lld,\"i\":%lld,\"busy\":1}", k, i);
        SendBuf(c, &sOut);
        return;
    }
    sText[0] = EOS;
    if (!Overworld_MapTypeAllowsTeleportAndFly(gMapHeader.mapType))
        StringCopy(sText, gText_CallCantBeMadeHere);
    else if (k == 1 && i >= 0 && i < REMATCH_TABLE_ENTRIES && IsRematchEntryRegistered(i))
        rematch = SelectMatchCallMessage(GetTrainerIdxByRematchIdx(i), sText);
    else if (k == 0 && i >= 0 && i < MC_HEADER_COUNT && MatchCall_GetEnabled(i))
        MatchCall_GetMessage(i, sText);
    BufAdd(&sOut, "{\"t\":\"call\",\"k\":%lld,\"i\":%lld,\"r\":%u,\"text\":", k, i, rematch);
    BufText(&sOut, sText, sizeof(sText) - 1);
    BufAdd(&sOut, "}");
    SendBuf(c, &sOut);
}

// ---------------------------------------------------------------------------------------------------- POKéGEAR

static void SendLines(int c, const char *what, u8 lines[][40], u8 n, bool8 ask)
{
    u8 i;
    BufReset(&sOut);
    BufAdd(&sOut, "{\"t\":\"gear\",\"%s\":1,\"ask\":%u,\"busy\":%u,\"lines\":[", what, ask, n == 0);
    for (i = 0; i < n; i++)
    {
        BufAdd(&sOut, "%s", i ? "," : "");
        BufText(&sOut, lines[i], 39);
    }
    BufAdd(&sOut, "]}");
    SendBuf(c, &sOut);
}

// {"t":"gear","call":WHO} (0 MOM, 1 PROF. ELM), {"t":"gear","answer":0|1} (MOM's question),
// {"t":"gear","radio":FREQ} (the station's name, its song, the show's next line)
static void OnGear(int c, const char *json)
{
    static u8 sLines[12][40];
    long long v;
    if (!CheckBagHasItem(ITEM_POKEGEAR, 1))
    {
        BufReset(&sOut);
        BufAdd(&sOut, "{\"t\":\"gear\",\"none\":1}");
        SendBuf(c, &sOut);
        return;
    }
    if (JsonNum(json, "call", &v))
    {
        bool8 ask = FALSE;
        u8 n = MP_IsLocalFree() && !gMain.inBattle ? MP_Pokegear_PhoneCall(v != 0, sLines, 12, &ask) : 0;
        SendLines(c, "call", sLines, n, ask);
    }
    else if (JsonNum(json, "answer", &v))
    {
        u8 n = MP_IsLocalFree() && !gMain.inBattle ? MP_Pokegear_PhoneAnswer(v != 0, sLines, 12) : 0;
        SendLines(c, "answer", sLines, n, FALSE);
    }
    else if (JsonNum(json, "radio", &v))
    {
        const u8 *name = NULL;
        u16 song = 0;
        u8 line[40];
        bool8 ok = MP_Pokegear_PhoneRadio(v < 0 ? 0 : (u8)v, &name, &song, line);
        BufReset(&sOut);
        BufAdd(&sOut, "{\"t\":\"gear\",\"radio\":%lld,\"busy\":%u,\"song\":%u,\"name\":", v, !ok, song);
        BufText(&sOut, ok && name ? name : (const u8 *)"\xFF", 40);
        BufAdd(&sOut, ",\"line\":");
        BufText(&sOut, ok ? line : (const u8 *)"\xFF", 39);
        BufAdd(&sOut, "}");
        SendBuf(c, &sOut);
    }
}

// --------------------------------------------------------------------------------------------------- battle mode

static struct { u32 hash; u8 timer; bool8 sent; } sFeed[8];

// The battle as the player sees it: {"t":"battle","on":1,"mons":[{"b": battler, "side": 0 yours / 1 theirs,
// "me": 1 the player's own, "s": species, "l": level, "st": status, "ty": its types now, and for the player's own
// "hp", "mhp", "m": moves, "pp"; for the others "pct": HP left in hundredths}], "party":[[species, hp, max hp,
// status],...]}; {"t":"battle","on":0} when it ends
static u32 BufBattle(struct Buf *b)
{
    u32 i, k, n = 0, h = 2166136261u;
    bool8 multi = gMpCoopBattle || (gBattleTypeFlags & BATTLE_TYPE_MULTI);
    BufAdd(b, "{\"t\":\"battle\",\"on\":1,\"wild\":%u,\"mons\":[", !(gBattleTypeFlags & BATTLE_TYPE_TRAINER));
    for (i = 0; i < gBattlersCount && i < MAX_BATTLERS_COUNT; i++)
    {
        struct BattlePokemon *m = &gBattleMons[i];
        u8 side = GetBattlerSide(i);
        bool8 mine = side == B_SIDE_PLAYER && (!multi || i == GetBattlerAtPosition(B_POSITION_PLAYER_LEFT));
        if (m->species == SPECIES_NONE)
            continue;
        BufAdd(b, "%s{\"b\":%u,\"side\":%u,\"me\":%u,\"s\":%u,\"l\":%u,\"st\":%u,\"ty\":[%u,%u]", n++ ? "," : "", i,
               side, mine, m->species, m->level, (unsigned)m->status1, m->types[0], m->types[1]);
        if (mine)
        {
            BufAdd(b, ",\"hp\":%u,\"mhp\":%u,\"m\":[%u,%u,%u,%u],\"pp\":[%u,%u,%u,%u]", m->hp, m->maxHP, m->moves[0],
                   m->moves[1], m->moves[2], m->moves[3], m->pp[0], m->pp[1], m->pp[2], m->pp[3]);
            h = (h ^ (m->hp | m->pp[0] << 16 | m->pp[1] << 24)) * 16777619u;
            h = (h ^ (m->pp[2] | m->pp[3] << 8 | m->moves[0] << 16)) * 16777619u;
        }
        else
            BufAdd(b, ",\"pct\":%u", m->maxHP ? (m->hp * 100 + m->maxHP - 1) / m->maxHP : 0);
        BufAdd(b, "}");
        h = (h ^ (i | m->species << 8 | m->status1 << 24)) * 16777619u;
        h = (h ^ (m->hp | m->types[0] << 16 | m->types[1] << 24)) * 16777619u;
    }
    BufAdd(b, "],\"party\":[");
    for (i = 0, k = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *p = &OwnParty()[i];
        u32 sp = GetMonData(p, MON_DATA_SPECIES), hp = GetMonData(p, MON_DATA_HP), st = GetMonData(p, MON_DATA_STATUS);
        if (sp == SPECIES_NONE)
            continue;
        BufAdd(b, "%s[%u,%u,%u,%u,%u]", k++ ? "," : "", sp, hp, (unsigned)GetMonData(p, MON_DATA_MAX_HP), st,
               (unsigned)GetMonData(p, MON_DATA_IS_EGG));
        h = (h ^ (sp | hp << 16)) * 16777619u;
        h = (h ^ st) * 16777619u;
    }
    BufAdd(b, "]}");
    return h;
}

void PhoneDev_Update(int c, bool8 live)
{
    if (c < 0 || c >= (int)ARRAY_COUNT(sFeed))
        return;
    if (!live)
    {
        sFeed[c].sent = FALSE;
        return;
    }
    if (!gMain.inBattle || gBattlersCount == 0)
    {
        if (sFeed[c].sent)
        {
            BufReset(&sOut);
            BufAdd(&sOut, "{\"t\":\"battle\",\"on\":0}");
            SendBuf(c, &sOut);
            sFeed[c].sent = FALSE;
            sFeed[c].hash = 0;
        }
        return;
    }
    // (a look every 6 frames, sent when something changed)
    if (++sFeed[c].timer < 6)
        return;
    sFeed[c].timer = 0;
    {
        static struct Buf sBattle;
        u32 h;
        BufReset(&sBattle);
        h = BufBattle(&sBattle);
        if (sFeed[c].sent && h == sFeed[c].hash)
            return;
        sFeed[c].hash = h;
        sFeed[c].sent = TRUE;
        SendBuf(c, &sBattle);
    }
}

void PhoneDev_Closed(int c)
{
    if (c >= 0 && c < (int)ARRAY_COUNT(sFeed))
        memset(&sFeed[c], 0, sizeof(sFeed[c]));
}

bool8 PhoneDev_OnSealed(int c, const char *t, const char *json)
{
    if (strcmp(t, "place") == 0)
        SendPlace(c, json);
    else if (strcmp(t, "calls") == 0)
        SendCalls(c);
    else if (strcmp(t, "call") == 0)
        SendCall(c, json);
    else if (strcmp(t, "gear") == 0)
        OnGear(c, json);
    else
        return FALSE;
    return TRUE;
}
