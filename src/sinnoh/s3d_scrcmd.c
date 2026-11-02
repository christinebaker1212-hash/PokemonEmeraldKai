// Moemon Kai: this game's SINNOH script command (data/script_cmd_table.inc 0xF6, `sinnoh OP, ARG`): what Platinum's
// scripts do that this game's commands don't, for the scripts tools/sinnoh/gen_scripts.py converts from Platinum's own
// (docs/SINNOH_HANDOFF.md step 4).
#include "global.h"
#include "event_data.h"
#include "script.h"
#include "string_util.h"
#include "constants/vars.h"
#include "constants/item.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "item.h"
#include "fieldmap.h"
#include "party_menu.h"
#include "mp_region.h"
#include "s3d.h"

#include "data.h"
#include "battle.h"
#include "constants/battle.h"
#include "pokemon.h"
#include "rtc.h"
#include <time.h>
#include "menu.h"
#include "script_menu.h"
#include "sound.h"
#include "task.h"
#include "palette.h"
#include "constants/songs.h"
#include "pokedex.h"
#include "bike.h"
#include "battle_setup.h"
#include "overworld.h"

enum {
    SINNOH_BUFFER_PLAYER = 1,       // ARG: Platinum's string buffer 0-2 (STR_VAR_1-3) <- the player's name
    SINNOH_BUFFER_RIVAL,            // ... <- the rival's (BARRY)
    SINNOH_BUFFER_COUNTERPART,      // ... <- the counterpart's (DAWN for a boy, LUCAS for a girl)
    SINNOH_YESNO,                   // ARG: a variable <- the YES / NO just asked, as Platinum's menus give it (YES 0)
    SINNOH_GET_DIR,                 // ARG: a variable <- the way the player faces, Platinum's numbers (N 0 S 1 W 2 E 3)
    SINNOH_GET_POS,                 // ARG, ARG2: variables <- the player's tile in Platinum's map coordinates
    SINNOH_GET_POCKET,              // ARG: an item (or a variable holding it), ARG2: a variable <- its Platinum pocket
    SINNOH_IS_TMHM,                 // ARG: an item, ARG2: a variable <- TRUE for a TM / HM
    SINNOH_BUFFER_TMHM_MOVE,        // ARG: a string buffer, ARG2: a TM / HM <- its move's name
    SINNOH_BUFFER_POCKET,           // ARG: a string buffer, ARG2: a Platinum pocket <- its name
    SINNOH_BADGE_CHECK,             // ARG: Platinum's badge (BADGE_ID_*): VAR_RESULT and the comparison <- held or not
    SINNOH_NATIONAL_DEX,            // ARG: a variable <- the NATIONAL DEX is on
    SINNOH_MENU_INIT,               // ARG: the variable the choice goes in, ARG2: B closes it
    SINNOH_MENU_ADD,                // ARG, ARG2: a line of SINNOH's text (bank, message), ARG3: the value it gives
    SINNOH_MENU_SHOW,               // the menu, waited on: the variable <- the value chosen (B: Platinum's MENU_CANCEL)
    SINNOH_FIRST_NON_EGG,           // ARG: a variable <- the first party slot that isn't an EGG
    SINNOH_COUNT_ALIVE_EXCEPT,      // ARG: a variable <- the party's POKéMON able to battle, ARG2's slot left out
    SINNOH_HAS_SPECIES,             // ARG: a variable <- the party has ARG2's species
    SINNOH_DAY_OF_WEEK,             // ARG: a variable <- the PC's day of the week (Sunday 0)
    SINNOH_TIME_OF_DAY,             // ARG: a variable <- Platinum's time of day (morning 0, day, twilight, night, late night 4)
    SINNOH_ON_BIKE,                 // ARG: a variable <- the player rides a bike
    SINNOH_STARTER,                 // ARG: a variable <- the player's SINNOH starter (by the party's lines; TURTWIG else)
    SINNOH_FACE,                    // ARG: an object (or a variable holding it), ARG2: Platinum's way (N 0 S 1 W 2 E 3)
    SINNOH_GAME_COMPLETED,          // ARG: a variable <- SINNOH's CHAMPION beaten (its HALL OF FAME)
    SINNOH_DID_NOT_CAPTURE,         // ARG: a variable <- the last wild battle didn't end in a catch
    SINNOH_GET_HOUR,                // ARG: a variable <- the hour (the PC's clock)
    SINNOH_SET_WARP,                // ARG: the map's warp, ARG2, ARG3: its new tile (Platinum's SetWarpEventPos)
    SINNOH_SET_BIKE,                // ARG: TRUE on the bike, FALSE on foot (the game's own GetOnOffBike)
    SINNOH_PARTY_SPECIES,           // ARG: a party slot (or a variable), ARG2: a variable <- its species
    SINNOH_HELD_ITEM,               // ARG: an item, ARG2: a variable <- a party POKéMON holds it
    SINNOH_OBJECT_POS,              // ARG: an object, ARG2, ARG3: Platinum's map tile (each a value or a variable)
    SINNOH_TWO_TRAINERS,            // ARG, ARG2: two trainers, the script's data 0 / 1 their defeat lines: the battle
    SINNOH_SET_BG,                  // ARG: the map's sign / bg event, ARG2, ARG3: its new tile (Platinum's SetBgEventPos)
    SINNOH_BUFFER_RIVAL_STARTER,    // ARG: a string buffer <- the rival's starter (the one strong against the player's)
};

