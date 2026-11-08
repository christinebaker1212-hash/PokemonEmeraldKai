// Moemon Kai: ORRE's opening (docs/ORRE_DECISIONS.md "The SNAG MACHINE: the player plays Colosseum's opening";
// docs/ORRE_HANDOFF.md "The opening"). Colosseum's opening is its own C code, not scripts: the decomp's demoScene00-30
// (src/game/gbaCommunication_*.c), each a camera shot (cameraPlayAnime) in one of the SNAGEM HIDEOUT's rooms, waited
// to its end, then the next room (floorRequestChange); the last goes to the OUTSKIRT STAND (room 1). The rooms aren't
// maps (no collision: kind 5, scenes only): ORRE's engine draws them as scene rooms under the shots (o3d.c
// O3D_SceneRoom). It has no words, so none are written. Played on the first landing without the SNAG MACHINE
// (data/maps/Orre_OutskirtStand/scripts.inc), which it gives at its end; START skips it.
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "main.h"
#include "task.h"
#include "palette.h"
#include "script.h"
#include "event_data.h"
#include "orre.h"
#include "o3d.h"
#include "constants/songs.h"
#include "sound.h"
#include "constants/rgb.h"

void MPNet_Log(const char *fmt, ...);
bool8 Orre_Vm_ScriptsOn(void);

// demoScene00-30 in order: the room the shot is in (the next scene's room is the one the scene before asked for) and
// the shot's resource id (cameraPlayAnime's; the data pack's index names its file)
static const struct { const char *room; u32 shot; } sScenes[] = {
    {"S2_out_1", 0x0B651800}, {"S2_out_1", 0x0B661800}, {"S2_bossroom_1", 0x0B871800}, {"S2_out_1", 0x0B831800},
    {"S2_bossroom_1", 0x0B881800}, {"S2_bossroom_1", 0x0B891800}, {"S2_out_3", 0x0CEF1800},
    {"S2_snatchroom_1", 0x0C3E1800}, {"S2_snatchroom_1", 0x0C401800}, {"S2_snatchroom_1", 0x0C411800},
    {"S2_snatchroom_1", 0x0C421800}, {"S2_hallway_1", 0x0C391800}, {"S2_out_3", 0x0CF01800},
    {"S2_out_3", 0x0CF11800}, {"S2_out_3", 0x0CF21800}, {"S2_out_3", 0x0CF31800}, {"S2_out_3", 0x0CF41800},
    {"S2_out_3", 0x0CF51800}, {"S2_out_3", 0x0CF61800}, {"S2_out_3", 0x0CF71800}, {"S2_out_3", 0x0CF91800},
    {"S2_out_3", 0x0CFA1800}, {"S2_out_3", 0x0CFB1800}, {"S2_out_3", 0x0CFC1800}, {"S2_out_3", 0x0CFD1800},
    {"S2_out_3", 0x0CFE1800}, {"S2_out_3", 0x0CFF1800}, {"S2_out_3", 0x0D001800}, {"S2_out_3", 0x0D011800},
    {"S2_out_3", 0x0D021800}, {"S2_out_3", 0x0D041800},
};

enum { OP_FADE_OUT, OP_NEXT, OP_FADE_IN, OP_PLAY, OP_END, OP_END_FADE };

#define tState data[0]
#define tScene data[1]

static bool8 Play(u32 i)
{
    if (!O3D_SceneRoom(sScenes[i].room))
        return FALSE;
    if (!O3D_CamPlayId(sScenes[i].shot, 0.0f, FALSE))
        return FALSE;
    O3D_CamSetKeep(TRUE);
    MPNet_Log("orre opening: scene %u (%s)", i, sScenes[i].room);
    return TRUE;
}

static void Task_OrreOpening(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (JOY_NEW(START_BUTTON) && tState != OP_END && tState != OP_END_FADE)
    {
        tState = OP_END;   // (skipped)
        MPNet_Log("orre opening: skipped");
    }
    switch (tState)
    {
    case OP_FADE_OUT:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        tState = OP_NEXT;
        break;
    case OP_NEXT:
        if (gPaletteFade.active)
            break;
        // (a scene that won't play is passed over: the rest still plays)
        while (tScene < (s16)ARRAY_COUNT(sScenes) && !Play(tScene))
            tScene++;
        tState = tScene < (s16)ARRAY_COUNT(sScenes) ? OP_FADE_IN : OP_END;
        break;
    case OP_FADE_IN:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        tState = OP_PLAY;
        break;
    case OP_PLAY:
        if (!O3D_CamDone())
            break;
        tScene++;
        // (a cut within a room: straight on; a new room: through black, as floorRequestChange fades)
        if (tScene < (s16)ARRAY_COUNT(sScenes) && strcmp(sScenes[tScene].room, sScenes[tScene - 1].room) == 0)
        {
            if (!Play(tScene))
                tState = OP_FADE_OUT;
        }
        else
            tState = OP_FADE_OUT;
        break;
    case OP_END:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        tState = OP_END_FADE;
        break;
    case OP_END_FADE:
        if (gPaletteFade.active)
            break;
        O3D_CamSetKeep(FALSE);
        O3D_CamStop();
        O3D_SceneRoom(NULL);
        Orre_SetFlag(ORRE_FLAG_SNAG_MACHINE, TRUE);   // (taken from the hideout: the opening's whole point)
        MPNet_Log("orre opening: done, the SNAG MACHINE");
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        DestroyTask(taskId);
        ScriptContext_Enable();
        break;
    }
}

// (special, waitstate) the opening, start to end
void Orre_Opening(void)
{
    u8 taskId = CreateTask(Task_OrreOpening, 80);
    gTasks[taskId].tState = OP_FADE_OUT;
    gTasks[taskId].tScene = 0;
}

// (special) VAR_RESULT: the opening is to play (ORRE's story on, no SNAG MACHINE yet)
void Orre_OpeningNeeded(void)
{
    gSpecialVar_Result = Orre_Vm_ScriptsOn() && !Orre_Flag(ORRE_FLAG_SNAG_MACHINE)
                      && !getenv("MOEMON_ORRE_NO_OPENING");   // (tests that start after it)
}
