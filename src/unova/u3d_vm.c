// Moemon Kai: UNOVA's scripts (docs/UNOVA_HANDOFF.md "Black's words and scripts"). Black's own script bytecode, from
// unova/black_scripts.pak beside the game (tools/unova/build_scripts.py, made from the user's Black ROM when the
// release is built; nothing of it is in this repository), run as Black's field VM runs it (aletheia's vm.c,
// script_work_cmd.c): a call stack that's also the comparisons' stack, Black's own event work (flags and works, kept
// in the region side file as Black lays them out: mp_region.c chunk UNEW), its messages from UNOVA's text pack, its
// people moved by its movement lists. Every command's arguments are known (src/unova/u3d_cmds.h from
// tools/unova/scrcmd.py), so one this file doesn't play yet is read past and the script goes on.
//
// The field: an UNOVA map's people, signs and trigger spots run Unova_EventScript_Talk / _Sign / _Pos
// (data/scripts/unova.inc), which start the script Black's event data gives them here and wait (waitstate) until it
// ends. A map's own scripts (Black's init file: the field-init script that places people by the story, the scene
// table run when a work holds a value) run from U3D_Vm_MapLoad and U3D_Vm_SceneCheck.
//
// Its own code, never SINNOH's (s3d_scrcmd.c) or ORRE's (orre_vm.c).
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "easy_chat.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "script.h"
#include "script_movement.h"
#include "sound.h"
#include "mp_kaisong.h"
#include "m4a.h"
#include "string_util.h"
#include "text.h"
#include "u3d.h"
#include "naming_screen.h"
#include "walda_phrase.h"
#include "constants/event_object_movement.h"
#include "constants/easy_chat.h"
#include "constants/event_objects.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/songs.h"
#include "u3d_cmds.h"
#include "u3d_maps.h"
#include "u3d_subway.h"
void ChoosePartyForBattleFrontier(void);   // (script_pokemon_util.c)
void BattleSetup_StartUnovaBattle(void);
#include "script_pokemon_util.h"
#include "shop.h"
#include "random.h"
#include "money.h"
#include "mp_region.h"
#include "rtc.h"
#include "daycare.h"
#include "load_save.h"
#include "data.h"
#include "party_menu.h"
#include "mp.h"
#include "pokedex.h"
#include "battle.h"
#include "constants/battle.h"
#include "credits.h"
#include "save.h"
#include "item.h"
#include "constants/items.h"
#include "pokemon_storage_system.h"
#include "menu.h"
#include "script_menu.h"
#include "platform.h"
#include "save.h"
#include "field_screen_effect.h"
#include "move_relearner.h"
#include "pokemon_summary_screen.h"
#include "constants/flags.h"
#include "item.h"
#include "battle_setup.h"
#include "constants/items.h"
#include "constants/species.h"
void MP_StarterTable_Open(const u16 *species, MainCallback back);

const char *Updater_ExeFile(const char *name);
int uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);
void MPNet_Log(const char *fmt, ...);

#define SYSTEM_BANKS 288        // (tools/unova/build_text.py: the system banks, then the story banks)
#define MSG_OWN 0x400            // (a message command's file: the script's own)
#define OBJ_PLAYER 0xFF

// ------------------------------------------------------------------------------------------------ the pack
static u8 *sPack;
static u32 sPackSize;
static bool8 sTried;
static u32 sFilesAt, sRangesAt, sZonesAt, sEventsAt, sTrainersAt, sTeamsAt, sSongKindsAt, sEncountAt, sTrMsgAt, sEyeSongsAt, sWarpsAt, sCallsAt, sTradesAt, sWfbcAt, sSubwayAt, sSubwayTrainersAt, sSubwayTablesAt;

static u32 Get32(u32 at) { return at + 4 <= sPackSize ? sPack[at] | sPack[at + 1] << 8 | sPack[at + 2] << 16 | (u32)sPack[at + 3] << 24 : 0; }
static u16 Get16(u32 at) { return at + 2 <= sPackSize ? sPack[at] | sPack[at + 1] << 8 : 0; }

static u32 ZoneSize(void) { return Get32(4) >= 4 ? 18 : 16; }   // (a zone's record: 9 u16 from version 4)

// (a block's files follow their table, each padded to 4 bytes: the next block starts after the last one)
static u32 BlockEnd(u32 at)
{
    u32 n = Get32(at), end = at + 4 + 8 * n;
    if (n)
        end = Get32(at + 4 + 8 * (n - 1)) + Get32(at + 8 + 8 * (n - 1));
    return (end + 3) & ~3;
}

static void Load(void)
{
    const char *path = getenv("MOEMON_U3D_SCRIPTS");
    FILE *f;
    long size;
    u8 *data;
    unsigned long raw;
    sTried = TRUE;
    if (path == NULL)
        path = Updater_ExeFile("unova/black_scripts.pak");
    if ((f = fopen(path, "rb")) == NULL)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(size > 0 ? size : 1);
    if (data == NULL || fread(data, 1, size, f) != (size_t)size || size < 8 || memcmp(data, "U3SZ", 4) != 0)
    {
        fclose(f);
        free(data);
        printf("[u3d] %s isn't UNOVA's scripts\n", path);
        return;
    }
    fclose(f);
    raw = data[4] | data[5] << 8 | data[6] << 16 | (unsigned long)data[7] << 24;
    sPack = malloc(raw ? raw : 1);
    sPackSize = raw;
    if (sPack == NULL || uncompress(sPack, &raw, data + 8, (unsigned long)(size - 8)) != 0 || memcmp(sPack, "U3DS", 4) != 0
     || Get32(4) < 1 || Get32(4) > 12)
    {
        printf("[u3d] %s: can't read UNOVA's scripts\n", path);
        free(sPack);
        sPack = NULL;
        sPackSize = 0;
    }
    free(data);
    if (sPack == NULL)
        return;
    sFilesAt = 8;
    sRangesAt = BlockEnd(sFilesAt);
    sZonesAt = sRangesAt + 4 + 10 * Get32(sRangesAt);
    sEventsAt = (sZonesAt + 4 + ZoneSize() * Get32(sZonesAt) + 3) & ~3;
    if (Get32(4) >= 2)   // (version 2: Black's trainers and their teams)
    {
        sTrainersAt = BlockEnd(sEventsAt);
        sTeamsAt = BlockEnd(sTrainersAt);
        if (Get32(4) >= 3)   // (version 3: each trainer's battle song kind)
            sSongKindsAt = BlockEnd(sTeamsAt);
        if (Get32(4) >= 4)   // (version 4: the wild POKéMON)
            sEncountAt = (sSongKindsAt + 4 + Get32(sSongKindsAt) + 3) & ~3;
        if (Get32(4) >= 5)   // (version 5: the trainers' words, the classes' encounter songs)
        {
            sTrMsgAt = BlockEnd(sEncountAt);
            sEyeSongsAt = BlockEnd(sTrMsgAt);
        }
        if (Get32(4) >= 6)   // (version 6: Black's warp points)
            sWarpsAt = sEyeSongsAt + 4 + 4 * Get32(sEyeSongsAt);
        if (Get32(4) >= 7)   // (version 7: the Xtransceiver's calls)
            sCallsAt = (sWarpsAt + 4 + 20 * Get32(sWarpsAt) + 3) & ~3;
        if (Get32(4) >= 8)   // (version 8: the in-game trades)
            sTradesAt = (BlockEnd(sCallsAt) + 3) & ~3;
        if (Get32(4) >= 9)   // (version 9: BLACK CITY's residents)
            sWfbcAt = sTradesAt + 4 + 0x6C * Get32(sTradesAt);
        if (Get32(4) >= 10)   // (version 10: the BATTLE SUBWAY's sets, trainers and tables)
        {
            sSubwayAt = (sWfbcAt + 4 + 0x30 * Get32(sWfbcAt) + 40 + 3) & ~3;
            sSubwayTrainersAt = sSubwayAt + 4 + 16 * Get32(sSubwayAt);
            sSubwayTablesAt = BlockEnd(sSubwayTrainersAt);
        }
    }
    printf("[u3d] UNOVA's scripts: %u files, %u zones\n", Get32(sFilesAt), Get32(sZonesAt));
}

static bool8 Ready(void)
{
    if (!sTried)
        Load();
    return sPack != NULL;
}

static const u8 *FileOf(u32 block, u32 index, u32 *size)
{
    if (!Ready() || index >= Get32(block))
    {
        *size = 0;
        return NULL;
    }
    *size = Get32(block + 8 + 8 * index);
    return sPack + Get32(block + 4 + 8 * index);
}

// Black's trainer ID's record (20 bytes) and team (u3d_battle.c); NULL without them
const u8 *U3D_Vm_Trainer(u16 id, u32 *size)
{
    return Ready() && sTrainersAt ? FileOf(sTrainersAt, id, size) : (*size = 0, NULL);
}

const u8 *U3D_Vm_TrainerTeam(u16 id, u32 *size)
{
    return Ready() && sTeamsAt ? FileOf(sTeamsAt, id, size) : (*size = 0, NULL);
}

// Black's trainer ID's battle song kind (tools/unova/build_scripts.py song_kinds: 0 a trainer .. 8 a Subway Boss)
u8 U3D_Vm_TrainerSongKind(u16 id)
{
    return Ready() && sSongKindsAt && id < Get32(sSongKindsAt) ? sPack[sSongKindsAt + 4 + id] : 0;
}

enum { Z_SCRIPT, Z_INIT, Z_MSG, Z_EVENT, Z_MATRIX, Z_PARENT, Z_TYPE, Z_PLACE, Z_ENCOUNT };
static u16 ZoneField(u16 zone, u32 field)
{
    if (!Ready() || zone >= Get32(sZonesAt))
        return 0xFFFF;
    if (field == Z_ENCOUNT && Get32(4) < 4)
        return 0xFFFF;
    return Get16(sZonesAt + 4 + ZoneSize() * zone + 2 * field);
}

// The zone's wild POKéMON row for SEASON (0xE8 bytes; tools/unova/build_scripts.py), NULL none
const u8 *U3D_Vm_WildRow(u16 zone, u8 season)
{
    u32 size;
    u16 id;
    const u8 *d;
    if (!Ready() || !sEncountAt)
        return NULL;
    id = ZoneField(zone, Z_ENCOUNT);
    if (id == 0xFFFF)
        return NULL;
    d = FileOf(sEncountAt, id, &size);
    if (d == NULL || (size != 0xE8 && size != 4 * 0xE8))
        return NULL;
    return size == 0xE8 ? d : d + 0xE8 * (season & 3);
}


// ------------------------------------------------------------------------------------------------ Black's event work
// (zonedata.c: works 0x4000 on, then the flags 1-0xB5F; 0x4000+ flags are temporary, 0x8000+ works the script's own)
#define WORKS 0x140
#define FLAG_BYTES 0x16C
struct UnovaRoamer { u16 species, hp; u8 level, state, loc, status; u32 personality, ivs; };   // (state: 0 none, 1
                                                                                                // roaming, 2 knocked out, 3 caught)
static struct { u16 work[WORKS]; u8 flags[FLAG_BYTES]; u8 began, badges, cgear;
                u8 white; struct UnovaRoamer roam[2];
                struct { u8 valid, npc[10], mood[10], flags[10], back[10]; u16 counter, value; } wfbc;
                struct U3DSubwaySave subway;
                u8 gym[8];
                struct { u16 team[8][PARTY_SIZE]; u16 mon[4]; u32 record[128];
                         struct { u16 national; u8 level, filled; u32 otId, personality;
                                  u8 nick[POKEMON_NAME_LENGTH + 1]; } snap[2]; } misc;
                u8 plateSwitches;   // (BURGH's gym's switches found, a bit each: Black's plate doors' save)
                u8 entralink[96];   // (the ENTRALINK's Pass Orbs, missions and PASS POWERS: u3d_entralink.c)
                u8 musical[96];     // (the MUSICALS' props, records and fans: u3d_musical.c)
                u8 stones[3][12][2], stonesPlaced;   // (CHARGESTONE CAVE's stones where they were pushed to, by
              } sWork;                                //  floor; placed: a bit a floor once laid out)   // (gym[0]: ELESA's gym's levers, a bit a lane: CartLanes below)   // (subway: the BATTLE SUBWAY's records, u3d_subway.c)   // (white: ZEKROM caught after the credits, bit 0)   // (began: Black's new-game
                                                // script has run; badges: UNOVA's, a bit each; cgear: the C-GEAR got)
u8 *U3D_Vm_BadgeFlags(void)
{
    return &sWork.badges;
}
static u8 sTempFlags[8];
static u16 sLocal[0x40];

void U3D_Vm_ResetWork(void)
{
    memset(&sWork, 0, sizeof(sWork));
    memset(sTempFlags, 0, sizeof(sTempFlags));
}

const void *U3D_Vm_WorkData(u32 *size)
{
    *size = sizeof(sWork);
    return &sWork;
}

void U3D_Vm_WorkLoad(const void *data, u32 size)
{
    U3D_Vm_ResetWork();
    memcpy(&sWork, data, size < sizeof(sWork) ? size : sizeof(sWork));
}

static u16 sDummyWork;
static u16 *WorkPtr(u16 id)
{
    if (id >= 0x4000 && id < 0x4000 + WORKS)
        return &sWork.work[id - 0x4000];
    if (id >= 0x8000 && id < 0xC000)
        return &sLocal[(id - 0x8000) & 0x3F];
    sDummyWork = 0;
    return &sDummyWork;
}

static u16 WorkValue(u16 id)
{
    return id < 0x4000 ? id : *WorkPtr(id);
}

// Emerald's vars U3D_VAR_BASE + n are Black's works 0x4000 + n (event_data.c GetVarPointer: an UNOVA map's trigger
// spots compare them as Black's do)
u16 *U3D_Vm_VarPointer(u16 id)
{
    return WorkPtr(id - U3D_VAR_BASE + 0x4000);
}

static u8 *FlagByte(u16 id, u8 *bit)
{
    *bit = 1 << (id % 8);
    if (id == 0)
        return NULL;
    if (id < 0x4000)
        return (id >> 3) < FLAG_BYTES ? &sWork.flags[id >> 3] : NULL;
    return ((id - 0x4000) >> 3) < sizeof(sTempFlags) ? &sTempFlags[(id - 0x4000) >> 3] : NULL;
}

static bool8 FlagOn(u16 id)
{
    u8 bit, *p = FlagByte(id, &bit);
    return p && (*p & bit);
}

// Black's system flags that are the START menu's buttons (ov021 field_menu.c FIELD_MENU_GetLayoutType): 0x961 the
// POKeMON, 0x962 the POKeDEX (set by JUNIPER's lab, NUVEMA zone 396; the NATIONAL DEX: the user's decision, as
// SetZukanObtained). Kai's own flags follow them, or START never shows the POKeDEX; and Kai's running.
static void SysFlagsToKai(void)
{
    if (!FlagGet(FLAG_SYS_B_DASH))   // (Black runs from the start, B held: no RUNNING SHOES to give)
        FlagSet(FLAG_SYS_B_DASH);
    if (FlagOn(0x961) && !FlagGet(FLAG_SYS_POKEMON_GET))
        FlagSet(FLAG_SYS_POKEMON_GET);
    if (FlagOn(0x962) && !FlagGet(FLAG_SYS_POKEDEX_GET))
    {
        FlagSet(FLAG_SYS_POKEDEX_GET);
        EnableNationalPokedex();
    }
}

static void FlagTo(u16 id, bool8 on)
{
    u8 bit, *p = FlagByte(id, &bit);
    if (p)
        *p = on ? (*p | bit) : (*p & ~bit);
    if (on && (id == 0x961 || id == 0x962))
        SysFlagsToKai();
}

bool8 U3D_Vm_Flag(u16 id) { return FlagOn(id); }
void U3D_Vm_SetFlag(u16 id) { FlagTo(id, TRUE); }
void U3D_Vm_ClearFlag(u16 id) { FlagTo(id, FALSE); }

// ------------------------------------------------------------------------------------------------ the zone's events
static u16 CurrentZone(void)
{
    return U3D_CurrentZone();
}

// Black's event file of a zone: the counts, then the tables (tools/unova/events.py)
static const u8 *Events(u16 zone, u32 *size)
{
    u16 ev = ZoneField(zone, Z_EVENT);
    return ev == 0xFFFF ? (*size = 0, NULL) : FileOf(sEventsAt, ev, size);
}

#define EV_BG 0x14
#define EV_PERSON 0x24
#define EV_CONNECT 0x14
#define EV_POS 0x16

static const u8 *Person(u16 zone, u8 index)
{
    u32 size;
    const u8 *e = Events(zone, &size);
    if (e == NULL || size < 8 || index >= e[5])
        return NULL;
    return e + 8 + EV_BG * e[4] + EV_PERSON * index;
}

static u16 Le16(const u8 *p) { return p[0] | p[1] << 8; }

// ---- forms (src/mp/mp_forms.c): Black's numbers are the game's, but for DEOXYS (Black: 0 Normal, 1 Attack, 2
// Defense, 3 Speed; here 0 is the game's own Speed Forme, 1 Normal, 2 Attack, 3 Defense)
#include "mp_forms.h"
void MP_Form_Change(struct Pokemon *mon, u8 form);
void MP_Form_SetRotom(struct Pokemon *mon, u8 form, u8 slot);
u8 U3D_Vm_KaiForm(u16 species, u16 blackForm)
{
    if (species == SPECIES_DEOXYS)
        return blackForm >= 3 ? 0 : blackForm + 1;
    return blackForm;
}

static u16 BlackForm(u16 species, u8 form)
{
    if (species == SPECIES_DEOXYS)
        return form == 0 ? 3 : form - 1;
    return form;
}

// ---- Black's NPC table (a/2/0/9, the pack's version 11; ov010 scrcmd_network.c): NIMBASA's BIG STADIUM and SMALL
// COURT trainers by the stadium's place (key A) and the story's stage (key B), the crowd's looks; the script sets a
// person's event (EVENTDATA_SYSTEM_SetNpcEventId) or look (SetNpcObjCode) for as long as the player is in the zone
#define NPC_OVERRIDES 64
static u16 sNpcEvOver[NPC_OVERRIDES], sNpcCodeOver[NPC_OVERRIDES], sOverZone = 0xFFFF;   // (+1; 0: the record's own)
static void OverridesHere(void)
{
    if (sOverZone != U3D_CurrentZone())
    {
        memset(sNpcEvOver, 0, sizeof(sNpcEvOver));
        memset(sNpcCodeOver, 0, sizeof(sNpcCodeOver));
        sOverZone = U3D_CurrentZone();
    }
}

// a person's script: a script's change (the table's trainer), else their record's
static u16 PersonScript(u8 local, const u8 *p)
{
    OverridesHere();
    if (local >= 1 && local <= NPC_OVERRIDES && sNpcEvOver[local - 1])
        return sNpcEvOver[local - 1] - 1;
    return Le16(p + 10);
}

// event_object_movement.c TrySpawnObjectEventTemplate: a person's look as a script set it (SetNpcTripleFromTable)
u16 U3D_Vm_PersonGfx(u8 localId, u16 gfx)
{
    if (!U3D_IsUnovaGroup(gSaveBlock1Ptr->location.mapGroup))
        return gfx;
    OverridesHere();
    if (localId >= 1 && localId <= NPC_OVERRIDES && sNpcCodeOver[localId - 1])
        return OBJ_EVENT_GFX_UNOVA_FIRST + sNpcCodeOver[localId - 1] - 1;
    return gfx;
}

// ------------------------------------------------------------------------------------------------ the map's people
// The map's object event of Black's object id OBJ: its person's place in the zone's event data + 1
// (tools/unova/gen_maps.py writes them in that order); 0xFF is the player
static u8 WfbcSlotOf(u8 npc);
bool8 U3D_Vm_CrowdLocal(u8 local);
static bool8 Run(u16 id, u16 target, bool8 scene);
static u8 sWfbcSlot;   // (BLACK CITY: the resident talked to)
static u8 LocalId(u16 obj)
{
    u32 size, i;
    const u8 *e;
    if (obj == OBJ_PLAYER)
        return OBJ_EVENT_ID_PLAYER;
    if (U3D_CurrentZone() == 0 && obj >= 0x80 && obj < 0x80 + 30)   // (BLACK CITY's residents: 0x80 + their number)
    {
        u8 slot = WfbcSlotOf(obj - 0x80);
        if (slot < 10)
            return slot + 1;
    }
    e = Events(U3D_CurrentZone(), &size);
    for (i = 0; e && size >= 8 && i < e[5]; i++)
        if (Le16(e + 8 + EV_BG * e[4] + EV_PERSON * i) == obj)
            return i + 1;
    if (obj >= 0x30 && obj < 0x30 + 32 && U3D_Vm_CrowdLocal(0xA0 + obj - 0x30))   // (CASTELIA's crowd: below)
        return 0xA0 + obj - 0x30;
    return (u8)(obj + 1);
}

// POKeDEX counts as Black's (zukan_save.c): KIND 1 national caught, 2 UNOVA's seen, 3 UNOVA's caught; COUNTED leaves
// out the species Black's completion leaves out (sZukanNational / RegionalExcludedSpecies)
static u32 DexCount(u32 kind, bool8 counted)
{
    static const u16 sExcluded[15] = {151, 251, 385, 386, 489, 490, 491, 492, 493, 645, 646, 647, 648, 649, 494};
    u32 n, k, count = 0;
    for (n = kind == 1 ? 1 : 494; n <= 649; n++)
    {
        bool8 skip = FALSE;
        for (k = kind == 1 ? 0 : 9; counted && k < ARRAY_COUNT(sExcluded); k++)
            skip |= sExcluded[k] == n;
        if (!skip && n <= NATIONAL_DEX_COUNT && GetSetPokedexFlag(n, kind == 2 ? FLAG_GET_SEEN : FLAG_GET_CAUGHT))
            count++;
    }
    return count;
}

static u16 Item(u16 black);
static u16 Species(u16 national);
void MP_Daycare_Deposit(u8 partySlot);
u8 MP_Daycare_Compatibility(const struct BoxPokemon *a, const struct BoxPokemon *b);   // (daycare.c)
u8 MP_Daycare_LevelNow(u8 slot);
u8 ItemIdToBallId(u16 ballItem);

// a type of the game as Black numbers it (Black has no ??? type: the game's past it are one lower there)
static u16 BlackType(u8 type)
{
    return type > TYPE_MYSTERY ? type - 1 : type;
}

// Black's SCRCMD_PROC_AddMonToPartyDetailed (POKE_TOOL_CreatePersonalRnd): GENDER 0 male, 1 female, 2 any; ABILITY
// the personality's bit 16; SHINY 0 never, 1 always, 2 as it falls; into the party only (0: it's full)
static u16 GiveDetailedMon(u16 national, u8 level, u16 gender, u16 ability, u16 shiny, u16 item, u16 ball)
{
    struct Pokemon mon;
    u16 species = Species(national), kaiItem = Item(item), kaiBall = Item(ball);
    u32 personality, otId = T1_READ_32(gSaveBlock2Ptr->playerTrainerId), tries;
    u8 ballId;
    if (species == SPECIES_NONE || CalculatePlayerPartyCount() >= PARTY_SIZE)
        return 0;
    for (tries = 0; tries < 0x10000; tries++)
    {
        u8 g;
        bool8 isShiny;
        personality = Random32();
        if (gender < 2 && gSpeciesInfo[species].genderRatio != MON_MALE && gSpeciesInfo[species].genderRatio != MON_FEMALE
         && gSpeciesInfo[species].genderRatio != MON_GENDERLESS)
        {
            g = GetGenderFromSpeciesAndPersonality(species, personality);
            if ((gender == 0) != (g == MON_MALE))
                continue;
        }
        if (ability < 2 && ((personality >> 16) & 1) != ability)
            continue;
        isShiny = (((otId >> 16) ^ (otId & 0xFFFF) ^ (personality >> 16) ^ (personality & 0xFFFF)) < SHINY_ODDS);
        if ((shiny == 0 && isShiny) || (shiny == 1 && !isShiny))
            continue;
        break;
    }
    CreateMon(&mon, species, level, USE_RANDOM_IVS, TRUE, personality, OT_ID_PLAYER_ID, 0);
    SetMonData(&mon, MON_DATA_HELD_ITEM, &kaiItem);
    ballId = ItemIdToBallId(kaiBall);
    if (kaiBall != ITEM_NONE && ballId != 0)
        SetMonData(&mon, MON_DATA_POKEBALL, &ballId);
    if (GiveMonToPlayer(&mon) != MON_GIVEN_TO_PARTY)
        return 0;
    GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_SEEN);
    GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_CAUGHT);
    return 1;
}

// Black's event record of the person with object id OBJ in the zone, and the reverse (a local id's object id)
static const u8 *PersonById(u16 obj)
{
    u32 size, i;
    const u8 *e = Events(U3D_CurrentZone(), &size);
    for (i = 0; e && size >= 8 && i < e[5]; i++)
        if (Le16(e + 8 + EV_BG * e[4] + EV_PERSON * i) == obj)
            return e + 8 + EV_BG * e[4] + EV_PERSON * i;
    return NULL;
}

static u16 BlackObjOf(u8 localId)
{
    const u8 *p = Person(U3D_CurrentZone(), localId - 1);
    return p ? Le16(p) : localId - 1;
}

// a person's move code (Black's MV_ numbers): a script's change (ChangeMmdlMoveCode) while in the zone, else the record's
static u8 ObjEventId(u8 localId);
static struct { u16 obj, code; } sMoveCodes[8];
static u16 MoveCodeOf(u16 obj)
{
    u32 i;
    const u8 *p;
    for (i = 0; i < ARRAY_COUNT(sMoveCodes); i++)
        if (sMoveCodes[i].code != 0 && sMoveCodes[i].obj == obj)
            return sMoveCodes[i].code - 1;
    p = PersonById(obj);
    return p ? Le16(p + 4) : 0;
}

static void ChangeMoveCode(u16 obj, u16 code)
{
    u32 i, free = ARRAY_COUNT(sMoveCodes);
    u8 slot;
    for (i = 0; i < ARRAY_COUNT(sMoveCodes); i++)
    {
        if (sMoveCodes[i].code != 0 && sMoveCodes[i].obj == obj)
            break;
        if (sMoveCodes[i].code == 0 && free == ARRAY_COUNT(sMoveCodes))
            free = i;
    }
    if (i == ARRAY_COUNT(sMoveCodes))
        i = free;
    if (i < ARRAY_COUNT(sMoveCodes))
        sMoveCodes[i].obj = obj, sMoveCodes[i].code = code + 1;
    if ((slot = ObjEventId(LocalId(obj))) < OBJECT_EVENTS_COUNT)
    {
        // (Black's wandering kind 3 wanders; any other stands, facing the way it faces: the game's people have those)
        u8 type = code == 3 ? MOVEMENT_TYPE_WANDER_AROUND
                : (gObjectEvents[slot].facingDirection == DIR_NORTH ? MOVEMENT_TYPE_FACE_UP
                 : gObjectEvents[slot].facingDirection == DIR_WEST ? MOVEMENT_TYPE_FACE_LEFT
                 : gObjectEvents[slot].facingDirection == DIR_EAST ? MOVEMENT_TYPE_FACE_RIGHT : MOVEMENT_TYPE_FACE_DOWN);
        SetTrainerMovementType(&gObjectEvents[slot], type);
    }
}

// The object event of a local id: on the player's map, else on any map (Black's people live by zone, and a scene's
// people walk with the player over a map's edge, or were added on the map before: AddMmdl)
static u8 ObjEventId(u8 localId)
{
    u8 id;
    // (while a map loads, its people aren't set out yet: those in the table are the last map's, sprites gone; a
    // script moves their templates then: InitMmdlGridPosition)
    if (gMain.callback2 != CB2_Overworld && gMain.callback2 != CB2_OverworldBasic)
        return OBJECT_EVENTS_COUNT;
    if (!TryGetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &id))
        return id;
    // (only a scene's own people, Black's object ids from 0x80: AddMmdl's; never a person of the map left behind,
    // whose sprite may be gone as the next map loads)
    for (id = 0; localId > 0x80 && id < OBJECT_EVENTS_COUNT; id++)
        if (gObjectEvents[id].active && !gObjectEvents[id].isPlayer && gObjectEvents[id].localId == localId
         && U3D_IsUnovaGroup(gObjectEvents[id].mapGroup) && gSprites[gObjectEvents[id].spriteId].inUse)
            return id;
    return OBJECT_EVENTS_COUNT;
}

static struct ObjectEvent *ObjectOf(u16 obj)
{
    u8 id = ObjEventId(LocalId(obj));
    return id < OBJECT_EVENTS_COUNT ? &gObjectEvents[id] : NULL;
}

// event_object_movement.c: a person of the map hidden by Black's flag (its event data's vanish flag)
// People a zone's script set somewhere else (Black's SetNpcGridPos: EVENTDATA_SYSTEM_SetNpcGridPos, CHEREN waiting at
// NACRENE's gate): Black's tile and direction, kept while the player is in the zone and put on each of its maps
#define NPC_AT_MAX 16
static struct { u8 local, dir; u16 x, z; } sNpcAt[NPC_AT_MAX];
static u8 sNpcAtCount;

static bool8 NpcAt(u8 local, u16 *x, u16 *z)
{
    u32 i;
    for (i = 0; i < sNpcAtCount; i++)
        if (sNpcAt[i].local == local)
        {
            *x = sNpcAt[i].x, *z = sNpcAt[i].z;
            return TRUE;
        }
    return FALSE;
}

// People a script deleted that have no flag of their own (CUT's trees, a boulder...): gone until the map is loaded
// again, as Black's come back on re-entering (the game would put them back the moment they come into view)
static u8 sGone[16], sGoneCount;

static void PlateDoorsRefresh(bool8 sound);
static void MineLift(u32 i);
#define ZONE_DRIFTVEIL_GYM 97
static bool8 TrySign(void);
static bool8 MineFloorHeight(s16 h);
static void Gone(u8 local, bool8 gone)
{
    u32 i;
    for (i = 0; i < sGoneCount; i++)
        if (sGone[i] == local)
        {
            if (!gone)
                sGone[i] = sGone[--sGoneCount];
            return;
        }
    if (gone && sGoneCount < ARRAY_COUNT(sGone))
        sGone[sGoneCount++] = local;
}

// White's ZEKROM after the credits (the user, 2026-11-12: White's exclusives in UNOVA): at the top of DRAGONSPIRAL
// TOWER (zone 213), where Black puts the player's RESHIRAM after the credits if it was knocked out at N's Castle
// (its person 3, flag 766 cleared): there, once Black's own isn't (its flag set), until caught. Drawn as Black's own
// ZEKROM (work object code 0xD6); its words are White's lines in Black's bank of N's throne room (zone 278)
#define ZONE_ZEKROM 213
#define ZONE_BLACK_CITY 0
#define ZONE_THRONE 278
#define ZEKROM_LOCAL 4
static bool8 WhiteZekromHere(u8 localId);
static void RoamersMove(void);
static u16 WfbcValue(u16 query);
static void WfbcMapLoad(void);
static void ConnMove(u8 event, s16 x, s16 z);
#define CONN_MOVES 8
static struct { u8 event, set; s16 x, z; } sConnMove[CONN_MOVES];
// WORK_CmdSetBgEventGridPos: a check spot of the zone's moved by its scripts (NACRENE's gym's shelves), as long as the
// player is in the zone (cleared with the moved connections)
#define BG_MOVES 8
static struct { u16 event; s16 x, z; bool8 set; } sBgMove[BG_MOVES];
static void RoamSpawn(u8 slot, u16 national);

bool8 U3D_Vm_ObjectHidden(u8 localId)
{
    const u8 *p;
    if (!U3D_IsUnovaGroup(gSaveBlock1Ptr->location.mapGroup) || localId == 0 || localId >= OBJ_EVENT_ID_PLAYER)
        return FALSE;
    {
        u32 i;
        for (i = 0; i < sGoneCount; i++)
            if (sGone[i] == localId)
                return TRUE;
    }
    if (CurrentZone() == ZONE_BLACK_CITY && localId <= 10)
        return sWork.wfbc.npc[localId - 1] == 0xFF || !sWork.wfbc.valid;   // (BLACK CITY's residents: the slot's own)
    p = Person(CurrentZone(), localId - 1);
    if (p == NULL)
        return FALSE;
    if (WhiteZekromHere(localId))
        return FALSE;
    if (Le16(p + 8) != 0 && FlagOn(Le16(p + 8)))   // (MmdlHeader: id, code, move, type, flag)
        return TRUE;
    if (CurrentZone() == ZONE_DRIFTVEIL_GYM && (Le16(p + 2) == 185 || Le16(p + 2) == 0))
        return TRUE;   // (CLAY's gym's unseen blockers, a floor each, kept as data: U3D_Vm_BlockerAt; as people they'd
                       // fill the game's 16 and its trainers and CLAY would never come out)
    {
        // (a person whose spot is on another of the zone's maps: tools/unova/gen_maps.py keeps its place in the list)
        s32 m = U3D_MapIndex(), x = Le16(p + 28), z = Le16(p + 30);
        u16 ax, az;
        if (NpcAt(localId, &ax, &az))
            x = ax, z = az;
        if (m >= 0 && Le16(p + 24) == 0)
        {
            x -= 32 * U3D_MapCellX(m), z -= 32 * U3D_MapCellZ(m);
            if (x < 0 || z < 0 || x >= gMapHeader.mapLayout->width || z >= gMapHeader.mapLayout->height)
                return TRUE;
        }
    }
    return FALSE;
}

static u8 BlackToDir(u16 d)   // (Black: up, down, left, right)
{
    static const u8 sDirs[4] = {DIR_NORTH, DIR_SOUTH, DIR_WEST, DIR_EAST};
    return sDirs[d & 3];
}

static u8 DirToBlack(u8 d)
{
    switch (d)
    {
    case DIR_NORTH: return 0;
    case DIR_WEST:  return 2;
    case DIR_EAST:  return 3;
    default:        return 1;
    }
}

// Black's tile coordinates (the matrix's) -> the map's own
static s32 MapX(u16 x) { s32 i = U3D_MapIndex(); return i < 0 ? x : x - 32 * U3D_MapCellX(i); }
static s32 MapZ(u16 z) { s32 i = U3D_MapIndex(); return i < 0 ? z : z - 32 * U3D_MapCellZ(i); }

// ------------------------------------------------------------------------------------------------ movement lists
// Black's movement codes (the ACMD table, ov021 0x021D5F58) as Emerald's movement actions. Black's frames are its
// field's 30 a second: 8 frames a tile is Emerald's normal walk
#define MAX_MOVE 96
static u8 sMoves[OBJECT_EVENTS_COUNT][MAX_MOVE];
static u8 sMoving[OBJECT_EVENTS_COUNT];   // local ids moving for the script
static u16 sPendingScript;                 // Black's SetProcAction: a script to run next
static u8 sSubText[400] = {EOS}, sSubCount;   // Black's side windows' words (AddSubWinMsg; empty: EOS, not 0, a space)

// ---- people a script added (Black's AddMmdl): kept until the map changes or the script deletes them, and put on the
// field once it is up (a zone's field-init script adds them while the map loads, before the map's people are set out)
static struct { u8 local, dir; u16 code, x, z; } sAdded[8];
static u8 sAddedCount;

static u8 BlackToDir(u16 d);
static s32 MapX(u16 x);
static s32 MapZ(u16 z);

static void SpawnAdded(void)
{
    u32 i;
    if (gMain.callback2 != CB2_Overworld)
        return;
    for (i = 0; i < sAddedCount; i++)
    {
        u8 id;
        if (ObjEventId(sAdded[i].local) < OBJECT_EVENTS_COUNT)
            continue;
        id = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_UNOVA_FIRST + sAdded[i].code, MOVEMENT_TYPE_FACE_DOWN,
                                                  sAdded[i].local, MapX(sAdded[i].x) + MAP_OFFSET,
                                                  MapZ(sAdded[i].z) + MAP_OFFSET, 3);
        if (id < OBJECT_EVENTS_COUNT)
            ObjectEventTurn(&gObjectEvents[id], BlackToDir(sAdded[i].dir));
    }
}

static void AddPerson(u8 local, u16 code, u16 x, u16 z, u16 dir)
{
    u32 i;
    for (i = 0; i < sAddedCount && sAdded[i].local != local; i++)
        ;
    if (i == sAddedCount && sAddedCount < ARRAY_COUNT(sAdded))
        sAddedCount++;
    if (i < ARRAY_COUNT(sAdded))
        sAdded[i].local = local, sAdded[i].code = code, sAdded[i].x = x, sAdded[i].z = z, sAdded[i].dir = dir;
    SpawnAdded();
}

// The BATTLE SUBWAY's commands about the field (u3d_subway.c hands them back): the trains' places, where the game goes
// on after a REST, the people on the platforms and in the cars
void SetContinueGameWarp(s8 mapGroup, s8 mapNum, s8 warpId, s16 x, s16 y);   // (overworld.c)
static u16 sPassengerParam[OBJECT_EVENTS_COUNT];
static void SubwayReturnTo(u16 zone, s16 x, s16 z)
{
    u8 group, num;
    s16 mx, my;
    if (U3D_MapOfZoneTile(zone, x, z, &group, &num, &mx, &my))
    {
        SetContinueGameWarp(group, num, WARP_ID_NONE, mx, my);
        SetContinueGameWarpStatus();
        FlagTo(0x966, TRUE);
    }
}

static void SubwayField(u16 cmd, u16 op1, u16 op2, u16 *result)
{
    u16 zone;
    s16 x, z;
    switch (cmd)
    {
    case 0:   // GET_TRAIN_INDEX (the platform's line)
        switch (CurrentZone())
        {
        case 0x43: *result = 0; break;
        case 0x44: *result = 5; break;
        case 0x45: *result = 1; break;
        case 0x46: *result = 6; break;
        case 0x47: *result = 2; break;
        case 0x48: *result = 7; break;
        case 0x49: *result = 4; break;
        default:   *result = 0; break;
        }
        break;
    case 4:   // SET_RETURN_LOCATION_AT_PLAYER
        SetContinueGameWarp(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, WARP_ID_NONE,
                            gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y);
        SetContinueGameWarpStatus();
        FlagTo(0x966, TRUE);
        break;
    case 5:   // CLEAR_RETURN_LOCATION_FLAG
        ClearContinueGameWarpStatus();
        FlagTo(0x966, FALSE);
        break;
    case 45:   // SET_RETURN_LOCATION_FROM_TABLE (by the platform's line)
    {
        u16 line;
        SubwayField(0, 0, 0, &line);
        if (U3D_Subway_Return(line, &zone, &x, &z))
            SubwayReturnTo(zone, x, z);
        break;
    }
    case 322:   // SET_RETURN_LOCATION_FROM_MODE
        if (U3D_Subway_Return(U3D_Subway_PlayMode(), &zone, &x, &z))
            SubwayReturnTo(zone, x, z);
        break;
    case 13:   // (SET_NPC_DIRECTION: a person shown or hidden)
    {
        u8 id = ObjEventId(LocalId(op1));
        if (id < OBJECT_EVENTS_COUNT)
            gObjectEvents[id].invisible = op2 != 0;
        break;
    }
    case 25:   // IS_NPC_PRESENT
        *result = ObjEventId(LocalId(op1)) < OBJECT_EVENTS_COUNT;
        break;
    case 26:   // GET_NPC_TRAIN_STATE (a passenger's parameter)
    {
        u8 local = LocalId(op1);
        *result = local < OBJECT_EVENTS_COUNT ? sPassengerParam[local] : 0;
        break;
    }
    case 329:   // SPAWN_FIELD_OBJECTS: the car's passengers by how far this streak has come
    {
        u32 i;
        u16 code, param, obj = 0x80;
        for (i = 0; U3D_Subway_Passenger(i, &code, &x, &z, &param); i++)
        {
            u8 local;
            if (code == 0xFFFF)
                continue;
            local = LocalId(obj);
            AddPerson(local, code, x, z, i % 4);
            if (local < OBJECT_EVENTS_COUNT)
                sPassengerParam[local] = param;
            if (obj < 0xFF)
                obj++;
        }
        break;
    }
    default:   // (23, 24 heights; 35, 36 animations; 37 a Wi-Fi record's balloon: nothing to do here)
        break;
    }
}

static void ForgetPerson(u8 local)
{
    u32 i;
    for (i = 0; i < sAddedCount; i++)
        if (sAdded[i].local == local)
        {
            sAdded[i] = sAdded[--sAddedCount];
            return;
        }
}

static u8 Dir4(u8 code, u8 base)   // Black's U D L R at code - base -> Emerald's D U L R offset
{
    static const u8 sOff[4] = {1, 0, 2, 3};
    return sOff[(code - base) & 3];
}

static u32 AddMoves(u8 *out, u32 n, u16 code)
{
#define PUT(a) do { if (n < MAX_MOVE - 2) out[n++] = (a); } while (0)
    static const u8 sStepMove[12] = {0, 1, 2, 3, 0, 1, 2, 2, 0, 1, 2, 3};   // (Step* / Hop* / DashStep*: the way walked)
    static const u8 sStepFace[12] = {2, 3, 1, 0, 3, 2, 0, 1, 1, 0, 3, 2};   // (... and the way faced after, Black's U D L R)
    if (code <= 0x03)
        PUT(MOVEMENT_ACTION_FACE_DOWN + Dir4(code, 0x00));
    else if (code <= 0x0B)
        PUT(MOVEMENT_ACTION_WALK_SLOW_DOWN + Dir4(code, 0x04));
    else if (code <= 0x0F)
        PUT(MOVEMENT_ACTION_WALK_NORMAL_DOWN + Dir4(code, 0x0C));
    else if (code <= 0x13)
        PUT(MOVEMENT_ACTION_WALK_FAST_DOWN + Dir4(code, 0x10));
    else if (code <= 0x17)
        PUT(MOVEMENT_ACTION_WALK_FASTER_DOWN + Dir4(code, 0x14));
    else if (code <= 0x1F)
        PUT(MOVEMENT_ACTION_WALK_IN_PLACE_SLOW_DOWN + Dir4(code, 0x18));
    else if (code <= 0x23)
        PUT(MOVEMENT_ACTION_WALK_IN_PLACE_NORMAL_DOWN + Dir4(code, 0x20));
    else if (code <= 0x27)
        PUT(MOVEMENT_ACTION_WALK_IN_PLACE_FAST_DOWN + Dir4(code, 0x24));
    else if (code <= 0x2B)
        PUT(MOVEMENT_ACTION_WALK_IN_PLACE_FASTER_DOWN + Dir4(code, 0x28));
    else if (code <= 0x33)
        PUT(MOVEMENT_ACTION_JUMP_IN_PLACE_DOWN + Dir4(code, 0x2C));
    else if (code <= 0x37)
        PUT(MOVEMENT_ACTION_JUMP_DOWN + Dir4(code, 0x34));
    else if (code <= 0x3B)
        PUT(MOVEMENT_ACTION_JUMP_2_DOWN + Dir4(code, 0x38));
    else if (code <= 0x42)
    {
        static const u8 sWait[7] = {2, 4, 8, 16, 30, 32, 64};   // (Wait 1, 2, 4, 8, 15, 16, 32 of Black's frames)
        u32 f = sWait[code - 0x3C];
        while (f >= 16) { PUT(MOVEMENT_ACTION_DELAY_16); f -= 16; }
        if (f >= 8) { PUT(MOVEMENT_ACTION_DELAY_8); f -= 8; }
        if (f >= 4) { PUT(MOVEMENT_ACTION_DELAY_4); f -= 4; }
        if (f >= 2) { PUT(MOVEMENT_ACTION_DELAY_2); f -= 2; }
    }
    else if (code == 0x45)
        PUT(MOVEMENT_ACTION_SET_INVISIBLE);
    else if (code == 0x46)
        PUT(MOVEMENT_ACTION_SET_VISIBLE);
    else if (code == 0x47)
        PUT(MOVEMENT_ACTION_LOCK_FACING_DIRECTION);
    else if (code == 0x48)
        PUT(MOVEMENT_ACTION_UNLOCK_FACING_DIRECTION);
    else if (code == 0x4B || code == 0x67 || code == 0x99)
        PUT(MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK);
    else if (code >= 0x4C && code <= 0x4F)
        PUT(MOVEMENT_ACTION_WALK_NORMAL_DOWN + Dir4(code, 0x4C));
    else if (code >= 0x50 && code <= 0x53)
        PUT(MOVEMENT_ACTION_WALK_FAST_DOWN + Dir4(code, 0x50));
    else if (code >= 0x54 && code <= 0x57)
        PUT(MOVEMENT_ACTION_WALK_FASTER_DOWN + Dir4(code, 0x54));
    else if (code >= 0x58 && code <= 0x5B)
        PUT(MOVEMENT_ACTION_WALK_FAST_DOWN + Dir4(code, 0x58));
    else if (code == 0x5C || code == 0x5D)
        PUT(MOVEMENT_ACTION_JUMP_DOWN + (code == 0x5C ? 2 : 3));
    else if (code == 0x5E || code == 0x5F)
        PUT(MOVEMENT_ACTION_JUMP_2_DOWN + (code == 0x5E ? 2 : 3));
    else if (code >= 0x60 && code <= 0x63)
        PUT(MOVEMENT_ACTION_WALK_NORMAL_DOWN + Dir4(code, 0x60));
    else if (code >= 0x69 && code <= 0x74)   // Step... Face... 8F
    {
        PUT(MOVEMENT_ACTION_WALK_NORMAL_DOWN + Dir4(sStepMove[code - 0x69], 0));
        PUT(MOVEMENT_ACTION_FACE_DOWN + Dir4(sStepFace[code - 0x69], 0));
    }
    else if (code >= 0x75 && code <= 0x78)
        PUT(MOVEMENT_ACTION_JUMP_2_DOWN + Dir4(code, 0x75));
    else if (code >= 0x79 && code <= 0x84)   // DashStep... 4F
    {
        PUT(MOVEMENT_ACTION_WALK_FAST_DOWN + Dir4(sStepMove[code - 0x79], 0));
        PUT(MOVEMENT_ACTION_FACE_DOWN + Dir4(sStepFace[code - 0x79], 0));
    }
    else if (code >= 0x85 && code <= 0x90)   // Hop... 8F: a hop on the spot, turning
    {
        PUT(MOVEMENT_ACTION_JUMP_IN_PLACE_DOWN + Dir4(sStepFace[code - 0x85], 0));
    }
    else if (code >= 0x91 && code <= 0x98)   // Step(dir)Face(opposite) 4F / 2F: a step back, still facing
    {
        static const u8 sBack[4] = {0, 1, 2, 3};
        u8 d = sBack[(code - 0x91) & 3];
        PUT(MOVEMENT_ACTION_LOCK_FACING_DIRECTION);
        PUT((code < 0x95 ? MOVEMENT_ACTION_WALK_FAST_DOWN : MOVEMENT_ACTION_WALK_FASTER_DOWN) + Dir4(d, 0));
        PUT(MOVEMENT_ACTION_UNLOCK_FACING_DIRECTION);
    }
    else if (code >= 0x9B && code <= 0x9E)
        PUT(MOVEMENT_ACTION_WALK_SLOW_DOWN + Dir4(code, 0x9B));
    else if (code == 0x9F)
        PUT(MOVEMENT_ACTION_EMOTE_QUESTION_MARK);
    else if (code == 0xA0)
        PUT(MOVEMENT_ACTION_EMOTE_HEART);
    else if (code >= 0xA3 && code <= 0xA6)
        PUT(MOVEMENT_ACTION_WALK_FAST_DOWN + Dir4(code, 0xA3));
    else if (code >= 0xA7 && code <= 0xAA)
        PUT(MOVEMENT_ACTION_WALK_SLOW_DOWN + Dir4(code, 0xA7));
    // (the rest: draw-state switches and effects Kai's people don't have)
    return n;
#undef PUT
}

static void StartMoves(u16 obj, const u8 *list, const u8 *end)
{
    // (a buffer per object event, never shared: the game's movement task keeps pointing into a finished list, so a
    // list written over another's buffer would walk that person too)
    u8 slot, localId = LocalId(obj);
    u32 n = 0;
    u8 *out;
    if ((slot = ObjEventId(localId)) >= OBJECT_EVENTS_COUNT)
        return;
    out = sMoves[slot];
    while (list + 4 <= end)
    {
        u16 code = Le16(list), count = Le16(list + 2), k;
        list += 4;
        if (code == 0xFE)
            break;
        for (k = 0; k < count && k < 32; k++)
            n = AddMoves(out, n, code);
    }
    out[n] = MOVEMENT_ACTION_STEP_END;
    sMoving[slot] = localId;
    ScriptMovement_StartObjectMovementScript(localId, gObjectEvents[slot].mapNum, gObjectEvents[slot].mapGroup, out);
}

static bool8 MovesDone(void)
{
    u8 slot;
    bool8 done = TRUE;
    for (slot = 0; slot < OBJECT_EVENTS_COUNT; slot++)
    {
        if (sMoving[slot] == 0)
            continue;
        if (!gObjectEvents[slot].active || gObjectEvents[slot].localId != sMoving[slot]
         || ScriptMovement_IsObjectMovementFinished(sMoving[slot], gObjectEvents[slot].mapNum, gObjectEvents[slot].mapGroup))
            sMoving[slot] = 0;
        else
            done = FALSE;
    }
    return done;
}

// trainers' eyes (the end of this file)
static struct EyeHit { u16 obj, trainer; u8 local, distance, dir; } sEye[2];
static u8 sEyeKind;   // (0 one trainer, 1 two who each saw the player, 2 a pair: GetTrainerHitKind)
static u16 TrainerOfScript(u16 script);
static void StartEye(u8 slot);
static u16 TrainerWords(u16 trainer, u16 kind);
static u16 EyeSong(u16 trainer);
static bool8 IceRide(u16 req);   // (BRYCEN's gym's slides: the end of this file)
static bool8 DragonHop(u16 direction);   // (DRAYDEN's gym's dragons: the end of this file)
static bool8 DragonLever(u16 unit, u16 side);
static bool8 DragonRotate(u16 unit);
static void DragonModels(void);
static void DragonInside(void);
static u8 sIceShown[3];
static void IceToggle(u32 i);
static struct { s8 lift; bool8 rider; u16 stall; float cur, goal, step; } sMineRide = {-1};
static struct { s8 model; u8 hex; s16 x, z, w, d; float cur, goal, step; } sPlat = {-1};
// (a lift's texture track played as it arrives (Black's lift events: the shown anime unpaused), then IDLE at its first
// frame, the event waiting for it)
static struct { s8 model; u8 track, idle; } sLiftTex = {-1};
static void PlatRide(u8 model, u8 hex, s16 x, s16 z, s16 w, s16 d, s16 from, s16 to);
static void LiftTexShow(u8 model, u8 track);
static void IceSecondary(u32 i);
#define ZONE_CLAY_LOBBY 98
#define ZONE_MISTRALTON_GYM 108
#define ZONE_NIMBASA_GYM 63
static void CartStep(void);
static void CartModels(void);
static void CartSwap(u32 lane);
static void CartLever(u32 lane);
static void CartTex(u32 lane);
static void CartDone(void);
static void DragonReset(void);
static s8 D06Floor(void);
static void D06Load(void);
static void D06Frame(void);
static void LanesLoad(void);
static void LanesFrame(void);
static void SoundObjsLoad(void);
static void SoundObjsFrame(void);
static void BridgeFrame(void);
static void LeagueFrame(void);
static void LeagueLoad(void);
static bool8 LeagueLandStart(void);
static bool8 LeagueWarpNow(void);
static bool8 LeagueBusy(void);

// ------------------------------------------------------------------------------------------------ the VM
// (BURGH's gym, the honey's trainers: Black's StartScriptedEvent run after its script, PlateDoorsFrame)
static u8 sPdScripted, sPdScriptedVariant;
static const float sPdEffectPos[2][3] = {{616, 0, 408}, {328, 0, 472}};   // (gPlateDoorsEffectPos)
enum { W_BGM_STOPPED = 100, W_EXP_ANIM, W_LIFT_RIDE, W_MUSICAL, W_SUBWAY_MAP, W_BMODEL, W_BMODEL_DOOR, W_SHAKE, W_TIMEFX, W_ICA, W_MOTION,
       W_ARC_GLOW, W_ARC_FLY, W_ARC_PEAK, W_ARC_END, W_LEAGUE };
static u16 sBModelWait;
// zone 389's time effect (ov065): sWork.white's TIMEFX_PENDING is Black's isEffectPending (its gimmick save 0x36)
#define ZONE_TIME_EFFECT 389
#define IS_TIME_EFFECT_ZONE(z) ((z) == ZONE_TIME_EFFECT || (z) == 397 || (z) == 317)   // (arc 0x65: gimmick 54 in all three)
#define TIMEFX_PENDING 0x80
static struct { bool8 playing, night, peaked; } sTimeFx;
static u8 TimeZoneNow(void)   // (Black's time of day: 0 morning, 1 day, 2 evening, 3 night, 4 late night; the clock
{                             // read again at most every ten seconds)
    static u8 sTz;
    static u32 sAt;
    u8 h;
    if (sAt == 0 || gMain.vblankCounter1 - sAt >= 600)
    {
        RtcCalcLocalTime();
        h = gLocalTime.hours;
        sTz = h < 5 ? 4 : h < 10 ? 0 : h < 17 ? 1 : h < 20 ? 2 : 3;
        sAt = gMain.vblankCounter1 | 1;
    }
    return sTz;
}

// CASTELIA's crowds (ov055 field_gimmick_crowd.c, started by command 0x134 in zones 32-34 and 254): people spawned
// on an area's entry edge as often as the time of day says (field frames), walking its length and gone; a solid
// one steps aside from anyone up to two tiles ahead, one in ten (with none ahead of it) is player-aware: one tile
// ahead of the player it stops and can be talked to (its look's script), else it steps aside and turns solid.
// Black's object ids 0x30 + slot are local ids 0xA0 + slot here. Tables: u3d_crowds.h (tools/unova/gen_crowds.py)
#include "u3d_crowds.h"
#define CROWD_MAX 32
#define CROWD_LOCAL 0xA0
enum { CROWD_WAIT, CROWD_NEXT, CROWD_LEAVE };
static struct { u8 active, state, aware, dir, sidestep, sideDir, area; u16 steps, script; } sCrowdPeople[CROWD_MAX];
static struct { s8 cfg; u16 zone; u16 counter[2]; u8 tick; } sCrowd = {-1};

bool8 U3D_Vm_CrowdLocal(u8 local)
{
    return sCrowd.cfg >= 0 && local >= CROWD_LOCAL && local < CROWD_LOCAL + CROWD_MAX
        && sCrowdPeople[local - CROWD_LOCAL].active;
}

static struct ObjectEvent *CrowdObject(u32 i)
{
    u8 id = ObjEventId(CROWD_LOCAL + i);
    return id < OBJECT_EVENTS_COUNT ? &gObjectEvents[id] : NULL;
}

static void CrowdRemove(u32 i)
{
    sCrowdPeople[i].active = FALSE;
    RemoveObjectEventByLocalIdAndMap(CROWD_LOCAL + i, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
}

// someone (the player too) MIN to MAX tiles straight ahead of crowd person I, where they stand or come from
static bool8 CrowdTileAhead(u32 i, s16 ox, s16 oz, s16 x, s16 z, s16 maxD, s16 minD)
{
    s32 ahead, side;
    switch (sCrowdPeople[i].dir)
    {
    case 0:  ahead = oz - z, side = ox - x; break;
    case 1:  ahead = z - oz, side = ox - x; break;
    case 2:  ahead = ox - x, side = oz - z; break;
    default: ahead = x - ox, side = oz - z; break;
    }
    return ahead <= maxD && ahead >= minD && side == 0;
}

static bool8 CrowdInFront(u32 i, const struct ObjectEvent *me, const struct ObjectEvent *o, s16 maxD, s16 minD)
{
    return CrowdTileAhead(i, me->currentCoords.x, me->currentCoords.y, o->currentCoords.x, o->currentCoords.y, maxD, minD)
        || CrowdTileAhead(i, me->currentCoords.x, me->currentCoords.y, o->previousCoords.x, o->previousCoords.y, maxD, minD);
}

static struct ObjectEvent *CrowdBlocker(u32 i, const struct ObjectEvent *me, s16 maxD)
{
    u32 k;
    for (k = 0; k < OBJECT_EVENTS_COUNT; k++)
        if (gObjectEvents[k].active && &gObjectEvents[k] != me && !gObjectEvents[k].invisible
         && CrowdInFront(i, me, &gObjectEvents[k], maxD, 0))
            return &gObjectEvents[k];
    return NULL;
}

static void CrowdWalk(u32 i, struct ObjectEvent *o, u8 dir)
{
    u8 frames = sCrowds[sCrowd.cfg].stepFrames, d = BlackToDir(dir);
    ObjectEventClearHeldMovementIfActive(o);
    ObjectEventSetHeldMovement(o, frames <= 2 ? GetWalkFasterMovementAction(d) : frames <= 4 ? GetWalkFastMovementAction(d)
                                                                              : GetWalkNormalMovementAction(d));
}

static void CrowdSidestep(u32 i, struct ObjectEvent *me, const struct ObjectEvent *blocker)   // (SidestepPerson)
{
    static const u8 sOpposite[4] = {1, 0, 3, 2};
    const u16 *area = sCrowds[sCrowd.cfg].area[sCrowdPeople[i].area];
    u8 bdir = DirToBlack(blocker->facingDirection) & 3, nd, me_dir = sCrowdPeople[i].dir;
    s32 x, z;
    if (sCrowdPeople[i].sidestep)
        nd = sCrowdPeople[i].sideDir;
    else if (bdir == me_dir || me_dir == sOpposite[bdir])
        nd = (Random() & 1) == 0 ? (bdir + 2) & 3 : sOpposite[(bdir + 2) & 3];
    else
        nd = sOpposite[bdir];
    x = me->currentCoords.x - MAP_OFFSET, z = me->currentCoords.y - MAP_OFFSET;
    x += nd == 2 ? -1 : nd == 3 ? 1 : 0, z += nd == 0 ? -1 : nd == 1 ? 1 : 0;
    if (nd <= 1 ? (z < MapZ(area[0]) || z > MapZ(area[1])) : (x < MapX(area[2]) || x > MapX(area[3])))
        nd = sOpposite[nd];
    CrowdWalk(i, me, nd);
    sCrowdPeople[i].sidestep = TRUE, sCrowdPeople[i].sideDir = nd;
}

// a person on the area's entry edge, STAGGER steps on (SpawnPerson), unless another stands there
static void CrowdSpawn(u32 i, u32 a, u16 stagger)
{
    const u16 *area = sCrowds[sCrowd.cfg].area[a];
    u32 r = Random(), k, look;
    u16 dir = area[4], x, z;
    bool8 aware = FALSE, ahead = FALSE;
    u8 id;
    if (dir <= 1)
        x = area[2] + r % (area[3] - area[2] + 1), z = dir == 0 ? area[0] - stagger : area[0] + stagger;
    else
        x = dir == 2 ? area[2] - stagger : area[2] + stagger, z = area[0] + r % (area[1] - area[0] + 1);
    for (k = 0; k < CROWD_MAX; k++)   // (FindPersonAtSpawn: a player-aware one there, or on its way)
    {
        struct ObjectEvent *o;
        s32 sx, sz;
        if (!sCrowdPeople[k].active || !sCrowdPeople[k].aware || (o = CrowdObject(k)) == NULL)
            continue;
        sx = o->currentCoords.x - MAP_OFFSET - MapX(x), sz = o->currentCoords.y - MAP_OFFSET - MapZ(z);
        sx = sx > 0 ? 1 : sx < 0 ? -1 : 0, sz = sz > 0 ? 1 : sz < 0 ? -1 : 0;
        if (sx == 0 && sz == 0)
            return;   // (occupied)
        if (sx == (dir == 2 ? -1 : dir == 3 ? 1 : 0) && sz == (dir == 0 ? -1 : dir == 1 ? 1 : 0))
            ahead = TRUE;
    }
    if (!ahead && r % 10 == 0)
        aware = TRUE;
    look = Random() % (sCrowds[sCrowd.cfg].looks ? sCrowds[sCrowd.cfg].looks : 1);
    RemoveObjectEventByLocalIdAndMap(CROWD_LOCAL + i, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    id = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_UNOVA_FIRST + sCrowds[sCrowd.cfg].code[look], MOVEMENT_TYPE_NONE,
                                              CROWD_LOCAL + i, MapX(x) + MAP_OFFSET, MapZ(z) + MAP_OFFSET, 3);
    if (id >= OBJECT_EVENTS_COUNT)
        return;
    ObjectEventTurn(&gObjectEvents[id], BlackToDir(dir));
    sCrowdPeople[i].active = TRUE, sCrowdPeople[i].state = CROWD_NEXT, sCrowdPeople[i].aware = aware;
    sCrowdPeople[i].dir = dir, sCrowdPeople[i].area = a, sCrowdPeople[i].steps = area[5] - stagger;
    sCrowdPeople[i].sidestep = FALSE, sCrowdPeople[i].script = 2000;
    for (k = 0; k < ARRAY_COUNT(sCrowdLooks); k++)
        if (sCrowdLooks[k].code == sCrowds[sCrowd.cfg].code[look])
            sCrowdPeople[i].script = sCrowdLooks[k].script[r % 6];
}

static s32 CrowdFree(void)
{
    u32 i;
    for (i = 0; i < CROWD_MAX; i++)
        if (!sCrowdPeople[i].active)
            return i;
    return -1;
}

static void CrowdStart(void)   // (CROWD_Init: the zone's setup, its areas filled as the spawn rate would have)
{
    u32 i, a, n, k, off;
    u8 tz = TimeZoneNow();
    sCrowd.cfg = -1;
    for (i = 0; i < ARRAY_COUNT(sCrowds); i++)
        if (sCrowds[i].zone == CurrentZone())
            sCrowd.cfg = i;
    memset(sCrowdPeople, 0, sizeof(sCrowdPeople));
    if (sCrowd.cfg < 0 || sCrowds[sCrowd.cfg].interval[tz] <= 0)
        return;
    sCrowd.zone = CurrentZone(), sCrowd.counter[0] = sCrowd.counter[1] = 0;
    for (a = 0; a < sCrowds[sCrowd.cfg].areas && a < 2; a++)
    {
        n = sCrowds[sCrowd.cfg].area[a][5] * 6 / sCrowds[sCrowd.cfg].interval[tz];
        for (k = 0, off = 0; k < n; k++)
        {
            s32 f = CrowdFree();
            if (f < 0)
                break;
            CrowdSpawn(f, a, off / 6);
            off += sCrowds[sCrowd.cfg].interval[tz];
        }
    }
    for (i = 0, n = 0; i < CROWD_MAX; i++)
        n += sCrowdPeople[i].active;
    MPNet_Log("u3d vm: CASTELIA's crowd in zone %u, %u people", CurrentZone(), (unsigned)n);
}

static void CrowdFrame(void)
{
    u32 i, a;
    u8 tz;
    if (sCrowd.cfg < 0)
        return;
    if (CurrentZone() != sCrowd.zone)
    {
        sCrowd.cfg = -1;   // (the zone left: its people went with the map)
        return;
    }
    tz = TimeZoneNow();
    if ((sCrowd.tick ^= 1) && sCrowds[sCrowd.cfg].interval[tz] > 0)   // (UpdateSpawns, a field frame each)
        for (a = 0; a < sCrowds[sCrowd.cfg].areas && a < 2; a++)
        {
            s32 f;
            if (sCrowd.counter[a] < sCrowds[sCrowd.cfg].interval[tz])
                sCrowd.counter[a]++;
            else if ((f = CrowdFree()) >= 0)
            {
                CrowdSpawn(f, a, 0);
                sCrowd.counter[a] = 0;
            }
        }
    for (i = 0; i < CROWD_MAX; i++)
    {
        struct ObjectEvent *o, *b;
        if (!sCrowdPeople[i].active)
            continue;
        if ((o = CrowdObject(i)) == NULL)
        {
            sCrowdPeople[i].active = FALSE;
            continue;
        }
        if (ScriptContext_IsEnabled() && sCrowdPeople[i].aware && sCrowdPeople[i].script == 2000)
            sCrowdPeople[i].aware = FALSE;   // (MakePersonSolid: a game event running)
        if (sCrowdPeople[i].state == CROWD_WAIT)
        {
            if (!ObjectEventClearHeldMovementIfFinished(o) && o->heldMovementActive)
                continue;
            sCrowdPeople[i].state = CROWD_NEXT;
        }
        if (sCrowdPeople[i].steps == 0)
        {
            CrowdRemove(i);
            continue;
        }
        if (sCrowdPeople[i].aware)
        {
            if (CrowdInFront(i, o, &gObjectEvents[gPlayerAvatar.objectEventId], 1, 1))
            {
                sCrowdPeople[i].script = sCrowdPeople[i].script;   // (it waits, and can be talked to)
                continue;
            }
            if ((b = CrowdBlocker(i, o, 1)) != NULL)
            {
                CrowdSidestep(i, o, b);
                sCrowdPeople[i].aware = FALSE;
            }
            else
                CrowdWalk(i, o, sCrowdPeople[i].dir), sCrowdPeople[i].steps--;
        }
        else if ((b = CrowdBlocker(i, o, 2)) != NULL)
            CrowdSidestep(i, o, b);
        else
            CrowdWalk(i, o, sCrowdPeople[i].dir), sCrowdPeople[i].steps--;
        sCrowdPeople[i].state = CROWD_WAIT;
    }
}

// talked to (U3D_VmTalk): a player-aware person stopped by the player runs its look's script
static bool8 CrowdTalk(u16 local)
{
    u32 i = local - CROWD_LOCAL;
    struct ObjectEvent *o;
    if (!U3D_Vm_CrowdLocal(local) || !sCrowdPeople[i].aware || (o = CrowdObject(i)) == NULL
     || !CrowdInFront(i, o, &gObjectEvents[gPlayerAvatar.objectEventId], 1, 1) || sCrowdPeople[i].script == 2000)
        return FALSE;
    MPNet_Log("u3d vm: a CASTELIA passer-by talked to (script %u)", sCrowdPeople[i].script);
    return Run(sCrowdPeople[i].script, 0x30 + i, TRUE);
}

// a person flown along a path (MMDL_ICA_Update: a point a field frame; at its end set down on the tile there)
#include "u3d_icas.h"
static struct { u8 local, path; bool8 playing; float frame, base[3]; } sIca;
static void IcaLand(void)
{
    u8 id = ObjEventId(sIca.local);
    const s16 *pt = sIcaPoints[sIcaPaths[sIca.path][0] + sIcaPaths[sIca.path][1] - 1];
    sIca.playing = FALSE;
    if (id >= OBJECT_EVENTS_COUNT)
        return;
    U3D_ObjectFly(gObjectEvents[id].spriteId, FALSE, 0, 0, 0);
    MoveObjectEventToMapCoords(&gObjectEvents[id], MapX((u16)floorf((sIca.base[0] + pt[0] / 8.0f) / 16.0f)) + MAP_OFFSET,
                               MapZ((u16)floorf((sIca.base[2] + pt[2] / 8.0f) / 16.0f)) + MAP_OFFSET);
    MPNet_Log("u3d vm: the flown person set down");
}

static void IcaFrame(void)
{
    u8 id;
    u32 f;
    const s16 *pt;
    if (!sIca.playing)
        return;
    if ((u32)sIca.frame >= sIcaPaths[sIca.path][1])
    {
        IcaLand();
        return;
    }
    if ((id = ObjEventId(sIca.local)) >= OBJECT_EVENTS_COUNT)
    {
        sIca.playing = FALSE;
        return;
    }
    f = (u32)sIca.frame;
    pt = sIcaPoints[sIcaPaths[sIca.path][0] + f];
    U3D_ObjectFly(gObjectEvents[id].spriteId, TRUE, sIca.base[0] + pt[0] / 8.0f, sIca.base[1] + pt[1] / 8.0f,
                  sIca.base[2] + pt[2] / 8.0f);
    if (pt[3] != 255)
        ObjectEventTurn(&gObjectEvents[id], BlackToDir(pt[3]));
    sIca.frame += 0.5f;
}

// people moved through the air by Black's commands, a field frame a step (U3D_ObjectFly; the person's place stays
// their tile): RISE to height H from where they are (kept up there), ORBIT round where they are, DROP from H above
// to the ground, SPINRISE up 150 spinning and then gone
enum { MOT_RISE, MOT_ORBIT, MOT_DROP, MOT_SPINRISE };
static struct { u8 local, kind; bool8 active, aloft; float frame, frames, h, start[3]; } sMotion[4];
static bool8 MotionStart(u8 local, u8 kind, float h, u16 frames)
{
    u8 id = ObjEventId(local);
    u32 k;
    if (id >= OBJECT_EVENTS_COUNT || frames == 0)
        return FALSE;
    for (k = 0; k < ARRAY_COUNT(sMotion) && sMotion[k].active && sMotion[k].local != local; k++)
        ;
    if (k == ARRAY_COUNT(sMotion))
        return FALSE;
    sMotion[k].aloft = U3D_ObjectFlyPos(gObjectEvents[id].spriteId, sMotion[k].start);
    if (!sMotion[k].aloft)
        U3D_TileWorldPos(gObjectEvents[id].currentCoords.x, gObjectEvents[id].currentCoords.y, sMotion[k].start);
    sMotion[k].local = local, sMotion[k].kind = kind, sMotion[k].active = TRUE, sMotion[k].frame = 0;
    sMotion[k].frames = frames, sMotion[k].h = h;
    if (kind == MOT_SPINRISE)
        PlaySE(SE_M_TELEPORT);
    MPNet_Log("u3d vm: person %u %s", local, kind == MOT_RISE ? "rising" : kind == MOT_ORBIT ? "circling"
                                           : kind == MOT_DROP ? "dropping in" : "spinning away");
    return TRUE;
}

static bool8 MotionWaiting(void)
{
    u32 k;
    for (k = 0; k < ARRAY_COUNT(sMotion); k++)
        if (sMotion[k].active && (sMotion[k].kind == MOT_RISE || sMotion[k].kind == MOT_ORBIT))
            return TRUE;
    return FALSE;
}

static void MotionFrame(void)
{
    static const u8 sSpin[4] = {DIR_SOUTH, DIR_EAST, DIR_NORTH, DIR_WEST};
    u32 k;
    for (k = 0; k < ARRAY_COUNT(sMotion); k++)
    {
        u8 id;
        float p[3], f, r = 0, a = 0;
        bool8 done;
        if (!sMotion[k].active)
            continue;
        if ((id = ObjEventId(sMotion[k].local)) >= OBJECT_EVENTS_COUNT)
        {
            sMotion[k].active = FALSE;
            continue;
        }
        sMotion[k].frame += 0.5f;
        f = sMotion[k].frame, done = f >= sMotion[k].frames;
        memcpy(p, sMotion[k].start, sizeof(p));
        switch (sMotion[k].kind)
        {
        case MOT_RISE:
            p[1] += (sMotion[k].h - sMotion[k].start[1]) * min(f / sMotion[k].frames, 1.0f);
            break;
        case MOT_ORBIT:   // (OrbitEventMain: two turns spiralling out, one round at three tiles, one spiralling in)
            if (f < 110)
                a = f / 110 * 2, r = f * 48 / 110;
            else if (f < 190)
                a = (f - 110) / 80, r = 48;
            else
                a = (f - 190) / 80, r = (270 - f) * 48 / 80;
            p[0] += cosf(a * 2 * (float)M_PI) * r, p[2] += sinf(a * 2 * (float)M_PI) * r;
            break;
        case MOT_DROP:   // (FIELD_TASK_CreatePlayerDrop: falling ever faster)
        {
            float t = 1.0f - min(f / sMotion[k].frames, 1.0f);
            p[1] += sMotion[k].h * t * t;
            break;
        }
        case MOT_SPINRISE:   // (a quarter turn every three frames, up 150 units)
            p[1] += 150 * min(f / sMotion[k].frames, 1.0f);
            ObjectEventTurn(&gObjectEvents[id], sSpin[(u32)(f / 3) & 3]);
            break;
        }
        if (!done)
        {
            U3D_ObjectFly(gObjectEvents[id].spriteId, TRUE, p[0], p[1], p[2]);
            continue;
        }
        sMotion[k].active = FALSE;
        if (sMotion[k].kind == MOT_DROP || (sMotion[k].kind == MOT_ORBIT && !sMotion[k].aloft))
            U3D_ObjectFly(gObjectEvents[id].spriteId, FALSE, 0, 0, 0);   // (on the ground again)
        else if (sMotion[k].kind == MOT_SPINRISE)
        {
            U3D_ObjectFly(gObjectEvents[id].spriteId, FALSE, 0, 0, 0);
            ForgetPerson(sMotion[k].local);
            Gone(sMotion[k].local, TRUE);
            RemoveObjectEventByLocalIdAndMap(gObjectEvents[id].localId, gObjectEvents[id].mapNum, gObjectEvents[id].mapGroup);
        }
        else
            U3D_ObjectFly(gObjectEvents[id].spriteId, TRUE, p[0], p[1], p[2]);   // (risen: kept up there)
    }
}

// (mp u3dmotion LOCAL KIND: a person of the map moved as Black's commands move them: 0 risen 4 tiles, 1 circling,
// 2 dropped in, 3 spinning away)
void U3D_Vm_DebugMotion(u8 local, u8 kind)
{
    MotionStart(local, kind, kind == MOT_RISE ? 64 : kind == MOT_DROP ? 250 : 0,
                kind == MOT_RISE ? 60 : kind == MOT_ORBIT ? 270 : kind == MOT_DROP ? 40 : 24);
}

// the LIGHT STONE's glow and its arc of light in N's throne room (ov064 field_gimmick_arc_effect.c, zone 278):
// model 0 the glow at the player (its last frame the event's end, a white flash at frame 101 held 15 frames), model 1
// the arc flown from a person to a tile over FRAMES, ARCH higher at its middle, then shown there playing
static struct { bool8 glow, flying, shown, debug; u8 se, fade, hold; float from[3], to[3], arch, frame, frames; } sArc;

static void ArcGlowStart(void)   // (StartPlayerEffect)
{
    float at[3];
    U3D_PlayerWorldPos(at);
    U3D_ExpPlace(0, at[0], at[1], at[2], 0);
    U3D_ExpHide(0, FALSE);
    U3D_ExpAnim(0, 0, 0, TRUE);
    U3D_ExpTex(0, 1, TRUE, 0, TRUE, FALSE);
    U3D_ExpTex(0, 2, TRUE, 0, TRUE, FALSE);
    U3D_ExpTex(0, 3, TRUE, 0, TRUE, FALSE);
    sArc.glow = TRUE, sArc.se = 0, sArc.fade = 0, sArc.hold = 0;
    MPNet_Log("u3d vm: the LIGHT STONE's glow");
}

static void ArcFlyStart(u8 local, s16 tx, s16 ty, s16 tz, s16 arch, u16 frames)   // (StartArcEffect)
{
    u8 id = ObjEventId(local);
    if (id < OBJECT_EVENTS_COUNT && !U3D_ObjectFlyPos(gObjectEvents[id].spriteId, sArc.from))
        U3D_TileWorldPos(gObjectEvents[id].currentCoords.x, gObjectEvents[id].currentCoords.y, sArc.from);
    else if (id >= OBJECT_EVENTS_COUNT)
        U3D_PlayerWorldPos(sArc.from);
    sArc.to[0] = tx * 16.0f + 8.0f, sArc.to[1] = ty * 8.0f, sArc.to[2] = tz * 16.0f + 8.0f;
    sArc.arch = arch * 8.0f, sArc.frame = 0, sArc.frames = frames ? frames : 1, sArc.flying = TRUE;
    U3D_ExpPlace(1, sArc.from[0], sArc.from[1], sArc.from[2], 0);
    U3D_ExpHide(1, FALSE);
    U3D_ExpAnim(1, 0, 0, FALSE);
    U3D_ExpTex(1, 1, FALSE, 0, FALSE, FALSE);
    U3D_ExpTex(1, 2, FALSE, 0, FALSE, FALSE);
    PlaySE(SE_M_SWIFT);
    MPNet_Log("u3d vm: an arc of light from person %u (%d,%d,%d) to %d,%d,%d", local, (s32)sArc.from[0],
              (s32)sArc.from[1], (s32)sArc.from[2], tx, ty, tz);
}

static void ArcShowAt(s16 tx, s16 ty, s16 tz)   // (ShowArcModelAt)
{
    U3D_ExpPlace(1, tx * 16.0f + 8.0f, ty * 8.0f, tz * 16.0f + 8.0f, 0);
    U3D_ExpHide(1, FALSE);
    U3D_ExpAnim(1, 0, 0, TRUE);
    U3D_ExpTex(1, 1, TRUE, 0, TRUE, FALSE);
    U3D_ExpTex(1, 2, TRUE, 0, TRUE, FALSE);
    sArc.shown = TRUE;
    PlaySE(SE_M_REFLECT);
    MPNet_Log("u3d vm: the arc of light shown at %d,%d,%d", tx, ty, tz);
}

// (mp u3darc: SCRCMD_StartArcEffect's subroutine in zone 278 without N's battle before it: from LOCAL to 16, 5, 6
// over 20 frames, 10 high, then shown there)
void U3D_Vm_DebugArc(u8 local)
{
    ArcFlyStart(LocalId(local), 0x10, 5, 6, 0xA, 0x14);
    sArc.debug = TRUE;
}

static void ArcFrame(void)   // (the events: Black's frames are two of ours)
{
    if (sArc.flying)
    {
        float f, k;
        u32 c;
        sArc.frame += 0.5f;
        if (sArc.frame >= sArc.frames)
        {
            sArc.frame = sArc.frames, sArc.flying = FALSE;
            if (sArc.debug)
                ArcShowAt(0x10, 5, 6), sArc.debug = FALSE;
        }
        k = sArc.frame / sArc.frames;
        f = sArc.frame >= (s32)(sArc.frames + 1) / 2 ? sArc.frames - sArc.frame : sArc.frame;
        {
            float p[3];
            for (c = 0; c < 3; c++)
                p[c] = sArc.from[c] + (sArc.to[c] - sArc.from[c]) * k;
            U3D_ExpPlace(1, p[0], p[1] + sArc.arch * f / sArc.frames, p[2], 0);
        }
    }
    if (sArc.shown && !U3D_ExpPlaying(1))
    {
        sArc.shown = FALSE;
        MPNet_Log("u3d vm: the arc of light over");
    }
    if (sArc.glow)
    {
        static const u16 sSes[] = {SE_M_MORNING_SUN, SE_M_DETECT, SE_M_HEAL_BELL, SE_SHINY};
        static const u16 sAt[] = {0, 108, 131, 250};
        float fr = U3D_ExpFrame(0);
        if (sArc.se < ARRAY_COUNT(sAt) && fr >= sAt[sArc.se])
            PlaySE(sSes[sArc.se++]);
        if (sArc.fade == 0 && fr >= 101)
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_WHITE);
            sArc.fade = 1, sArc.hold = 30;
        }
        else if (sArc.fade == 1 && !gPaletteFade.active && (sArc.hold == 0 || --sArc.hold == 0))
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_WHITE);
            sArc.fade = 2;
        }
        if (!U3D_ExpPlaying(0) && sArc.fade != 1 && !gPaletteFade.active)
        {
            U3D_ExpHide(0, TRUE);
            sArc.glow = FALSE;
            MPNet_Log("u3d vm: the LIGHT STONE's glow over");
        }
    }
}

static void TimeFxStart(void)   // (StartEffect: shown, both tracks played, its sound)
{
    if (!IS_TIME_EFFECT_ZONE(CurrentZone()))
        return;
    sTimeFx.playing = TRUE, sTimeFx.peaked = FALSE;
    U3D_ExpHide(0, FALSE);
    U3D_ExpAnim(0, 0, 0, TRUE);
    U3D_ExpTex(0, 1, TRUE, 0, TRUE, FALSE);
    PlaySE(sTimeFx.night ? SE_M_MOONLIGHT : SE_M_MORNING_SUN);
    MPNet_Log("u3d vm: zone 389's time effect, by %s", sTimeFx.night ? "night" : "day");
}

static void TimeFxFrame(void)   // (Update: the day's peak sound at frame 60; at the end hidden, no longer pending)
{
    if (!sTimeFx.playing)
        return;
    if (!sTimeFx.night && !sTimeFx.peaked && U3D_ExpFrame(0) >= 60)
    {
        sTimeFx.peaked = TRUE;
        PlaySE(SE_M_HEAL_BELL);
    }
    if (!U3D_ExpPlaying(0))
    {
        U3D_ExpHide(0, TRUE);
        sTimeFx.playing = FALSE;
        sWork.white &= ~TIMEFX_PENDING;
        MPNet_Log("u3d vm: zone 389's time effect over");
    }
}   // (W_MUSICAL: the MUSICAL dressing room closed)   // (W_EXP_ANIM: a gym model's animation to its end, sExpWait)
static u16 sExpWait;
enum { W_NONE, W_FRAMES, W_MESSAGE, W_MESSAGE_KEY, W_KEY, W_MOVES, W_FADE, W_YESNO, W_CLOSE, W_SCREEN, W_SONG_THEN_POP, W_SE, W_CAMERA, W_CRY, W_MENU,
       W_WARP, W_PIC_KEY };   // (W_WARP: a warp's map is loading; the script goes on there: U3D_Vm_SceneStart)

// ---- Black's songs: its sound archive's sequence numbers as the game's song slots (tools/kaisong/unova_songs.py);
// a pushed song comes back when the one played over it ends (a fanfare: Black's PushAndPlayBgm, StartWaitBgmThenPop)
#include "../data/unova_songs.h"
#include "u3d_cameras.h"
static u16 sSongStack[4];
static u8 sSongDepth;

static u16 SongOf(u16 seq)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sUnovaSongs); i++)
        if (sUnovaSongs[i].seq == seq)
            return sUnovaSongs[i].song;
    return MUS_NONE;
}

static void PushSong(void)
{
    if (sSongDepth < ARRAY_COUNT(sSongStack))
        sSongStack[sSongDepth++] = GetCurrentMapMusic();
}

static void PopSong(void)
{
    if (sSongDepth)
        PlayNewMapMusic(sSongStack[--sSongDepth]);
}
// The screens Black's scripts open, from Unova_EventScript_Screen (data/scripts/unova.inc)
enum { SCREEN_NONE, SCREEN_STARTER, SCREEN_BATTLE, SCREEN_CATCH_DEMO, SCREEN_WILD, SCREEN_PARTY, SCREEN_CLEAR, SCREEN_TRADE,
       SCREEN_SUBWAY_PARTY, SCREEN_SUBWAY_BATTLE, SCREEN_SUBWAY_RESET, SCREEN_MART, SCREEN_BPSHOP, SCREEN_SELL,
       SCREEN_DAYCARE_PARTY, SCREEN_MOVE_SELECT, SCREEN_RELEARN, SCREEN_DIPLOMA, SCREEN_EASY_CHAT, SCREEN_WORD };
static u8 sScreen;
static u16 sScreenVar, sPartySlotWork;
static u16 sEasyChatKind, sEasyChatOut[2], sEasyChatWords[EASY_CHAT_BATTLE_WORDS_COUNT];
static u8 sWildResult = 2;
#define STACK 48

struct Frame { const u8 *code; u32 size, pc; u16 bank; u8 sp; };   // (a script that called another: CallScript)
#define FRAMES 6

struct Thread
{
    struct Frame frames[FRAMES];
    u8 depth;
    const u8 *code;
    u32 size, pc;
    u32 stack[STACK];
    u8 sp, cmp, wait;
    bool8 alive, box;
    u16 zone, bank, waitFrames, target;
    u16 *result;
    u16 script;   // (the script it was started as: a trainer's is 3000 + its number, GetScriptTrainerId)
};

static struct Thread sThread;
static u8 sPlaceholder[8];
static bool8 sHolding;   // Unova_EventScript_* waits (waitstate) until the script ends
static u16 sLogged[U3D_CMD_COUNT / 16 + 1];

static void Push(struct Thread *t, u32 v)
{
    if (t->sp < STACK)
        t->stack[t->sp++] = v;
}

static u32 Pop(struct Thread *t)
{
    return t->sp ? t->stack[--t->sp] : 0;
}

static u8 Code8(struct Thread *t) { return t->pc < t->size ? t->code[t->pc++] : 0; }
static u16 Code16(struct Thread *t) { u16 v = t->pc + 2 <= t->size ? Le16(t->code + t->pc) : 0; t->pc += 2; return v; }
static s32 Code32(struct Thread *t) { s32 v = t->pc + 4 <= t->size ? (s32)(Le16(t->code + t->pc) | (u32)Le16(t->code + t->pc + 2) << 16) : 0; t->pc += 4; return v; }

// A script id as the zone runs it (scripts.py script_file): its file, its start, its message bank
static bool8 Locate(u16 zone, u16 scriptId, const u8 **code, u32 *size, u32 *pc, u16 *bank)
{
    u32 i, n = Ready() ? Get32(sRangesAt) : 0, index = 0xFFFFFFFF, file = 0xFFFF, msg = 0xFFFF;
    for (i = 0; i < n; i++)
    {
        u32 at = sRangesAt + 4 + 10 * i;
        if (scriptId >= Get16(at) && scriptId <= Get16(at + 2))
        {
            index = scriptId - Get16(at);
            file = Get16(at + 4);
            msg = Get16(at + 8);
            break;
        }
    }
    if (index == 0xFFFFFFFF)
    {
        if (scriptId == 0)
            return FALSE;
        index = scriptId - 1;
        file = ZoneField(zone, Z_SCRIPT);
        msg = ZoneField(zone, Z_MSG);
    }
    *code = FileOf(sFilesAt, file, size);
    if (*code == NULL || 4 * index + 4 > *size || Le16(*code + 4 * index) == 0xFD13)
        return FALSE;
    *pc = 4 * index + 4 + (s32)(Le16(*code + 4 * index) | (u32)Le16(*code + 4 * index + 2) << 16);
    *bank = SYSTEM_BANKS + msg;
    return *pc < *size;
}

static void Task_Vm(u8 taskId);

static u8 sClearMode;
static u16 sTradeId, sTradeSlot, sIss;
// ---- Black's POKeMARTs (ov021 scrcmd_proc_fld.c, the pack's lists after the BATTLE SUBWAY's tables): shop 255 the
// list for the badges UNOVA has given, any other shop N list N + 6; the game's own POKeMART sells them (Black's items
// as the game's: Item(), those it doesn't have left out)
static u16 Item(u16 black);
static u16 sMartItems[48];
static u8 sBpShop;
void U3D_BpShop_Open(u8 which);
void MP_CreatePokemartDirect(const u16 *itemsForSale, bool8 sell);   // (shop.c)
static u32 ShopsAt(void)
{
    u32 tables;
    if (!Ready() || sSubwayAt == 0)
        return 0;
    tables = sSubwayTablesAt + 488;   // (u3d_subway.c struct Tables: the passengers' count, then theirs)
    return tables + 4 + 16 * Get32(tables);
}

// Black's NPC table after the BP exchanges (version 11): u16 key A, key B, object code, trainer each
static const u8 *NpcTable(u32 *count);

// the BP exchanges' lists after the marts' (WHICH 0: shop 254, items; 1: shop 253, TMs): u16 item, pad, s32 BP each
const u8 *U3D_Vm_BpList(u8 which, u32 *count)
{
    u32 at = ShopsAt(), lists, total = 0, i;
    *count = 0;
    if (at == 0 || (lists = Get32(at)) == 0 || lists > 64)
        return NULL;
    for (i = 0; i < lists; i++)
        total += sPack[at + 4 + i];
    at = (at + 4 + lists + (lists % 2) + 2 * total + 9 + 3) & ~3;
    if (which)
        at += 4 + 8 * Get32(at);
    *count = Get32(at);
    return sPack + at + 4;
}

static const u8 *NpcTable(u32 *count)
{
    u32 n;
    const u8 *tms = Ready() && Get32(4) >= 11 ? U3D_Vm_BpList(1, &n) : NULL;
    *count = 0;
    if (tms == NULL)
        return NULL;
    tms += 8 * n;
    *count = tms[0] | tms[1] << 8 | tms[2] << 16 | (u32)tms[3] << 24;
    return tms + 4;
}

// SCRCMD_NETWORK_FindNpcTableRow: the row of keys A and B (0 if none)
// The BATTLE SUBWAY's map after the NPC table (version 12): u16 width, height, then the pixels (u3d_subwaymap.c)
const u16 *U3D_Vm_SubwayMap(u16 *w, u16 *h)
{
    u32 n;
    const u8 *t = Get32(4) >= 12 ? NpcTable(&n) : NULL;
    if (t == NULL)
        return NULL;
    t += 8 * n;
    *w = Le16(t), *h = Le16(t + 2);
    return (const u16 *)(t + 4);
}

bool8 Unova_Screen_IsActive(void);
static bool8 Unova_Screen_IsActive_(void) { return Unova_Screen_IsActive(); }

static u32 NpcTableRow(u16 keyA, u16 keyB)
{
    u32 n, i;
    const u8 *t = NpcTable(&n);
    for (i = 0; t && i < n; i++)
        if (Le16(t + 8 * i) == keyA && Le16(t + 8 * i + 2) == keyB)
            return i;
    return 0;
}

static u8 ObjEventId(u8 localId);
static void NpcFromRow(u8 slot, u32 row, bool8 secondary, bool8 look)
{
    u32 n;
    const u8 *t = NpcTable(&n);
    if (t == NULL || row >= n || slot >= NPC_OVERRIDES)
        return;
    OverridesHere();
    sNpcEvOver[slot] = Le16(t + 8 * row + 6) + (secondary ? 5000 : 3000) + 1;   // (EVENT_TRAINER_GetPrimary/SecondaryScriptId)
    if (look)
    {
        u8 id = ObjEventId(slot + 1);
        sNpcCodeOver[slot] = Le16(t + 8 * row + 4) + 1;
        if (id < OBJECT_EVENTS_COUNT)   // (already on the field: shown at once)
            ObjectEventSetGraphicsId(&gObjectEvents[id], OBJ_EVENT_GFX_UNOVA_FIRST + Le16(t + 8 * row + 4));
    }
    MPNet_Log("u3d vm: npc table: person %u is trainer %u (row %u)", slot, Le16(t + 8 * row + 6), row);
}

// BLACK CITY's market (shops 247-251, FIELD_WFBC_DATA_LOCAL_FillPeopleListEntries): stall K sells what residents 2K
// and 2K + 1 brought (their records' list entries), at their prices (shop.c MP_Shop_SetPrices)
static const u8 *WfbcRecord(u8 npc);
static u32 sMartPrices[48];
void MP_Shop_SetPrices(const u16 *items, const u32 *prices);
static bool8 BlackCityMart(u16 shop)
{
    u32 k = shop - 247, i, n = 0;
    for (i = 2 * k; i < 2 * k + 2 && i < 10; i++)
    {
        const u8 *r = sWork.wfbc.valid && sWork.wfbc.npc[i] != 0xFF ? WfbcRecord(sWork.wfbc.npc[i]) : NULL;
        u16 item = r != NULL ? Item(Le16(r + 0x28)) : ITEM_NONE;
        if (item == ITEM_NONE)
            continue;
        sMartItems[n] = item;
        sMartPrices[n++] = Le16(r + 0x2A);
    }
    sMartItems[n] = ITEM_NONE;
    MP_Shop_SetPrices(sMartItems, sMartPrices);
    MPNet_Log("u3d vm: BLACK CITY's stall %u: %u things", k, n);
    return n != 0;
}

static bool8 MartList(u16 shop)
{
    u32 at = ShopsAt(), lists, items, idx, i, from = 0, n = 0, badges = 0;
    if (at == 0 || (lists = Get32(at)) == 0 || lists > 64)
        return FALSE;
    items = at + 4 + lists + (lists % 2);
    for (i = 0; i < 8; i++)
        badges += (sWork.badges >> i) & 1;
    for (i = 0; i < lists; i++)
        from += sPack[at + 4 + i];
    if (shop == 255)
        idx = sPack[items + 2 * from + badges];   // (MISC_CountBadges: the list for that many)
    else
        idx = shop + 6;
    if (idx >= lists)
        return FALSE;
    for (i = 0, from = 0; i < idx; i++)
        from += sPack[at + 4 + i];
    for (i = 0; i < sPack[at + 4 + idx] && n < ARRAY_COUNT(sMartItems) - 1; i++)
    {
        u16 item = Item(Get16(items + 2 * (from + i)));
        if (item != ITEM_NONE)
            sMartItems[n++] = item;
    }
    sMartItems[n] = ITEM_NONE;
    MPNet_Log("u3d vm: mart %u (list %u): %u things", shop, idx, n);
    return n != 0;
}

static const u8 *TradeRecord(u16 id);
static u32 Get32At(const u8 *p, u32 o);
static u8 GenderOf(struct Pokemon *mon);
static void UnovaTradeSetUp(void);
void CreateInGameTradePokemon(void);   // (trade.c)
void DoInGameTradeScene(void);
static const u8 *sWarpTable, *sWarpTableEnd;   // (SetWarpTable: an elevator's floors in the running script)
static struct { u8 group, num, set; s16 x, y; } sSpecialLoc;   // (SetSpecialLocation: Black's continue spot)
static void UnovaGameClear(void);
void SetContinueGameWarp(s8 mapGroup, s8 mapNum, s8 warpId, s16 x, s16 y);   // (overworld.c)
int GameClear(void);   // (post_battle_event_funcs.c)

// Black's script wants a screen of the game's: the field script takes it from here (U3D_VmOpenScreen) and the VM
// goes on when it's back (U3D_VmResume)
// Black's word lists (arm9 pms_word.c: g_pmsWordBankSizes / FileIds; a word's number counts through them in turn:
// the POKeMON, the moves, the types, the Abilities, then Black's own words' groups), and a word typed on the naming
// screen found in them: letters and digits alike whatever their case or accent, PK MN as POKeMON
static const u16 sPmsSizes[13] = {652, 560, 17, 165, 44, 38, 48, 103, 47, 32, 26, 10, 61};
static const u16 sPmsFiles[13] = {284, 286, 287, 285, 149, 150, 151, 152, 153, 154, 155, 148, 156};
static u8 sWordBuf[WALDA_PHRASE_LENGTH + 1];

static u32 WordKey(const u8 *s, u8 *out, u32 max)
{
    u32 n = 0;
    for (; *s != EOS && n + 4 < max; s++)
    {
        u8 c = *s;
        if (c >= CHAR_a && c <= CHAR_z)
            c = c - CHAR_a + CHAR_A;
        if (c == 0x06 || c == 0x1B)   // (É, é)
            c = CHAR_E;
        if (c == CHAR_PK || c == CHAR_MN)
        {
            const u8 *w = c == CHAR_PK ? (const u8 *)"\xCA\xC9\xC5" : (const u8 *)"\xC7\xC9\xC8";   // (POK, MON)
            out[n++] = w[0], out[n++] = w[1], out[n++] = w[2];
            if (c == CHAR_PK)
                out[n++] = CHAR_E;
            continue;
        }
        if ((c >= CHAR_A && c <= CHAR_Z) || (c >= CHAR_0 && c <= CHAR_9))
            out[n++] = c;
        else if (c >= 0xF7)   // (a control code: none in a word)
            break;
    }
    out[n] = EOS;
    return n;
}

static u16 WordId(const u8 *typed)
{
    u8 want[40], have[40];
    u32 b, w, id = 0;
    if (WordKey(typed, want, sizeof(want)) == 0)
        return 0xFFFF;
    for (b = 0; b < ARRAY_COUNT(sPmsSizes); id += sPmsSizes[b], b++)
        for (w = 0; w < sPmsSizes[b]; w++)
            if (WordKey(U3D_Text(sPmsFiles[b], w), have, sizeof(have)) && StringCompare(want, have) == 0)
                return id + w;
    return 0xFFFF;
}

// the naming screen's title: the quiz host's ask, the last line of the zone's message 9 ("Challenger, please answer!")
static void WordTitle(u16 bank)
{
    u8 buf[40], cur[40];
    const u8 *m = U3D_Text(bank, 9);
    u32 n = 0;
    buf[0] = EOS;
    for (;; m++)
    {
        if (*m == EOS || *m == CHAR_NEWLINE || *m == CHAR_PROMPT_SCROLL || *m == CHAR_PROMPT_CLEAR || *m >= 0xF7)
        {
            if (n)   // (a line with words: the last one kept)
            {
                cur[n] = EOS;
                memcpy(buf, cur, n + 1);
            }
            n = 0;
            if (*m == EOS)
                break;
            if (*m == EXT_CTRL_CODE_BEGIN && m[1] != EOS)
                m++;
            continue;
        }
        if (n < sizeof(cur) - 1)
            cur[n++] = *m;
    }
    NamingScreen_SetUnovaTitle(buf);
}

static void OpenScreen(struct Thread *t, u8 screen, u16 var)
{
    sScreen = screen;
    sScreenVar = var;
    t->wait = W_SCREEN;
    if (sHolding)
        ScriptContext_Enable();
}

static void Finish(struct Thread *t)
{
    t->alive = FALSE;
    gTextFlags.autoScroll = FALSE;   // (Black's auto mode ends with its script)
    if (!IsFieldMessageBoxHidden())
        HideFieldMessageBox();
    if (sHolding)
    {
        sHolding = FALSE;
        ScriptContext_Enable();
    }
}

static void MessageInBank(struct Thread *t, u16 bank, u16 msg);

static void Message(struct Thread *t, u16 file, u16 msg)
{
    MessageInBank(t, file == MSG_OWN ? t->bank : SYSTEM_BANKS + file, msg);
}

static void MessageInBank(struct Thread *t, u16 bank, u16 msg)
{
    sPlaceholder[0] = PLACEHOLDER_BEGIN;
    sPlaceholder[1] = PLACEHOLDER_ID_UNOVA_TEXT;
    sPlaceholder[2] = (bank >> 7) & 0x7F;
    sPlaceholder[3] = bank & 0x7F;
    sPlaceholder[4] = (msg >> 7) & 0x7F;
    sPlaceholder[5] = msg & 0x7F;
    sPlaceholder[6] = EOS;
    if (!IsFieldMessageBoxHidden())
        HideFieldMessageBox();
    ShowFieldMessage(sPlaceholder);
    t->wait = W_MESSAGE;
}

static u8 *Slot(u8 slot)   // (Black's word slots, UNOVA's own: the game's battles rewrite gStringVar1-3)
{
    return gUnovaStrVar[slot % UNOVA_STR_VARS];
}

// Black's item numbers as the game's (tools/unova/gen_items.py: by name; ITEM_NONE where the game has no such item yet)
#include "../data/unova_items.h"
#include "u3d_railpos.h"
#include "u3d_tables.h"   // (Black's small tables: tools/unova/gen_tables.py)
#include "u3d_gyms.h"   // (BRYCEN's and DRAYDEN's gyms: tools/unova/gen_gyms.py)
#include "tv.h"
static u16 Item(u16 black)
{
    return black < ARRAY_COUNT(sUnovaItems) ? sUnovaItems[black] : ITEM_NONE;
}

u16 U3D_Vm_Item(u16 black)
{
    return Item(black);
}

static u16 Species(u16 national)
{
    return national ? NationalPokedexNumToSpecies(national) : SPECIES_NONE;
}

// Black's move tutors (ov021 GetTutorMoveId, CheckTutorGroup, FindTutorCandidates): tutor 0 DRACO METEOR (tutor bit 6),
// 1 FRENZY PLANT / BLAST BURN / HYDRO CANNON (bits 3-5), 2 GRASS / FIRE / WATER PLEDGE (bits 0-2); full friendship
static const u16 sTutorMoves[3][3] = {{MOVE_DRACO_METEOR}, {MOVE_FRENZY_PLANT, MOVE_BLAST_BURN, MOVE_HYDRO_CANNON},
                                      {MOVE_GRASS_PLEDGE, MOVE_FIRE_PLEDGE, MOVE_WATER_PLEDGE}};
static const u8 sTutorFirstBit[3] = {6, 3, 0}, sTutorBitCount[3] = {1, 3, 3};

static u32 TutorBits(struct Pokemon *mon)
{
    u16 national = SpeciesToNationalPokedexNum(GetMonData(mon, MON_DATA_SPECIES));
    s32 lo = 0, hi = ARRAY_COUNT(sTutorCompat) - 1;
    while (lo <= hi)
    {
        s32 mid = (lo + hi) / 2;
        if (sTutorCompat[mid][0] == national)
            return sTutorCompat[mid][1];
        if (sTutorCompat[mid][0] < national)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return 0;
}

static bool8 TutorGroup(struct Pokemon *mon, u32 tutor)
{
    return tutor < 3 && (TutorBits(mon) >> sTutorFirstBit[tutor]) & ((1 << sTutorBitCount[tutor]) - 1);
}

static u16 TutorMove(struct Pokemon *mon, u32 tutor)
{
    u32 i, bits = TutorBits(mon);
    if (tutor >= 3)
        return MOVE_BLAST_BURN;
    for (i = 0; i < sTutorBitCount[tutor]; i++)
        if (bits & (1 << (sTutorFirstBit[tutor] + i)))
            return sTutorMoves[tutor][i];
    return tutor == 0 ? MOVE_DRACO_METEOR : MOVE_BLAST_BURN;   // (Black's own answer for one not of its kinds)
}

// 0 some can learn (mask: which), 1 none of the tutor's kinds, 2 none friendly enough (when checked)
static u32 TutorCandidates(u32 tutor, bool8 friendship, u8 *mask)
{
    u32 i;
    bool8 kind = FALSE;
    *mask = 0;
    for (i = 0; i < gPlayerPartyCount; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_IS_EGG) || !TutorGroup(mon, tutor))
            continue;
        kind = TRUE;
        if (friendship && GetMonData(mon, MON_DATA_FRIENDSHIP) < MAX_FRIENDSHIP)
            continue;
        *mask |= 1 << i;
    }
    return !kind ? 1 : *mask == 0 ? 2 : 0;
}

// Black's SetMonMove: a move into a slot; MOVE_NONE forgets it, the moves after it moving up; a slot past 4 learns it
// as a fifth would (the first forgotten)
static void SetPartyMonMove(struct Pokemon *mon, u16 slot, u16 move)
{
    u32 i;
    if (slot > MAX_MON_MOVES)
    {
        if (GiveMoveToMon(mon, move) == MON_HAS_MAX_MOVES)
            DeleteFirstMoveAndGiveMoveToMon(mon, move);
        return;
    }
    if (slot >= MAX_MON_MOVES)
        return;
    if (move != MOVE_NONE)
    {
        RemoveMonPPBonus(mon, slot);
        SetMonMoveSlot(mon, move, slot);
        return;
    }
    for (i = slot; i < MAX_MON_MOVES - 1; i++)
    {
        u16 m = GetMonData(mon, MON_DATA_MOVE1 + i + 1);
        u8 pp = GetMonData(mon, MON_DATA_PP1 + i + 1), bonus = GetMonData(mon, MON_DATA_PP_BONUSES);
        u8 up = (bonus >> ((i + 1) * 2)) & 3;
        if (m == MOVE_NONE)
            break;
        bonus = (bonus & ~(3 << (i * 2))) | (up << (i * 2));
        SetMonData(mon, MON_DATA_MOVE1 + i, &m);
        SetMonData(mon, MON_DATA_PP1 + i, &pp);
        SetMonData(mon, MON_DATA_PP_BONUSES, &bonus);
    }
    {
        u16 none = MOVE_NONE;
        u8 zero = 0, bonus = GetMonData(mon, MON_DATA_PP_BONUSES) & ~(3 << (i * 2));
        SetMonData(mon, MON_DATA_MOVE1 + i, &none);
        SetMonData(mon, MON_DATA_PP1 + i, &zero);
        SetMonData(mon, MON_DATA_PP_BONUSES, &bonus);
    }
}

static struct { u8 party; u16 newMove, posWork; } sMoveSel;
static u16 sButtonModels, sButtonModelsSet;   // (STRIATON's gym: its nine state models at their last frame, and
                                              // those its scripts have set, each visit)
#define ZONE_STRIATON_GYM 7
#define ZONE_NACRENE_GYM 18   // (SelectMonMove: the summary's move list)

// v (work or constant), p (a work's id), h, w, b: as u16_cmds.h says, read into a[]
static u32 ReadArgs(struct Thread *t, u16 op, u32 *a)
{
    const char *s = op < U3D_CMD_COUNT ? sU3dCmdArgs[op] : "";
    u32 n = 0;
    for (; *s && n < 16; s++, n++)
    {
        switch (*s)
        {
        case 'b': a[n] = Code8(t); break;
        case 'w': a[n] = (u32)Code32(t); break;
        default:  a[n] = Code16(t); break;
        }
    }
    return n;
}

#define V(i) WorkValue((u16)a[i])

// ------------------------------------------------------------------------------------------------ the ROYAL UNOVA
// Black's pleasure boat (ov021 pleasure_boat.c, ov036 daily_layout.c): the day's 15 passengers dealt to the cabins
// (trainers by weekday: MON 4, TUE 3, WED 4, THU 5, FRI 4, SAT 6, SUN 7, two bonus people, the rest filler), the
// trip's clock (30 ticks a second, four periods of 1350 ticks: a chime at each, the zone's script 1 at the end)
struct BoatPassenger { u16 id, type, value, param, extra[2], occupied; };
static struct
{
    bool8 alive, locked, eventPending;
    u8 periodCount;
    u16 mainCount;
    u32 ticks;
    struct BoatPassenger p[15];
} sBoat;

static void BoatPick(u8 pool, u8 count, u8 *picks)   // (DAILY_LAYOUT_PickDistinct)
{
    u8 scratch[15], i;
    for (i = 0; i < pool; i++)
        scratch[i] = i;
    for (i = 0; i < count; i++)
    {
        u8 r = Random() % pool;
        picks[i] = scratch[r];
        scratch[r] = scratch[--pool];
    }
}

static bool8 BoatDeal(u8 *dealt)   // (DAILY_LAYOUT_DealCells: an anchor cell, a companion for the other half, the rest)
{
    u8 pool[15], last = 14, n = 0, i, remaining = 14, cell;
    for (i = 0; i < 15; i++)
        pool[i] = i;
#define BOAT_TAKE(c) do { u8 _k; for (_k = 0; _k <= last && pool[_k] != (c); _k++) ; if (_k > last) return FALSE; \
                          pool[_k] = pool[last]; if (last) last--; dealt[n++] = (c); } while (0)
    i = Random() % 6;
    cell = sBoat_anchor[i];
    BOAT_TAKE(cell);
    // (Black reads the flags of the anchor's index, not its cell: the companion always comes from the A list or the B)
    cell = sBoat_flags[i][0] == 0 ? sBoat_compB[Random() % 8] : sBoat_compA[Random() % 7];
    BOAT_TAKE(cell);
    remaining--;
#undef BOAT_TAKE
    for (i = 0; i < remaining; i++)
    {
        u8 r = Random() % (remaining - i);
        if (r > last)
            return FALSE;
        dealt[n++] = pool[r];
        pool[r] = pool[last];
        if (last)
            last--;
    }
    return TRUE;
}

static void BoatCreate(void)
{
    static const u8 sMainByDay[7] = {7, 4, 3, 4, 5, 4, 6};   // (SUNDAY first)
    u16 MP_JohtoWeekday(void);
    u8 dealt[15], mainPick[7], bonusPick[2], fillerPick[15], i, k = 0, fill;
    memset(&sBoat, 0, sizeof(sBoat));
    sBoat.alive = TRUE;
    sBoat.mainCount = sMainByDay[MP_JohtoWeekday() % 7];
    if (!BoatDeal(dealt))
        for (i = 0; i < 15; i++)
            dealt[i] = 0;
    BoatPick(10, sBoat.mainCount, mainPick);
    BoatPick(5, 2, bonusPick);
    fill = 15 - (sBoat.mainCount + 2);
    BoatPick(15, fill, fillerPick);
    for (i = 0; i < sBoat.mainCount; i++, k++)
    {
        struct BoatPassenger *b = &sBoat.p[dealt[k]];
        const u16 *d = sBoat_main[mainPick[i]];
        b->id = mainPick[i], b->type = d[1], b->value = d[0], b->param = 0, b->extra[0] = d[2], b->extra[1] = d[3];
    }
    for (i = 0; i < 2; i++, k++)
    {
        struct BoatPassenger *b = &sBoat.p[dealt[k]];
        const u16 *d = sBoat_bonus[bonusPick[i]];
        b->id = i, b->type = 0, b->value = d[0], b->param = 2, b->extra[0] = d[1], b->extra[1] = d[2];
    }
    for (i = 0; i < fill; i++, k++)
    {
        struct BoatPassenger *b = &sBoat.p[dealt[k]];
        const u16 *d = sBoat_filler[fillerPick[i]];
        b->id = i, b->type = 0, b->value = d[0], b->param = 1, b->extra[0] = d[1], b->extra[1] = d[2];
    }
    MPNet_Log("u3d vm: the ROYAL UNOVA sails with %u trainers", sBoat.mainCount);
}

// (mp_debug.c `mp u3dboat CABIN MAIN`: a cabin's passenger made the main trainer MAIN of Black's ten, for tests)
void U3D_Vm_DebugBoat(u16 cabin, u16 main)
{
    struct BoatPassenger *b = &sBoat.p[cabin % 15];
    const u16 *d = sBoat_main[main % 10];
    b->id = main % 10, b->type = d[1], b->value = d[0], b->param = 0, b->extra[0] = d[2], b->extra[1] = d[3];
    b->occupied = 0;
    MPNet_Log("u3d vm: the ROYAL UNOVA's cabin %u: trainer %u", cabin % 15, d[1]);
}

static u16 BoatStat(u16 kind)   // (PLEASURE_BOAT_GetStat: 0-3 the groups, 4 the trainers aboard, 5 the cabins visited)
{
    u16 n = 0, i;
    if (kind == 4)
        return sBoat.mainCount;
    if (kind == 5)
        for (i = 0; i < 15; i++)
            n += sBoat.p[i].occupied != 0;
    return n;
}

static void BoatAddTime(u16 seconds, u16 clamp)   // (PLEASURE_BOAT_AdvanceTime: a period's end stops one tick short)
{
    u32 ticks = sBoat.ticks % 1350 + seconds * 30;
    if (ticks < 1350)
        sBoat.ticks += seconds * 30;
    else if (clamp)
        sBoat.ticks = (sBoat.periodCount + 1) * 1350 - 1;
    else
        sBoat.ticks = (sBoat.periodCount + 1) * 1350, sBoat.periodCount++;
}



// A POKéMON knows a move, Black's numbers being the game's (CUT 15...): Kai's held HMs count (scrcmd.c checkpartymove)
static bool8 PartyMonHasMove(struct Pokemon *mon, u16 move)
{
    return GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG)
        && (MonKnowsMove(mon, move) || MP_HeldHmMove(mon) == move);
}

// Black's script menus (MSGWIN_CmdInitMenu..., AddMenuItem, StartMenu): items from the script's own words (InitMenu:
// story bank 262, Black's shared menu words), a value each; the choice's value into the result work, B 0xFFFE where
// the menu may be cancelled. Drawn as the game's multichoice windows, its right and bottom edges the YES / NO window's
#define MENU_MAX 16
static struct
{
    u16 bank, values[MENU_MAX], *result;
    u8 count, cursor, cancel, window;
    u8 text[MENU_MAX][48];
    struct MenuAction actions[MENU_MAX];
} sMenu;

static void MenuInit(u16 bank, u16 cursor, u8 cancel, u16 *result)
{
    sMenu.bank = bank, sMenu.count = 0, sMenu.cursor = cursor, sMenu.cancel = cancel, sMenu.result = result;
}

static void MenuAdd(u16 msg, u16 value)
{
    if (sMenu.count >= MENU_MAX)
        return;
    StringExpandPlaceholders(sMenu.text[sMenu.count], U3D_Text(sMenu.bank, msg));
    sMenu.actions[sMenu.count].text = sMenu.text[sMenu.count];
    sMenu.actions[sMenu.count].func.void_u8 = NULL;
    sMenu.values[sMenu.count++] = value;
}

static void MenuStart(struct Thread *t)
{
    s32 i, width = 0;
    u8 w, top;
    if (sMenu.count == 0)
        return;
    for (i = 0; i < sMenu.count; i++)
        width = max(width, GetStringWidth(FONT_NORMAL, sMenu.text[i], 0));
    w = (width + 9 + 7) / 8 + 1;   // (the cursor's room)
    top = sMenu.count * 2 + 1 < 13 ? 13 - sMenu.count * 2 : 1;
    sMenu.window = CreateWindowFromRect(w < 25 ? 26 - w : 1, top, w, sMenu.count * 2);
    SetStandardWindowBorderStyle(sMenu.window, FALSE);
    PrintMenuTable(sMenu.window, sMenu.count, sMenu.actions);
    InitMenuInUpperLeftCornerNormal(sMenu.window, sMenu.count, sMenu.cursor < sMenu.count ? sMenu.cursor : 0);
    ScheduleBgCopyTilemapToVram(0);
    t->wait = W_MENU;
}

static bool8 MenuInput(void)
{
    s8 r = Menu_ProcessInput();
    if (r == MENU_NOTHING_CHOSEN || (r == MENU_B_PRESSED && !sMenu.cancel))
        return FALSE;
    if (sMenu.result)
        *sMenu.result = r == MENU_B_PRESSED ? 0xFFFE : sMenu.values[r];
    ClearStdWindowAndFrameToTransparent(sMenu.window, TRUE);
    RemoveWindow(sMenu.window);
    return TRUE;
}

static void SetWhiteoutPoint(u16 warpId);

static void Step(struct Thread *t)
{
    u32 guard;
    for (guard = 0; guard < 4096 && t->alive && t->wait == W_NONE; guard++)
    {
        u32 a[16], here = t->pc;
        u16 op;
        if (t->pc + 2 > t->size)
        {
            Finish(t);
            return;
        }
        op = Code16(t);
        if (op >= U3D_CMD_COUNT)
        {
            MPNet_Log("u3d vm: op %04X at %X, script ends", op, here);
            Finish(t);
            return;
        }
        // (IfJump / IfCall read their own: b then w)
        ReadArgs(t, op, a);
        {
            static s8 sTrace = -1;   // (MOEMON_U3D_TRACE=1: every command played, for tests)
            if (sTrace < 0)
                sTrace = getenv("MOEMON_U3D_TRACE") != NULL;
            if (sTrace)
                printf("[u3d] trace %05X %03X %X %X\n", here, op, a[0], a[1]);
        }
        switch (op)
        {
        case U3C_Nop:
        case U3C_Dummy:
            break;
        case U3C_Halt:
        case U3C_End:
            if (t->depth)   // (a called script's end: back to the one that called it)
            {
                struct Frame *f = &t->frames[--t->depth];
                t->code = f->code, t->size = f->size, t->pc = f->pc, t->bank = f->bank, t->sp = f->sp;
                break;
            }
            Finish(t);
            return;
        case U3C_TimeWait:
            t->waitFrames = 2 * a[0];
            t->wait = W_FRAMES;
            break;
        case U3C_Call:
            Push(t, t->pc);
            t->pc += (s32)a[0];
            break;
        case U3C_Return:
            t->pc = Pop(t);
            break;
        case U3C_Jump:
            t->pc += (s32)a[0];
            break;
        case U3C_IfJump:
        case U3C_IfCall:
        {
            bool8 go;
            static const u8 sCond[6][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0}, {0, 1, 1}, {1, 0, 1}};
            if (a[0] == 0xFF)
                go = Pop(t) != 1;   // (after CompareStack: the "else" taken when it didn't hold)
            else
                go = a[0] < 6 && sCond[a[0]][t->cmp];
            if (go)
            {
                if (op == U3C_IfCall)
                    Push(t, t->pc);
                t->pc += (s32)a[1];
            }
            break;
        }
        case U3C_PushConst:
            Push(t, a[0]);
            break;
        case U3C_PushWorkValue:
            Push(t, V(0));
            break;
        case U3C_PopToWork:
            *WorkPtr(a[0]) = Pop(t);
            break;
        case U3C_PopDiscard:
            Pop(t);
            break;
        case U3C_Add: { u32 x = Pop(t), y = Pop(t); Push(t, y + x); break; }
        case U3C_Sub: { u32 x = Pop(t), y = Pop(t); Push(t, y - x); break; }
        case U3C_Mul: { u32 x = Pop(t), y = Pop(t); Push(t, y * x); break; }
        case U3C_Div: { u32 x = Pop(t), y = Pop(t); Push(t, x ? y / x : 0); break; }
        case U3C_PushEventFlag:
            Push(t, FlagOn(V(0)));
            break;
        case U3C_CompareStack:
        {
            u32 top = Pop(t), below = Pop(t), r;
            switch (a[0])
            {
            case 0: r = below < top; break;
            case 1: r = below == top; break;
            case 2: r = below > top; break;
            case 3: r = below <= top; break;
            case 4: r = below >= top; break;
            case 5: r = below != top; break;
            case 6: r = below == 1 || top == 1; break;
            case 7: r = below == 1 && top == 1; break;
            default: r = 0; break;
            }
            Push(t, r);
            break;
        }
        case U3C_CompareWorkConst:
        {
            u16 x = *WorkPtr(a[0]), y = a[1];
            t->cmp = x < y ? 0 : x == y ? 1 : 2;
            break;
        }
        case U3C_CompareWorks:
        {
            u16 x = *WorkPtr(a[0]), y = *WorkPtr(a[1]);
            t->cmp = x < y ? 0 : x == y ? 1 : 2;
            break;
        }
        case U3C_SetEventFlag:
            FlagTo(V(0), TRUE);
            break;
        case U3C_ResetEventFlag:
            FlagTo(V(0), FALSE);
            break;
        case U3C_CheckEventFlag:
            *WorkPtr(a[1]) = FlagOn(V(0));
            break;
        case U3C_AddWork: *WorkPtr(a[0]) += V(1); break;
        case U3C_SubWork: *WorkPtr(a[0]) -= V(1); break;
        case U3C_MulWork: *WorkPtr(a[0]) *= V(1); break;
        case U3C_DivWork: { u16 d = V(1); if (d) *WorkPtr(a[0]) /= d; break; }
        case U3C_ModWork: { u16 d = V(1); if (d) *WorkPtr(a[0]) %= d; break; }
        case U3C_AndWork: *WorkPtr(a[0]) &= V(1); break;
        case U3C_OrWork: *WorkPtr(a[0]) |= V(1); break;
        case U3C_LoadWorkConst: *WorkPtr(a[0]) = a[1]; break;
        case U3C_LoadWorkValue: *WorkPtr(a[0]) = V(1); break;
        case U3C_CopyWork: *WorkPtr(a[0]) = *WorkPtr(a[1]); break;
        case U3C_ChangeScript:   // (another script in this one's place)
        case U3C_CallScript:     // (another script run to its end, then this one goes on: SCRIPT_WORK_WaitChildScript)
        {
            const u8 *code;
            u32 size, pc;
            u16 bank;
            if (!Locate(t->zone, a[0], &code, &size, &pc, &bank))
            {
                MPNet_Log("u3d vm: no script %u", a[0]);
                break;
            }
            if (op == U3C_CallScript)
            {
                struct Frame *f;
                if (t->depth >= FRAMES)
                    break;
                f = &t->frames[t->depth++];
                f->code = t->code, f->size = t->size, f->pc = t->pc, f->bank = t->bank, f->sp = t->sp;
            }
            t->code = code, t->size = size, t->pc = pc, t->bank = bank;
            break;
        }
        case U3C_CallFieldCommand:   // (the people stop: Emerald's lock did it)
        case U3C_CallScriptEvent:
            break;
        case U3C_PopAllBgm:
            break;
        case U3C_WaitKeyDecide:
        case U3C_WaitKeyAny:
            t->wait = W_KEY;
            break;
        // ---- messages
        case U3C_BalloonWinMsg:          // file, message, person, kind, window
        case U3C_BalloonWinMsgTarget:    // file, message, kind, window
            Message(t, V(0), V(1));
            break;
        case U3C_BalloonWinMsgGender:    // file, (he, she), person...
            Message(t, V(0), gSaveBlock2Ptr->playerGender == FEMALE ? V(2) : V(1));
            break;
        case U3C_BalloonWinMsgSkipArg:   // file, (unused), message, ...
            Message(t, V(0), V(2));
            break;
        case U3C_InfoSysWinMsg:
        case U3C_InfoSysWinMsgNow:
        case U3C_PlainWinMsg:
        case U3C_PlainWinMsgFrame1:
            Message(t, MSG_OWN, V(0));
            break;
        case U3C_BgWinMsg:   // (a sign's board, a special window: it waits for a key itself, WaitBgWin)
        case U3C_BgWinMsgFont:   // (the ABYSSAL RUINS' inscriptions in their own letters: shown as a sign's board)
        case U3C_SpWinMsg:
            Message(t, MSG_OWN, a[0]);
            t->wait = W_MESSAGE_KEY;
            break;
        case U3C_CloseInfoSysWin:
        case U3C_ClosePlainWin:
        case U3C_CloseTalkWin:
        case U3C_CloseAllWins:
        case U3C_CloseBgWin:
        case U3C_CloseSpWin:
            HideFieldMessageBox();   // (also when its mode already says hidden: a finished page can leave the box drawn)
            break;
        case U3C_PROC_CallValueScreen:   // the Xtransceiver's call N: each step's words, a key between (ov174)
        {
            static s16 sCallStep = -1;
            u32 size, k;
            const u8 *c = sCallsAt ? FileOf(sCallsAt, V(0), &size) : NULL;
            if (gPaletteFade.active)   // (the scene's fade out first)
            {
                t->pc = here, t->wait = W_FADE;
                return;
            }
            if (sCallStep < 0)
            {
                // (the call over the field, in place of Black's call screen: the field faded back in for it)
                sCallStep = 0;
                if (gPaletteFade.y != 0)
                {
                    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
                    t->pc = here, t->wait = W_FADE;
                    return;
                }
            }
            for (k = 8 + 4 * sCallStep; c && k + 4 <= size && c[k] != 0; k += 4, sCallStep++)
                if (c[k] == 1)
                {
                    MPNet_Log("u3d vm: call %u, step %d", V(0), sCallStep);
                    MessageInBank(t, 0xBF + V(0), Le16(c + k + 2));   // (system bank 0xBF + N)
                    t->wait = W_MESSAGE_KEY;
                    sCallStep++;
                    t->pc = here;   // (back here for the next step)
                    return;
                }
            sCallStep = -1;
            if (!IsFieldMessageBoxHidden())
                HideFieldMessageBox();
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);   // (dark again: the scene fades in after)
            t->wait = W_FADE;
            break;
        }
        // ---- warps (more of Black's: the 3D one's tile is x, z; y the height)
        case U3C_WarpToPosition3D:   // zone, x, y, z, direction
        case U3C_WarpToPositionEvent:   // zone, x, z
        {
            u8 group, num;
            s16 x, y;
            u16 wz = a[0], wx = a[1], wzz = op == U3C_WarpToPosition3D ? a[3] : a[2];
            if (U3D_MapOfZoneTile(wz, wx, wzz, &group, &num, &x, &y))
            {
                MPNet_Log("u3d vm: warp to zone %u (%d.%d at %d,%d)", wz, group, num, x, y);
                sHolding = FALSE;
                t->wait = W_WARP;   // (Black's map change is an event the script waits on, then goes on in the new map)
                SetWarpDestination(group, num, WARP_ID_NONE, x, y);
                DoWarp();
                return;
            }
            break;
        }
        case U3C_WarpToRailLocation:   // zone, rail line, along, across, direction (its tile from u3d_railpos.h)
        case U3C_WarpToRailLocationWithTransition:
        {
            u32 i;
            u8 group, num;
            s16 x, y;
            for (i = 0; i < ARRAY_COUNT(sUnovaRailWarps); i++)
                if (sUnovaRailWarps[i].zone == a[0] && sUnovaRailWarps[i].line == a[1]
                 && sUnovaRailWarps[i].along == a[2] && sUnovaRailWarps[i].across == a[3])
                    break;
            if (i < ARRAY_COUNT(sUnovaRailWarps)
             && U3D_MapOfZoneTile(a[0], sUnovaRailWarps[i].x, sUnovaRailWarps[i].z, &group, &num, &x, &y))
            {
                MPNet_Log("u3d vm: rail warp to zone %u (%d.%d at %d,%d)", a[0], group, num, x, y);
                sHolding = FALSE;
                t->wait = W_WARP;   // (Black's map change is an event the script waits on, then goes on in the new map)
                SetWarpDestination(group, num, WARP_ID_NONE, x, y);
                DoWarp();
                return;
            }
            MPNet_Log("u3d vm: rail warp to zone %u (%u %u %u) not laid out", a[0], a[1], a[2], a[3]);
            break;
        }
        // ---- battles
        case U3C_PROC_FLD_StartScriptedWildBattle:   // species, level, flags (a legend's, a static POKeMON's)
            MPNet_Log("u3d vm: scripted wild battle: national %u Lv%u", V(0), V(1));
            CreateScriptedWildMon(Species(V(0)), V(1), ITEM_NONE);
            OpenScreen(t, SCREEN_WILD, 0);
            break;
        case U3C_POKEMON_FLD_ControlLegendMon:   // op, result: 0 is RESHIRAM (the player's own) in the party,
        {                                         // 1 to the front, 2 the boxed one swapped with slot *result first
            u16 legend = Species(643);   // (PM_VERSION_LEGEND_SPECIES: Black's player gets RESHIRAM, N has ZEKROM)
            u32 me = GetPlayerIDAsU32();
            s32 i, at = -1;
            if (V(0) == 2)
            {
                u16 slot = *WorkPtr(a[1]);
                u32 b, p;
                for (b = 0; b < TOTAL_BOXES_COUNT && slot < PARTY_SIZE; b++)
                    for (p = 0; p < IN_BOX_COUNT; p++)
                        if (GetBoxMonDataAt(b, p, MON_DATA_SPECIES) == legend && !GetBoxMonDataAt(b, p, MON_DATA_IS_EGG)
                            && GetBoxMonDataAt(b, p, MON_DATA_OT_ID) == me)
                        {
                            struct BoxPokemon boxed = *GetBoxedMonPtr(b, p);
                            SetBoxMonAt(b, p, &gPlayerParty[slot].box);
                            BoxMonToMon(&boxed, &gPlayerParty[slot]);
                            b = TOTAL_BOXES_COUNT;
                            break;
                        }
            }
            for (i = 0; i < PARTY_SIZE && at < 0; i++)
                if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == legend && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG)
                    && GetMonData(&gPlayerParty[i], MON_DATA_OT_ID) == me)
                    at = i;
            if (V(0) == 0)
                *WorkPtr(a[1]) = at >= 0;
            else if (at > 0)
            {
                struct Pokemon tmp = gPlayerParty[0];
                gPlayerParty[0] = gPlayerParty[at];
                gPlayerParty[at] = tmp;
            }
            break;
        }
        case U3C_WORK_CmdCheckLastBattleResultB:
            *WorkPtr(a[0]) = U3D_BattleResult() != 2;
            break;
        case U3C_WORK_CmdEndScriptWhiteOutB:
            sHolding = FALSE;
            t->alive = FALSE;
            return;
        case U3C_PROC_FLD_StoreBattleResult:
            *WorkPtr(a[0]) = sWildResult;
            break;
        // ---- the party
        case U3C_POKEMON_FLD_SelectPartyMon:   // result (1 chosen), slot, mode (as every script of Black's reads them)
            sPartySlotWork = a[1];
            OpenScreen(t, SCREEN_PARTY, a[0]);
            break;
        case U3C_PROC_CheckMonIsEgg:   // result, slot
            *WorkPtr(a[0]) = V(1) < PARTY_SIZE && GetMonData(&gPlayerParty[V(1)], MON_DATA_IS_EGG);
            break;
        case U3C_PROC_GetMonSpeciesOrEgg:   // result, slot (national; an egg 0x28F, Black's EGG)
        {
            u16 sp = V(1) < PARTY_SIZE ? GetMonData(&gPlayerParty[V(1)], MON_DATA_SPECIES_OR_EGG) : SPECIES_NONE;
            *WorkPtr(a[0]) = sp == SPECIES_EGG ? 0x28F : sp ? SpeciesToNationalPokedexNum(sp) : 0;
            break;
        }
        case U3C_PROC_GetMonFriendship:   // result, slot
            *WorkPtr(a[0]) = V(1) < PARTY_SIZE ? GetMonData(&gPlayerParty[V(1)], MON_DATA_FRIENDSHIP) : 0;
            break;
        case U3C_PROC_GetMonMove:   // result, slot, index
            *WorkPtr(a[0]) = V(1) < PARTY_SIZE && V(2) < MAX_MON_MOVES ? GetMonData(&gPlayerParty[V(1)], MON_DATA_MOVE1 + V(2)) : 0;
            break;
        case U3C_POKEMON_FLD_GetMonIvStat:   // slot, stat, result
            *WorkPtr(a[2]) = V(0) < PARTY_SIZE && V(1) < NUM_STATS ? GetMonData(&gPlayerParty[V(0)], MON_DATA_HP_IV + V(1)) : 0;
            break;
        case U3C_PROC_FindFirstPartyIndex:   // result, mode (1 not an egg, 2 able to battle)
        {
            u32 i;
            *WorkPtr(a[0]) = 0;
            for (i = 0; i < PARTY_SIZE; i++)
            {
                struct Pokemon *mon = &gPlayerParty[i];
                if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
                    continue;
                if (V(1) == 2 && GetMonData(mon, MON_DATA_HP) == 0)
                    continue;
                *WorkPtr(a[0]) = i;
                break;
            }
            break;
        }
        case U3C_PROC_FindPartyMonBySpecies:   // species, found, slot
        {
            u32 i;
            u16 want = Species(V(0));
            *WorkPtr(a[1]) = 0;
            for (i = 0; i < PARTY_SIZE; i++)
                if (!GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG) && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == want)
                {
                    *WorkPtr(a[1]) = 1, *WorkPtr(a[2]) = i;
                    break;
                }
            break;
        }
        case U3C_PROC_GetBoxMonCount:   // result, kind
            *WorkPtr(a[0]) = CountAllStorageMons();   // (the PC's POKeMON)
            break;
        // ---- money (the game's money box)
        case U3C_OpenMoneyWin:   // x, y
            DrawMoneyBox(GetMoney(&gSaveBlock1Ptr->money), 0, 0);
            break;
        case U3C_UpdateMoneyWin:
            ChangeAmountInMoneyBox(GetMoney(&gSaveBlock1Ptr->money));
            break;
        case U3C_CloseMoneyWin:
            HideMoneyBox();
            break;
        case U3C_PROC_CheckMoneyAtLeast:   // result, amount
            *WorkPtr(a[0]) = IsEnoughMoney(&gSaveBlock1Ptr->money, V(1));
            break;
        case U3C_PROC_SubMoney:
            RemoveMoney(&gSaveBlock1Ptr->money, V(0));
            break;
        // ---- the time, the season, the player
        case U3C_WORK_CmdGetHourAndMinute:
        {
            struct SiiRtcInfo dt;
            Platform_GetDateTime(&dt);
            *WorkPtr(a[0]) = ConvertBcdToBinary(dt.hour), *WorkPtr(a[1]) = ConvertBcdToBinary(dt.minute);
            break;
        }
        case U3C_WORK_CmdGetSeason:
            *WorkPtr(a[0]) = U3D_Season();
            break;
        case U3C_GetPlayerForm:   // (0 walking, 1 a bike, 2 surfing)
            *WorkPtr(a[0]) = TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING) ? 2
                           : TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE) ? 1 : 0;
            break;
        // ---- a person's move code, event flag, params (Black's event record; a script's change kept by obj id)
        case U3C_GetMmdlMoveCode:   // result, person
            *WorkPtr(a[0]) = MoveCodeOf(V(1));
            break;
        case U3C_ChangeMmdlMoveCode:   // person, code
            ChangeMoveCode(V(0), V(1));
            break;
        case U3C_GetMmdlEventFlag:   // person, result
        {
            const u8 *p = PersonById(V(0));
            if (p)
                *WorkPtr(a[1]) = Le16(p + 8);
            break;
        }
        case U3C_GetMmdlParam:   // person, index 0-2, result
        {
            const u8 *p = PersonById(V(0));
            if (p && V(1) <= 2)
                *WorkPtr(a[2]) = Le16(p + 14 + 2 * V(1));
            break;
        }
        case U3C_SearchMmdlInFrontOfPlayer:   // obj id, found
        case U3C_SearchMmdlAtGrid:            // obj id, found, x, y, z (Black's tiles)
        {
            s16 x, y;
            s32 m = U3D_MapIndex();
            u8 id;
            if (op == U3C_SearchMmdlInFrontOfPlayer)
            {
                GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
            }
            else
            {
                x = V(2) - (m >= 0 ? 32 * U3D_MapCellX(m) : 0) + MAP_OFFSET;
                y = V(4) - (m >= 0 ? 32 * U3D_MapCellZ(m) : 0) + MAP_OFFSET;
            }
            id = GetObjectEventIdByXY(x, y);
            *WorkPtr(a[1]) = id < OBJECT_EVENTS_COUNT && id != gPlayerAvatar.objectEventId;
            if (*WorkPtr(a[1]))
                *WorkPtr(a[0]) = BlackObjOf(gObjectEvents[id].localId);
            break;
        }
        // ---- the POKeDEX's ratings (scrcmd_pokemon_fld.c; zukan_save.c: UNOVA's are 494-649, the counted leave out
        // the excluded; complete is 150 regional, 634 national)
        case U3C_POKEMON_FLD_GetPokedexComplete:   // result, kind (1 national caught, 2 regional seen, 3 caught)
            *WorkPtr(a[0]) = DexCount(V(1), TRUE) >= (V(1) == 1 ? 634 : 150);
            break;
        case U3C_POKEMON_FLD_GetPokedexRating:   // kind, rating message, count
        {
            u32 kind = V(0), count, k;
            *WorkPtr(a[1]) = 5;
            *WorkPtr(a[2]) = 0;
            if (kind < 1 || kind > 3)
                break;
            count = DexCount(kind, FALSE);
            if (DexCount(kind, TRUE) >= (kind == 1 ? 634 : 150))
                *WorkPtr(a[1]) = kind == 1 ? 0x32 : kind == 2 ? 0xD : 0x1E;
            else
                for (k = 0; k < ARRAY_COUNT(sDexRating[0]); k++)
                    if (sDexRating[kind - 1][k][0] == 0 || count <= sDexRating[kind - 1][k][0])
                    {
                        *WorkPtr(a[1]) = sDexRating[kind - 1][k][1];
                        break;
                    }
            *WorkPtr(a[2]) = count;
            break;
        }
        // ---- the item buyers' and prize counters' key / value tables (archive 0xD0)
        case U3C_POKEMON_FLD_CheckTableItemInBag:   // table, result: any of its items in the bag
        {
            u32 k;
            *WorkPtr(a[1]) = 0;
            for (k = 0; V(0) < ARRAY_COUNT(sKeyValueTables) && k < sKeyValueTables[V(0)][1]; k++)
                if (Item(sKeyValues[sKeyValueTables[V(0)][0] + k][0]) != ITEM_NONE
                 && CheckBagHasItem(Item(sKeyValues[sKeyValueTables[V(0)][0] + k][0]), 1))
                {
                    *WorkPtr(a[1]) = 1;
                    break;
                }
            break;
        }
        case U3C_POKEMON_FLD_LookupTableValue:          // key, table, result
        case U3C_POKEMON_FLD_RegisterTableValueNumber:  // key, table, word slot, digits
        case U3C_POKEMON_FLD_AddTableValueToPoints:     // key, table: the value as money
        {
            u32 k, v = 0;
            bool8 found = FALSE;
            for (k = 0; V(1) < ARRAY_COUNT(sKeyValueTables) && k < sKeyValueTables[V(1)][1]; k++)
                if (sKeyValues[sKeyValueTables[V(1)][0] + k][0] == V(0))
                {
                    v = sKeyValues[sKeyValueTables[V(1)][0] + k][1];
                    found = TRUE;
                    break;
                }
            if (op == U3C_POKEMON_FLD_LookupTableValue)
                *WorkPtr(a[2]) = v;
            else if (op == U3C_POKEMON_FLD_RegisterTableValueNumber && found)
                ConvertIntToDecimalStringN(Slot(V(2)), v, STR_CONV_MODE_LEFT_ALIGN, V(3) ? V(3) : 8);
            else if (found)
                AddMoney(&gSaveBlock1Ptr->money, v);
            break;
        }
        // ---- ROUTE 3's DAY CARE: Black's scripts on the game's own DAY CARE (Kai's breeding; not reworked)
        case U3C_PROC_CheckDaycareHasEgg:   // result
            *WorkPtr(a[0]) = GetDaycareState() == DAYCARE_EGG_WAITING;
            break;
        case U3C_PROC_ReceiveDaycareEgg:
            GiveEggFromDaycare();
            break;
        case U3C_PROC_DeclineDaycareEgg:
            RejectEggFromDayCare();
            break;
        case U3C_PROC_GetDaycareMonCount:   // result
            *WorkPtr(a[0]) = CountPokemonInDaycare(&gSaveBlock1Ptr->daycare);
            break;
        case U3C_PROC_GetDaycareCompatibility:   // result (0, 20, 50, 70, as Black's)
            *WorkPtr(a[0]) = MP_Daycare_Compatibility(&gSaveBlock1Ptr->daycare.mons[0].mon, &gSaveBlock1Ptr->daycare.mons[1].mon);
            break;
        case U3C_PROC_DepositDaycareMon:   // party slot
            MP_Daycare_Deposit(V(0));
            break;
        case U3C_PROC_WithdrawDaycareMon:   // day care slot
            gSpecialVar_0x8004 = V(0);
            TakePokemonFromDaycare();
            break;
        case U3C_PROC_GetDaycareMonSpecies:   // result, slot (Black's numbers: national)
            *WorkPtr(a[0]) = V(1) < DAYCARE_MON_COUNT ? SpeciesToNationalPokedexNum(
                GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[V(1)].mon, MON_DATA_SPECIES)) : 0;
            break;
        case U3C_PROC_GetDaycareMonForm:   // result, slot (the game keeps no forms outside battle)
            *WorkPtr(a[0]) = 0;
            break;
        case U3C_PROC_GetDaycareMonGender:                 // result, slot (0 male, 1 female, 2 none)
        case U3C_PROC_GetDaycareMonGenderKindIfFirst:      // result, kind, slot
        {
            u16 slot = op == U3C_PROC_GetDaycareMonGender ? V(1) : V(2);
            u8 g = slot < DAYCARE_MON_COUNT ? GetBoxMonGender(&gSaveBlock1Ptr->daycare.mons[slot].mon) : MON_GENDERLESS;
            *WorkPtr(a[0]) = op == U3C_PROC_GetDaycareMonGenderKindIfFirst && V(1) != 0 ? 0
                           : g == MON_MALE ? 0 : g == MON_FEMALE ? 1 : 2;
            break;
        }
        case U3C_PROC_GetDaycareSlotLevel:   // result, slot (its level now)
            *WorkPtr(a[0]) = MP_Daycare_LevelNow(V(1));
            break;
        case U3C_PROC_GetDaycareSlotLevelsGrown:   // result, slot
            gSpecialVar_0x8004 = V(1);
            *WorkPtr(a[0]) = V(1) < DAYCARE_MON_COUNT ? GetNumLevelsGainedFromDaycare() : 0;
            break;
        case U3C_PROC_GetDaycareSlotPrice:   // result, slot
            gSpecialVar_0x8004 = V(1);
            GetDaycareCostAndPrepareString();
            *WorkPtr(a[0]) = gSpecialVar_0x8005;
            break;
        case U3C_BufferDaycareMonNickname:   // word slot, day care slot
            if (V(1) < DAYCARE_MON_COUNT)
            {
                GetBoxMonNickname(&gSaveBlock1Ptr->daycare.mons[V(1)].mon, Slot(a[0]));
                StringGet_Nickname(Slot(a[0]));
            }
            break;
        case U3C_PROC_CallDaycarePartyMenu:   // result: the slot picked (6 none), the game's day care party screen
            OpenScreen(t, SCREEN_DAYCARE_PARTY, a[0]);
            break;
        case U3C_PlayPerapVoice:   // species, form (CHATOT's recorded voice: its cry here)
            PlayCry_Normal(Species(V(0)), 0);
            break;
        case U3C_StartWaitBgmStopped:
            t->wait = W_BGM_STOPPED;
            t->waitFrames = 60 * 30;   // (a song that loops never ends: half a minute at most)
            break;
        case U3C_IsItemMachine:   // item, result: a TM or an HM
            *WorkPtr(a[1]) = Item(V(0)) != ITEM_NONE && GetPocketByItemId(Item(V(0))) == POCKET_TM_HM;
            break;
        case U3C_WORK_CmdGetButtonMode:   // (Black's L / R button setting: the game's own, "normal")
            *WorkPtr(a[0]) = 0;
            break;
        case U3C_WORK_CmdCheckExtraSaveData:   // (the HALL OF FAME's records once Black's flag 0x97B says the game is
                                               // cleared: 1 some)
            *WorkPtr(a[0]) = FlagOn(0x97B) ? 1 : 0;
            break;
        case U3C_WORK_CmdGetTrainerViewGroupAndMarkSeen:   // seen before, the look's group (the TRAINER CARD's sign)
            *WorkPtr(a[0]) = sWork.misc.record[127] != 0;
            *WorkPtr(a[1]) = 0;
            sWork.misc.record[127] = 1;
            break;
        case U3C_PROC_LookupTableValue:   // index, result (ov010 sScrcmdLookupTable)
            *WorkPtr(a[1]) = V(0) < ARRAY_COUNT(sProcLookup) ? sProcLookup[V(0)] : 0;
            break;
        case U3C_GetMmdlGridPos:   // person, x, z (Black's tiles)
        {
            struct ObjectEvent *o = ObjectOf(V(0));
            s32 m = U3D_MapIndex();
            if (o)
            {
                *WorkPtr(a[1]) = o->currentCoords.x - MAP_OFFSET + (m >= 0 ? 32 * U3D_MapCellX(m) : 0);
                *WorkPtr(a[2]) = o->currentCoords.y - MAP_OFFSET + (m >= 0 ? 32 * U3D_MapCellZ(m) : 0);
            }
            break;
        }
        case U3C_WORK_CmdGetMsgDataId:   // (the script's own words' file: a shared script prints from it)
            *WorkPtr(a[0]) = t->bank >= SYSTEM_BANKS ? t->bank - SYSTEM_BANKS : t->bank;
            break;
        case U3C_GetContinueFlag:
            *WorkPtr(a[0]) = 0;
            break;
        // ---- what has no screen of its own here yet (read past quietly)
        // ---- the POKeDEX (national numbers: Black's species numbers are the game's national ones)
        case U3C_WORK_CmdGetNationalZukanCount:   // mode (0 seen, else caught), result
            *WorkPtr(a[1]) = GetNationalPokedexCount(V(0) == 0 ? FLAG_GET_SEEN : FLAG_GET_CAUGHT);
            break;
        case U3C_WORK_CmdRegisterZukanSeen:   // species, form
            if (V(0) && V(0) <= NATIONAL_DEX_COUNT)
                GetSetPokedexFlag(V(0), FLAG_SET_SEEN);
            break;
        case U3C_WORK_CmdCheckZukanSeenOrCaught:   // species, mode (0 seen, else caught), result
            *WorkPtr(a[2]) = V(0) && V(0) <= NATIONAL_DEX_COUNT
                           && GetSetPokedexFlag(V(0), V(1) == 0 ? FLAG_GET_SEEN : FLAG_GET_CAUGHT);
            break;
        case U3C_WORK_CmdLookupTable:   // index, result (ov010 gScrcmdValueTable)
        {
            static const u16 sValues[] = {15, 55, 64, 30, 53, 43, 154, 13};
            *WorkPtr(a[1]) = V(0) < ARRAY_COUNT(sValues) ? sValues[V(0)] : 0;
            break;
        }
        case U3C_PLEASURE_BOAT_ScrCmdCreate:   // the ROYAL UNOVA: the day's passengers, the trip's clock
            if (!sBoat.alive)
                BoatCreate();
            break;
        case U3C_PLEASURE_BOAT_ScrCmdDelete:
            sBoat.alive = FALSE;
            break;
        case U3C_PLEASURE_BOAT_ScrCmdGetStat:
            *WorkPtr(a[1]) = sBoat.alive ? BoatStat(a[0]) : 0;
            break;
        case U3C_PLEASURE_BOAT_ScrCmdAddTime:
            if (sBoat.alive)
                BoatAddTime(a[0], a[1]);
            break;
        case U3C_SYMBOL_MAP_ScrCmdGetEventStateValue:   // (a cabin's passenger: 4 object, 5 kind, 6 trainer, 7 words, 8 met)
        {
            u16 i = V(0), out = 0;
            if (!sBoat.alive || i >= 15)
                out = a[1] == 4 && !sBoat.alive ? 10 : 0;
            else if (a[1] == 4)
                out = sBoat.p[i].value;
            else if (a[1] == 5)
                out = sBoat.p[i].param;
            else if (a[1] == 6)
                out = sBoat.p[i].type;
            else if (a[1] == 7)
                out = sBoat.p[i].extra[V(2) & 1];
            else if (a[1] == 8)
                out = sBoat.p[i].occupied != 0;
            *WorkPtr(a[3]) = out;
            break;
        }
        case U3C_PLEASURE_BOAT_ScrCmdSetPassengerInfo:   // (only 8: the cabin's passenger met)
            if (sBoat.alive && a[1] == 8 && V(0) < 15)
                sBoat.p[V(0)].occupied = V(3);
            break;
        case U3C_SYMBOL_MAP_ScrCmdStartRecordTask:   // (the deck's view of the sea, ov203's 3D scene: not drawn here; 0 =
            *WorkPtr(a[0]) = 0;                      // looked away before the trip's end, the boat sails on)
            break;
        case U3C_PLEASURE_BOAT_ScrCmdLock:
            sBoat.locked = TRUE;
            break;
        case U3C_SetAutoMode:   // (Black's MSGWIN auto mode: messages go on by themselves, the MUSICAL's announcements;
            gTextFlags.autoScroll = V(0) != 0;   // the game's own text auto-scroll, till the script ends)
            break;
        // ---- the POKeMON MUSICALS (Black's scrcmd_musical.c; the show itself: u3d_musical.c)
        case U3C_MUSICAL_StartMusical:   // party slot, mode: 0 a show (1 Black's link show: the same, friends joining by
        {                                // their dress-ups), 2 the theater's photo viewer (no photos kept here)
            bool8 Unova_Musical_Begin(u8 slot);
            if (a[1] <= 1 && Unova_Musical_Begin(V(0)))
            {
                Finish(t);   // (Black's VM_End: the show is an event of its own, its scripts run in turn)
                return;
            }
            break;
        }
        case U3C_MUSICAL_ShowPartyMonMusical:   // party slot: the dressing room for practice
        {
            bool8 Unova_Musical_Practice(u8 slot);
            if (Unova_Musical_Practice(V(0)))
            {
                t->wait = W_MUSICAL;
                if (sHolding)
                    ScriptContext_Enable();
            }
            break;
        }
        case U3C_MUSICAL_GetValue:   // op, value, result
        {
            u16 Unova_Musical_GetValue(u8 op, u16 arg, bool8 *set);
            bool8 set;
            u16 v = Unova_Musical_GetValue(a[0], V(1), &set);
            if (set)
                *WorkPtr(a[2]) = v;
            break;
        }
        case U3C_MUSICAL_GetProgramInfo:   // fan, kind, result (Black's "programs": the ten fans after a show)
        {
            u16 Unova_Musical_FanInfo(u8 slot, u8 kind);
            *WorkPtr(a[2]) = Unova_Musical_FanInfo(V(0), a[1]);
            break;
        }
        case U3C_MUSICAL_AddOwnedProp:
        {
            void Unova_Musical_AddProp(u8 prop);
            Unova_Musical_AddProp(V(0));
            break;
        }
        case U3C_MUSICAL_ResetProgramRank:   // (a fan's prop handed over)
        {
            void Unova_Musical_ResetFan(u8 slot);
            Unova_Musical_ResetFan(V(0));
            break;
        }
        case U3C_MUSICAL_BufferString:   // kind, word slot, value
        {
            void Unova_Musical_BufferString(u8 kind, u8 slot, u16 arg);
            Unova_Musical_BufferString(a[0], a[1], V(2));
            break;
        }
        case U3C_MUSICAL_SelectPartyMon:   // result (1 chosen), slot: the game's party screen
            sPartySlotWork = a[1];
            OpenScreen(t, SCREEN_PARTY, a[0]);
            break;
        case U3C_MUSICAL_LinkControl:   // command, value, -, result: Black's wireless sessions; alone (and in Kai's
            if (V(0) == 12 || V(0) == 13 || V(0) >= 19)   // rooms) there's none: no session, no error
                *WorkPtr(a[3]) = 0;
            break;
        case U3C_MUSICAL_WifiListCommand:   // (the Wi-Fi shows' list: none)
            *WorkPtr(a[1]) = 0;
            break;
        case U3C_PROC_FLD_StartFkDemoSubProc:   // (an Elite Four room's 3D entrance flight: not drawn)
        case U3C_PROC_FLD_SetCameraSettingFromFkDemoSlot:
        case U3C_StartMmdlRailSlipDown:   // (a slide down a rail slope: the tiles walk it)
        case U3C_FadeBgmVolume:   // (the song quieter under a scene: kept as it is)
        case U3C_RestoreBgmVolume:
        case U3C_PROC_CallSubEvent:   // (a field menu's screen opened from a scene)
        case U3C_FLD_FACEUP_ScrCmdStart:   // (a speaker's face-up portrait: the words come in the script's messages)
        case U3C_FLD_FACEUP_ScrCmdWaitEnd:
        case U3C_FLD_FACEUP_ScrCmdPrepareField:
        case U3C_NETWORK_CmdRequestCommExit:   // (Black's wireless: Kai's co-op stands in for it)
        case U3C_NETWORK_CmdRestartAutoComm:
        case U3C_NETWORK_CmdStartAutoComm:
        case U3C_NETWORK_CmdWaitCommState:
        case U3C_NETWORK_CmdClearAlwaysNet:
        case U3C_StopSe:
        case U3C_SAVEANIME_Start:
        case U3C_TurnPlayerToFrontEvent:
        case U3C_SYMBOL_MAP_ScrCmdSetSubscreenMode:   // (the lower screen's mode: Kai has one screen)
        case U3C_WORK_CmdSetBoxFlags:   // (Black's PC wallpapers the game clear unlocks: Kai's PC keeps its own)
            break;
        // Black's build models played by its scripts (scrcmd_screeneffects.c; u3d.c U3D_BModel*): a model of an entry
        // kind near a tile, all of its model (PlayEntryAnim) or its own copy, a node by its handle
        case U3C_SCREENEFFECTS_CreateBuildModelNode:   // handle work, kind, x, z
            *WorkPtr(a[0]) = U3D_BModelNode(V(1), V(2), V(3));
            break;
        case U3C_SCREENEFFECTS_DeleteBuildModelNode:
            U3D_BModelNodeDelete(V(0));
            break;
        case U3C_SCREENEFFECTS_PlayBuildModelNodeSlotWithSe:   // handle, slot (opening; its kind's sound)
            U3D_BModelNodePlay(V(0), V(1), 0);
            PlaySE(SE_DOOR);
            break;
        case U3C_SCREENEFFECTS_PlayBuildModelNodeSlotMode2:   // handle, slot (looping)
            U3D_BModelNodePlay(V(0), V(1), 2);
            break;
        case U3C_SCREENEFFECTS_PlayBuildModelNodeCurrentMode3:   // handle (stopped where it is)
            U3D_BModelNodePlay(V(0), -1, 3);
            break;
        case U3C_SCREENEFFECTS_WaitBuildModelNodeAnim:   // handle: till its slot is still
            sBModelWait = V(0);
            if (U3D_BModelNodeBusy(sBModelWait))
                t->wait = W_BMODEL;
            break;
        case U3C_SCREENEFFECTS_PlayBuildModelAnimPair:   // kind, x, z: slot 0 closing, then stopped (shown shut)
            U3D_BModelPlay(V(0), V(1), V(2), 0, 1);
            U3D_BModelPlay(V(0), V(1), V(2), 0, 3);
            break;
        case U3C_SCREENEFFECTS_PlayBuildModelAnimTwo:   // kind, x, z: slot 0 looping
            U3D_BModelPlay(V(0), V(1), V(2), 0, 2);
            break;
        case U3C_SCREENEFFECTS_StartDoorOpen:   // the automatic door by the player (field_door_anime.c)
            U3D_DoorNearPlayer(0);
            t->wait = W_BMODEL_DOOR;
            break;
        case U3C_SCREENEFFECTS_StartDoorSetOpen:
            U3D_DoorNearPlayer(1);
            break;
        case U3C_SCREENEFFECTS_StartDoorClose:   // quiet (no sound) when its value says so
            U3D_DoorNearPlayer(V(0) ? 3 : 2);
            break;
        case U3C_SCREENEFFECTS_SwapBuildModelId:   // (a model swapped for another of its block: none here)
        case U3C_SCREENEFFECTS_SetBuildModelIndex:
            break;
        case U3C_NETWORK_CmdGetCommValue:   // (wireless on: Kai's co-op stands in for it, docs/UNOVA_DECISIONS.md)
            *WorkPtr(a[0]) = 1;
            break;
        case U3C_PROC_FLD_ObtainCGearAndCallCommResetEvent:   // (the C-GEAR got: CGEAR_SAVE_SetObtained)
            sWork.cgear = TRUE;
            break;
        case U3C_ShowOrClearInfoWindow:   // (Black's touch screen's info window: nothing to show on one screen)
            break;
        case U3C_InitMenu:   // x, y, cursor, cancellable, result
            MenuInit(262, V(2), a[3], WorkPtr(a[4]));
            break;
        case U3C_InitMenuMsgData:
        case U3C_InitMenuMsgDataMode1:
            MenuInit(t->bank, V(2), a[3], WorkPtr(a[4]));
            break;
        case U3C_AddMenuItem:   // message, help message, value
            MenuAdd(a[0], V(2));
            break;
        case U3C_StartMenu:
        case U3C_StartMenuEx:
            MenuStart(t);
            break;
        case U3C_YesNoWin:
            DisplayYesNoMenuDefaultYes();
            t->result = WorkPtr(a[0]);
            t->wait = W_YESNO;
            break;
        case U3C_BufferPlayerName:
            StringCopy(Slot(a[0]), gSaveBlock2Ptr->playerName);
            break;
        case U3C_BufferSpeciesName:
            GetSpeciesName(Slot(a[0]), Species(V(1)));
            break;
        // ---- Black's scene cameras (u3d.c): to a place and back; the player's view is the one "saved"
        case U3C_PROC_FLD_SaveCamera:
        case U3C_PROC_FLD_StopCameraTargetTrace:
        case U3C_PROC_FLD_ResumeCameraTargetTrace:
            break;
        case U3C_PROC_FLD_StartCameraShake:   // x, y, steps, swings, dx, dy, after, every (scrcmd_misc.c ShakeTask)
            U3D_CameraShake(V(0), V(1), V(2), V(3), V(4), V(5), V(6), V(7));
            MPNet_Log("u3d vm: the camera shakes (%d, %d, %u swings of %u)", V(0), V(1), V(3), V(2));
            t->wait = W_SHAKE;
            break;
        case U3C_PROC_FLD_StartCameraMoveFromArchive:   // member, frames: Black's archive 0xA3 (u3d_cameras.h)
            if (a[0] < ARRAY_COUNT(sCameraMoves))
            {
                const typeof(sCameraMoves[0]) *cm = &sCameraMoves[a[0]];
                U3D_CameraMoveMasked(cm->pitch, cm->yaw, cm->dist, cm->x, cm->y, cm->z, a[1], cm->mask);
            }
            break;
        case U3C_PROC_FLD_StartCameraMoveFromStream:   // pitch, yaw, distance, x, y, z, frames
            U3D_CameraMoveTo(a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
            break;
        case U3C_PROC_FLD_StartCameraMoveToSaved:
        case U3C_PROC_FLD_StartCameraMoveToSetting:
            U3D_CameraMoveBack(a[0]);
            break;
        case U3C_PROC_FLD_RestoreCameraCmd:
            if (!U3D_CameraMoving())
                U3D_CameraReset();
            break;
        case U3C_PROC_FLD_WaitCameraIdle:
            t->wait = W_CAMERA;
            break;
        // ---- the save's misc bytes (Black's Wi-Fi and event slots: none here)
        case U3C_WORK_CmdSetMiscByte:
            break;
        case U3C_WORK_CmdGetMiscByte:
        case U3C_WORK_CmdCheckMiscByte:
            *WorkPtr(a[1]) = 0;
            break;
        case U3C_PROC_FLD_CallCatchTutorial:   // JUNIPER's catching demonstration (u3d_battle.c)
            OpenScreen(t, SCREEN_CATCH_DEMO, 0);
            break;
        case U3C_StartFld3dCi:   // (Black's 3D cut-ins: the POKéDEX's, a Gym Leader's... not drawn yet)
        case U3C_StartFld3dCiByTable:
        case U3C_StartFld3dCiByVar:   // (a field move's: its POKéMON over the field)
        case U3C_IAIGIRI_StartOnPlayer:   // (CUT's slash on the player: the tree goes with its own sound after)
        case U3C_PALPARK_SetBeacon:   // (a link beacon of what the player did: no wireless here)
        case U3C_DebugWork:   // (Black's debug prints)
        case U3C_DebugStack:
            break;
        case U3C_FIELD_GIMMICK_LIFT_ELEVATOR_CallLiftEvent:   // CLAY's gym: a lever sends its lift to its other height
            if (V(0) < 6 && sMineRide.lift < 0)   // (sMineLift's)
            {
                MineLift(V(0));
                t->wait = W_LIFT_RIDE;
            }
            break;
        // (the lobby's lift and the mine's elevator between them: Black's ov044 / ov043 rides, the player on board)
        case U3C_FIELD_GIMMICK_LIFT_ELEVATOR_SnapElevatorToTop:   // (the mine's elevator up at 880, arriving)
            U3D_ExpSetY(7, 880);
            break;
        case U3C_FIELD_GIMMICK_LIFT_ELEVATOR_CallElevatorEvent:   // rising (to the lobby, 800 to 880) or down (880 to 800)
            if (CurrentZone() == ZONE_DRIFTVEIL_GYM)
            {
                PlatRide(7, 2, 13, 24, 5, 3, a[0] ? 800 : 880, a[0] ? 880 : 800);
                if (!a[0])
                    LiftTexShow(7, 1);   // (going down: its track 1, played as it arrives)
                t->wait = W_LIFT_RIDE;
            }
            break;
        case U3C_FIELD_GIMMICK_LIFT_SnapLiftToBottom:   // (the lobby's lift down at -80, arriving)
            U3D_ExpSetY(0, -80);
            break;
        case U3C_FIELD_GIMMICK_LIFT_CallEvent:   // descending (0 to -80) or not (-80 to 0)
            if (CurrentZone() == ZONE_CLAY_LOBBY)
            {
                PlatRide(0, 0, 5, 3, 5, 3, a[0] ? 0 : -80, a[0] ? -80 : 0);
                if (!a[0])
                    LiftTexShow(0, 0);   // (coming up: its track 0, played as it arrives)
                t->wait = W_LIFT_RIDE;
            }
            break;
        case U3C_FIELD_GIMMICK_SWITCH_TOGGLE_ReplacePlayerIfRequirementMet:   // BRYCEN's gym: onto the slide
            if (IceRide(a[0]))
                t->wait = W_MOVES;
            break;
        case U3C_FIELD_GIMMICK_SWITCH_TOGGLE_StartSecondarySwitchEvent:   // (its switch's and ice piece's models
            if (CurrentZone() == ICE_GYM_ZONE && a[0] < 3)                   // turning: the works and blockers say it;
                IceSecondary(a[0]);                                          // the second model's light: IceSecondary)
            break;
        // STRIATON's gym (ov042 button states) and NACRENE's (ov040 flag anime): models only, the way through being
        // Black's works and people. Their states are kept for the models: sWork.gym[6] the curtains played to their
        // end (oneShotPlayed, bits 0-2), sButtonModels the nine state models at their last frame; sWork.gym[7] bit 0
        // NACRENE's shelf moved
        case U3C_FIELD_GIMMICK_BUTTON_STATES_CreateOneShotEventU16:   // (in BRYCEN's gym: its switch's toggle)
            if (CurrentZone() == ICE_GYM_ZONE)
            {
                if (a[0] < 3 && !(sIceShown[a[0]] & 2))
                {
                    IceToggle(a[0]);
                    sExpWait = a[0];
                    if (U3D_ExpPlaying(sExpWait))
                        t->wait = W_EXP_ANIM;
                }
                break;
            }
            // fall through
        case U3C_FIELD_GIMMICK_BUTTON_STATES_CreateOneShotEvent:   // a curtain's one-shot animation, played
            if (a[0] < 3)
            {
                sWork.gym[6] |= 1 << a[0];
                if (CurrentZone() == ZONE_STRIATON_GYM)
                {
                    U3D_ExpAnim(9 + a[0], 0, 0, TRUE);
                    sExpWait = 9 + a[0];
                    if (U3D_ExpPlaying(sExpWait))
                        t->wait = W_EXP_ANIM;
                }
            }
            break;
        case U3C_FIELD_GIMMICK_BUTTON_STATES_CreateStateEventWithFlag:   // a state model at its last frame
        case U3C_FIELD_GIMMICK_BUTTON_STATES_CreateStateEvent:   // ... at its first
            if (a[0] < 9)
            {
                bool8 last = op == U3C_FIELD_GIMMICK_BUTTON_STATES_CreateStateEventWithFlag;
                sButtonModels = last ? sButtonModels | (1 << a[0]) : sButtonModels & ~(1 << a[0]);
                sButtonModelsSet |= 1 << a[0];
                if (CurrentZone() == ZONE_STRIATON_GYM)
                    U3D_ExpAnim(a[0], 0, last ? -1 : 0, FALSE);
            }
            break;
        case U3C_PROC_Ov037SetValueB:   // (GIMMICK_INIT_SetButtonStatesFlags: the curtains played, from the works;
            sWork.gym[6] = V(0) & 7;    //  the zone's script sets it as the gym is entered)
            if (CurrentZone() == ZONE_STRIATON_GYM)
            {
                u32 k;
                for (k = 0; k < 3; k++)
                    U3D_ExpAnim(9 + k, (sWork.gym[6] >> k) & 1 ? 0 : 0xFF, -1, FALSE);
            }
            break;
        case U3C_PROC_Ov037SetValueA:   // (GIMMICK_INIT_SetFlagAnimeFlag)
            sWork.gym[7] = V(0) ? sWork.gym[7] | 1 : sWork.gym[7] & ~1;
            if (CurrentZone() == ZONE_NACRENE_GYM)
                U3D_ExpAnim(0, sWork.gym[7] & 1 ? 0 : 0xFF, -1, FALSE);
            break;
        case U3C_FIELD_GIMMICK_FLAG_ANIME_CreateEvent:   // NACRENE's shelf slides aside (its model, played to its end)
            sWork.gym[7] |= 1;
            if (CurrentZone() == ZONE_NACRENE_GYM && a[0] < 1)
            {
                U3D_ExpAnim(0, 0, 0, TRUE);
                sExpWait = 0;
                if (U3D_ExpPlaying(sExpWait))
                    t->wait = W_EXP_ANIM;
            }
            break;
        case U3C_FIELD_GIMMICK_CART_LANES_CreateSwitchEvent:   // ELESA's gym: a floor switch throws a lane's lever
            if (V(0) < 4)
            {
                sWork.gym[0] ^= 1 << V(0);
                CartSwap(V(0));   // (the other cart, as the lane's comes round to its start)
                CartLever(V(0));
            }
            PlaySE(SE_SWITCH);
            break;
        case U3C_FIELD_GIMMICK_CART_LANES_UnlockLaneFromScript:   // (lanes 1, 2: Black's flags 0x74, 0x75 say so)
        case U3C_FIELD_GIMMICK_CART_LANES_TransferSound:
            break;
        case U3C_FIELD_GIMMICK_PLATE_DOORS_StartSwitchEvent:   // BURGH's gym: a floor switch found (PlateDoorsRefresh)
            if (V(0) < 8)
            {
                *WorkPtr(0x4000 + V(0)) = 1;   // (the script sets it as it ends, too)
                sWork.plateSwitches |= 1 << V(0);   // (kept as Black's gimmick save keeps it)
            }
            PlaySE(SE_SWITCH);
            break;
        case U3C_FIELD_GIMMICK_PLATE_DOORS_StartPlateEvent:   // ... its plates lit: a door both of whose are opens
            PlateDoorsRefresh(TRUE);
            break;
        case U3C_FIELD_GIMMICK_PLATE_DOORS_StartScriptedEvent:   // the honey's trainer (PlateDoorsFrame), after this
            if (V(0) < 2)                                        // script ends as Black's event starts
            {
                u32 k;
                sPdScriptedVariant = V(0);
                U3D_ExpPlace(32, sPdEffectPos[V(0)][0], sPdEffectPos[V(0)][1], sPdEffectPos[V(0)][2], 0);
                U3D_ExpHide(32, FALSE);
                U3D_ExpAnim(32, 1, 0, TRUE);
                for (k = 0; k < 1; k++)
                    U3D_ExpTex(32, k, TRUE, 0, TRUE, FALSE);
                sPdScripted = 1;
            }
            break;
        case U3C_FIELD_GIMMICK_PLATE_DOORS_StartDoorEvent:   // (in DRAYDEN's gym, ov046's button: unit, side)
            if (CurrentZone() == DRAGON_GYM_ZONE)
            {
                PlaySE(SE_SWITCH);
                if (V(0) < 3 && V(1) < 2)   // (ButtonEventProc: the button's texture track 0 played from its start)
                {
                    U3D_ExpTex(V(0) * 5 + 3 + V(1), 0, TRUE, 0, TRUE, FALSE);
                    MPNet_Log("u3d vm: DRAYDEN's gym, button %u.%u", (unsigned)V(0), (unsigned)V(1));
                }
            }
            break;
        case U3C_FIELD_GIMMICK_ROTATING_ROOM_StartHopEvent:   // DRAYDEN's gym: the button bounces the player a tile
            if (DragonHop(a[0]))
                t->wait = W_MOVES;
            break;
        case U3C_FIELD_GIMMICK_ROTATING_ROOM_StartLeverEvent:   // ... its lever (unit, side) thrown
            if (DragonLever(V(0), V(1)))
                t->wait = W_EXP_ANIM;
            break;
        case U3C_FIELD_GIMMICK_ROTATING_ROOM_StartRotateEvent:   // ... the dragon turns to the levers' pose
            if (DragonRotate(V(0)))
                t->wait = W_EXP_ANIM;
            break;
        case U3C_PROC_Ov037Finish:   // (ov037 GIMMICK_INIT_ResetRotatingRoomPanels: DRAYDEN's dragons as they start)
            DragonReset();
            break;
        case U3C_PROC_Ov037InitFromVars:   // (NIMBASA's lanes unlocked: Black's flags 0x74 / 0x75 already say it)
            break;
        case U3C_PROC_Ov037ReadEightVars:   // (GIMMICK_INIT_RestorePlateDoorsState: CASTELIA's switches found, from the
        {                                   //  gimmick's save, into the eight works; the doors and plates follow)
            u32 k;
            for (k = 0; k < 8; k++)
                *WorkPtr(a[k]) = (sWork.plateSwitches >> k) & 1;
            PlateDoorsRefresh(FALSE);
            break;
        }
        case U3C_PROC_CheckPartyMonKnowsMove:   // result, move, party slot
            *WorkPtr(a[0]) = V(2) < PARTY_SIZE && PartyMonHasMove(&gPlayerParty[V(2)], V(1));
            break;
        case U3C_PROC_FindPartyMonWithMove:   // result (6: nobody), move: a field move's POKéMON
        {
            u32 i;
            *WorkPtr(a[0]) = PARTY_SIZE;
            for (i = 0; i < PARTY_SIZE; i++)
                if (PartyMonHasMove(&gPlayerParty[i], V(1)))
                {
                    *WorkPtr(a[0]) = i;
                    break;
                }
            break;
        }
        case U3C_PROC_GetMonParamIfListed:   // result, party slot, Black's param (POKE_PARA: its 12 a script may read)
        {
            struct Pokemon *mon = V(1) < PARTY_SIZE ? &gPlayerParty[V(1)] : NULL;
            u32 v = 0;
            if (mon && GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES))
            {
                switch (V(2))
                {
                case 5:    // species (National Dex number)
                    v = SpeciesToNationalPokedexNum(GetMonData(mon, MON_DATA_SPECIES));
                    break;
                case 111:  // gender: 0 male, 1 female, 2 none
                {
                    u8 g = GetMonGender(mon);
                    v = g == MON_MALE ? 0 : g == MON_FEMALE ? 1 : 2;
                    break;
                }
                case 117:  // nicknamed
                {
                    u8 nick[POKEMON_NAME_LENGTH + 1], name[POKEMON_NAME_LENGTH + 1];
                    GetMonData(mon, MON_DATA_NICKNAME, nick);
                    GetSpeciesName(name, GetMonData(mon, MON_DATA_SPECIES));
                    v = StringCompare(nick, name) != 0;
                    break;
                }
                case 154:  // POKéRUS
                    v = GetMonData(mon, MON_DATA_POKERUS);
                    break;
                }
            }
            *WorkPtr(a[0]) = v;
            break;
        }
        case U3C_BufferCategoryEntryName:   // (bank 0xC7: Black's type names)
            StringCopy(Slot(a[0]), U3D_Text(199, V(1)));
            break;
        case U3C_BufferConvertedEntryName:   // slot, a TM / HM (Black's number): the move it teaches (the game's TM's)
        {
            u16 item = Item(V(1));
            if (IS_ITEM_TMHM(item))
                StringCopy(Slot(a[0]), gMoveNames[ItemIdToBattleMoveId(item)]);
            else
                *Slot(a[0]) = EOS;
            break;
        }
        // ---- Black's misc records (MISC save: the teams the Gym Leaders were beaten with, a POKeMON's record, two
        // party POKeMON's names kept, the trainer record's counters)
        case U3C_POKEMON_FLD_StorePartySpeciesRecord:   // gym 0-7: the party's species (national numbers)
            if (V(0) < 8)
            {
                u32 k;
                for (k = 0; k < PARTY_SIZE; k++)
                    sWork.misc.team[V(0)][k] = GetMonData(&gPlayerParty[k], MON_DATA_SANITY_HAS_SPECIES)
                                             && !GetMonData(&gPlayerParty[k], MON_DATA_IS_EGG)
                        ? SpeciesToNationalPokedexNum(GetMonData(&gPlayerParty[k], MON_DATA_SPECIES)) : 0;
            }
            break;
        case U3C_POKEMON_FLD_RegisterRecordSpecies:   // gym, count: the team's species into word slots 0..
        {
            u32 k, n = 0;
            for (k = 0; V(0) < 8 && k < PARTY_SIZE; k++)
                if (sWork.misc.team[V(0)][k])
                    GetSpeciesName(Slot(n++), Species(sWork.misc.team[V(0)][k]));
            *WorkPtr(a[1]) = n;
            break;
        }
        case U3C_POKEMON_FLD_StoreMonRecord:   // party slot: species, form, egg, gender
            if (V(0) < PARTY_SIZE && GetMonData(&gPlayerParty[V(0)], MON_DATA_SANITY_HAS_SPECIES))
            {
                struct Pokemon *mon = &gPlayerParty[V(0)];
                sWork.misc.mon[0] = SpeciesToNationalPokedexNum(GetMonData(mon, MON_DATA_SPECIES));
                sWork.misc.mon[1] = 0;
                sWork.misc.mon[2] = GetMonData(mon, MON_DATA_IS_EGG);
                sWork.misc.mon[3] = GetMonGender(mon) == MON_FEMALE ? 1 : GetMonGender(mon) == MON_GENDERLESS ? 2 : 0;
            }
            break;
        case U3C_POKEMON_FLD_StoreMonName:   // party slot, snapshot 0-1 (MON_SNAPSHOT: a POKeMON traded away, its
                                             // name, level, OT, personality; its new trainer battles with it)
            if (V(0) < PARTY_SIZE && V(1) < 2 && GetMonData(&gPlayerParty[V(0)], MON_DATA_SANITY_HAS_SPECIES))
            {
                struct Pokemon *mon = &gPlayerParty[V(0)];
                sWork.misc.snap[V(1)].national = SpeciesToNationalPokedexNum(GetMonData(mon, MON_DATA_SPECIES));
                sWork.misc.snap[V(1)].level = GetMonData(mon, MON_DATA_LEVEL);
                sWork.misc.snap[V(1)].otId = GetMonData(mon, MON_DATA_OT_ID);
                sWork.misc.snap[V(1)].personality = GetMonData(mon, MON_DATA_PERSONALITY);
                GetMonData(mon, MON_DATA_NICKNAME, sWork.misc.snap[V(1)].nick);
                sWork.misc.snap[V(1)].nick[POKEMON_NAME_LENGTH] = EOS;
                sWork.misc.snap[V(1)].filled = TRUE;
            }
            break;
        case U3C_POKEMON_FLD_RegisterStoredName:   // snapshot, word slot
            if (V(0) < 2)
                StringCopy(Slot(V(1)), sWork.misc.snap[V(0)].filled ? sWork.misc.snap[V(0)].nick : (const u8[]){EOS});
            break;
        case U3C_StartTrainerBattleWithMon:   // trainer, a second, a third, snapshot: the trainer's lead is the
                                              // snapshot's POKeMON (its own species kept: EVENT_BATTLE_..WithMon)
            if (U3D_PrepareBattle(V(0), V(1)))
            {
                if (V(3) < 2 && sWork.misc.snap[V(3)].filled)
                    U3D_BattleLeadAs(sWork.misc.snap[V(3)].level, sWork.misc.snap[V(3)].otId,
                                     sWork.misc.snap[V(3)].personality, sWork.misc.snap[V(3)].nick);
                OpenScreen(t, SCREEN_BATTLE, 0);
            }
            break;
        case U3C_AddTrainerRecordValue:   // counter, amount
            if (V(0) < ARRAY_COUNT(sWork.misc.record))
                sWork.misc.record[V(0)] += V(1);
            break;
        case U3C_GetTrainerRecordValue:   // counter, result (clamped to 0xFFFF)
            *WorkPtr(a[1]) = V(0) < ARRAY_COUNT(sWork.misc.record) ? min(sWork.misc.record[V(0)], 0xFFFF) : 0;
            break;
        // ---- party POKeMON (Black's party values)
        case U3C_PROC_ChangeMonFriendship:   // party slot, amount, 0 set / 1 add / 2 take
            if (V(0) < PARTY_SIZE && !GetMonData(&gPlayerParty[V(0)], MON_DATA_IS_EGG))
            {
                s32 f = GetMonData(&gPlayerParty[V(0)], MON_DATA_FRIENDSHIP);
                f = a[2] == 0 ? (s32)min(V(1), 255) : a[2] == 1 ? min(f + (s32)V(1), 255) : max(f - (s32)V(1), 0);
                SetMonData(&gPlayerParty[V(0)], MON_DATA_FRIENDSHIP, &f);
            }
            break;
        case U3C_PROC_GetMonForm:   // result, party slot: its form, as Black numbers it (mp_forms.c)
            *WorkPtr(a[0]) = V(1) < PARTY_SIZE ? BlackForm(GetMonData(&gPlayerParty[V(1)], MON_DATA_SPECIES),
                                                           MP_Form_Of(&gPlayerParty[V(1)].box)) : 0;
            break;
        case U3C_PROC_ChangeMonForm:   // party slot, form (Black's numbers): DEOXYS at NACRENE's meteorite...
            if (V(0) < PARTY_SIZE)
                MP_Form_Change(&gPlayerParty[V(0)], U3D_Vm_KaiForm(GetMonData(&gPlayerParty[V(0)], MON_DATA_SPECIES), V(1)));
            break;
        case U3C_PROC_SetRotomFormMove:   // party slot, the move's slot (the one forgotten when it knows none of the
            if (V(0) < PARTY_SIZE)        // forms' moves), form (Black's script 10315 picks it from its menu)
                MP_Form_SetRotom(&gPlayerParty[V(0)], V(2), V(1));
            break;
        // ---- moves: Black's scripts on the game's own screens (the summary's move list, the move relearner)
        case U3C_POKEMON_FLD_SelectMonMove:   // decided (1 a move picked), its slot, party slot, the move to learn (0 none)
            sMoveSel.party = V(2);
            sMoveSel.newMove = V(3);
            sMoveSel.posWork = a[1];
            *WorkPtr(a[0]) = 0;
            if (V(2) < PARTY_SIZE)
                OpenScreen(t, SCREEN_MOVE_SELECT, a[0]);
            break;
        case U3C_PROC_SetMonMove:   // party slot, move slot, move (0 forgets it, the rest moving up; past 4 pushes it)
            if (V(0) < PARTY_SIZE)
                SetPartyMonMove(&gPlayerParty[V(0)], V(1), V(2));
            break;
        case U3C_PROC_GetRelearnableMoveCount:   // result (any to relearn), party slot
            *WorkPtr(a[0]) = V(1) < PARTY_SIZE && !GetMonData(&gPlayerParty[V(1)], MON_DATA_IS_EGG)
                && GetNumberOfRelearnableMoves(&gPlayerParty[V(1)]) != 0;
            break;
        case U3C_PROC_CallMoveRelearn:   // result (0 learned, 1 not), party slot: the game's move relearner
            *WorkPtr(a[0]) = 0;
            if (V(1) < PARTY_SIZE)
            {
                gSpecialVar_0x8004 = V(1);
                OpenScreen(t, SCREEN_RELEARN, a[0]);
            }
            break;
        case U3C_POKEMON_FLD_GetTutorAvailability:   // tutor, result (0 some can learn, 1 none of its kinds, 2 none friendly enough)
        {
            u8 mask;
            *WorkPtr(a[1]) = TutorCandidates(V(0), TRUE, &mask);
            break;
        }
        case U3C_POKEMON_FLD_GetMonTutorMove:   // tutor, party slot, result
            *WorkPtr(a[2]) = V(1) < PARTY_SIZE ? TutorMove(&gPlayerParty[V(1)], V(0)) : 0;
            break;
        case U3C_POKEMON_FLD_CheckMonTutorStatus:   // tutor, party slot, result (0 can, 1 not its kind, 2 not friendly, 3 knows it)
        {
            struct Pokemon *mon = V(1) < PARTY_SIZE ? &gPlayerParty[V(1)] : NULL;
            u16 *r = WorkPtr(a[2]);
            if (mon == NULL || GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || !TutorGroup(mon, V(0)))
                *r = 1;
            else if (MonKnowsMove(mon, TutorMove(mon, V(0))))
                *r = 3;
            else if (GetMonData(mon, MON_DATA_FRIENDSHIP) < MAX_FRIENDSHIP)
                *r = 2;
            else
                *r = 0;
            break;
        }
        case U3C_POKEMON_FLD_SelectTutorMon:   // tutor, picked (0 none), party slot: the game's party screen
        {
            u8 mask;
            *WorkPtr(a[1]) = 0;
            if (TutorCandidates(V(0), FALSE, &mask) == 0)
            {
                sPartySlotWork = a[2];
                OpenScreen(t, SCREEN_PARTY, a[1]);
            }
            break;
        }
        case U3C_PROC_CountMonMoves:   // result, party slot
        {
            u32 k, n = 0;
            for (k = 0; V(1) < PARTY_SIZE && k < MAX_MON_MOVES; k++)
                n += GetMonData(&gPlayerParty[V(1)], MON_DATA_MOVE1 + k) != MOVE_NONE;
            *WorkPtr(a[0]) = n;
            break;
        }
        case U3C_PROC_GetMonEvTotal:   // result, party slot
        {
            u32 k, n = 0;
            for (k = 0; V(1) < PARTY_SIZE && k < NUM_STATS; k++)
                n += GetMonData(&gPlayerParty[V(1)], MON_DATA_HP_EV + k);
            *WorkPtr(a[0]) = n;
            break;
        }
        case U3C_PROC_CountPartyMonsOfSpecies:   // species, result
        {
            u32 k, n = 0;
            for (k = 0; V(0) && k < PARTY_SIZE; k++)
                n += !GetMonData(&gPlayerParty[k], MON_DATA_IS_EGG)
                  && GetMonData(&gPlayerParty[k], MON_DATA_SPECIES) == Species(V(0));
            *WorkPtr(a[1]) = n;
            break;
        }
        case U3C_PROC_GetMonPairedParams:   // type 1, type 2, party slot (Black's type numbers)
            *WorkPtr(a[0]) = *WorkPtr(a[1]) = 0;
            if (V(2) < PARTY_SIZE && GetMonData(&gPlayerParty[V(2)], MON_DATA_SANITY_HAS_SPECIES))
            {
                u16 sp = GetMonData(&gPlayerParty[V(2)], MON_DATA_SPECIES);
                *WorkPtr(a[0]) = BlackType(gSpeciesInfo[sp].types[0]);
                *WorkPtr(a[1]) = BlackType(gSpeciesInfo[sp].types[1]);
            }
            break;
        case U3C_PROC_AddMonToPartyDetailed:   // result, species, form, level, gender, ability bit, shiny, item, ball
            *WorkPtr(a[0]) = GiveDetailedMon(V(1), V(3), V(4), V(5), V(6), V(7), V(8));
            break;
        // ---- the daily gifts (ANVILLE's: an item a day by the day of the month, Black's special dates the last)
        case U3C_POKEMON_FLD_GetDailyGift:   // index, result
        {
            static const u16 sGifts[11] = {117, 117, 118, 133, 248, 249, 250, 251, 252, 253, 254};
            *WorkPtr(a[1]) = sGifts[V(0) < 11 ? V(0) : 0];
            break;
        }
        case U3C_POKEMON_FLD_GetDailyGiftIndex:   // result: by the day of the month, Black's special dates 10
        {
            static const u8 sByDay[10] = {9, 1, 3, 5, 4, 2, 6, 8, 0, 7};
            static const u16 sSpecial[5] = {0xA01, 0xA0E, 0xC1E, 0x201, 0x60C};
            struct SiiRtcInfo rtc;
            u32 k, day, month;
            RtcGetInfo(&rtc);
            day = ConvertBcdToBinary(rtc.day), month = ConvertBcdToBinary(rtc.month);
            *WorkPtr(a[0]) = sByDay[day % 10];
            for (k = 0; k < ARRAY_COUNT(sSpecial); k++)
                if (sSpecial[k] == (month << 8 | day))
                    *WorkPtr(a[0]) = 10;
            break;
        }
        // ---- Black's other word slots (arm9 wordset.c: each a system text bank's entry)
        case U3C_BufferGroupEntryName:   // (bank 0xCB)
            StringCopy(Slot(a[0]), U3D_Text(0xCB, V(1)));
            break;
        case U3C_BufferGroupedEntryName:   // (bank 0xE7; past its end: empty)
            StringCopy(Slot(a[0]), U3D_Text(0xE7, V(1)));
            break;
        case U3C_BufferByteEntryNameA:   // (bank 0x2B: entries 0-8)
            if ((u8)V(1) <= 8)
                StringCopy(Slot(a[0]), U3D_Text(0x2B, (u8)V(1)));
            else
                *Slot(a[0]) = EOS;
            break;
        case U3C_BufferByteEntryNameB:   // (bank 0: entries 0-0x91)
            if ((u8)V(1) <= 0x91)
                StringCopy(Slot(a[0]), U3D_Text(0, (u8)V(1)));
            else
                *Slot(a[0]) = EOS;
            break;
        case U3C_BufferSignedEntryName:   // (bank 0xA0: past 0x2F the first)
            StringCopy(Slot(a[0]), U3D_Text(0xA0, (s16)V(1) >= 0x30 || (s16)V(1) < 0 ? 0 : V(1)));
            break;
        case U3C_BufferMapEntryNameA:   // (bank 0xBF)
            StringCopy(Slot(a[0]), U3D_Text(0xBF, (u8)V(1)));
            break;
        case U3C_BufferEasyChatWord:   // (an easy-chat word: arm9 pms_word.c, thirteen banks by the word's number)
        {
            static const u16 sBank[13] = {284, 286, 287, 285, 149, 150, 151, 152, 153, 154, 155, 148, 156};
            static const u16 sSize[13] = {652, 560, 17, 165, 44, 38, 48, 103, 47, 32, 26, 10, 61};
            u32 w = V(1), i, sum = 0;
            *Slot(a[0]) = EOS;
            for (i = 0; w != 0xFFFF && i < 13; i++)
            {
                if ((w & 0x7FF) < sum + sSize[i])
                {
                    StringCopy(Slot(a[0]), U3D_Text(sBank[i], (w & 0x7FF) - sum));
                    break;
                }
                sum += sSize[i];
            }
            break;
        }
        case U3C_BufferShortEntryName:   // (bank 0x37: Black's bag pockets)
            StringCopy(Slot(a[0]), U3D_Text(55, V(1)));
            break;
        case U3C_BufferNumber:
            ConvertIntToDecimalStringN(Slot(a[0]), V(1), STR_CONV_MODE_LEFT_ALIGN, 6);
            break;
        // ---- the game's screens
        case U3C_PROC_CallWordScreen:   // (ov223: the gift box's three POKé BALLS; the ball picked 0-2 in the work)
            OpenScreen(t, SCREEN_STARTER, a[0]);
            break;
        // ---- trainers (their eyes: U3D_Vm_TrainerSight)
        case U3C_GetTrainerHitKind:
            *WorkPtr(a[0]) = sEyeKind;
            break;
        case U3C_GetTrainerEyeTrainerId:   // which hit, result
            *WorkPtr(a[1]) = sEye[V(0) & 1].trainer;
            break;
        case U3C_GetTrainerEyeObjId:
            *WorkPtr(a[1]) = sEye[V(0) & 1].obj;
            break;
        case U3C_CreateTrainerEye:
            StartEye(V(0));
            break;
        case U3C_CallTrainerEyePairEvent:   // (the walks up end; the player turns to the trainer)
            ObjectEventTurn(&gObjectEvents[gPlayerAvatar.objectEventId], GetOppositeDirection(sEye[0].dir));
            t->wait = W_MOVES;
            break;
        case U3C_GetScriptTrainerId:
            *WorkPtr(a[0]) = TrainerOfScript(t->script);
            break;
        case U3C_GetTrainerBattleKind:   // trainer, result (0 single, 1 double, 2 triple, 3 rotation)
        case U3C_ClassifyTrainer:        // trainer, result (1 heals the party after, 2 gives an item, 0 neither)
        case U3C_GetTrainerBattleParam:  // trainer, result (the item it gives after the battle)
        {
            u32 size;
            const u8 *tr = U3D_Vm_Trainer(V(0), &size);
            u16 r = 0;
            if (tr && size >= 20)
                r = op == U3C_GetTrainerBattleKind ? tr[2]
                  : op == U3C_GetTrainerBattleParam ? (tr[18] | tr[19] << 8)
                  : tr[16] ? 1 : (tr[18] | tr[19] << 8) ? 2 : 0;
            *WorkPtr(a[1]) = r;
            break;
        }
        case U3C_GetTrainerMessageIds:   // (the words a trainer says: before, after, after a loss; a pair's own)
        {
            u32 size;
            const u8 *tr = U3D_Vm_Trainer(TrainerOfScript(t->script), &size);
            bool8 doubles = tr && size >= 3 && tr[2] == 1;
            *WorkPtr(a[0]) = !doubles ? 0 : t->script >= 5000 ? 7 : 3;
            *WorkPtr(a[1]) = !doubles ? 2 : t->script >= 5000 ? 9 : 5;
            *WorkPtr(a[2]) = !doubles ? 24 : t->script >= 5000 ? 10 : 6;
            break;
        }
        case U3C_PlayTrainerEncounterBgm:
            PushSong();
            if (EyeSong(V(0)) != MUS_NONE)
                PlayNewMapMusic(EyeSong(V(0)));
            break;
        case U3C_BalloonWinSavedText:   // trainer, kind, person
        {
            u16 m = TrainerWords(V(0), V(1));
            if (m != 0xFFFF)
                MessageInBank(t, 189, m);
            break;
        }
        case U3C_StartTrainerBattle:   // trainer, a second, a third (u3d_battle.c: Black's trainers)
            if (U3D_PrepareBattle(V(0), V(1)))
                OpenScreen(t, SCREEN_BATTLE, 0);
            else
                MPNet_Log("u3d vm: no trainer %u", V(0));
            break;
        case U3C_StartMultiTrainerBattle:   // partner, the first, the second, (a fourth: unused)
            if (U3D_PrepareTagBattle(V(0), V(1), V(2)))
                OpenScreen(t, SCREEN_BATTLE, 0);
            else
                MPNet_Log("u3d vm: no tag battle %u %u %u", V(0), V(1), V(2));
            break;
        // ---- POKéMON
        case U3C_PROC_HealParty:
        case U3C_SCREENEFFECTS_StartRecoveryPartyEvent:   // (the heal with its jingle in Black: healed)
            HealPlayerParty();
            break;
        case U3C_PROC_GetPartyCount:   // (result, kind: 0 all, else those that can battle)
        {
            u32 i, n = 0;
            for (i = 0; i < PARTY_SIZE; i++)
            {
                u16 sp = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
                if (sp != SPECIES_NONE && (V(1) == 0 || (sp != SPECIES_EGG && GetMonData(&gPlayerParty[i], MON_DATA_HP))))
                    n++;
            }
            *WorkPtr(a[0]) = n;
            break;
        }
        case U3C_PROC_AddMonToPartySimple:   // result, species, form, level
            *WorkPtr(a[0]) = ScriptGiveMon(Species(V(1)), V(3), ITEM_NONE, 0, 0, 0) != MON_CANT_GIVE;
            break;
        case U3C_BufferPartyMonNickname:
            if (V(1) < PARTY_SIZE)
            {
                GetMonData(&gPlayerParty[V(1)], MON_DATA_NICKNAME, Slot(a[0]));
                StringGet_Nickname(Slot(a[0]));
            }
            break;
        case U3C_BufferPartyMonSpeciesName:
            if (V(1) < PARTY_SIZE)
                GetSpeciesName(Slot(a[0]), GetMonData(&gPlayerParty[V(1)], MON_DATA_SPECIES));
            break;
        case U3C_PlayPokeVoice:   // species, form
            PlayCry_Normal(Species(V(0)), 0);
            break;
        case U3C_WORK_CmdGetPlayerSex:
            *WorkPtr(a[0]) = gSaveBlock2Ptr->playerGender;
            break;
        case U3C_WORK_CmdCheckLastBattleResultA:
            *WorkPtr(a[0]) = U3D_BattleResult() != 2;   // (Black: 1 won, 0 the player lost)
            break;
        case U3C_WORK_CmdGetRandom:
            *WorkPtr(a[0]) = V(1) ? Random() % V(1) : 0;
            break;
        case U3C_PROC_AddMoney:
            AddMoney(&gSaveBlock1Ptr->money, V(0));
            break;
        // ---- items (Black's numbers: Item())
        case U3C_AddItem:   // item, count, result (1: it fit)
            if (Item(V(0)) == ITEM_NONE)
                MPNet_Log("u3d vm: Black's item %u isn't in the game yet", V(0));
            *WorkPtr(a[2]) = Item(V(0)) == ITEM_NONE || AddBagItem(Item(V(0)), V(1));
            break;
        case U3C_RemoveItem:
            *WorkPtr(a[2]) = Item(V(0)) != ITEM_NONE && RemoveBagItem(Item(V(0)), V(1));
            break;
        case U3C_CanAddItem:
            *WorkPtr(a[2]) = Item(V(0)) == ITEM_NONE || CheckBagHasSpace(Item(V(0)), V(1));
            break;
        case U3C_HasItem:
            *WorkPtr(a[2]) = Item(V(0)) != ITEM_NONE && CheckBagHasItem(Item(V(0)), V(1));
            break;
        case U3C_GetItemPartyUse:   // (Black's item parameter 15: 4 a POKe BALL; CASTELIA's ZORUA girl asks for one)
            *WorkPtr(a[1]) = Item(V(0)) != ITEM_NONE && GetPocketByItemId(Item(V(0))) == POCKET_POKE_BALLS ? 4 : 0;
            break;
        case U3C_POKEMON_FLD_GetRandomBagPocketValue:   // a TM or HM in the BAG at random: its move (0: none)
        {
            u16 have[256], n = 0, i;
            for (i = 0; i < gBagPockets[TMHM_POCKET].capacity && n < ARRAY_COUNT(have); i++)
            {
                u16 item = gBagPockets[TMHM_POCKET].itemSlots[i].itemId;
                if (item != ITEM_NONE && CheckBagHasItem(item, 1))
                    have[n++] = item;
            }
            *WorkPtr(a[0]) = n ? ItemIdToBattleMoveId(have[Random() % n]) : 0;
            break;
        }
        case U3C_POKEMON_FLD_GetMonTypeForTm:   // a party POKeMON's HIDDEN POWER type (0xFFFF: it can't learn TM10)
        {
            struct Pokemon *mon = V(1) < PARTY_SIZE ? &gPlayerParty[V(1)] : NULL;
            u16 out = 0xFFFF;
            if (mon && GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG)
             && CanMonLearnTMHM(mon, TMHM_INDEX(ITEM_TM10)))
            {
                // (the battle's own formula: battle_script_commands.c Cmd_hiddenpowercalc)
                u8 bits = (GetMonData(mon, MON_DATA_HP_IV) & 1) | (GetMonData(mon, MON_DATA_ATK_IV) & 1) << 1
                        | (GetMonData(mon, MON_DATA_DEF_IV) & 1) << 2 | (GetMonData(mon, MON_DATA_SPEED_IV) & 1) << 3
                        | (GetMonData(mon, MON_DATA_SPATK_IV) & 1) << 4 | (GetMonData(mon, MON_DATA_SPDEF_IV) & 1) << 5;
                u8 type = ((NUMBER_OF_MON_TYPES - 3) * bits) / 63 + 1;
                if (type >= TYPE_MYSTERY)
                    type++;
                out = BlackType(type);
            }
            *WorkPtr(a[0]) = out;
            break;
        }
        case U3C_StartCountOtherOtIdsTask:   // (scrcmd_otid.c: the trainers other than the player whose POKeMON the
        {                                    // party and the boxes hold, EGGS left out, 50 at most)
            static u32 sIds[50];
            u32 n = 0, i, j, me = T1_READ_32(gSaveBlock2Ptr->playerTrainerId);
            struct PokemonStorage *st = MP_PersonalStorage();
            for (i = 0; i < PARTY_SIZE + TOTAL_BOXES_COUNT * IN_BOX_COUNT && n < ARRAY_COUNT(sIds); i++)
            {
                const struct BoxPokemon *b = i < PARTY_SIZE ? &gPlayerParty[i].box
                                                            : &st->boxes[(i - PARTY_SIZE) / IN_BOX_COUNT][(i - PARTY_SIZE) % IN_BOX_COUNT];
                u32 ot;
                if (!GetBoxMonData(b, MON_DATA_SANITY_HAS_SPECIES) || GetBoxMonData(b, MON_DATA_IS_EGG))
                    continue;
                if ((ot = GetBoxMonData(b, MON_DATA_OT_ID)) == me)
                    continue;
                for (j = 0; j < n && sIds[j] != ot; j++)
                    ;
                if (j == n)
                    sIds[n++] = ot;
            }
            *WorkPtr(a[0]) = n;
            MPNet_Log("u3d vm: %u other trainers' POKeMON", n);
            break;
        }
        case U3C_GetNpcGreetingMessage:   // fixed, result: Black's greeting by the clock (scrcmd_misc.c)
        {
            // gGreetingCounterTable (ov021 0x021DD774): two records each and their digits (-1: none)
            static const s8 sRecA[29] = {-1, 30, 1, 11, 21, 23, 7, 5, 10, 8, 4, 3, 9, 24, 28, 29, 31, 72, 73, 74, 75, 78, 79,
                                         89, 90, 81, 83, 85, 87};
            static const s8 sRecB[29] = {-1, -1, -1, -1, 22, -1, -1, 6, -1, 80, 71};
            static const u8 sDigA[29] = {0, 6, 9, 9, 9, 9, 9, 9, 9, 6, 9, 9, 6, 9, 6, 6, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
            static const u8 sDigB[29] = {0, 0, 0, 0, 9, 0, 0, 9, 0, 4, 4};
            u16 msg, base, i;
            s32 party;
            RtcCalcLocalTime();
            StringCopy(Slot(0), gSaveBlock2Ptr->playerName);
            for (party = 0; party < PARTY_SIZE; party++)
                if (GetMonData(&gPlayerParty[party], MON_DATA_SPECIES) && !GetMonData(&gPlayerParty[party], MON_DATA_IS_EGG))
                {
                    GetSpeciesName(Slot(3), GetMonData(&gPlayerParty[party], MON_DATA_SPECIES));
                    break;
                }
            if (gLocalTime.minutes < 10)
            {
                u32 ca = 0, cb = 0;
                base = 0;
                msg = Random() % 29;
                if (sRecA[msg] >= 0)
                {
                    ca = min(sWork.misc.record[(u8)sRecA[msg]], 999999999);
                    ConvertIntToDecimalStringN(Slot(1), ca, STR_CONV_MODE_LEFT_ALIGN, sDigA[msg]);
                }
                if (msg < 11 && sRecB[msg] >= 0)
                {
                    cb = min(sWork.misc.record[(u8)sRecB[msg]], 999999999);
                    ConvertIntToDecimalStringN(Slot(2), cb, STR_CONV_MODE_LEFT_ALIGN, sDigB[msg]);
                }
                if (msg < 11 && ((sRecA[msg] >= 0 && sRecB[msg] >= 0 && (ca == 0 || cb == 0)) || (sRecB[msg] < 0 && ca == 0)))
                    msg = 0;
                if (msg >= 11 || gSaveBlock2Ptr->playerGender == FEMALE)
                    msg += 11;
            }
            else if (gLocalTime.minutes < 35)
                base = 0x28, msg = Random() % 0x32;
            else
                base = 0x5A, msg = Random() % 0x50;
            msg += base;
            if (msg >= 102 && msg <= 109 && !FlagGet(FLAG_UNOVA_BADGE01 + msg - 102))   // (gGreetingMiscMessageIds: a badge's)
                msg = 0x5A + Random() % 12;
            for (i = 135; V(0) == 0 && i <= 149; i++)   // (gGreetingRandomMessageIds)
                if (msg == i)
                {
                    msg = 0x6E + Random() % 20;
                    break;
                }
            *WorkPtr(a[1]) = msg;
            break;
        }
        case U3C_GetBirthdayMessageIds:   // (the DS owner's birthday isn't kept here: Black's other days' lines)
        {
            // gBirthdayTextIdTable (ov021 0x021DD6D0)
            static const u16 sText[30] = {1697, 1494, 1689, 1534, 1671, 1443, 1572, 1699, 1562, 1717, 1547, 1538, 1696,
                                          1405, 585, 522, 599, 587, 595, 520, 511, 531, 563, 528, 555, 594, 518, 581, 618, 709};
            u16 i = Random() % (V(0) ? 0x1E : 0x16);
            *WorkPtr(a[1]) = i + 0x16;
            *WorkPtr(a[2]) = i + 0x35;
            *WorkPtr(a[3]) = sText[i];
            break;
        }
        case U3C_FLD_BATTLE_RegisterAllowedSpecies:   // regulation, count: the regulation's barred POKeMON the player
        {                                             // has caught, named in the word slots (scrcmd_fld_battle.c)
            u16 n = 0, nat;
            bool8 U3D_Subway_Banned(u16 species);
            for (nat = 1; nat <= 649 && n < UNOVA_STR_VARS; nat++)
            {
                u16 sp = NationalPokedexNumToSpecies(nat);
                if (U3D_Subway_Banned(sp) && GetSetPokedexFlag(nat, FLAG_GET_CAUGHT))
                    GetSpeciesName(Slot(n++), sp);
            }
            *WorkPtr(a[1]) = n;
            break;
        }
        case U3C_HasPlayerLanguage:   // (the player's registered country, Black's GeoNet: none registered here)
            *WorkPtr(a[0]) = 0;
            break;
        case U3C_NETWORK_CmdLoadNpcTable:   // (the table is the pack's, always at hand)
        case U3C_NETWORK_CmdFreeNpcTable:
            break;
        case U3C_NETWORK_CmdSetNpcEventFromTable:   // (key A, the person, key B)
            NpcFromRow(V(1), NpcTableRow(V(0), V(2)), FALSE, FALSE);
            break;
        case U3C_NETWORK_CmdSetNpcEventPairFromTable:   // (key A, a person and their partner, key B: a double battle)
        {
            u32 row = NpcTableRow(V(0), V(3));
            NpcFromRow(V(1), row, FALSE, FALSE);
            NpcFromRow(V(2), row, TRUE, FALSE);
            break;
        }
        case U3C_NETWORK_CmdSetNpcTripleFromTable:   // (three people, three rows: their looks and their trainers)
            NpcFromRow(V(0), V(3), FALSE, TRUE);
            NpcFromRow(V(1), V(4), FALSE, TRUE);
            NpcFromRow(V(2), V(5), FALSE, TRUE);
            break;
        case U3C_NETWORK_CmdApplyTableEventKeys:   // (every table trainer can be battled again: a new day's lineup)
        {
            u32 n, i;
            const u8 *t = NpcTable(&n);
            for (i = 0; t && i < n; i++)
                FlagTo(1420 + Le16(t + 8 * i + 6), FALSE);
            break;
        }
        case U3C_NETWORK_PickSixDistinctRandom:   // (six different rows of the table's last 52, from 0x37)
        {
            u16 picked[6], pool = 0x34;
            u32 d, c, k;
            for (d = 0; d < 6; d++, pool--)
            {
                s32 skip = Random() % pool;
                for (c = 0; c < 0x34; c++)
                {
                    for (k = 0; k < d && picked[k] != c; k++)
                        ;
                    if (k == d && --skip < 0)
                    {
                        picked[d] = c;
                        break;
                    }
                }
            }
            for (d = 0; d < 6; d++)
                *WorkPtr(a[d]) = picked[d] + 0x37;
            break;
        }
        case U3C_NETWORK_CmdGetAlwaysNet:   // (Black's "always connect" setting: off; the CO-OP menu is Kai's)
            *WorkPtr(a[0]) = 0;
            break;
        case U3C_PROC_FLD_StoreEffectEncountItemId:   // (the item a dust cloud or a bird's shadow left: u3d_phenom.c)
        {
            u16 Unova_Phenom_Item(void);
            *WorkPtr(a[0]) = Unova_Phenom_Item();
            break;
        }
        case U3C_GetItemPocket:   // (Black's pocket: 0 ITEMS, 1 MEDICINE, 2 TMs & HMs, 3 BERRIES, 4 KEY ITEMS)
            *WorkPtr(a[1]) = V(0) < ARRAY_COUNT(sUnovaItemPockets) ? sUnovaItemPockets[V(0)] : 0;
            break;
        case U3C_GetItemQuantity:
            *WorkPtr(a[1]) = Item(V(0)) == ITEM_NONE ? 0 : CountTotalItemQuantityInBag(Item(V(0)));
            break;
        case U3C_BufferItemName:
        case U3C_BufferItemNameAlt:
        case U3C_BufferItemNameWithCount:
            if (Item(V(1)) != ITEM_NONE)
                CopyItemName(Item(V(1)), Slot(a[0]));
            else
                StringCopy(Slot(a[0]), U3D_Text(54, V(1)));   // (Black's own name for an item the game lacks)
            break;
        // ---- warps (Black's zone and tile: the map holding it, u3d.c)
        case U3C_WarpToPosition:
        case U3C_WarpToPositionPlain:
        case U3C_WarpToPositionFixedExit:
        {
            u8 group, num;
            s16 x, y;
            if (U3D_MapOfZoneTile(a[0], a[1], a[2], &group, &num, &x, &y))
            {
                MPNet_Log("u3d vm: warp to zone %u (%d.%d at %d,%d)", a[0], group, num, x, y);
                sHolding = FALSE;   // (the map's load starts the field's scripts afresh; this one goes on there)
                t->wait = W_WARP;
                SetWarpDestination(group, num, WARP_ID_NONE, x, y);
                DoWarp();
                return;
            }
            MPNet_Log("u3d vm: no map for zone %u at %u,%u", a[0], a[1], a[2]);
            break;
        }
        case U3C_ShowPokeIconWindow:   // national number, form, EGG, gender: the POKeMON in a window (the game's own
        {                               // picture box) until A or B
            u16 sp = V(2) ? SPECIES_EGG : Species(V(0));
            if (sp != SPECIES_NONE && ScriptMenu_ShowPokemonPic(sp, 10, 3))
                t->wait = W_PIC_KEY;
            break;
        }
        // ---- Black's elevators (SCRCMD_POKEMON_FLD_*WarpTable / WarpEntry): a table in the script of up to 8 floors
        // {message, zone, x, y, z}, ended by 0xFFFF; the menu lists the floors but the one the player is on
        case U3C_POKEMON_FLD_SetWarpTable:
            sWarpTable = t->pc + (s32)a[0] <= t->size ? t->code + t->pc + (s32)a[0] : NULL;
            sWarpTableEnd = t->code + t->size;
            break;
        case U3C_POKEMON_FLD_AddWarpMenuItems:
        {
            u32 i;
            for (i = 0; i < 8 && sWarpTable && sWarpTable + 10 * i + 10 <= sWarpTableEnd
                        && Le16(sWarpTable + 10 * i) != 0xFFFF; i++)
                if (Le16(sWarpTable + 10 * i + 2) != CurrentZone())
                    MenuAdd(Le16(sWarpTable + 10 * i), i);
            break;
        }
        case U3C_POKEMON_FLD_WarpToEntry:
        {
            u32 i = V(0), k;
            u8 group, num;
            s16 x, y;
            for (k = 0; k < 8 && sWarpTable && sWarpTable + 10 * k + 10 <= sWarpTableEnd
                        && Le16(sWarpTable + 10 * k) != 0xFFFF; k++)
                ;
            if (i < k && U3D_MapOfZoneTile(Le16(sWarpTable + 10 * i + 2), Le16(sWarpTable + 10 * i + 4),
                                           Le16(sWarpTable + 10 * i + 8), &group, &num, &x, &y))
            {
                MPNet_Log("u3d vm: elevator to zone %u", Le16(sWarpTable + 10 * i + 2));
                sHolding = FALSE;
                t->wait = W_WARP;
                SetWarpDestination(group, num, WARP_ID_NONE, x, y);
                DoWarp();
                return;
            }
            break;
        }
        case U3C_SYMBOL_MAP_ScrCmdStartPmsInput:   // kind, word, word, result (1 entered): kinds 0-2 the player's battle
        {                                       // phrases (the game's EASY CHAT: introduction, won, lost), 3 "No. 1"
            if (V(0) <= 3)                      // (asked, not kept: Kai has no such phrase); 4 one of Black's own words
            {                                   // (the PEP QUIZ: typed, then found in Black's word lists, WordScreen);
                sEasyChatKind = V(0), sEasyChatOut[0] = a[1], sEasyChatOut[1] = a[2];   // 5 MR. LOCK's password
                OpenScreen(t, SCREEN_EASY_CHAT, a[3]);                                   // (an event's: closed)
            }
            else if (V(0) == 4)
            {
                WordTitle(t->bank);
                sEasyChatOut[0] = a[1];
                OpenScreen(t, SCREEN_WORD, a[3]);
            }
            else
                *WorkPtr(a[3]) = 0;
            break;
        }
        case U3C_CheckPlayerFormModel:   // (the player in a form of its own model: never here)
            *WorkPtr(a[0]) = 0;
            break;
        case U3C_PROC_CallModeScreen:   // 0 / 1: the UNOVA / National POKeDEX's diploma (the game's DIPLOMA screen); 2
            if (V(0) == 2)              // the BATTLE SUBWAY's map (ov224: Black's picture, u3d_subwaymap.c)
            {
                bool8 Unova_SubwayMap_Open(void);
                if (Unova_SubwayMap_Open())
                {
                    t->wait = W_SUBWAY_MAP;
                    if (sHolding)
                        ScriptContext_Enable();
                }
            }
            else if (V(0) <= 1)
            {
                extern s8 gMpDiplomaNational;
                gMpDiplomaNational = V(0);
                OpenScreen(t, SCREEN_DIPLOMA, 0);
            }
            break;
        case U3C_WORK_CmdSetBgEventGridPos:   // check spot, x, y, z: moved for as long as the player is in the zone
        {
            u32 k;
            for (k = 0; k < BG_MOVES; k++)
                if (!sBgMove[k].set || sBgMove[k].event == V(0))
                {
                    sBgMove[k].event = V(0), sBgMove[k].x = (s16)V(1), sBgMove[k].z = (s16)V(3), sBgMove[k].set = TRUE;
                    break;
                }
            break;
        }
        case U3C_WORK_CmdSetSpecialLocationHere:   // where the game goes on after a link room: here
            sSpecialLoc.group = gSaveBlock1Ptr->location.mapGroup, sSpecialLoc.num = gSaveBlock1Ptr->location.mapNum;
            sSpecialLoc.x = gSaveBlock1Ptr->pos.x, sSpecialLoc.y = gSaveBlock1Ptr->pos.y, sSpecialLoc.set = TRUE;
            break;
        case U3C_WORK_CmdSetSpecialLocation:   // zone, direction, x, y, z: where the game goes on after its clear
        {
            u8 group, num;
            s16 x, y;
            if (U3D_MapOfZoneTile(V(0), V(2), V(4), &group, &num, &x, &y))
                sSpecialLoc.group = group, sSpecialLoc.num = num, sSpecialLoc.x = x, sSpecialLoc.y = y, sSpecialLoc.set = TRUE;
            break;
        }
        case U3C_PROC_FLD_SetGameClearEventAndEnd:   // mode (0 the story's end: the credits; 1 the Champion: the HALL OF
            sClearMode = V(0);                        // FAME and the credits); the script ends here (VM_End)
            OpenScreen(t, SCREEN_CLEAR, 0);
            return;
        case U3C_PROC_FLD_SpawnMovePokemonIfInactive:   // slot: Black's TORNADUS sets off roaming (White's THUNDURUS too)
            RoamSpawn(1, 641);   // (Black's slot 1: TORNADUS, SpawnEntry)
            RoamSpawn(0, 642);   // (slot 0: THUNDURUS, White's)
            break;
        // ---- the in-game trades (ov021 fld_trade.c: Black's records, a/1/6/5 in the scripts pack)
        case U3C_POKEMON_FLD_CheckTradeMon:   // result, trade, slot: 0 fine, 1 an EGG, 2 not the species, 3 not the gender
        {
            const u8 *r = TradeRecord(V(1));
            u16 slot = V(2);
            u16 res = 0;
            if (r == NULL || slot >= PARTY_SIZE)
                res = 2;
            else if (GetMonData(&gPlayerParty[slot], MON_DATA_IS_EGG))
                res = 1;
            else if (GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES) != Species(Get32At(r, 0x5C)))
                res = 2;
            else if (Get32At(r, 0x60) != 2 && GenderOf(&gPlayerParty[slot]) != Get32At(r, 0x60))
                res = 3;
            *WorkPtr(a[0]) = res;
            break;
        }
        case U3C_POKEMON_FLD_StartTradeEvent:   // trade, slot: the game's trade scene
            sTradeId = V(0), sTradeSlot = V(1);
            OpenScreen(t, SCREEN_TRADE, 0);
            break;
        case U3C_PROC_AddEggToParty:   // result, species, form: an EGG from someone (ROUTE 18's LARVESTA)
            *WorkPtr(a[0]) = CalculatePlayerPartyCount() < PARTY_SIZE;
            if (CalculatePlayerPartyCount() < PARTY_SIZE)
            {
                ScriptGiveEgg(Species(V(1)));
                MPNet_Log("u3d vm: an EGG of national %u", V(1));
            }
            break;
        case U3C_PROC_CheckMonIsPlayerOwned:   // result, slot: the player's own (OT ID and name)
        {
            u8 name[PLAYER_NAME_LENGTH + 1];
            u16 slot = V(1);
            bool8 own = FALSE;
            if (slot < PARTY_SIZE && GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES)
             && GetMonData(&gPlayerParty[slot], MON_DATA_OT_ID) == GetPlayerIDAsU32())
            {
                GetMonData(&gPlayerParty[slot], MON_DATA_OT_NAME, name);
                name[PLAYER_NAME_LENGTH] = EOS;
                own = StringCompareN(name, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH) == 0;
            }
            *WorkPtr(a[0]) = own;
            break;
        }
        case U3C_FIELD_G3D_MAP_ScrCmdReadSaveByte:   // roamer slot, result: its state (Black's: 0 none, 1 roaming, 3 caught)
            *WorkPtr(a[1]) = V(0) < 2 ? sWork.roam[V(0)].state : 0;
            break;
        case U3C_PROC_GetMonTripleParams:   // three results, slot (params 0x92-0x94: none kept here)
            *WorkPtr(a[0]) = *WorkPtr(a[1]) = *WorkPtr(a[2]) = 0;
            break;
        case U3C_PROC_FLD_StoreRandomUniqueEntry:   // result: one of the zone's wild POKeMON (its grass, at random)
        {
            const u8 *row = U3D_Vm_WildRow(CurrentZone(), U3D_Season());
            u16 n = 0, pick[12];
            u32 i;
            for (i = 0; row != NULL && i < 12; i++)
            {
                u16 nat = (row[8 + 4 * i] | row[9 + 4 * i] << 8) & 0x7FF;
                if (nat)
                    pick[n++] = nat;
            }
            *WorkPtr(a[0]) = n ? pick[Random() % n] : 0;
            break;
        }
        case U3C_IssSwitchOn:   // (VILLAGE BRIDGE's players: a part of the song each)
            sIss |= 1 << (V(0) & 15);
            break;
        case U3C_IssSwitchOff:
            sIss &= ~(1 << (V(0) & 15));
            break;
        case U3C_IsIssSwitchOn:
            *WorkPtr(a[0]) = (sIss >> (V(1) & 15)) & 1;
            break;
        case U3C_GetPlayerLocationValue:   // (the steps message counter: none kept)
            *WorkPtr(a[0]) = 0;
            break;
        case U3C_EVENT_MON_CmdFindEventMonInParty:   // national, form, slot, found: Black asks for a distributed POKeMON
        {                                         // (ZORUA's CELEBI, ZOROARK's ENTEI / RAIKOU / SUICUNE); Kai gives
            u16 sp = Species(V(0));                // Black's distributions itself (the LIBERTY PASS, event items:
            u32 i;                                 // CheckBeaconTrainerKey), so a real one in the party will do
            *WorkPtr(a[2]) = 0;
            *WorkPtr(a[3]) = 0;
            for (i = 0; i < PARTY_SIZE && sp != SPECIES_NONE; i++)
                if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == sp && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
                {
                    *WorkPtr(a[2]) = i;
                    *WorkPtr(a[3]) = 1;
                    break;
                }
            break;
        }
        case U3C_PROC_SetMonParamChecked:
        case U3C_StartPlayerPosOffsetTween:
        case U3C_StartSubProcOv057:
        case U3C_GIMMICK_OBJ_ELBOARD_CmdStartCameraEvent:   // (UNDELLA GATE's news board)
            break;
        case U3C_PROC_FLD_CallOv056Event:   // the ELITE FOUR rooms' entrances (ov056 field_fk_demo.c): the room's
        {                                   // platform carries the player up to gFkDemoEndTile[variant - 1]
            static const u8 sFkEnd[4][2] = {{15, 13}, {15, 16}, {15, 18}, {14, 16}};
            u8 group, num;
            s16 x, y;
            if (V(0) >= 1 && V(0) <= 4
             && U3D_MapOfZoneTile(CurrentZone(), sFkEnd[V(0) - 1][0], sFkEnd[V(0) - 1][1], &group, &num, &x, &y))
            {
                sHolding = FALSE;
                t->wait = W_WARP;   // (the script goes on in the room once the player is up)
                SetWarpDestination(group, num, WARP_ID_NONE, x, y);
                DoWarp();
                return;
            }
            break;
        }
        case U3C_PALPARK_CallFallEvent:   // the LEAGUE's platform sinks (ov010 namein_setup.c NAMEIN_SETUP_RunFallEvent):
        {                                 // the descent and the landing in zone 0x8A (League*)
            void LeagueFallStart(void);
            LeagueFallStart();
            t->wait = W_LEAGUE;
            break;
        }
        case U3C_EVENT_MON_CmdClearEventMonFlag:
        case U3C_EVENT_MON_CmdStartSpPokeGimmickTask:
        case U3C_EVENT_MON_CmdMoveSpPokeGimmick:
        case U3C_EVENT_MON_CmdStartSpPokeGimmickStepA:
        case U3C_EVENT_MON_CmdStartSpPokeGimmickStepB:
            break;
        // ---- BLACK CITY's residents (ov020 field_wfbc_data_local.c: the one talked to, sWfbcSlot)
        case U3C_FIELD_WFBC_DATA_LOCAL_ScrCmdGetWfbcValue:   // query, result
            *WorkPtr(a[1]) = WfbcValue(V(0));
            break;
        case U3C_FIELD_WFBC_DATA_LOCAL_ScrCmdGetFacingPersonMessageIndex:   // result, kind (1-4): its own words
            *WorkPtr(a[0]) = (u16)(V(1) - 1) + sWork.wfbc.npc[sWfbcSlot % 10] * 4 + 1;
            break;
        case U3C_FIELD_WFBC_DATA_LOCAL_ScrCmdMarkFacingPerson:   // talked to today (its mood up at the next day)
            sWork.wfbc.flags[sWfbcSlot % 10] |= 2;
            break;
        case U3C_FIELD_WFBC_DATA_LOCAL_ScrCmdCheckAndMarkFacingPerson:   // battled: no more today
            if (sWork.wfbc.flags[sWfbcSlot % 10] & 1)
            {
                sWork.wfbc.flags[sWfbcSlot % 10] &= ~1;
                sWork.wfbc.counter++;
            }
            break;
        case U3C_FIELD_WFBC_DATA_LOCAL_ScrCmdAddPoints:
            if (sWork.wfbc.value < 10000)
                sWork.wfbc.value += 10;
            break;
        case U3C_FIELD_WFBC_DATA_LOCAL_ScrCmdCallPersonTalkEvent:   // (another player's city through the ENTRALINK: CO-OP's)
            break;
        case U3C_WORK_CmdSetConnectGridPos:   // connection, x, y, z (grid): moved while the player is in the zone
            ConnMove(V(0), (s16)V(1), (s16)V(3));
            break;
        case U3C_WORK_CmdSetWarpDestination:   // Black's warp point (a POKéMON CENTER, home): where a whiteout returns
            SetWhiteoutPoint(V(0));
            break;
        case U3C_WORK_CmdEndScriptWhiteOutA:   // (a loss the story doesn't go on from)
            sHolding = FALSE;
            t->alive = FALSE;
            SetMainCallback2(CB2_WhiteOut);
            return;
        // ---- the game's state
        case U3C_WORK_CmdGetRomVersion:
            *WorkPtr(a[0]) = 21;   // (Black: PM_VERSION_ID; White's branches aren't taken)
            break;
        case U3C_WORK_CmdCheckBadge:
            *WorkPtr(a[0]) = V(1) < 8 && FlagGet(FLAG_UNOVA_BADGE01 + V(1));
            break;
        case U3C_WORK_CmdSetBadge:
            if (V(0) < 8)
                FlagSet(FLAG_UNOVA_BADGE01 + V(0));
            break;
        case U3C_WORK_CmdGetBadgeCount:
        {
            u32 i, n = 0;
            for (i = 0; i < 8; i++)
                n += FlagGet(FLAG_UNOVA_BADGE01 + i);
            *WorkPtr(a[0]) = n;
            break;
        }
        case U3C_WORK_CmdGetZukanNationalFlag:
            *WorkPtr(a[0]) = FlagGet(FLAG_SYS_NATIONAL_DEX);
            break;
        case U3C_WORK_CmdSetZukanObtained:   // (JUNIPER's POKéDEX: the user's decision, the NATIONAL DEX)
            FlagSet(FLAG_SYS_POKEDEX_GET);
            EnableNationalPokedex();
            break;
        // ---- Black's map replacements (ov010 d_02170FB0: flag, work, the work's value that means "replaced"): the
        // works the scripts read, the matrices u3d.c draws (src/unova/u3d_replace.h)
        case U3C_WORK_CmdSetMapReplaceFlag:   // flag, on, now
        case U3C_WORK_CmdCheckMapReplaceFlag:   // flag, result
        {
            static const u16 sReplace[10][2] = {{0x4031, 4640}, {0x4032, 548}, {0x4033, 518}, {0x4034, 4617},
                                                {0x4035, 790}, {0x4036, 4647}, {0x4037, 2096}, {0x4038, 1032},
                                                {0x4039, 288}, {0x403A, 4136}};
            u16 f = V(0);
            if (op == U3C_WORK_CmdSetMapReplaceFlag)
            {
                if (f < 10)
                    *WorkPtr(sReplace[f][0]) = V(1) ? sReplace[f][1] : 0;
                if (V(2))   // (now: the map under the player too)
                    U3D_RefreshBlocks();
            }
            else
                *WorkPtr(a[1]) = f < 10 && *WorkPtr(sReplace[f][0]) == sReplace[f][1];
            break;
        }
        // ---- a trainer beaten: Black's event flag 1420 + the trainer's number (EVENT_TRAINER_*DefeatedFlag)
        case U3C_WORK_CmdSetTrainerDefeatedFlag:
            FlagTo(1420 + V(0), TRUE);
            break;
        case U3C_WORK_CmdClearTrainerDefeatedFlag:
            FlagTo(1420 + V(0), FALSE);
            break;
        case U3C_WORK_CmdCheckTrainerDefeatedFlag:
            *WorkPtr(a[1]) = FlagOn(1420 + V(0));
            break;
        case U3C_WORK_CmdSetFixedEventFlag:
            FlagTo(0x963, TRUE);
            break;
        case U3C_WORK_CmdGetTimeZone:   // Black's time of day: 0 morning, 1 day, 2 evening, 3 night, 4 late night
            *WorkPtr(a[0]) = TimeZoneNow();
            break;
        // zone 389's time effect (ov065 field_gimmick_time_effect.c; overlays share addresses, so these commands'
        // names are another overlay's): its pending flag set, the effect started, the script waiting for its end
        // people flown along Black's paths (scrcmd_misc.c MMDL_ICA: archive 0xCB, u3d_icas.h), from where the player
        // stood: the roamer's fly-by, ABUNDANT SHRINE, ROUTE 7
        case U3C_CreateMmdlIcaProc:   // person
            sIca.local = LocalId(V(0)), sIca.playing = FALSE;
            U3D_PlayerWorldPos(sIca.base);
            break;
        case U3C_StartMmdlIcaAnime:   // path
            if (V(0) < ARRAY_COUNT(sIcaPaths))
            {
                sIca.path = V(0), sIca.frame = 0, sIca.playing = TRUE;
                IcaFrame();
                MPNet_Log("u3d vm: a person flown along path %u", V(0));
            }
            break;
        case U3C_WaitMmdlIcaAnime:
            if (sIca.playing)
                t->wait = W_ICA;
            break;
        case U3C_EndMmdlIcaProc:
            if (sIca.playing)
                IcaLand();
            break;
        // a person risen to a height, circled round, dropped in (scrcmd_mmdl.c Rise / Orbit, scrcmd_fldmmdl.c DropIn)
        case U3C_MMDL_StartRise:   // person, height (half tiles: Black's height << 15), field frames
            if (MotionStart(LocalId(V(0)), MOT_RISE, V(1) * 8.0f, V(2)))
                t->wait = W_MOTION;
            break;
        case U3C_MMDL_StartOrbit:   // person: a spiral out to three tiles (110 frames), round (80), back in (80)
            if (MotionStart(LocalId(V(0)), MOT_ORBIT, 0, 270))
                t->wait = W_MOTION;
            break;
        case U3C_StartMmdlDropIn:   // person, x, z, direction: put there and dropped in from 250 units over 40 frames
        {
            u8 id = ObjEventId(LocalId(V(0)));
            if (id < OBJECT_EVENTS_COUNT)
            {
                MoveObjectEventToMapCoords(&gObjectEvents[id], MapX(V(1)) + MAP_OFFSET, MapZ(V(2)) + MAP_OFFSET);
                ObjectEventTurn(&gObjectEvents[id], BlackToDir(V(3)));
                MotionStart(gObjectEvents[id].localId, MOT_DROP, 250, 40);
            }
            break;
        }
        case U3C_POKEMON_FLD_StartOv55SubProc:   // CASTELIA's crowd (ov055's sub process)
            CrowdStart();
            break;
        case U3C_StartArcPlayerEffect:   // (the LIGHT STONE's glow: the script waits for it)
            if (CurrentZone() == ZONE_THRONE)
                ArcGlowStart(), t->wait = W_ARC_GLOW;
            break;
        case U3C_StartArcEffect:   // person, tile x, y (half tiles up), z, arch (half tiles), frames: waits for it
            if (CurrentZone() == ZONE_THRONE)
                ArcFlyStart(LocalId(V(0)), (s16)V(1), (s16)V(2), (s16)V(3), (s16)V(4), V(5)), t->wait = W_ARC_FLY;
            break;
        case U3C_ShowArcModelAt:
            if (CurrentZone() == ZONE_THRONE)
                ArcShowAt((s16)V(0), (s16)V(1), (s16)V(2));
            break;
        case U3C_WaitArcEffectPeak:   // (its first animation past frame 5)
            if (sArc.shown)
                t->wait = W_ARC_PEAK;
            break;
        case U3C_WaitArcEffectEnd:
            if (sArc.shown)
                t->wait = W_ARC_END;
            break;
        case U3C_TRCARD_SYS_ScrCmdSetRecordFlag36:
            sWork.white |= TIMEFX_PENDING;
            break;
        case U3C_TRCARD_SYS_ScrCmdStartFieldProc:
            TimeFxStart();
            break;
        case U3C_TRCARD_SYS_ScrCmdRunScriptTask:
            if (sTimeFx.playing)
                t->wait = W_TIMEFX;
            break;
        case U3C_WORK_CmdGetTrainerCardStars:   // (Black's card's stars: the game's TRAINER CARD keeps its own)
            *WorkPtr(a[0]) = 0;
            break;
        case U3C_WORK_CmdGetMonthAndDay:   // month, day (the PC's clock)
        {
            struct SiiRtcInfo r;
            Platform_GetDateTime(&r);
            *WorkPtr(a[0]) = ConvertBcdToBinary(r.month);
            *WorkPtr(a[1]) = ConvertBcdToBinary(r.day);
            break;
        }
        case U3C_WORK_CmdGetBirthday:   // (the DS's owner's birthday: none here, so the nurse never says it)
            *WorkPtr(a[0]) = 0;
            *WorkPtr(a[1]) = 0;
            break;
        case U3C_POKEMON_FLD_CheckPartyFlagged:   // a POKeMON of the party has POKeRUS
            *WorkPtr(a[0]) = CheckPartyPokerus(gPlayerParty, 0x3F) != 0;
            break;
        case U3C_POKEMON_FLD_HandleMysteryGift:   // (Black's Mystery Gift deliveryman: none to give)
            *WorkPtr(a[1]) = 0;
            break;
        case U3C_StartPlayerFormChangeAndWait:   // (Black turns the player to the counter and back)
            break;
        case U3C_SCREENEFFECTS_StartRecoveryBallsEvent:   // the healing machine: its tune, the song after
            PushSong();
            PlayNewMapMusic(MUS_UNOVA_ME_ASA);
            t->waitFrames = 60 * 6;
            t->wait = W_SONG_THEN_POP;
            break;
        case U3C_WORK_CmdGetWeek:
            RtcCalcLocalTime();
            *WorkPtr(a[0]) = gLocalTime.days % 7;
            break;
        case U3C_PROC_CheckMonHpNotFullOrEgg:   // (result 1: full HP or an EGG)
        {
            u16 i = V(1);
            *WorkPtr(a[0]) = i < PARTY_SIZE && GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES)
                          && (GetMonData(&gPlayerParty[i], MON_DATA_HP) == GetMonData(&gPlayerParty[i], MON_DATA_MAX_HP)
                              || GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG));
            break;
        }
        case U3C_StartWaitSeStopped:
            t->wait = W_SE;
            break;
        case U3C_StartWaitPokeVoice:   // (a POKéMON's cry played to its end)
            t->wait = W_CRY;
            break;
        case U3C_FadeInField:
        case U3C_SCREENEFFECTS_StartFieldFadeIn:   // (the field faded back in, the season's look kept)
            BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
            t->wait = W_FADE;
            break;
        case U3C_ReportSave:   // (result: 1 saved)
            *WorkPtr(a[0]) = TrySavingData(SAVE_NORMAL) == SAVE_STATUS_OK;
            break;
        case U3C_POKEMON_FLD_StartSelectEvent:   // (nickname the POKéMON? Black asks; the game's own naming comes later)
            *WorkPtr(a[0]) = 0;
            break;
        case U3C_WORK_CmdEnableNationalDex:
            EnableNationalPokedex();
            break;
        case U3C_WaitMsgKey:   // (once the message has printed)
            t->wait = W_MESSAGE_KEY;
            break;
        // ---- Black's small side windows (a crowd's murmurs) shown together in the message box, the script going on;
        // its WaitMsgKey / close end them
        case U3C_AddSubWinMsg:   // message, x, y, window
        {
            static u8 sLine[160];
            u16 bank = t->bank;
            const u8 *text = U3D_Text(bank, a[0]);
            StringExpandPlaceholders(sLine, text);
            if (sSubCount && sSubCount % 2)
                StringAppend(sSubText, (const u8[]){CHAR_NEWLINE, EOS});
            else if (sSubCount)
                StringAppend(sSubText, (const u8[]){CHAR_PROMPT_SCROLL, EOS});
            if (StringLength(sSubText) + StringLength(sLine) < sizeof(sSubText) - 4)
                StringAppend(sSubText, sLine);
            sSubCount++;
            if (!IsFieldMessageBoxHidden())
                HideFieldMessageBox();
            ShowFieldMessage(sSubText);
            break;
        }
        case U3C_DeleteSubWinMsg:
            sSubCount = 0;
            sSubText[0] = EOS;
            break;
        // ---- people
        case U3C_TurnTargetMmdlToPlayer:
        {
            struct ObjectEvent *o = ObjectOf(t->target);
            if (t->target != 0xFFFF && o)
                ObjectEventTurn(o, GetOppositeDirection(gObjectEvents[gPlayerAvatar.objectEventId].facingDirection));
            break;
        }
        case U3C_StartMmdlAnime:
            StartMoves(V(0), t->code + t->pc + (s32)a[1], t->code + t->size);
            break;
        case U3C_WaitAnimeTasks:
            t->wait = W_MOVES;
            break;
        case U3C_AddMmdl:   // x, z (Black's tiles), direction, object id, Black's object code, move code
            Gone(LocalId(V(3)), FALSE);
            AddPerson(LocalId(V(3)), V(4), V(0), V(1), V(2));
            break;
        case U3C_AddPartyFollowerMmdl:   // a person of the zone's event data brought out (its flag cleared before)
        {
            u8 local = LocalId(V(0));
            Gone(local, FALSE);
            if (ObjEventId(local) >= OBJECT_EVENTS_COUNT)
                TrySpawnObjectEvent(local, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
            break;
        }
        case U3C_SetMmdlZoneDeleteOff:      // (a person who stays as the player crosses zones: ObjEventId finds it)
        case U3C_SCREENEFFECTS_ShowPlaceName:   // (the game shows a map's name as it is entered)
            break;
        case U3C_SetProcAction:   // a script to run once this one has ended, by its id in the zone the player is then in
            sPendingScript = a[0];
            break;
        case U3C_StartSwapNpcEvent:   // person out, person in: the second set out where the first stood (CASTELIA's lost
        {                              // boy turning into ZORUA; Black's puff of smoke not drawn, its sound the script's)
            u8 from = LocalId(V(0)), to = LocalId(V(1)), id = ObjEventId(from);
            s16 x = -1, y = -1;
            if (id < OBJECT_EVENTS_COUNT && !gObjectEvents[id].isPlayer)
            {
                x = gObjectEvents[id].currentCoords.x - MAP_OFFSET, y = gObjectEvents[id].currentCoords.y - MAP_OFFSET;
                RemoveObjectEventByLocalIdAndMap(gObjectEvents[id].localId, gObjectEvents[id].mapNum, gObjectEvents[id].mapGroup);
            }
            ForgetPerson(from);
            Gone(from, TRUE);
            Gone(to, FALSE);
            if (x >= 0)
                SetObjEventTemplateCoords(to, x, y);
            id = ObjEventId(to);
            if (id >= OBJECT_EVENTS_COUNT)
                TrySpawnObjectEvent(to, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
            else if (x >= 0)
                MoveObjectEventToMapCoords(&gObjectEvents[id], x + MAP_OFFSET, y + MAP_OFFSET);
            break;
        }
        case U3C_StartMmdlSpinRise:   // a person spinning up and away (the SHADOW TRIAD's way): 150 units in 24 field
            if (MotionStart(LocalId(V(0)), MOT_SPINRISE, 0, 24))   // frames, spinning, then gone (Black's script
                break;                                            // deletes them after)
        case U3C_DeleteMmdl:
        {
            u8 id = ObjEventId(LocalId(V(0)));
            ForgetPerson(LocalId(V(0)));
            Gone(LocalId(V(0)), TRUE);
            if (id < OBJECT_EVENTS_COUNT && !gObjectEvents[id].isPlayer)
                RemoveObjectEventByLocalIdAndMap(gObjectEvents[id].localId, gObjectEvents[id].mapNum, gObjectEvents[id].mapGroup);
        }
            break;
        case U3C_WORK_CmdSetNpcGridPos:   // person (its event id), direction, x, y, z: its place while in the zone
        {
            u8 local = LocalId(V(0));
            u32 i;
            for (i = 0; i < sNpcAtCount && sNpcAt[i].local != local; i++)
                ;
            if (i < NPC_AT_MAX)
            {
                sNpcAt[i].local = local, sNpcAt[i].dir = V(1), sNpcAt[i].x = V(2), sNpcAt[i].z = V(4);
                if (i == sNpcAtCount)
                    sNpcAtCount++;
                SetObjEventTemplateCoords(local, MapX(V(2)), MapZ(V(4)));
            }
            break;
        }
        case U3C_SetPlayerRailLocation:   // line, along, across: the player put on the zone's rail (FIELD_RAIL_WORK_
        {                                 // SetLineLocation), as the tile its place is on (tools/unova/rails.py tile_of)
            static const u16 sRailPlaces[][6] = {{66, 2, 0, 1, 16, 27}, {66, 6, 0, 0, 16, 30}};   // (Black's only ones)
            u32 i;
            for (i = 0; i < ARRAY_COUNT(sRailPlaces); i++)
                if (sRailPlaces[i][0] == CurrentZone() && sRailPlaces[i][1] == V(0) && sRailPlaces[i][2] == V(1)
                 && sRailPlaces[i][3] == V(2))
                {
                    MoveObjectEventToMapCoords(&gObjectEvents[gPlayerAvatar.objectEventId],
                                               MapX(sRailPlaces[i][4]) + MAP_OFFSET, MapZ(sRailPlaces[i][5]) + MAP_OFFSET);
                    MPNet_Log("u3d vm: the player set on the rail at %u,%u", sRailPlaces[i][4], sRailPlaces[i][5]);
                }
            break;
        }
        case U3C_StartMmdlMoveToGrid:   // person, x, y, z: there at once (Black glides it)
        {
            struct ObjectEvent *o = ObjectOf(V(0));
            if (o && !o->isPlayer)
                TryMoveObjectEventToMapCoords(o->localId, o->mapNum, o->mapGroup, MapX(V(1)), MapZ(V(3)));
            else if (o)
                MoveObjectEventToMapCoords(o, MapX(V(1)) + MAP_OFFSET, MapZ(V(3)) + MAP_OFFSET);
            break;
        }
        case U3C_InitMmdlGridPosition:   // person, x, y, z, direction
        {
            u16 obj = V(0);
            struct ObjectEvent *o = ObjectOf(obj);
            s32 x = MapX(V(1)), z = MapZ(V(3));
            if (obj != OBJ_PLAYER)
                SetObjEventTemplateCoords(LocalId(obj), x, z);
            if (o)
            {
                TryMoveObjectEventToMapCoords(o->localId, o->mapNum, o->mapGroup, x, z);
                ObjectEventTurn(o, BlackToDir(V(4)));
            }
            break;
        }
        case U3C_GetPlayerDir:
            *WorkPtr(a[0]) = DirToBlack(gObjectEvents[gPlayerAvatar.objectEventId].facingDirection);
            break;
        case U3C_GetPlayerGridPos:
        {
            s32 i = U3D_MapIndex();
            *WorkPtr(a[0]) = gSaveBlock1Ptr->pos.x + (i < 0 ? 0 : 32 * U3D_MapCellX(i));
            *WorkPtr(a[1]) = gSaveBlock1Ptr->pos.y + (i < 0 ? 0 : 32 * U3D_MapCellZ(i));
            break;
        }
        // ---- music
        case U3C_FadeOutPlayBgm:
        case U3C_FadeOutPlayBgmWithFade:
        case U3C_FadeOutPlayFixedBgm:
            if (SongOf(a[0]) != MUS_NONE)
                FadeOutAndPlayNewMapMusic(SongOf(a[0]), 4);
            break;
        case U3C_PushAndPlayBgm:
        case U3C_PushBgmAndPlayWait:   // (the same, the field's song held under it: a scene's own song)
            PushSong();
            if (SongOf(a[0]) != MUS_NONE)
                PlayNewMapMusic(SongOf(a[0]));
            break;
        case U3C_StartWaitBgmThenPop:   // (the fanfare to its end, then the song it played over)
            t->waitFrames = 60 * 20;
            t->wait = W_SONG_THEN_POP;
            break;
        case U3C_PushBgm:
            PushSong();
            break;
        case U3C_PopBgm:
            PopSong();
            break;
        case U3C_ResumeZoneBgm:
        case U3C_ResumeZoneBgmWithFade:
        case U3C_ResumeBgmSlots:
            sSongDepth = 0;
            if (GetCurrentMapMusic() != GetCurrLocationDefaultMusic())
                FadeOutAndPlayNewMapMusic(GetCurrLocationDefaultMusic(), 4);
            break;
        // ---- sound
        case U3C_PlaySe:
            if (V(0) == 547)   // (the talk sound)
                PlaySE(SE_SELECT);
            break;
        // ---- screen (a fade waits for the one before it to end: the game ignores one started over another)
        case U3C_SCREENEFFECTS_WaitScriptEffect:
            t->wait = W_FADE;
            break;
        case U3C_PROC_FLD_StartFieldMapFadeIn:   // (after a screen Black opens over the field)
            if (gPaletteFade.active)
            {
                t->pc = here, t->wait = W_FADE;
                break;
            }
            if (gPaletteFade.y != 0)
                BeginNormalPaletteFade(PALETTES_ALL, 2, 16, 0, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_FadeOutSlow:
        case U3C_SCREENEFFECTS_FadeOutSlowAltScreens:
            if (gPaletteFade.active)
            {
                t->pc = here, t->wait = W_FADE;
                break;
            }
            BeginNormalPaletteFade(PALETTES_ALL, 2, 0, 16, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_FadeOutFast:
        case U3C_SCREENEFFECTS_FadeOutFastAltScreens:
            if (gPaletteFade.active)
            {
                t->pc = here, t->wait = W_FADE;
                break;
            }
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_FadeInSlow:
        case U3C_SCREENEFFECTS_FadeInSlowAgain:
        case U3C_SCREENEFFECTS_FadeInSlowAltScreens:
            if (gPaletteFade.active)
            {
                t->pc = here, t->wait = W_FADE;
                break;
            }
            BeginNormalPaletteFade(PALETTES_ALL, 2, 16, 0, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_FadeInFast:
        case U3C_SCREENEFFECTS_FadeInFastAltScreens:
            if (gPaletteFade.active)
            {
                t->pc = here, t->wait = W_FADE;
                break;
            }
            BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_WaitBrightness:
            t->wait = W_FADE;
            break;
        case U3C_SCREENEFFECTS_StartBrightnessChange:   // screens, from, to (Black's brightness, 16 = gone), speed
        {
            s16 from = (s16)a[1], to = (s16)a[2];
            if (gPaletteFade.active)
            {
                t->pc = here, t->wait = W_FADE;
                break;
            }
            BeginNormalPaletteFade(PALETTES_ALL, a[3] > 1 ? a[3] - 1 : 0, from < 0 ? -from : from, to < 0 ? -to : to,
                                  (a[0] & 0xC) ? RGB_WHITE : RGB_BLACK);   // (GFL's FADE: screens 1 / 2 black, 4 / 8 white)
            break;
        }
        case U3C_PROC_CallFieldOpenEvent:    // (Black reopens its field after a screen: the game's own does that)
        case U3C_PROC_CallFieldCloseEvent:
        case U3C_RESEARCH_CmdSetSurveyAnswer:   // (the Xtransceiver survey: Wi-Fi's)
            break;
        case U3C_NAMEIN_SETUP_ScrCmdSetFieldSaveFlag:   // (a field save flag for a scene's restart: Kai saves its own way)
            break;
        case U3C_WORK_CmdGetSaveStatus:   // has a save, a new game, a big save to write: a save there, not new, small
            *WorkPtr(a[0]) = 1;
            *WorkPtr(a[1]) = 0;
            *WorkPtr(a[2]) = 0;
            break;
        case U3C_SetTimeIcon:          // (the saving clock: the game shows its own)
        case U3C_ChangeSubscreenMode:  // (the lower screen's mode: one screen here)
        case U3C_ResetSubscreenMode:
            break;
        case U3C_WORK_CmdGetScriptId:   // (the zone the script was started in: the GEAR STATION's lines read it)
            *WorkPtr(a[0]) = CurrentZone();
            break;
        case U3C_FLD_BATTLE_CheckPartyRegulation:   // regulation, result: 0 a team can be made, 3 not (the BATTLE SUBWAY's
            *WorkPtr(a[1]) = U3D_Subway_PartyUsable(V(0)) ? 0 : 3;   // 20-22; others: no BATTLE BOX to look at)
            break;
        // ---- the BATTLE SUBWAY (u3d_subway.c: Black's bsubway_scr.c); the field's people and places here
        case U3C_BSUBWAY_SCR_CmdClearWorkPtr:
            break;
        case U3C_BSUBWAY_SCR_CmdCreateWork:   // resume, line
            U3D_Subway_Bind(&sWork.subway);
            U3D_Subway_CreateWork(a[0], V(1));
            break;
        case U3C_BSUBWAY_SCR_CmdDeleteWork:
            U3D_Subway_DeleteWork();
            break;
        case U3C_BSUBWAY_SCR_CmdTool:   // command, two values, the result's work
        {
            u16 *dst = WorkPtr(a[3]), result = *dst, bank = 0, msg = 0;
            u8 act;
            U3D_Subway_Bind(&sWork.subway);
            act = U3D_Subway_Tool(a[0], V(1), V(2), &result, &bank, &msg);
            *dst = result;
            if (act == U3D_SUBWAY_VM)
                SubwayField(a[0], V(1), V(2), dst);
            else if (act == U3D_SUBWAY_PARTY)
                OpenScreen(t, SCREEN_SUBWAY_PARTY, 0);
            else if (act == U3D_SUBWAY_BATTLE)
                OpenScreen(t, SCREEN_SUBWAY_BATTLE, 0);
            else if (act == U3D_SUBWAY_RESET)
                OpenScreen(t, SCREEN_SUBWAY_RESET, 0);
            else if (act == U3D_SUBWAY_MESSAGE)   // (a trainer's greeting in a balloon: closed with A or B)
            {
                MessageInBank(t, bank, msg);
                t->wait = W_MESSAGE_KEY;
            }
            else if (act == U3D_SUBWAY_YESNO)
            {
                DisplayYesNoMenuDefaultYes();
                t->result = dst;
                t->wait = W_YESNO;
            }
            if (sScreen == SCREEN_SUBWAY_PARTY || sScreen == SCREEN_SUBWAY_BATTLE || sScreen == SCREEN_SUBWAY_RESET)
                return;
            break;
        }
        case U3C_SHOP_StartShop:   // shop, (result): Black's lists in the game's own POKeMART
            if (V(0) == 254 || V(0) == 253)   // (the GEAR STATION's BP exchanges: u3d_bpshop.c)
            {
                sBpShop = V(0) == 253;
                OpenScreen(t, SCREEN_BPSHOP, 0);
                return;
            }
            if (V(0) >= 247 && V(0) <= 251 ? BlackCityMart(V(0)) : MartList(V(0)))
            {
                *WorkPtr(a[1]) = 1;   // (Black's clerk asks again: "Is there anything else?")
                OpenScreen(t, SCREEN_MART, 0);
                return;
            }
            MPNet_Log("u3d vm: shop %u not open yet", V(0));
            break;
        case U3C_PROC_CallBagScreen:   // mode (0: selling to a clerk), result, item
            if (V(0) == 0)
            {
                *WorkPtr(a[1]) = 1;
                *WorkPtr(a[2]) = 0;
                OpenScreen(t, SCREEN_SELL, 0);
                return;
            }
            MPNet_Log("u3d vm: bag screen %u not open yet", V(0));
            break;
        case U3C_CheckBeaconTrainerKey:   // (whether an event item came from a real distribution: Kai gives them itself)
            *WorkPtr(a[1]) = 1;
            break;
        default:
            if (!(sLogged[op / 16] & (1 << (op % 16))))
            {
                sLogged[op / 16] |= 1 << (op % 16);
                MPNet_Log("u3d vm: %03X not played yet (read past)", op);
                printf("[u3d] command %03X not played yet\n", op);
            }
            break;
        }
    }
}

static void Wait(struct Thread *t)
{
    switch (t->wait)
    {
    case W_FRAMES:
        if (t->waitFrames == 0 || --t->waitFrames == 0)
            t->wait = W_NONE;
        break;
    case W_MESSAGE:
        if (!IsTextPrinterActive(0))
            t->wait = W_NONE;
        break;
    case W_MESSAGE_KEY:
        if (!IsTextPrinterActive(0))
            t->wait = W_KEY, t->waitFrames = 0;
        break;
    case W_KEY:
        if (JOY_NEW(A_BUTTON | B_BUTTON) || (gTextFlags.autoScroll && ++t->waitFrames >= 40))
            t->wait = W_NONE, t->waitFrames = 0;
        break;
    case W_PIC_KEY:   // (ShowPokeIconWindow: the picture until A or B, SCRCMD_POKE_ICON_Task)
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            ScriptMenu_HidePokemonPic();
            t->wait = W_NONE;
        }
        break;
    case W_MOVES:
        if (MovesDone())
            t->wait = W_NONE;
        break;
    case W_FADE:
        if (!gPaletteFade.active)
            t->wait = W_NONE;
        break;
    case W_SE:
        if (!IsSEPlaying())
            t->wait = W_NONE;
        break;
    case W_CRY:
        if (!IsCryPlaying())
            t->wait = W_NONE;
        break;
    case W_CAMERA:
        if (!U3D_CameraMoving())
            t->wait = W_NONE;
        break;
    case W_SUBWAY_MAP:   // (the BATTLE SUBWAY's map, till it closes)
        if (!Unova_Screen_IsActive_())
            t->wait = W_NONE;
        break;
    case W_MUSICAL:   // (MUSICAL_ShowPartyMonMusical: the dressing room, till it closes: u3d_musical.c)
    {
        bool8 Unova_Musical_Busy(void);
        if (!Unova_Musical_Busy())
            t->wait = W_NONE;
        break;
    }
    case W_LIFT_RIDE:   // (CLAY's mine: Black's lift event, till the lift arrives)
        if (sMineRide.lift < 0 && sPlat.model < 0 && sLiftTex.model < 0)
            t->wait = W_NONE;
        break;
    case W_MOTION:   // (a rise or a circling over: their events' ends)
        if (!MotionWaiting())
            t->wait = W_NONE;
        break;
    case W_ICA:   // (the person's flight over: MMDL_ICA_IsDone)
        if (!sIca.playing)
            t->wait = W_NONE;
        break;
    case W_ARC_GLOW:   // (the LIGHT STONE's events: PlayerEffectEvent, ArcEffectEvent, IsArcPeakReached, IsArcAnimeStopped)
        if (!sArc.glow)
            t->wait = W_NONE;
        break;
    case W_ARC_FLY:
        if (!sArc.flying)
            t->wait = W_NONE;
        break;
    case W_ARC_PEAK:
        if (!sArc.shown || U3D_ExpFrame(1) >= 5)
            t->wait = W_NONE;
        break;
    case W_ARC_END:
        if (!sArc.shown)
            t->wait = W_NONE;
        break;
    case W_LEAGUE:   // (the LEAGUE's descent: its warp once the screen is black, then the landing to its end)
        if (LeagueWarpNow())
        {
            sHolding = FALSE;
            t->wait = W_WARP;
            return;
        }
        if (!LeagueBusy())
            t->wait = W_NONE;
        break;
    case W_TIMEFX:   // (zone 389's time effect over: WaitEffectEventMain)
        if (!sTimeFx.playing)
            t->wait = W_NONE;
        break;
    case W_SHAKE:   // (the camera's shake over)
        if (!U3D_CameraShaking())
            t->wait = W_NONE;
        break;
    case W_BMODEL:   // (a build model's node, still: CheckBuildModelNodeAnimDone)
        if (!U3D_BModelNodeBusy(sBModelWait))
            t->wait = W_NONE;
        break;
    case W_BMODEL_DOOR:   // (the door opened: FIELD_DOOR_ANIME_OpenTask waits for its sound)
        if (!U3D_DoorBusy())
            t->wait = W_NONE;
        break;
    case W_EXP_ANIM:   // (Black's gimmick events wait for their model's animation: FLD_EXP_OBJ_IsAnimeStopped)
        if (!U3D_ExpPlaying(sExpWait))
            t->wait = W_NONE;
        break;
    case W_BGM_STOPPED:   // (Black's StartWaitBgmStopped: till the song ends, a jingle's say)
        if (IsBGMStopped() || t->waitFrames == 0 || --t->waitFrames == 0)
            t->wait = W_NONE;
        break;
    case W_SONG_THEN_POP:
        if (IsBGMStopped() || t->waitFrames == 0 || --t->waitFrames == 0)
        {
            PopSong();
            t->wait = W_NONE;
        }
        break;
    case W_MENU:
        if (MenuInput())
            t->wait = W_NONE;
        break;
    case W_YESNO:
    {
        s8 r = Menu_ProcessInputNoWrapClearOnChoose();
        if (r != MENU_NOTHING_CHOSEN)
        {
            if (t->result)
                *t->result = r == 0 ? 0 : 1;   // (Black: 0 yes, 1 no)
            t->wait = W_NONE;
        }
        break;
    }
    }
}

static void Task_Vm(u8 taskId)
{
    if (sThread.wait == W_SCREEN)
        return;
    struct Thread *t = &sThread;
    if (!t->alive)
    {
        DestroyTask(taskId);
        return;
    }
    if (t->wait == W_WARP)
        return;
    if (t->wait != W_NONE)
        Wait(t);
    if (t->wait == W_NONE)
        Step(t);
}

// Start Black's script SCRIPT_ID of the player's zone (TARGET: the person talked to, Black's id; 0xFFFF none)
static bool8 Run(u16 scriptId, u16 target, bool8 hold)
{
    struct Thread *t = &sThread;
    u16 zone = CurrentZone();
    if (t->alive || zone == 0xFFFF || !Locate(zone, scriptId, &t->code, &t->size, &t->pc, &t->bank))
        return FALSE;
    t->sp = 0, t->cmp = 0, t->wait = W_NONE, t->alive = TRUE, t->zone = zone, t->target = target, t->result = NULL;
    t->depth = 0;
    t->script = scriptId;
    memset(sLocal, 0, sizeof(sLocal));
    sLocal[0x11] = target;   // (0x8011: the person talked to, SCRIPT_WORK_SetTargetObj)
    memset(sMoving, 0, sizeof(sMoving));
    sHolding = hold;
    MPNet_Log("u3d vm: zone %u script %u", zone, scriptId);
    CreateTask(Task_Vm, 80);
    return TRUE;
}

// A script of Black's own commands put together here (White's ZEKROM), with the throne room's words
static bool8 RunCode(const u8 *code, u32 size, u16 target)
{
    struct Thread *t = &sThread;
    u16 zone = CurrentZone();
    const u8 *c;
    u32 sz, pc;
    if (t->alive || zone == 0xFFFF || !Locate(ZONE_THRONE, 1, &c, &sz, &pc, &t->bank))
        return FALSE;
    t->code = code, t->size = size, t->pc = 0;
    t->sp = 0, t->cmp = 0, t->wait = W_NONE, t->alive = TRUE, t->zone = zone, t->target = target, t->result = NULL;
    t->depth = 0;
    t->script = 0;
    memset(sLocal, 0, sizeof(sLocal));
    sLocal[0x11] = target;
    memset(sMoving, 0, sizeof(sMoving));
    sHolding = TRUE;
    MPNet_Log("u3d vm: zone %u, a script of Kai's (%u bytes)", zone, size);
    CreateTask(Task_Vm, 80);
    return TRUE;
}

// Each field frame aboard (PLEASURE_BOAT_AckEvent, then _Update): the chime of a period's end, the trip's end
static void BoatFrame(void)
{
    extern const u8 Unova_EventScript_Scene[];
    if (!sBoat.alive || sBoat.locked || sThread.alive || ArePlayerFieldControlsLocked())
        return;
    if (sBoat.eventPending)
    {
        PlaySE(SE_DING_DONG);   // (Black's 0x6DD)
        sBoat.periodCount++;
        sBoat.eventPending = FALSE;
        MPNet_Log("u3d vm: the ROYAL UNOVA's chime %u", sBoat.periodCount);
    }
    sBoat.ticks++;
    if (sBoat.ticks >= 5400 && sBoat.periodCount >= 4)
    {
        if (Run(1, 0xFFFF, TRUE))   // (SCRIPT_SYS_CreateScriptEventSimple(gs, 1, 0, 0x8015): the zone's script 1)
        {
            MPNet_Log("u3d vm: the ROYAL UNOVA comes back to port");
            ScriptContext_SetupScript(Unova_EventScript_Scene);
        }
    }
    else if (sBoat.ticks % 1350 == 0)
        sBoat.eventPending = TRUE;
}

// ------------------------------------------------------------------------------------------------ the field's hooks
// (a special that ends at once still lets its waitstate stop the script first: the enable comes a frame later)
static void Task_EnableNext(u8 taskId)
{
    ScriptContext_Enable();
    DestroyTask(taskId);
}

static void NothingToRun(void)
{
    CreateTask(Task_EnableNext, 80);
}

// special U3D_VmTalk: the person the player talked to (gSpecialVar_LastTalked) runs their script (Black's event data)
static bool8 RunCode(const u8 *code, u32 size, u16 target);
static bool8 sZekromBattle;
static const u8 *WfbcRecord(u8 npc);

void U3D_VmTalk(void)
{
    u16 local = gSpecialVar_LastTalked;
    if (U3D_Vm_CrowdLocal(local))
    {
        if (!CrowdTalk(local))
            NothingToRun();
        return;
    }
    const u8 *p = local ? Person(CurrentZone(), local - 1) : NULL;
    if (CurrentZone() == ZONE_BLACK_CITY && local >= 1 && local <= 10 && sWork.wfbc.valid && sWork.wfbc.npc[local - 1] != 0xFF)
    {
        // (a resident of BLACK CITY: its record's script there, Black's WfbcNpcRecord eventIds[1])
        const u8 *r = WfbcRecord(sWork.wfbc.npc[local - 1]);
        sWfbcSlot = local - 1;
        if (r != NULL && Run(Le16(r + 0x1E), 0x80 + sWork.wfbc.npc[local - 1], TRUE))
            return;
    }
    if (local < 256 && WhiteZekromHere(local) && p != NULL)
    {
        // Black's own commands and words (zone 278's bank: White's lines for ZEKROM): its cry, then the battle at
        // Lv 50, as Black's RESHIRAM (script 7)
        static const u8 sCode[] = {
            0xA6, 0x00, 0x47, 0x05,                                // PlaySe 547
            0xAB, 0x00, 0x84, 0x02, 0x00, 0x00,                    // PlayPokeVoice ZEKROM
            0x4A, 0x00, 0x18, 0x00, 0x00,                          // MSGWIN_CmdPlainWinMsgFrame1 18 ("Bazzazzazzash!")
            0xAC, 0x00, 0x4B, 0x00, 0x39, 0x00,                    // StartWaitPokeVoice, WaitMsgKey, ClosePlainWin
            0x78, 0x01, 0x84, 0x02, 0x32, 0x00, 0x00, 0x00,        // PROC_FLD_StartScriptedWildBattle ZEKROM 50
            0x02, 0x00,                                            // Halt
        };
        if (RunCode(sCode, sizeof(sCode), Le16(p)))
        {
            sZekromBattle = TRUE;
            return;
        }
    }
    if (p != NULL && PersonScript(local, p) == 2000 && TrySign())
        return;   // (a person with nothing to say over a sign, Black's blockers: the sign answers, CLAY's gym's levers)
    if (p == NULL || !Run(PersonScript(local, p), Le16(p), TRUE))
        NothingToRun();
}

// special U3D_VmSign: the sign / spot the player faces
// a sign's height (Black's y, 16 units a tile's height) on the player's floor (the ground: below the platforms)
static bool8 SignOnFloor(s32 y)
{
    float floor = U3D_Floor();
    return isnan(floor) ? !MineFloorHeight((s16)(y / 16)) : fabsf(y - floor) < 64;
}

void U3D_VmSign(void)
{
    if (!TrySign())
        NothingToRun();
}

// the sign in front of the player, run (FALSE none)
static bool8 TrySign(void)
{
    u32 size, i;
    const u8 *e = Events(CurrentZone(), &size);
    s16 x, y;
    s32 m = U3D_MapIndex();
    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    x -= MAP_OFFSET, y -= MAP_OFFSET;
    for (i = 0; e && size >= 8 && i < e[4]; i++)
    {
        const u8 *b = e + 8 + EV_BG * i;
        s32 bx = (s32)(Le16(b + 8) | Le16(b + 10) << 16), bz = (s32)(Le16(b + 12) | Le16(b + 14) << 16);
        u32 k;
        for (k = 0; k < BG_MOVES; k++)
            if (sBgMove[k].set && sBgMove[k].event == i)
                bx = sBgMove[k].x, bz = sBgMove[k].z;
        if (m >= 0)
            bx -= 32 * U3D_MapCellX(m), bz -= 32 * U3D_MapCellZ(m);
        if (bx == x && bz == y && U3D_HexActive() && !SignOnFloor((s32)(Le16(b + 16) | Le16(b + 18) << 16)))
            continue;   // (a sign of another floor at this spot: CLAY's gym's levers, one each height)
        if (bx == x && bz == y && getenv("MOEMON_U3D_TRACE"))
            printf("[u3d] sign %d,%d: bg %u script %u\n", x, y, i, Le16(b));
        if (bx == x && bz == y && Run(Le16(b), 0xFFFF, TRUE))
            return TRUE;
    }
    return FALSE;
}

// RELIC CASTLE's quicksand (a sunken 5 x 5 trigger) fires where the sand is deep, its middle
// (Black's sand drags the player there; here the player walks round its rim as Black's would struggle across it)
static bool8 PosDeepEnough(s16 y, u16 w, u16 h)
{
    float g;
    if (y != -2 || w != 5 || h != 5)   // (the quicksand's own: a sunken 5 x 5; other triggers as they are)
        return TRUE;
    g = U3D_TestGround(gSaveBlock1Ptr->pos.x * 16 + 8, gSaveBlock1Ptr->pos.y * 16 + 8, y);   // (world units)
    return isnan(g) || g <= y - 6;
}

// special U3D_VmPos: the trigger spot the player stepped on (its work held its value: the coord event's var)
void U3D_VmPos(void)
{
    u32 size, i;
    const u8 *e = Events(CurrentZone(), &size);
    s32 m = U3D_MapIndex();
    s32 x = gSaveBlock1Ptr->pos.x, y = gSaveBlock1Ptr->pos.y;
    if (m >= 0)
        x += 32 * U3D_MapCellX(m), y += 32 * U3D_MapCellZ(m);
    for (i = 0; e && size >= 8 && i < e[7]; i++)
    {
        const u8 *p = e + 8 + EV_BG * e[4] + EV_PERSON * e[5] + EV_CONNECT * e[6] + EV_POS * i;
        u16 px = Le16(p + 10), pz = Le16(p + 12), w = Le16(p + 14), h = Le16(p + 16);
        if (Le16(p + 8) == 0 && x >= px && x < px + (w ? w : 1) && y >= pz && y < pz + (h ? h : 1)
         && WorkValue(Le16(p + 4)) == Le16(p + 2) && PosDeepEnough((s16)Le16(p + 18), w, h)
         && Run(Le16(p), 0xFFFF, TRUE))
            return;
    }
    // (a trigger on a rail: its tiles laid out by tools/unova/gen_maps.py, src/unova/u3d_railpos.h)
    for (i = 0; e && size >= 8 && i < ARRAY_COUNT(sUnovaRailPos); i++)
    {
        const u8 *p;
        if (sUnovaRailPos[i].zone != CurrentZone() || sUnovaRailPos[i].x != x || sUnovaRailPos[i].z != y
         || sUnovaRailPos[i].index >= e[7])
            continue;
        p = e + 8 + EV_BG * e[4] + EV_PERSON * e[5] + EV_CONNECT * e[6] + EV_POS * sUnovaRailPos[i].index;
        if (WorkValue(Le16(p + 4)) == Le16(p + 2) && Run(Le16(p), 0xFFFF, TRUE))
            return;
    }
    NothingToRun();
}

// A map's own scripts (Black's init file): on loading it the field-init script (kind 4) runs to its end at once
static const u8 *InitFile(u16 zone, u32 *size)
{
    u16 f = ZoneField(zone, Z_INIT);
    return f == 0xFFFF ? (*size = 0, NULL) : FileOf(sFilesAt, f, size);
}

static u16 InitScript(u16 zone, u16 kind)
{
    u32 size, o;
    const u8 *d = InitFile(zone, &size);
    for (o = 0; d && o + 6 <= size && Le16(d + o) != 0; o += 6)
        if (Le16(d + o) == kind)
            return Le16(d + o + 2);
    return 0xFFFF;
}

static void RunNow(u16 scriptId)
{
    struct Thread *t = &sThread;
    u32 guard;
    // (a script paused by its warp: the new map's own load scripts run beside it, then it goes on as it was)
    static struct Thread sPaused;
    static u16 sPausedLocal[ARRAY_COUNT(sLocal)];
    static u8 sPausedMoving[ARRAY_COUNT(sMoving)];
    bool8 paused = t->alive && t->wait == W_WARP, holding = sHolding;
    if (scriptId == 0xFFFF)
        return;
    if (paused)
    {
        sPaused = *t;
        memcpy(sPausedLocal, sLocal, sizeof(sLocal));
        memcpy(sPausedMoving, sMoving, sizeof(sMoving));
        t->alive = FALSE;
    }
    if (Run(scriptId, 0xFFFF, FALSE))
    {
        for (guard = 0; guard < 64 && t->alive; guard++)
        {
            t->wait = W_NONE;   // (no waits on loading: it places people and sets works)
            Step(t);
        }
        t->alive = FALSE;
    }
    if (paused)
    {
        *t = sPaused;
        memcpy(sLocal, sPausedLocal, sizeof(sLocal));
        memcpy(sMoving, sPausedMoving, sizeof(sMoving));
        sHolding = holding;
    }
}

// ON_TRANSITION of every UNOVA map (data/scripts/unova.inc): the zone-change and field-init scripts; the first time,
// Black's new-game script before them (SCRIPT_SYS_RunInitScript: script 9600, the flags that hide what isn't there yet)
#define SCRIPT_NEW_GAME 9600
// BURGH's gym (zone 29, Black's gimmick 38, ov038 field_gimmick_plate_doors.c / ov037 gimmick_init_helpers.c): its
// eight floor switches (works 0x4000-0x4007, set as each is stepped on) light plates (gPlateDoorsSwitchPlates); a honey
// door opens once both its plates are lit (gPlateDoorsDoorPlates), the entry's always. Black's doors are the zone's
// people 0-6, standing in the doors' cells: an open door's is gone, so the player walks through it north or south (Black
// pushes through the honey; the walls beside a door keep it to that), a shut one's stays a wall. The doors' and plates'
// models: PlateModels
#define ZONE_CASTELIA_GYM 29
static const s8 sPdSwitchPlates[8][2] = {{0, -1}, {1, -1}, {2, -1}, {3, 4}, {5, -1}, {6, -1}, {7, 8}, {9, -1}};
static const u8 sPdDoorPlates[6][2] = {{0, 1}, {2, 3}, {4, 5}, {6, 6}, {7, 7}, {8, 9}};
static const u8 sPdDoorPerson[7] = {6, 4, 5, 3, 2, 1, 0};   // (door -> its person: gPlateDoorsDoorPos's cells)

static bool8 PlateDoorOpen(u32 door)
{
    u32 lit = 0, s, k;
    if (door >= 6)
        return TRUE;
    for (s = 0; s < 8; s++)
        if (*WorkPtr(0x4000 + s) != 0)   // (a switch found)
            for (k = 0; k < 2; k++)
                if (sPdSwitchPlates[s][k] >= 0)
                    lit |= 1 << sPdSwitchPlates[s][k];
    return (lit >> sPdDoorPlates[door][0] & 1) && (lit >> sPdDoorPlates[door][1] & 1);
}

// the models (ov038): a plate lit shows pressed (its press animation, 1, at its end; played as it lights), a door its
// open model or its shut one (Init's doorOpen)
static u16 sPdPlatesShown;

static u16 PlatesLit(void)
{
    u32 s, k;
    u16 lit = 0;
    for (s = 0; s < 8; s++)
        if (*WorkPtr(0x4000 + s) != 0)
            for (k = 0; k < 2; k++)
                if (sPdSwitchPlates[s][k] >= 0)
                    lit |= 1 << sPdSwitchPlates[s][k];
    return lit;
}

// (the switches found as shown: their two texture tracks, lit at their last frame; the doors shown open; a door
// opening: Black's effect model 33 at its place, its tracks 4-7, then the open door: PlateDoorsFrame)
static u8 sPdSwitchesShown, sPdDoorsShown, sPdOpening = 0xFF;

static void PlateModels(bool8 load)
{
    u16 lit = PlatesLit();
    u32 i, k;
    for (i = 0; i < 8; i++)   // (Init: each switch's tracks on and paused, at their last frame once found; found,
    {                         //  EventPressSwitch plays them)
        bool8 on = *WorkPtr(0x4000 + i) != 0, was = (sPdSwitchesShown >> i) & 1;
        if (load || on != was)
            for (k = 0; k < 2; k++)
                U3D_ExpTex(i, k, TRUE, on ? (load ? -1 : 0) : 0, on && !load, FALSE);
    }
    sPdSwitchesShown = 0;
    for (i = 0; i < 8; i++)
        sPdSwitchesShown |= (*WorkPtr(0x4000 + i) != 0) << i;
    for (i = 0; i < 10; i++)
    {
        bool8 on = (lit >> i) & 1, was = (sPdPlatesShown >> i) & 1;
        if (load || on != was)
        {
            U3D_ExpAnim(8 + i, on ? 1 : 0xFF, on && !load ? 0 : -1, on && !load);
            U3D_ExpTex(8 + i, 0, TRUE, on ? (load ? -1 : 0) : 0, on && !load, FALSE);   // (its texture track too)
        }
    }
    sPdPlatesShown = lit;
    for (i = 0; i < 7; i++)
    {
        bool8 open = PlateDoorOpen(i);
        if (open && !load && !((sPdDoorsShown >> i) & 1) && i < 6 && sPdOpening == 0xFF)
        {
            float pos[3];   // (EventPressPlate: the shut door hidden, the effect played where it stood)
            U3D_ExpGetPos(18 + i, pos);
            U3D_ExpPlace(33, pos[0], pos[1], pos[2], 0);
            U3D_ExpHide(33, FALSE);
            U3D_ExpAnim(33, 4, 0, TRUE);
            for (k = 5; k < 8; k++)
                U3D_ExpTex(33, k, TRUE, 0, TRUE, FALSE);
            sPdOpening = i;
            MPNet_Log("u3d vm: BURGH's gym, door %u opening (the effect at %d,%d)", (unsigned)i, (int)pos[0], (int)pos[2]);
        }
        U3D_ExpHide(18 + i, open);
        U3D_ExpHide(25 + i, !open || sPdOpening == i);
    }
    sPdDoorsShown = 0;
    for (i = 0; i < 7; i++)
        sPdDoorsShown |= PlateDoorOpen(i) << i;
    if (load)
        sPdOpening = 0xFF, U3D_ExpHide(33, TRUE);
}

// BURGH's gym each frame: the door effect over (its joint track 4 stopped: the open door shown), and Black's scripted
// event (StartScriptedEvent: the honey cocoon's effect model 32 at its place, zone script 3 / 4 called, then 5 / 6
// once the effect stops: the trainer out of the honey and the battle)
extern const u8 Unova_EventScript_Scene[];

static void PlateDoorsFrame(void)
{
    if (sPdOpening != 0xFF && !U3D_ExpPlaying(33))
    {
        u32 k;
        for (k = 5; k < 8; k++)
            U3D_ExpTex(33, k, FALSE, 0, FALSE, FALSE);
        U3D_ExpHide(33, TRUE);
        U3D_ExpHide(25 + sPdOpening, FALSE);
        PlaySE(SE_UNLOCK);   // (Black's 0x6DC as the door's model swaps)
        MPNet_Log("u3d vm: BURGH's gym, door %u now open", (unsigned)sPdOpening);
        sPdOpening = 0xFF;
    }
    if (sPdScripted && !sThread.alive && !ScriptContext_IsEnabled())
    {
        u16 id = sPdScripted == 1 ? (sPdScriptedVariant ? 4 : 3) : (sPdScriptedVariant ? 6 : 5);
        if (sPdScripted == 2 && U3D_ExpPlaying(32))
            return;
        if (Run(id, 0xFFFF, TRUE))
            ScriptContext_SetupScript(Unova_EventScript_Scene);
        MPNet_Log("u3d vm: BURGH's gym, the honey's script %u", id);
        sPdScripted = sPdScripted == 1 ? 2 : 0;
    }
}

static void PlateDoorsRefresh(bool8 sound)
{
    u32 d;
    if (CurrentZone() != ZONE_CASTELIA_GYM)
        return;
    PlateModels(!sound);
    for (d = 0; d < ARRAY_COUNT(sPdDoorPerson); d++)
        if (PlateDoorOpen(d))
        {
            u8 local = LocalId(sPdDoorPerson[d]), id = ObjEventId(local);
            bool8 wasThere = id < OBJECT_EVENTS_COUNT && !gObjectEvents[id].isPlayer;
            if (getenv("MOEMON_U3D_TRACE"))
                printf("[u3d] gym door %u open: person local %u object %u\n", (unsigned)d, local, id);
            Gone(local, TRUE);
            if (wasThere)
            {
                RemoveObjectEventByLocalIdAndMap(gObjectEvents[id].localId, gObjectEvents[id].mapNum,
                                                 gObjectEvents[id].mapGroup);
                if (sound)
                    PlaySE(SE_UNLOCK);   // (Black's door opening, 0x6DC)
            }
        }
}

// CLAY's gym (zone 97, Black's gimmick 43, ov043 field_gimmick_lift_elevator.c): a mine of platforms at heights 50, 40
// and 30 over its ground (Black's height areas, sLiftElevatorFixedHeightAreas: u3d.c U3D_HexSet), and six lifts, each
// a platform at one of two heights (sLiftElevatorLiftZones) its levers send between them (Black's signs at the lifts'
// ends, a sign for each height: only the player's floor's answer). Whoever stands on a lift rides it; lift 5 goes down
// to the ground, CLAY's. The lifts' states sWork.gym[1] (a bit each), the player's floor sWork.gym[2] (128 + its
// height; 0 not yet). The lifts, the obstacle and the elevator: Black's models (MineModels), ridden at Black's speed
static const s16 sMineArea[8][5] = {{7, 19, 17, 1, 50}, {7, 23, 17, 1, 50}, {13, 24, 5, 3, 50}, {7, 19, 17, 1, 40},
                                    {7, 20, 1, 3, 40}, {7, 23, 17, 1, 40}, {7, 19, 6, 1, 30}, {7, 23, 11, 1, 30}};
static const s16 sMineLift[6][6] = {{18, 24, 5, 3, 50, 40}, {18, 20, 5, 3, 40, 50}, {8, 24, 5, 3, 40, 30},
                                    {13, 24, 5, 3, 30, 40}, {8, 20, 5, 3, 30, 50}, {14, 20, 3, 3, 50, -59}};

static bool8 MineFloorHeight(s16 h)   // (a height one of the platforms' floors is at, not the ground's)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sMineArea); i++)
        if (sMineArea[i][4] == h)
            return TRUE;
    return FALSE;
}

// (the tables' heights are in tiles; the engine's, Black's world units, a tile's 16: the pit's floor is near -1000)
#define MINE_H(h) ((h) * 16)

static void MineSetup(void)
{
    u32 i;
    U3D_HexClear();
    for (i = 0; i < ARRAY_COUNT(sMineArea); i++)
        U3D_HexSet(i, sMineArea[i][0], sMineArea[i][1], sMineArea[i][2], sMineArea[i][3], MINE_H(sMineArea[i][4]));
    for (i = 0; i < ARRAY_COUNT(sMineLift); i++)
        U3D_HexSet(8 + i, sMineLift[i][0], sMineLift[i][1], sMineLift[i][2], sMineLift[i][3],
                   MINE_H(sMineLift[i][4 + ((sWork.gym[1] >> i) & 1)]));
    if (sWork.gym[2] == 0)   // (arriving: on the highest floor where they stand, the lobby's lift's 50)
    {
        float top = U3D_HexTopAtPlayer();
        sWork.gym[2] = isnan(top) ? 1 : 128 + (s32)top / 16;
    }
    U3D_SetFloor(sWork.gym[2] == 1 ? NAN : (float)MINE_H((s32)sWork.gym[2] - 128));
    MPNet_Log("u3d vm: CLAY's mine, lifts %02X, floor %d", sWork.gym[1], sWork.gym[2] - 128);
}

// The mine's models (ov043): the lifts at their heights (Black's world units, a tile 16), the obstacle broken once the
// big lift (5) has gone down past it (sWork.gym[1] bit 7, Black's obstacleBroken)
static const u8 sMineLiftFlip[6] = {1, 0, 1, 0, 0, 1};   // (sLiftElevatorLiftAnimeFlip)

// (a model's texture track TRACK shown at its first frame, paused, the other of its two off: ShowLiftAnime,
// ShowElevatorIdleAnime, ov044 ShowIdleAnime)
static void LiftTexShow(u8 model, u8 track)
{
    U3D_ExpTex(model, track ^ 1, FALSE, 0, FALSE, FALSE);
    U3D_ExpTex(model, track, TRUE, 0, FALSE, FALSE);
}

static void LiftTexArrive(u8 model, u8 track, u8 idle)
{
    U3D_ExpTex(model, track ^ 1, FALSE, 0, FALSE, FALSE);
    U3D_ExpTex(model, track, TRUE, 0, TRUE, FALSE);
    sLiftTex.model = model, sLiftTex.track = track, sLiftTex.idle = idle;
    MPNet_Log("u3d vm: model %u's lamp, track %u", model, track);
}

static void LiftTexStep(void)
{
    if (sLiftTex.model >= 0 && !U3D_ExpTexPlaying(sLiftTex.model, sLiftTex.track))
    {
        LiftTexShow(sLiftTex.model, sLiftTex.idle);
        sLiftTex.model = -1;
    }
}

static void MineModels(void)
{
    u32 i;
    U3D_ExpLoad(43);
    sLiftTex.model = -1;
    for (i = 0; i < ARRAY_COUNT(sMineLift); i++)
    {
        U3D_ExpSetY(i, MINE_H(sMineLift[i][4 + ((sWork.gym[1] >> i) & 1)]));
        LiftTexShow(i, ((sWork.gym[1] >> i) & 1) ^ sMineLiftFlip[i]);
    }
    U3D_ExpAnim(6, 0, sWork.gym[1] & 0x80 ? -1 : 0, FALSE);
    LiftTexShow(7, 0);
}

// a lever: lift I to its other height, the player riding it if they stand on it at the height it leaves. It rides as
// Black's lift event does (LiftEventMain: 4 units a field frame, the big lift 8; it stops at the obstacle, which
// breaks, and goes on at 2 after a while), the script waiting till it arrives
static void MineLift(u32 i)
{
    u32 was = (sWork.gym[1] >> i) & 1;
    s16 from = sMineLift[i][4 + was], to = sMineLift[i][4 + !was];
    float floor = U3D_Floor();
    sWork.gym[1] ^= 1 << i;
    sMineRide.rider = U3D_HexHasPlayer(8 + i)
        && ((isnan(floor) && !MineFloorHeight(from)) || (!isnan(floor) && fabsf(floor - MINE_H(from)) < 64));
    sMineRide.lift = i;
    sMineRide.cur = MINE_H(from);
    sMineRide.goal = MINE_H(to);
    sMineRide.step = (i == 5 ? 4.0f : 2.0f) * (to < from ? -1 : 1);   // (a game frame: half a field frame's)
    sMineRide.stall = 0;
    PlaySE(SE_ELEVATOR);
    MPNet_Log("u3d vm: lift %u %d -> %d%s", (unsigned)i, from, to, sMineRide.rider ? " (riding)" : "");
}

// the lobby's lift and the mine's elevator (ov044 LiftEventMain, ov043 ElevatorEventMain: 2 units a field frame), the
// player riding on a floor area (Black's HEIGHT_EX) under its tiles that goes with it
static void PlatRide(u8 model, u8 hex, s16 x, s16 z, s16 w, s16 d, s16 from, s16 to)
{
    sPlat.model = model, sPlat.hex = hex, sPlat.x = x, sPlat.z = z, sPlat.w = w, sPlat.d = d;
    sPlat.cur = from, sPlat.goal = to, sPlat.step = to < from ? -1.0f : 1.0f;
    U3D_HexSet(hex, x, z, w, d, from);
    U3D_SetFloor(from);
    U3D_ExpSetY(model, from);
    PlaySE(SE_ELEVATOR);
    MPNet_Log("u3d vm: model %u rides %d -> %d", model, from, to);
}

static void PlatStep(void)
{
    LiftTexStep();
    if (sPlat.model < 0)
        return;
    sPlat.cur += sPlat.step;
    if ((sPlat.step < 0 && sPlat.cur <= sPlat.goal) || (sPlat.step > 0 && sPlat.cur >= sPlat.goal))
    {
        sPlat.cur = sPlat.goal;
        U3D_ExpSetY(sPlat.model, sPlat.cur);
        MPNet_Log("u3d vm: model %d arrived at %d", sPlat.model, (int)sPlat.cur);
        if (CurrentZone() == ZONE_CLAY_LOBBY && sPlat.step > 0)   // (ov044: rising, its track 0 played, then idle 1)
            LiftTexArrive(0, 0, 1);
        else if (CurrentZone() != ZONE_CLAY_LOBBY && sPlat.step < 0)   // (ov043's elevator down: track 1, idle 0)
            LiftTexArrive(sPlat.model, 1, 0);
        sPlat.model = -1;
        if (CurrentZone() == ZONE_CLAY_LOBBY)   // (the lobby's floor again: the grid's own)
        {
            U3D_HexClear();
            U3D_SetFloor(NAN);
        }
        else
            MineSetup();   // (the mine's floors, the elevator's area back at its platform's 800)
        return;
    }
    U3D_ExpSetY(sPlat.model, sPlat.cur);
    U3D_HexSet(sPlat.hex, sPlat.x, sPlat.z, sPlat.w, sPlat.d, (s16)sPlat.cur);
    U3D_SetFloor(sPlat.cur);
}

static void MineRideStep(void)
{
    s32 i = sMineRide.lift;
    s16 to;
    if (i < 0)
        return;
    if (sMineRide.stall)   // (stopped at the broken obstacle: the camera shakes, then slower)
    {
        if (--sMineRide.stall == 0)
            sMineRide.step = -1.0f;
        return;
    }
    sMineRide.cur += sMineRide.step;
    if (i == 5 && !(sWork.gym[1] & 0x80) && sMineRide.cur <= -560.0f)   // (UpdateBigLift: the obstacle at -0x230000)
    {
        sWork.gym[1] |= 0x80;
        sMineRide.cur = -560.0f;
        sMineRide.stall = 200;   // (Black's 100 shakes, every second field frame)
        U3D_ExpAnim(6, 0, 0, TRUE);
        PlaySE(SE_M_ROCK_THROW);
    }
    if ((sMineRide.step < 0 && sMineRide.cur <= sMineRide.goal) || (sMineRide.step > 0 && sMineRide.cur >= sMineRide.goal))
        sMineRide.cur = sMineRide.goal, sMineRide.lift = -1;
    U3D_ExpSetY(i, sMineRide.cur);   // (the lift's floor area goes with it: whoever stands on it rides it)
    U3D_HexSet(8 + i, sMineLift[i][0], sMineLift[i][1], sMineLift[i][2], sMineLift[i][3], (s16)sMineRide.cur);
    if (sMineRide.rider)
        U3D_SetFloor(sMineRide.cur);
    if (sMineRide.lift < 0)
    {
        to = (s16)(sMineRide.goal / 16.0f);
        if (sMineRide.rider)
        {
            sWork.gym[2] = MineFloorHeight(to) ? 128 + to : 1;
            U3D_SetFloor(MineFloorHeight(to) ? (float)MINE_H(to) : NAN);
        }
        PlaySE(SE_PIN);
        LiftTexArrive(i, (((sWork.gym[1] >> i) & 1) ^ 1) ^ sMineLiftFlip[i], ((sWork.gym[1] >> i) & 1) ^ sMineLiftFlip[i]);
        MPNet_Log("u3d vm: lift %d arrived at %d, floor %d", (int)i, to, sWork.gym[2] - 128);
    }
}

// an unseen blocker of CLAY's gym at grid tile X, Z on the floor at height H (NAN: the ground) (u3d.c U3D_StepBlocked)
bool8 U3D_Vm_BlockerAt(s32 x, s32 z, float h)
{
    u32 size, i;
    const u8 *e;
    if (!Ready() || CurrentZone() != ZONE_DRIFTVEIL_GYM || (e = Events(CurrentZone(), &size)) == NULL || size < 8)
        return FALSE;
    for (i = 0; i < e[5]; i++)
    {
        const u8 *p = e + 8 + EV_BG * e[4] + EV_PERSON * i;
        float ph = (s16)Le16(p + 34) * 16.0f;   // (Black's event height in tiles: world units, as the floors)
        if ((Le16(p + 2) == 185 || Le16(p + 2) == 0) && Le16(p + 28) == x && Le16(p + 30) == z
         && (isnan(h) ? ph < 320.0f : fabsf(ph - h) < 128.0f))
            return TRUE;
    }
    return FALSE;
}

// a person's height in Black's event record (its y's whole part, in tiles), for the floors of height areas (u3d.c), in
// world units as they are
bool8 U3D_Vm_PersonHeight(u8 localId, float *h)
{
    const u8 *p;
    if (localId == 0 || localId > 0xF0 || !Ready() || (p = Person(CurrentZone(), localId - 1)) == NULL)
        return FALSE;
    *h = ((s16)Le16(p + 34) + Le16(p + 32) / 65536.0f) * 16.0f;
    return TRUE;
}

// The field shown on a map (u3d.c U3D_Update; a continued game's people come from the save, past the map's load script)
// BRYCEN's switches (ov045 FIELD_GIMMICK_SWITCH_TOGGLE): a switch's model at its state's pose (SetSwitchModelPose:
// animation 1 on, 0 off, at their first frame). Its toggle (command 0x194: in this gym the overlays' shared address is
// ov045's StartSwitchToggleEvent, not ov042's one-shot) plays the old state's animation to its end, then the new pose;
// the script then sets the switch's work (0x4001 + the switch) to match. sIceShown: bit 0 the state shown, bit 1 the
// change playing

// (the second models, 3-5: SetSwitchModelPose's texture track 1 at its first frame while on; SecondarySwitchEventProc
// plays the shown one to its end, then the other pose. sIceSec: bit i on, bit 4 + i its track playing)
static u8 sIceSec;
static void IceSecondary(u32 i)
{
    if (sIceSec & (1 << i))
    {
        U3D_ExpTex(3 + i, 1, TRUE, 0, TRUE, FALSE);
        sIceSec = (sIceSec & ~(1 << i)) | (16 << i);
    }
    else
    {
        U3D_ExpTex(3 + i, 1, TRUE, 0, FALSE, FALSE);
        sIceSec |= 1 << i;
    }
    MPNet_Log("u3d vm: BRYCEN's gym, switch %u's light %s", (unsigned)i, sIceSec & (1 << i) ? "on" : "going off");
}

static void IceToggle(u32 i)
{
    U3D_ExpAnim(i, sIceShown[i] & 1, 0, TRUE);
    sIceShown[i] = (~sIceShown[i] & 1) | 2;
}

static void IceModels(bool8 load)
{
    u32 i;
    for (i = 0; i < 3; i++)
    {
        u8 on = WorkValue(0x4001 + i) != 0;
        if (!load && (sIceSec & (16 << i)) && !U3D_ExpTexPlaying(3 + i, 1))
        {
            sIceSec &= ~(16 << i);
            U3D_ExpTex(3 + i, 1, FALSE, 0, FALSE, FALSE);
        }
        if (load)
        {
            sIceShown[i] = on;
            U3D_ExpAnim(i, on, 0, FALSE);
            sIceSec = (sIceSec & ~(17 << i)) | (on << i);
            U3D_ExpTex(3 + i, 1, on, 0, FALSE, FALSE);
        }

        else if (sIceShown[i] & 2)
        {
            if (!U3D_ExpPlaying(i))
            {
                sIceShown[i] &= 1;
                U3D_ExpAnim(i, sIceShown[i], 0, FALSE);
            }
        }
        else if (on != sIceShown[i])   // (a work set some other way: the change shown all the same)
            IceToggle(i);
    }
}

// u3d.c U3D_Update, each frame on an UNOVA map: the gyms' machinery that follows the player and the works
void U3D_Vm_FieldFrame(void)
{
    {
        void Unova_Screen_FieldFrame(void);
        Unova_Screen_FieldFrame();   // (what a closed C-GEAR / ENTRALINK / MUSICAL screen left to do: u3d_screen.c)
    }
    if (!Ready())
        return;
    if (CurrentZone() == DRAGON_GYM_ZONE)
        DragonInside();
    else if (CurrentZone() == ZONE_CASTELIA_GYM)
        PlateDoorsFrame();
    else if (CurrentZone() == ICE_GYM_ZONE)
        IceModels(FALSE);
    else if (CurrentZone() == ZONE_NIMBASA_GYM)
        CartStep();
    else if (IS_TIME_EFFECT_ZONE(CurrentZone()))
        TimeFxFrame();
    else if (CurrentZone() == ZONE_THRONE)
        ArcFrame();
    else if (CurrentZone() == ZONE_DRIFTVEIL_GYM || CurrentZone() == ZONE_CLAY_LOBBY)
    {
        MineRideStep();
        PlatStep();
    }
    CrowdFrame();
    IcaFrame();
    MotionFrame();
    BoatFrame();
    U3D_Elboard_Frame();
    D06Frame();
    LanesFrame();
    SoundObjsFrame();
    BridgeFrame();
    LeagueFrame();
    {
        void U3D_Subway_SwayFrame(u16 zone);
        U3D_Subway_SwayFrame(CurrentZone());   // (the BATTLE SUBWAY's carriage: u3d_subway.c)
    }
}

void U3D_Vm_FieldShown(void)
{
    if (!Ready())
        return;
    PlateDoorsRefresh(FALSE);
    {
        void U3D_TownMapArrive(u16 zone);
        U3D_TownMapArrive(CurrentZone());   // (Black's arrival flags: FLY's towns, src/unova/u3d_townmap.c)
    }
    {
        void U3D_Elboard_Clear(void);
        U3D_Elboard_Clear();
    }
    if (CurrentZone() == DRAGON_GYM_ZONE)   // (the gyms' machinery: Black's models, u3d.c U3D_Exp*)
    {
        U3D_ExpLoad(46);
        DragonModels();
        DragonInside();
    }
    else if (CurrentZone() == ICE_GYM_ZONE)
    {
        U3D_ExpLoad(45);
        IceModels(TRUE);
    }
    else if (CurrentZone() == ZONE_STRIATON_GYM)   // (ov042 Init: the curtains played at their end; the state models
    {                                              // as the zone's scripts set them)
        u32 k;
        U3D_ExpLoad(42);
        for (k = 0; k < 9; k++)
            if (sButtonModelsSet & (1 << k))
                U3D_ExpAnim(k, 0, (sButtonModels >> k) & 1 ? -1 : 0, FALSE);
        for (k = 0; k < 3; k++)
            if (sWork.gym[6] & (1 << k))
                U3D_ExpAnim(9 + k, 0, -1, FALSE);
    }
    else if (CurrentZone() == ZONE_CLAY_LOBBY)   // (ov044 Init: the lift up, at the lobby's floor)
    {
        U3D_ExpLoad(44);
        sLiftTex.model = -1;
        LiftTexShow(0, 1);
    }
    else if (CurrentZone() == ZONE_MISTRALTON_GYM)   // (ov041 Init: the cannons, turned, at rest)
        U3D_ExpLoad(41);
    else if (CurrentZone() == ZONE_NIMBASA_GYM)   // (ov039 Setup: each lane's cart where it has got to)
    {
        u32 lane;
        U3D_ExpLoad(39);
        CartModels();
        for (lane = 0; lane < 4; lane++)
            CartTex(lane);
    }
    else if (CurrentZone() == ZONE_CASTELIA_GYM)   // (ov038 Init: plates and doors as the switches found make them)
    {
        U3D_ExpLoad(38);
        PlateModels(TRUE);
    }
    else if (IS_TIME_EFFECT_ZONE(CurrentZone()))   // (ov065 Init: by night or day, hidden unless it's pending)
    {
        u8 tz = TimeZoneNow();
        sTimeFx.night = tz >= 3, sTimeFx.playing = FALSE;
        U3D_ExpLoad(sTimeFx.night ? 165 : 65);
        U3D_ExpHide(0, !(sWork.white & TIMEFX_PENDING));
        if (sWork.white & TIMEFX_PENDING)
        {
            U3D_ExpAnim(0, 0, 0, FALSE);
            U3D_ExpTex(0, 1, TRUE, 0, FALSE, FALSE);
        }
    }
    else if (CurrentZone() == ZONE_THRONE)   // (ov064 Init: both models hidden)
    {
        U3D_ExpLoad(64);
        memset(&sArc, 0, sizeof(sArc));
    }
    else if (U3D_Elboard_Load(CurrentZone()))   // (ov048: a gate's news board, u3d_elboard.c)
        ;
    else if (D06Floor() >= 0)   // (ov051: CHARGESTONE CAVE's stones)
        D06Load();
    else if (CurrentZone() == 326 || CurrentZone() == 157)   // (ov058: ROUTE 4's traffic)
        LanesLoad();
    else if (CurrentZone() == 249 || CurrentZone() == 254)   // (ov053 / ov054: SKYARROW's and TUBELINE's traffic)
        SoundObjsLoad();
    else if (CurrentZone() == 137 || CurrentZone() == 138)   // (ov049 / ov050: the LEAGUE's hub and the CHAMPION's room)
        LeagueLoad();
    else if (CurrentZone() >= 67 && CurrentZone() <= 76)   // (ov052: the GEAR STATION's platforms, their trains
        U3D_ExpLoad(52);                                   //  brought in by the subway's scripts: u3d_subway.c)
    else if (CurrentZone() == ZONE_NACRENE_GYM)   // (ov040 Init: the shelf moved, if it has been)
    {
        U3D_ExpLoad(40);
        if (sWork.gym[7] & 1)
            U3D_ExpAnim(0, 0, -1, FALSE);
    }
    else
        U3D_ExpClear();
    if (CurrentZone() == ZONE_DRIFTVEIL_GYM)
    {
        MineSetup();
        MineModels();
    }
    else if (U3D_HexActive())
    {
        U3D_HexClear();
        sWork.gym[1] = sWork.gym[2] = 0;   // (Black's gimmick starts afresh on the next visit, its arrival script setting
                                           // the levers' flags 0xA-0xF; it starts where the lobby's lift leaves them)
    }
}

// ELESA's gym (zone 63, Black's gimmick 39, ov039 field_gimmick_cart_lanes.c): four lanes of coaster rails, each with
// two carts on loops its lever picks (cart = lane * 2 + lever: a switch thrown takes the other cart as the lane's comes
// round to its start). The carts run all the time (UpdateTask: a frame of the cart's animation each field frame),
// parking at each of their stops (sStopFrames / sStopDocks) for 90 field frames. The player walks onto a dock facing
// the rails: with the lane's cart parked there (CheckRideEvent) the gate opens, Black's script 13 boards them, the gate
// shuts and they ride on Black's path for the cart (archive 0x6B, at the cart's frame), hidden in it; at a stop DOWN
// gets them off (RideEventMain): the gate, the exit two cells north of the dock, Black's script 14. Lanes 1 and 2 are
// locked until their trainer is beaten (Black's flags 0x74, 0x75: zone 63's script 8 unlocks them as it sets them).
// The floor switches throw a lane's lever (CART_LANES_CreateSwitchEvent). Tables: u3d_gyms.h (gen_gyms.py)
#define CART_DOCK_FRAMES 90
static struct
{
    bool8 init;
    u8 cart[4], swap[4], tick, phase, wait, lane, riding, arriving;
    u16 frame[4], timer[4];
    s8 parked[4], holdLane;
    u8 gate[4];   // (the gate's resting frame: 0 shut, CART_GATE_OPEN open)
} sCarts;
static u8 sRideFrom;

static void CartSwap(u32 lane)
{
    sCarts.swap[lane] = TRUE;
}

// (Setup: the lane's gate (the cart model's texture track 0) shut at its first frame, paused; each of the lane's two
// levers (models 4-7, 8-11) showing the lever's track at its last frame)
static void CartTex(u32 lane)
{
    u32 k, on = (sWork.gym[0] >> lane) & 1;
    U3D_ExpTex(lane, 0, TRUE, sCarts.gate[lane], FALSE, FALSE);
    for (k = 1; k <= 2; k++)
    {
        U3D_ExpTex(lane + 4 * k, on ^ 1, FALSE, 0, FALSE, FALSE);
        U3D_ExpTex(lane + 4 * k, on, TRUE, -1, FALSE, FALSE);
    }
}

// (the lever event: the other track on, played from its start)
static void CartLever(u32 lane)
{
    u32 k, on = (sWork.gym[0] >> lane) & 1;
    for (k = 1; k <= 2; k++)
    {
        U3D_ExpTex(lane + 4 * k, on ^ 1, FALSE, 0, FALSE, FALSE);
        U3D_ExpTex(lane + 4 * k, on, TRUE, 0, TRUE, FALSE);
    }
    MPNet_Log("u3d vm: ELESA's gym, lane %u's levers to %u", (unsigned)lane, (unsigned)on);
}

// (the gate: frames 0-15 open it, 15-30 shut it again: GATE_OPEN_FRAME)
#define CART_GATE_OPEN 15
static void CartGate(u32 lane, bool8 open)
{
    sCarts.gate[lane] = open ? CART_GATE_OPEN : 0;
    U3D_ExpTex(lane, 0, TRUE, open ? 0 : CART_GATE_OPEN, TRUE, FALSE);
    MPNet_Log("u3d vm: ELESA's gym, lane %u's gate %s", (unsigned)lane, open ? "opening" : "shutting");
}

static u16 CartFrames(u32 cart)
{
    return sCartRides[cart][1] ? sCartRides[cart][1] : 1;
}

static void CartModels(void)
{
    u32 lane;
    if (!sCarts.init)
    {
        for (lane = 0; lane < 4; lane++)
        {
            sCarts.cart[lane] = lane * 2 + ((sWork.gym[0] >> lane) & 1);
            sCarts.parked[lane] = -1;
        }
        sCarts.holdLane = -1;
        sCarts.init = TRUE;
    }
    for (lane = 0; lane < 4; lane++)
        U3D_ExpAnim(lane, sCarts.cart[lane] + 1, sCarts.frame[lane], FALSE);
}

static void CartStep(void)
{
    u32 lane, k;
    if (!sCarts.init)
        return;
    sCarts.tick ^= 1;
    if (sCarts.tick)
    {
        for (lane = 0; lane < 4; lane++)
        {
            u32 cart = sCarts.cart[lane];
            if (sCarts.swap[lane] && sCarts.frame[lane] == 0)   // (the other cart, as the lane's comes round)
            {
                sCarts.cart[lane] = cart = lane * 2 + ((sWork.gym[0] >> lane) & 1);
                sCarts.swap[lane] = FALSE;
            }
            if (sCarts.timer[lane])
            {
                if (sCarts.holdLane != (s8)lane && --sCarts.timer[lane] == 0)
                    sCarts.parked[lane] = -1;
                continue;
            }
            sCarts.frame[lane] = (sCarts.frame[lane] + 1) % CartFrames(cart);
            for (k = 0; k < 2; k++)
                if (sCartStops[cart][k][1] != 255 && sCartStops[cart][k][0] == sCarts.frame[lane])
                {
                    sCarts.timer[lane] = CART_DOCK_FRAMES;
                    sCarts.parked[lane] = sCartStops[cart][k][1];
                    break;
                }
        }
        CartModels();
    }
    if (sCarts.arriving == 1 && sThread.alive && sThread.script == 14)
        sCarts.arriving = 2;
    else if (sCarts.arriving == 2 && !sThread.alive)
    {
        sCarts.arriving = 0;
        CartDone();
    }
    if (!sCarts.riding && sCarts.holdLane < 0 && !ScriptContext_IsEnabled() && !ArePlayerFieldControlsLocked()
     && !gObjectEvents[gPlayerAvatar.objectEventId].singleMovementActive
     && gObjectEvents[gPlayerAvatar.objectEventId].facingDirection == DIR_NORTH && U3D_Vm_DockStep(DIR_NORTH)
     && sCarts.parked[sCartDocks[sRideFrom][2]] == (s8)sRideFrom)
    {
        // (CheckRideEvent, as the field checks it while the player stands on the dock: the cart's come)
        extern const u8 Unova_EventScript_Ride[];
        ScriptContext_SetupScript(Unova_EventScript_Ride);
    }
    if (sCarts.riding)   // (the rider: Black's path for the cart at its frame, from its lane's place)
    {
        u32 cart = sCarts.cart[sCarts.lane], f = sCarts.frame[sCarts.lane];
        const s16 *pt = sCartPath[sCartRides[cart][0] + (f < sCartRides[cart][1] ? f : 0)];
        const s16 *o = sCartRideOffset[sCarts.lane];
        U3D_PlayerFly(TRUE, pt[0] / 8.0f + o[0], pt[1] / 8.0f + o[1], pt[2] / 8.0f + o[2]);
    }
}

// (field_control_avatar.c: a step onto a dock of the gym, facing north as Black's rider)
bool8 U3D_Vm_DockStep(u16 direction)
{
    s32 m = U3D_MapIndex(), x = gSaveBlock1Ptr->pos.x, y = gSaveBlock1Ptr->pos.y;
    u32 d;
    if (direction != DIR_NORTH || !Ready() || CurrentZone() != ZONE_NIMBASA_GYM || m < 0)
        return FALSE;
    x += 32 * U3D_MapCellX(m), y += 32 * U3D_MapCellZ(m);
    for (d = 0; d < ARRAY_COUNT(sCartDocks); d++)
        if (sCartDocks[d][0] == x && sCartDocks[d][1] == y)
        {
            sRideFrom = d;
            return TRUE;
        }
    return FALSE;
}

static bool8 CartLaneLocked(u32 lane)
{
    return (lane == 1 && !FlagOn(0x74)) || (lane == 2 && !FlagOn(0x75));
}

// special: 1 the lane's cart is parked here (held for the boarding), 2 no cart here (nothing), 0 the lane is locked
u16 U3D_VmRideKind(void)
{
    u32 lane = sCartDocks[sRideFrom][2];
    if (!sCarts.init || sCarts.parked[lane] != (s8)sRideFrom)
        return 2;
    sCarts.holdLane = lane;   // (Black pauses the cart as the player steps up)
    if (CartLaneLocked(lane))
        return 0;
    sCarts.lane = lane;
    return 1;
}

static void Task_CartRide(u8 taskId)
{
    struct ObjectEvent *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
    u32 lane = sCarts.lane;
    if (sCarts.wait && --sCarts.wait)
        return;
    switch (sCarts.phase)
    {
    case 0:   // (the gate, open for the boarding: 15 field frames)
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    case 1:   // (the gate shut behind them: the cart goes, the player in it)
        pl->invisible = TRUE;
        sCarts.holdLane = -1;
        sCarts.timer[lane] = 0;
        sCarts.parked[lane] = -1;
        sCarts.riding = TRUE;
        PlaySE(SE_TRUCK_MOVE);
        sCarts.phase = 2;
        return;
    case 2:   // (riding: at a stop, DOWN gets them off)
        if (sCarts.parked[lane] >= 0 && JOY_HELD(DPAD_DOWN))
        {
            s32 m = U3D_MapIndex();
            u32 dock = sCarts.parked[lane];
            sCarts.holdLane = lane;
            gSpecialVar_0x8004 = sCartDocks[dock][0] - (m >= 0 ? 32 * U3D_MapCellX(m) : 0);
            gSpecialVar_0x8005 = sCartDocks[dock][1] - 2 - (m >= 0 ? 32 * U3D_MapCellZ(m) : 0);
            PlaySE(SE_TRUCK_STOP);
            CartGate(lane, TRUE);   // (RideEventMain 5: the gate opens again)
            sCarts.phase = 3;
            sCarts.wait = 30;
        }
        return;
    case 3:   // (the gate open again: out they get; Black's script 14 as the exit cell loads, then the cart goes)
        sCarts.riding = FALSE;
        U3D_PlayerFly(FALSE, 0, 0, 0);
        pl->invisible = FALSE;
        sPendingScript = 14;
        sCarts.arriving = 1;
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }
}

// special (waitstate): a door the player walks at opens (u3d.c U3D_DoorOpen, at 0x8004 / 0x8005), the walk then
// going on into it (field_control_avatar.c)
static void Task_DoorWait(u8 taskId)
{
    if (U3D_DoorBusy())
        return;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

void U3D_VmDoorOpen(void)
{
    if (U3D_DoorOpen(gSpecialVar_0x8004, gSpecialVar_0x8005, FALSE))
        CreateTask(Task_DoorWait, 80);
    else
        ScriptContext_Enable();
}

// special (waitstate): the gate opens for the boarding
void U3D_VmCartGate(void)
{
    sCarts.phase = 0;
    sCarts.wait = 30;
    CartGate(sCarts.lane, TRUE);
    PlaySE(SE_DOOR);
    CreateTask(Task_CartRide, 80);
}

// special: Black's script 13, the player climbs into the cart
void U3D_VmCartBoard(void)
{
    if (!Run(13, 0xFFFF, TRUE))
        NothingToRun();
}

// special (waitstate): the gate shuts, the ride, off at a stop with DOWN (0x8004 / 0x8005 the exit cell)
void U3D_VmCartRide(void)
{
    sCarts.phase = 1;
    sCarts.wait = 30;
    CartGate(sCarts.lane, FALSE);
    PlaySE(SE_DOOR);
    CreateTask(Task_CartRide, 80);
}

// the gate shut, the cart goes on its way (once Black's script 14 has run on the exit cell)
static void CartDone(void)
{
    if (sCarts.holdLane >= 0)
    {
        CartGate(sCarts.holdLane, FALSE);   // (the gate shut behind them)
        sCarts.timer[sCarts.holdLane] = 0;
        sCarts.parked[sCarts.holdLane] = -1;
    }
    sCarts.holdLane = -1;
}

// special: a locked lane's dock: Black's scripts there, its trainer comes over (7), then the battle that unlocks it (8)
void U3D_VmRideLocked(void)
{
    if (Run(7, 0xFFFF, TRUE))
        sPendingScript = 8;
    else
        NothingToRun();
}

// SKYLA's gym's cannons (Black's field_gimmick_cannons, ov041): the player walks into the cell before a cannon
// (CreateLaunchEvent: two cells ahead of the facing). The launch (EventLaunch): the cannon's animation for its
// direction (the facing turned by the cannon's quarter turns) runs to its fire frame, Black's script 2 climbs the player
// in, the player is hidden and the animation goes on: the flight starts at its flight frame, along the flight's ICA path
// from the cannon's cell, the player seen again at its landing frame, and the flight ends on its landing cell. The
// tables are ov041's (src/unova/u3d_gyms.h from tools/unova/gen_gyms.py); no flight that way: nothing happens
static struct { s8 cannon; u8 ring, dir, phase, seen; float frame; s16 ox, oz;
                u16 followEnd, ret, bounce; bool8 looking, back; } sShot = {-1};
// (the flight's camera, EventLaunch: turned to sCannonCamera's pitch and yaw before the shot; once the landing
// frame's bounce (8 field frames) is over the eye holds and the view follows the player to the flight's follow frames,
// then goes back over its return frames)
static u8 sCannonLandX, sCannonLandZ;

bool8 U3D_Vm_CannonAhead(u16 direction)
{
    static const u8 sRing[] = {[DIR_NORTH] = 0, [DIR_SOUTH] = 1, [DIR_EAST] = 2, [DIR_WEST] = 3};
    s32 m = U3D_MapIndex(), x = gSaveBlock1Ptr->pos.x, z = gSaveBlock1Ptr->pos.y, ox = 0, oz = 0;
    u32 c;
    if (!Ready() || CurrentZone() != ZONE_MISTRALTON_GYM || m < 0 || direction < DIR_SOUTH || direction > DIR_EAST)
        return FALSE;
    ox = 32 * U3D_MapCellX(m), oz = 32 * U3D_MapCellZ(m);
    switch (direction)
    {
    case DIR_NORTH: z -= 2; break;
    case DIR_SOUTH: z += 2; break;
    case DIR_WEST:  x -= 2; break;
    default:        x += 2; break;
    }
    for (c = 0; c < ARRAY_COUNT(sCannonCells); c++)
        if (sCannonCells[c][0] == x + ox && sCannonCells[c][2] == z + oz)
        {
            const s16 *fl = sCannonFlights[c][sRing[direction]];
            if (fl[1] == 0)
                return FALSE;   // (Black: no flight that way, nothing happens)
            sShot.cannon = c, sShot.ring = sRing[direction], sShot.dir = sCannonAnimDir[c][sRing[direction]];
            sShot.ox = ox, sShot.oz = oz;
            sCannonLandX = fl[2] - ox, sCannonLandZ = fl[4] - oz;
            return TRUE;
        }
    return FALSE;
}


// a cannon's firing: Black enables its joint track (2 x the direction) with the material track after it (the glow's
// colour and alpha: field_gimmick_cannons.c, both enabled and disabled together)
static void CannonAnim(u8 anim, s16 frame, bool8 play)
{
    u32 d;
    for (d = 0; d < 4; d++)
        U3D_ExpTex(sShot.cannon, d * 2 + 1, anim == d * 2, frame, play, FALSE);
    U3D_ExpAnim(sShot.cannon, anim, frame, play);
}

static void Task_CannonAim(u8 taskId)
{
    float f = U3D_ExpFrame(sShot.cannon);
    if (!gTasks[taskId].data[0] && f >= sCannonFrames[CANNON_FIRE_SE][sShot.dir])
    {
        gTasks[taskId].data[0] = TRUE;
        PlaySE(SE_M_STRENGTH);   // (Black's 0x698: the cannon turns to its aim)
    }
    if (f < 0 || f >= sCannonFrames[CANNON_FIRE_DONE][sShot.dir])
    {
        CannonAnim(sShot.dir * 2, sCannonFrames[CANNON_FIRE_DONE][sShot.dir], FALSE);
        DestroyTask(taskId);
        ScriptContext_Enable();
    }
}

// special (waitstate): the cannon's animation to its fire frame
void U3D_VmCannonAim(void)
{
    if (sShot.cannon < 0)
    {
        ScriptContext_Enable();
        return;
    }
    CannonAnim(sShot.dir * 2, 0, TRUE);
    CreateTask(Task_CannonAim, 80);
}

// special: Black's script 2 (the player climbs in, by the way they face); 0x8004 / 0x8005 the landing cell
void U3D_VmCannon(void)
{
    gSpecialVar_0x8004 = sCannonLandX;
    gSpecialVar_0x8005 = sCannonLandZ;
    if (!Run(2, 0xFFFF, TRUE))
        NothingToRun();
}

static void Task_CannonFly(u8 taskId)
{
    const s16 *fl = sCannonFlights[sShot.cannon][sShot.ring];
    struct ObjectEvent *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
    float f = U3D_ExpFrame(sShot.cannon);
    const u16 *cam = sCannonCamera[fl[5] < (s16)ARRAY_COUNT(sCannonCamera) ? fl[5] : 0];
    if (sShot.phase == 3)   // (the turn to the flight's view, then the shot: EventLaunch 3)
    {
        if (U3D_CameraMoving())
            return;
        gObjectEvents[gPlayerAvatar.objectEventId].invisible = TRUE;
        CannonAnim(sShot.dir * 2, sCannonFrames[CANNON_FIRE_DONE][sShot.dir], TRUE);
        PlaySE(SE_M_EXPLOSION);   // (Black's 0x699, the shot)
        sShot.phase = 0;
        return;
    }
    if (sShot.phase == 0 && f >= sCannonFrames[CANNON_FLIGHT_START][sShot.dir])
    {
        sShot.phase = 1;   // (the flight: Black's 0x6A0)
        sShot.frame = 0;
        PlaySE(SE_M_FLY);
    }
    if (!sShot.seen && f >= sCannonFrames[CANNON_LANDING][sShot.dir])
    {
        sShot.seen = TRUE;   // (Black shows the player again at the animation's landing frame)
        pl->invisible = FALSE;
        if (!sShot.back)
            sShot.bounce = 1;   // (StartBounce)
    }
    if (sShot.phase == 1)
    {
        u32 k = (u32)sShot.frame;
        const s16 *pt = sCannonPath[fl[0] + (k < (u32)fl[1] ? k : (u32)fl[1] - 1)];
        const u16 *cell = sCannonCells[sShot.cannon];
        U3D_PlayerFly(TRUE, pt[0] / 8.0f + cell[0] * 16 + 8, pt[1] / 8.0f + cell[1] * 16, pt[2] / 8.0f + cell[2] * 16 + 8);
        if (sShot.frame == (u32)sShot.frame)   // (a field frame: Black's camera steps)
        {
            if (sShot.bounce && ++sShot.bounce > 8)
                sShot.bounce = 0xFFFF;   // (UpdateBounce: two up-and-downs, the camera free to follow)
            if (sShot.bounce == 0xFFFF && !sShot.looking && k < sShot.followEnd)
            {
                U3D_CameraLook(sShot.followEnd - k);
                sShot.looking = TRUE;
            }
            if (k == sShot.followEnd && !sShot.back)
            {
                U3D_CameraMoveBack(sShot.ret);
                sShot.back = TRUE;
            }
            if (fl[5] == 11 ? k == 0x46 : k == (u32)fl[1] - 1 - (s16)cam[5])
                PlaySE(SE_LEDGE);   // (Black's 0x697, the landing, so many frames before the end; flight 11 its 0x6A1)
        }
        sShot.frame += 0.5f;   // (a point a field frame, 30 a second)
        if (sShot.frame >= fl[1])
        {
            if (!sShot.back)
                U3D_CameraMoveBack(sShot.ret);   // (PopState)
            U3D_PlayerFly(FALSE, 0, 0, 0);
            pl->invisible = FALSE;
            CannonAnim(0xFF, 0, FALSE);   // (its animations off again, the cannon at rest)
            sShot.cannon = -1;
            DestroyTask(taskId);
            ScriptContext_Enable();
        }
    }
}

// special (waitstate): the shot, the player hidden in the cannon, and the flight to the landing cell
void U3D_VmCannonFly(void)
{
    if (sShot.cannon < 0)
    {
        ScriptContext_Enable();
        return;
    }
    {
        const s16 *fl = sCannonFlights[sShot.cannon][sShot.ring];
        const u16 *cam = sCannonCamera[fl[5] < (s16)ARRAY_COUNT(sCannonCamera) ? fl[5] : 0];
        sShot.ret = cam[4];
        sShot.followEnd = cam[3] + cam[4] >= fl[1] ? fl[1] - (cam[4] + 1) : cam[3];
        sShot.bounce = 0;
        sShot.looking = sShot.back = FALSE;
        U3D_CameraTurn(cam[0], cam[1], cam[2]);   // (EventLaunch 2: pitch and yaw, over its move frames)
        MPNet_Log("u3d vm: SKYLA's cannon %d, flight camera %d", sShot.cannon, fl[5]);
    }
    sShot.phase = 3;
    sShot.seen = FALSE;
    CreateTask(Task_CannonFly, 80);
}

// BRYCEN's gym (ICIRRUS, zone 114, Black's gimmick ov045 field_gimmick_switch_toggle.c): an ice rink whose curved
// slides are rails (src/unova/u3d_gyms.h from tools/unova/gen_gyms.py). A slide starts where Black checks the player's
// facing (a pos event of check type 1-4) and its script hands the player to it when the switches allow
// (ReplacePlayerIfRequirementMet: switch S on or off, ov045's requirement table); here the player rides it to its
// far end and the ice takes them on. The three floor switches are works 0x4001-0x4003, flipped by Black's own scripts
// as they move the turning pieces' unseen blockers (the switches' pieces: Black's models, IceModels)
static s8 sIceRide = -1;

static bool8 IceRequirementMet(u16 req)
{
    if (req == ICE_NO_REQUIREMENT)
        return TRUE;
    return req < ARRAY_COUNT(sIceRequirement)
        && (WorkValue(0x4001 + sIceRequirement[req][0]) != 0) == sIceRequirement[req][1];
}

// field_control_avatar.c: the player stands on a slide's start facing it, and its switch lets them on
bool8 U3D_Vm_IceSlideAhead(u16 direction)
{
    s32 m = U3D_MapIndex(), x = gSaveBlock1Ptr->pos.x, z = gSaveBlock1Ptr->pos.y;
    u32 i;
    if (!Ready() || CurrentZone() != ICE_GYM_ZONE || m < 0)
        return FALSE;
    x += 32 * U3D_MapCellX(m), z += 32 * U3D_MapCellZ(m);
    for (i = 0; i < ARRAY_COUNT(sIceRides); i++)
        if (sIceRides[i].x == x && sIceRides[i].z == z && sIceRides[i].facing == direction
         && IceRequirementMet(sIceRides[i].req))
        {
            sIceRide = i;
            return TRUE;
        }
    return FALSE;
}

// special: the slide's own script (Black's: it turns the player and hands them to the rail); the way down from
// BRYCEN has none (Black enters it by the tile's attribute): ridden at once, VAR_RESULT 1 (the script waits for it)
void U3D_VmIceSlide(void)
{
    gSpecialVar_Result = 0;
    if (sIceRide >= 0 && sIceRides[sIceRide].script == 0)
    {
        gSpecialVar_Result = IceRide(ICE_NO_REQUIREMENT);
        sMoving[gPlayerAvatar.objectEventId] = 0;   // (the field script waits for it, not the VM)
        return;
    }
    if (sIceRide < 0 || !Run(sIceRides[sIceRide].script, 0xFFFF, TRUE))
        NothingToRun();
}

// the ride down a slide, tile by tile at the ice's pace
static bool8 IceRide(u16 req)
{
    static const u8 sSlide[] = {[DIR_SOUTH] = MOVEMENT_ACTION_SLIDE_DOWN, [DIR_NORTH] = MOVEMENT_ACTION_SLIDE_UP,
                                [DIR_WEST] = MOVEMENT_ACTION_SLIDE_LEFT, [DIR_EAST] = MOVEMENT_ACTION_SLIDE_RIGHT};
    u8 slot = gPlayerAvatar.objectEventId, *out = sMoves[slot];
    u32 k, n = 0;
    s8 i = sIceRide;
    sIceRide = -1;
    if (i < 0 || sIceRides[i].req != req || !IceRequirementMet(req))
        return FALSE;
    for (k = 0; k < sIceRides[i].count && n < MAX_MOVE - 1; k++)
        out[n++] = sSlide[sIceRides[i].steps[k]];
    out[n] = MOVEMENT_ACTION_STEP_END;
    sMoving[slot] = OBJ_EVENT_ID_PLAYER;
    ScriptMovement_StartObjectMovementScript(OBJ_EVENT_ID_PLAYER, gObjectEvents[slot].mapNum,
                                             gObjectEvents[slot].mapGroup, out);
    if (req == ICE_NO_REQUIREMENT)
        MPNet_Log("u3d vm: ice ride %u (no switch: a ramp or the way down), %u tiles", (unsigned)i, (unsigned)sIceRides[i].count);
    else
        MPNet_Log("u3d vm: ice slide %u (switch %u %s), %u tiles", (unsigned)i, sIceRequirement[req][0],
                  sIceRequirement[req][1] ? "on" : "off", (unsigned)sIceRides[i].count);
    return TRUE;
}

// DRAYDEN's gym (OPELUCID, zone 121, Black's gimmick ov046 field_gimmick_rotating_room.c): three dragon statues, each
// with two levers (thrown by buttons that bounce the player a tile, StartHopEvent) whose states pick its pose
// (CalcUnitPose: levers 0 0 pose 0, 0 1 pose 3, 1 0 pose 2, 1 1 pose 1); a pose opens a pair of the dragon's rail
// lines and closes the rest (ApplyPoseLines; the tiles in u3d_gyms.h). sWork.gym[3 + unit]: the pose (bits 0-1), the
// levers (bits 2, 3), set as Black's zone-change script resets them (DragonReset; the dragons, levers and buttons:
// Black's models, DragonModels)
#define DRAGON_STATE(u) sWork.gym[3 + (u)]

static bool8 DragonHop(u16 direction)
{
    u8 slot = gPlayerAvatar.objectEventId, *out = sMoves[slot];
    if (CurrentZone() != DRAGON_GYM_ZONE)
        return FALSE;
    out[0] = direction == 2 ? MOVEMENT_ACTION_JUMP_LEFT : MOVEMENT_ACTION_JUMP_RIGHT;   // (Black's 2 left, 3 right)
    out[1] = MOVEMENT_ACTION_STEP_END;
    sMoving[slot] = OBJ_EVENT_ID_PLAYER;
    ScriptMovement_StartObjectMovementScript(OBJ_EVENT_ID_PLAYER, gObjectEvents[slot].mapNum,
                                             gObjectEvents[slot].mapGroup, out);
    PlaySE(SE_LEDGE);
    return TRUE;
}

// the models (ov046: five a unit, its dragon, two levers, two buttons): the dragon at its pose's turn's end
// (FIELD_GIMMICK_ROTATING_ROOM_InitUnitPose: pose 0 animation 1, 1 5, 2 2, 3 0), each lever at its own's
static void DragonModels(void)
{
    static const u8 sPoseAnim[4] = {1, 5, 2, 0};
    u32 u;
    for (u = 0; u < 3; u++)
    {
        U3D_ExpAnim(u * 5, sPoseAnim[DRAGON_STATE(u) & 3], -1, FALSE);
        U3D_ExpAnim(u * 5 + 1, (DRAGON_STATE(u) >> 2) & 1, -1, FALSE);
        U3D_ExpAnim(u * 5 + 2, (DRAGON_STATE(u) >> 3) & 1, -1, FALSE);
    }
}

// (its script waits for the lever's animation, as Black's lever event does)
static bool8 DragonLever(u16 unit, u16 side)
{
    if (CurrentZone() != DRAGON_GYM_ZONE || unit >= 3 || side >= 2)
        return FALSE;
    DRAGON_STATE(unit) ^= 4 << side;
    PlaySE(SE_SWITCH);
    U3D_ExpAnim(unit * 5 + 1 + side, (DRAGON_STATE(unit) >> (2 + side)) & 1, 0, TRUE);
    sExpWait = unit * 5 + 1 + side;
    return U3D_ExpPlaying(sExpWait);
}

// (... and for the dragon's turn: the animation Black's table gives from the old pose to the new)
static bool8 DragonRotate(u16 unit)
{
    static const u8 sPose[4] = {0, 2, 3, 1};   // (levers: bit 0 the first, bit 1 the second)
    u32 from, to;
    if (CurrentZone() != DRAGON_GYM_ZONE || unit >= 3)
        return FALSE;
    from = DRAGON_STATE(unit) & 3;
    to = sPose[(DRAGON_STATE(unit) >> 2) & 3];
    DRAGON_STATE(unit) = (DRAGON_STATE(unit) & ~3) | to;
    PlaySE(SE_ELEVATOR);
    MPNet_Log("u3d vm: dragon %u turns to pose %u", (unsigned)unit, DRAGON_STATE(unit) & 3);
    if (sDragonRotateAnim[from][to] < 0)   // (no turn between the two: the dragon as its pose stands)
        DragonModels();
    else
        U3D_ExpAnim(unit * 5, sDragonRotateAnim[from][to], 0, TRUE);
    sExpWait = unit * 5;
    return U3D_ExpPlaying(sExpWait);
}

// the zone-change script's reset (ov037 GIMMICK_INIT_ResetRotatingRoomPanels): pose 3 (levers off, on), 1 (on, on),
// 2 (on, off)
static void DragonReset(void)
{
    DRAGON_STATE(0) = 3 | 8;
    DRAGON_STATE(1) = 1 | 4 | 8;
    DRAGON_STATE(2) = 2 | 4;
    DragonModels();
}

// Black hides the far dragon, its buttons and the room's outside model (15) while the player is in the room
// (FIELD_GIMMICK_ROTATING_ROOM_IsPosInside: no higher than 64, x from 256, z from 480)
static void DragonInside(void)
{
    s32 i = U3D_MapIndex();
    float x, z, y = U3D_ObjectHeight(&gObjectEvents[gPlayerAvatar.objectEventId]);
    u32 k;
    bool8 inside;
    if (i < 0)
        return;
    x = (gSaveBlock1Ptr->pos.x + 32 * U3D_MapCellX(i)) * 16 + 8;
    z = (gSaveBlock1Ptr->pos.y + 32 * U3D_MapCellZ(i)) * 16 + 8;
    inside = !(y > 64) && x >= 256 && z >= 480;
    for (k = 10; k <= 15; k++)
        U3D_ExpHide(k, inside);
}

static bool8 DragonTileIn(u32 unit, u32 pose, s32 x, s32 z)
{
    u32 k, first = sDragonPoseTiles[unit][pose][0], n = sDragonPoseTiles[unit][pose][1];
    for (k = first; k < first + n; k++)
        if (sDragonTiles[k][0] == x && sDragonTiles[k][1] == z)
            return TRUE;
    return FALSE;
}

// DRAYDEN's gym's rails (u3d_gyms.h sDragonRails): the ways out of a tile's rail cell at about height H, OR'd over
// its lines there; FALSE when no rail of it is at that height (the floor below a walkway, say)
static bool8 DragonRailWays(s32 x, s32 z, float h, u8 *ways)
{
    u32 i;
    bool8 on = FALSE;
    *ways = 0;
    for (i = 0; i < ARRAY_COUNT(sDragonRails); i++)
        if (sDragonRails[i].x == x && sDragonRails[i].z == z && fabsf(h - sDragonRails[i].h) <= 10.0f)
            *ways |= sDragonRails[i].ways, on = TRUE;
    return on;
}

// u3d.c U3D_StepBlocked: a step in DRAYDEN's gym from grid tile FX, FZ (at height HF) to TX, TZ (its ground HT) that
// gets on or off a rail other than as Black lets it (along its line, across within its width, at its ends): none
bool8 U3D_Vm_RailWayBlocked(s32 fx, s32 fz, float hf, s32 tx, s32 tz, float ht)
{
    u8 wa, wb, d, back;
    bool8 onA, onB;
    if (!Ready() || CurrentZone() != DRAGON_GYM_ZONE)
        return FALSE;
    d = tz < fz ? 1 : tz > fz ? 2 : tx < fx ? 4 : 8;
    back = d == 1 ? 2 : d == 2 ? 1 : d == 4 ? 8 : 4;
    onA = DragonRailWays(fx, fz, hf, &wa);
    onB = DragonRailWays(tx, tz, ht, &wb);
    if (!onA && !onB)
        return FALSE;
    return !((onA && (wa & d)) || (onB && (wb & back)));
}

// ------------------------------------------------------------------------------------------------ ROUTE 4's lanes
// ROUTE 4's traffic (ov058 field_gimmick_lane_trains.c, zones 326 / 157; pack gimmick 58): an eastbound and a
// westbound vehicle, each a lead and a trail 96 units behind on one of three lanes' models, run the track a tile a
// Black frame, then wait a while (east 30-59 frames, west 0-29) and take a lane at random; one passing in front of
// the player is heard (the player between z 8224 and 8512, the lead 64 units or less ahead)
#define LANE_MIN_X 6000.0f
#define LANE_MAX_X 7264.0f
static struct { bool8 on; u8 tick; struct { u16 wait, step; u8 lane; bool8 heard; } t[2]; } sLanes;

static void LanesLoad(void)
{
    memset(&sLanes, 0, sizeof(sLanes));
    sLanes.on = CurrentZone() == 326 || CurrentZone() == 157;
    if (sLanes.on)
    {
        float pw[3];
        U3D_ExpLoad(58);
        U3D_PlayerWorldPos(pw);
        MPNet_Log("u3d vm: ROUTE 4's lanes (the player at %d,%d,%d)", (int)pw[0], (int)pw[1], (int)pw[2]);
    }
}

static void LanesFrame(void)
{
    u32 k;
    float pw[3];
    if (!sLanes.on || (CurrentZone() != 326 && CurrentZone() != 157))
        return;
    if (++sLanes.tick & 1)   // (Black's field: 30 frames a second)
        return;
    U3D_PlayerWorldPos(pw);
    for (k = 0; k < 2; k++)
    {
        typeof(sLanes.t[0]) *t = &sLanes.t[k];
        u16 m = t->lane * 4 + k * 2;
        float lead;
        if (t->wait)
        {
            t->wait--;
            continue;
        }
        t->step++;
        lead = k == 0 ? LANE_MIN_X + t->step * 16.0f : LANE_MAX_X - t->step * 16.0f;
        U3D_ExpPlace(m, lead, 80, k == 0 ? 8448 : 8368, k == 0 ? 0x4000 : 0xC000);
        U3D_ExpPlace(m + 1, k == 0 ? lead - 96 : lead + 96, 80, k == 0 ? 8448 : 8368, k == 0 ? 0x4000 : 0xC000);
        if (pw[2] >= 8224 && pw[2] <= 8512 && !t->heard
         && (k == 0 ? pw[0] - lead : lead - pw[0]) >= 0 && (k == 0 ? pw[0] - lead : lead - pw[0]) <= 64)
        {
            t->heard = TRUE;
            PlaySE(SE_TRUCK_MOVE);
        }
        if (k == 0 ? lead >= LANE_MAX_X : lead <= LANE_MIN_X)
        {
            t->step = 0, t->heard = FALSE;
            t->wait = k == 0 ? Random() % 30 + 30 : Random() % 30;
            t->lane = Random() % 3;
        }
    }
}

// ------------------------------------------------------------------------------------------------ the bridges
// SKYARROW's and TUBELINE's traffic (ov053 field_gimmick_sound_objs.c, ov054 _b.c; pack gimmicks 53 / 54, paths
// u3d_soundobjs.h): each model runs its path once, a Black frame a field frame (30 a second: drawn halfway between
// on the frames between), then sits at its start a random wait between its limits; SKYARROW's trailing cars (1, 3)
// start again with their lead (0, 2). Where each was is kept while the player is away (Black's gimmick work)
#include "u3d_soundobjs.h"
static struct { s8 zone; u8 tick; s16 frame[8]; u16 wait[8]; s16 vol[16], pan[16]; s16 fade; u64 bits[16];
                bool8 bike, sliding; s32 prevZ; s32 ys[32]; u8 head, count; } sSnd = {-1};
static s16 sSndKept[ARRAY_COUNT(sSoundObjs)];

// Black's 3D sound (ISS_3DS_SYS: archive 0xB9's songs silence the parts SILENT on arrival; a source k sets part k's
// volume by its distance from the camera's eye, (reach - distance) / reach of its volume, and its pan by which side
// of the view it's on, each moved 16 / 32 a Black frame towards it; SKYARROW's part 6 comes in as the camera rises,
// FIELD_GIMMICK_SOUND_OBJS_UpdateBgmFade). The song's parts are its own tracks (INFO parts=: the studio's
// MMS_RIP_PARTS, tools/kaisong/unova_songs.py PARTS)
#define SND_SILENT_SKYARROW 0x3C0   // (archive 0xB9: SEQ_BGM_H_01 tracks 6-9, SEQ_BGM_H_03 tracks 7-9)
#define SND_SILENT_TUBELINE 0x380

static void SoundObjsHear(void)
{
    u32 k, n = sSoundZones[sSnd.zone].n, silent = CurrentZone() == 249 ? SND_SILENT_SKYARROW : SND_SILENT_TUBELINE;
    float eye[3], fwd[3];
    U3D_CameraEye(eye, fwd);
    for (k = 0; k < n; k++)
    {
        u32 o = sSoundZones[sSnd.zone].first + k;
        s8 t = sSoundObjs[o].track;
        float pos[3], d[3], dist, right[3], h, len;
        s32 vol, pan;
        if (t < 0)
            continue;
        U3D_ExpGetPos(k, pos);
        d[0] = pos[0] - eye[0], d[1] = pos[1] - eye[1], d[2] = pos[2] - eye[2];
        dist = sqrtf(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
        vol = dist > sSoundObjs[o].reach ? 0 : (s32)(sSoundObjs[o].volume * (sSoundObjs[o].reach - dist) / sSoundObjs[o].reach);
        // (CalcSourcePan: the way to the source along the view's level, on its right: + right, - left)
        right[0] = -fwd[2], right[1] = 0, right[2] = fwd[0];
        len = sqrtf(right[0] * right[0] + right[2] * right[2]);
        if (len > 0.0001f)
            right[0] /= len, right[2] /= len;
        {
            float up[3] = {right[1] * fwd[2] - right[2] * fwd[1], right[2] * fwd[0] - right[0] * fwd[2],
                           right[0] * fwd[1] - right[1] * fwd[0]};
            h = d[0] * up[0] + d[1] * up[1] + d[2] * up[2];
            d[0] -= up[0] * h, d[1] -= up[1] * h, d[2] -= up[2] * h;
        }
        len = sqrtf(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
        pan = len > 0.0001f ? (s32)((d[0] * right[0] + d[2] * right[2]) / len * 128.0f) : 0;
        pan = pan < -128 ? -128 : pan > 127 ? 127 : pan;
        if (getenv("MOEMON_SND_TRACE") && sSnd.tick % 60 == 0)
            printf("[snd] src %d at %d,%d,%d eye %d,%d,%d dist %d vol %d\n", t, (int)pos[0], (int)pos[1], (int)pos[2],
                   (int)eye[0], (int)eye[1], (int)eye[2], (int)dist, (int)vol);
        if (sSnd.vol[t] == 0 && vol > 0)
            MPNet_Log("u3d vm: part %d heard (%d units off, to %d, pan %d)", t, (int)dist, (int)vol, (int)pan);
        sSnd.vol[t] = sSnd.vol[t] < vol ? min(sSnd.vol[t] + 16, vol) : max(sSnd.vol[t] - 16, vol);
        sSnd.pan[t] = sSnd.pan[t] < pan ? min(sSnd.pan[t] + 32, pan) : max(sSnd.pan[t] - 32, pan);
    }
    if (sSoundZones[sSnd.zone].fadeTracks)   // (SKYARROW: the camera's height brings a part in)
    {
        float lo = sSoundZones[sSnd.zone].fadeLow, hi = sSoundZones[sSnd.zone].fadeHigh;
        s32 to = eye[1] <= lo ? 0 : eye[1] > hi ? 127 : (s32)(127.0f * (eye[1] - lo) / (hi - lo));
        sSnd.fade = sSnd.fade < to ? min(sSnd.fade + 16, to) : max(sSnd.fade - 16, to);
        for (k = 0; k < 16; k++)
            if (sSoundZones[sSnd.zone].fadeTracks >> k & 1)
                sSnd.vol[k] = sSnd.fade, sSnd.pan[k] = 0;
    }
    (void)silent;
}

static void SoundObjsApply(void)
{
    u32 k, silent = CurrentZone() == 249 ? SND_SILENT_SKYARROW : SND_SILENT_TUBELINE;
    for (k = 0; k < 16; k++)
        if ((silent >> k & 1) && sSnd.bits[k])
            MP_M4aTracksControl(&gMPlayInfo_BGM, sSnd.bits[k], sSnd.vol[k] * 256 / 127, sSnd.pan[k] / 2);
}

// SKYARROW's slide (FIELD_GIMMICK_SOUND_OBJS_UpdateSlideSe): the view crossing a line across the bridge (z 1884 on
// foot, 1980 on a bike) after dropping 100 or more over its last 32 moving frames plays a rush of wind (Black's
// SEQ_SE_FLD_173 / 171: SE_M_GUST / GUST2 here)
static void SoundObjsSlide(void)
{
    float eye[3], fwd[3];
    s32 z, line;
    U3D_CameraEye(eye, fwd);
    z = (s32)(eye[2] * 4096.0f);
    if (z != sSnd.prevZ)
    {
        u8 i;
        if (sSnd.count < 32)
            i = (sSnd.head + sSnd.count++) % 32;
        else
            i = sSnd.head, sSnd.head = (sSnd.head + 1) % 32;
        sSnd.ys[i] = (s32)(eye[1] * 4096.0f);
    }
    else
        sSnd.count = sSnd.head = 0;
    if (sSnd.count > 0)
    {
        bool8 bike = TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE);
        line = bike ? 0x7BC000 : 0x75C000;
        if (((z < line && sSnd.prevZ > line) || (z > line && sSnd.prevZ < line))
         && sSnd.ys[sSnd.head] - (s32)(eye[1] * 4096.0f) >= 0x64000)
        {
            PlaySE(bike ? SE_M_GUST2 : SE_M_GUST);
            MPNet_Log("u3d vm: SKYARROW's slide (the view down past z %d)", (int)(line >> 12));
        }
    }
    sSnd.prevZ = z;
}

static void SoundObjPlace(u32 o, u32 m, float f)
{
    u32 n = sSoundObjs[o].frames, pts = (n + SOUND_PATH_STEP - 1) / SOUND_PATH_STEP, i;
    float t, x, y, z, rx, ry, drx, dry;
    if (f >= n - 1)
        f = n - 1;
    i = (u32)(f / SOUND_PATH_STEP);
    t = (f - i * SOUND_PATH_STEP) / SOUND_PATH_STEP;
    {
        typeof(sSoundPath[0]) a = sSoundPath[sSoundObjs[o].at + i];
        typeof(sSoundPath[0]) b = sSoundPath[sSoundObjs[o].at + (i + 1 < pts ? i + 1 : i)];
        x = (a.x + (b.x - a.x) * t) / 4.0f;
        y = (a.y + (b.y - a.y) * t) / 4.0f;
        z = (a.z + (b.z - a.z) * t) / 4.0f;
        drx = (s16)(u16)(b.rx - a.rx), dry = (s16)(u16)(b.ry - a.ry);   // (the short way round)
        rx = a.rx + drx * t, ry = a.ry + dry * t;
    }
    U3D_ExpPlaceRot(m, x, y, z, (u16)(s32)rx, (u16)(s32)ry);
}

static void SoundObjsLoad(void)
{
    u32 z, k;
    sSnd.zone = -1;
    for (z = 0; z < ARRAY_COUNT(sSoundZones); z++)
        if (sSoundZones[z].zone == CurrentZone())
            sSnd.zone = z;
    if (sSnd.zone < 0)
        return;
    U3D_ExpLoad(CurrentZone() == 249 ? 53 : 54);
    memset(sSnd.vol, 0, sizeof(sSnd.vol));   // (ISS_3DS_SYS_Enter: the song's sources' parts silent, then brought in)
    memset(sSnd.pan, 0, sizeof(sSnd.pan));
    sSnd.fade = 0, sSnd.count = sSnd.head = 0, sSnd.prevZ = 0;
    for (k = 0; k < 16; k++)
        sSnd.bits[k] = MP_Kaisong_PartTracks(gMPlayInfo_BGM.songHeader, k);
    SoundObjsApply();
    if (CurrentZone() == 249)
        U3D_ExpTex(4, 0, TRUE, 0, TRUE, TRUE);   // (the ship's wake: SetAnimeEnabled, looping)
    sSnd.tick = 0;
    for (k = 0; k < sSoundZones[sSnd.zone].n; k++)
    {
        u32 o = sSoundZones[sSnd.zone].first + k;
        sSnd.frame[k] = sSndKept[o];   // (RestoreAnimeFrames)
        sSnd.wait[k] = 0;
        SoundObjPlace(o, k, sSnd.frame[k]);
    }
    {
        float pw[3], eye[3], fwd[3];
        U3D_PlayerWorldPos(pw);
        U3D_CameraEye(eye, fwd);
        MPNet_Log("u3d vm: %s's traffic, %u models (the player at %d,%d,%d, the eye at %d,%d,%d)",
                  CurrentZone() == 249 ? "SKYARROW" : "TUBELINE", (unsigned)sSoundZones[sSnd.zone].n, (int)pw[0],
                  (int)pw[1], (int)pw[2], (int)eye[0], (int)eye[1], (int)eye[2]);
    }
}

static void SoundObjsFrame(void)
{
    u32 k, n;
    if (sSnd.zone < 0)
        return;
    if (sSoundZones[sSnd.zone].zone != CurrentZone())
    {
        sSnd.zone = -1;
        return;
    }
    if (!sSnd.bits[8] && !sSnd.bits[9])   // (the song started after the zone's load: its parts now)
    {
        for (k = 0; k < 16; k++)
            sSnd.bits[k] = MP_Kaisong_PartTracks(gMPlayInfo_BGM.songHeader, k);
        if (sSnd.bits[8] || sSnd.bits[9])
            MPNet_Log("u3d vm: the song's parts 6-9 on tracks %llx %llx %llx %llx", (unsigned long long)sSnd.bits[6],
                      (unsigned long long)sSnd.bits[7], (unsigned long long)sSnd.bits[8], (unsigned long long)sSnd.bits[9]);
    }
    n = sSoundZones[sSnd.zone].n;
    SoundObjsApply();
    if (++sSnd.tick & 1)   // (between Black's frames: halfway on)
    {
        for (k = 0; k < n; k++)
            if (!sSnd.wait[k] && sSnd.frame[k] + 1 < sSoundObjs[sSoundZones[sSnd.zone].first + k].frames)
                SoundObjPlace(sSoundZones[sSnd.zone].first + k, k, sSnd.frame[k] + 0.5f);
        return;
    }
    for (k = 0; k < n; k++)
    {
        u32 o = sSoundZones[sSnd.zone].first + k;
        if (sSnd.wait[k])
        {
            if (--sSnd.wait[k] == 0)
            {
                sSnd.frame[k] = 0;   // (RestartAnime)
                SoundObjPlace(o, k, 0);
                if (n == 5 && (k == 0 || k == 2))   // (the trailing car goes with its lead)
                    sSnd.wait[k + 1] = 1;
            }
        }
        else
        {
            if (++sSnd.frame[k] >= sSoundObjs[o].frames)   // (ICA_ANIME_IncFrameLoop: round to the start, wait)
            {
                sSnd.frame[k] = 0;
                sSnd.wait[k] = sSoundObjs[o].waitMin + Random() % (sSoundObjs[o].waitMax - sSoundObjs[o].waitMin + 1);
            }
            SoundObjPlace(o, k, sSnd.frame[k]);
        }
        sSndKept[o] = sSnd.frame[k];
    }
    SoundObjsHear();
    if (CurrentZone() == 249)
        SoundObjsSlide();
}

// ------------------------------------------------------------------------------------------------ the LEAGUE
// The LEAGUE's hub (ov049 field_gimmick_models_a.c, zone 137): a lamp for each ELITE FOUR member (models 2-5, lit by
// flags 0x96B, 0x96A, 0x969, 0x96C: their second animation, else their first), the centre (models 0, 1) alight once
// all four are. Script command 0x1BB (ov010 NAMEIN_SETUP_RunFallEvent): the view moves to Black's (sLeagueCam, 20
// Black frames), the centre platform sinks with the player on it (sLeagueFallY, a rumble playing), the screen goes
// black and the player is in the CHAMPION's room (zone 138, ov050 field_gimmick_models_b.c), where RunLandEvent brings
// the platform down from above (sLeagueLandY) with its glow shown and both playing once; landed, a thud and the view
// shakes; when the glow's done it's hidden and the player is on the floor. The script goes on there.
#include "u3d_league.h"
enum { LG_NONE, LG_CAMERA, LG_FALL, LG_FADE, LG_WARP, LG_LAND, LG_SETTLE };
static struct { u8 phase, tick; u16 f; bool8 landing; float x, z; } sLeague;
static const u16 sLeagueLampFlags[4] = {0x96B, 0x96A, 0x969, 0x96C};   // (models 2-5)

static void LeagueLoad(void)
{
    u32 k, lit = 0;
    if (CurrentZone() == 138)
    {
        U3D_ExpLoad(50);   // (InitModels: the platform's animation stopped, the glow hidden)
        return;
    }
    U3D_ExpLoad(49);
    for (k = 0; k < 4; k++)
    {
        bool8 on = FlagOn(sLeagueLampFlags[k]);
        lit += on;
        U3D_ExpTex(2 + k, on ? 1 : 0, TRUE, 0, TRUE, TRUE);
    }
    if (lit == 4)
        U3D_ExpTex(0, 0, TRUE, 0, TRUE, TRUE), U3D_ExpTex(1, 0, TRUE, 0, TRUE, TRUE);
    MPNet_Log("u3d vm: the LEAGUE's lamps, %u of 4 lit", (unsigned)lit);
}

void LeagueFallStart(void)
{
    float pw[3];
    U3D_PlayerWorldPos(pw);
    sLeague.x = pw[0], sLeague.z = pw[2];
    sLeague.phase = LG_CAMERA, sLeague.f = 0, sLeague.tick = 0;
    U3D_PlayerDepthTest(2);
    U3D_CameraMoveTo((u16)sLeagueCam[0], (u16)sLeagueCam[1], sLeagueCam[2], sLeagueCam[3], sLeagueCam[4], sLeagueCam[5],
                     40);
    MPNet_Log("u3d vm: the LEAGUE's platform sinks");
}

static bool8 LeagueWarpNow(void)
{
    u8 group, num;
    s16 x, y;
    if (sLeague.phase != LG_WARP)
        return FALSE;
    sLeague.phase = LG_NONE;
    if (!U3D_MapOfZoneTile(0x8A, 30, 83, &group, &num, &x, &y))   // (Black's zone header: its start, 30, 83)
    {
        U3D_PlayerFly(FALSE, 0, 0, 0);
        return FALSE;
    }
    sLeague.landing = TRUE;
    SetWarpDestination(group, num, WARP_ID_NONE, x, y);
    DoWarp();
    return TRUE;
}

static bool8 LeagueLandStart(void)
{
    if (!sLeague.landing || CurrentZone() != 0x8A)
        return FALSE;
    sLeague.landing = FALSE;
    sLeague.phase = LG_LAND, sLeague.f = 0, sLeague.tick = 0;
    sLeague.x = 30 * 16 + 8, sLeague.z = 83 * 16 + 8;   // (SnapObjToGrid: the platform's tile, z 32 on)
    U3D_ExpSetY(0, sLeagueLandY[0]);
    U3D_PlayerFly(TRUE, sLeague.x, sLeagueLandY[0], sLeague.z);
    U3D_ExpTex(0, 0, TRUE, 0, TRUE, FALSE);   // (StartModel0Anime, ShowModel1: each played once)
    U3D_ExpHide(1, FALSE);
    U3D_ExpTex(1, 0, TRUE, 0, TRUE, FALSE);
    U3D_ExpTex(1, 1, TRUE, 0, TRUE, FALSE);
    PlaySE(SE_ELEVATOR);
    MPNet_Log("u3d vm: the LEAGUE's platform comes down into the CHAMPION's room");
    return TRUE;
}

static bool8 LeagueBusy(void)
{
    return sLeague.phase != LG_NONE || sLeague.landing;
}

static void LeagueFrame(void)
{
    float y;
    switch (sLeague.phase)
    {
    case LG_CAMERA:
        if (!U3D_CameraMoving())
        {
            PlaySE(SE_ELEVATOR);   // (Black's SEQ_SE_FLD_148, the platform's hum)
            sLeague.phase = LG_FALL, sLeague.f = 0;
        }
        break;
    case LG_FALL:
    case LG_LAND:
    {
        const s16 *ys = sLeague.phase == LG_FALL ? sLeagueFallY : sLeagueLandY;
        u16 n = sLeague.phase == LG_FALL ? ARRAY_COUNT(sLeagueFallY) : ARRAY_COUNT(sLeagueLandY);
        y = ys[sLeague.f];
        if (sLeague.f + 1 < n && (sLeague.tick & 1))   // (between Black's frames: halfway)
            y = (y + ys[sLeague.f + 1]) / 2.0f;
        U3D_ExpSetY(0, y);
        U3D_PlayerFly(TRUE, sLeague.x, y, sLeague.z);
        if (sLeague.phase == LG_FALL)   // (into the pit: once below the floor its edge hides the player as they go down)
            U3D_PlayerDepthTest(y < -24.0f ? 1 : 2);
        if (++sLeague.tick & 1)
            break;
        if (++sLeague.f < n)
            break;
        if (sLeague.phase == LG_FALL)
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sLeague.phase = LG_FADE;
        }
        else
        {
            m4aSongNumStop(SE_ELEVATOR);
            PlaySE(SE_M_STRENGTH);   // (Black's SEQ_SE_FLD_94)
            U3D_CameraShake(2, 0, 3, 2, 0, 1, 0, 2);   // (the land event's shake)
            sLeague.phase = LG_SETTLE;
            MPNet_Log("u3d vm: the LEAGUE's platform has landed");
        }
        break;
    }
    case LG_FADE:
        if (!gPaletteFade.active)
        {
            U3D_PlayerDepthTest(0);
            m4aSongNumStop(SE_ELEVATOR);
            sLeague.phase = LG_WARP;
        }
        break;
    case LG_SETTLE:   // (both models' animations over: the platform stopped, the glow hidden, the player on the floor)
        if (!U3D_ExpTexPlaying(0, 0) && !U3D_ExpTexPlaying(1, 0) && !U3D_ExpTexPlaying(1, 1) && !U3D_CameraShaking())
        {
            U3D_ExpTex(0, 0, FALSE, 0, FALSE, FALSE);
            U3D_ExpHide(1, TRUE);
            U3D_ExpTex(1, 0, FALSE, 0, FALSE, FALSE);
            U3D_ExpTex(1, 1, FALSE, 0, FALSE, FALSE);
            U3D_PlayerFly(FALSE, 0, 0, 0);
            sLeague.phase = LG_NONE;
            MPNet_Log("u3d vm: the LEAGUE's landing over");
        }
        break;
    }
}

// ------------------------------------------------------------------------------------------------ ROUTE 10's bridge
// ROUTE 10's bridge (ov047 field_gimmick_bridge.c, zone 362): standing on its planks (Black's no-dash ground, terrain
// 0x33: phenomenon kind 9 in the pack, build_pack.py phenom_code) the player bobs up and down, 8 degrees of a sine a Black frame (4 a field frame here) to a
// height picked each swing from gBridgeSwayAmplitudes (8, 8, 12 or 14 units); off them, still
#define BRIDGE_ZONE 362
static const u8 sBridgeAmp[4] = {8, 8, 12, 14};
static struct { s16 angle; u8 amp; bool8 on; } sBridge;

static void BridgeFrame(void)
{
    struct ObjectEvent *p = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (CurrentZone() != BRIDGE_ZONE || U3D_PhenomKindAt(p->currentCoords.x, p->currentCoords.y) != 9)
    {
        if (sBridge.on)
            U3D_PlayerBob(0.0f);
        sBridge.on = FALSE, sBridge.angle = 0;
        return;
    }
    if (!sBridge.on)
        MPNet_Log("u3d vm: on ROUTE 10's bridge, swaying");
    sBridge.on = TRUE;
    if (sBridge.angle == 0)
        sBridge.amp = sBridgeAmp[Random() % 4];
    U3D_PlayerBob(sinf(sBridge.angle * (float)M_PI / 180.0f) * sBridge.amp);
    sBridge.angle = (sBridge.angle + 4) % 360;
}

// ------------------------------------------------------------------------------------------------ CHARGESTONE
// CHARGESTONE CAVE's stones (ov051 field_gimmick_d06.c, its effect ov021 fldeff_d06denki.c): each floor's stones
// (gGimmickD06StoneTable) float on their tiles; a push slides one straight on to the electric panel it meets
// (gGimmickD06PanelTable: CheckPushStone / ScanSlide, 32 tiles at most; no panel that way, it won't budge), and one
// resting against a panel stays (its float frozen, IsStoneNextToPanel); a panel walked up to flashes. Drawn by pack
// gimmicks 151-153 (models 0-11 the stones, 12-23 their shadows, 24 on the panels); where they are, kept in the save
#define D06_ZONE 195
static const u8 sD06Count[3] = {12, 5, 3};
static const u8 sD06Stones[3][12][2] = {
    {{15, 39}, {24, 46}, {9, 26}, {20, 27}, {21, 26}, {24, 16}, {24, 17}, {34, 26}, {40, 12}, {60, 42}, {61, 52}, {61, 53}},
    {{43, 49}, {47, 45}, {45, 14}, {43, 12}, {36, 18}},
    {{9, 23}, {21, 9}, {24, 3}}};
static const u8 sD06Level[3][12] = {{0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0}, {1, 1, 0, 0, 0}, {0, 0, 0}};   // (their y: a level
                                                                                                    //  is 16 units)
static const u8 sD06PanelCount[3] = {14, 11, 11};
static const u8 sD06Panels[3][14][2] = {
    {{8, 47}, {8, 37}, {13, 18}, {40, 24}, {27, 13}, {12, 11}, {38, 15}, {55, 50}, {62, 45}, {50, 16}, {66, 25},
     {63, 24}, {64, 26}, {25, 54}},
    {{54, 51}, {31, 48}, {31, 49}, {31, 50}, {61, 37}, {46, 41}, {50, 26}, {56, 17}, {41, 17}, {46, 5}, {52, 10}},
    {{8, 14}, {8, 35}, {12, 42}, {17, 49}, {15, 53}, {11, 50}, {20, 35}, {24, 25}, {25, 14}, {27, 6}, {23, 12}}};
static struct { s8 floor; bool8 sliding; u8 stone, frame, per; s8 dx, dz; float from[2], to[2]; u16 frozen; s32 tried;
                float h[12]; } sD06 = {-1};

static s8 D06Floor(void)
{
    u16 z = CurrentZone();
    return z >= D06_ZONE && z < D06_ZONE + 3 ? z - D06_ZONE : -1;
}

static bool8 D06Panel(s8 f, s32 x, s32 z)
{
    u32 i;
    for (i = 0; i < sD06PanelCount[f]; i++)
        if (sD06Panels[f][i][0] == x && sD06Panels[f][i][1] == z)
            return TRUE;
    return FALSE;
}

static s32 D06StoneAt(s8 f, s32 x, s32 z)
{
    u32 i;
    for (i = 0; i < sD06Count[f]; i++)
        if (sWork.stones[f][i][0] == x && sWork.stones[f][i][1] == z)
            return i;
    return -1;
}

// (a tile that stops a sliding stone: the cave's own, a person, another stone; FLDMMDL_MOVE_HitCheckMoveCurrent)
static bool8 D06Stops(s8 f, s32 x, s32 z)
{
    s32 mx = MapX(x) + MAP_OFFSET, my = MapZ(z) + MAP_OFFSET;
    if (x < 0 || z < 0 || D06Panel(f, x, z) || MapGridGetCollisionAt(mx, my) || D06StoneAt(f, x, z) >= 0)
        return TRUE;   // (a panel's tile is Black's terrain: MAP_ATTR_IsElectricPanel stops what moves)
    if (GetObjectEventIdByXY(mx, my) != OBJECT_EVENTS_COUNT)
        return TRUE;
    return mx == gSaveBlock1Ptr->pos.x + MAP_OFFSET && my == gSaveBlock1Ptr->pos.y + MAP_OFFSET;
}

// (ScanSlide: the tiles to the panel the stone meets going DX, DZ; 0 when what stops it isn't a panel)
static u32 D06Scan(s8 f, u32 i, s32 dx, s32 dz, u32 max)
{
    s32 x = sWork.stones[f][i][0], z = sWork.stones[f][i][1];
    u32 n = 0;
    while (n < max)
    {
        x += dx, z += dz;
        if (D06Stops(f, x, z))
            return D06Panel(f, x, z) ? n : 0;
        n++;
    }
    return 0;
}

static bool8 D06NextToPanel(s8 f, u32 i)
{
    static const s8 sDir[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    u32 d;
    for (d = 0; d < 4; d++)
    {
        s32 x = sWork.stones[f][i][0] + sDir[d][0], z = sWork.stones[f][i][1] + sDir[d][1];
        if (D06Panel(f, x, z))
            return TRUE;
    }
    return FALSE;
}

static void D06Place(s8 f, u32 i, float x, float z)
{
    float h = sD06.h[i] = U3D_GridGround((s32)floorf(x / 16.0f), (s32)floorf(z / 16.0f), sD06.h[i]);   // (its own floor's)
    U3D_ExpPlace(i, x, h, z, 0);
    U3D_ExpPlace(12 + i, x, h - 4.0f, z + 2.5f, 0);   // (its shadow: FLDEFF_D06DENKI_FollowUpdate's sub object)
}

static void D06Load(void)
{
    s8 f = D06Floor();
    u32 i;
    sD06.floor = f, sD06.sliding = FALSE, sD06.frozen = 0, sD06.tried = -1;
    if (f < 0)
        return;
    U3D_ExpLoad(151 + f);
    if (!(sWork.stonesPlaced & (1 << f)))   // (the first visit: the stones where Black first puts them)
    {
        memcpy(sWork.stones[f], sD06Stones[f], sizeof(sD06Stones[f]));
        sWork.stonesPlaced |= 1 << f;
    }
    for (i = 0; i < sD06Count[f]; i++)
    {
        sD06.h[i] = sD06Level[f][i] * 16.0f;   // (a pushed stone stays on the level it began on)
        D06Place(f, i, sWork.stones[f][i][0] * 16 + 8, sWork.stones[f][i][1] * 16 + 8);
        U3D_ExpHide(i, FALSE);
        U3D_ExpHide(12 + i, FALSE);
        if (D06NextToPanel(f, i))
            sD06.frozen |= 1 << i, U3D_ExpAnim(i, 0, 0, FALSE);
        else
            U3D_ExpAnim(i, 0, 0, TRUE);
    }
    for (i = 0; i < sD06PanelCount[f]; i++)
        U3D_ExpAnim(24 + i, 0, 0, TRUE);
    MPNet_Log("u3d vm: CHARGESTONE floor %d, %u stones", f, sD06Count[f]);
}

static void D06Frame(void)
{
    s8 f = sD06.floor;
    u32 i;
    if (f < 0 || D06Floor() != f)
        return;
    if (!gObjectEvents[gPlayerAvatar.objectEventId].heldMovementActive && !JOY_HELD(DPAD_ANY))
        sD06.tried = -1;   // (the d-pad let go: a new push may come)
    for (i = 0; i < sD06Count[f]; i++)   // (the floats and the panels' glow go round)
        if (!(sD06.frozen & (1 << i)) && !U3D_ExpPlaying(i))
            U3D_ExpAnim(i, 0, 0, TRUE);
    for (i = 0; i < sD06PanelCount[f]; i++)
        if (!U3D_ExpPlaying(24 + i))
            U3D_ExpAnim(24 + i, 0, 0, TRUE);
    if (sD06.sliding)   // (EventSlideStone: a tile a step, slower over its last few)
    {
        float t = (float)++sD06.frame / sD06.per;
        u32 k = sD06.stone;
        D06Place(f, k, sD06.from[0] + (sD06.to[0] - sD06.from[0]) * t, sD06.from[1] + (sD06.to[1] - sD06.from[1]) * t);
        if (sD06.frame >= sD06.per)
        {
            sD06.sliding = FALSE;
            PlaySE(SE_M_THUNDER_WAVE);
            if (D06NextToPanel(f, k))
                sD06.frozen |= 1 << k, U3D_ExpAnim(k, 0, 0, FALSE);
            MPNet_Log("u3d vm: stone %u slid to %u,%u", k, sWork.stones[f][k][0], sWork.stones[f][k][1]);
        }
    }
}

// (mp u3dstones: each stone of this floor, where it is and the tiles it would slide each way)
void U3D_Vm_DebugStones(void)
{
    s8 f = D06Floor();
    u32 i;
    if (f < 0)
        return;
    for (i = 0; i < sD06Count[f]; i++)
        printf("[test] stone %u at %u,%u: up %u down %u left %u right %u%s\n", i, sWork.stones[f][i][0],
               sWork.stones[f][i][1], D06Scan(f, i, 0, -1, 32), D06Scan(f, i, 0, 1, 32), D06Scan(f, i, -1, 0, 32),
               D06Scan(f, i, 1, 0, 32), D06NextToPanel(f, i) ? " (at a panel)" : "");
    for (i = 0; i < sD06PanelCount[f]; i++)
    {
        s32 x = sD06Panels[f][i][0], z = sD06Panels[f][i][1];
        printf("[test] panel %u at %d,%d: collision %u, beside %u %u %u %u\n", i, (int)x, (int)z,
               MapGridGetCollisionAt(MapX(x) + MAP_OFFSET, MapZ(z) + MAP_OFFSET),
               MapGridGetCollisionAt(MapX(x) + MAP_OFFSET, MapZ(z - 1) + MAP_OFFSET),
               MapGridGetCollisionAt(MapX(x) + MAP_OFFSET, MapZ(z + 1) + MAP_OFFSET),
               MapGridGetCollisionAt(MapX(x - 1) + MAP_OFFSET, MapZ(z) + MAP_OFFSET),
               MapGridGetCollisionAt(MapX(x + 1) + MAP_OFFSET, MapZ(z) + MAP_OFFSET));
    }
}

// u3d.c U3D_StepBlocked: a step onto grid tile X, Z from FX, FZ in CHARGESTONE CAVE: a stone there blocks it, and the
// player walking into one pushes it (CheckPushStone)
bool8 U3D_Vm_StoneBlocked(s32 fx, s32 fz, s32 x, s32 z, bool8 isPlayer)
{
    s8 f = D06Floor();
    s32 i;
    u32 steps;
    if (!Ready() || f < 0 || sD06.floor != f)
        return FALSE;
    if (isPlayer)
    {
        u32 k;
        for (k = 0; k < sD06PanelCount[f]; k++)   // (a panel walked up to flashes: TriggerPanelPulse)
            if (sD06Panels[f][k][0] == x && sD06Panels[f][k][1] == z && !U3D_ExpPlaying(24 + k))
                U3D_ExpAnim(24 + k, 1, 0, TRUE);
    }
    if ((i = D06StoneAt(f, x, z)) < 0)
        return D06Panel(f, x, z);   // (a panel's tile isn't walked onto either)
    if (!isPlayer || sD06.sliding || D06NextToPanel(f, i))
        return TRUE;
    {
        s32 try = ((fx * 128 + fz) * 128 + x) * 128 + z;   // (one push a walk into it: the d-pad held asks each frame)
        if (try == sD06.tried)
            return TRUE;
        sD06.tried = try;
    }
    steps = D06Scan(f, i, x - fx, z - fz, 32);
    if (steps == 0)
    {
        PlaySE(SE_WALL_HIT);   // (EventStoneBlocked)
        return TRUE;
    }
    sD06.sliding = TRUE, sD06.stone = i, sD06.frame = 0;
    sD06.per = steps * 8 + 8;   // (Black's slide: about 8 of this game's frames a tile, slowing at the end)
    sD06.from[0] = x * 16 + 8, sD06.from[1] = z * 16 + 8;
    sWork.stones[f][i][0] += (x - fx) * steps, sWork.stones[f][i][1] += (z - fz) * steps;
    sD06.to[0] = sWork.stones[f][i][0] * 16 + 8, sD06.to[1] = sWork.stones[f][i][1] * 16 + 8;
    PlaySE(SE_M_STRENGTH);
    MPNet_Log("u3d vm: stone %u pushed %d,%d, %u tiles", i, (int)(x - fx), (int)(z - fz), steps);
    return TRUE;
}

// u3d.c U3D_StepBlocked: a dragon's line its pose keeps closed (grid tile X, Z)
bool8 U3D_Vm_DragonBlocked(s32 x, s32 z)
{
    u32 u, p;
    if (!Ready() || CurrentZone() != DRAGON_GYM_ZONE)
        return FALSE;
    for (u = 0; u < 3; u++)
    {
        u32 pose = DRAGON_STATE(u) & 3;
        if (DragonTileIn(u, pose, x, z))
            continue;
        for (p = 0; p < 4; p++)
            if (p != pose && DragonTileIn(u, p, x, z))
                return TRUE;
    }
    return FALSE;
}

// overworld.c LoadMapFromCameraTransition: the next map load is a walk across a map's edge (one zone's maps side by
// side), not a warp
static bool8 sSeamless;
void U3D_Vm_SeamlessLoad(void)
{
    sSeamless = TRUE;
}

void U3D_VmMapLoad(void)
{
    u16 zone = CurrentZone();
    static u16 sLastZone = 0xFFFF;
    bool8 seamless = sSeamless;
    sSeamless = FALSE;
    if (zone == 0xFFFF || !Ready())
        return;
    sAddedCount = 0;   // (a new map: its field-init script adds its own)
    sGoneCount = 0;
    SysFlagsToKai();   // (saves from before 1.192.1 got JUNIPER's POKeDEX in Black's flags only)
    if (!sWork.began)
    {
        sWork.began = TRUE;
        RunNow(SCRIPT_NEW_GAME);
    }
    if (zone != sLastZone)
    {
        memset(sMoveCodes, 0, sizeof(sMoveCodes));   // (a script's move-code changes last the zone)
        RoamersMove();   // (Black moves its roamers on as the player changes zones)
    }
    // (every warp, into the same zone too, runs the zone-change script: Black's EVENT_MAPCHANGE_ApplyLocation; the
    // ROYAL UNOVA's cabins are doors within its zone, their passengers drawn by the works it sets)
    if (zone != sLastZone || !seamless)
    {
        u32 i;
        // (Black's SCRIPT_SYS_ClearLocalWork before the zone-change script: the zone's own flags 1-0x63 and works
        // 0x4000-0x401F start afresh in each zone; RELIC CASTLE's quicksand waits for work 0x4000 to be 0)
        for (i = 1; i <= 0x63; i++)
            FlagTo(i, FALSE);
        for (i = 0x4000; i <= 0x401F; i++)
            *WorkPtr(i) = 0;
        memset(sConnMove, 0, sizeof(sConnMove));
        memset(sBgMove, 0, sizeof(sBgMove));   // (a new zone: its connections where its data puts them)
        sNpcAtCount = 0;   // (a new zone: its people where its data puts them, till its scripts say)
        RunNow(InitScript(zone, 2));
    }
    else
    {
        u32 i;
        for (i = 0; i < sNpcAtCount; i++)   // (another of the zone's maps: its people set as before)
            SetObjEventTemplateCoords(sNpcAt[i].local, MapX(sNpcAt[i].x), MapZ(sNpcAt[i].z));
    }
    sLastZone = zone;
    RunNow(InitScript(zone, 4));
    if (zone == ZONE_BLACK_CITY)
        WfbcMapLoad();
    PlateDoorsRefresh(FALSE);
    if (WhiteZekromHere(ZEKROM_LOCAL))
        sWork.work[0x20] = 0xD6;   // (its work object code: Black's ZEKROM sprite, D3 / D4 its RESHIRAM)
}

// script.c TryRunOnFrameMapScript, on an UNOVA map: a scene the zone's table starts (its work holds its value), held by
// Unova_EventScript_Scene until it ends
extern const u8 Unova_EventScript_Scene[];
bool8 U3D_Vm_SceneStart(void)
{
    u32 size, o, at;
    u16 zone = CurrentZone();
    const u8 *d;
    if (zone == 0xFFFF || !Ready())
        return FALSE;
    if (sThread.alive && sThread.wait == W_WARP)
    {
        // (the script whose warp brought the player here goes on: FENNEL's scene sets its work after hers)
        SpawnAdded();
        sThread.wait = LeagueLandStart() ? W_LEAGUE : W_NONE;   // (the LEAGUE's landing: RunLandEvent first)
        sHolding = TRUE;
        if (!FuncIsActiveTask(Task_Vm))
            CreateTask(Task_Vm, 80);
        MPNet_Log("u3d vm: script %u goes on in zone %u", sThread.script, zone);
        ScriptContext_SetupScript(Unova_EventScript_Scene);
        return TRUE;
    }
    if (sThread.alive)
        return FALSE;
    SpawnAdded();
    if (sPendingScript)   // (a script's SetProcAction: FIELD_EVENT_CHECK_StartPendingScript)
    {
        u16 id = sPendingScript;
        sPendingScript = 0;
        if (Run(id, 0xFFFF, TRUE))
        {
            ScriptContext_SetupScript(Unova_EventScript_Scene);
            return TRUE;
        }
    }
    d = InitFile(zone, &size);
    for (o = 0; d && o + 6 <= size && Le16(d + o) != 0; o += 6)
    {
        if (Le16(d + o) != 1)
            continue;
        at = o + 6 + (Le16(d + o + 2) | Le16(d + o + 4) << 16);   // (from the entry after its own: FindSceneScript)
        for (; at + 6 <= size && Le16(d + at) != 0; at += 6)
            if (WorkValue(Le16(d + at)) == Le16(d + at + 2))
            {
                if (!Run(Le16(d + at + 4), 0xFFFF, TRUE))
                    return FALSE;
                ScriptContext_SetupScript(Unova_EventScript_Scene);
                return TRUE;
            }
    }
    return FALSE;
}

// special U3D_VmHold (Unova_EventScript_Scene): its waitstate lasts until the scene's script ends
void U3D_VmHold(void)
{
    if (!sThread.alive)
        NothingToRun();
}

// specialvar U3D_VmScreen: the screen Black's script waits for (0: it ended)
u16 U3D_VmScreen(void)
{
    return sThread.alive && sThread.wait == W_SCREEN ? sScreen : SCREEN_NONE;
}

static void CB2_ScreenDone(void)
{
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

// special U3D_VmOpenScreen (waitstate: the script goes on when the screen closes)
void U3D_VmOpenScreen(void)
{
    static const u16 sGiftBox[3] = {495, 498, 501};   // (Black's three, national numbers: the script's 0-2)
    static u16 sSpecies[3];
    u32 i;
    switch (sScreen)
    {
    case SCREEN_STARTER:
        for (i = 0; i < 3; i++)
            sSpecies[i] = Species(sGiftBox[i]);
        MP_StarterTable_Open(sSpecies, CB2_ScreenDone);
        break;
    case SCREEN_BATTLE:
        BattleSetup_StartUnovaBattle();   // (back to the field and this script when it ends)
        break;
    case SCREEN_CATCH_DEMO:
        U3D_CatchTutorial();   // (the party saved; back to the field and this script when it ends)
        break;
    case SCREEN_WILD:   // (Black's scripted wild battle: the POKeMON set up by StartScriptedWildBattle)
        BattleSetup_StartScriptedWildBattle();
        ScriptContext_Stop();
        break;
    case SCREEN_PARTY:   // (Black's SelectPartyMon: the game's party screen, the slot in VAR_0x8004)
        ChoosePartyMon();
        break;
    case SCREEN_DAYCARE_PARTY:   // (Black's CallDaycarePartyMenu: the game's own day care party screen)
        ChooseSendDaycareMon();
        break;
    case SCREEN_MOVE_SELECT:   // (Black's SelectMonMove: the game's summary on its moves, as the move deleter's)
        ShowSelectMovePokemonSummaryScreen(gPlayerParty, sMoveSel.party, gPlayerPartyCount - 1, CB2_ReturnToField,
                                           sMoveSel.newMove);
        gFieldCallback = FieldCB_ContinueScriptHandleMusic;
        break;
    case SCREEN_DIPLOMA:   // (diploma.c, back to the field and this script when it closes)
    {
        extern void (*gMpDiplomaExit)(void);
        void CB2_ShowDiploma(void);
        gMpDiplomaExit = CB2_ScreenDone;
        SetMainCallback2(CB2_ShowDiploma);
        break;
    }
    case SCREEN_EASY_CHAT:   // (easy_chat.c on the save's battle phrases; back to this script when it closes)
    {
        static const u8 sTypes[4] = {EASY_CHAT_TYPE_BATTLE_START, EASY_CHAT_TYPE_BATTLE_WON, EASY_CHAT_TYPE_BATTLE_LOST,
                                     EASY_CHAT_TYPE_BATTLE_WON};
        u16 *words = sEasyChatKind == 0 ? gSaveBlock1Ptr->easyChatBattleStart
                   : sEasyChatKind == 1 ? gSaveBlock1Ptr->easyChatBattleWon
                   : sEasyChatKind == 2 ? gSaveBlock1Ptr->easyChatBattleLost : sEasyChatWords;
        if (words == sEasyChatWords)
            memcpy(sEasyChatWords, gSaveBlock1Ptr->easyChatBattleWon, sizeof(sEasyChatWords));
        DoEasyChatScreen(sTypes[sEasyChatKind & 3], words, CB2_ScreenDone, EASY_CHAT_PERSON_DISPLAY_NONE);
        break;
    }
    case SCREEN_WORD:   // (Black's PMS input of one word: typed on the game's naming screen)
        sWordBuf[0] = EOS;
        MPNet_Log("u3d vm: a word asked for");
        DoNamingScreen(NAMING_SCREEN_UNOVA_WORD, sWordBuf, 0, 0, 0, CB2_ScreenDone);
        break;
    case SCREEN_RELEARN:   // (Black's CallMoveRelearn: the game's move relearner, the POKeMON in VAR_0x8004)
        TeachMoveRelearnerMove();
        break;
    case SCREEN_TRADE:   // (Black's trade: the game's in-game trade scene with its POKeMON, trade.c MP_SetUnovaTrade)
        UnovaTradeSetUp();
        gSpecialVar_0x8004 = 0;
        gSpecialVar_0x8005 = sTradeSlot;
        CreateInGameTradePokemon();
        DoInGameTradeScene();
        break;
    case SCREEN_SUBWAY_PARTY:   // (the game's frontier party menu: Black's number of POKeMON, any level, its banned list)
        gSpecialVar_0x8004 = FRONTIER_LVL_OPEN;
        gSpecialVar_0x8005 = U3D_Subway_MemberNum();
        ChoosePartyForBattleFrontier();
        break;
    case SCREEN_SUBWAY_BATTLE:
        U3D_Subway_StartBattle();
        BattleSetup_StartUnovaBattle();
        break;
    case SCREEN_BPSHOP:   // (u3d_bpshop.c: its end starts the script again)
        U3D_BpShop_Open(sBpShop);
        break;
    case SCREEN_MART:   // (the game's POKeMART on its BUY list: its end starts the script again)
        MP_CreatePokemartDirect(sMartItems, FALSE);
        break;
    case SCREEN_SELL:   // (... on SELL: the bag)
        sMartItems[0] = ITEM_NONE;
        MP_CreatePokemartDirect(sMartItems, TRUE);
        break;
    case SCREEN_SUBWAY_RESET:   // (a REST: the game saved by the script, back to the title screen)
        DoSoftReset();
        break;
    case SCREEN_CLEAR:   // (the script ended: the clear takes the game from here)
        sScreen = SCREEN_NONE;
        sThread.alive = FALSE;
        sHolding = FALSE;
        ScriptContext_Stop();
        UnovaGameClear();
        break;
    default:
        NothingToRun();
        break;
    }
}

// special U3D_VmResume: back from the screen, the script goes on
void U3D_VmResume(void)
{
    switch (sScreen)
    {
    case SCREEN_STARTER:
        *WorkPtr(sScreenVar) = gSpecialVar_Result < 3 ? gSpecialVar_Result : 0;
        break;
    case SCREEN_CATCH_DEMO:
        LoadPlayerParty();   // (the player's own party back)
        break;
    case SCREEN_WILD:
        // (Black's GetResultFlag: 0 caught, 1 ran, 2 knocked out; CheckLastBattleResult: the player not beaten)
        sWildResult = gBattleOutcome == B_OUTCOME_CAUGHT ? 0 : gBattleOutcome == B_OUTCOME_WON ? 2 : 1;
        U3D_BattleEnded(!(gBattleOutcome == B_OUTCOME_LOST || gBattleOutcome == B_OUTCOME_DREW));
        if (sZekromBattle && gBattleOutcome == B_OUTCOME_CAUGHT)
        {
            sWork.white |= 1;
            Gone(ZEKROM_LOCAL, TRUE);
            RemoveObjectEventByLocalIdAndMap(ZEKROM_LOCAL, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
            MPNet_Log("u3d vm: ZEKROM caught");
        }
        sZekromBattle = FALSE;
        break;
    case SCREEN_TRADE:
        break;
    case SCREEN_SUBWAY_PARTY:
        U3D_Subway_PartyChosen();
        break;
    case SCREEN_MOVE_SELECT:   // (VAR_0x8005: the slot, MAX_MON_MOVES for none or the new move kept out)
        *WorkPtr(sScreenVar) = gSpecialVar_0x8005 < MAX_MON_MOVES;
        if (sMoveSel.posWork)
            *WorkPtr(sMoveSel.posWork) = gSpecialVar_0x8005 < MAX_MON_MOVES ? gSpecialVar_0x8005 : 0;
        break;
    case SCREEN_EASY_CHAT:   // (gSpecialVar_Result: the words kept; Black's first two words of the phrase)
    {
        const u16 *words = sEasyChatKind == 0 ? gSaveBlock1Ptr->easyChatBattleStart
                         : sEasyChatKind == 1 ? gSaveBlock1Ptr->easyChatBattleWon
                         : sEasyChatKind == 2 ? gSaveBlock1Ptr->easyChatBattleLost : sEasyChatWords;
        *WorkPtr(sScreenVar) = gSpecialVar_Result != 0;
        *WorkPtr(sEasyChatOut[0]) = words[0];
        *WorkPtr(sEasyChatOut[1]) = words[1];
        break;
    }
    case SCREEN_WORD:   // (the word's number in Black's lists; "entered" only for a word they have)
    {
        u16 id = WordId(sWordBuf);
        *WorkPtr(sScreenVar) = id != 0xFFFF;
        *WorkPtr(sEasyChatOut[0]) = id;
        MPNet_Log("u3d vm: a word entered, Black's word %d", id == 0xFFFF ? -1 : id);
        break;
    }
    case SCREEN_RELEARN:   // (as Black's script reads it: 0 a move learned, the HEART SCALE taken; 1 none)
        *WorkPtr(sScreenVar) = gSpecialVar_0x8004 != TRUE;
        break;
    case SCREEN_DAYCARE_PARTY:
        if (sScreenVar)
            *WorkPtr(sScreenVar) = gSpecialVar_0x8004 < PARTY_SIZE ? gSpecialVar_0x8004 : PARTY_SIZE;
        break;
    case SCREEN_PARTY:
        if (sPartySlotWork)
            *WorkPtr(sPartySlotWork) = gSpecialVar_0x8004 < PARTY_SIZE ? gSpecialVar_0x8004 : 0xFF;
        if (sScreenVar)
            *WorkPtr(sScreenVar) = gSpecialVar_0x8004 < PARTY_SIZE;
        break;
    }
    sScreen = SCREEN_NONE;
    if (sThread.alive)
    {
        sThread.wait = W_NONE;
        if (!FuncIsActiveTask(Task_Vm))
            CreateTask(Task_Vm, 80);
    }
    else
        NothingToRun();
}

bool8 U3D_VmRunning(void)
{
    return sThread.alive;
}

// tests (mp_debug.c `mp u3dwork WORK VALUE` / `mp u3dflag FLAG 0|1`, Black's numbers in hex): a scene set up without
// playing the story to it
// (tests: `mp u3dget WORK` / `mp u3dget fFLAG`, hex: what a story step left behind)
// (tests: `mp u3drun ID`: a script started as the zone's scenes are, held by Unova_EventScript_Scene)
bool8 U3D_Vm_DebugRun(u16 id)
{
    if (!Ready() || sThread.alive || !Run(id, 0xFFFF, TRUE))
        return FALSE;
    ScriptContext_SetupScript(Unova_EventScript_Scene);
    return TRUE;
}

u16 U3D_Vm_DebugGet(bool8 flag, u16 id)
{
    return flag ? FlagOn(id) : *WorkPtr(id);
}

void U3D_Vm_DebugSet(bool8 flag, u16 id, u16 value)
{
    if (!flag && id == 0xFFFF)   // (`mp u3dwork FFFF 0`: the running script ends where it is: the opening's room)
    {
        if (sThread.alive)
            Finish(&sThread);
        return;
    }
    if (flag)
        FlagTo(id, value != 0);
    else
        *WorkPtr(id) = value;
}

// ------------------------------------------------------------------------------------------------ trainers' eyes
// Black's trainers who see the player (event_trainer_eye.c): a person of the zone's event data whose kind is 1 (sees
// the way it faces) or 2 (every way) and whose script is a trainer's (3000 + its number; 5000 + for a pair's second),
// not beaten (event flag 1420 + its number), the player in a straight line within its range (its first parameter),
// nothing blocking. The hit starts Black's common script 3616, which asks for these as it goes
#define SCRIPT_TRAINER_EYE 3616
#define TRAINER_FLAG_BASE 1420

static u16 TrainerOfScript(u16 script)
{
    return script >= 5000 ? script - 5000 : script >= 3000 ? script - 3000 : 0xFFFF;
}

static bool8 SeesPlayer(const u8 *p, u8 local, struct EyeHit *hit)
{
    struct ObjectEvent *o, *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 id = ObjEventId(local), d, dir, k;
    u16 trainer = TrainerOfScript(PersonScript(local, p)), range = Le16(p + 14);
    if (trainer == 0xFFFF || id >= OBJECT_EVENTS_COUNT || FlagOn(TRAINER_FLAG_BASE + trainer) || range == 0)
        return FALSE;
    o = &gObjectEvents[id];
    if (o->invisible || U3D_ObjectsApart(o, pl))   // (a trainer on another floor: CLAY's gym)
        return FALSE;
    for (dir = DIR_SOUTH; dir <= DIR_EAST; dir++)
    {
        s16 dx = gDirectionToVectors[dir].x, dy = gDirectionToVectors[dir].y;
        if (Le16(p + 6) == 1 && dir != o->facingDirection)
            continue;
        for (d = 1; d <= range; d++)
        {
            s16 x = o->currentCoords.x + dx * d, y = o->currentCoords.y + dy * d;
            if (x == pl->currentCoords.x && y == pl->currentCoords.y)
            {
                for (k = 1; k < d; k++)   // (nothing in the way)
                    if (MapGridGetCollisionAt(o->currentCoords.x + dx * k, o->currentCoords.y + dy * k)
                     || GetObjectEventIdByXY(o->currentCoords.x + dx * k, o->currentCoords.y + dy * k) != OBJECT_EVENTS_COUNT)
                        return FALSE;
                hit->obj = Le16(p), hit->trainer = trainer, hit->local = local, hit->distance = d, hit->dir = dir;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// field_control_avatar.c ProcessPlayerFieldInput, on an UNOVA map: a trainer has seen the player (their script runs)
bool8 U3D_Vm_TrainerSight(void)
{
    u32 size, i, n = 0;
    const u8 *e;
    if (!U3D_IsUnovaGroup(gSaveBlock1Ptr->location.mapGroup) || sThread.alive || !Ready())
        return FALSE;
    e = Events(CurrentZone(), &size);
    for (i = 0; e && size >= 8 && i < e[5] && n < 2; i++)
    {
        const u8 *p = e + 8 + EV_BG * e[4] + EV_PERSON * i;
        u16 kind = Le16(p + 6);
        if ((kind == 1 || kind == 2) && SeesPlayer(p, i + 1, &sEye[n]))
            n++;
    }
    if (n == 0)
        return FALSE;
    sEyeKind = 0;
    if (n == 2 && sEye[0].trainer == sEye[1].trainer)
        sEyeKind = 2;   // (a pair: one trainer of two)
    else if (n == 2 && CalculatePlayerPartyCount() >= 2)
        sEyeKind = 1;   // (two trainers at once)
    if (!Run(SCRIPT_TRAINER_EYE, sEye[0].obj, TRUE))
        return FALSE;
    ScriptContext_SetupScript(Unova_EventScript_Scene);
    return TRUE;
}

// CreateTrainerEye: the trainer's "!" and walk up to the player
static void StartEye(u8 slot)
{
    u8 id, n = 0, k;
    u8 *out;
    if (slot > 1 || (id = ObjEventId(sEye[slot].local)) >= OBJECT_EVENTS_COUNT)
        return;
    out = sMoves[id];
    out[n++] = MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK;
    for (k = 1; k < sEye[slot].distance && n < MAX_MOVE - 2; k++)
        out[n++] = MOVEMENT_ACTION_WALK_NORMAL_DOWN + (sEye[slot].dir - DIR_SOUTH);
    out[n] = MOVEMENT_ACTION_STEP_END;
    sMoving[id] = sEye[slot].local;
    ScriptMovement_StartObjectMovementScript(sEye[slot].local, gObjectEvents[id].mapNum, gObjectEvents[id].mapGroup, out);
}

// A trainer's words of KIND (a/0/9/0's records: their place is the message in system bank 189); 0xFFFF none
static u16 TrainerWords(u16 trainer, u16 kind)
{
    u32 size, i;
    const u8 *d = sTrMsgAt ? FileOf(sTrMsgAt, 0, &size) : NULL;
    for (i = 0; d && i + 4 <= size; i += 4)
        if (Le16(d + i) == trainer && Le16(d + i + 2) == kind)
            return i / 4;
    return 0xFFFF;
}

// The encounter song of a trainer's class (arm9's table; 1114 for one not in it)
static u16 EyeSong(u16 trainer)
{
    u32 size, i, n;
    const u8 *t = U3D_Vm_Trainer(trainer, &size);
    if (t == NULL || size < 2 || !sEyeSongsAt)
        return SongOf(1114);
    n = Get32(sEyeSongsAt);
    for (i = 0; i < n; i++)
        if (Le16(sPack + sEyeSongsAt + 4 + 4 * i) == t[1])
            return SongOf(Le16(sPack + sEyeSongsAt + 6 + 4 * i));
    return SongOf(1114);
}

// Black's warp points (ov010's 17: MAP_REPLACE / GAME_DATA_SetWarpId): u16 arrival flag, kind, the zone inside (a
// POKéMON CENTER, the player's home), its tile (x low byte, z high), the zone outside and its tile. Kai's whiteout
// returns to the save's last heal place (overworld.c SetWarpDestinationToLastHealLocation): the one inside, in front
// of the nurse, as Black's whiteout (or the outside one, for a point with no inside tile)
static void SetWhiteoutPoint(u16 warpId)
{
    const u8 *w;
    u8 group, num;
    s16 x, y;
    u16 zone, tx, tz;
    if (!Ready() || !sWarpsAt || warpId == 0 || warpId > Get32(sWarpsAt))
        return;
    w = sPack + sWarpsAt + 4 + 20 * (warpId - 1);
    zone = Le16(w + 4), tx = w[6], tz = w[7];
    if (Le16(w + 6) == 0)
        zone = Le16(w + 8), tx = Le16(w + 10), tz = Le16(w + 12);
    if (U3D_MapOfZoneTile(zone, tx, tz, &group, &num, &x, &y))
    {
        struct WarpData *h = &gSaveBlock1Ptr->lastHealLocation;
        h->mapGroup = group, h->mapNum = num, h->warpId = WARP_ID_NONE, h->x = x, h->y = y;
        MPNet_Log("u3d vm: whiteout point %u: zone %u (%d.%d at %d,%d)", warpId, zone, group, num, x, y);
    }
}

// A new game in UNOVA: Black's first warp point (the player's home in NUVEMA: MAP_REPLACE_GetDefaultWarpId)
void U3D_Vm_WhiteoutHome(void)
{
    SetWhiteoutPoint(1);
}

// The C-GEAR (FENNEL's gift in STRIATON): UNOVA's device once it's got
// The C-GEAR's ENTRALINK button (u3d_screen.c): off to the ENTRALINK (zone 279, Black's start tile)
#define ZONE_ENTRALINK 279
bool8 U3D_Vm_InEntralink(void)
{
    return CurrentZone() == ZONE_ENTRALINK;
}

void U3D_Vm_GoToEntralink(void)
{
    u8 group, num;
    s16 x, y;
    if (CurrentZone() == ZONE_ENTRALINK)
        return;
    if (U3D_MapOfZoneTile(ZONE_ENTRALINK, 16, 16, &group, &num, &x, &y))
    {
        MPNet_Log("u3d vm: the C-GEAR to the ENTRALINK");
        PlaySE(SE_WARP_IN);
        SetWarpDestination(group, num, WARP_ID_NONE, x, y);
        DoWarp();
    }
}

// The MUSICALS (u3d_musical.c): their saved state, a script of the zone run as a scene, the VM idle, a word slot
void *U3D_Vm_MusicalSave(void)
{
    return sWork.musical;
}

bool8 U3D_Vm_RunScene(u16 scriptId)
{
    extern const u8 Unova_EventScript_Scene[];
    if (!Run(scriptId, 0xFFFF, TRUE))
        return FALSE;
    ScriptContext_SetupScript(Unova_EventScript_Scene);
    return TRUE;
}

bool8 U3D_Vm_Idle(void)
{
    return !sThread.alive && !ScriptContext_IsEnabled();
}

#define SCRIPT_GAME_CLEAR 9601   // (Black's game-clear script: the works and flags of after the credits)
// (tests: `mp u3dclear`: as after the credits: Black's game-clear script, UNOVA cleared, its badges)
void U3D_Vm_DebugClear(void)
{
    RunNow(SCRIPT_GAME_CLEAR);
    MP_Region_SetCleared(REGION_UNOVA);
    sWork.badges = 0xFF;
    if (!CheckBagHasItem(ITEM_SS_TICKET, 1))
        AddBagItem(ITEM_SS_TICKET, 1);
    if (!CheckBagHasItem(ITEM_LIBERTY_PASS, 1))
        AddBagItem(ITEM_LIBERTY_PASS, 1);   // (as the real clear: UnovaGameClear)
    MPNet_Log("u3d vm: debug: after the credits");
}

// (tests: `mp u3dzone`: the script running where the player was ends)
void U3D_Vm_DebugStop(void)
{
    if (sThread.alive)
        Finish(&sThread);
}

u8 *U3D_Vm_WordSlot(u8 slot)
{
    return Slot(slot);
}

// The ENTRALINK's saved state and Black's PASS POWER table (u3d_entralink.c; u3d_tables.h sGPower)
void *U3D_Vm_EntralinkSave(void)
{
    return sWork.entralink;
}

const u16 *U3D_Vm_GPower(u8 id)
{
    return sGPower[id < ARRAY_COUNT(sGPower) ? id : 0];
}

// (mp_debug.c `mp cgear`: the C-GEAR given, for tests)
void U3D_Vm_GiveCGear(void)
{
    sWork.cgear = TRUE;
}

bool8 U3D_Vm_HasCGear(void)
{
    return sWork.cgear;
}

// A work of Black's event work (u3d.c: map replacements by story flags)
u16 U3D_Vm_Work(u16 id)
{
    return *WorkPtr(id);
}

// Black's EVENT_GAMECLEAR (ov010 event_gameclear.c): its recover script 9601 (the story's people put back, the party
// healed, home as the warp point and the continue spot), then mode 0 (N's Castle) the credits and the save, mode 1
// (the Champion) the HALL OF FAME and the credits (post_battle_event_funcs.c GameClear: the game's own). UNOVA counts as
// cleared (its boat, the GLOBAL PC) from the story's end.
void U3D_Vm_ContinueHome(void)
{
    if (!sSpecialLoc.set)
        SetWhiteoutPoint(1), sSpecialLoc.group = gSaveBlock1Ptr->lastHealLocation.mapGroup,
            sSpecialLoc.num = gSaveBlock1Ptr->lastHealLocation.mapNum, sSpecialLoc.x = gSaveBlock1Ptr->lastHealLocation.x,
            sSpecialLoc.y = gSaveBlock1Ptr->lastHealLocation.y, sSpecialLoc.set = TRUE;
    SetContinueGameWarp(sSpecialLoc.group, sSpecialLoc.num, WARP_ID_NONE, sSpecialLoc.x, sSpecialLoc.y);
}

static void UnovaGameClear(void)
{
    sSpecialLoc.set = FALSE;
    {
        u32 i;   // (MOVE_POKEMON_SpawnPendingEntries: a roamer knocked out roams again)
        for (i = 0; i < 2; i++)
            if (sWork.roam[i].state == 2)
            {
                sWork.roam[i].state = 0;
                RoamSpawn(i, sWork.roam[i].species);
            }
    }
    RunNow(SCRIPT_GAME_CLEAR);
    MP_Region_SetCleared(REGION_UNOVA);
    if (!CheckBagHasItem(ITEM_SS_TICKET, 1))
        AddBagItem(ITEM_SS_TICKET, 1);
    // the LIBERTY PASS as Kai's event tickets come (HOENN's with becoming Champion): its boat at CASTELIA's pier
    if (!CheckBagHasItem(ITEM_LIBERTY_PASS, 1))
        AddBagItem(ITEM_LIBERTY_PASS, 1);
    MPNet_Log("u3d vm: game clear, mode %u", sClearMode);
    if (sClearMode == 1)
    {
        FlagSet(FLAG_IS_CHAMPION);
        GameClear();   // (its UNOVA branch: U3D_Vm_ContinueHome)
        return;
    }
    HealPlayerParty();
    SetContinueGameWarpStatus();
    U3D_Vm_ContinueHome();
    TrySavingData(SAVE_NORMAL);
    SetMainCallback2(CB2_StartCreditsSequence);
}

static bool8 WhiteZekromHere(u8 localId)
{
    return localId == ZEKROM_LOCAL && CurrentZone() == ZONE_ZEKROM && MP_Region_Cleared(REGION_UNOVA) && !(sWork.white & 1)
        && FlagOn(766);
}

// ------------------------------------------------------------------------------------------------ the roamers
// Black's MOVE_POKEMON (ov010 move_pokemon.c): after the credits a scene on ROUTE 7 sets TORNADUS roaming its 16 route
// zones (d_021711C6), a fifth of the moves nowhere; Lv 40, its HP and status kept between battles. White's THUNDURUS
// roams beside it (the user, 2026-11-12). Met on its zone's grass half the time, as SINNOH's roamers.
static const u16 sRoamZones[16] = {319, 321, 326, 329, 331, 337, 345, 348, 355, 365, 368, 370, 374, 378, 383, 387};
static s8 sRoamBattled = -1;

static void RoamMove(struct UnovaRoamer *r)
{
    u8 to;
    if (r->loc != 16 && Random() % 100 < 20)
    {
        r->loc = 16;
        return;
    }
    do
        to = Random() % 16;
    while (to == r->loc);
    r->loc = to;
}

// (MOVE_POKEMON_FindEntryByZone: a roamer's zone has its weather, TORNADUS's strong storm, THUNDURUS's thunderstorm;
// zone_weather.c WEATHER_GetZoneWeatherId asks it first) 0xFFFF when none is there
u16 U3D_Vm_RoamerWeather(u16 zone)
{
    u32 i;
    for (i = 0; i < 2; i++)
        if (sWork.roam[i].state == 1 && sWork.roam[i].loc < 16 && sRoamZones[sWork.roam[i].loc] == zone)
            return sWork.roam[i].species == 642 ? 6 : 7;
    return 0xFFFF;
}

static void RoamSpawn(u8 slot, u16 national)
{
    struct UnovaRoamer *r = &sWork.roam[slot];
    struct Pokemon mon;
    if (r->state != 0)
        return;
    CreateMon(&mon, Species(national), 40, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    r->species = national, r->level = 40, r->state = 1, r->status = 0, r->loc = 5;
    r->personality = GetMonData(&mon, MON_DATA_PERSONALITY);
    r->ivs = GetMonData(&mon, MON_DATA_HP_IV) | GetMonData(&mon, MON_DATA_ATK_IV) << 5 | GetMonData(&mon, MON_DATA_DEF_IV) << 10
             | GetMonData(&mon, MON_DATA_SPEED_IV) << 15 | GetMonData(&mon, MON_DATA_SPATK_IV) << 20
             | GetMonData(&mon, MON_DATA_SPDEF_IV) << 25;
    r->hp = GetMonData(&mon, MON_DATA_MAX_HP);
    RoamMove(r);
    MPNet_Log("u3d vm: roamer %u set roaming", national);
}

static void RoamersMove(void)
{
    u32 i;
    for (i = 0; i < 2; i++)
        if (sWork.roam[i].state == 1)
            RoamMove(&sWork.roam[i]);
}

// (roamer.c TryStartRoamerEncounter in UNOVA) a roamer on this zone half the time: it, in gEnemyParty[0]
bool8 U3D_TryRoamerEncounter(u8 *level)
{
    u8 here[2], n = 0, i, s;
    u16 zone = CurrentZone();
    struct Pokemon *mon = &gEnemyParty[0];
    u32 status;
    for (i = 0; i < 2; i++)
        if (sWork.roam[i].state == 1 && sWork.roam[i].loc < 16 && sRoamZones[sWork.roam[i].loc] == zone)
            here[n++] = i;
    if (n == 0 || Random() % 2 == 0)
        return FALSE;
    s = here[n > 1 ? Random() % n : 0];
    ZeroEnemyPartyMons();
    CreateMonWithIVsPersonality(mon, Species(sWork.roam[s].species), sWork.roam[s].level, sWork.roam[s].ivs,
                                sWork.roam[s].personality);
    status = sWork.roam[s].status;
    SetMonData(mon, MON_DATA_STATUS, &status);
    SetMonData(mon, MON_DATA_HP, &sWork.roam[s].hp);
    sRoamBattled = s;
    *level = sWork.roam[s].level;
    MPNet_Log("u3d vm: roamer %u met (hp %u)", sWork.roam[s].species, sWork.roam[s].hp);
    return TRUE;
}

// (battle_main.c, a roamer battle's end) TRUE: it was UNOVA's
bool8 U3D_RoamerAfterBattle(struct Pokemon *mon, u8 outcome)
{
    s8 s = sRoamBattled;
    struct UnovaRoamer *r;
    sRoamBattled = -1;
    if (s < 0)
        return FALSE;
    r = &sWork.roam[s];
    if (outcome == B_OUTCOME_CAUGHT)
        r->state = 3;   // (Black's MOVE_POKEMON states: 3 caught, 2 knocked out: back at the next game clear)
    else if (outcome == B_OUTCOME_WON && GetMonData(mon, MON_DATA_HP) == 0)
        r->state = 2;
    else
    {
        r->hp = GetMonData(mon, MON_DATA_HP);
        r->status = GetMonData(mon, MON_DATA_STATUS);
        RoamMove(r);
    }
    MPNet_Log("u3d vm: roamer %u battle over (outcome %u)", r->species, outcome);
    return TRUE;
}

// Black's work object codes (WKOBJCODE00-0F, 0xA2-0xB1): a person drawn as the code its work 0x4020 + n holds, set by
// the zone's scripts (N's ZEKROM and RESHIRAM, a MUSICAL's performers...)
u16 U3D_Vm_ObjCode(u16 code)
{
    u16 v;
    if (code < 0xA2 || code > 0xB1)
        return code;
    v = sWork.work[0x20 + code - 0xA2];
    return v != 0 && v < OBJ_EVENT_GFX_UNOVA_COUNT ? v : code;
}

// (mp_debug.c `mp u3droam`: the roamers moved onto this zone if it's one of theirs, for tests)
bool8 U3D_Vm_RoamHere(void)
{
    u32 i, j;
    bool8 any = FALSE;
    for (i = 0; i < 2; i++)
        for (j = 0; j < 16; j++)
            if (sWork.roam[i].state == 1 && sRoamZones[j] == CurrentZone())
                sWork.roam[i].loc = j, any = TRUE;
    return any;
}

// ------------------------------------------------------------------------------------------------ moved connections
// WORK_CmdSetConnectGridPos (EVENTDATA_SYSTEM_SetConnectGridPos): a zone's script moves one of its connections (the
// GIANT CHASM's cave mouths, LOSTLORN FOREST's way out) for as long as the player is in the zone. The game's warps stay
// where the map has them; field_control_avatar.c GetWarpEventAtPosition asks here where each one stands now
static void ConnMove(u8 event, s16 x, s16 z)
{
    u32 i;
    for (i = 0; i < CONN_MOVES; i++)
        if (!sConnMove[i].set || sConnMove[i].event == event)
        {
            sConnMove[i].event = event, sConnMove[i].x = x, sConnMove[i].z = z, sConnMove[i].set = TRUE;
            return;
        }
}

// A warp tile of the map (its own coordinates) where it stands now
void U3D_Vm_WarpPos(s16 *x, s16 *y)
{
    u32 size, i, k;
    const u8 *e;
    s32 m, ax, az;
    if (!sConnMove[0].set || !U3D_IsUnovaGroup(gSaveBlock1Ptr->location.mapGroup))
        return;
    e = Events(CurrentZone(), &size);
    m = U3D_MapIndex();
    if (e == NULL || m < 0)
        return;
    ax = *x + 32 * U3D_MapCellX(m), az = *y + 32 * U3D_MapCellZ(m);
    for (i = 0; i < e[6]; i++)
    {
        const u8 *c = e + 8 + EV_BG * e[4] + EV_PERSON * e[5] + EV_CONNECT * i;
        s32 cx = (s16)Le16(c + 8) / 16, cz = (s16)Le16(c + 12) / 16, w = Le16(c + 14), h = Le16(c + 16);   // (ConnectData)
        if (w == 0)
            w = 1;
        if (h == 0)
            h = 1;
        if (ax < cx || az < cz || ax >= cx + w || az >= cz + h)
            continue;
        for (k = 0; k < CONN_MOVES && sConnMove[k].set; k++)
            if (sConnMove[k].event == i)
            {
                *x = sConnMove[k].x + (ax - cx) - 32 * U3D_MapCellX(m);
                *y = sConnMove[k].z + (az - cz) - 32 * U3D_MapCellZ(m);
            }
        return;
    }
}

// ------------------------------------------------------------------------------------------------ the trades
#define TRADE_BANK 0x23   // (FLD_TRADE_MSG_BANK in Black: the traded POKeMON's nicknames and their trainers' names)
static u32 Get32At(const u8 *p, u32 o)
{
    return p[o] | p[o + 1] << 8 | p[o + 2] << 16 | (u32)p[o + 3] << 24;
}

// ---- the BATTLE SUBWAY's data (u3d_subway.c): a set (16 bytes), a trainer's list, ov021's / ov010's tables
const u8 *U3D_Vm_SubwaySet(u16 id)
{
    if (!Ready() || sSubwayAt == 0 || id >= Get32(sSubwayAt))
        return NULL;
    return sPack + sSubwayAt + 4 + 16 * id;
}

const u8 *U3D_Vm_SubwayTrainer(u16 member, u32 *size)
{
    if (!Ready() || sSubwayAt == 0)
        return NULL;
    return FileOf(sSubwayTrainersAt, member, size);
}

const u8 *U3D_Vm_SubwayTables(u32 *size)
{
    if (!Ready() || sSubwayAt == 0)
        return NULL;
    *size = sPackSize - sSubwayTablesAt;
    return sPack + sSubwayTablesAt;
}

static const u8 *TradeRecord(u16 id)
{
    if (!Ready() || sTradesAt == 0 || id >= Get32(sTradesAt))
        return NULL;
    return sPack + sTradesAt + 4 + 0x6C * id;
}

// Black's gender numbers (0 male, 1 female, 2 none)
static u8 GenderOf(struct Pokemon *mon)
{
    u8 g = GetMonGender(mon);
    return g == MON_MALE ? 0 : g == MON_FEMALE ? 1 : 2;
}

void MP_SetUnovaTrade(const u8 *nickname, u16 species, const u8 *ivs, u8 abilityNum, u32 otId, u32 personality,
                      u16 heldItem, const u8 *otName, u8 otGender, u16 requestedSpecies);   // (trade.c)

// The trade's POKeMON as Black's FLD_TRADE_BuildPokemon makes it: its level, IVs, nature and gender (a personality that
// gives both), ability slot, held item, its nickname and its trainer's name (system bank 0x22) and ID
static void UnovaTradeSetUp(void)
{
    const u8 *r = TradeRecord(sTradeId);
    u8 ivs[NUM_STATS], i;
    u16 species;
    u32 pid = 0, tries;
    u8 nature, gender;
    if (r == NULL)
        return;
    species = Species(Get32At(r, 4));
    for (i = 0; i < NUM_STATS; i++)
        ivs[i] = Get32At(r, 0x10 + 4 * i) > 31 ? 31 : Get32At(r, 0x10 + 4 * i);
    nature = Get32At(r, 0x2C) % NUM_NATURES;
    gender = Get32At(r, 0x30);
    for (tries = 0; tries < 50000; tries++)
    {
        pid = Random32();
        if (pid % NUM_NATURES != nature)
            continue;
        if (gender == 2 || gSpeciesInfo[species].genderRatio == MON_GENDERLESS
         || (GetGenderFromSpeciesAndPersonality(species, pid) == MON_FEMALE) == (gender == 1))
            break;
    }
    {
        u8 nick[POKEMON_NAME_LENGTH + 1];   // (U3D_Text's buffer is reused by the next call)
        StringCopyN(nick, U3D_Text(TRADE_BANK, Get32At(r, 0x64)), POKEMON_NAME_LENGTH);
        nick[POKEMON_NAME_LENGTH] = EOS;
        MP_SetUnovaTrade(nick, species, ivs, Get32At(r, 0x28) ? 1 : 0, Get32At(r, 0x34), pid, Item(Get32At(r, 0x4C)),
                         U3D_Text(TRADE_BANK, Get32At(r, 0x68)), Get32At(r, 0x50) ? FEMALE : MALE,
                         Species(Get32At(r, 0x5C)));
    }
    MPNet_Log("u3d vm: trade %u: species %u for slot %u", sTradeId, species, sTradeSlot);
}

// ------------------------------------------------------------------------------------------------ BLACK CITY
// Black's WFBC (ov020 field_wfbc_data_local.c): ten residents drawn by weight from its thirty (a/1/5/4, scripts pack
// version 9) and three waiting, each a mood (falling 5 a day, 3 back for a talk; at 0 they leave for the waiting list),
// a battle a day on arrival; the ten stand at Black's spots (tools/unova/gen_maps.py) drawn as their own sprites (the
// work object codes 0xA2 + n). The other side, people from other games' cities, is CO-OP's (the ENTRALINK).
static const u8 *WfbcRecord(u8 npc)
{
    if (!Ready() || sWfbcAt == 0 || npc >= Get32(sWfbcAt))
        return NULL;
    return sPack + sWfbcAt + 4 + 0x30 * npc;
}

static u8 WfbcSlotOf(u8 npc)
{
    u8 i;
    for (i = 0; i < 10; i++)
        if (sWork.wfbc.npc[i] == npc)
            return i;
    return 0xFF;
}

static bool8 WfbcPresent(u8 npc)
{
    u8 i;
    for (i = 0; i < 10; i++)
        if (sWork.wfbc.npc[i] == npc || sWork.wfbc.back[i] == npc)
            return TRUE;
    return FALSE;
}

// FIELD_WFBC_DATA_LOCAL_PickNpcByWeight: the first absent one whose running weight passes ROLL
static u8 WfbcPick(u32 roll)
{
    u32 sum = 0, i;
    u8 last = 0;
    for (i = 0; i < 30; i++)
    {
        const u8 *r = WfbcRecord(i);
        if (r == NULL || WfbcPresent(i))
            continue;
        last = i;
        sum += Le16(r + 0x2C);
        if (sum > roll)
            return i;
    }
    return last;
}

// FIELD_WFBC_DATA_LOCAL_Init (Black: BLACK CITY, its residents' Black City moods)
static void WfbcInit(void)
{
    u32 remaining = 0x1CE, i;
    memset(&sWork.wfbc, 0xFF, sizeof(sWork.wfbc.npc) + sizeof(sWork.wfbc.mood) + sizeof(sWork.wfbc.flags) + sizeof(sWork.wfbc.back) + 1);
    sWork.wfbc.counter = sWork.wfbc.value = 0;
    for (i = 0; i < 13; i++)
    {
        u8 npc = WfbcPick(remaining ? Random() % remaining : 0);
        const u8 *r = WfbcRecord(npc);
        if (r == NULL)
            break;
        if (i < 10)
            sWork.wfbc.npc[i] = npc, sWork.wfbc.mood[i] = Le16(r + 6), sWork.wfbc.flags[i] = 7;
        else
            sWork.wfbc.back[i - 10] = npc;
        remaining -= Le16(r + 0x2C) < remaining ? Le16(r + 0x2C) : remaining;
    }
    sWork.wfbc.valid = 1;
    MPNet_Log("u3d vm: BLACK CITY's residents chosen");
}

static void WfbcMapLoad(void)
{
    u32 i;
    if (!sWork.wfbc.valid && sWfbcAt)
        WfbcInit();
    for (i = 0; i < 10; i++)
    {
        const u8 *r = sWork.wfbc.npc[i] != 0xFF ? WfbcRecord(sWork.wfbc.npc[i]) : NULL;
        sWork.work[0x20 + i] = r ? Le16(r) : 0;   // (the work object code 0xA2 + i: its own sprite)
    }
}

static u16 WfbcValue(u16 query)
{
    u8 slot = sWfbcSlot % 10, npc = sWork.wfbc.npc[slot], n = 0, i;
    const u8 *r = npc != 0xFF ? WfbcRecord(npc) : NULL;
    for (i = 0; i < 10; i++)
        n += sWork.wfbc.npc[i] != 0xFF;
    switch (query)
    {
    case 0:  return r ? Le16(r + 0x18) : 0;          // (its trainer)
    case 1:  return sWork.wfbc.mood[slot] <= 20;      // (bored)
    case 2:  return 1;                                // (BLACK CITY's own: the player its "parent")
    case 3:  return n;
    case 4:
    case 7:  return sWork.wfbc.counter;
    case 5:
    case 8:  return sWork.wfbc.value;
    case 6:  return n != 0;
    case 9:  return npc;
    case 10: return sWork.wfbc.flags[slot] & 1;       // (just arrived: a battle)
    case 11: return 0x80 + npc;
    }
    return 0;
}

// (clock.c's daily update, DAYS since the last) FIELD_WFBC_DATA_LOCAL_AdvanceDays and ApplyTalkedMoodBonus
void U3D_Vm_NewDay(u16 days)
{
    u32 i, j;
    s32 delta;
    if (!sWork.wfbc.valid)
        return;
    if (days > 21)
        days = 21;
    delta = -5 * (s32)(days ? days : 1);
    for (i = 0; i < 10; i++)
    {
        s32 m;
        if (sWork.wfbc.npc[i] == 0xFF)
            continue;
        m = sWork.wfbc.mood[i] + delta + ((sWork.wfbc.flags[i] & 2) ? 3 : 0);
        sWork.wfbc.flags[i] = 7;
        if (m > 100)
            m = 100;
        if (m <= 0)
        {
            for (j = 0; j < 10 && sWork.wfbc.back[j] != 0xFF; j++)
                ;
            if (j < 10)   // (bored: to the waiting list, first in it)
            {
                memmove(&sWork.wfbc.back[1], &sWork.wfbc.back[0], 9);
                sWork.wfbc.back[0] = sWork.wfbc.npc[i];
                sWork.wfbc.npc[i] = 0xFF;
                continue;
            }
            m = 1;
        }
        sWork.wfbc.mood[i] = m;
    }
}