static struct MapEvents sStoryEvents;                 // (Sinnoh_PlatinumMapScripts below)
static struct ObjectEventTemplate sStoryObjects[64];
static struct WarpEvent sStoryWarps[64];
static struct BgEvent sStoryBgs[64];

// Platinum's menus are made as the script goes (InitGlobalTextMenu / AddMenuEntryImm / ShowMenu): here as a
// multichoice window of the same look as this game's, at the right of the screen
#define MENU_MAX 16
static struct
{
    u16 var;
    u8 count;
    bool8 canExit;
    u16 values[MENU_MAX];
    u8 texts[MENU_MAX][48];
    struct MenuAction actions[MENU_MAX];
} sMenu;

static void Task_SinnohMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    s8 selection;
    if (gPaletteFade.active)
        return;
    if (data[1])
    {
        data[1]--;
        return;
    }
    selection = sMenu.count > 3 ? Menu_ProcessInput() : Menu_ProcessInputNoWrap();
    if (selection == MENU_NOTHING_CHOSEN)
        return;
    if (selection == MENU_B_PRESSED)
    {
        if (!sMenu.canExit)
            return;
        PlaySE(SE_SELECT);
        VarSet(sMenu.var, 0xFFFE);   // (Platinum's MENU_CANCEL, -2)
    }
    else
        VarSet(sMenu.var, sMenu.values[selection]);
    ClearToTransparentAndRemoveWindow(data[0]);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void ShowSinnohMenu(void)
{
    int i, width = 0, left;
    u8 windowId, taskId;
    for (i = 0; i < sMenu.count; i++)
        width = DisplayTextAndGetWidth(sMenu.texts[i], width);
    width = ConvertPixelWidthToTileWidth(width);
    left = 29 - width;
    if (left < 0)
        left = 0;
    windowId = CreateWindowFromRect(left, 1, width, sMenu.count * 2);
    SetStandardWindowBorderStyle(windowId, FALSE);
    PrintMenuTable(windowId, sMenu.count, sMenu.actions);
    InitMenuInUpperLeftCornerNormal(windowId, sMenu.count, 0);
    ScheduleBgCopyTilemapToVram(0);
    taskId = CreateTask(Task_SinnohMenu, 80);
    gTasks[taskId].data[0] = windowId;
    gTasks[taskId].data[1] = 2;
}

// Platinum's pockets (include/constants/items.h there): ITEMS 0, MEDICINE 1, BALLS 2, TMHMS 3, BERRIES 4, MAIL 5,
// BATTLE ITEMS 6, KEY ITEMS 7
static u16 PlatinumPocket(u16 item)
{
    switch (GetItemPocket(item))
    {
    case POCKET_POKE_BALLS: return 2;
    case POCKET_TM_HM: return 3;
    case POCKET_BERRIES: return 4;
    case POCKET_KEY_ITEMS: return 7;
    default: return 0;
    }
}

static u8 *StringVar(u16 n)
{
    n = VarGet(n);   // (a buffer number, or a variable holding one)
    return n == 0 ? gStringVar1 : n == 1 ? gStringVar2 : gStringVar3;
}

static const u8 sPocket0[] = _("ITEMS"), sPocket1[] = _("MEDICINE"), sPocket2[] = _("POKé BALLS"),
                sPocket3[] = _("TMs & HMs"), sPocket4[] = _("BERRIES"), sPocket5[] = _("MAIL"),
                sPocket6[] = _("BATTLE ITEMS"), sPocket7[] = _("KEY ITEMS");
