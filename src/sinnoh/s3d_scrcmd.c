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
};

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
