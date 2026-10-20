// Moemon Co-op: quiet autosave. Every few minutes of play on the map (sooner
// after a battle), the game saves itself the moment the player is standing
// free: no script, menu, message, battle or warp running. Nothing is shown.
// It never saves where the game's own START menu has no SAVE (link rooms,
// SAFARI ZONE, BATTLE PIKE / PYRAMID...), never over an older save file from a
// new game that wasn't saved by hand yet, and not during the intro truck ride.
#include "global.h"
#include "mp.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "cable_club.h"
#include "event_data.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "fieldmap.h"
#include "link.h"
#include "main.h"
#include "overworld.h"
#include "safari_zone.h"
#include "save.h"
#include "union_room.h"
#include "constants/battle_pyramid.h"

extern bool8 gDifferentSaveFile;

#define FRAMES_PER_MINUTE  (60 * 60)
#define AFTER_BATTLE_DELAY (1 * FRAMES_PER_MINUTE)   // progress from a battle (catches, levels) is kept soon
#define SETTLE_FRAMES      30                         // standing free this long first

static u32 sFrames;        // map time since the last save
static bool8 sAfterBattle;
static u8 sSettle;

void MP_Autosave_Saved(void)
{
    sFrames = 0;
    sAfterBattle = FALSE;
    sSettle = 0;
}

static bool8 CanSaveHere(void)
{
    if (!MP_IsLocalFree() || gMain.callback2 != CB2_Overworld || MP_QuickPickOpen() || MP_Piggy_IsRiding())
        return FALSE;
    if (gPlayerAvatar.tileTransitionState != T_NOT_MOVING)
        return FALSE;
    // Places the START menu has no SAVE in
    if (IsOverworldLinkActive() || InUnionRoom() || GetSafariZoneFlag() || InBattlePike()
     || CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE || InMultiPartnerRoom())
        return FALSE;
    if (FlagGet(FLAG_MP_SKIP_TRUCK))
        return FALSE;   // still riding in the intro truck
    // A new game started over an older save: only a save by hand may replace it
    if (gDifferentSaveFile && gSaveFileStatus != SAVE_STATUS_EMPTY && gSaveFileStatus != SAVE_STATUS_CORRUPT)
        return FALSE;
    return TRUE;
}

static void Save(void)
{
    u8 status;
    SaveMapView();
    if (gDifferentSaveFile == TRUE)
    {
        status = TrySavingData(SAVE_OVERWRITE_DIFFERENT_FILE);
        gDifferentSaveFile = FALSE;
    }
    else
    {
        status = TrySavingData(SAVE_NORMAL);
    }
    MPNet_Log("autosave: saved (status %d)", status);
}

// Every frame (MP_FrameUpdate)
void MP_Autosave_Update(void)
{
    u32 due;
    if (gMpConfig.autosaveMinutes == 0)
        return;
    if (gMain.inBattle)
    {
        sAfterBattle = TRUE;
        return;
    }
    if (!MP_IsInOverworld())
        return;
    sFrames++;
    due = gMpConfig.autosaveMinutes * FRAMES_PER_MINUTE;
    if (sAfterBattle && due > AFTER_BATTLE_DELAY)
        due = AFTER_BATTLE_DELAY;
    if (sFrames < due)
        return;
    if (!CanSaveHere())
    {
        sSettle = 0;
        return;
    }
    if (++sSettle < SETTLE_FRAMES)
        return;
    Save();
    MP_Autosave_Saved();
}