static const u8 *const sPockets[] = {sPocket0, sPocket1, sPocket2, sPocket3, sPocket4, sPocket5, sPocket6, sPocket7};

bool8 ScrCmd_sinnoh(struct ScriptContext *ctx)
{
    static const u8 sBarry[] = _("BARRY"), sDawn[] = _("DAWN"), sLucas[] = _("LUCAS");
    u8 op = ScriptReadByte(ctx);
    u16 arg = ScriptReadHalfword(ctx);
    u16 arg2 = ScriptReadHalfword(ctx);
    u16 arg3 = ScriptReadHalfword(ctx);
    switch (op)
    {
    case SINNOH_BUFFER_PLAYER:
        StringCopy(StringVar(arg), gSaveBlock2Ptr->playerName);
        break;
    case SINNOH_BUFFER_RIVAL:
        StringCopy(StringVar(arg), sBarry);
        break;
    case SINNOH_BUFFER_COUNTERPART:
        StringCopy(StringVar(arg), gSaveBlock2Ptr->playerGender == MALE ? sDawn : sLucas);
        break;
    case SINNOH_YESNO:
        VarSet(arg, gSpecialVar_Result == 1 ? 0 : 1);   // (yesnobox: 1 YES)
        break;
    case SINNOH_GET_DIR:
        switch (GetPlayerFacingDirection())
        {
        case DIR_NORTH: VarSet(arg, 0); break;
        case DIR_WEST: VarSet(arg, 2); break;
        case DIR_EAST: VarSet(arg, 3); break;
        default: VarSet(arg, 1); break;
        }
        break;
    case SINNOH_GET_POS:
    {
        s32 cx = 0, cz = 0;
        struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
        S3D_MapCells(&cx, &cz);
        VarSet(arg, player->currentCoords.x - MAP_OFFSET + cx * 32);
        VarSet(arg2, player->currentCoords.y - MAP_OFFSET + cz * 32);
        break;
    }
    case SINNOH_GET_POCKET:
        VarSet(arg2, PlatinumPocket(VarGet(arg)));
        break;
    case SINNOH_IS_TMHM:
        VarSet(arg2, GetItemPocket(VarGet(arg)) == POCKET_TM_HM);
        break;
    case SINNOH_BUFFER_TMHM_MOVE:
    {
        u16 item = VarGet(arg2), move = GetItemPocket(item) == POCKET_TM_HM ? ItemIdToBattleMoveId(item) : MOVE_NONE;
        StringCopy(StringVar(arg), gMoveNames[move < MOVES_COUNT ? move : MOVE_NONE]);
        break;
    }
    case SINNOH_BUFFER_POCKET:
        StringCopy(StringVar(arg), sPockets[VarGet(arg2) < 8 ? VarGet(arg2) : 0]);
        break;
    case SINNOH_MENU_INIT:
        sMenu.var = arg;
        sMenu.canExit = arg2;
        sMenu.count = 0;
        break;
    case SINNOH_MENU_ADD:
        if (sMenu.count < MENU_MAX)
        {
            u8 *t = sMenu.texts[sMenu.count];
            StringCopyN(t, S3D_Text(arg, arg2), sizeof(sMenu.texts[0]) - 1);
            t[sizeof(sMenu.texts[0]) - 1] = EOS;
            sMenu.actions[sMenu.count].text = t;
            sMenu.values[sMenu.count] = arg3;
            sMenu.count++;
        }
        break;
    case SINNOH_MENU_SHOW:
        if (sMenu.count == 0)
            break;
        ShowSinnohMenu();
        ScriptContext_Stop();
        return TRUE;
    case SINNOH_FIRST_NON_EGG:
    {
        u8 i;
        for (i = 0; i < PARTY_SIZE; i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
                break;
        VarSet(arg, i < PARTY_SIZE ? i : 0);
        break;
    }
    case SINNOH_COUNT_ALIVE_EXCEPT:
    {
        u8 i, n = 0;
        for (i = 0; i < PARTY_SIZE; i++)
            if (i != VarGet(arg2) && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE
             && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG) && GetMonData(&gPlayerParty[i], MON_DATA_HP) > 0)
                n++;
        VarSet(arg, n);
        break;
    }
    case SINNOH_HAS_SPECIES:
    {
        u8 i;
        bool8 has = FALSE;
        for (i = 0; i < PARTY_SIZE; i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == VarGet(arg2) && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
                has = TRUE;
        VarSet(arg, has);
        break;
    }
    case SINNOH_DAY_OF_WEEK:
    {
        time_t now = time(NULL);
        struct tm *tm = localtime(&now);
        VarSet(arg, tm ? tm->tm_wday : 0);
        break;
    }
    case SINNOH_TIME_OF_DAY:
    {
        u8 h;
        RtcCalcLocalTime();
        h = gLocalTime.hours;
        VarSet(arg, h >= 20 ? 3 : h >= 17 ? 2 : h >= 10 ? 1 : h >= 4 ? 0 : 4);
        break;
    }
    case SINNOH_ON_BIKE:
        VarSet(arg, TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE) != 0);
        break;
    case SINNOH_STARTER:
    {
        static const u16 sLines[3][3] = {{SPECIES_TURTWIG, SPECIES_GROTLE, SPECIES_TORTERRA},
                                         {SPECIES_CHIMCHAR, SPECIES_MONFERNO, SPECIES_INFERNAPE},
                                         {SPECIES_PIPLUP, SPECIES_PRINPLUP, SPECIES_EMPOLEON}};
        u8 i, k, l;
        u16 found = SPECIES_TURTWIG;
        if (VarGet(VAR_SINNOH_STARTER))
        {
            VarSet(arg, VarGet(VAR_SINNOH_STARTER));   // (picked from ROWAN's briefcase in Platinum's story)
            break;
        }
        for (i = 0; i < PARTY_SIZE; i++)
            for (l = 0; l < 3; l++)
                for (k = 0; k < 3; k++)
                    if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == sLines[l][k] && found == SPECIES_TURTWIG)
                        found = sLines[l][0];
        VarSet(arg, found);
        break;
    }
    case SINNOH_FACE:
    {
        static const u8 sDirs[4] = {DIR_NORTH, DIR_SOUTH, DIR_WEST, DIR_EAST};
        ObjectEventTurnByLocalIdAndMap(VarGet(arg), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup,
                                       sDirs[arg2 & 3]);
        break;
    }
    case SINNOH_GAME_COMPLETED:
        VarSet(arg, MP_Region_Cleared(REGION_SINNOH));
        break;
    case SINNOH_DID_NOT_CAPTURE:
        VarSet(arg, gBattleOutcome != B_OUTCOME_CAUGHT);
        break;
    case SINNOH_GET_HOUR:
        RtcCalcLocalTime();
        VarSet(arg, gLocalTime.hours);
        break;
    case SINNOH_SET_WARP:
        if (gMapHeader.events == &sStoryEvents && arg < sStoryEvents.warpCount)
        {
            sStoryWarps[arg].x = (s16)arg2;
            sStoryWarps[arg].y = (s16)arg3;
        }
        break;
    case SINNOH_SET_BG:
        if (gMapHeader.events == &sStoryEvents && arg < sStoryEvents.bgEventCount)
        {
            sStoryBgs[arg].x = arg2;
            sStoryBgs[arg].y = arg3;
        }
        break;
    case SINNOH_BUFFER_RIVAL_STARTER:
    {
        u16 mine = VarGet(VAR_SINNOH_STARTER);
        GetSpeciesName(StringVar(arg), mine == SPECIES_TURTWIG ? SPECIES_CHIMCHAR : mine == SPECIES_CHIMCHAR ? SPECIES_PIPLUP
                                                                                                          : SPECIES_TURTWIG);
        break;
    }
    case SINNOH_SET_BIKE:
        if (!arg != !TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE))
            GetOnOffBike(PLAYER_AVATAR_FLAG_MACH_BIKE);
        break;
    case SINNOH_PARTY_SPECIES:
    {
        u16 slot = VarGet(arg);
        VarSet(arg2, slot < PARTY_SIZE ? GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES_OR_EGG) : SPECIES_NONE);
        break;
    }
    case SINNOH_HELD_ITEM:
    {
        u32 i;
        u16 item = VarGet(arg);
        VarSet(arg2, FALSE);
        for (i = 0; i < CalculatePlayerPartyCount(); i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_HELD_ITEM) == item)
                VarSet(arg2, TRUE);
        break;
    }
    case SINNOH_OBJECT_POS:
    {
        s32 cx = 0, cz = 0;
        u8 id = VarGet(arg);
        s16 x, y;
        S3D_MapCells(&cx, &cz);
        x = VarGet(arg2) - cx * 32;
        y = VarGet(arg3) - cz * 32;
        SetObjEventTemplateCoords(id, x, y);
        TryMoveObjectEventToMapCoords(id, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, x, y);
        break;
    }
    case SINNOH_TWO_TRAINERS:
        MP_StartTwoTrainerBattle(arg, arg2, (const u8 *)ctx->data[0], (const u8 *)ctx->data[1]);
        return TRUE;
    case SINNOH_NATIONAL_DEX:
        VarSet(arg, IsNationalPokedexEnabled());
        break;
    case SINNOH_BADGE_CHECK:
        gSpecialVar_Result = arg < 8 && FlagGet(FLAG_SINNOH_BADGE01 + arg);
        ctx->comparisonResult = gSpecialVar_Result;   // (as checkflag: goto_if TRUE / FALSE)
        break;
    }
    return FALSE;
}

