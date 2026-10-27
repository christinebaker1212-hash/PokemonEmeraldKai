// Moemon Fight: the VS cabinet. Two slot machines back to back in MAUVILLE's GAME CORNER are a
// fighting game cabinet (the engine: fight/, include/mf_engine.h). Sitting down offers WAIT FOR 2P
// or FIGHT CPU. A fight is the game picture until it ends (MP_Fight_Step, like the card screens),
// then the cabinet's script goes on with the result (docs/FIGHT_DECISIONS.md).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp_fight.h"
#include "event_data.h"
#include "script.h"
#include "mf_engine.h"
#include "platform/fight.h"

enum
{
    FIGHT_RESULT_NONE,
    FIGHT_RESULT_WON,
    FIGHT_RESULT_LOST,
    FIGHT_RESULT_DRAW,
    FIGHT_RESULT_CANCELLED,
    FIGHT_RESULT_BROKEN,
};

static bool8 sRunning;      // a fight was started from the cabinet's script, which waits for it
static u8 sSeat;            // 0 west (left side), 1 east
static u8 sResult;
static u16 sTestButtons;    // the test harness's buttons (MP_Fight_TestPress)

void MPNet_Log(const char *fmt, ...);

void MP_Fight_TestPress(u16 buttons)
{
    sTestButtons = buttons;
}

bool8 MP_Fight_IsActive(void)
{
    return sRunning;
}

static u8 ResultFor(int result)
{
    switch (result)
    {
    case MF_RESULT_P1:
        return sSeat == 0 ? FIGHT_RESULT_WON : FIGHT_RESULT_LOST;
    case MF_RESULT_P2:
        return sSeat == 1 ? FIGHT_RESULT_WON : FIGHT_RESULT_LOST;
    case MF_RESULT_DRAW:
        return FIGHT_RESULT_DRAW;
    case MF_RESULT_ERROR:
        return FIGHT_RESULT_BROKEN;
    default:
        return FIGHT_RESULT_CANCELLED;
    }
}

bool8 MP_Fight_Step(void)
{
    unsigned short buttons[2] = {0, 0};

    if (!sRunning)
        return FALSE;
    if (!MfEngine_IsActive())
    {
        // the fight is over: back to the GAME CORNER, the script goes on
        sResult = ResultFor(MfEngine_Result());
        MPNet_Log("fight: result %d (engine %d, seat %d)", sResult, MfEngine_Result(), sSeat);
        if (sResult == FIGHT_RESULT_BROKEN)
            MPNet_Log("fight: %s", MfEngine_Error());
        sRunning = FALSE;
        ScriptContext_Enable();
        return FALSE;
    }
    buttons[sSeat] = Fight_PollButtons() | sTestButtons;
    MfEngine_Step(buttons);
    return TRUE;
}

void MP_Fight_StartCpu(void)
{
    struct MfSession session;

    memset(&session, 0, sizeof(session));
    // against the CPU the player is always on the left (the engine's arcade mode)
    sSeat = 0;
    session.mode = MF_MODE_CPU;
    session.seed = (unsigned int)rand();
    strcpy(session.cabinet, "mauville_gc");
    sResult = FIGHT_RESULT_NONE;
    if (!MfEngine_Begin(&session))
    {
        sResult = FIGHT_RESULT_BROKEN;
        ScriptContext_Enable();
        return;
    }
    sRunning = TRUE;
}

void MP_Fight_Result(void)
{
    gSpecialVar_Result = sResult;
}
