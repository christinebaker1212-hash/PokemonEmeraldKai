// Moemon Kai: SINNOH's elevators (tools/sinnoh/gen_maps.py, src/data/sinnoh_elevators.h): the panel or the attendant
// opens a floor menu (Sinnoh_EventScript_Elevator, data/scripts/sinnoh_elevators.inc) and the doors open on the floor
// picked: the elevator's door is a dynamic warp (Emerald's LILYCOVE elevator), set to that floor's door into it
#include "global.h"
#include "event_data.h"
#include "overworld.h"
#include "script_menu.h"
#include "constants/map_groups.h"
#include "constants/script_menu.h"
#include "constants/field_specials.h"
#include "script.h"
#include <stdio.h>
#include "string_util.h"
#include "strings.h"
#include "menu.h"
#include "s3d.h"
#include "text_window.h"
#include "window.h"
#include "../data/sinnoh_elevators.h"

extern const u8 *gQzScrollOptions[];
extern u8 gQzScrollCount;
extern u16 (*gQzScrollResult)(u16 choice);
void ShowScrollableMultichoice(void);

static u8 sPick;

static s32 Here(void)
{
    u32 i;
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    for (i = 0; i < ARRAY_COUNT(sSinnohElevators); i++)
        if (sSinnohElevators[i].map == map)
            return i;
    return -1;
}

// The floor the elevator is at: the one the player came in from (the dynamic warp Emerald sets on the way in)
static u8 FloorNow(s32 e)
{
    u8 i;
    u16 map = (gSaveBlock1Ptr->dynamicWarp.mapGroup << 8) | gSaveBlock1Ptr->dynamicWarp.mapNum;
    for (i = 0; i < sSinnohElevators[e].count; i++)
        if (sSinnohElevators[e].floors[i] == map)
            return i;
    return sSinnohElevators[e].count - 1;
}

static u16 Picked(u16 row)
{
    s32 e = Here();
    sPick = row;
    return e >= 0 && row < sSinnohElevators[e].count ? row : MULTI_B_PRESSED;
}

// The floor menu (waitstate; VAR_RESULT: the row, or MULTI_B_PRESSED)
u16 Sinnoh_ElevatorMenu(void)
{
    s32 e = Here();
    u8 i;
    if (e < 0)
    {
        gSpecialVar_Result = MULTI_B_PRESSED;
        ScriptContext_Enable();
        return 0;
    }
    for (i = 0; i < sSinnohElevators[e].count; i++)
        gQzScrollOptions[i] = sSinnohElevators[e].names[i];
    gQzScrollOptions[i] = gText_Exit;
    gQzScrollCount = i + 1;
    gQzScrollResult = Picked;
    gSpecialVar_0x8004 = SCROLL_MULTI_QZ_DYNAMIC;
    ShowScrollableMultichoice();
    return 0;
}

// VAR_RESULT 1: another floor (the doors now open there; MoveElevator's VAR_0x8005 / 0x8006: from, to)
u16 Sinnoh_ElevatorGo(void)
{
    s32 e = Here();
    const struct MapHeader *floor;
    u8 now, i;
    if (e < 0 || sPick >= sSinnohElevators[e].count || sPick == (now = FloorNow(e)))
        return gSpecialVar_Result = 0;
    floor = Overworld_GetMapHeaderByGroupAndId(sSinnohElevators[e].floors[sPick] >> 8, sSinnohElevators[e].floors[sPick] & 0xFF);
    for (i = 0; i < floor->events->warpCount; i++)
    {
        const struct WarpEvent *w = &floor->events->warps[i];
        if (((w->mapGroup << 8) | w->mapNum) == sSinnohElevators[e].map)
        {
            SetDynamicWarp(0, sSinnohElevators[e].floors[sPick] >> 8, sSinnohElevators[e].floors[sPick] & 0xFF, i);
            gSpecialVar_0x8005 = sSinnohElevators[e].count - now;
            gSpecialVar_0x8006 = sSinnohElevators[e].count - sPick;
            printf("[test] elevator: floor %d -> %d\n", now, sPick);
            return gSpecialVar_Result = 1;
        }
    }
    return gSpecialVar_Result = 0;
}