// Platinum's story (FLAG_SINNOH_PLATINUM_STORY): each SINNOH map's own map scripts with Platinum's added (on load,
// on transition, on resume, the frame table), its people as Platinum's (their hide flags, places and converted
// scripts) and its step triggers, all from sinnoh/platinum_scripts.inc (tools/sinnoh/gen_scripts.py map_index, made
// when the game is built; its end: map 0xFFFF). The map header keeps the map's own events: gMapHeader gets a copy
struct SinnohStoryPerson { u8 localId, pad; u16 flag; s16 x, y; const u8 *script; };
struct SinnohMapStory
{
    u16 map, people, triggers, pad;
    const u8 *scripts;
    const struct SinnohStoryPerson *person;
    const struct CoordEvent *trigger;
};
extern const struct SinnohMapStory gSinnohPlatinumMapScripts[];


void Sinnoh_PlatinumMapScripts(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    const struct SinnohMapStory *m;
    u32 i, j;
    if (!S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup) || !FlagGet(FLAG_SINNOH_PLATINUM_STORY))
        return;
    for (m = gSinnohPlatinumMapScripts; m->map != 0xFFFF; m++)
        if (m->map == map)
            break;
    if (m->map == 0xFFFF)
        return;
    if (m->scripts)
        gMapHeader.mapScripts = m->scripts;
    if (gMapHeader.events == NULL)
        return;
    sStoryEvents = *gMapHeader.events;
    if (sStoryEvents.warpCount <= ARRAY_COUNT(sStoryWarps))
    {
        // (Platinum's scripts move warps: SINNOH_SET_WARP)
        memcpy(sStoryWarps, sStoryEvents.warps, sStoryEvents.warpCount * sizeof(sStoryWarps[0]));
        sStoryEvents.warps = sStoryWarps;
    }
    if (sStoryEvents.bgEventCount <= ARRAY_COUNT(sStoryBgs))
    {
        memcpy(sStoryBgs, sStoryEvents.bgEvents, sStoryEvents.bgEventCount * sizeof(sStoryBgs[0]));
        sStoryEvents.bgEvents = sStoryBgs;
    }
    if (m->people && sStoryEvents.objectEventCount <= ARRAY_COUNT(sStoryObjects))
    {
        memcpy(sStoryObjects, sStoryEvents.objectEvents, sStoryEvents.objectEventCount * sizeof(sStoryObjects[0]));
        for (i = 0; i < m->people; i++)
            for (j = 0; j < sStoryEvents.objectEventCount; j++)
                if (sStoryObjects[j].localId == m->person[i].localId)
                {
                    sStoryObjects[j].flagId = m->person[i].flag;
                    sStoryObjects[j].x = m->person[i].x;
                    sStoryObjects[j].y = m->person[i].y;
                    if (m->person[i].script)
                        sStoryObjects[j].script = m->person[i].script;
                    break;
                }
        sStoryEvents.objectEvents = sStoryObjects;
    }
    if (m->triggers)
    {
        sStoryEvents.coordEvents = m->trigger;
        sStoryEvents.coordEventCount = m->triggers;
    }
    gMapHeader.events = &sStoryEvents;
}

// Platinum's story begins (a new game in SINNOH, or a save's first time there): its flag, and Platinum's own new-game
// script (its people hidden until the story shows them: scripts_init_new_game, converted by gen_scripts.py)
extern const u8 gSinnohPlatinumNewGame[];
void Sinnoh_PlatinumStoryBegin(void)
{
    FlagSet(FLAG_SINNOH_PLATINUM_STORY);
    RunScriptImmediately(gSinnohPlatinumNewGame);
}