// ------------------------------------------------------------------------------------------------ Platinum's scripts
// The elevators' attendants run Platinum's own scripts (`sinnoh SINNOH_ELEVATOR`, tools/sinnoh/gen_scripts.py) on this
// elevator: the floor the player came in from is Platinum's "special location" (the dynamic warp), the floors above
// it Platinum's GetFloorsAbove (the table's floors are top first: the same count), SetSpecialLocation the floor the
// doors open on, PlayElevatorAnimation Emerald's MoveElevator.
static u8 sFloorWindow = WINDOW_NONE;

void Sinnoh_ElevatorCloseFloor(void)
{
    if (sFloorWindow != WINDOW_NONE)
    {
        ClearToTransparentAndRemoveWindow(sFloorWindow);
        sFloorWindow = WINDOW_NONE;
    }
}

void Sinnoh_ElevatorCall(u16 op, u16 a, u16 b)
{
    s32 e = Here();
    switch (op)
    {
    case 0:   // GetFloorsAbove VAR
        VarSet(a, e >= 0 ? FloorNow(e) : 0);
        break;
    case 1:   // BufferFloorNumber SLOT, FLOOR (0: B1F; Platinum's menu_entries words)
    {
        char name[32];
        const u8 *t;
        if (b == 0)
            t = S3D_TextByName("MenuEntries_Text_B1F");
        else
        {
            snprintf(name, sizeof(name), "MenuEntries_Text_%uF", b);
            t = S3D_TextByName(name);
        }
        StringCopy(a == 0 ? gStringVar1 : a == 1 ? gStringVar2 : gStringVar3, t ? t : gText_EmptyString2);
        break;
    }
    case 2:   // ShowCurrentFloor: "Current Floor" and the floor (slot 0) in a window while the floor menu is up
    {
        static const u8 sIndent[] = _("    ");
        u8 line[40];
        Sinnoh_ElevatorCloseFloor();
        {
            // (tiles of its own past the floor menu's, CreateWindowFromRect's at 100)
            u8 w = ConvertPixelWidthToTileWidth(GetStringWidth(FONT_NORMAL, S3D_TextByName("pl_msg_00000361_00015"), 0)) + 1;
            struct WindowTemplate t = CreateWindowTemplate(0, 2, 2, w, 4, 15, 300);
            sFloorWindow = AddWindow(&t);
            if (sFloorWindow == WINDOW_NONE)
                break;
            PutWindowTilemap(sFloorWindow);
        }
        SetStandardWindowBorderStyle(sFloorWindow, FALSE);
        AddTextPrinterParameterized(sFloorWindow, FONT_NORMAL, S3D_TextByName("pl_msg_00000361_00015"), 0, 1, TEXT_SKIP_DRAW, NULL);
        StringCopy(line, sIndent);
        StringAppend(line, gStringVar1);
        AddTextPrinterParameterized(sFloorWindow, FONT_NORMAL, line, 0, 17, TEXT_SKIP_DRAW, NULL);
        CopyWindowToVram(sFloorWindow, COPYWIN_FULL);
        break;
    }
    case 3:   // SetSpecialLocation: the doors open on map A (its warp into this elevator)
    {
        const struct MapHeader *floor = Overworld_GetMapHeaderByGroupAndId(a >> 8, a & 0xFF);
        u8 i;
        if (e < 0 || floor == NULL || floor->events == NULL)
            break;
        for (i = 0; i < floor->events->warpCount; i++)
        {
            const struct WarpEvent *w = &floor->events->warps[i];
            if (((w->mapGroup << 8) | w->mapNum) == sSinnohElevators[e].map)
            {
                SetDynamicWarp(0, a >> 8, a & 0xFF, i);
                printf("[test] elevator: doors open on %d.%d\n", a >> 8, a & 0xFF);
                break;
            }
        }
        break;
    }
    case 4:   // PlayElevatorAnimation DIR (0 up, 1 down; a variable), LOOPS: MoveElevator's from / to (VAR_0x8005 / 0x8006)
        a = VarGet(a);
        gSpecialVar_0x8005 = a == 0 ? 0 : 2;
        gSpecialVar_0x8006 = a == 0 ? 2 : 0;
        break;
    }
}
